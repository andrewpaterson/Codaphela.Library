#ifndef __INDEXED_DATA_EVICTION_CALLBACK_H__
#define __INDEXED_DATA_EVICTION_CALLBACK_H__
#include "IndexedGeneral.h"

class CIndexedDataEvictionCallback
{
public:
	virtual bool IndexEvicted(OIndex oi, void* pvData, int iDataSize) =0;
};


#endif // __INDEXED_EVICTION_CALLBACK_H__

