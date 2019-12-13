#include "Numbers.h"
#include "GlobalMemory.h"
#include "Logger.h"
#include "IndexTreeMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Init(CMallocator* pcMalloc)
{
	CIndexTree::Init(pcMalloc, sizeof(CIndexTreeNodeMemory), sizeof(CIndexTreeNodeMemory*));
	mpcRoot = AllocateRoot();
	miSize = 0;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::FakeInit(void)
{
	//This exists so that TreeNodes can be tested without a full tree.  All they need to do is query the size of their child nodes.
	//Kill should not be called.
	CIndexTree::Init(NULL, sizeof(CIndexTreeNodeMemory), sizeof(CIndexTreeNodeMemory*));
	mpcRoot = NULL;
	miSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::Kill(void)
{
	RecurseKill(mpcRoot);
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
	CIndexTreeNodeMemory*		psNode;

	psNode = DataGetHeader<CIndexTreeNodeMemory, void>(pvData);
	return psNode;
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

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	for (i = 0; i < iKeySize; i++)
	{
		c = ((unsigned char*)pvKey)[i];

		pcCurrent = pcCurrent->Get(c);
		if (pcCurrent == NULL)
		{
			return NULL;
		}
	}
	return pcCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeMemory::Get(void* pvKey, int iKeySize, unsigned short* puiDataSize)
{
	CIndexTreeNodeMemory*	pcNode;
	void*					pv;
	unsigned short			uiDataSize;

	pcNode = GetNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return NULL;
	}
	else
	{
		uiDataSize = pcNode->ObjectSize();
		if (puiDataSize)
		{
			*puiDataSize = uiDataSize;
		}

		if (uiDataSize == 0)
		{
			return NULL;
		}

		pv = pcNode->GetObjectPtr();
		return pv;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeMemory::Put(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize)
{
	CIndexTreeNodeMemory*	pcCurrent;
	CIndexTreeNodeMemory*	pcReallocatedCurrent;
	unsigned char			c;
	BOOL					bResult;
	unsigned short			uiOriginalSize;

	if (iKeySize == 0)
	{
		return FALSE;
	}

	pcCurrent = mpcRoot;

	for (int i = 0; i < iKeySize; i++)
	{
		c = ((char*)pvKey)[i];
		pcCurrent = SetOldWithCurrent(pcCurrent, c);
	}

	if (!pcCurrent->HasObject())
	{
		miSize++;
	}

	if (pcCurrent->ObjectSize() <= uiDataSize)
	{ 
		pcReallocatedCurrent = ReallocateNodeForLargerData(pcCurrent, uiDataSize);
		bResult = pcReallocatedCurrent->SetObject(pvObject, uiDataSize);
	}
	else
	{
		uiOriginalSize = pcCurrent->ObjectSize();
		bResult = pcCurrent->SetObject(pvObject, uiDataSize);
		pcReallocatedCurrent = ReallocateNodeForSmallerData(pcCurrent, uiOriginalSize);
	}

	if (pcCurrent != pcReallocatedCurrent)
	{
		pcReallocatedCurrent->SetChildsParent();
	}

	if (bResult)
	{
		return pcReallocatedCurrent->GetObjectPtr();
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
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, unsigned char uiIndex)
{
	CIndexTreeNodeMemory*	pcOldNode;
	size_t					tNewNodeSize;
	size_t					tOldNodeSize;

	if (pcNode->ContainsIndex(uiIndex))
	{
		return pcNode;
	}

	tOldNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();
	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForIndex(uiIndex);

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeMemory*)Realloc(pcNode, tNewNodeSize, tOldNodeSize);
	pcNode->Contain(uiIndex);

	RemapChildParents(pcOldNode, pcNode);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForLargerData(CIndexTreeNodeMemory* pcNode, unsigned short uiDataSize)
{
	size_t					tNewNodeSize;
	size_t					tOldNodeSize;

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForData(uiDataSize);
	tOldNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();

	return ReallocateNodeForData(pcNode, tNewNodeSize, tOldNodeSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForSmallerData(CIndexTreeNodeMemory* pcNode, unsigned short uiOriginalSize)
{
	size_t					tNewNodeSize;
	size_t					tOldNodeSize;

	tOldNodeSize = pcNode->CalculateRequiredNodeSizeForData(uiOriginalSize);
	 tNewNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();

	return ReallocateNodeForData(pcNode, tNewNodeSize, tOldNodeSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::ReallocateNodeForData(CIndexTreeNodeMemory* pcNode, size_t tNewNodeSize, size_t tOldNodeSize)
{
	CIndexTreeNodeMemory*	pcOldNode;

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeMemory*)Realloc(pcNode, tNewNodeSize, tOldNodeSize);

	RemapChildParents(pcOldNode, pcNode);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemory::RemapChildParents(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNode)
{
	CIndexTreeNodeMemory*		pcParent;

	if (pcOldNode != pcNode)
	{
		pcParent = (CIndexTreeNodeMemory*)pcNode->GetParent();
		if (pcParent)
		{
			pcParent->RemapChildNodes(pcOldNode, pcNode);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeMemory::SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, unsigned char c)
{
	CIndexTreeNodeMemory* pcNew;
	CIndexTreeNodeMemory* pcCurrent;
	CIndexTreeNodeMemory* pcReallocedParent;

	pcCurrent = pcParent->Get(c);
	if (pcCurrent == NULL)
	{
		pcNew = AllocateNode(pcParent, c);
		pcReallocedParent = ReallocateNodeForIndex(pcParent, c);
		pcReallocedParent->Set(c, pcNew);
		if (pcParent != pcReallocedParent)
		{
			pcReallocedParent->SetChildsParent();
		}
		return pcNew;
	}
	else
	{
		return pcCurrent;
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

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return FALSE;
	}

	pcCurrent = mpcRoot;
	for (i = 0; i < iKeySize; i++)
	{
		c = ((char*)pvKey)[i];
		pcCurrent = pcCurrent->Get(c);
		if (pcCurrent == NULL)
		{
			return FALSE;
		}
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
	CIndexTreeNodeMemory*	pcOldParent;
	CIndexTreeNodeMemory*	pcNode;
	void*					pvObject;
	BOOL					bResizeNode;
	size_t					tNewNodeSize;
	size_t					tOldNodeSize;

	if (pcCurrent->ObjectSize() == 0)
	{
		return FALSE;
	}

	pvObject = ((void**) pcCurrent->GetObjectPtr());

	pcNode = pcCurrent;
	pcParent = (CIndexTreeNodeMemory*)pcNode->GetParent();
	pcCurrent->ClearObject();
	for (;;)
	{
		c = pcNode->GetIndexInParent();

		if (pcNode->IsEmpty())
		{
			tOldNodeSize = pcParent->CalculateRequiredNodeSizeForCurrent();
			if (pcParent != mpcRoot)
			{
				bResizeNode = pcParent->ClearAndUncontain(c);
				if (bResizeNode)
				{
					tNewNodeSize = pcParent->CalculateRequiredNodeSizeForCurrent();
					pcOldParent = pcParent;

					pcParent = (CIndexTreeNodeMemory*)Realloc(pcParent, tNewNodeSize, tOldNodeSize);
					pcParent->SetChildsParent();
					RemapChildParents(pcOldParent, pcParent);
				}
			}
			else
			{
				pcParent->Clear(c);
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
	void*					pvObject;

	if (pcNode != NULL)
	{
		pvObject = pcNode->GetObjectPtr();
		if (pvObject != NULL)
		{
			papvElements->Add(pvObject);
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
BOOL CIndexTreeMemory::Write(CFileWriter* pcFileWriter)
{
	SIndexTreeMemoryIterator	sIter;
	void*				pvData;
	int					iDataSize;
	int					iKeySize;
	BOOL				bResult;
	unsigned char		acKey[MAX_KEY_SIZE];
	int					iCount;

	bResult = gcMallocators.WriteMallocator(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return FALSE;
	}

	if (!pcFileWriter->WriteInt(miSize))
	{
		return FALSE;
	}

	iCount = 0;
	bResult = StartIteration(&sIter, &pvData, &iDataSize);
	while (bResult)
	{
		iKeySize = GetKey(acKey, pvData, FALSE);
		if (!pcFileWriter->WriteInt(iKeySize))
		{
			return FALSE;
		}
		if (!pcFileWriter->WriteInt(iDataSize))
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

		bResult = Iterate(&sIter, &pvData, &iDataSize);
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
	CMallocator*	pcMalloc;
	int				iCount;
	int				i;
	char			acKey[1024];
	int				iKeySize;
	int				iDataSize;
	void*			pvData;

	pcMalloc = gcMallocators.ReadMallocator(pcFileReader);
	if (pcMalloc == NULL)
	{
		return FALSE;
	}

	Init(pcMalloc);

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
int CIndexTreeMemory::GetKey(void* pvKey, void* pvData, BOOL zeroTerminate)
{
	CIndexTreeNodeMemory*	pcNode;
	unsigned char*			pucKey;
	int						iLength;
	CIndexTreeNodeMemory*	pcParent;

	if (pvData == NULL)
	{
		return 0;
	}

	iLength = 0;
	pucKey = (unsigned char*)pvKey;
	pcNode = GetNodeForData(pvData);
	pcParent = (CIndexTreeNodeMemory*)pcNode->GetParent();

	while (pcParent != NULL)
	{
		pucKey[iLength] = pcParent->FindIndex(pcNode);
		iLength++;
		pcNode = pcParent;
		pcParent = (CIndexTreeNodeMemory*)pcNode->GetParent();
	}

	ReverseBytes(pucKey, iLength);
	if (zeroTerminate)
	{
		pucKey[iLength] = '\0';
	}
	return iLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemory::StartIteration(SIndexTreeMemoryIterator* psIterator, void** pvData, int* piDataSize)
{
	psIterator->pcNode = mpcRoot;
	psIterator->iIndex = mpcRoot->GetFirstIndex();

	if (StepNext(psIterator))
	{
		if (pvData)
		{
			*pvData = psIterator->pcNode->GetObjectPtr();
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->ObjectSize();
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
BOOL CIndexTreeMemory::Iterate(SIndexTreeMemoryIterator* psIterator, void** pvData, int* piDataSize)
{
	if (StepNext(psIterator))
	{
		if (pvData)
		{
			*pvData = psIterator->pcNode->GetObjectPtr();
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->ObjectSize();
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
BOOL CIndexTreeMemory::StepNext(SIndexTreeMemoryIterator* psIterator)
{
	CIndexTreeNodeMemory*	pcChild;
	void*					pvObject;
	CIndexTreeNodeMemory*	pcParent;

	for (;;)
	{
		pcChild = psIterator->pcNode->Get(psIterator->iIndex);

		if (pcChild != NULL)
		{
			psIterator->pcNode = pcChild;
			psIterator->iIndex = pcChild->GetFirstIndex();

			pvObject = pcChild->GetObjectPtr();
			if (pvObject != NULL)
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
		if (pcNode->HasObject())
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
	unsigned short			uiSize;
	size_t					tSize;

	tSize = 0;
	if (pcNode != NULL)
	{
		tSize += pcNode->CalculateRequiredNodeSizeForCurrent();
		uiSize = pcNode->ObjectSize();

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

	return pcNode->ObjectSize() != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeMemory::CountAllocatedNodes(void)
{
	return RecurseCountAllocatedNodes(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeMemory::RecurseCountAllocatedNodes(CIndexTreeNodeMemory* pcNode)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;

	if (pcNode != NULL)
	{
		int count = 1;
		for (i = 0; i < pcNode->NumIndexes(); i++)
		{
			pcChild = pcNode->GetNode(i);
			count += RecurseCountAllocatedNodes(pcChild);
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

