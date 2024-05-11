#include "Numbers.h"
#include "GlobalMemory.h"
#include "DataOrderers.h"
#include "LifeCycle.h"
#include "Logger.h"
#include "MapHelper.h"
#include "IndexTreeMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Init(void)
{
	Init(LifeLocal<CMallocator>(&gcSystemAllocator), IKR_No, MAX_DATA_SIZE, MAX_KEY_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Init(EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	Init(LifeLocal<CMallocator>(&gcSystemAllocator), eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE, cDataOrderer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Init(EIndexKeyReverse eKeyReverse)
{
	Init(LifeLocal<CMallocator>(&gcSystemAllocator), eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse)
{
	Init(cMalloc, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, size iMaxDataSize, size iMaxKeySize)
{
	Init(cMalloc, eKeyReverse, iMaxDataSize, iMaxKeySize, LifeNull<CIndexTreeDataOrderer>());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Init(CIndexTreeConfig* pcConfig)
{
	if (pcConfig)
	{
		Init(pcConfig->GetMalloc(), pcConfig->GetKeyReverse(), pcConfig->GetMaxDataSize(), pcConfig->GetMaxKeySize(), pcConfig->GetDataOrderer());
	}
	else
	{
		Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, size iMaxDataSize, size iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	CIndexTree::Init(cMalloc, eKeyReverse, sizeof(CIndexTreeNodeMemory), sizeof(CIndexTreeNodeMemory) + sizeof(CIndexTreeDataNode), sizeof(CIndexTreeNodeMemory*), iMaxDataSize, iMaxKeySize, cDataOrderer);
	mpcRoot = AllocateRoot();
	miSize = 0;
	mpcDataFree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::Kill(void)
{
	RecurseKill(mpcRoot);
	return CIndexTree::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::RecurseKill(CIndexTreeNodeMemory* pcNode)
{
	size					i;
	CIndexTreeNodeMemory*	pcChild;

	if (pcNode != NULL)
	{
		for (i = 0; i < pcNode->NumIndexes(); i++)
		{
			pcChild = pcNode->GetNode(i);
			RecurseKill(pcChild);
		}
		if (mpcDataFree)
		{
			if (pcNode->HasData())
			{
				mpcDataFree->DataWillBeFreed(GetDataForNode(pcNode));
			}
		}
		FreeNode(pcNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::AllocateRoot(void)
{
	CIndexTreeNodeMemory*	pcNode;

	pcNode = (CIndexTreeNodeMemory*)Malloc(CalculateRootNodeSize());
	pcNode->Init(this, NULL, 0, MAX_UCHAR, 0);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::AllocateNode(CIndexTreeNodeMemory* pcParent, size uiIndexInParent)
{
	CIndexTreeNodeMemory*	pcNode;

	pcNode = (CIndexTreeNodeMemory*)Malloc(SizeofNode());
	pcNode->Init(this, pcParent, uiIndexInParent);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::GetNodeForData(void* pvData)
{
	return (CIndexTreeNodeMemory*)CIndexTree::GetNodeForData(pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::GetRoot(void)
{
	return mpcRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::GetNode(void* pvKey, size iKeySize)
{
	CIndexTreeNodeMemory*	pcCurrent;
	size					i;
	size					c;
	bool					bExecute;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((uint8*)pvKey)[i];

		pcCurrent = pcCurrent->Get(c);
		if (pcCurrent == NULL)
		{
			return NULL;
		}
		bExecute = LoopKey(&i, iKeySize);
	}
	return pcCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::GetNodeFromLongestPartialKey(void* pvKey, size iKeySize)
{
	CIndexTreeNodeMemory*	pcCurrent;
	size					i;
	size					c;
	bool					bExecute;
	CIndexTreeNodeMemory*	pcFound;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcFound = NULL;
	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((uint8*)pvKey)[i];

		pcCurrent = pcCurrent->Get(c);
		if (pcCurrent == NULL)
		{
			break;
		}
		if (pcCurrent->HasData())
		{
			pcFound = pcCurrent;
		}
		bExecute = LoopKey(&i, iKeySize);
	}
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::Get(void* pvKey, size iKeySize, void* pvDestData, size* puiDataSize, size uiMaxDataSize)
{
	CIndexTreeNodeMemory*	pcNode;
	size					uiDataSize;

	pcNode = GetNode(pvKey, iKeySize);
	return GetNodeData(pcNode, uiDataSize, puiDataSize, pvDestData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::GetLongestPartial(void* pvKey, size iKeySize, void* pvDestData, size* puiDataSize, size uiMaxDataSize)
{
	CIndexTreeNodeMemory*	pcNode;
	size					uiDataSize;

	pcNode = GetNodeFromLongestPartialKey(pvKey, iKeySize);
	return GetNodeData(pcNode, uiDataSize, puiDataSize, pvDestData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::GetNodeData(CIndexTreeNodeMemory* pcNode, size& uiDataSize, size* puiDataSize, void* pvDestData, size uiMaxDataSize)
{
	void* pvData;

	if (pcNode == NULL)
	{
		return false;
	}

	if (pcNode->HasData())
	{
		GetReorderData(pcNode);

		uiDataSize = pcNode->GetDataSize();
		SafeAssign(puiDataSize, uiDataSize);
		pvData = pcNode->GetDataPtr();
		if (pvDestData)
		{
			pvData = pcNode->GetDataPtr();
			memcpy(pvDestData, pvData, MinDataSize(uiDataSize, uiMaxDataSize));
		}

		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeMemory::Put(void* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	CIndexTreeNodeMemory*	pcCurrent;
	size					uiDataSize;
	bool					bNewNode;

	ReturnNullOnFalse(ValidatePut(iKeySize, iDataSize));

	uiDataSize = iDataSize;

	pcCurrent = GetOrAllocateKey(pvKey, iKeySize);
	if (pcCurrent == NULL)
	{
		return NULL;
	}

	bNewNode = false;
	if (!pcCurrent->HasData())
	{
		miSize++;
		bNewNode = true;
	}
	else
	{
		if (mpcDataFree)
		{
			mpcDataFree->DataWillBeFreed(GetDataForNode(pcCurrent));
		}
	}

	pcCurrent = SetNodeData(pcCurrent, pvData, uiDataSize);
	if (pcCurrent == NULL)
	{
		return NULL;
	}

	if (bNewNode)
	{
		InsertReorderData(pcCurrent);
	}
	PutReorderData(pcCurrent);

	return pcCurrent->GetDataPtr();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::GetOrAllocateKey(void* pvKey, size iKeySize)
{
	CIndexTreeNodeMemory*	pcCurrent;
	size					c;
	bool					bExecute;
	size					i;

	if (iKeySize == 0)
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((uint8*)pvKey)[i];
		pcCurrent = SetOldWithCurrent(pcCurrent, c);
		bExecute = LoopKey(&i, iKeySize);
	}
	return pcCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::SetNodeData(CIndexTreeNodeMemory* pcCurrent, void* pvData, size uiDataSize)
{
	CIndexTreeNodeMemory*	pcReallocatedCurrent;
	size					uiOldDataSize;

	uiOldDataSize = pcCurrent->GetDataSize();
	if (uiDataSize > uiOldDataSize)
	{
		pcReallocatedCurrent = ReallocateNodeForLargerData(pcCurrent, pvData, uiDataSize);
	}
	else if (uiDataSize < uiOldDataSize)
	{
		pcReallocatedCurrent = ReallocateNodeForSmallerData(pcCurrent, pvData, uiDataSize);
	}
	else
	{
		pcReallocatedCurrent = pcCurrent;
		pcReallocatedCurrent->SetData(pvData, uiDataSize);
	}

	if (pcReallocatedCurrent == NULL)
	{
		return NULL;
	}

	return pcReallocatedCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, size uiIndex)
{
	CIndexTreeNodeMemory*	pcOldNode;
	size					tNewNodeSize;

	if (pcNode->ContainsIndex(uiIndex))
	{
		return pcNode;
	}

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForIndex(uiIndex);

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeMemory*)Realloc(pcNode, tNewNodeSize);
	if (pcNode)
	{
		pcNode->Contain(uiIndex);

		RemapNodePointers(pcOldNode, pcNode);
		return pcNode;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForLargerData(CIndexTreeNodeMemory* pcNode, void* pvData, size uiDataSize)
{
	size					tNewNodeSize;
	CIndexTreeNodeMemory*	pcOldNode;

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForData(uiDataSize);

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeMemory*)Realloc(pcNode, tNewNodeSize);
	if (pcNode)
	{
		pcNode->SetData(pvData, uiDataSize);

		RemapNodePointers(pcOldNode, pcNode);
		return pcNode;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForSmallerData(CIndexTreeNodeMemory* pcNode, void* pvData, size uiDataSize)
{
	size					tNewNodeSize;
	CIndexTreeNodeMemory*	pcOldNode;
	size			uiOriginalSize;

	uiOriginalSize = pcNode->GetDataSize();
	pcNode->SetData(pvData, uiDataSize);

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();
	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeMemory*)Realloc(pcNode, tNewNodeSize);
	if (pcNode)
	{
		RemapNodePointers(pcOldNode, pcNode);
		return pcNode;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForRemove(CIndexTreeNodeMemory* pcNode, size tNewNodeSize)
{
	CIndexTreeNodeMemory* pcOldNode;

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeMemory*)Realloc(pcNode, tNewNodeSize);
	if (pcNode)
	{
		RemapNodePointers(pcOldNode, pcNode);
		return pcNode;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::RemapNodePointers(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNode)
{
	CIndexTreeNodeMemory*		pcParent;

	if (pcOldNode != pcNode)
	{
		pcParent = (CIndexTreeNodeMemory*)pcNode->GetParent();
		if (pcParent)
		{
			pcParent->RemapChildNodes(pcOldNode, pcNode);
		}
		pcNode->SetChildrensParent();

		if (mpcDataOrderer)
		{
			pcNode->RemapvDataLinks(pcOldNode, pcNode);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, size uiIndexInParent)
{
	CIndexTreeNodeMemory* pcNew;
	CIndexTreeNodeMemory* pcChildNodeOnParent;
	CIndexTreeNodeMemory* pcReallocedParent;

	pcChildNodeOnParent = pcParent->Get(uiIndexInParent);
	if (pcChildNodeOnParent == NULL)
	{
		pcReallocedParent = ReallocateNodeForIndex(pcParent, uiIndexInParent);
		pcNew = AllocateNode(pcReallocedParent, uiIndexInParent);
		pcReallocedParent->Set(uiIndexInParent, pcNew);
		return pcNew;
	}
	else
	{
		return pcChildNodeOnParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::Remove(void* pvKey, size iKeySize)
{
	size					c;
	CIndexTreeNodeMemory*	pcCurrent;
	size					i;
	bool					bExecute;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return false;
	}

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((uint8*)pvKey)[i];
		pcCurrent = pcCurrent->Get(c);
		if (pcCurrent == NULL)
		{
			return false;
		}
		bExecute = LoopKey(&i, iKeySize);
	}

	return Remove(pcCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::Remove(CIndexTreeNodeMemory*	pcCurrent)
{
	size					c;
	CIndexTreeNodeMemory*	pcParent;
	CIndexTreeNodeMemory*	pcNode;
	bool					bResizeNode;
	size					tNewNodeSize;
	size			uiOldDataSize;

	uiOldDataSize = pcCurrent->GetDataSize();
	if (uiOldDataSize == 0)
	{
		return false;
	}

	RemoveReorderData(pcCurrent);
	if (mpcDataFree)
	{
		mpcDataFree->DataWillBeFreed(GetDataForNode(pcCurrent));
	}
	pcNode = ReallocateNodeForSmallerData(pcCurrent, NULL, 0);

	pcParent = (CIndexTreeNodeMemory*)pcNode->GetParent();
	for (;;)
	{
		c = pcNode->GetIndexInParent();

		if (pcNode->IsEmpty())
		{
			if (pcParent != mpcRoot)
			{
				bResizeNode = pcParent->ClearAndUncontain(c);
				if (bResizeNode)
				{
					tNewNodeSize = pcParent->CalculateRequiredNodeSizeForCurrent();

					pcParent = ReallocateNodeForRemove(pcParent, tNewNodeSize);
				}
			}
			else
			{
				pcParent->ClearIndex(c);
			}

			Free(pcNode);
		}
		else
		{
			break;
		}

		pcNode = pcParent;
		pcParent = (CIndexTreeNodeMemory*)pcNode->GetParent();
		if (!pcParent)
		{
			break;
		}
	}

	miSize--;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::FindAll(CArrayVoidPtr* papvElements)
{
	RecurseFindAll(mpcRoot, papvElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::RecurseFindAll(CIndexTreeNodeMemory* pcNode, CArrayVoidPtr* papvElements)
{
	size						i;
	CIndexTreeNodeMemory*	pcChild;
	void*					pvData;

	if (pcNode != NULL)
	{
		pvData = pcNode->GetDataPtr();
		if (pvData != NULL)
		{
			papvElements->Add(pvData);
		}

		for (i = 0; i < pcNode->NumIndexes(); i++)
		{
			pcChild = pcNode->GetNode(i);
			RecurseFindAll(pcChild, papvElements);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::WriteConfig(CFileWriter* pcFileWriter)
{
	CIndexTreeMemoryConfig				cConfig;
	CLifeInit<CMallocator>				cMalloc;
	CLifeInit<CIndexTreeDataOrderer>	cDataOrderer;
	bool								bResult;

	cMalloc.Init(mpcMalloc, mcMallocLife.MustFree(), mcMallocLife.MustKill());
	cDataOrderer.Init(mpcDataOrderer, mcDataOrdererLife.MustFree(), mcDataOrdererLife.MustKill());
	cConfig.Init(cMalloc, meReverseKey, miMaxDataSize, miMaxKeySize, cDataOrderer);
	bResult = cConfig.Write(pcFileWriter);
	cConfig.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::Write(CFileWriter* pcFileWriter)
{
	SIndexTreeMemoryUnsafeIterator		sIter;
	void*								pvData;
	size								iDataSize;
	size								iKeySize;
	bool								bResult;
	uint8								acKey[MAX_KEY_SIZE];
	size								iCount;

	if (!WriteConfig(pcFileWriter))
	{
		return false;
	}

	if (!pcFileWriter->WriteSize(miSize))
	{
		return false;
	}

	iCount = 0;
	bResult = StartUnsafeIteration(&sIter, &pvData, &iDataSize);
	while (bResult)
	{
		iKeySize = GetKey(pvData, acKey, MAX_KEY_SIZE);
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

		bResult = UnsafeIterate(&sIter, &pvData, &iDataSize);
		iCount ++;
	}
	return miSize == iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::ReadConfig(CFileReader* pcFileReader)
{
	CIndexTreeMemoryConfig				cConfig;
	bool								bResult;

	bResult = cConfig.Read(pcFileReader);
	if (bResult)
	{
		Init(cConfig.GetMalloc(), cConfig.GetKeyReverse(), cConfig.GetMaxDataSize(), cConfig.GetMaxKeySize(), cConfig.GetDataOrderer());
	}
	cConfig.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	size				iCount;
	size				i;
	uint8				acKey[1024];
	size				iKeySize;
	size				iDataSize;
	void*			pvData;

	if (!ReadConfig(pcFileReader))
	{
		return false;
	}

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
		pvData = Put(acKey, iKeySize, NULL, iDataSize);
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
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::NumElements(void)
{
	return miSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::SetDataFreeCallback(CDataFree* pcDataFree)
{
	mpcDataFree = pcDataFree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::GetKey(void* pvData, uint8* pvDestKey, size iDestKeySize)
{
	CIndexTreeNodeMemory*	pcNode;

	if (pvData == NULL)
	{
		return 0;
	}

	pcNode = GetNodeForData(pvData);

	GetReorderData(pcNode);

	return GetNodeKey(pcNode, pvDestKey, iDestKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::GetNodeKeySize(CIndexTreeNode* pcNode)
{
	size	iKeySize;

	iKeySize = 0;
	while (pcNode != mpcRoot)
	{
		iKeySize++;
		pcNode = pcNode->GetParent();
	}
	return iKeySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::GetNodeKey(CIndexTreeNode* pcNode, uint8* pvDestKey, size iDestKeySize)
{
	size	iKeySize;
	size	iLength;

	if (pcNode == NULL)
	{
		return 0;
	}

	iKeySize = 0;
	iLength = 0;
	while (pcNode && pcNode != mpcRoot)
	{
		if (iKeySize < iDestKeySize)
		{
			pvDestKey[iKeySize] = (uint8)pcNode->GetIndexInParent();
			iLength++;
		}
		iKeySize++;
		pcNode = (CIndexTreeNodeMemory*)pcNode->GetParent();
	}

	if (meReverseKey == IKR_No)
	{
		//The key is already reversed by revese node traversal.
		ReverseBytes(pvDestKey, iLength);
	}
	else if (meReverseKey == IKR_Unknown)
	{
		gcLogger.Error2(__METHOD__, " Don't know how to order key bytes for direction [IKR_Unknown].", NULL);
		return false;
	}

	if (iKeySize < iDestKeySize)
	{
		pvDestKey[iKeySize] = 0;
	}

	return iKeySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::GetNodeDataSize(CIndexTreeNode* pcNode)
{
	if (pcNode->HasData())
	{
		return pcNode->GetDataSize();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, size iDestDataSize)
{
	size				iDataSize;
	void* pvData;

	if (pcNode->HasData())
	{
		iDataSize = pcNode->GetDataSize();
		pvData = pcNode->GetDataPtr();
		if (iDataSize > iDestDataSize)
		{
			memcpy_fast(pvDestData, pvData, iDestDataSize);
		}
		else
		{
			memcpy_fast(pvDestData, pvData, iDataSize);
		}
		return iDataSize;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::StartIteration(SIndexTreeMemoryIterator* psIterator, void* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* piDataSize, size iMaxDataSize)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	size							iDataSize;
	bool							bResult;

	memset(psIterator->pvKey, 0, MAX_KEY_SIZE);
	psIterator->iIndex = mpcRoot->GetFirstIndex();
	psIterator->iKeyLength = 0;

	bResult = StartUnsafeIteration(&sIter, psIterator->pvKey, &psIterator->iKeyLength, MAX_KEY_SIZE, NULL, NULL);
	psIterator->iIndex = sIter.iIndex;

	if (bResult)
	{
		CopyData((uint8*)pvKey, psIterator->pvKey, psIterator->iKeyLength, iMaxKeySize);
		SafeAssign(piKeySize, psIterator->iKeyLength);
		if (piDataSize)
		{
			iDataSize = GetNodeData(sIter.pcNode, pvData, iMaxDataSize);
			*piDataSize, iDataSize;
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::Iterate(SIndexTreeMemoryIterator* psIterator, void* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* piDataSize, size iMaxDataSize)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	size							iDataSize;
	bool							bResult;

	sIter.iIndex = psIterator->iIndex;
	sIter.pcNode = GetNode(psIterator->pvKey, psIterator->iKeyLength);

	if (!sIter.pcNode)
	{
		return false;
	}

	bResult = UnsafeIterate(&sIter, (uint8*)psIterator->pvKey, &psIterator->iKeyLength, MAX_KEY_SIZE, NULL, NULL);
	psIterator->iIndex = sIter.iIndex;

	if (bResult)
	{
		CopyData((uint8*)pvKey, psIterator->pvKey, psIterator->iKeyLength, iMaxKeySize);
		SafeAssign(piKeySize, psIterator->iKeyLength);
		if (piDataSize)
		{
			iDataSize = GetNodeData(sIter.pcNode, pvData, iMaxDataSize);
			*piDataSize, iDataSize;
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::StartUnsafeIteration(SIndexTreeMemoryUnsafeIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void** ppvData, size* piDataSize)
{
	psIterator->pcNode = mpcRoot;
	psIterator->iIndex = mpcRoot->GetFirstIndex();

	return UnsafeIterate(psIterator, pvKey, piKeySize, iMaxKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::UnsafeIterate(SIndexTreeMemoryUnsafeIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void** pvData, size* piDataSize)
{
	void*	pvDataTemp;
	size		iKeySize;

	if (StepNext(psIterator))
	{
		pvDataTemp = psIterator->pcNode->GetDataPtr();
		if (pvData)
		{
			*pvData = pvDataTemp;
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->GetDataSize();
		}
		if (pvKey)
		{
			iKeySize = GetNodeKey(psIterator->pcNode, pvKey, iMaxKeySize);
			if (piKeySize)
			{
				*piKeySize = iKeySize;
			}
		}
		else if (piKeySize)
		{
			*piKeySize = GetNodeKeySize(psIterator->pcNode);
		}

		GetReorderData(psIterator->pcNode);

		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::StartUnsafeIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size* piDataSize)
{
	return StartUnsafeIteration(psIterator, NULL, NULL, miMaxDataSize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::UnsafeIterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size* piDataSize)
{
	return UnsafeIterate(psIterator, NULL, NULL, miMaxDataSize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::StepNext(SIndexTreeMemoryUnsafeIterator* psIterator)
{
	CIndexTreeNodeMemory*	pcChild;
	void*					pvData;
	CIndexTreeNodeMemory*	pcParent;
	size						iChildCount;
	size						iDownParentCount;

	for (iChildCount = 0;; iChildCount++)
	{
		pcChild = psIterator->pcNode->Get(psIterator->iIndex);

		if (pcChild != NULL)
		{
			psIterator->pcNode = pcChild;
			psIterator->iIndex = pcChild->GetFirstIndex();

			pvData = pcChild->GetDataPtr();
			if (pvData != NULL)
			{
				return true; 
			}
		}
		else
		{
			for (iDownParentCount = 0;; iDownParentCount++)
			{
				psIterator->iIndex++;
				if (psIterator->iIndex > psIterator->pcNode->GetLastIndex())
				{
					pcParent = (CIndexTreeNodeMemory*)psIterator->pcNode->GetParent();
					if (pcParent == NULL)
					{
						return false;
					}
					psIterator->iIndex = pcParent->FindIndex(psIterator->pcNode);
					psIterator->pcNode = pcParent;
				}
				else
				{
					break;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::ValidateSize(void)
{
	size iSizeFromRecursion;

	iSizeFromRecursion = RecurseSize(mpcRoot);

	if (miSize != iSizeFromRecursion)
	{
		return false;
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::ValidateIndexTree(void)
{
	bool bResult;

	bResult = ValidateLimits();
	bResult &= ValidateNodeTree();
	bResult &= ValidateSize();
	bResult &= ValidateParentIndex();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::GetUserMemorySize(void)
{
	return 0; //mcMalloc.AllocatedUserSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Print(CChars* pszDest, bool bShowFlags, bool bShowSize)
{
	CChars				sz;

	sz.Init();
	sz.Append("= [IndexTreeMemory]");
	if (bShowSize)
	{
		sz.Append("[");
		sz.Append((uint64)this->GetUserMemorySize());
		sz.Append("] ");
	}

	sz.Append('=', mpcRoot->NumIndexes() * 2 - sz.Length() + 18);
	pszDest->AppendNewLine(sz);
	sz.Kill();

	PrintChildren(pszDest, bShowFlags, bShowSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Dump(void)
{
	CChars				sz;

	sz.Init();
	Print(&sz, true, true);
	sz.Dump();
	sz.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::PrintChildren(CChars* pszDest, bool bShowFlags, bool bShowSize)
{
	CIndexTreeRecursor	cCursor;

	cCursor.Init(mpcRoot);

	DebugNodeChildren(pszDest, mpcRoot, -1, bShowFlags, bShowSize);
	RecurseDump(pszDest, &cCursor, bShowFlags, bShowSize);

	cCursor.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AppendIndexTreeMemoryNodeDescrition(CChars* psz, CIndexTreeNodeMemory* pcCurrent, size uIndexFromParent)
{
	char	szArrow[] = " -> ";
	bool	bHasChildren;
	bool	bHasObject;

	if (uIndexFromParent == -1)
	{
		psz->Append("  root -> ");
	}
	else
	{
		if ((uIndexFromParent >= 32) && (uIndexFromParent <= 126))
		{
			psz->Append("  ");
			psz->Append((char)uIndexFromParent);
		}
		else
		{
			if (uIndexFromParent < 10)
			{
				psz->Append("  ");
			}
			else if (uIndexFromParent < 100)
			{
				psz->Append(" ");
			}
			psz->Append(uIndexFromParent);
		}

		bHasObject = pcCurrent->HasData();
		if (bHasObject)
		{
			psz->Append("(X)");
		}
		else
		{
			psz->Append("   ");
		}

		bHasChildren = pcCurrent->HasNodes();
		if (bHasChildren)
		{
			psz->Append(szArrow);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::DebugNodeChildren(CChars* pszDest, CIndexTreeNodeMemory* pcCurrent, size uIndexFromParent, bool bShowFlags, bool bShowSize)
{
	CChars	szMemory;
	CChars	sz;
	CChars	szFlags;

	szMemory.Init();
	AppendIndexTreeMemoryNodeDescrition(&szMemory, pcCurrent, uIndexFromParent);

	szFlags.Init();
	if (bShowFlags)
	{
		szFlags.Append(" (");
		pcCurrent->GetFlagsString(&szFlags);
		szFlags.Append(")");
	}
	if (bShowSize)
	{
		szFlags.Append(" [");
		szFlags.Append(pcCurrent->CalculateRequiredNodeSizeForCurrent());
		szFlags.Append("]");
	}

	sz.Init();
	sz.Append(" Memory: ");
	sz.Append(szMemory);
	sz.Append(&szFlags);
	pszDest->AppendNewLine(sz);
	sz.Kill();

	szFlags.Kill();
	szMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, bool bShowFlags, bool bShowSize)
{
	CIndexTreeNodeMemory*	pcNode;
	size						i;
	size						iKeySize;
	CIndexTreeNodeMemory*	pcChild;
	CStackMemory<32>		cStack;
	uint8*					pvKey;
	CChars					szKey;
	bool					bHasNodes;
	bool					bHasData;
	size					iFirst;
	size					iLast;

	pcNode = (CIndexTreeNodeMemory*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		bHasNodes = pcNode->HasNodes();
		bHasData = pcNode->HasData();
		if (bHasData)
		{
			iKeySize = GetNodeKeySize(pcNode);
			pvKey = (uint8*)cStack.Init(iKeySize);
			GetNodeKey(pcNode, pvKey, iKeySize);

			szKey.Init("Key: ------------- [");
			szKey.AppendData2((const char*)pvKey, iKeySize);
			szKey.Append("] -------------");
			pszDest->AppendNewLine(szKey);
			szKey.Kill();

			DebugKey(pszDest, pvKey, iKeySize, true, bShowFlags, bShowSize, false);
			cStack.Kill();
		}

		if (bHasNodes)
		{
			iFirst = pcNode->GetFirstIndex();
			iLast = pcNode->GetLastIndex();

			for (i = iFirst; i <= iLast; i++)
			{
				pcChild = pcNode->Get(i);
				pcCursor->Push(pcChild, (uint8)i);
				RecurseDump(pszDest, pcCursor, bShowFlags, bShowSize);
			}
		}
	}
	pcCursor->Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::DebugKey(CChars* pszDest, void* pvKey, size iKeySize, bool bSkipRoot, bool bShowFlags, bool bShowSize, bool bKeyAlreadyReversed)
{
	CIndexTreeNodeMemory*	pcCurrent;
	size					c;
	size					i;
	bool					bExecute;

	pcCurrent = mpcRoot;

	if (!bSkipRoot)
	{
		DebugNodeChildren(pszDest, pcCurrent, -1, bShowFlags, bShowSize);
	}

	if (bKeyAlreadyReversed)
	{
		i = 0;
		bExecute = i < iKeySize;
	}
	else
	{
		bExecute = StartKey(&i, iKeySize);
	}

	while (bExecute)
	{
		c = ((uint8*)pvKey)[i];
		if (pcCurrent != NULL)
		{
			pcCurrent = DebugNode(pszDest, pcCurrent, c, bShowFlags, bShowSize);
		}
		else
		{
			pszDest->Append("Broken Node!");
			pszDest->AppendNewLine();
		}

		if (bKeyAlreadyReversed)
		{
			i++;
			bExecute = i < iKeySize;
		}
		else
		{
			bExecute = LoopKey(&i, iKeySize);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::DebugNode(CChars* pszDest, CIndexTreeNodeMemory* pcParent, size uiIndexInParent, bool bShowFlags, bool bShowSize)
{
	CIndexTreeNodeMemory* pcChild;

	pcChild = pcParent->Get(uiIndexInParent);
	if (pcChild)
	{
		DebugNodeChildren(pszDest, pcChild, uiIndexInParent, bShowFlags, bShowSize);
		return pcChild;
	}
	else
	{
		//Data for key does not exist.
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::RecurseSize(void)
{
	return RecurseSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::RecurseSize(CIndexTreeNodeMemory* pcNode)
{
	size						i;
	CIndexTreeNodeMemory*	pcChild;

	size count = 0;

	if (pcNode != NULL)
	{
		if (pcNode->HasData())
		{
			count++;
		}

		for (i = 0; i < pcNode->NumIndexes(); i++)
		{
			pcChild = pcNode->GetNode(i);
			count += RecurseSize(pcChild);
		}
	}
	return count;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::ByteSize(void)
{
	return RecurseByteSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::RecurseByteSize(CIndexTreeNodeMemory* pcNode)
{
	size					i;
	CIndexTreeNodeMemory*	pcChild;
	size					uiSize;

	uiSize = 0;
	if (pcNode != NULL)
	{
		uiSize += pcNode->CalculateRequiredNodeSizeForCurrent();

		for (i = 0; i < pcNode->NumIndexes(); i++)
		{
			pcChild = pcNode->GetNode(i);
			uiSize += RecurseByteSize(pcChild);
		}
	}
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::ValidateLimits(void)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateLimits(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::RecurseValidateLimits(CIndexTreeRecursor* pcCursor)
{
	size					i;
	CIndexTreeNodeMemory*	pcChild;
	size					iFirst;
	size					iLast;
	bool					bResult;
	CIndexTreeNodeMemory*	pcFirst;
	CIndexTreeNodeMemory*	pcLast;
	CIndexTreeNodeMemory*	pcNode;

	pcNode = (CIndexTreeNodeMemory*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasNodes())
		{
			iFirst = pcNode->GetFirstIndex();
			iLast = pcNode->GetLastIndex();

			if (pcNode != mpcRoot)
			{
				if (!pcNode->ContainsIndex(iFirst))
				{
					pcCursor->GenerateBad();
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] did not contain first index [", IntToString(iFirst), "].", NULL);
					return false;
				}
				if (!pcNode->ContainsIndex(iLast))
				{
					pcCursor->GenerateBad();
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] did not contain last index [", IntToString(iLast), "].", NULL);
					return false;
				}

				pcFirst = pcNode->Get(iFirst);
				if (pcFirst == NULL)
				{
					pcCursor->GenerateBad();
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] first child [", IntToString(iFirst), "] was NULL.", NULL);
					return false;
				}

				pcLast = pcNode->Get(iLast);
				if (pcLast == NULL)
				{
					pcCursor->GenerateBad();
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] last child [", IntToString(iFirst), "] was NULL.", NULL);
					return false;
				}
			}

			for (i = iFirst; i <= iLast; i++)
			{
				pcChild = pcNode->Get(i);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseValidateLimits(pcCursor);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}
		}
	}
	pcCursor->Pop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::ValidateParentIndex(void)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateParentIndex(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeMemory*	pcNode;
	size						i;
	CIndexTreeNodeMemory*	pcChild;
	bool					bResult;
	CIndexTreeNodeMemory*	pcChildsParent;
	size					uiIndexInParent;

	pcNode = (CIndexTreeNodeMemory*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = pcNode->Get(i);
				if (pcChild != NULL)
				{
					pcChildsParent = (CIndexTreeNodeMemory*)pcChild->GetParent();
					if (pcChildsParent != pcNode)
					{
						pcCursor->GenerateBad();
						gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] points to the wrong parent node.", NULL);
						return false;
					}

					uiIndexInParent = pcChild->GetIndexInParent();
					if (uiIndexInParent != i)
					{
						pcCursor->GenerateBad();
						gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] points to the wrong parent node.", NULL);
						return false;
					}
				}
			}

			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = pcNode->Get(i);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseValidateParentIndex(pcCursor);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}

		}
	}
	pcCursor->Pop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::ValidateNodeTree(void)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateNodeTree(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::RecurseValidateNodeTree(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeMemory*	pcNode;
	size					i;
	CIndexTreeNodeMemory*	pcChild;
	bool					bResult;
	CIndexTreeNodeMemory*	pcBackNode;
	size					iCount;

	pcNode = (CIndexTreeNodeMemory*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->GetIndexTree() != this)
		{
			pcCursor->GenerateBad();
			gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] does not point back into its Index Tree.", NULL);
			return false;
		}

		pcBackNode = pcNode;
		iCount = 0;
		while (pcBackNode != mpcRoot)
		{
			pcBackNode = pcBackNode->GetParent();
			iCount++;
			if (iCount > MAX_KEY_SIZE)
			{
				pcCursor->GenerateBad();
				gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] does not point back to its root node.", NULL);
				return false;
			}
		}

		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = pcNode->Get(i);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseValidateNodeTree(pcCursor);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}

		}
	}
	pcCursor->Pop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemory::HasKey(void* pvKey, size iKeySize)
{
	CIndexTreeNodeMemory* pcNode;

	pcNode = GetNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return false;
	}

	if (pcNode->HasData())
	{
		HasKeyReorderData(pcNode);

		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::GetDataSize(void* pvKey, size iKeySize)
{
	CIndexTreeNodeMemory* pcNode;

	pcNode = GetNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return 0;
	}

	GetReorderData(pcNode);

	return pcNode->GetDataSize();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::NumAllocatedNodes(void)
{
	return RecurseNumNodes(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::RecurseNumNodes(CIndexTreeNodeMemory* pcNode)
{
	size						i;
	CIndexTreeNodeMemory*	pcChild;

	if (pcNode != NULL)
	{
		size count = 1;
		for (i = 0; i < pcNode->NumIndexes(); i++)
		{
			pcChild = pcNode->GetNode(i);
			count += RecurseNumNodes(pcChild);
		}
		return count;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::CountListSize(void)
{
	return RecurseCountListSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeMemory::RecurseCountListSize(CIndexTreeNodeMemory* pcNode)
{
	size					i;
	CIndexTreeNodeMemory*		pcChild;

	if (pcNode != NULL)
	{
		size count = pcNode->NumIndexes();
		for (i = 0; i < pcNode->NumIndexes(); i++)
		{
			pcChild = pcNode->Get(i);
			count += RecurseCountListSize(pcChild);
		}
		return count;
	}
	else
	{
		return 0;
	}
}

