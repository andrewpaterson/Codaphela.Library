#ifndef __INDEX_TREE_EVICTED_NODE_LIST__
#define __INDEX_TREE_EVICTED_NODE_LIST__
#include "BaseLib/ListVariable.h"
#include "IndexTreeEvictionCallback.h"


class CIndexTreeEvictedNodeList : public CIndexTreeEvictionCallback
{
private:
	CListVariable	mcKeys;
	CListVariable	mcDatas;

public:
	void			Init(void);
	void			Kill(void);

	BOOL			NodeEvicted(CIndexTreeFile* pcIndexTree, unsigned char* pvKey, int iKeySize, void* pvData, int iDataSize);
	
	int				NumElements(void);
	unsigned char*	GetKey(int iIndex, int* piKeySize = NULL);
	void*			GetData(int iIndex, int* piDataSize = NULL);
	void			Clear(void);
};


#endif // __INDEX_TREE_EVICTED_NODE_LIST__


