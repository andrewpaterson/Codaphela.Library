#ifndef __INDEX_TREE_DATA_ORDERER_H__
#define __INDEX_TREE_DATA_ORDERER_H__
#include "LinkedList.h"
#include "Constructable.h"
#include "Killable.h"
#include "IndexTreeDataNode.h"


class CIndexTreeNode;
struct SDataOrderIterator
{
	CIndexTreeDataNode* pcNode;
};


class CIndexTree;
class CIndexTreeDataOrderer : public CConstructable, public CKillable
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

			//This is not safe.  The tree could change under the SDataOrderIterator.
			bool				StartIteration(SDataOrderIterator* psIter, uint8* pvDestKey, size* piKeySize, size iDestKeySize, void* pvDestData, size* piDataSize, size iDestDataSize);
			bool				Iterate(SDataOrderIterator* psIter, uint8* pvDestKey, size* piKeySize, size iDestKeySize, void* pvDestData, size* piDataSize, size iDestDataSize);

			CIndexTreeNode*		GetFirstTreeNode(void);
			CIndexTreeNode*		GetLastTreeNode(void);

protected:
			bool				IterateNode(SLLNode* psNode, SDataOrderIterator* psIter, uint8* pvDestKey, size* piKeySize, size iDestKeySize, void* pvDestData, size* piDataSize, size iDestDataSize);
};


#endif // __INDEX_TREE_DATA_ORDERER_H__

