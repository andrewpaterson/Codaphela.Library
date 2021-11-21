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
void CIndexTreeMemory::Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int	iMaxKeySize)
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
void CIndexTreeMemory::Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int	iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
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
BOOL CIndexTreeMemory::Kill(void)
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
	int						i;
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
CIndexTreeNodeMemory* CIndexTreeMemory::AllocateNode(CIndexTreeNodeMemory* pcParent, unsigned char uiIndexInParent)
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
CIndexTreeNodeMemory* CIndexTreeMemory::GetNode(void* pvKey, int iKeySize)
{
	CIndexTreeNodeMemory*	pcCurrent;
	int						i;
	unsigned char			c;
	BOOL					bExecute;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((unsigned char*)pvKey)[i];

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
CIndexTreeNodeMemory* CIndexTreeMemory::GetNodeFromLongestPartialKey(void* pvKey, int iKeySize)
{
	CIndexTreeNodeMemory*	pcCurrent;
	int						i;
	unsigned char			c;
	BOOL					bExecute;
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
		c = ((unsigned char*)pvKey)[i];

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
BOOL CIndexTreeMemory::Get(void* pvKey, int iKeySize, void* pvDestData, size_t* puiDataSize, size_t uiMaxDataSize)
{
	CIndexTreeNodeMemory*	pcNode;
	uint16					uiDataSize;

	pcNode = GetNode(pvKey, iKeySize);
	return GetNodeData(pcNode, uiDataSize, puiDataSize, pvDestData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::GetLongestPartial(void* pvKey, int iKeySize, void* pvDestData, size_t* puiDataSize, size_t uiMaxDataSize)
{
	CIndexTreeNodeMemory*	pcNode;
	uint16					uiDataSize;

	pcNode = GetNodeFromLongestPartialKey(pvKey, iKeySize);
	return GetNodeData(pcNode, uiDataSize, puiDataSize, pvDestData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::GetNodeData(CIndexTreeNodeMemory* pcNode, uint16& uiDataSize, size_t* puiDataSize, void* pvDestData, size_t uiMaxDataSize)
{
	void* pvData;

	if (pcNode == NULL)
	{
		return FALSE;
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

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeMemory::Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize)
{
	CIndexTreeNodeMemory*	pcCurrent;
	uint16					uiDataSize;
	BOOL					bNewNode;

	ReturnNullOnFalse(ValidatePut(iKeySize, iDataSize));

	uiDataSize = (uint16)iDataSize;

	pcCurrent = GetOrAllocateKey(pvKey, iKeySize);

	bNewNode = FALSE;
	if (!pcCurrent->HasData())
	{
		miSize++;
		bNewNode = TRUE;
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
CIndexTreeNodeMemory* CIndexTreeMemory::GetOrAllocateKey(void* pvKey, int iKeySize)
{
	CIndexTreeNodeMemory*	pcCurrent;
	unsigned char			c;
	BOOL					bExecute;
	int						i;

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((char*)pvKey)[i];
		pcCurrent = SetOldWithCurrent(pcCurrent, c);
		bExecute = LoopKey(&i, iKeySize);
	}
	return pcCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::SetNodeData(CIndexTreeNodeMemory* pcCurrent, void* pvData, uint16 uiDataSize)
{
	CIndexTreeNodeMemory*	pcReallocatedCurrent;
	uint16					uiOldDataSize;

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
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, unsigned char uiIndex)
{
	CIndexTreeNodeMemory*	pcOldNode;
	size_t					tNewNodeSize;

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
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForLargerData(CIndexTreeNodeMemory* pcNode, void* pvData, uint16 uiDataSize)
{
	size_t					tNewNodeSize;
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
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForSmallerData(CIndexTreeNodeMemory* pcNode, void* pvData, uint16 uiDataSize)
{
	size_t					tNewNodeSize;
	CIndexTreeNodeMemory*	pcOldNode;
	uint16			uiOriginalSize;

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
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForRemove(CIndexTreeNodeMemory* pcNode, size_t tNewNodeSize)
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
			pcNode->RemapDataLinks(pcOldNode, pcNode);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, unsigned char uiIndexInParent)
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
BOOL CIndexTreeMemory::Remove(void* pvKey, int iKeySize)
{
	unsigned char			c;
	CIndexTreeNodeMemory*	pcCurrent;
	int						i;
	BOOL					bExecute;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return FALSE;
	}

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((char*)pvKey)[i];
		pcCurrent = pcCurrent->Get(c);
		if (pcCurrent == NULL)
		{
			return FALSE;
		}
		bExecute = LoopKey(&i, iKeySize);
	}

	return Remove(pcCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::Remove(CIndexTreeNodeMemory*	pcCurrent)
{
	unsigned char			c;
	CIndexTreeNodeMemory*	pcParent;
	CIndexTreeNodeMemory*	pcNode;
	BOOL					bResizeNode;
	size_t					tNewNodeSize;
	uint16			uiOldDataSize;

	uiOldDataSize = pcCurrent->GetDataSize();
	if (uiOldDataSize == 0)
	{
		return FALSE;
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
	return TRUE;
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
	int						i;
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
BOOL CIndexTreeMemory::WriteConfig(CFileWriter* pcFileWriter)
{
	CIndexTreeMemoryConfig				cConfig;
	CLifeInit<CMallocator>				cMalloc;
	CLifeInit<CIndexTreeDataOrderer>	cDataOrderer;
	BOOL								bResult;

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
BOOL CIndexTreeMemory::Write(CFileWriter* pcFileWriter)
{
	SIndexTreeMemoryUnsafeIterator		sIter;
	void*								pvData;
	size_t								iDataSize;
	size_t								iKeySize;
	BOOL								bResult;
	char								acKey[MAX_KEY_SIZE];
	int									iCount;

	if (!WriteConfig(pcFileWriter))
	{
		return FALSE;
	}

	if (!pcFileWriter->WriteInt(miSize))
	{
		return FALSE;
	}

	iCount = 0;
	bResult = StartUnsafeIteration(&sIter, &pvData, &iDataSize);
	while (bResult)
	{
		iKeySize = GetKey(pvData, acKey, MAX_KEY_SIZE);
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

		bResult = UnsafeIterate(&sIter, &pvData, &iDataSize);
		iCount ++;
	}
	return miSize == iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::ReadConfig(CFileReader* pcFileReader)
{
	CIndexTreeMemoryConfig				cConfig;
	BOOL								bResult;

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
BOOL CIndexTreeMemory::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	int				iCount;
	int				i;
	char			acKey[1024];
	int				iKeySize;
	int				iDataSize;
	void*			pvData;

	if (!ReadConfig(pcFileReader))
	{
		return FALSE;
	}

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
		pvData = Put(acKey, iKeySize, NULL, iDataSize);
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
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeMemory::NumElements(void)
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
int CIndexTreeMemory::GetKey(void* pvData, char* pvDestKey, int iDestKeySize)
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
int CIndexTreeMemory::GetNodeKeySize(CIndexTreeNode* pcNode)
{
	int				iKeySize;

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
int CIndexTreeMemory::GetNodeKey(CIndexTreeNode* pcNode, char* pvDestKey, int iDestKeySize)
{
	int				iKeySize;
	int				iLength;

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
			pvDestKey[iKeySize] = pcNode->GetIndexInParent();
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
		return FALSE;
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
uint16 CIndexTreeMemory::GetNodeDataSize(CIndexTreeNode* pcNode)
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
uint16 CIndexTreeMemory::GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, int iDestDataSize)
{
	int				iDataSize;
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
BOOL CIndexTreeMemory::StartIteration(SIndexTreeMemoryIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	size_t							iDataSize;
	BOOL							bResult;

	memset(psIterator->pvKey, 0, MAX_KEY_SIZE);
	psIterator->iIndex = mpcRoot->GetFirstIndex();
	psIterator->iKeyLength = 0;

	bResult = StartUnsafeIteration(&sIter, psIterator->pvKey, &psIterator->iKeyLength, MAX_KEY_SIZE, NULL, NULL);
	psIterator->iIndex = sIter.iIndex;

	if (bResult)
	{
		CopyData((char*)pvKey, psIterator->pvKey, psIterator->iKeyLength, iMaxKeySize);
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
BOOL CIndexTreeMemory::Iterate(SIndexTreeMemoryIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	size_t							iDataSize;
	BOOL							bResult;

	sIter.iIndex = psIterator->iIndex;
	sIter.pcNode = GetNode(psIterator->pvKey, psIterator->iKeyLength);

	if (!sIter.pcNode)
	{
		return FALSE;
	}

	bResult = UnsafeIterate(&sIter, (char*)psIterator->pvKey, &psIterator->iKeyLength, MAX_KEY_SIZE, NULL, NULL);
	psIterator->iIndex = sIter.iIndex;

	if (bResult)
	{
		CopyData((char*)pvKey, psIterator->pvKey, psIterator->iKeyLength, iMaxKeySize);
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
BOOL CIndexTreeMemory::StartUnsafeIteration(SIndexTreeMemoryUnsafeIterator* psIterator, char* pvKey, int* piKeySize, int iMaxKeySize, void** ppvData, size_t* piDataSize)
{
	psIterator->pcNode = mpcRoot;
	psIterator->iIndex = mpcRoot->GetFirstIndex();

	return UnsafeIterate(psIterator, pvKey, piKeySize, iMaxKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::UnsafeIterate(SIndexTreeMemoryUnsafeIterator* psIterator, char* pvKey, int* piKeySize, int iMaxKeySize, void** pvData, size_t* piDataSize)
{
	void*	pvDataTemp;
	int		iKeySize;

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

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::StartUnsafeIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* piDataSize)
{
	return StartUnsafeIteration(psIterator, NULL, NULL, miMaxDataSize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::UnsafeIterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* piDataSize)
{
	return UnsafeIterate(psIterator, NULL, NULL, miMaxDataSize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::StepNext(SIndexTreeMemoryUnsafeIterator* psIterator)
{
	CIndexTreeNodeMemory*	pcChild;
	void*					pvData;
	CIndexTreeNodeMemory*	pcParent;
	int						iChildCount;
	int						iDownParentCount;

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
				return TRUE; 
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
						return FALSE;
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
BOOL CIndexTreeMemory::ValidateSize(void)
{
	int iSizeFromRecursion;

	iSizeFromRecursion = RecurseSize(mpcRoot);

	if (miSize != iSizeFromRecursion)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::ValidateIndexTree(void)
{
	BOOL bResult;

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
size_t CIndexTreeMemory::GetUserMemorySize(void)
{
	return 0; //mcMalloc.AllocatedUserSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Print(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize)
{
	CChars				sz;

	sz.Init();
	sz.Append("= [IndexTreeMemory]");
	if (bShowSize)
	{
		sz.Append("[");
		sz.Append((unsigned long long int)this->GetUserMemorySize());
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
	Print(&sz, TRUE, TRUE);
	sz.Dump();
	sz.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::PrintChildren(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize)
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
void AppendIndexTreeMemoryNodeDescrition(CChars* psz, CIndexTreeNodeMemory* pcCurrent, int uIndexFromParent)
{
	char	szArrow[] = " -> ";
	BOOL	bHasChildren;
	BOOL	bHasObject;

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
void CIndexTreeMemory::DebugNodeChildren(CChars* pszDest, CIndexTreeNodeMemory* pcCurrent, int uIndexFromParent, BOOL bShowFlags, BOOL bShowSize)
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
void CIndexTreeMemory::RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, BOOL bShowFlags, BOOL bShowSize)
{
	CIndexTreeNodeMemory*	pcNode;
	int						i;
	int						iKeySize;
	CIndexTreeNodeMemory*	pcChild;
	CStackMemory<32>		cStack;
	char*					pvKey;
	CChars					szKey;
	BOOL					bHasNodes;
	BOOL					bHasData;
	unsigned char			iFirst;
	unsigned char			iLast;

	pcNode = (CIndexTreeNodeMemory*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		bHasNodes = pcNode->HasNodes();
		bHasData = pcNode->HasData();
		if (bHasData)
		{
			iKeySize = GetNodeKeySize(pcNode);
			pvKey = (char*)cStack.Init(iKeySize);
			GetNodeKey(pcNode, pvKey, iKeySize);

			szKey.Init("Key: ------------- [");
			szKey.AppendData2((const char*)pvKey, iKeySize);
			szKey.Append("] -------------");
			pszDest->AppendNewLine(szKey);
			szKey.Kill();

			DebugKey(pszDest, pvKey, iKeySize, TRUE, bShowFlags, bShowSize, FALSE);
			cStack.Kill();
		}

		if (bHasNodes)
		{
			iFirst = pcNode->GetFirstIndex();
			iLast = pcNode->GetLastIndex();

			for (i = iFirst; i <= iLast; i++)
			{
				pcChild = pcNode->Get(i);
				pcCursor->Push(pcChild, i);
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
void CIndexTreeMemory::DebugKey(CChars* pszDest, void* pvKey, int iKeySize, BOOL bSkipRoot, BOOL bShowFlags, BOOL bShowSize, BOOL bKeyAlreadyReversed)
{
	CIndexTreeNodeMemory*	pcCurrent;
	unsigned char			c;
	int						i;
	BOOL					bExecute;

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
		c = ((unsigned char*)pvKey)[i];
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
CIndexTreeNodeMemory* CIndexTreeMemory::DebugNode(CChars* pszDest, CIndexTreeNodeMemory* pcParent, int uiIndexInParent, BOOL bShowFlags, BOOL bShowSize)
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
int CIndexTreeMemory::RecurseSize(void)
{
	return RecurseSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeMemory::RecurseSize(CIndexTreeNodeMemory* pcNode)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;

	int count = 0;

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
size_t CIndexTreeMemory::ByteSize(void)
{
	return RecurseByteSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeMemory::RecurseByteSize(CIndexTreeNodeMemory* pcNode)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	uint16					uiSize;
	size_t					tSize;

	tSize = 0;
	if (pcNode != NULL)
	{
		tSize += pcNode->CalculateRequiredNodeSizeForCurrent();
		uiSize = pcNode->GetDataSize();

		for (i = 0; i < pcNode->NumIndexes(); i++)
		{
			pcChild = pcNode->GetNode(i);
			tSize += RecurseByteSize(pcChild);
		}
	}
	return tSize;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::ValidateLimits(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateLimits(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::RecurseValidateLimits(CIndexTreeRecursor* pcCursor)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	unsigned char			iFirst;
	unsigned char			iLast;
	BOOL					bResult;
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
					return FALSE;
				}
				if (!pcNode->ContainsIndex(iLast))
				{
					pcCursor->GenerateBad();
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] did not contain last index [", IntToString(iLast), "].", NULL);
					return FALSE;
				}

				pcFirst = pcNode->Get(iFirst);
				if (pcFirst == NULL)
				{
					pcCursor->GenerateBad();
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] first child [", IntToString(iFirst), "] was NULL.", NULL);
					return FALSE;
				}

				pcLast = pcNode->Get(iLast);
				if (pcLast == NULL)
				{
					pcCursor->GenerateBad();
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] last child [", IntToString(iFirst), "] was NULL.", NULL);
					return FALSE;
				}
			}

			for (i = iFirst; i <= iLast; i++)
			{
				pcChild = pcNode->Get(i);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateLimits(pcCursor);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
				}
			}
		}
	}
	pcCursor->Pop();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::ValidateParentIndex(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateParentIndex(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeMemory*	pcNode;
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	BOOL					bResult;
	CIndexTreeNodeMemory*	pcChildsParent;
	unsigned char			uiIndexInParent;


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
						return FALSE;
					}

					uiIndexInParent = pcChild->GetIndexInParent();
					if (uiIndexInParent != i)
					{
						pcCursor->GenerateBad();
						gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] points to the wrong parent node.", NULL);
						return FALSE;
					}
				}
			}

			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = pcNode->Get(i);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateParentIndex(pcCursor);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
				}
			}

		}
	}
	pcCursor->Pop();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::ValidateNodeTree(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateNodeTree(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::RecurseValidateNodeTree(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeMemory*	pcNode;
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	BOOL					bResult;
	CIndexTreeNodeMemory*	pcBackNode;
	int						iCount;

	pcNode = (CIndexTreeNodeMemory*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->GetIndexTree() != this)
		{
			pcCursor->GenerateBad();
			gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] does not point back into its Index Tree.", NULL);
			return FALSE;
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
				return FALSE;
			}
		}

		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = pcNode->Get(i);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateNodeTree(pcCursor);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
				}
			}

		}
	}
	pcCursor->Pop();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::HasKey(void* pvKey, int iKeySize)
{
	CIndexTreeNodeMemory* pcNode;

	pcNode = GetNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return FALSE;
	}

	if (pcNode->HasData())
	{
		HasKeyReorderData(pcNode);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CIndexTreeMemory::GetDataSize(void* pvKey, int iKeySize)
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
int CIndexTreeMemory::NumAllocatedNodes(void)
{
	return RecurseNumNodes(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeMemory::RecurseNumNodes(CIndexTreeNodeMemory* pcNode)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;

	if (pcNode != NULL)
	{
		int count = 1;
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
int CIndexTreeMemory::CountListSize(void)
{
	return RecurseCountListSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeMemory::RecurseCountListSize(CIndexTreeNodeMemory* pcNode)
{
	int					i;
	CIndexTreeNodeMemory*		pcChild;

	if (pcNode != NULL)
	{
		int count = pcNode->NumIndexes();
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

