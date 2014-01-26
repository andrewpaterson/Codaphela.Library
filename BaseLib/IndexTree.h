#ifndef __INDEX_TREE_H__
#define __INDEX_TREE_H__
#include "Chars.h"
#include "ArrayVoidPtr.h"
#include "ArrayEmbedded.h"
#include "IndexTreeNode.h"


class CIndexTree
{
protected:
	CIndexTreeNode* mpRoot;
	int miSize;
	int miModifications;

public:
void Init(void)
{
	mpRoot = AllocateNode();
	miSize = 0;
	miModifications = 0;
}

CIndexTreeNode* AllocateNode(void)
{
	CIndexTreeNode*	pcNode;

	pcNode = (CIndexTreeNode*)malloc(sizeof(CIndexTreeNode));
	pcNode->Init();
	return pcNode;
}

CIndexTreeNode* GetIndexNode(char* pszKey)
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

void* Get(char* pszKey)
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


void Put(void* pvObject, char* pszKey)
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

CIndexTreeNode* SetOldWithCurrent(CIndexTreeNode* pcCurrent, CIndexTreeNode* pcOld, char c)
{
	pcCurrent = pcCurrent->Get(c);
	if (pcCurrent == NULL)
	{
		pcCurrent = AllocateNode();
		pcOld->Set(c, pcCurrent);
	}
	return pcCurrent;
}

void* Remove(char* pszKey)
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

void FindAll(CArrayVoidPtr* papvElements)
{
	RecurseFindAll(mpRoot, papvElements);
}

void RecurseFindAll(CIndexTreeNode* pcNode, CArrayVoidPtr* papvElements)
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
			RecurseFindAll(pcChild, papvElements);
		}
	}
}

int GetSize()
{
	return miSize;
}

BOOL ValidateSize()
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

int RecurseSize()
{
	return RecurseSize(mpRoot);
}

int RecurseSize(CIndexTreeNode* pcNode)
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
			count += RecurseSize(pcChild);
		}
	}
	return count;
}


BOOL HasKey(char* pszKey)
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

int CountAllocatedNodes(void)
{
	return RecurseCountAllocatedNodes(mpRoot);
}

int RecurseCountAllocatedNodes(CIndexTreeNode* pcNode)
{
	int					i;
	CIndexTreeNode*		pcChild;

	if (pcNode != NULL)
	{
		int count = 1;
		for (i = 0; i < pcNode->GetEndIndex(); i++)
		{
			count += RecurseCountAllocatedNodes(pcChild);
		}
		return count;
	}
	else
	{
		return 0;
	}
}

int CountListSize(void)
{
	return RecurseCountListSize(mpRoot);
}

int RecurseCountListSize(CIndexTreeNode* pcNode)
{
	int					i;
	CIndexTreeNode*		pcChild;

	if (pcNode != NULL)
	{
		int count = pcNode->GetEndIndex()+1;
		for (i = 0; i < pcNode->GetEndIndex(); i++)
		{
			count += RecurseCountListSize(pcChild);
		}
		return count;
	}
	else
	{
		return 0;
	}
}

int GetModifications(void)
{
	return miModifications;
}

//virtual String transformKey(String pszKey) =0;

};


#endif // __INDEX_TREE_H__

