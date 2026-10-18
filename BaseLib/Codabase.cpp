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
	mcWriterFinished.Init();
	mbWriterActive = false;
	miWriterThread = 0;
	mbSessionOpen = false;
	mbFailed = false;
	mcVersions.Init();
	miCommitNumber = 0;
	maiSnapshots.Init();

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
	mcWriterFinished.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Open(void)
{
	//A direct session is a writer.  It waits for any other writer to finish.
	if (!AcquireWriter())
	{
		return false;
	}

	mcMutex.Lock();
	mbSessionOpen = true;
	mcMutex.Unlock();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Close(void)
{
	bool	bResult;

	mcMutex.Lock();
	if (!mbSessionOpen)
	{
		mcMutex.Unlock();
		return gcLogger.Error2(__METHOD__, " Cannot close a Codabase that is not open.", NULL);
	}

	bResult = CommitLocked();
	mbSessionOpen = false;
	ReleaseWriter();
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CWriteTransaction* CCodabase::BeginWriteTransaction(void)
{
	CWriteTransaction*	pcTransaction;

	if (!mbStarted)
	{
		gcLogger.Error2(__METHOD__, " Cannot begin a write transaction on a Codabase that is not started.", NULL);
		return NULL;
	}

	if (!AcquireWriter())
	{
		return NULL;
	}

	pcTransaction = NewMalloc<CWriteTransaction>();
	pcTransaction->Init(this);
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
bool CCodabase::AcquireWriter(void)
{
	mcMutex.Lock();
	if (mbWriterActive && (miWriterThread == CurrentThreadId()))
	{
		mcMutex.Unlock();
		return gcLogger.Error2(__METHOD__, " This thread is already writing to the Codabase.  Waiting for itself would never end.", NULL);
	}

	while (mbWriterActive)
	{
		mcWriterFinished.Wait(&mcMutex);
	}

	if (mbFailed)
	{
		mcMutex.Unlock();
		return gcLogger.Error2(__METHOD__, " A commit failed.  The Codabase is read only until it is restarted.", NULL);
	}

	mbWriterActive = true;
	miWriterThread = CurrentThreadId();
	mcMutex.Unlock();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::ReleaseWriter(void)
{
	//mcMutex must be locked.
	mbWriterActive = false;
	miWriterThread = 0;
	mcWriterFinished.WakeAll();
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
void CCodabase::SaveVersions(OIndex oi, const char* szNewName)
{
	//mcMutex must be locked.  A direct session changes the store before it commits, so the versions before the change
	//are kept even when no read transaction is open.  One could start before the session closes.
	if (IsValidIndex(oi))
	{
		mcVersions.SaveRecord(miCommitNumber + 1, oi, &mcNamedIndexedData);
	}
	if (!StrEmpty(szNewName))
	{
		mcVersions.SaveName(miCommitNumber + 1, szNewName, &mcNamedIndexedData);
	}
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
		paoi->Add(&oi);
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
bool CCodabase::CanWriteDirectly(void)
{
	//mcMutex must be locked.
	if (mbFailed)
	{
		return gcLogger.Error2(__METHOD__, " A commit failed.  The Codabase is read only until it is restarted.", NULL);
	}
	if (!mbSessionOpen)
	{
		return gcLogger.Error2(__METHOD__, " Writing directly to a Codabase needs Open first.  Use a write transaction otherwise.", NULL);
	}
	return true;
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

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(oi, NULL);
	bResult = mcNamedIndexedData.Add(oi, pvData, uiDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Add(OIndex oi, const char* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(oi, szName);
	bResult = mcNamedIndexedData.Add(oi, szName, pvData, uiDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Add(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(oi, szName ? szName->Text() : NULL);
	bResult = mcNamedIndexedData.Add(oi, szName, pvData, uiDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(OIndex oi, void* pvData)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(oi, NULL);
	bResult = mcNamedIndexedData.Set(oi, pvData);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(OIndex oi, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(oi, NULL);
	bResult = mcNamedIndexedData.Set(oi, pvData, uiDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(const char* szName, void* pvData)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(mcNamedIndexedData.GetIndex(szName), NULL);
	bResult = mcNamedIndexedData.Set(szName, pvData);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(const char* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(mcNamedIndexedData.GetIndex(szName), NULL);
	bResult = mcNamedIndexedData.Set(szName, pvData, uiDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(CChars* szName, void* pvData)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(mcNamedIndexedData.GetIndex(szName), NULL);
	bResult = mcNamedIndexedData.Set(szName, pvData);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Set(CChars* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(mcNamedIndexedData.GetIndex(szName), NULL);
	bResult = mcNamedIndexedData.Set(szName, pvData, uiDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Put(OIndex oi, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(oi, NULL);
	bResult = mcNamedIndexedData.Put(oi, pvData, uiDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Put(OIndex oi, const char* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(oi, szName);
	bResult = mcNamedIndexedData.Put(oi, szName, pvData, uiDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Put(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(oi, szName ? szName->Text() : NULL);
	bResult = mcNamedIndexedData.Put(oi, szName, pvData, uiDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(OIndex oi, void* pvData)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Get(oi, pvData);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(OIndex oi, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Get(oi, puiDataSize, pvData, uiMaxDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(const char* szName, void* pvData)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Get(szName, pvData);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(const char* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Get(szName, puiDataSize, pvData, uiMaxDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(CChars* szName, void* pvData)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Get(szName, pvData);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Get(CChars* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Get(szName, puiDataSize, pvData, uiMaxDataSize);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::GetName(OIndex oi, CChars* szName)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.GetName(oi, szName);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::GetName(OIndex oi, char* szName, uint32* puiNameLength, uint32 uiMaxNameLength)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.GetName(oi, szName, puiNameLength, uiMaxNameLength);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::GetIndex(const char* szName)
{
	OIndex	oiResult;

	mcMutex.Lock();
	oiResult = mcNamedIndexedData.GetIndex(szName);
	mcMutex.Unlock();
	return oiResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::GetIndex(CChars* szName)
{
	OIndex	oiResult;

	mcMutex.Lock();
	oiResult = mcNamedIndexedData.GetIndex(szName);
	mcMutex.Unlock();
	return oiResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Contains(OIndex oi)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Contains(oi);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Contains(const char* szName)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Contains(szName);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Contains(CChars* szName)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.Contains(szName);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Remove(OIndex oi)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(oi, NULL);
	bResult = mcNamedIndexedData.Remove(oi);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Remove(const char* szName)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(mcNamedIndexedData.GetIndex(szName), NULL);
	bResult = mcNamedIndexedData.Remove(szName);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::Remove(CChars* szName)
{
	bool	bResult;

	mcMutex.Lock();
	if (!CanWriteDirectly())
	{
		mcMutex.Unlock();
		return false;
	}
	SaveVersions(mcNamedIndexedData.GetIndex(szName), NULL);
	bResult = mcNamedIndexedData.Remove(szName);
	mcMutex.Unlock();
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

	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumIndices();
	mcMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumIndicesCached(void)
{
	int64	iResult;

	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumIndicesCached();
	mcMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumIndicesCached(size iSize)
{
	int64	iResult;

	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumIndicesCached(iSize);
	mcMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumDataCached(void)
{
	int64	iResult;

	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumDataCached();
	mcMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumDataCached(size iSize)
{
	int64	iResult;

	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumDataCached(iSize);
	mcMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumNames(void)
{
	int64	iResult;

	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumNames();
	mcMutex.Unlock();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumNamesCached(void)
{
	int64	iResult;

	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumNamesCached();
	mcMutex.Unlock();
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

	mcMutex.Lock();
	oiResult = mcNamedIndexedData.StartIndexIteration(psIterator, pvData, piDataSize, iMaxDataSize);
	mcMutex.Unlock();
	return oiResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize)
{
	OIndex	oiResult;

	mcMutex.Lock();
	oiResult = mcNamedIndexedData.IndexIterate(psIterator, pvData, piDataSize, iMaxDataSize);
	mcMutex.Unlock();
	return oiResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.StartNameIteration(psIterator, szKey, poi);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = mcNamedIndexedData.NameIterate(psIterator, szKey, poi);
	mcMutex.Unlock();
	return bResult;
}

