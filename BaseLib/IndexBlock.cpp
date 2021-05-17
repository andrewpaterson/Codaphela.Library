#include "SystemAllocator.h"
#include "IndexBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::Init(void)
{
	Init(&gcSystemAllocator, NULL);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::Init(CMallocator* pcMalloc, CIndexTreeConfig* pcConfig)
{
	CMalloc::Init(pcMalloc);

	mcIndex.Init(pcConfig);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::Kill(void)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	SIndexBlockNode*				psNode;
	BOOL							bResult;

	bResult = mcIndex.StartUnsafeIteration(&sIter, (void**)&psNode, NULL);
	while (bResult)
	{
		if (psNode)
		{
			mpcMalloc->Free(psNode->pvData);
		}
		bResult = mcIndex.UnsafeIterate(&sIter, (void**)&psNode, NULL);
	}
	mcIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::Get(void* pvKey, int iKeySize, void** ppvData, int* piDataSize)
{
	SIndexBlockNode		sNode;
	BOOL				bResult;

	bResult = mcIndex.Get(pvKey, iKeySize, &sNode, NULL, sizeof(SIndexBlockNode));
	if (bResult)
	{
		SafeAssign(ppvData, sNode.pvData);
		SafeAssign(piDataSize, sNode.iDataSize);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexBlock::Get(void* pvKey, int iKeySize)
{
	SIndexBlockNode		sNode;
	BOOL				bResult;

	bResult = mcIndex.Get(pvKey, iKeySize, &sNode, NULL, sizeof(SIndexBlockNode));
	if (bResult)
	{
		return sNode.pvData;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexBlock::Put(void* pvKey, int iKeySize, int iDataSize)
{
	SIndexBlockNode		sNode;
	void*				pvData;
	BOOL				bExists;

	bExists = mcIndex.Get(pvKey, iKeySize, &sNode, NULL, sizeof(SIndexBlockNode));
	if (bExists)
	{
		mpcMalloc->Free(sNode.pvData);
	}

	pvData = mpcMalloc->Malloc(iDataSize);
	sNode.iDataSize = iDataSize;
	sNode.pvData = pvData;
	mcIndex.Put(pvKey, iKeySize, &sNode, sizeof(sNode));
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::Put(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	void*	pv;

	pv = Put(pvKey, iKeySize, iDataSize);
	if (pv)
	{
		memcpy(pv, pvData, iDataSize);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::Remove(void* pvKey, int iKeySize)
{
	SIndexBlockNode		sNode;
	BOOL				bExists;

	bExists = mcIndex.Get(pvKey, iKeySize, &sNode, NULL, sizeof(SIndexBlockNode));
	if (bExists)
	{
		mpcMalloc->Free(sNode.pvData);
		return mcIndex.Remove(pvKey, iKeySize);
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CIndexBlock::NumElements(void)
{
	return mcIndex.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size_t CIndexBlock::DataSize(void* pvKey, int iKeySize)
{
	return mcIndex.GetDataSize(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::HasKey(void* pvKey, int iKeySize)
{
	return mcIndex.HasKey(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize)
{
	return mcIndex.StartUnsafeIteration(psIterator, (char*)pvDestKey, (int*)puiKeySize, uiMaxKeySize, ppvData, puiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize)
{
	return mcIndex.UnsafeIterate(psIterator, (char*)pvDestKey, (int*)puiKeySize, uiMaxKeySize, ppvData, puiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::Write(CFileWriter* pcFileWriter)
{
	return mcIndex.Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::Read(CFileReader* pcFileReader)
{
	return mcIndex.Read(pcFileReader);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::Dump(void)
{
}


