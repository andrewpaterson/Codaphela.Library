#include "Numbers.h"
#include "GlobalMemory.h"
#include "LifeCycle.h"
#include "Logger.h"
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
	Init(pcConfig->GetMalloc(), pcConfig->GetKeyReverse(), pcConfig->GetMaxDataSize(), pcConfig->GetMaxKeySize(), pcConfig->GetDataOrderer()); 
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
void* CIndexTreeMemory::Get(void* pvKey, int iKeySize, size_t* piDataSize)
{
	CIndexTreeNodeMemory*	pcNode;
	void*					pv;
	uint16			uiDataSize;

	pcNode = GetNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return NULL;
	}
	else
	{
		if (pcNode->HasData())
		{
			GetReorderData(pcNode);

			uiDataSize = pcNode->GetDataSize();
			SafeAssign(piDataSize, uiDataSize);
			pv = pcNode->GetDataPtr();
			return pv;
		}
		else
		{
			SafeAssign(piDataSize, 0);
			return NULL;
		}
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

	if (!pcCurrent->HasData())
	{
		miSize++;
	}

	bNewNode = FALSE;
	if (!pcCurrent->HasData())
	{
		bNewNode = TRUE;
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
	uint16			uiOldDataSize;

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

	cMalloc.Init(mpcMalloc, mcMallocLife.MustFree(), mcMallocLife.MustKill());
	cDataOrderer.Init(mpcDataOrderer, mcDataOrdererLife.MustFree(), mcDataOrdererLife.MustKill());
	cConfig.Init(cMalloc, meReverseKey, miMaxDataSize, miMaxKeySize, cDataOrderer);
	return cConfig.Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::Write(CFileWriter* pcFileWriter)
{
	SIndexTreeMemoryIterator	sIter;
	void*						pvData;
	size_t						iDataSize;
	size_t						iKeySize;
	BOOL						bResult;
	char						acKey[MAX_KEY_SIZE];
	int							iCount;

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
BOOL CIndexTreeMemory::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	int				iCount;
	int				i;
	char			acKey[1024];
	int				iKeySize;
	int				iDataSize;
	void*			pvData;

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
BOOL CIndexTreeMemory::StartIteration(SIndexTreeMemoryIterator* psIterator, void* pvKey, int* piKeySize, void* pvData, size_t* piDataSize)
{
	psIterator->pcNode = mpcRoot;
	psIterator->iIndex = mpcRoot->GetFirstIndex();

	return Iterate(psIterator, pvKey, piKeySize, pvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::Iterate(SIndexTreeMemoryIterator* psIterator, void* pvKey, int* piKeySize, void* pvData, size_t* piDataSize)
{
	int		iKeySize;

	if (StepNext(psIterator))
	{
		*piDataSize = psIterator->pcNode->GetDataSize();
		memcmp_fast(pvData, psIterator->pcNode->GetDataPtr(), *piDataSize);
		iKeySize = GetNodeKey(psIterator->pcNode, (char*)pvKey, MAX_KEY_SIZE);
		*piKeySize = iKeySize;

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
BOOL CIndexTreeMemory::StartUnsafeIteration(SIndexTreeMemoryIterator* psIterator, void** ppvData, size_t* piDataSize)
{
	psIterator->pcNode = mpcRoot;
	psIterator->iIndex = mpcRoot->GetFirstIndex();

	return UnsafeIterate(psIterator, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::UnsafeIterate(SIndexTreeMemoryIterator* psIterator, void** ppvData, size_t* piDataSize)
{
	if (StepNext(psIterator))
	{
		if (ppvData)
		{
			*ppvData = psIterator->pcNode->GetDataPtr();
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->GetDataSize();
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
BOOL CIndexTreeMemory::StepNext(SIndexTreeMemoryIterator* psIterator)
{
	CIndexTreeNodeMemory*	pcChild;
	void*					pvData;
	CIndexTreeNodeMemory*	pcParent;

	for (;;)
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
			for (;;)
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
	bResult &= ValidateSize();
	bResult &= ValidateParentIndex();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Dump(void)
{
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
	uint16			uiSize;
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

					pcCursor->Push(pcChild, i);
					bResult = RecurseValidateParentIndex(pcCursor);
					if (!bResult)
					{
						pcCursor->Pop();
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
		return FALSE;
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

