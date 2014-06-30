#ifndef __INDEX_TREE_NODE_H__
#define __INDEX_TREE_NODE_H__
#include "Bool.h"
#include "DataTypes.h"


class CIndexTreeNode
{
protected:
	void*				mpvObject;
	CIndexTreeNode*		mpcParent;
	unsigned char		muiFirstIndex;
	unsigned char		muiLastIndex;
	unsigned char		muiUnused;
	unsigned char		mbNodesEmpty;
	CIndexTreeNode*		mapcChildren[1];  //The rest are allocated after the end of the object, up to MAX_UCHAR.

public:
	void				Init(CIndexTreeNode* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex);
	void				Init(CIndexTreeNode* pcParent, void* pvObject);
	void				Init(CIndexTreeNode* pcParent);
	void				Contain(unsigned char uiIndex);
	CIndexTreeNode*		Get(unsigned char uiIndex);
	void*				GetObject(void);
	void				Set(unsigned char uiIndex, CIndexTreeNode* pcNode);
	void				Clear(unsigned char uiIndex);
	BOOL				SetObject(void* pvObject);
	void				ClearObject(void);
	BOOL				IsEmpty(void);
	unsigned char		GetFirstIndex(void);
	unsigned char		GetLastIndex(void);
	BOOL				HasNodes(void);
	BOOL				ContainsIndex(unsigned char uiIndex);
	int					GetAdditionalIndexes(unsigned char uiIndex);
	int					GetNumIndexes(void);
	CIndexTreeNode*		GetNode(int i);
	CIndexTreeNode*		GetParent(void);
	void				RemapChildNodes(CIndexTreeNode* pcOldNode, CIndexTreeNode* pcNewNode);
	void				SetChildsParent(void);
};


#endif // __INDEX_TREE_NODE_H__

