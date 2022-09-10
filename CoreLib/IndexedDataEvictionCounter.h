#ifndef __INDEXED_DATA_EVICTION_COUNTER_H__
#define __INDEXED_DATA_EVICTION_COUNTER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "IndexedDataEvictionCallback.h"


class CIndexedDataEvictionCounter : public CIndexedDataEvictionCallback
{
private:
	int64	miEvictions;

public:
	void	Init(void);
	void	Reset(void);
	bool	IndexEvicted(OIndex oi, void* pvData, int iDataSize);
	int64	EvictionCount(void);
};


#endif // !__INDEXED_DATA_EVICTION_COUNTER_H__

