#ifndef __INDEX_TREE_DATA_ORDERER_H__
#define __INDEX_TREE_DATA_ORDERER_H__
#include "LinkedList.h"
#include "IndexTreeDataNode.h"


class CIndexTreeNode;
struct SDataOrderIterator
{
	CIndexTreeDataNode* pcNode;
};


class CIndexTree;
class CIndexTreeDataOrderer
{
protected:
	CLinkedList		mcDataOrder;
	CIndexTree*		mpcIndexTree;

public:
			void				Init(void);
	virtual void				Kill(void);

			void				New(CIndexTreeDataNode* psNode);
	virtual void				Put(CIndexTreeDataNode* psNode) =0;
	virtual void				Get(CIndexTreeDataNode* psNode) =0;
	virtual void				Remove(CIndexTreeDataNode* psNode) =0;
	virtual void				HasKey(CIndexTreeDataNode* psNode) =0;

			void				SetIndexTree(CIndexTree* pcIndexTree);

			BOOL				StartIteration(SDataOrderIterator* psIter, void** ppvData, int* piDataSize);
			BOOL				Iterate(SDataOrderIterator* psIter, void** ppvData, int* piDataSize);

protected:
			BOOL				IterateNode(SLLNode* psNode, SDataOrderIterator* psIter, void** ppvData, int* piDataSize);
};


#endif // __INDEX_TREE_DATA_ORDERER_H__

