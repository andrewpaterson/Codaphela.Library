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
	maiSnapshots.Add(iSnapshot);
	mcMutex.Unlock();

	pcTransaction = NewMalloc<CReadTransaction>();
	pcTransaction->Init(this, iSnapshot);
	return pcTransaction;
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
bool CCodabase::ReserveName(const char* szName, int64 iTransaction, bool bRemovedByTransaction)
{
	int64*	piOwner;

	//The check of the committed names and the reservation are made together, so no other transaction can add the name
	//in between.  The first transaction to add a name owns it until it commits or aborts.  A name the transaction
	//removed itself can be added again.
	mcMutex.Lock();
	if (!bRemovedByTransaction && (mcNamedIndexedData.GetIndex(szName) != INVALID_O_INDEX))
	{
		mcMutex.Unlock();
		return gcLogger.Error2(__METHOD__, " Cannot Add Name [", szName, "].  It already exists.", NULL);
	}
	piOwner = mcReservedNames.Get(szName);
	if ((piOwner != NULL) && (*piOwner != iTransaction))
	{
		mcMutex.Unlock();
		return gcLogger.Error2(__METHOD__, " Cannot Add Name [", szName, "].  Another transaction is adding it.", NULL);
	}
	if (piOwner == NULL)
	{
		mcReservedNames.Put(szName, &iTransaction);
	}
	mcMutex.Unlock();
	return true;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::ReleaseNames(CArrayChars* paszNames, int64 iTransaction)
{
	size	i;
	int64*	piOwner;
	char*	szName;

	mcMutex.Lock();
	for (i = 0; i < paszNames->NumElements(); i++)
	{
		szName = paszNames->GetText(i);
		piOwner = mcReservedNames.Get(szName);
		if ((piOwner != NULL) && (*piOwner == iTransaction))
		{
			mcReservedNames.Remove(szName);
		}
	}
	mcMutex.Unlock();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::StoredGet(OIndex oi, size* puiDataSize, void* pvData, size uiMaxDataSize)
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
bool CCodabase::StoredGet(OIndex oi, void* pvData)
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
bool CCodabase::SnapshotGet(int64 iSnapshot, OIndex oi, size* puiDataSize, void* pvData, size uiMaxDataSize, bool bAllData)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = SnapshotGetLocked(iSnapshot, oi, puiDataSize, pvData, uiMaxDataSize, bAllData);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::SnapshotContains(int64 iSnapshot, OIndex oi)
{
	bool	bResult;

	mcMutex.Lock();
	bResult = SnapshotContainsLocked(iSnapshot, oi);
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::SnapshotGetName(int64 iSnapshot, OIndex oi, CChars* pszName)
{
	CVersion*	pcVersion;
	bool		bResult;

	mcMutex.Lock();
	pcVersion = mcVersions.FindRecord(oi, iSnapshot);
	if (pcVersion == NULL)
	{
		bResult = mcNamedIndexedData.GetName(oi, pszName);
	}
	else if (pcVersion->bExists)
	{
		pszName->Append(pcVersion->GetName(), pcVersion->iNameLength);
		bResult = true;
	}
	else
	{
		bResult = false;
	}
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::SnapshotGetIndex(int64 iSnapshot, const char* szName)
{
	OIndex	oi;

	mcMutex.Lock();
	oi = SnapshotGetIndexLocked(iSnapshot, szName);
	mcMutex.Unlock();
	return oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::SnapshotIndices(int64 iSnapshot, CArrayOIndex* paoi)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	size					iDataSize;
	CArrayOIndex			aoiVersioned;
	size					i;

	//The indices that existed at the snapshot: those in the store that have not been added since, and those that have
	//been removed since.
	mcMutex.Lock();
	oi = mcNamedIndexedData.StartIndexIteration(&sIter, NULL, &iDataSize, 0);
	while (oi != INVALID_O_INDEX)
	{
		if (SnapshotContainsLocked(iSnapshot, oi))
		{
			paoi->Add(oi);
		}
		oi = mcNamedIndexedData.IndexIterate(&sIter, NULL, &iDataSize, 0);
	}

	aoiVersioned.Init();
	mcVersions.GetIndices(&aoiVersioned);
	for (i = 0; i < aoiVersioned.NumElements(); i++)
	{
		oi = aoiVersioned.GetValue(i);
		if (!mcNamedIndexedData.Contains(oi) && SnapshotContainsLocked(iSnapshot, oi))
		{
			paoi->Add(oi);
		}
	}
	aoiVersioned.Kill();
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::SnapshotNames(int64 iSnapshot, CArrayChars* paszNames)
{
	CArrayChars	aszNames;
	size		i;
	char*		szName;

	//The stored names and the names changed since the snapshot, if the name existed at the snapshot.
	aszNames.Init();
	StoredNames(&aszNames);
	mcMutex.Lock();
	mcVersions.GetNames(&aszNames);
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		szName = aszNames.GetText(i);
		if (SnapshotGetIndexLocked(iSnapshot, szName) != INVALID_O_INDEX)
		{
			paszNames->Add(szName);
		}
	}
	mcMutex.Unlock();
	aszNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::SnapshotGetLocked(int64 iSnapshot, OIndex oi, size* puiDataSize, void* pvData, size uiMaxDataSize, bool bAllData)
{
	CVersion*	pcVersion;
	size		uiCopySize;

	//mcMutex must be locked.  A version means the object changed after the snapshot.  Without one the store still has
	//it as it was.
	pcVersion = mcVersions.FindRecord(oi, iSnapshot);
	if (pcVersion == NULL)
	{
		if (bAllData)
		{
			return mcNamedIndexedData.Get(oi, pvData);
		}
		return mcNamedIndexedData.Get(oi, puiDataSize, pvData, uiMaxDataSize);
	}

	if (!pcVersion->bExists)
	{
		return false;
	}
	SafeAssign(puiDataSize, pcVersion->uiDataSize);
	if (pvData)
	{
		uiCopySize = pcVersion->uiDataSize;
		if (!bAllData && (uiMaxDataSize < uiCopySize))
		{
			uiCopySize = uiMaxDataSize;
		}
		memcpy(pvData, pcVersion->GetData(), uiCopySize);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::SnapshotContainsLocked(int64 iSnapshot, OIndex oi)
{
	CVersion*	pcVersion;

	//mcMutex must be locked.
	pcVersion = mcVersions.FindRecord(oi, iSnapshot);
	if (pcVersion == NULL)
	{
		return mcNamedIndexedData.Contains(oi);
	}
	return pcVersion->bExists;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::SnapshotGetIndexLocked(int64 iSnapshot, const char* szName)
{
	OIndex	oi;

	//mcMutex must be locked.
	if (StrEmpty(szName))
	{
		return INVALID_O_INDEX;
	}
	if (mcVersions.FindName(szName, iSnapshot, &oi))
	{
		return oi;
	}
	return mcNamedIndexedData.GetIndex(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ELockResult CCodabase::LockObject(int64 iTransaction, OIndex oi, ELockMode eMode)
{
	return mcLocks.Lock(iTransaction, oi, eMode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::ReleaseLocks(int64 iTransaction, CArrayOIndex* paoiLocked)
{
	mcLocks.Release(iTransaction, paoiLocked);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::CommitWriteSet(CMapBlock* pcRecords)
{
	bool	bResult;

	mcMutex.Lock();
	if (mbFailed)
	{
		bResult = gcLogger.Error2(__METHOD__, " A commit failed.  The Codabase is read only until it is restarted.", NULL);
	}
	else
	{
		//Removals go first so a name removed from one object can be given to another.
		bResult = ApplyLocked(pcRecords, true) && ApplyLocked(pcRecords, false);
		if (bResult)
		{
			bResult = CommitLocked();
		}
		else
		{
			//Some of the changes may already be in the caches, so nothing more may be written.
			mbFailed = true;
			gcLogger.Error2(__METHOD__, " Could not apply the transaction.  The Codabase is read only until it is restarted.", NULL);
		}
	}
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabase::ApplyLocked(CMapBlock* pcRecords, bool bRemovals)
{
	SMapIterator		sIter;
	void*				pvKey;
	void*				pvData;
	size				iKeySize;
	size				iDataSize;
	bool				bExists;
	OIndex				oi;
	CWriteRecord*		pcRecord;
	bool				bResult;
	bool				bSaveVersions;
	int64				iValidUntil;

	//mcMutex must be locked.  Open read transactions need the versions before this commit.  None can begin while the
	//mutex is held, so without any open the versions are not kept.
	bSaveVersions = maiSnapshots.NumElements() > 0;
	iValidUntil = miCommitNumber + 1;
	bExists = pcRecords->StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		oi = *((OIndex*)pvKey);
		pcRecord = (CWriteRecord*)pvData;
		bResult = true;
		if (bRemovals)
		{
			if ((pcRecord->bRemoved || pcRecord->bReplacesStored) && mcNamedIndexedData.Contains(oi))
			{
				if (bSaveVersions)
				{
					mcVersions.SaveRecord(iValidUntil, oi, &mcNamedIndexedData);
				}
				bResult = mcNamedIndexedData.Remove(oi);
			}
		}
		else if (!pcRecord->bRemoved)
		{
			if (bSaveVersions)
			{
				mcVersions.SaveRecord(iValidUntil, oi, &mcNamedIndexedData);
				if (pcRecord->iNameLength > 0)
				{
					mcVersions.SaveName(iValidUntil, pcRecord->GetName(), &mcNamedIndexedData);
				}
			}
			if (mcNamedIndexedData.Contains(oi))
			{
				bResult = mcNamedIndexedData.Set(oi, pcRecord->GetData(), pcRecord->uiDataSize);
			}
			else if (pcRecord->iNameLength > 0)
			{
				bResult = mcNamedIndexedData.Add(oi, pcRecord->GetName(), pcRecord->GetData(), pcRecord->uiDataSize);
			}
			else
			{
				bResult = mcNamedIndexedData.Add(oi, pcRecord->GetData(), pcRecord->uiDataSize);
			}
		}

		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " Could not apply the change to Index [", IndexToString(oi), "].", NULL);
		}
		bExists = pcRecords->Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	return true;
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
		iSnapshot = maiSnapshots.GetValue(i);
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
	size	iIndex;

	mcMutex.Lock();
	iIndex = maiSnapshots.Find(iSnapshot);
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		maiSnapshots.RemoveAt(iIndex);
	}
	RemoveOldVersions();
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::StoredIndices(CArrayOIndex* paoi)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	size					iDataSize;

	mcMutex.Lock();
	oi = mcNamedIndexedData.StartIndexIteration(&sIter, NULL, &iDataSize, 0);
	while (oi != INVALID_O_INDEX)
	{
		paoi->Add(oi);
		oi = mcNamedIndexedData.IndexIterate(&sIter, NULL, &iDataSize, 0);
	}
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::StoredNames(CArrayChars* paszNames)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	char					szKey[MAX_KEY_SIZE + 1];
	bool					bExists;

	mcMutex.Lock();
	memset(szKey, 0, MAX_KEY_SIZE + 1);
	bExists = mcNamedIndexedData.StartNameIteration(&sIter, szKey, &oi);
	while (bExists)
	{
		paszNames->Add(szKey);
		memset(szKey, 0, MAX_KEY_SIZE + 1);
		bExists = mcNamedIndexedData.NameIterate(&sIter, szKey, &oi);
	}
	mcMutex.Unlock();
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
bool CCodabase::Add(OIndex oi, void* pvData, size uiDataSize)
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
bool CCodabase::Add(OIndex oi, const char* szName, void* pvData, size uiDataSize)
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
bool CCodabase::Add(OIndex oi, CChars* szName, void* pvData, size uiDataSize)
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
bool CCodabase::Set(OIndex oi, void* pvData, size uiDataSize)
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
bool CCodabase::Set(const char* szName, void* pvData, size uiDataSize)
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
bool CCodabase::Set(CChars* szName, void* pvData, size uiDataSize)
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
bool CCodabase::Put(OIndex oi, void* pvData, size uiDataSize)
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
bool CCodabase::Put(OIndex oi, const char* szName, void* pvData, size uiDataSize)
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
bool CCodabase::Put(OIndex oi, CChars* szName, void* pvData, size uiDataSize)
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
bool CCodabase::Get(OIndex oi, size* puiDataSize, void* pvData, size uiMaxDataSize)
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
bool CCodabase::Get(const char* szName, size* puiDataSize, void* pvData, size uiMaxDataSize)
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
bool CCodabase::Get(CChars* szName, size* puiDataSize, void* pvData, size uiMaxDataSize)
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
bool CCodabase::GetName(OIndex oi, char* szName, size* puiNameLength, size uiMaxNameLength)
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
	int64			iResult;
	SWriteCounts	sCounts;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumIndices();
	mcMutex.Unlock();

	//A session counts its own changes.
	if (mpcSession != NULL)
	{
		mpcSession->CountChanges(&sCounts, 0);
		iResult += sCounts.iIndices;
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
	int64			iResult;
	SWriteCounts	sCounts;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumIndicesCached();
	mcMutex.Unlock();

	//A session's new objects are counted as cached, though they are in the session until Close.
	if (mpcSession != NULL)
	{
		mpcSession->CountChanges(&sCounts, 0);
		iResult += sCounts.iNewIndices;
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
	int64			iResult;
	SWriteCounts	sCounts;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumIndicesCached(iSize);
	mcMutex.Unlock();

	//A session's new objects are counted as cached, though they are in the session until Close.
	if (mpcSession != NULL)
	{
		mpcSession->CountChanges(&sCounts, iSize);
		iResult += sCounts.iNewIndicesOfSize;
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
	int64			iResult;
	SWriteCounts	sCounts;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumDataCached();
	mcMutex.Unlock();

	//A session's new objects are counted as cached, though they are in the session until Close.
	if (mpcSession != NULL)
	{
		mpcSession->CountChanges(&sCounts, 0);
		iResult += sCounts.iNewIndices;
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
	int64			iResult;
	SWriteCounts	sCounts;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumDataCached(iSize);
	mcMutex.Unlock();

	//A session's new objects are counted as cached, though they are in the session until Close.
	if (mpcSession != NULL)
	{
		mpcSession->CountChanges(&sCounts, iSize);
		iResult += sCounts.iNewIndicesOfSize;
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
	int64			iResult;
	SWriteCounts	sCounts;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumNames();
	mcMutex.Unlock();

	//A session counts its own changes.
	if (mpcSession != NULL)
	{
		mpcSession->CountChanges(&sCounts, 0);
		iResult += sCounts.iNames;
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
	int64			iResult;
	SWriteCounts	sCounts;

	mcSessionMutex.Lock();
	mcMutex.Lock();
	iResult = mcNamedIndexedData.NumNamesCached();
	mcMutex.Unlock();

	//A session's new objects are counted as cached, though they are in the session until Close.
	if (mpcSession != NULL)
	{
		mpcSession->CountChanges(&sCounts, 0);
		iResult += sCounts.iNewNames;
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

