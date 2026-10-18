#include "Logger.h"
#include "StringHelper.h"
#include "PointerRemapper.h"
#include "IndexedGeneral.h"
#include "IntegerHelper.h"
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
void CWriteTransaction::Init(CCodabase* pcCodabase)
{
	mpcCodabase = pcCodabase;
	mcRecords.Init(true);
	mcNames.Init(true, true);
	mbActive = true;
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
	maszIteration.Kill();
	maoiIteration.Kill();
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

	if (!CheckActive(__METHOD__))
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
	mpcCodabase->ReleaseWriter();
	mpcCodabase->mcMutex.Unlock();

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
		mpcCodabase->mcMutex.Lock();
		mpcCodabase->ReleaseWriter();
		mpcCodabase->mcMutex.Unlock();
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
	bool	bReplacesStored;

	if (!CheckActive(__METHOD__))
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
	if (Exists(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Add Index [", IndexToString(oi), "].  It already exists.", NULL);
	}
	if (!StrEmpty(szName) && (FindName(szName) != INVALID_O_INDEX))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Add Name [", szName, "] with Index [", IndexToString(oi), "].  The name already exists.", NULL);
	}

	//A stored object that does not exist any more was removed by this transaction.
	bReplacesStored = StoredContains(oi);
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
	if (!CheckActive(__METHOD__))
	{
		return false;
	}
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

	oi = FindName(szName);
	if (oi == INVALID_O_INDEX)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Set Name [", StringToString(szName), "].  It does not exist.", NULL);
	}
	return Set(oi, pvData, uiDataSize);
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
	if (Exists(oi))
	{
		return Set(oi, pvData, uiDataSize);
	}
	else
	{
		return Add(oi, pvData, uiDataSize);
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

	//The same rules as CNamedIndexedData::Put.
	oiExisting = FindName(szName);
	bExists = Exists(oi);
	if (bExists && (oiExisting != INVALID_O_INDEX))
	{
		if (oiExisting != oi)
		{
			return gcLogger.Error2(__METHOD__, " Put Index [", IndexToString(oi), "] not equal to existing index [", IndexToString(oiExisting), "] for name [", szName, "].", NULL);
		}
		return Set(oi, pvData, uiDataSize);
	}
	else if (!bExists && (oiExisting == INVALID_O_INDEX))
	{
		return Add(oi, szName, pvData, uiDataSize);
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

	psRecord = GetRecord(oi);
	if (psRecord == NULL)
	{
		return mpcCodabase->Get(oi, pvData);
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

	psRecord = GetRecord(oi);
	if (psRecord == NULL)
	{
		return mpcCodabase->Get(oi, puiDataSize, pvData, uiMaxDataSize);
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

	oi = FindName(szName);
	if (oi == INVALID_O_INDEX)
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

	oi = FindName(szName);
	if (oi == INVALID_O_INDEX)
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
	return FindName(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CWriteTransaction::GetIndex(CChars* szName)
{
	return FindName(szName ? szName->Text() : NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Contains(OIndex oi)
{
	return Exists(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Contains(const char* szName)
{
	return FindName(szName) != INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Contains(CChars* szName)
{
	return FindName(szName ? szName->Text() : NULL) != INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteTransaction::Remove(OIndex oi)
{
	CChars	szName;
	OIndex	oiRemoved;

	if (!CheckActive(__METHOD__))
	{
		return false;
	}
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

	if (StoredContains(oi))
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
bool CWriteTransaction::Remove(const char* szName)
{
	OIndex	oi;

	oi = FindName(szName);
	if (oi == INVALID_O_INDEX)
	{
		return false;
	}
	return Remove(oi);
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

	while (miIndexPosition < maoiIteration.NumElements())
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

	while (miNamePosition < maszIteration.NumElements())
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
bool CWriteTransaction::CheckActive(const char* szMethod)
{
	if (!mbActive)
	{
		return gcLogger.Error2(szMethod, " The write transaction has already been committed or aborted.", NULL);
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
bool CWriteTransaction::StoredContains(OIndex oi)
{
	return mpcCodabase->Contains(oi);
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
	return StoredContains(oi);
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
	return mpcCodabase->GetIndex(szName);
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
	return mpcCodabase->GetName(oi, pszName);
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
	return mpcCodabase->Get(oi, puiDataSize, NULL, 0);
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

