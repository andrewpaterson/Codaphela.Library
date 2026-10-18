#include "PointerRemapper.h"
#include "IndexedGeneral.h"
#include "CodabaseVersions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* SVersion::GetName(void)
{
	return (char*)RemapSinglePointer(this, sizeof(SVersion));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* SVersion::GetData(void)
{
	return RemapSinglePointer(this, sizeof(SVersion) + iNameLength + 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabaseVersions::Init(void)
{
	mcRecords.Init(true);
	mcNames.Init(true, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabaseVersions::Kill(void)
{
	RemoveUpTo(MAX_INT64);
	mcNames.Kill();
	mcRecords.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabaseVersions::SaveRecord(int64 iValidUntil, OIndex oi, CNamedIndexedData* pcStore)
{
	SVersion**	ppsHead;
	SVersion*	psTail;
	SVersion*	psVersion;
	CChars		szName;
	uint32		uiDataSize;
	bool		bExists;

	//Only the version before the first change in a commit is kept.
	ppsHead = mcRecords.Get(oi);
	psTail = NULL;
	if (ppsHead != NULL)
	{
		psTail = *ppsHead;
		while (psTail->psNext != NULL)
		{
			psTail = psTail->psNext;
		}
		if (psTail->iValidUntil == iValidUntil)
		{
			return true;
		}
	}

	szName.Init();
	uiDataSize = 0;
	bExists = pcStore->Get(oi, &uiDataSize, NULL, 0);
	if (bExists)
	{
		pcStore->GetName(oi, &szName);
	}
	else
	{
		uiDataSize = 0;
	}

	psVersion = (SVersion*)malloc(sizeof(SVersion) + szName.Length() + 1 + uiDataSize);
	psVersion->iValidUntil = iValidUntil;
	psVersion->psNext = NULL;
	psVersion->bExists = bExists;
	psVersion->iNameLength = (int32)szName.Length();
	psVersion->uiDataSize = uiDataSize;
	memcpy(psVersion->GetName(), szName.Text(), szName.Length() + 1);
	if (uiDataSize > 0)
	{
		pcStore->Get(oi, &uiDataSize, psVersion->GetData(), uiDataSize);
	}

	if (psTail == NULL)
	{
		mcRecords.Put(oi, &psVersion);
	}
	else
	{
		psTail->psNext = psVersion;
	}

	//The object's name changes with it.
	if (!szName.Empty())
	{
		SaveName(iValidUntil, szName.Text(), pcStore);
	}
	szName.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabaseVersions::SaveName(int64 iValidUntil, const char* szName, CNamedIndexedData* pcStore)
{
	SNameVersion**	ppsHead;
	SNameVersion*	psTail;
	SNameVersion*	psVersion;

	ppsHead = mcNames.Get(szName);
	psTail = NULL;
	if (ppsHead != NULL)
	{
		psTail = *ppsHead;
		while (psTail->psNext != NULL)
		{
			psTail = psTail->psNext;
		}
		if (psTail->iValidUntil == iValidUntil)
		{
			return true;
		}
	}

	psVersion = (SNameVersion*)malloc(sizeof(SNameVersion));
	psVersion->iValidUntil = iValidUntil;
	psVersion->psNext = NULL;
	psVersion->oi = pcStore->GetIndex(szName);

	if (psTail == NULL)
	{
		mcNames.Put(szName, &psVersion);
	}
	else
	{
		psTail->psNext = psVersion;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SVersion* CCodabaseVersions::FindRecord(OIndex oi, int64 iSnapshot)
{
	SVersion**	ppsHead;
	SVersion*	psVersion;

	ppsHead = mcRecords.Get(oi);
	if (ppsHead == NULL)
	{
		return NULL;
	}

	psVersion = *ppsHead;
	while (psVersion != NULL)
	{
		if (psVersion->iValidUntil > iSnapshot)
		{
			return psVersion;
		}
		psVersion = psVersion->psNext;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodabaseVersions::FindName(const char* szName, int64 iSnapshot, OIndex* poi)
{
	SNameVersion**	ppsHead;
	SNameVersion*	psVersion;

	ppsHead = mcNames.Get(szName);
	if (ppsHead == NULL)
	{
		return false;
	}

	psVersion = *ppsHead;
	while (psVersion != NULL)
	{
		if (psVersion->iValidUntil > iSnapshot)
		{
			*poi = psVersion->oi;
			return true;
		}
		psVersion = psVersion->psNext;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabaseVersions::RemoveUpTo(int64 iOldestSnapshot)
{
	SMapIterator	sIter;
	void*			pvKey;
	void*			pvData;
	size			iKeySize;
	size			iDataSize;
	bool			bExists;
	SVersion*		psVersion;
	SVersion*		psNextVersion;
	SNameVersion*	psName;
	SNameVersion*	psNextName;
	CArrayOIndex	aoiEmpty;
	CArrayChars		aszEmpty;
	CChars			szName;
	size			i;

	//No reader has a snapshot before iOldestSnapshot, so versions that stopped being valid by then are not needed.
	aoiEmpty.Init();
	bExists = mcRecords.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		psVersion = *((SVersion**)pvData);
		while ((psVersion != NULL) && (psVersion->iValidUntil <= iOldestSnapshot))
		{
			psNextVersion = psVersion->psNext;
			free(psVersion);
			psVersion = psNextVersion;
		}
		*((SVersion**)pvData) = psVersion;
		if (psVersion == NULL)
		{
			aoiEmpty.Add((OIndex*)pvKey);
		}
		bExists = mcRecords.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	for (i = 0; i < aoiEmpty.NumElements(); i++)
	{
		mcRecords.Remove(*aoiEmpty.Get(i));
	}
	aoiEmpty.Kill();

	aszEmpty.Init();
	bExists = mcNames.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		psName = *((SNameVersion**)pvData);
		while ((psName != NULL) && (psName->iValidUntil <= iOldestSnapshot))
		{
			psNextName = psName->psNext;
			free(psName);
			psName = psNextName;
		}
		*((SNameVersion**)pvData) = psName;
		if (psName == NULL)
		{
			szName.Init((char*)pvKey, 0, strnlen((char*)pvKey, iKeySize));
			aszEmpty.Add(&szName);
			szName.Kill();
		}
		bExists = mcNames.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	for (i = 0; i < aszEmpty.NumElements(); i++)
	{
		mcNames.Remove(aszEmpty.GetText(i));
	}
	aszEmpty.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabaseVersions::GetIndices(CArrayOIndex* paoi)
{
	SMapIterator	sIter;
	void*			pvKey;
	void*			pvData;
	size			iKeySize;
	size			iDataSize;
	bool			bExists;

	bExists = mcRecords.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		paoi->Add((OIndex*)pvKey);
		bExists = mcRecords.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabaseVersions::GetNames(CArrayChars* paszNames)
{
	SMapIterator	sIter;
	void*			pvKey;
	void*			pvData;
	size			iKeySize;
	size			iDataSize;
	bool			bExists;
	CChars			szName;

	bExists = mcNames.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		szName.Init((char*)pvKey, 0, strnlen((char*)pvKey, iKeySize));
		paszNames->Add(&szName);
		szName.Kill();
		bExists = mcNames.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCodabaseVersions::NumVersions(void)
{
	SMapIterator	sIter;
	void*			pvKey;
	void*			pvData;
	size			iKeySize;
	size			iDataSize;
	bool			bExists;
	size			uiCount;
	SVersion*		psVersion;
	SNameVersion*	psName;

	uiCount = 0;
	bExists = mcRecords.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		for (psVersion = *((SVersion**)pvData); psVersion != NULL; psVersion = psVersion->psNext)
		{
			uiCount++;
		}
		bExists = mcRecords.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	bExists = mcNames.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		for (psName = *((SNameVersion**)pvData); psName != NULL; psName = psName->psNext)
		{
			uiCount++;
		}
		bExists = mcNames.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	return uiCount;
}

