#ifndef __INDEXED_DATA_EVICTION_CALLBACK_H__
#define __INDEXED_DATA_EVICTION_CALLBACK_H__
#include "BaseLib/Bool.h"
#include "IndexedGeneral.h"

class CIndexedDataEvictionCallback
{
public:
	virtual BOOL IndexEvicted(OIndex oi, void* pvData, int iDataSize) =0;
};


#endif // __INDEXED_EVICTION_CALLBACK_H__

