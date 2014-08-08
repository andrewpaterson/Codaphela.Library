#include "Numbers.h"
#include "GlobalMemory.h"
#include "IndexTreeBlock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlock::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlock::Init(CMallocator* pcMalloc)
{
	mpcMalloc = pcMalloc;
	mpcRoot = AllocateRoot();
	miSize = 0;
	miLargestKeySize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlock::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlock::RecurseKill(CIndexTreeNode* pcNode)
{
	int					i;
	CIndexTreeNode*		pcChild;

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
void* CIndexTreeBlock::Malloc(size_t tSize)
{
	return mpcMalloc->Malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlock::Free(void* pv)
{
	mpcMalloc->Free(pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeBlock::Realloc(void* pv, size_t tSize)
{
	return mpcMalloc->Realloc(pv, tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeBlock::AllocateNode(CIndexTreeNode* pcParent)
{
	CIndexTreeNode*	pcNode;

	pcNode = (CIndexTreeNode*)Malloc(SizeofNode());
	pcNode->Init(pcParent);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeBlock::AllocateRoot(void)
{
	CIndexTreeNode*		pcNode;
	int					iAdditionalSize;

	iAdditionalSize = MAX_UCHAR * SizeofNodePtr();
	pcNode = (CIndexTreeNode*)Malloc(SizeofNode() + iAdditionalSize);
	pcNode->Init(NULL, 0, MAX_UCHAR);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeBlock::ReallocateNodeForIndex(CIndexTreeNode* pcNode, unsigned char uiIndex)
{
	CIndexTreeNode*		pcOldNode;
	size_t				tNewNodeSize;

	if (pcNode->ContainsIndex(uiIndex))
	{
		return pcNode;
	}

	if (pcNode->HasNodes())
	{
		tNewNodeSize = pcNode->CalculateRequiredNodeSizeForIndex(uiIndex);
	}
	else
	{
		tNewNodeSize = pcNode->CalculateRequiredNodeSizeForEmpty();
	}

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNode*)Realloc(pcNode, tNewNodeSize);
	pcNode->Contain(uiIndex);

	RemapChildParents(pcOldNode, pcNode);

	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlock::RemapChildParents(CIndexTreeNode* pcOldNode, CIndexTreeNode* pcNode)
{
	CIndexTreeNode*		pcParent;

	if (pcOldNode != pcNode)
	{
		pcParent = pcNode->GetParent();
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
CIndexTreeNode* CIndexTreeBlock::ReallocateNodeForData(CIndexTreeNode* pcNode, unsigned char uiDataSize)
{
	CIndexTreeNode*		pcOldNode;
	CIndexTreeNode*		pcParent;
	size_t				tNewNodeSize;

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForData(uiDataSize);

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNode*)Realloc(pcNode, tNewNodeSize);

	if (pcOldNode != pcNode)
	{
		pcParent = pcNode->GetParent();
		if (pcParent)
		{
			pcParent->RemapChildNodes(pcOldNode, pcNode);
		}
	}

	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlock::FreeNode(CIndexTreeNode* pcNode)
{
	Free(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeBlock::GetNodeForData(void* pvData)
{
	CIndexTreeNode*		psNode;

	psNode = DataGetHeader<CIndexTreeNode, void>(pvData);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeBlock::GetIndexNode(void* pvKey, int iKeySize)
{
	CIndexTreeNode* pcCurrent;

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
void* CIndexTreeBlock::Get(void* pvKey, int iKeySize)
{
	CIndexTreeNode* pcNode;
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
void* CIndexTreeBlock::Get(char* pszKey)
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
BOOL CIndexTreeBlock::PutPtr(void* pvKey, int iKeySize, void* pvPointer)
{
	return Put(pvKey, iKeySize, &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlock::PutPtr(char* pszKey, void* pvPointer)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Put(pszKey, iKeySize, &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlock::Put(char* pszKey, void* pvObject, unsigned char uiObjectSize)
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
BOOL CIndexTreeBlock::Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiObjectSize)
{
	CIndexTreeNode*		pcCurrent;
	CIndexTreeNode*		pcReallocatedCurrent;
	unsigned char		c;
	BOOL				bResult;

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
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeBlock::Put(void* pvKey, int iKeySize, unsigned char uiObjectSize)
{
	CIndexTreeNode*		pcCurrent;
	CIndexTreeNode*		pcReallocatedCurrent;
	unsigned char		c;
	BOOL				bResult;

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
	bResult = pcReallocatedCurrent->SetObject(NULL, uiObjectSize);
	if (pcCurrent != pcReallocatedCurrent)
	{
		pcReallocatedCurrent->SetChildsParent();
	}
	return pcReallocatedCurrent->GetObjectPtr();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeBlock::SetOldWithCurrent(CIndexTreeNode* pcParent, unsigned char c)
{
	CIndexTreeNode* pcNew;
	CIndexTreeNode* pcCurrent;
	CIndexTreeNode* pcReallocedParent;

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
BOOL CIndexTreeBlock::Remove(char* pszKey)
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
BOOL CIndexTreeBlock::Remove(void* pvKey, int iKeySize)
{
	char											c;
	CIndexTreeNode*									pcParent;
	CIndexTreeNode*									pcOldParent;
	CIndexTreeNode*									pcNode;
	CIndexTreeNode*									pcCurrent;
	void*											pvObject;
	BOOL											bResizeNode;
	size_t											tNewNodeSize;
	int												i;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return FALSE;
	}

	pcCurrent = mpcRoot;
	for (i = 0; i < iKeySize; i++)
	{
		char c = ((char*)pvKey)[i];
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
	pcParent = pcNode->GetParent();
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
				pcParent = (CIndexTreeNode*)Realloc(pcParent, tNewNodeSize);
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
		pcParent = pcNode->GetParent();
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
void CIndexTreeBlock::FindAll(CArrayVoidPtr* papvElements)
{
	RecurseFindAll(mpcRoot, papvElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlock::RecurseFindAll(CIndexTreeNode* pcNode, CArrayVoidPtr* papvElements)
{
	int					i;
	CIndexTreeNode*		pcChild;
	void*				pvObject;

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
BOOL CIndexTreeBlock::Write(CFileWriter* pcFileWriter)
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
BOOL CIndexTreeBlock::Read(CFileReader* pcFileReader)
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
int CIndexTreeBlock::NumElements(void)
{
	return miSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlock::GetLargestKeySize(void)
{
	return miLargestKeySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlock::GetKey(void* pvKey, void* pvData, BOOL zeroTerminate)
{
	CIndexTreeNode*	pcNode;
	unsigned char*	pucKey;
	int				iLength;
	CIndexTreeNode*	pcParent;

	if (pvData == NULL)
	{
		return 0;
	}

	iLength = 0;
	pucKey = (unsigned char*)pvKey;
	pcNode = GetNodeForData(pvData);
	pcParent = pcNode->GetParent();

	while (pcParent != NULL)
	{
		pucKey[iLength] = pcParent->FindIndex(pcNode);
		iLength++;
		pcNode = pcParent;
		pcParent = pcNode->GetParent();
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
BOOL CIndexTreeBlock::StartIteration(SIndexTreeIterator* psIterator, void** pvData, int* piDataSize)
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
BOOL CIndexTreeBlock::Iterate(SIndexTreeIterator* psIterator, void** pvData, int* piDataSize)
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


BOOL CIndexTreeBlock::StepNext(SIndexTreeIterator* psIterator)
{
	CIndexTreeNode*		pcChild;
	void*				pvObject;
	CIndexTreeNode*		pcParent;

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
					pcParent = psIterator->pcNode->GetParent();
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
BOOL CIndexTreeBlock::ValidateSize(void)
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
int CIndexTreeBlock::RecurseSize(void)
{
	return RecurseSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlock::RecurseSize(CIndexTreeNode* pcNode)
{
	int					i;
	CIndexTreeNode*		pcChild;
	unsigned char		uiSize;

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
BOOL CIndexTreeBlock::HasKey(void* pvKey, int iKeySize)
{
	CIndexTreeNode* pcNode;

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
BOOL CIndexTreeBlock::HasKey(char* pszKey)
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
int CIndexTreeBlock::CountAllocatedNodes(void)
{
	return RecurseCountAllocatedNodes(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlock::RecurseCountAllocatedNodes(CIndexTreeNode* pcNode)
{
	int					i;
	CIndexTreeNode*		pcChild;

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
int CIndexTreeBlock::CountListSize(void)
{
	return RecurseCountListSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeBlock::RecurseCountListSize(CIndexTreeNode* pcNode)
{
	int					i;
	CIndexTreeNode*		pcChild;

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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeBlock::SizeofNode(void)
{
	return sizeof(CIndexTreeNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeBlock::SizeofNodePtr(void)
{
	return sizeof(CIndexTreeNode*);
}

