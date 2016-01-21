#include "Numbers.h"
#include "GlobalMemory.h"
#include "IndexTreeBlockMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockMemory::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockMemory::Init(CMallocator* pcMalloc)
{
	CIndexTreeBlock::Init(pcMalloc, sizeof(CIndexTreeNodeMemory), sizeof(CIndexTreeNodeMemory*));
	mpcRoot = AllocateRoot();
	miSize = 0;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockMemory::FakeInit(void)
{
	//This exists so that TreeNodes can be tested without a full tree.  All they need to do is query the size of their child nodes.
	//Kill should not be called.
	CIndexTreeBlock::Init(NULL, sizeof(CIndexTreeNodeMemory), sizeof(CIndexTreeNodeMemory*));
	mpcRoot = NULL;
	miSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockMemory::Kill(void)
{
	RecurseKill(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockMemory::RecurseKill(CIndexTreeNodeMemory* pcNode)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;

	if (pcNode != NULL)
	{
		for (i = 0; i < pcNode->GetNumIndexes(); i++)
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
size_t CIndexTreeBlockMemory::CalculateRootNodeSize(void)
{
	int						iAdditionalSize;

	iAdditionalSize = (MAX_UCHAR + 1) * SizeofNodePtr();
	return SizeofNode() + iAdditionalSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeBlockMemory::AllocateRoot(void)
{
	CIndexTreeNodeMemory*	pcNode;

	pcNode = (CIndexTreeNodeMemory*)Malloc(CalculateRootNodeSize());
	pcNode->Init(this, NULL, 0, MAX_UCHAR);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeBlockMemory::AllocateNode(CIndexTreeNodeMemory* pcParent)
{
	CIndexTreeNodeMemory*	pcNode;

	pcNode = (CIndexTreeNodeMemory*)Malloc(SizeofNode());
	pcNode->Init(this, pcParent);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeBlockMemory::GetNodeForData(void* pvData)
{
	CIndexTreeNodeMemory*		psNode;

	psNode = DataGetHeader<CIndexTreeNodeMemory, void>(pvData);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeBlockMemory::GetIndexNode(void* pvKey, int iKeySize)
{
	CIndexTreeNodeMemory* pcCurrent;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	for (int i = 0; i < iKeySize; i++)
	{
		char c = ((char*)pvKey)[i];

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
void* CIndexTreeBlockMemory::Get(void* pvKey, int iKeySize)
{
	CIndexTreeNodeMemory* pcNode;
	void*			pv;

	pcNode = GetIndexNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return NULL;
	}
	else
	{
		if (pcNode->GetObjectSize() == 0)
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
void* CIndexTreeBlockMemory::Get(char* pszKey)
{
	int		iKeySize;

	if (StrEmpty(pszKey))
	{
		return NULL;
	}

	iKeySize = strlen(pszKey);
	return Get(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockMemory::PutPtr(void* pvKey, int iKeySize, void* pvPointer)
{
	void* pvResult;

	pvResult = Put(pvKey, iKeySize, &pvPointer, sizeof(void*));
	return pvResult != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockMemory::PutPtr(char* pszKey, void* pvPointer)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return PutPtr(pszKey, iKeySize, pvPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeBlockMemory::Put(char* pszKey, void* pvObject, unsigned char uiObjectSize)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Put(pszKey, iKeySize, pvObject, uiObjectSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeBlockMemory::Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiObjectSize)
{
	CIndexTreeNodeMemory*	pcCurrent;
	CIndexTreeNodeMemory*	pcReallocatedCurrent;
	unsigned char			c;
	BOOL					bResult;

	if (iKeySize == 0)
	{
		return FALSE;
	}

	pcCurrent = mpcRoot;
	if (iKeySize > miLargestKeySize)
	{
		miLargestKeySize = iKeySize;
	}

	for (int i = 0; i < iKeySize; i++)
	{
		c = ((char*)pvKey)[i];
		pcCurrent = SetOldWithCurrent(pcCurrent, c);
	}

	miSize++;

	pcReallocatedCurrent = ReallocateNodeForData(pcCurrent, uiObjectSize);
	bResult = pcReallocatedCurrent->SetObject(pvObject, uiObjectSize);
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
void* CIndexTreeBlockMemory::Put(void* pvKey, int iKeySize, unsigned char uiObjectSize)
{
	return Put(pvKey, iKeySize, NULL, uiObjectSize);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeBlockMemory::ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, unsigned char uiIndex)
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
	pcNode->Contain(uiIndex);

	RemapChildParents(pcOldNode, pcNode);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeBlockMemory::ReallocateNodeForData(CIndexTreeNodeMemory* pcNode, unsigned char uiDataSize)
{
	CIndexTreeNodeMemory*	pcOldNode;
	size_t					tNewNodeSize;

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForData(uiDataSize);

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeMemory*)Realloc(pcNode, tNewNodeSize);

	RemapChildParents(pcOldNode, pcNode);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockMemory::RemapChildParents(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNode)
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
CIndexTreeNodeMemory* CIndexTreeBlockMemory::SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, unsigned char c)
{
	CIndexTreeNodeMemory* pcNew;
	CIndexTreeNodeMemory* pcCurrent;
	CIndexTreeNodeMemory* pcReallocedParent;

	pcCurrent = pcParent->Get(c);
	if (pcCurrent == NULL)
	{
		pcNew = AllocateNode(pcParent);
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
BOOL CIndexTreeBlockMemory::Remove(char* pszKey)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Remove(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockMemory::Remove(void* pvKey, int iKeySize)
{
	char					c;
	CIndexTreeNodeMemory*	pcParent;
	CIndexTreeNodeMemory*	pcOldParent;
	CIndexTreeNodeMemory*	pcNode;
	CIndexTreeNodeMemory*	pcCurrent;
	void*					pvObject;
	BOOL					bResizeNode;
	size_t					tNewNodeSize;
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

	if (pcCurrent->GetObjectSize() == 0)
	{
		return FALSE;
	}

	pvObject = ((void**) pcCurrent->GetObjectPtr());

	pcNode = pcCurrent;
	pcParent = (CIndexTreeNodeMemory*)pcNode->GetParent();
	pcCurrent->ClearObject();
	for (;;)
	{
		c = ((char*)pvKey)[(i - 1)];
		i--;

		if (pcNode->IsEmpty())
		{
			bResizeNode = pcParent->Clear(c);
			if (bResizeNode)
			{
				tNewNodeSize = pcParent->CalculateRequiredNodeSizeForCurrent();
				pcOldParent = pcParent;
				pcParent = (CIndexTreeNodeMemory*)Realloc(pcParent, tNewNodeSize);
				pcParent->SetChildsParent();
				RemapChildParents(pcOldParent, pcParent);
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
			break;;
		}
	}

	miSize--;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockMemory::FindAll(CArrayVoidPtr* papvElements)
{
	RecurseFindAll(mpcRoot, papvElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockMemory::RecurseFindAll(CIndexTreeNodeMemory* pcNode, CArrayVoidPtr* papvElements)
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

		for (i = 0; i < pcNode->GetNumIndexes(); i++)
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
BOOL CIndexTreeBlockMemory::Write(CFileWriter* pcFileWriter)
{
	SIndexTreeIterator	sIter;
	void*				pvData;
	int					iDataSize;
	int					iKeySize;
	BOOL				bResult;
	char				acKey[1024];
	int					iCount;

	bResult = gcMallocators.WriteMallocator(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return FALSE;
	}

	if (miLargestKeySize >= 1024)
	{
		return FALSE;
	}

	if (!pcFileWriter->WriteInt(miSize))
	{
		return FALSE;
	}
	if (!pcFileWriter->WriteInt(miLargestKeySize))
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
BOOL CIndexTreeBlockMemory::Read(CFileReader* pcFileReader)
{
	CMallocator*	pcMalloc;
	int				iCount;
	int				i;
	int				iLargestKey;
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
	if (!pcFileReader->ReadInt(&iLargestKey))
	{
		return FALSE;
	}

	if (iLargestKey >= 1024)
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
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlockMemory::NumElements(void)
{
	return miSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlockMemory::GetLargestKeySize(void)
{
	return miLargestKeySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlockMemory::GetKey(void* pvKey, void* pvData, BOOL zeroTerminate)
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
BOOL CIndexTreeBlockMemory::StartIteration(SIndexTreeIterator* psIterator, void** pvData, int* piDataSize)
{
	psIterator->pcNode = mpcRoot;
	psIterator->iIndex = 0;

	if (StepNext(psIterator))
	{
		if (pvData)
		{
			*pvData = psIterator->pcNode->GetObjectPtr();
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->GetObjectSize();
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
BOOL CIndexTreeBlockMemory::Iterate(SIndexTreeIterator* psIterator, void** pvData, int* piDataSize)
{
	if (StepNext(psIterator))
	{
		if (pvData)
		{
			*pvData = psIterator->pcNode->GetObjectPtr();
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->GetObjectSize();
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
BOOL CIndexTreeBlockMemory::StepNext(SIndexTreeIterator* psIterator)
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
BOOL CIndexTreeBlockMemory::ValidateSize(void)
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
int CIndexTreeBlockMemory::RecurseSize(void)
{
	return RecurseSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlockMemory::RecurseSize(CIndexTreeNodeMemory* pcNode)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	unsigned char			uiSize;

	int count = 0;

	if (pcNode != NULL)
	{
		uiSize = pcNode->GetObjectSize();
		if (uiSize != 0)
		{
			count++;
		}

		for (i = 0; i < pcNode->GetNumIndexes(); i++)
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
BOOL CIndexTreeBlockMemory::HasKey(void* pvKey, int iKeySize)
{
	CIndexTreeNodeMemory* pcNode;

	pcNode = GetIndexNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return FALSE;
	}

	return pcNode->GetObjectSize() != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockMemory::HasKey(char* pszKey)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return HasKey(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlockMemory::CountAllocatedNodes(void)
{
	return RecurseCountAllocatedNodes(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlockMemory::RecurseCountAllocatedNodes(CIndexTreeNodeMemory* pcNode)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;

	if (pcNode != NULL)
	{
		int count = 1;
		for (i = 0; i < pcNode->GetNumIndexes(); i++)
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
int CIndexTreeBlockMemory::CountListSize(void)
{
	return RecurseCountListSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlockMemory::RecurseCountListSize(CIndexTreeNodeMemory* pcNode)
{
	int					i;
	CIndexTreeNodeMemory*		pcChild;

	if (pcNode != NULL)
	{
		int count = pcNode->GetNumIndexes();
		for (i = 0; i < pcNode->GetNumIndexes(); i++)
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

