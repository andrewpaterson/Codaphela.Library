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
	void				Init(void);
	CIndexTreeNode*		AllocateNode(void);
	CIndexTreeNode*		GetIndexNode(char* pszKey);
	void*				Get(char* pszKey);
	void				Put(void* pvObject, char* pszKey);
	void*				Remove(char* pszKey);
	CIndexTreeNode*		SetOldWithCurrent(CIndexTreeNode* pcCurrent, CIndexTreeNode* pcOld, char c);
	void				FindAll(CArrayVoidPtr* papvElements);
	void				RecurseFindAll(CIndexTreeNode* pcNode, CArrayVoidPtr* papvElements);
	int					GetSize(void);
	BOOL				ValidateSize(void);
	int					RecurseSize(void);
	int					RecurseSize(CIndexTreeNode* pcNode);
	BOOL				HasKey(char* pszKey);
	//Iterator<void*>	Iterator()
	int					CountAllocatedNodes(void);
	int					RecurseCountAllocatedNodes(CIndexTreeNode* pcNode);
	int					CountListSize(void);
	int					RecurseCountListSize(CIndexTreeNode* pcNode);
	int					GetModifications(void);
	//virtual String	TransformKey(String pszKey) =0;
};


#endif // __INDEX_TREE_H__

