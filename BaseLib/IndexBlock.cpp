#include "SystemAllocator.h"
#include "IndexBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlockMapDataFreeCallback::Init(CIndexBlock* pcIndexBlock)
{
	mpcIndexBlock = pcIndexBlock;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlockMapDataFreeCallback::DataWillBeFreed(void* pvData)
{
	SIndexBlockNode* psNode;

	psNode = (SIndexBlockNode*)pvData;

	mpcIndexBlock->DataWillBeFreed(psNode);
	mpcIndexBlock->Free(psNode->pvData);
}


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
	mcIndexCallback.Init(this);
	mcIndex.SetDataFreeCallback(&mcIndexCallback);
	mpcDataFree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::Kill(void)
{
	mcIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::DataWillBeFreed(SIndexBlockNode* psNode)
{
	if (mpcDataFree)
	{
		mpcDataFree->DataWillBeFreed(psNode->pvData);
	}
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
BOOL CIndexBlock::GetLongestPartial(void* pvKey, int iKeySize, void** ppvData, int* piDataSize)
{
	SIndexBlockNode		sNode;
	BOOL				bResult;

	bResult = mcIndex.GetLongestPartial(pvKey, iKeySize, &sNode, NULL, sizeof(SIndexBlockNode));
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
void* CIndexBlock::GetLongestPartial(void* pvKey, int iKeySize)
{
	SIndexBlockNode		sNode;
	BOOL				bResult;

	bResult = mcIndex.GetLongestPartial(pvKey, iKeySize, &sNode, NULL, sizeof(SIndexBlockNode));
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
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::SetDataFreeCallback(CDataFree* pcDataFree)
{
	mpcDataFree = pcDataFree;
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
	BOOL				bResult;
	SIndexBlockNode*	psNode;

	bResult = mcIndex.StartUnsafeIteration(psIterator, (char*)pvDestKey, (int*)puiKeySize, uiMaxKeySize, (void**)&psNode, NULL);
	if (bResult)
	{
		SafeAssign(ppvData, psNode->pvData);
		SafeAssign(puiDataSize, psNode->iDataSize);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize)
{
	BOOL				bResult;
	SIndexBlockNode*	psNode;

	bResult = mcIndex.UnsafeIterate(psIterator, (char*)pvDestKey, (int*)puiKeySize, uiMaxKeySize, (void**)&psNode, NULL);
	if (bResult)
	{
		SafeAssign(ppvData, psNode->pvData);
		SafeAssign(puiDataSize, psNode->iDataSize);
	}
	return bResult;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::Write(CFileWriter* pcFileWriter)
{
	SIndexTreeMemoryUnsafeIterator		sIter;
	void*								pvData;
	size_t								iDataSize;
	size_t								iKeySize;
	BOOL								bResult;
	char								acKey[MAX_KEY_SIZE];
	int									iCount;

	if (!mcIndex.WriteConfig(pcFileWriter))
	{
		return FALSE;
	}

	if (!pcFileWriter->WriteInt(mcIndex.NumElements()))
	{
		return FALSE;
	}

	iCount = 0;
	bResult = StartIteration(&sIter, &pvData, &iDataSize, acKey, &iKeySize, MAX_KEY_SIZE);
	while (bResult)
	{
		if (!pcFileWriter->WriteInt((int)iKeySize))
		{
			return FALSE;
		}
		if (!pcFileWriter->WriteInt((int)iDataSize))
		{
			return FALSE;
		}
		if (!pcFileWriter->WriteData(acKey, iKeySize))
		{
			return FALSE;
		}
		if (!pcFileWriter->WriteData(pvData, iDataSize))
		{
			return FALSE;
		}

		bResult = Iterate(&sIter, &pvData, &iDataSize, acKey, &iKeySize, MAX_KEY_SIZE);
		iCount ++;
	}
	return mcIndex.NumElements() == iCount;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	int				iCount;
	int				i;
	char			acKey[1024];
	int				iKeySize;
	int				iDataSize;
	void* pvData;

	if (!mcIndex.ReadConfig(pcFileReader))
	{
		return FALSE;
	}

	mcIndexCallback.Init(this);
	mcIndex.SetDataFreeCallback(&mcIndexCallback);
	mpcDataFree = NULL;
	mpcMalloc = mcIndex.GetMallocator();

	if (!pcFileReader->ReadInt(&iCount))
	{
		return FALSE;
	}

	for (i = 0; i < iCount; i++)
	{
		if (!pcFileReader->ReadInt(&iKeySize))
		{
			return FALSE;
		}
		if (!pcFileReader->ReadInt(&iDataSize))
		{
			return FALSE;
		}
		if (!pcFileReader->ReadData(acKey, iKeySize))
		{
			return FALSE;
		}
		pvData = Put(acKey, iKeySize, iDataSize);
		if (!pvData)
		{
			return FALSE;
		}
		if (!pcFileReader->ReadData(pvData, iDataSize))
		{
			return FALSE;
		}
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::Dump(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexBlock::ValidateIndexTree(void)
{
	return mcIndex.ValidateIndexTree();
}

