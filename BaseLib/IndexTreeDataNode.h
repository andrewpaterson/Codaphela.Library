#ifndef __INDEX_TREE_DATA_NODE_H__
#define __INDEX_TREE_DATA_NODE_H__
#include "LinkedList.h"


class CIndexTreeDataNode
{
protected:
	SLLNode			msListNode;
	unsigned short	muiDataSize;   // Size of object "pointed" to by this node.  The object is small: usually a OIndex, a pointer or a CFileId.

public:
	void			Init(unsigned short uiDataSize);
	void			Kill(void);

	unsigned short	GetDataSize(void);
	void			SetDataSize(unsigned short uiDataSize);
	SLLNode*		GetListNode(void);
	void			RemapListNode(void);
};


#endif // __INDEX_TREE_DATA_NODE_H__

