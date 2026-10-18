#include "Logger.h"
#include "StringHelper.h"
#include "PointerRemapper.h"
#include "IndexedGeneral.h"
#include "IntegerHelper.h"
#include "NativeThread.h"
#include "Codabase.h"
#include "WriteTransaction.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* SWriteRecord::GetName(void)
{
	return (char*)RemapSinglePointer(this, sizeof(SWriteRecord));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* SWriteRecord::GetData(void)
{
	return RemapSinglePointer(this, sizeof(SWriteRecord) + iNameLength + 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteTransaction::Init(CCodabase* pcCodabase, int64 iStartNumber)
{
	mpcCodabase = pcCodabase;
	miStartNumber = iStartNumber;
	miThread = CurrentThreadId();
	mcRecords.Init(true);
	mcNames.Init(true, true);
	maszReserved.Init();
	mbActive = true;
	mbDied = false;
	maoiIteration.Init();
	miIndexPosition = 0;
	maszIteration.Init();
	miNamePosition = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteTransaction::Kill(void)
{
	//Killing a transaction that has not committed aborts it.
	Abort();
	mpcCodabase->RemoveWriter(this);
	maszIteration.Kill();
	maoiIteration.Kill();
	maszReserved.Kill();
	mcNames.Kill();
	mcRecords.Kill();
	mpcCodabase = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Commit(void)
{
	bool	bResult;
	size	i;

	if (!CheckActive())
	{
		return false;
	}

	mpcCodabase->mcMutex.Lock();
	if (mpcCodabase->mbFailed)
	{
		bResult = gcLogger.Error2(__METHOD__, " A commit failed.  The Codabase is read only until it is restarted.", NULL);
	}
	else
	{
		//Removals go first so a name removed from one object can be given to another.
		bResult = Apply(true) && Apply(false);
		if (bResult)
		{
			bResult = mpcCodabase->CommitLocked();
		}
		else
		{
			//Some of the changes may already be in the caches, so nothing more may be written.
			mpcCodabase->mbFailed = true;
			gcLogger.Error2(__METHOD__, " Could not apply the transaction.  The Codabase is read only until it is restarted.", NULL);
		}
	}
	for (i = 0; i < maszReserved.NumElements(); i++)
	{
		mpcCodabase->ReleaseName(maszReserved.GetText(i), miStartNumber);
	}
	mpcCodabase->mcMutex.Unlock();
	maszReserved.ReInit();

	//The locks are released only once the commit is durable.
	mpcCodabase->mcLocks.ReleaseAll(miStartNumber);
	Finish();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteTransaction::Abort(void)
{
	if (mbActive)
	{
		ReleaseResources();
		Finish();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::IsActive(void)
{
	return mbActive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::HasDied(void)
{
	return mbDied;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Restart(void)
{
	if (!mbDied)
	{
		return gcLogger.Error2(__METHOD__, " Only a write transaction that died can be restarted.", NULL);
	}

	//The start number is kept, so the transaction is older than every one begun since it first started.
	mbDied = false;
	mbActive = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CWriteTransaction::GetStartNumber(void)
{
	return miStartNumber;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWriteTransaction::GetThread(void)
{
	return miThread;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Lock(OIndex oi, ELockMode eMode)
{
	if (!CheckActive())
	{
		return false;
	}
	return LockOrDie(oi, eMode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Add(OIndex oi, void* pvData, uint32 uiDataSize)
{
	return Add(oi, (const char*)NULL, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Add(OIndex oi, const char* szName, void* pvData, uint32 uiDataSize)
{
	if (!CheckActive())
	{
		return false;
	}
	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Add Index [", IndexToString(oi), "].", NULL);
	}
	if ((pvData == NULL) || (uiDataSize == 0))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Add Index [", IndexToString(oi), "] with empty data.", NULL);
	}
	if (!LockOrDie(oi, LM_Exclusive))
	{
		return false;
	}
	return AddLocked(oi, szName, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Add(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize)
{
	return Add(oi, szName ? szName->Text() : NULL, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Set(OIndex oi, void* pvData)
{
	return Set(oi, pvData, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Set(OIndex oi, void* pvData, uint32 uiDataSize)
{
	if (!CheckActive())
	{
		return false;
	}
	if (!LockOrDie(oi, LM_Exclusive))
	{
		return false;
	}
	return SetLocked(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Set(const char* szName, void* pvData)
{
	return Set(szName, pvData, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Set(const char* szName, void* pvData, uint32 uiDataSize)
{
	OIndex	oi;

	if (!CheckActive())
	{
		return false;
	}
	if (!FindAndLock(szName, LM_Exclusive, &oi))
	{
		if (mbDied)
		{
			return false;
		}
		return gcLogger.Error2(__METHOD__, " Cannot Set Name [", StringToString(szName), "].  It does not exist.", NULL);
	}
	return SetLocked(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Set(CChars* szName, void* pvData)
{
	return Set(szName ? szName->Text() : NULL, pvData, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Set(CChars* szName, void* pvData, uint32 uiDataSize)
{
	return Set(szName ? szName->Text() : NULL, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Put(OIndex oi, void* pvData, uint32 uiDataSize)
{
	if (!CheckActive())
	{
		return false;
	}
	if (!LockOrDie(oi, LM_Exclusive))
	{
		return false;
	}

	if (Exists(oi))
	{
		return SetLocked(oi, pvData, uiDataSize);
	}
	else
	{
		return AddLocked(oi, NULL, pvData, uiDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Put(OIndex oi, const char* szName, void* pvData, uint32 uiDataSize)
{
	OIndex	oiExisting;
	bool	bExists;

	if (!CheckActive())
	{
		return false;
	}
	if (!LockOrDie(oi, LM_Exclusive))
	{
		return false;
	}

	//The same rules as CNamedIndexedData::Put.
	oiExisting = FindName(szName);
	bExists = Exists(oi);
	if (bExists && (oiExisting != INVALID_O_INDEX))
	{
		if (oiExisting != oi)
		{
			return gcLogger.Error2(__METHOD__, " Put Index [", IndexToString(oi), "] not equal to existing index [", IndexToString(oiExisting), "] for name [", szName, "].", NULL);
		}
		return SetLocked(oi, pvData, uiDataSize);
	}
	else if (!bExists && (oiExisting == INVALID_O_INDEX))
	{
		if ((pvData == NULL) || (uiDataSize == 0))
		{
			return gcLogger.Error2(__METHOD__, " Cannot Put Index [", IndexToString(oi), "] with empty data.", NULL);
		}
		return AddLocked(oi, szName, pvData, uiDataSize);
	}
	else if (oiExisting != INVALID_O_INDEX)
	{
		return gcLogger.Error2(__METHOD__, " Index [", IndexToString(oi), "] does not have an existing name [", szName, "].", NULL);
	}
	else if (StrEmpty(szName))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Put existing Index [", IndexToString(oi), "] with an empty name.", NULL);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Name [", szName, "] does not have an existing index.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Put(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize)
{
	return Put(oi, szName ? szName->Text() : NULL, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Get(OIndex oi, void* pvData)
{
	SWriteRecord*	psRecord;

	if (!CheckActive() || !LockOrDie(oi, LM_Shared))
	{
		return false;
	}

	psRecord = GetRecord(oi);
	if (psRecord == NULL)
	{
		return mpcCodabase->StoredGetAll(oi, pvData);
	}
	if (psRecord->bRemoved)
	{
		return false;
	}
	memcpy(pvData, psRecord->GetData(), psRecord->uiDataSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Get(OIndex oi, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	SWriteRecord*	psRecord;

	if (!CheckActive() || !LockOrDie(oi, LM_Shared))
	{
		return false;
	}

	psRecord = GetRecord(oi);
	if (psRecord == NULL)
	{
		return mpcCodabase->StoredGet(oi, puiDataSize, pvData, uiMaxDataSize);
	}
	if (psRecord->bRemoved)
	{
		return false;
	}
	SafeAssign(puiDataSize, psRecord->uiDataSize);
	if (pvData)
	{
		memcpy(pvData, psRecord->GetData(), (psRecord->uiDataSize < uiMaxDataSize) ? psRecord->uiDataSize : uiMaxDataSize);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Get(const char* szName, void* pvData)
{
	OIndex	oi;

	if (!CheckActive() || !FindAndLock(szName, LM_Shared, &oi))
	{
		return false;
	}
	return Get(oi, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Get(const char* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	OIndex	oi;

	if (!CheckActive() || !FindAndLock(szName, LM_Shared, &oi))
	{
		return false;
	}
	return Get(oi, puiDataSize, pvData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Get(CChars* szName, void* pvData)
{
	return Get(szName ? szName->Text() : NULL, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Get(CChars* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	return Get(szName ? szName->Text() : NULL, puiDataSize, pvData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::GetName(OIndex oi, CChars* szName)
{
	if (!CheckActive() || !LockOrDie(oi, LM_Shared))
	{
		return false;
	}
	if (!Exists(oi))
	{
		return false;
	}
	return GetCurrentName(oi, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::GetName(OIndex oi, char* szName, uint32* puiNameLength, uint32 uiMaxNameLength)
{
	CChars	sz;
	bool	bResult;

	sz.Init();
	bResult = GetName(oi, &sz);
	if (bResult)
	{
		StrCpySafe(szName, sz.Text(), uiMaxNameLength);
		SafeAssign(puiNameLength, (uint32)sz.Length());
	}
	else
	{
		szName[0] = '\0';
		SafeAssign(puiNameLength, 0);
	}
	sz.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CWriteTransaction::GetIndex(const char* szName)
{
	if (!CheckActive())
	{
		return INVALID_O_INDEX;
	}
	return FindName(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CWriteTransaction::GetIndex(CChars* szName)
{
	return GetIndex(szName ? szName->Text() : NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Contains(OIndex oi)
{
	if (!CheckActive() || !LockOrDie(oi, LM_Shared))
	{
		return false;
	}
	return Exists(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Contains(const char* szName)
{
	return GetIndex(szName) != INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Contains(CChars* szName)
{
	return GetIndex(szName) != INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Remove(OIndex oi)
{
	if (!CheckActive() || !LockOrDie(oi, LM_Exclusive))
	{
		return false;
	}
	return RemoveLocked(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Remove(const char* szName)
{
	OIndex	oi;

	if (!CheckActive() || !FindAndLock(szName, LM_Exclusive, &oi))
	{
		return false;
	}
	return RemoveLocked(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Remove(CChars* szName)
{
	return Remove(szName ? szName->Text() : NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Flush(void)
{
	//Nothing reaches the Codabase before Commit.
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Flush(bool bClearCache)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::IsWriteThrough(void)
{
	return mpcCodabase->IsWriteThrough();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CWriteTransaction::StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize)
{
	SMapIterator	sIter;
	void*			pvKey;
	void*			pvRecord;
	size			iKeySize;
	size			iRecordSize;
	bool			bExists;

	//The stored indices and the ones this transaction changed.  Removed ones are skipped when iterating.
	maoiIteration.ReInit();
	mpcCodabase->mcMutex.Lock();
	mpcCodabase->GetStoredIndices(&maoiIteration);
	mpcCodabase->mcMutex.Unlock();
	bExists = mcRecords.StartIteration(&sIter, &pvKey, &iKeySize, &pvRecord, &iRecordSize);
	while (bExists)
	{
		maoiIteration.Add((OIndex*)pvKey);
		bExists = mcRecords.Iterate(&sIter, &pvKey, &iKeySize, &pvRecord, &iRecordSize);
	}

	maoiIteration.QuickSort(CompareLong);
	miIndexPosition = 0;
	return IndexIterate(psIterator, pvData, piDataSize, iMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CWriteTransaction::IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize)
{
	OIndex	oi;
	uint32	uiDataSize;

	while (mbActive && (miIndexPosition < maoiIteration.NumElements()))
	{
		oi = *maoiIteration.Get(miIndexPosition);
		miIndexPosition++;

		//An index can be both stored and changed.  It is only returned once.
		if ((miIndexPosition > 1) && (*maoiIteration.Get(miIndexPosition - 2) == oi))
		{
			continue;
		}

		if (Get(oi, &uiDataSize, pvData, (uint32)iMaxDataSize))
		{
			SafeAssign(piDataSize, (size)uiDataSize);
			return oi;
		}
	}
	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	CArrayChars		aszNames;
	SMapIterator	sIter;
	void*			pvKey;
	void*			pvIndex;
	size			iKeySize;
	size			iIndexSize;
	bool			bExists;
	CChars			szName;
	size			i;

	//The stored names and the ones this transaction added or removed.  Removed ones are skipped when iterating.
	maszIteration.ReInit();
	aszNames.Init();
	mpcCodabase->mcMutex.Lock();
	mpcCodabase->GetStoredNames(&aszNames);
	mpcCodabase->mcMutex.Unlock();
	bExists = mcNames.StartIteration(&sIter, &pvKey, &iKeySize, &pvIndex, &iIndexSize);
	while (bExists)
	{
		szName.Init((char*)pvKey, 0, strnlen((char*)pvKey, iKeySize));
		aszNames.Add(&szName);
		szName.Kill();
		bExists = mcNames.Iterate(&sIter, &pvKey, &iKeySize, &pvIndex, &iIndexSize);
	}

	for (i = 0; i < aszNames.NumElements(); i++)
	{
		if (maszIteration.FindInSorted(aszNames.Get(i)) == ARRAY_ELEMENT_NOT_FOUND)
		{
			maszIteration.InsertIntoSorted(aszNames.Get(i));
		}
	}
	aszNames.Kill();

	miNamePosition = 0;
	return NameIterate(psIterator, szKey, poi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	char*	szName;
	OIndex	oi;

	while (mbActive && (miNamePosition < maszIteration.NumElements()))
	{
		szName = maszIteration.GetText(miNamePosition);
		miNamePosition++;

		oi = FindName(szName);
		if (oi != INVALID_O_INDEX)
		{
			StrCpySafe(szKey, szName, MAX_KEY_SIZE);
			SafeAssign(poi, oi);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteTransaction::GetCountChanges(int64* piIndices, int64* piNames)
{
	SMapIterator	sIter;
	void*			pvKey;
	void*			pvData;
	size			iKeySize;
	size			iDataSize;
	bool			bExists;
	SWriteRecord*	psRecord;
	bool			bStored;
	OIndex			oi;
	CChars			szName;

	//How the number of objects and names will change when the transaction commits.
	*piIndices = 0;
	*piNames = 0;
	bExists = mcRecords.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		psRecord = (SWriteRecord*)pvData;
		bStored = mpcCodabase->StoredContains(*((OIndex*)pvKey));
		if (!psRecord->bRemoved && !bStored)
		{
			(*piIndices)++;
		}
		else if (psRecord->bRemoved && bStored)
		{
			(*piIndices)--;
		}
		bExists = mcRecords.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}

	bExists = mcNames.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		oi = *((OIndex*)pvData);
		szName.Init((char*)pvKey, 0, strnlen((char*)pvKey, iKeySize));
		bStored = mpcCodabase->StoredGetIndex(szName.Text()) != INVALID_O_INDEX;
		szName.Kill();
		if ((oi != INVALID_O_INDEX) && !bStored)
		{
			(*piNames)++;
		}
		else if ((oi == INVALID_O_INDEX) && bStored)
		{
			(*piNames)--;
		}
		bExists = mcNames.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::CheckActive(void)
{
	//__METHOD__ is only used when there is an error.  It is built in a buffer that is not thread safe.
	if (mbActive)
	{
		return true;
	}
	if (mbDied)
	{
		//Not an error.  The caller sees HasDied and restarts.
		return false;
	}
	return gcLogger.Error2(__METHOD__, " The write transaction has already been committed or aborted.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::LockOrDie(OIndex oi, ELockMode eMode)
{
	if (mpcCodabase->mcLocks.Lock(miStartNumber, oi, eMode) == LR_Died)
	{
		Die();
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::FindAndLock(const char* szName, ELockMode eMode, OIndex* poi)
{
	OIndex	oi;

	//The name may move to another object while the lock is waited for, so it is looked up again once locked.
	for (;;)
	{
		oi = FindName(szName);
		*poi = oi;
		if (oi == INVALID_O_INDEX)
		{
			return false;
		}
		if (!LockOrDie(oi, eMode))
		{
			return false;
		}
		if (FindName(szName) == oi)
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteTransaction::Die(void)
{
	//Wait-die: a younger transaction never waits for an older one.  Everything it did is thrown away.
	ReleaseResources();
	mcRecords.Kill();
	mcRecords.Init(true);
	mcNames.Kill();
	mcNames.Init(true, true);
	mbActive = false;
	mbDied = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteTransaction::ReleaseResources(void)
{
	size	i;

	mpcCodabase->mcMutex.Lock();
	for (i = 0; i < maszReserved.NumElements(); i++)
	{
		mpcCodabase->ReleaseName(maszReserved.GetText(i), miStartNumber);
	}
	mpcCodabase->mcMutex.Unlock();
	maszReserved.ReInit();
	mpcCodabase->mcLocks.ReleaseAll(miStartNumber);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::ReserveNewName(const char* szName)
{
	OIndex*	poiChanged;
	bool	bRemovedHere;
	bool	bResult;
	CChars	szReserved;

	//The check of the committed names and the reservation are made together, so no other transaction can add the name
	//in between.  A name this transaction removed can be added again.
	poiChanged = mcNames.Get(szName);
	bRemovedHere = (poiChanged != NULL) && (*poiChanged == INVALID_O_INDEX);
	mpcCodabase->mcMutex.Lock();
	if (!bRemovedHere && (mpcCodabase->mcNamedIndexedData.GetIndex(szName) != INVALID_O_INDEX))
	{
		mpcCodabase->mcMutex.Unlock();
		return gcLogger.Error2(__METHOD__, " Cannot Add Name [", szName, "].  It already exists.", NULL);
	}
	bResult = mpcCodabase->ReserveName(szName, miStartNumber);
	mpcCodabase->mcMutex.Unlock();
	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Add Name [", szName, "].  Another transaction is adding it.", NULL);
	}

	if (maszReserved.FindInSorted(szName) == ARRAY_ELEMENT_NOT_FOUND)
	{
		szReserved.Init(szName);
		maszReserved.InsertIntoSorted(&szReserved);
		szReserved.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SWriteRecord* CWriteTransaction::GetRecord(OIndex oi)
{
	return (SWriteRecord*)mcRecords.Get(&oi, sizeof(OIndex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::PutRecord(OIndex oi, bool bRemoved, bool bReplacesStored, const char* szName, void* pvData, uint32 uiDataSize)
{
	SWriteRecord*	psRecord;
	int32			iNameLength;

	//szName and pvData must not point into the record being replaced.
	iNameLength = szName ? (int32)strlen(szName) : 0;
	mcRecords.Remove(&oi, sizeof(OIndex));
	psRecord = (SWriteRecord*)mcRecords.Put(&oi, sizeof(OIndex), sizeof(SWriteRecord) + iNameLength + 1 + uiDataSize);
	if (psRecord == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Could not store the change to Index [", IndexToString(oi), "].", NULL);
	}

	psRecord->bRemoved = bRemoved;
	psRecord->bReplacesStored = bReplacesStored;
	psRecord->iNameLength = iNameLength;
	psRecord->uiDataSize = uiDataSize;
	memcpy(psRecord->GetName(), szName ? szName : "", iNameLength + 1);
	if (uiDataSize > 0)
	{
		memcpy(psRecord->GetData(), pvData, uiDataSize);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Exists(OIndex oi)
{
	SWriteRecord*	psRecord;

	psRecord = GetRecord(oi);
	if (psRecord != NULL)
	{
		return !psRecord->bRemoved;
	}
	return mpcCodabase->StoredContains(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CWriteTransaction::FindName(const char* szName)
{
	OIndex*	poi;

	if (StrEmpty(szName))
	{
		return INVALID_O_INDEX;
	}

	//Names this transaction added or removed come first.
	poi = mcNames.Get(szName);
	if (poi != NULL)
	{
		return *poi;
	}
	return mpcCodabase->StoredGetIndex(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::GetCurrentName(OIndex oi, CChars* pszName)
{
	SWriteRecord*	psRecord;

	psRecord = GetRecord(oi);
	if (psRecord != NULL)
	{
		if (psRecord->bRemoved)
		{
			return false;
		}
		pszName->Append(psRecord->GetName(), psRecord->iNameLength);
		return true;
	}
	return mpcCodabase->StoredGetName(oi, pszName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::GetDataSize(OIndex oi, uint32* puiDataSize)
{
	SWriteRecord*	psRecord;

	psRecord = GetRecord(oi);
	if (psRecord != NULL)
	{
		*puiDataSize = psRecord->uiDataSize;
		return !psRecord->bRemoved;
	}
	return mpcCodabase->StoredGet(oi, puiDataSize, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::SetData(OIndex oi, void* pvData, uint32 uiDataSize)
{
	CChars			szName;
	SWriteRecord*	psRecord;
	bool			bReplacesStored;
	bool			bResult;

	//The name is copied because the record it may come from is replaced.
	szName.Init();
	GetCurrentName(oi, &szName);
	psRecord = GetRecord(oi);
	bReplacesStored = (psRecord != NULL) ? psRecord->bReplacesStored : false;
	bResult = PutRecord(oi, false, bReplacesStored, szName.Empty() ? NULL : szName.Text(), pvData, uiDataSize);
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::AddLocked(OIndex oi, const char* szName, void* pvData, uint32 uiDataSize)
{
	bool	bReplacesStored;

	//The object is locked exclusively.
	if (Exists(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Add Index [", IndexToString(oi), "].  It already exists.", NULL);
	}
	if (!StrEmpty(szName))
	{
		if (FindName(szName) != INVALID_O_INDEX)
		{
			return gcLogger.Error2(__METHOD__, " Cannot Add Name [", szName, "] with Index [", IndexToString(oi), "].  The name already exists.", NULL);
		}
		if (!ReserveNewName(szName))
		{
			return false;
		}
	}

	//A stored object that does not exist any more was removed by this transaction.
	bReplacesStored = mpcCodabase->StoredContains(oi);
	if (!PutRecord(oi, false, bReplacesStored, szName, pvData, uiDataSize))
	{
		return false;
	}
	if (!StrEmpty(szName))
	{
		mcNames.Put(szName, &oi);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::SetLocked(OIndex oi, void* pvData, uint32 uiDataSize)
{
	//The object is locked exclusively.
	if (!Exists(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Set Index [", IndexToString(oi), "].  It does not exist.", NULL);
	}
	if (pvData == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Set Index [", IndexToString(oi), "] with empty data.", NULL);
	}

	//A size of zero keeps the current size.
	if (uiDataSize == 0)
	{
		if (!GetDataSize(oi, &uiDataSize))
		{
			return false;
		}
	}
	return SetData(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::RemoveLocked(OIndex oi)
{
	CChars	szName;
	OIndex	oiRemoved;

	//The object is locked exclusively.
	if (!Exists(oi))
	{
		return false;
	}

	szName.Init();
	GetCurrentName(oi, &szName);
	if (!szName.Empty())
	{
		oiRemoved = INVALID_O_INDEX;
		mcNames.Put(szName.Text(), &oiRemoved);
	}
	szName.Kill();

	if (mpcCodabase->StoredContains(oi))
	{
		return PutRecord(oi, true, false, NULL, NULL, 0);
	}
	else
	{
		//The object was only ever added by this transaction.
		return mcRecords.Remove(&oi, sizeof(OIndex));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Apply(bool bRemovals)
{
	SMapIterator		sIter;
	void*				pvKey;
	void*				pvData;
	size				iKeySize;
	size				iDataSize;
	bool				bExists;
	OIndex				oi;
	SWriteRecord*		psRecord;
	bool				bResult;
	CNamedIndexedData*	pcStore;
	CCodabaseVersions*	pcVersions;
	int64				iValidUntil;

	//mcMutex of the Codabase is locked, so its store is used directly.  Open read transactions need the versions before
	//this commit.  None can begin while the mutex is held, so without any open the versions are not kept.
	pcStore = &mpcCodabase->mcNamedIndexedData;
	pcVersions = (mpcCodabase->maiSnapshots.NumElements() > 0) ? &mpcCodabase->mcVersions : NULL;
	iValidUntil = mpcCodabase->miCommitNumber + 1;
	bExists = mcRecords.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		oi = *((OIndex*)pvKey);
		psRecord = (SWriteRecord*)pvData;
		bResult = true;
		if (bRemovals)
		{
			if ((psRecord->bRemoved || psRecord->bReplacesStored) && pcStore->Contains(oi))
			{
				if (pcVersions)
				{
					pcVersions->SaveRecord(iValidUntil, oi, pcStore);
				}
				bResult = pcStore->Remove(oi);
			}
		}
		else if (!psRecord->bRemoved)
		{
			if (pcVersions)
			{
				pcVersions->SaveRecord(iValidUntil, oi, pcStore);
				if (psRecord->iNameLength > 0)
				{
					pcVersions->SaveName(iValidUntil, psRecord->GetName(), pcStore);
				}
			}
			if (pcStore->Contains(oi))
			{
				bResult = pcStore->Set(oi, psRecord->GetData(), psRecord->uiDataSize);
			}
			else if (psRecord->iNameLength > 0)
			{
				bResult = pcStore->Add(oi, psRecord->GetName(), psRecord->GetData(), psRecord->uiDataSize);
			}
			else
			{
				bResult = pcStore->Add(oi, psRecord->GetData(), psRecord->uiDataSize);
			}
		}

		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " Could not apply the change to Index [", IndexToString(oi), "].", NULL);
		}
		bExists = mcRecords.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteTransaction::Finish(void)
{
	mbActive = false;
}

