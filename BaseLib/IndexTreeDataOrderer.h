#ifndef __INDEX_TREE_DATA_ORDERER_H__
#define __INDEX_TREE_DATA_ORDERER_H__
#include "LinkedList.h"
#include "IndexTreeDataNode.h"


class CIndexTreeDataOrderer
{
	CLinkedList		mcDataOrder;

public:
			void	Init(void);
	virtual void	Kill(void);

	virtual void	Put(CIndexTreeDataNode* psNode) =0;
	virtual void	Get(CIndexTreeDataNode* psNode) =0;
	virtual void	Remove(CIndexTreeDataNode* psNode) =0;
	virtual void	HasKey(CIndexTreeDataNode* psNode) =0;
};


#endif // __INDEX_TREE_DATA_ORDERER_H__

