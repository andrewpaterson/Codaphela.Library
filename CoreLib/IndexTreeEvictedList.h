#ifndef __INDEX_TREE_EVICTED_LIST_H__
#define __INDEX_TREE_EVICTED_LIST_H__
#include "BaseLib/ListVariable.h"
#include "IndexTreeEvictionCallback.h"


class CIndexTreeEvictedList : public CIndexTreeEvictionCallback
{
private:
	CListVariable	mcKeys;
	CListVariable	mcDatas;

public:
	void	Init(void);
	void	Kill(void);

	BOOL	IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	
	int		NumElements(void);
	void*	GetKey(int iIndex, int* piKeySize = NULL);
	void*	GetData(int iIndex, int* piDataSize = NULL);
	void	Clear(void);
};


#endif // !__INDEX_TREE_EVICTED_LIST_H__

