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
	miModifications = 0;
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

	pcNode = (CIndexTreeNode*)Malloc(sizeof(CIndexTreeNode));
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

	iAdditionalSize = MAX_UCHAR * sizeof(CIndexTreeNode*);
	pcNode = (CIndexTreeNode*)Malloc(sizeof(CIndexTreeNode) + iAdditionalSize);
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
	CIndexTreeNode*		pcParent;
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
	int					iKeySize;

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
BOOL CIndexTreeBlock::PutPtr(void* pvPointer, void* pvKey, int iKeySize)
{
	return Put(&pvPointer, sizeof(void*), pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlock::PutPtr(void* pvPointer, char* pszKey)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Put(&pvPointer, sizeof(void*), pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlock::Put(void* pvObject, unsigned char uiObjectSize, char* pszKey)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Put(pvObject, uiObjectSize, pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlock::Put(void* pvObject, unsigned char uiObjectSize, void* pvKey, int iKeySize)
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

	for (int i = 0; i < iKeySize; i++)
	{
		c = ((char*)pvKey)[i];
		pcCurrent = SetOldWithCurrent(pcCurrent, c);
	}

	miModifications++;
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
	CIndexTreeNode*									pcNode;
	CArrayTemplateEmbedded<CIndexTreeNode*, 64>		apcPath;
	CIndexTreeNode*									pcCurrent;
	void*											pvObject;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return FALSE;
	}

	apcPath.Init();
	pcCurrent = mpcRoot;
	apcPath.Add(&pcCurrent);
	for (int i = 0; i < iKeySize; i++)
	{
		char c = ((char*)pvKey)[i];
		pcCurrent = pcCurrent->Get(c);
		if (pcCurrent == NULL)
		{
			return FALSE;
		}
		apcPath.Add(&pcCurrent);
	}

	if (pcCurrent->GetObjectSize() == 0)
	{
		return FALSE;
	}

	pvObject = ((void**) pcCurrent->GetObjectPtr());

	pcCurrent->ClearObject();
	for (int i = apcPath.NumElements() - 1; i > 0; i--)
	{
		c = ((char*)pvKey)[(i - 1)];
		pcNode = *apcPath.Get(i);
		pcParent = *apcPath.Get(i - 1);

		if (pcNode->IsEmpty())
		{
			pcParent->Clear(c);
		}
	}

	miModifications++;
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
int CIndexTreeBlock::NumElements(void)
{
	return miSize;
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
int CIndexTreeBlock::GetModifications(void)
{
	return miModifications;
}

