#include "IndexTree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTree::Init(void)
{
	mpRoot = AllocateNode();
	miSize = 0;
	miModifications = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTree::AllocateNode(void)
{
	CIndexTreeNode*	pcNode;

	pcNode = (CIndexTreeNode*)malloc(sizeof(CIndexTreeNode));
	pcNode->Init();
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTree::GetIndexNode(char* pszKey)
{
	CIndexTreeNode* pcCurrent;
	int				iKeySize;

	if (StrEmpty(pszKey))
	{
		return NULL;
	}

	//pszKey = TransformKey(pszKey);

	iKeySize = strlen(pszKey);

	pcCurrent = mpRoot;
	for (int i = 0; i < iKeySize; i++)
	{
		char c = pszKey[i];

		pcCurrent = pcCurrent->Get(c);
		if (pcCurrent == NULL)
		{
			return NULL;
		}
	}
	return pcCurrent;
}


void* CIndexTree::Get(char* pszKey)
{
	CIndexTreeNode* pcNode;

	if (StrEmpty(pszKey))
	{
		return NULL;
	}

	pcNode = GetIndexNode(pszKey);
	if (pcNode == NULL)
	{
		return NULL;
	}
	else
	{
		return pcNode->GetObject();
	}
}


void CIndexTree::Put(void* pvObject, char* pszKey)
{
	int					iKeySize;
	CIndexTreeNode*		pcCurrent;
	CIndexTreeNode*		pcOld;

	if (StrEmpty(pszKey))
	{
		return;
	}

	//	pszKey = transformKey(pszKey);

	iKeySize = strlen(pszKey);
	pcCurrent = mpRoot;
	pcOld = mpRoot;

	for (int i = 0; i < iKeySize; i++)
	{
		char c = pszKey[i];
		pcCurrent = SetOldWithCurrent(pcCurrent, pcOld, c);
		pcOld = pcCurrent;
	}

	miModifications++;
	miSize++;
	pcCurrent->SetObject(pvObject);
}

CIndexTreeNode* CIndexTree::SetOldWithCurrent(CIndexTreeNode* pcCurrent, CIndexTreeNode* pcOld, char c)
{
	pcCurrent = pcCurrent->Get(c);
	if (pcCurrent == NULL)
	{
		pcCurrent = AllocateNode();
		pcOld->Set(c, pcCurrent);
	}
	return pcCurrent;
}

void* CIndexTree::Remove(char* pszKey)
{
	char									c;
	int										iKeySize;
	CIndexTreeNode*							pcParent;
	CIndexTreeNode*							pcNode;
	CArrayEmbedded<CIndexTreeNode*, 64>		apcPath;
	CIndexTreeNode*							pcCurrent;

	if (pszKey == NULL)
	{
		return NULL;
	}

	//	pszKey = transformKey(pszKey);

	apcPath.Init();
	iKeySize = strlen(pszKey);
	pcCurrent = mpRoot;
	apcPath.Add(&pcCurrent);
	for (int i = 0; i < iKeySize; i++)
	{
		char c = pszKey[i];
		pcCurrent = pcCurrent->Get(c);
		if (pcCurrent == NULL)
		{
			return NULL;
		}
		apcPath.Add(&pcCurrent);
	}

	void* pvObject = (void*) pcCurrent->GetObject();
	if (pvObject == NULL)
	{
		return NULL;
	}

	pcCurrent->ClearObject();
	for (int i = apcPath.NumElements() - 1; i > 0; i--)
	{
		c = pszKey[(i - 1)];
		pcNode = *apcPath.Get(i);
		pcParent = *apcPath.Get(i - 1);

		if (pcNode->IsEmpty())
		{
			pcParent->Clear(c);
		}
	}

	miModifications++;
	miSize--;
	return pvObject;
}


void CIndexTree::FindAll(CArrayVoidPtr* papvElements)
{
	RecurseFindAll(mpRoot, papvElements);
}

void CIndexTree::RecurseFindAll(CIndexTreeNode* pcNode, CArrayVoidPtr* papvElements)
{
	int					i;
	CIndexTreeNode*		pcChild;

	if (pcNode != NULL)
	{
		void* pvObject = (void*) pcNode->GetObject();
		if (pvObject != NULL)
		{
			papvElements->Add(pvObject);
		}

		for (i = 0; i < pcNode->GetEndIndex(); i++)
		{
			pcChild = pcNode->Get(i);
			RecurseFindAll(pcChild, papvElements);
		}
	}
}

int CIndexTree::GetSize(void)
{
	return miSize;
}

BOOL CIndexTree::ValidateSize(void)
{
	int iSizeFromRecursion;

	iSizeFromRecursion = RecurseSize(mpRoot);

	if (miSize != iSizeFromRecursion)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

int CIndexTree::RecurseSize(void)
{
	return RecurseSize(mpRoot);
}

int CIndexTree::RecurseSize(CIndexTreeNode* pcNode)
{
	int					i;
	CIndexTreeNode*		pcChild;

	int count = 0;

	if (pcNode != NULL)
	{
		void* pvObject = (void*) pcNode->GetObject();
		if (pvObject != NULL)
		{
			count++;
		}

		for (i = 0; i < pcNode->GetEndIndex(); i++)
		{
			pcChild = pcNode->Get(i);
			count += RecurseSize(pcChild);
		}
	}
	return count;
}


BOOL CIndexTree::HasKey(char* pszKey)
{
	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	CIndexTreeNode* pcNode = GetIndexNode(pszKey);
	if (pcNode == NULL)
	{
		return FALSE;
	}

	return pcNode->GetObject() != NULL;
}

//Iterator<void*> Iterator()
//{
//	return new IndexTreeIterator(this);
//}

int CIndexTree::CountAllocatedNodes(void)
{
	return RecurseCountAllocatedNodes(mpRoot);
}

int CIndexTree::RecurseCountAllocatedNodes(CIndexTreeNode* pcNode)
{
	int					i;
	CIndexTreeNode*		pcChild;

	if (pcNode != NULL)
	{
		int count = 1;
		for (i = 0; i < pcNode->GetEndIndex(); i++)
		{
			pcChild = pcNode->Get(i);
			count += RecurseCountAllocatedNodes(pcChild);
		}
		return count;
	}
	else
	{
		return 0;
	}
}

int CIndexTree::CountListSize(void)
{
	return RecurseCountListSize(mpRoot);
}

int CIndexTree::RecurseCountListSize(CIndexTreeNode* pcNode)
{
	int					i;
	CIndexTreeNode*		pcChild;

	if (pcNode != NULL)
	{
		int count = pcNode->GetEndIndex()+1;
		for (i = 0; i < pcNode->GetEndIndex(); i++)
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

int CIndexTree::GetModifications(void)
{
	return miModifications;
}

