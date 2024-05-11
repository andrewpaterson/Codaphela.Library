#ifndef __INDEXED_DATA_EVICTED_LIST_H__
#define __INDEXED_DATA_EVICTED_LIST_H__
#include "ArrayLong.h"
#include "ListVariable.h"
#include "IndexedDataEvictionCallback.h"


class CIndexedDataEvictedList : public CIndexedDataEvictionCallback
{
private:
	CArrayLong		mcKeys;
	CListVariable	mcDatas;

public:
	void	Init(void);
	void	Kill(void);

	bool	IndexEvicted(OIndex oi, void* pvData, size iDataSize);

	size		NumElements(void);
	OIndex	GetKey(size iIndex);
	void*	GetData(size iIndex, size* piDataSize = NULL);
	void	Clear(void);

};


#endif // __INDEXED_DATA_EVICTED_LIST_H__

