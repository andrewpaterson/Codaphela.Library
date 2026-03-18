#include "SystemAllocator.h"
#include "IndexBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::Init(void)
{
	Init(LifeLocal<CMallocator>(&gcSystemAllocator));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::Init(CLifeInit<CMallocator> cMalloc)
{
	mcIndex.Init(cMalloc, this, this);
	mcIndex.SetDataFreeCallback(this);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::Init(CIndexTreeConfig* pcConfig)
{
	mcIndex.Init(pcConfig, this, this);
	mcIndex.SetDataFreeCallback(this);
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

	pvData = Malloc(iDataSize);
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
	mcIndex.SetDataFreeCallback(pcDataFree);
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

	bResult = mcIndex.StartIteration(psIterator, (uint8*)pvDestKey, puiKeySize, uiMaxKeySize, (void**)&psNode, NULL);
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

	bResult = mcIndex.Iterate(psIterator, (uint8*)pvDestKey, puiKeySize, uiMaxKeySize, (void**)&psNode, NULL);
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
bool CIndexBlock::StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void** ppvData, size* puiDataSize, size uiMaxDataSize)
{
	bool				bResult;
	SIndexBlockNode		sNode;

	bResult = mcIndex.StartIteration(psIterator, (uint8*)pvKey, piKeySize, iMaxKeySize, (void*)&sNode, NULL, sizeof(SIndexBlockNode));
	if (bResult)
	{
		if (sNode.pvData)
		{
			SafeAssign(ppvData, *((void**)sNode.pvData));
		}
		SafeAssign(puiDataSize, sNode.iDataSize);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexBlock::Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void** ppvData, size* puiDataSize, size uiMaxDataSize)
{
	bool				bResult;
	SIndexBlockNode		sNode;

	bResult = mcIndex.Iterate(psIterator, (uint8*)pvKey, piKeySize, iMaxKeySize, (void*)&sNode, puiDataSize, uiMaxDataSize);
	if (bResult)
	{
		if (sNode.pvData)
		{
			SafeAssign(ppvData, *((void**)sNode.pvData));
		}
		SafeAssign(puiDataSize, sNode.iDataSize);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexBlock::Write(CFileWriter* pcFileWriter)
{
	return mcIndex.Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexBlock::WriteData(CFileWriter* pcFileWriter, void* pvData)
{
	SIndexIOElement*	sElement;
	SIndexBlockNode*	psValue;

	sElement = (SIndexIOElement*)pvData;
	psValue = (SIndexBlockNode*)sElement->pvData;
	return pcFileWriter->WriteData(psValue->pvData, psValue->iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexBlock::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	mcIndex.SetDataFreeCallback(this);
	return mcIndex.Read(pcFileReader, this, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexBlock::ReadData(CFileReader* pcFileReader, void* pvData)
{
	SIndexIOElement*	sElement;
	void*				pvValue;

	sElement = (SIndexIOElement*)pvData;
	pvValue = Put((uint8*)sElement->pvData, sElement->iKeySize, sElement->iValueSize);
	if (pvValue == NULL)
	{
		return false;
	}
	return pcFileReader->ReadData(pvValue, sElement->iValueSize);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexBlock::AdjustDataSize(void* pvValue, size iValueSize)
{
	SIndexBlockNode* psValue;

	psValue = (SIndexBlockNode*)pvValue;
	return psValue->iDataSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexBlock::Malloc(size uiSize)
{
	return mcIndex.GetMallocator()->Malloc(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexBlock::Realloc(void* pv, size uiSize)
{
	return mcIndex.GetMallocator()->Realloc(pv, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::Free(void* pv)
{
	mcIndex.GetMallocator()->Free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexBlock::FreeData(void* pvData)
{
	SIndexBlockNode* psNode;

	psNode = (SIndexBlockNode*)pvData;
	Free(psNode->pvData);
}

