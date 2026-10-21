#include "PointerRemapper.h"
#include "IndexedGeneral.h"
#include "StackMemory.h"
#include "CodabaseVersions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CVersion::GetName(void)
{
	return (char*)RemapSinglePointer(this, sizeof(CVersion));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CVersion::GetData(void)
{
	return RemapSinglePointer(this, sizeof(CVersion) + iNameLength + 1);
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
	CVersion**			ppcHead;
	CVersion*			pcTail;
	CVersion*			pcVersion;
	CChars				szName;
	size				uiDataSize;
	bool				bExists;
	CStackMemory<2 KB>	cStack;
	void*				pvData;

	//Only the version before the first change in a commit is kept.
	ppcHead = mcRecords.Get(oi);
	pcTail = NULL;
	if (ppcHead != NULL)
	{
		pcTail = *ppcHead;
		while (pcTail->pcNext != NULL)
		{
			pcTail = pcTail->pcNext;
		}
		if (pcTail->iValidUntil == iValidUntil)
		{
			return true;
		}
	}

	//One read of the store gets the name and, usually, the data.  Only data bigger than the stack is read again.
	szName.Init();
	uiDataSize = 0;
	pvData = cStack.Init();
	bExists = pcStore->GetWithName(oi, &szName, &uiDataSize, pvData, (size)cStack.GetStackSize());
	if (!bExists)
	{
		uiDataSize = 0;
	}
	else if (uiDataSize > (size)cStack.GetStackSize())
	{
		cStack.Kill();
		pvData = cStack.Init(uiDataSize);
		pcStore->Get(oi, &uiDataSize, pvData, uiDataSize);
	}

	pcVersion = (CVersion*)malloc(sizeof(CVersion) + szName.Length() + 1 + uiDataSize);
	pcVersion->iValidUntil = iValidUntil;
	pcVersion->pcNext = NULL;
	pcVersion->bExists = bExists;
	pcVersion->iNameLength = (int32)szName.Length();
	pcVersion->uiDataSize = uiDataSize;
	memcpy(pcVersion->GetName(), szName.Text(), szName.Length() + 1);
	if (uiDataSize > 0)
	{
		memcpy(pcVersion->GetData(), pvData, uiDataSize);
	}
	cStack.Kill();

	if (pcTail == NULL)
	{
		mcRecords.Put(oi, &pcVersion);
	}
	else
	{
		pcTail->pcNext = pcVersion;
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
CVersion* CCodabaseVersions::FindRecord(OIndex oi, int64 iSnapshot)
{
	CVersion**	ppcHead;
	CVersion*	pcVersion;

	ppcHead = mcRecords.Get(oi);
	if (ppcHead == NULL)
	{
		return NULL;
	}

	pcVersion = *ppcHead;
	while (pcVersion != NULL)
	{
		if (pcVersion->iValidUntil > iSnapshot)
		{
			return pcVersion;
		}
		pcVersion = pcVersion->pcNext;
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
	CVersion*		pcVersion;
	CVersion*		pcNextVersion;
	SNameVersion*	psName;
	SNameVersion*	psNextName;
	CArrayOIndex	aoiEmpty;
	CArrayChars		aszEmpty;
	size			i;

	//No reader has a snapshot before iOldestSnapshot, so versions that stopped being valid by then are not needed.
	aoiEmpty.Init();
	bExists = mcRecords.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		pcVersion = *((CVersion**)pvData);
		while ((pcVersion != NULL) && (pcVersion->iValidUntil <= iOldestSnapshot))
		{
			pcNextVersion = pcVersion->pcNext;
			free(pcVersion);
			pcVersion = pcNextVersion;
		}
		*((CVersion**)pvData) = pcVersion;
		if (pcVersion == NULL)
		{
			aoiEmpty.Add(*((OIndex*)pvKey));
		}
		bExists = mcRecords.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	for (i = 0; i < aoiEmpty.NumElements(); i++)
	{
		mcRecords.Remove(aoiEmpty.GetValue(i));
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
			aszEmpty.Add((char*)pvKey);
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
		paoi->Add(*((OIndex*)pvKey));
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

	//String map keys keep their terminating zero.
	bExists = mcNames.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		paszNames->Add((char*)pvKey);
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
	CVersion*		pcVersion;
	SNameVersion*	psName;

	uiCount = 0;
	bExists = mcRecords.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		for (pcVersion = *((CVersion**)pvData); pcVersion != NULL; pcVersion = pcVersion->pcNext)
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

