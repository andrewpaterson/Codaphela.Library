#ifndef __MEMORY_CACHE_EVICTION_CALLBACK_H__
#define __MEMORY_CACHE_EVICTION_CALLBACK_H__
#include "BaseLib/Bool.h"


class CMemoryCacheEvictionCallback
{
public:
	virtual BOOL CacheDataEvicted(void* pvData, size_t iDataSize) = 0;
};


#endif // __MEMORY_CACHE_EVICTION_CALLBACK_H__

