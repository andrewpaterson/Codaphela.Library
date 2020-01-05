#ifndef __INDEXED_EVICTED_LIST_H__
#define __INDEXED_EVICTED_LIST_H__
#include "BaseLib/ArrayLong.h"
#include "BaseLib/ListVariable.h"
#include "IndexedEvictionCallback.h"


class CIndexedEvictedList : public CIndexedEvictionCallback
{
private:
	CArrayLong		mcKeys;
	CListVariable	mcDatas;

public:
	void	Init(void);
	void	Kill(void);

	BOOL	IndexEvicted(OIndex oi, void* pvData, int iDataSize);

	int		NumElements(void);
	OIndex	GetKey(int iIndex);
	void*	GetData(int iIndex, int* piDataSize = NULL);
	void	Clear(void);

};


#endif // __INDEXED_EVICTED_LIST_H__

