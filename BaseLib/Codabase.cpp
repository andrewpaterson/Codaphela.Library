#include "Logger.h"
#include "NativeThread.h"
#include "WriteTransaction.h"
#include "ReadTransaction.h"
#include "IndexTreeHelper.h"
#include "Codabase.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::Init(const char* szDirectory, EDurableSync eSync, CLifeInit<CIndexedDataConfig> cIndexConfig, CLifeInit<CNamedIndexesConfig> cNamedConfig)
{
	CIndexTreeHelper	cHelper;

	cHelper.Init(szDirectory, "Primary", "Backup", false);

	//A previous session that crashed during a commit is recovered by Start.
	mbControllerInitialised = mcFileController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	mcFileController.SetSync(eSync);
	mcIndexConfig = cIndexConfig;
	mcNamedConfig = cNamedConfig;
	mbStarted = false;
	mcMutex.Init();
	mbFailed = false;
	mcVersions.Init();
	miCommitNumber = 0;
	maiSnapshots.Init();
	mcLocks.Init();
	mcReservedNames.Init(true, true);
	miLastStartNumber = 0;
	mapcWriters.Init();
	mcSessionMutex.Init();
	mcSessionFinished.Init();
	mpcSession = NULL;
	miSessionThread = 0;

	cHelper.Kill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Start(void)
{
	bool	bResult;

	if (mbStarted)
	{
		return gcLogger.Error2(__METHOD__, " Codabase has already been started.", NULL);
	}
	if (!mbControllerInitialised)
	{
		return gcLogger.Error2(__METHOD__, " Codabase file controller could not be initialised.", NULL);
	}

	mcMutex.Lock();
	bResult = mcFileController.Recover();
	if (!bResult)
	{
		mcMutex.Unlock();
		return gcLogger.Error2(__METHOD__, " Codabase could not recover from an incomplete commit.", NULL);
	}

	//The bracket stays open from here on so the store can always be read.  Commits end it and begin the next one.
	bResult = mcFileController.Begin();
	if (bResult)
	{
		mcNamedIndexedData.Init(&mcFileController, mcIndexConfig, mcNamedConfig);
		mbStarted = true;
		bResult = mcFileController.End();
		bResult = bResult && mcFileController.Begin();
	}
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::Kill(void)
{
	CLife<CIndexedDataConfig>		cIndexLife;
	CLife<CNamedIndexesConfig>		cNamedLife;
	CIndexedDataConfig*				pcIndexConfig;
	CNamedIndexesConfig*			pcNamedConfig;

	if (mpcSession != NULL)
	{
		//A session that was not closed is aborted.
		SafeKill(mpcSession);
	}

	if (mbStarted)
	{
		//The open bracket is thrown away: whatever was not committed is lost, as in a crash.
		mcFileController.Abort();
		mcNamedIndexedData.Kill();
	}
	else
	{
		//The configs were never handed to mcNamedIndexedData so they must be released here.
		mcIndexConfig.ConfigureLife(&cIndexLife, &pcIndexConfig);
		cIndexLife.Kill();
		mcNamedConfig.ConfigureLife(&cNamedLife, &pcNamedConfig);
		cNamedLife.Kill();
	}
	mcFileController.Kill();
	mcVersions.Kill();
	maiSnapshots.Kill();
	mcLocks.Kill();
	mcReservedNames.Kill();
	mapcWriters.Kill();
	mcSessionFinished.Kill();
	mcSessionMutex.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Open(void)
{
	CWriteTransaction*	pcSession;

	if (!mbStarted)
	{
		return gcLogger.Error2(__METHOD__, " Cannot open a Codabase that is not started.", NULL);
	}
	if (HasWriterOnThisThread())
	{
		return gcLogger.Error2(__METHOD__, " This thread already has a write transaction.  The session could wait for it for ever.", NULL);
	}

	//Only one session at a time.  It is an implicit write transaction with start number 0, the oldest.
	mcSessionMutex.Lock();
	if ((mpcSession != NULL) && (miSessionThread == CurrentThreadId()))
	{
		mcSessionMutex.Unlock();
		return gcLogger.Error2(__METHOD__, " This thread already has the Codabase open.  Waiting for itself would never end.", NULL);
	}
	while (mpcSession != NULL)
	{
		mcSessionFinished.Wait(&mcSessionMutex);
	}
	if (IsFailed())
	{
		mcSessionMutex.Unlock();
		return gcLogger.Error2(__METHOD__, " A commit failed.  The Codabase is read only until it is restarted.", NULL);
	}

	pcSession = NewMalloc<CWriteTransaction>();
	pcSession->Init(this, 0);
	mpcSession = pcSession;
	miSessionThread = CurrentThreadId();
	mcSessionMutex.Unlock();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Close(void)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession == NULL)
	{
		mcSessionMutex.Unlock();
		return gcLogger.Error2(__METHOD__, " Cannot close a Codabase that is not open.", NULL);
	}

	bResult = mpcSession->Commit();
	SafeKill(mpcSession);
	miSessionThread = 0;
	mcSessionFinished.WakeAll();
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CWriteTransaction* CCodabase::BeginWriteTransaction(void)
{
	CWriteTransaction*	pcTransaction;
	int64				iStartNumber;
	bool				bSessionThread;

	if (!mbStarted)
	{
		gcLogger.Error2(__METHOD__, " Cannot begin a write transaction on a Codabase that is not started.", NULL);
		return NULL;
	}

	mcSessionMutex.Lock();
	bSessionThread = (mpcSession != NULL) && (miSessionThread == CurrentThreadId());
	mcSessionMutex.Unlock();
	if (bSessionThread)
	{
		gcLogger.Error2(__METHOD__, " This thread has the Codabase open.  The session could wait for the transaction for ever.", NULL);
		return NULL;
	}

	//Write transactions do not wait for each other here.  They wait, or die, when they lock objects.
	mcMutex.Lock();
	if (mbFailed)
	{
		mcMutex.Unlock();
		gcLogger.Error2(__METHOD__, " A commit failed.  The Codabase is read only until it is restarted.", NULL);
		return NULL;
	}
	miLastStartNumber++;
	iStartNumber = miLastStartNumber;
	pcTransaction = NewMalloc<CWriteTransaction>();
	pcTransaction->Init(this, iStartNumber);
	mapcWriters.Add(&pcTransaction);
	mcMutex.Unlock();
	return pcTransaction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CReadTransaction* CCodabase::BeginReadTransaction(void)
{
	CReadTransaction*	pcTransaction;
	int64				iSnapshot;

	if (!mbStarted)
	{
		gcLogger.Error2(__METHOD__, " Cannot begin a read transaction on a Codabase that is not started.", NULL);
		return NULL;
	}

	//Reading never waits for a writer.  The snapshot is the last published commit.
	mcMutex.Lock();
	iSnapshot = miCommitNumber;
	maiSnapshots.Add(&iSnapshot);
	mcMutex.Unlock();

	pcTransaction = NewMalloc<CReadTransaction>();
	pcTransaction->Init(this, iSnapshot);
	return pcTransaction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::GetCommitNumber(void)
{
	int64	iCommitNumber;

	mcMutex.Lock();
	iCommitNumber = miCommitNumber;
	mcMutex.Unlock();
	return iCommitNumber;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCodabase::NumVersions(void)
{
	size	uiNumVersions;

	mcMutex.Lock();
	uiNumVersions = mcVersions.NumVersions();
	mcMutex.Unlock();
	return uiNumVersions;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::IsFailed(void)
{
	bool	bFailed;

	mcMutex.Lock();
	bFailed = mbFailed;
	mcMutex.Unlock();
	return bFailed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::CommitLocked(void)
{
	bool	bResult;

	//mcMutex must be locked.  The open bracket is committed and the next one begun.
	if (mbFailed)
	{
		return gcLogger.Error2(__METHOD__, " A commit failed.  The Codabase is read only until it is restarted.", NULL);
	}

	bResult = true;
	if (!mcNamedIndexedData.IsWriteThrough())
	{
		bResult = mcNamedIndexedData.Flush(false);
	}
	bResult = bResult && mcFileController.End();
	bResult = bResult && mcFileController.Begin();
	if (!bResult)
	{
		//The files may be part way through the commit and the caches may not match them.  Start recovers.
		mbFailed = true;
		return gcLogger.Error2(__METHOD__, " Commit failed.  The Codabase is read only until it is restarted.", NULL);
	}

	//Publish the commit only once it is durable.  Read transactions that start from now on see it.
	miCommitNumber++;
	RemoveOldVersions();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLockManager* CCodabase::GetLockManager(void)
{
	return &mcLocks;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::NeedsSession(const char* szMethod)
{
	return gcLogger.Error2(szMethod, " Writing directly to a Codabase needs Open first.  Use a write transaction otherwise.", NULL);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::HasWriterOnThisThread(void)
{
	size	i;
	int		iThread;
	bool	bFound;

	iThread = CurrentThreadId();
	bFound = false;
	mcMutex.Lock();
	for (i = 0; i < mapcWriters.NumElements(); i++)
	{
		if ((*mapcWriters.Get(i))->GetThread() == iThread)
		{
			bFound = true;
			break;
		}
	}
	mcMutex.Unlock();
	return bFound;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::RemoveWriter(CWriteTransaction* pcTransaction)
{
	size	i;

	mcMutex.Lock();
	for (i = 0; i < mapcWriters.NumElements(); i++)
	{
		if (*mapcWriters.Get(i) == pcTransaction)
		{
			mapcWriters.RemoveAt(i);
			break;
		}
	}
	mcMutex.Unlock();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::ReserveName(const char* szName, int64 iTransaction)
{
	int64*	piOwner;

	//mcMutex must be locked.  The first transaction to add a name owns it until it commits or aborts.
	piOwner = mcReservedNames.Get(szName);
	if (piOwner != NULL)
	{
		return *piOwner == iTransaction;
	}
	mcReservedNames.Put(szName, &iTransaction);
	return true;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::ReleaseName(const char* szName, int64 iTransaction)
{
	int64*	piOwner;

	//mcMutex must be locked.
	piOwner = mcReservedNames.Get(szName);
	if ((piOwner != NULL) && (*piOwner == iTransaction))
	{
		mcReservedNames.Remove(szName);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::StoredGet(OIndex oi, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	bool	bResult;

	//The committed store, whether or not a session is open.
	mcMutex.Lock();
	bResult = mcNamedIndexedData.Get(oi, puiDataSize, pvData, uiMaxDataSize);
	mcMutex.Unlock();
	return bResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::StoredGetAll(OIndex oi, void* pvData)
{
	bool	bResult;

	//The committed store, whether or not a session is open.
	mcMutex.Lock();
	bResult = mcNamedIndexedData.Get(oi, pvData);
	mcMutex.Unlock();
	return bResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::StoredContains(OIndex oi)
{
	bool	bResult;

	//The committed store, whether or not a session is open.
	mcMutex.Lock();
	bResult = mcNamedIndexedData.Contains(oi);
	mcMutex.Unlock();
	return bResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::StoredGetName(OIndex oi, CChars* pszName)
{
	bool	bResult;

	//The committed store, whether or not a session is open.
	mcMutex.Lock();
	bResult = mcNamedIndexedData.GetName(oi, pszName);
	mcMutex.Unlock();
	return bResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::StoredGetIndex(const char* szName)
{
	OIndex	bResult;

	//The committed store, whether or not a session is open.
	mcMutex.Lock();
	bResult = mcNamedIndexedData.GetIndex(szName);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::RemoveOldVersions(void)
{
	size	i;
	int64	iOldest;
	int64	iSnapshot;

	//mcMutex must be locked.  Versions that no open read transaction can see are thrown away.
	iOldest = miCommitNumber;
	for (i = 0; i < maiSnapshots.NumElements(); i++)
	{
		iSnapshot = *maiSnapshots.Get(i);
		if (iSnapshot < iOldest)
		{
			iOldest = iSnapshot;
		}
	}
	mcVersions.RemoveUpTo(iOldest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::EndRead(int64 iSnapshot)
{
	size	i;

	mcMutex.Lock();
	for (i = 0; i < maiSnapshots.NumElements(); i++)
	{
		if (*maiSnapshots.Get(i) == iSnapshot)
		{
			maiSnapshots.RemoveAt(i);
			break;
		}
	}
	RemoveOldVersions();
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::GetStoredIndices(CArrayOIndex* paoi)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	size					iDataSize;

	//mcMutex must be locked.
	oi = mcNamedIndexedData.StartIndexIteration(&sIter, NULL, &iDataSize, 0);
	while (oi != INVALID_O_INDEX)
	{
		paoi->Add(oi);
		oi = mcNamedIndexedData.IndexIterate(&sIter, NULL, &iDataSize, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::GetStoredNames(CArrayChars* paszNames)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	char					szKey[MAX_KEY_SIZE + 1];
	bool					bExists;

	//mcMutex must be locked.
	memset(szKey, 0, MAX_KEY_SIZE + 1);
	bExists = mcNamedIndexedData.StartNameIteration(&sIter, szKey, &oi);
	while (bExists)
	{
		paszNames->Add(szKey);
		memset(szKey, 0, MAX_KEY_SIZE + 1);
		bExists = mcNamedIndexedData.NameIterate(&sIter, szKey, &oi);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::ValidateConfigInitialised(void)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.ValidateConfigInitialised();
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::ValidateConfigKilled(void)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.ValidateConfigKilled();
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::ValidateIdentifiers(void)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.ValidateIdentifiers();
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Add(OIndex oi, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Add(oi, pvData, uiDataSize);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Add(OIndex oi, const char* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Add(oi, szName, pvData, uiDataSize);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Add(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Add(oi, szName, pvData, uiDataSize);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(OIndex oi, void* pvData)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Set(oi, pvData);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(OIndex oi, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Set(oi, pvData, uiDataSize);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(const char* szName, void* pvData)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Set(szName, pvData);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(const char* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Set(szName, pvData, uiDataSize);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(CChars* szName, void* pvData)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Set(szName, pvData);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(CChars* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Set(szName, pvData, uiDataSize);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Put(OIndex oi, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Put(oi, pvData, uiDataSize);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Put(OIndex oi, const char* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Put(oi, szName, pvData, uiDataSize);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Put(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Put(oi, szName, pvData, uiDataSize);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(OIndex oi, void* pvData)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Get(oi, pvData);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.Get(oi, pvData);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(OIndex oi, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Get(oi, puiDataSize, pvData, uiMaxDataSize);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.Get(oi, puiDataSize, pvData, uiMaxDataSize);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(const char* szName, void* pvData)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Get(szName, pvData);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.Get(szName, pvData);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(const char* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Get(szName, puiDataSize, pvData, uiMaxDataSize);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.Get(szName, puiDataSize, pvData, uiMaxDataSize);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(CChars* szName, void* pvData)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Get(szName, pvData);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.Get(szName, pvData);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(CChars* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Get(szName, puiDataSize, pvData, uiMaxDataSize);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.Get(szName, puiDataSize, pvData, uiMaxDataSize);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::GetName(OIndex oi, CChars* szName)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->GetName(oi, szName);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.GetName(oi, szName);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::GetName(OIndex oi, char* szName, uint32* puiNameLength, uint32 uiMaxNameLength)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->GetName(oi, szName, puiNameLength, uiMaxNameLength);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.GetName(oi, szName, puiNameLength, uiMaxNameLength);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::GetIndex(const char* szName)
{
	OIndex	oiResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		oiResult = mpcSession->GetIndex(szName);
	}
	else
	{
		mcMutex.Lock();
		oiResult = mcNamedIndexedData.GetIndex(szName);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return oiResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::GetIndex(CChars* szName)
{
	OIndex	oiResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		oiResult = mpcSession->GetIndex(szName);
	}
	else
	{
		mcMutex.Lock();
		oiResult = mcNamedIndexedData.GetIndex(szName);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return oiResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Contains(OIndex oi)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Contains(oi);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.Contains(oi);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Contains(const char* szName)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Contains(szName);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.Contains(szName);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Contains(CChars* szName)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Contains(szName);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.Contains(szName);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Remove(OIndex oi)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Remove(oi);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Remove(const char* szName)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Remove(szName);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Remove(CChars* szName)
{
	bool	bResult;

	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->Remove(szName);
	}
	else
	{
		bResult = NeedsSession(__METHOD__);
	}
	mcSessionMutex.Unlock();
	return bResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Flush(void)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Flush(false);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Flush(bool bClearCache)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Flush(bClearCache);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::IsWriteThrough(void)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.IsWriteThrough();
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumIndices(void)
{
	int64	iResult;
	int64	iIndicesChange;
	int64	iNamesChange;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumIndices();
	mcMutex.Unlock();

	//A session counts its own changes.
	if (mpcSession != NULL)
	{
		mpcSession->GetCountChanges(&iIndicesChange, &iNamesChange);
		iResult += iIndicesChange;
	}
	mcSessionMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumIndicesCached(void)
{
	int64	iResult;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumIndicesCached();
	mcMutex.Unlock();

	//A session's new objects are counted as cached, though they are in the session until Close.
	if (mpcSession != NULL)
	{
		iResult += mpcSession->NumNewRecords(true, 0);
	}
	mcSessionMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumIndicesCached(size iSize)
{
	int64	iResult;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumIndicesCached(iSize);
	mcMutex.Unlock();

	//A session's new objects are counted as cached, though they are in the session until Close.
	if (mpcSession != NULL)
	{
		iResult += mpcSession->NumNewRecords(false, iSize);
	}
	mcSessionMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumDataCached(void)
{
	int64	iResult;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumDataCached();
	mcMutex.Unlock();

	//A session's new objects are counted as cached, though they are in the session until Close.
	if (mpcSession != NULL)
	{
		iResult += mpcSession->NumNewRecords(true, 0);
	}
	mcSessionMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumDataCached(size iSize)
{
	int64	iResult;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumDataCached(iSize);
	mcMutex.Unlock();

	//A session's new objects are counted as cached, though they are in the session until Close.
	if (mpcSession != NULL)
	{
		iResult += mpcSession->NumNewRecords(false, iSize);
	}
	mcSessionMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumNames(void)
{
	int64	iResult;
	int64	iIndicesChange;
	int64	iNamesChange;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumNames();
	mcMutex.Unlock();

	//A session counts its own changes.
	if (mpcSession != NULL)
	{
		mpcSession->GetCountChanges(&iIndicesChange, &iNamesChange);
		iResult += iNamesChange;
	}
	mcSessionMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumNamesCached(void)
{
	int64	iResult;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumNamesCached();
	mcMutex.Unlock();

	//A session's new objects are counted as cached, though they are in the session until Close.
	if (mpcSession != NULL)
	{
		iResult += mpcSession->NumNewNames();
	}
	mcSessionMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::Dump(void)
{
	mcMutex.Lock();
	mcNamedIndexedData.Dump();
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize)
{
	OIndex	oiResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		oiResult = mpcSession->StartIndexIteration(psIterator, pvData, piDataSize, iMaxDataSize);
	}
	else
	{
		mcMutex.Lock();
		oiResult = mcNamedIndexedData.StartIndexIteration(psIterator, pvData, piDataSize, iMaxDataSize);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return oiResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize)
{
	OIndex	oiResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		oiResult = mpcSession->IndexIterate(psIterator, pvData, piDataSize, iMaxDataSize);
	}
	else
	{
		mcMutex.Lock();
		oiResult = mcNamedIndexedData.IndexIterate(psIterator, pvData, piDataSize, iMaxDataSize);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return oiResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->StartNameIteration(psIterator, szKey, poi);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.StartNameIteration(psIterator, szKey, poi);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	bool	bResult;

	//A session sees its own changes.  Without one the committed store is read.
	mcSessionMutex.Lock();
	if (mpcSession != NULL)
	{
		bResult = mpcSession->NameIterate(psIterator, szKey, poi);
	}
	else
	{
		mcMutex.Lock();
		bResult = mcNamedIndexedData.NameIterate(psIterator, szKey, poi);
		mcMutex.Unlock();
	}
	mcSessionMutex.Unlock();
	return bResult;
}

