#ifndef __INDEX_TREE_DATA_NODE_H__
#define __INDEX_TREE_DATA_NODE_H__
#include "PrimitiveTypes.h"
#include "LinkedList.h"


class CIndexTreeDataNode
{
protected:
	SLLNode		msListNode;
	uint16		muiDataSize;   // Size of object "pointed" to by this node.  The object is small: usually a OIndex, a pointer or a CFileId.

public:
	void		Init(uint16 uiDataSize);
	void		Kill(void);

	uint16		GetDataSize(void);
	void		SetDataSize(uint16 uiDataSize);
	SLLNode*	GetListNode(void);
	void		RemapListNode(void);
};


#endif // __INDEX_TREE_DATA_NODE_H__

