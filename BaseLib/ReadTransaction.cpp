#include "StringHelper.h"
#include "IntegerHelper.h"
#include "Codabase.h"
#include "ReadTransaction.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReadTransaction::Init(CCodabase* pcCodabase, int64 iSnapshot)
{
	mpcCodabase = pcCodabase;
	miSnapshot = iSnapshot;
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
void CReadTransaction::Kill(void)
{
	if (mbActive)
	{
		//The versions this transaction needed can be thrown away.
		mpcCodabase->EndRead(miSnapshot);
		mbActive = false;
	}
	maszIteration.Kill();
	maoiIteration.Kill();
	mpcCodabase = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CReadTransaction::GetSnapshot(void)
{
	return miSnapshot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::Get(OIndex oi, void* pvData)
{
	bool	bResult;

	mpcCodabase->mcMutex.Lock();
	bResult = GetLocked(oi, NULL, pvData, 0, true);
	mpcCodabase->mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::Get(OIndex oi, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	bool	bResult;

	mpcCodabase->mcMutex.Lock();
	bResult = GetLocked(oi, puiDataSize, pvData, uiMaxDataSize, false);
	mpcCodabase->mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::Get(const char* szName, void* pvData)
{
	bool	bResult;
	OIndex	oi;

	mpcCodabase->mcMutex.Lock();
	oi = GetIndexLocked(szName);
	bResult = (oi != INVALID_O_INDEX) && GetLocked(oi, NULL, pvData, 0, true);
	mpcCodabase->mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::Get(const char* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	bool	bResult;
	OIndex	oi;

	mpcCodabase->mcMutex.Lock();
	oi = GetIndexLocked(szName);
	bResult = (oi != INVALID_O_INDEX) && GetLocked(oi, puiDataSize, pvData, uiMaxDataSize, false);
	mpcCodabase->mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::Get(CChars* szName, void* pvData)
{
	return Get(szName ? szName->Text() : NULL, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::Get(CChars* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize)
{
	return Get(szName ? szName->Text() : NULL, puiDataSize, pvData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::GetName(OIndex oi, CChars* szName)
{
	bool	bResult;

	mpcCodabase->mcMutex.Lock();
	bResult = GetNameLocked(oi, szName);
	mpcCodabase->mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::GetName(OIndex oi, char* szName, uint32* puiNameLength, uint32 uiMaxNameLength)
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
OIndex CReadTransaction::GetIndex(const char* szName)
{
	OIndex	oi;

	mpcCodabase->mcMutex.Lock();
	oi = GetIndexLocked(szName);
	mpcCodabase->mcMutex.Unlock();
	return oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CReadTransaction::GetIndex(CChars* szName)
{
	return GetIndex(szName ? szName->Text() : NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::Contains(OIndex oi)
{
	bool	bResult;

	mpcCodabase->mcMutex.Lock();
	bResult = ContainsLocked(oi);
	mpcCodabase->mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::Contains(const char* szName)
{
	return GetIndex(szName) != INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::Contains(CChars* szName)
{
	return GetIndex(szName) != INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CReadTransaction::StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize)
{
	CArrayOIndex	aoi;
	size			i;
	OIndex			oi;

	//The indices that existed at the snapshot: those in the store that have not been added since, and those that have
	//been removed since.
	maoiIteration.ReInit();
	aoi.Init();
	mpcCodabase->mcMutex.Lock();
	mpcCodabase->GetStoredIndices(&aoi);
	mpcCodabase->mcVersions.GetIndices(&aoi);
	for (i = 0; i < aoi.NumElements(); i++)
	{
		oi = *aoi.Get(i);
		if (ContainsLocked(oi))
		{
			maoiIteration.Add(&oi);
		}
	}
	mpcCodabase->mcMutex.Unlock();
	aoi.Kill();

	maoiIteration.QuickSort(CompareLong);
	miIndexPosition = 0;
	return IndexIterate(psIterator, pvData, piDataSize, iMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CReadTransaction::IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize)
{
	OIndex	oi;
	uint32	uiDataSize;
	bool	bResult;

	while (miIndexPosition < maoiIteration.NumElements())
	{
		oi = *maoiIteration.Get(miIndexPosition);
		miIndexPosition++;

		//An index can be both stored and versioned.  It is only returned once.
		if ((miIndexPosition > 1) && (*maoiIteration.Get(miIndexPosition - 2) == oi))
		{
			continue;
		}

		mpcCodabase->mcMutex.Lock();
		bResult = GetLocked(oi, &uiDataSize, pvData, (uint32)iMaxDataSize, false);
		mpcCodabase->mcMutex.Unlock();
		if (bResult)
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
bool CReadTransaction::StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	CArrayChars	aszNames;
	size		i;
	char*		szName;

	maszIteration.ReInit();
	aszNames.Init();
	mpcCodabase->mcMutex.Lock();
	mpcCodabase->GetStoredNames(&aszNames);
	mpcCodabase->mcVersions.GetNames(&aszNames);
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		szName = aszNames.GetText(i);
		if ((GetIndexLocked(szName) != INVALID_O_INDEX) && (maszIteration.FindInSorted(szName) == ARRAY_ELEMENT_NOT_FOUND))
		{
			maszIteration.InsertIntoSorted(aszNames.Get(i));
		}
	}
	mpcCodabase->mcMutex.Unlock();
	aszNames.Kill();

	miNamePosition = 0;
	return NameIterate(psIterator, szKey, poi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	char*	szName;
	OIndex	oi;

	while (miNamePosition < maszIteration.NumElements())
	{
		szName = maszIteration.GetText(miNamePosition);
		miNamePosition++;

		oi = GetIndex(szName);
		if (oi != INVALID_O_INDEX)
		{
			if (szKey)
			{
				StrCpySafe(szKey, szName, MAX_KEY_SIZE);
			}
			SafeAssign(poi, oi);
			return true;
		}
	}

	//As the store does: the index is set to INVALID_O_INDEX at the end, and some callers only test that.
	SafeAssign(poi, INVALID_O_INDEX);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::GetLocked(OIndex oi, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize, bool bAllData)
{
	SVersion*	psVersion;
	uint32		uiCopySize;

	//A version means the object changed after the snapshot.  Without one the store still has it as it was.
	psVersion = mpcCodabase->mcVersions.FindRecord(oi, miSnapshot);
	if (psVersion == NULL)
	{
		if (bAllData)
		{
			return mpcCodabase->mcNamedIndexedData.Get(oi, pvData);
		}
		return mpcCodabase->mcNamedIndexedData.Get(oi, puiDataSize, pvData, uiMaxDataSize);
	}

	if (!psVersion->bExists)
	{
		return false;
	}
	SafeAssign(puiDataSize, psVersion->uiDataSize);
	if (pvData)
	{
		uiCopySize = psVersion->uiDataSize;
		if (!bAllData && (uiMaxDataSize < uiCopySize))
		{
			uiCopySize = uiMaxDataSize;
		}
		memcpy(pvData, psVersion->GetData(), uiCopySize);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CReadTransaction::GetIndexLocked(const char* szName)
{
	OIndex	oi;

	if (StrEmpty(szName))
	{
		return INVALID_O_INDEX;
	}
	if (mpcCodabase->mcVersions.FindName(szName, miSnapshot, &oi))
	{
		return oi;
	}
	return mpcCodabase->mcNamedIndexedData.GetIndex(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::GetNameLocked(OIndex oi, CChars* pszName)
{
	SVersion*	psVersion;

	psVersion = mpcCodabase->mcVersions.FindRecord(oi, miSnapshot);
	if (psVersion == NULL)
	{
		return mpcCodabase->mcNamedIndexedData.GetName(oi, pszName);
	}
	if (!psVersion->bExists)
	{
		return false;
	}
	pszName->Append(psVersion->GetName(), psVersion->iNameLength);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadTransaction::ContainsLocked(OIndex oi)
{
	SVersion*	psVersion;

	psVersion = mpcCodabase->mcVersions.FindRecord(oi, miSnapshot);
	if (psVersion == NULL)
	{
		return mpcCodabase->mcNamedIndexedData.Contains(oi);
	}
	return psVersion->bExists;
}

