#ifndef __INDEX_TREE_EVICTED_LIST_H__
#define __INDEX_TREE_EVICTED_LIST_H__
#include "ListVariable.h"
#include "IndexTreeEvictionCallback.h"


class CIndexTreeEvictedList : public CIndexTreeEvictionCallback
{
private:
	CListVariable	mcKeys;
	CListVariable	mcDatas;

public:
	void	Init(void);
	void	Kill(void);

	bool	IndexTreeNodeEvicted(void* pvKey, size iKeySize, void* pvData, size iDataSize);
	
	size	NumElements(void);
	void*	GetKey(size iIndex, size* piKeySize = NULL);
	void*	GetData(size iIndex, size* piDataSize = NULL);
	void	Clear(void);
};


#endif // __INDEX_TREE_EVICTED_LIST_H__

