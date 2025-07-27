#include "SystemAllocator.h"
#include "IndexBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlockMapvDataFreeCallback::Init(CIndexBlock* pcIndexBlock)
{
	mpcIndexBlock = pcIndexBlock;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlockMapvDataFreeCallback::DataWillBeFreed(void* pvData)
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
bool CIndexBlock::Get(uint8* pvKey, size iKeySize, void** ppvData, size* piDataSize)
{
	SIndexBlockNode		sNode;
	bool				bResult;

	bResult = mcIndex.Get(pvKey, iKeySize, &sNode, NULL, sizeof(SIndexBlockNode));
	if (bResult)
	{
		SafeAssign(ppvData, sNode.pvData);
		SafeAssign(piDataSize, sNode.iDataSize);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexBlock::Get(uint8* pvKey, size iKeySize)
{
	SIndexBlockNode		sNode;
	bool				bResult;

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
bool CIndexBlock::GetLongestPartial(uint8* pvKey, size iKeySize, void** ppvData, size* piDataSize)
{
	SIndexBlockNode		sNode;
	bool				bResult;

	bResult = mcIndex.GetLongestPartial(pvKey, iKeySize, &sNode, NULL, sizeof(SIndexBlockNode));
	if (bResult)
	{
		SafeAssign(ppvData, sNode.pvData);
		SafeAssign(piDataSize, sNode.iDataSize);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexBlock::GetLongestPartial(uint8* pvKey, size iKeySize)
{
	SIndexBlockNode		sNode;
	bool				bResult;

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
void* CIndexBlock::Put(uint8* pvKey, size iKeySize, size iDataSize)
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
bool CIndexBlock::Put(uint8* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	void*	pv;

	pv = Put(pvKey, iKeySize, iDataSize);
	if (pv)
	{
		memcpy(pv, pvData, iDataSize);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexBlock::Remove(uint8* pvKey, size iKeySize)
{
	SIndexBlockNode		sNode;
	bool				bExists;

	bExists = mcIndex.Get(pvKey, iKeySize, &sNode, NULL, sizeof(SIndexBlockNode));
	if (bExists)
	{
		return mcIndex.Remove(pvKey, iKeySize);
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CIndexBlock::NumElements(void)
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
size CIndexBlock::DataSize(uint8* pvKey, size iKeySize)
{
	return mcIndex.GetDataSize(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexBlock::HasKey(uint8* pvKey, size iKeySize)
{
	return mcIndex.HasKey(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexBlock::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size* puiDataSize, void* pvDestKey, size* puiKeySize, size uiMaxKeySize)
{
	bool				bResult;
	SIndexBlockNode*	psNode;

	bResult = mcIndex.StartUnsafeIteration(psIterator, (uint8*)pvDestKey, puiKeySize, uiMaxKeySize, (void**)&psNode, NULL);
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
bool CIndexBlock::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size* puiDataSize, void* pvDestKey, size* puiKeySize, size uiMaxKeySize)
{
	bool				bResult;
	SIndexBlockNode*	psNode;

	bResult = mcIndex.UnsafeIterate(psIterator, (uint8*)pvDestKey, puiKeySize, uiMaxKeySize, (void**)&psNode, NULL);
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
bool CIndexBlock::Write(CFileWriter* pcFileWriter)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	void*							pvData;
	size							iDataSize;
	size							iKeySize;
	bool							bResult;
	char							acKey[MAX_KEY_SIZE];
	size							iCount;

	if (!mcIndex.WriteConfig(pcFileWriter))
	{
		return false;
	}

	if (!pcFileWriter->WriteSize(mcIndex.NumElements()))
	{
		return false;
	}

	iCount = 0;
	bResult = StartIteration(&sIter, &pvData, &iDataSize, acKey, &iKeySize, MAX_KEY_SIZE);
	while (bResult)
	{
		if (!pcFileWriter->WriteSize(iKeySize))
		{
			return false;
		}
		if (!pcFileWriter->WriteSize(iDataSize))
		{
			return false;
		}
		if (!pcFileWriter->WriteData(acKey, iKeySize))
		{
			return false;
		}
		if (!pcFileWriter->WriteData(pvData, iDataSize))
		{
			return false;
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
bool CIndexBlock::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	size	iCount;
	size	i;
	uint8	acKey[1024];
	size	iKeySize;
	size	iDataSize;
	void*	pvData;

	if (!mcIndex.ReadConfig(pcFileReader))
	{
		return false;
	}

	mcIndexCallback.Init(this);
	mcIndex.SetDataFreeCallback(&mcIndexCallback);
	mpcDataFree = NULL;
	mpcMalloc = mcIndex.GetMallocator();

	if (!pcFileReader->ReadSize(&iCount))
	{
		return false;
	}

	for (i = 0; i < iCount; i++)
	{
		if (!pcFileReader->ReadSize(&iKeySize))
		{
			return false;
		}
		if (!pcFileReader->ReadSize(&iDataSize))
		{
			return false;
		}
		if (!pcFileReader->ReadData(acKey, iKeySize))
		{
			return false;
		}
		pvData = Put(acKey, iKeySize, iDataSize);
		if (!pvData)
		{
			return false;
		}
		if (!pcFileReader->ReadData(pvData, iDataSize))
		{
			return false;
		}
	}

	return true;
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
bool CIndexBlock::ValidateIndexTree(void)
{
	return mcIndex.ValidateIndexTree();
}

