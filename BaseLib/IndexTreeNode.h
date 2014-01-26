#ifndef __INDEX_TREE_NODE_H__
#define __INDEX_TREE_NODE_H__
#include "Bool.h"
#include "DataTypes.h"


class CIndexTreeNode
{
protected:
	CIndexTreeNode*		mapbChildren[MAX_UCHAR];
	void*				mpvObject;

public:
	void				Init(void);
	CIndexTreeNode*		Get(int iIndex);
	void*				GetObject(void);
	void				Set(int iIndex, CIndexTreeNode* pcNode);
	void				Clear(int iIndex);
	BOOL				SetObject(void* pvObject);
	void				ClearObject(void);
	BOOL				IsEmpty(void);
	int					GetEndIndex(void);
};


#endif // __INDEX_TREE_NODE_H__

