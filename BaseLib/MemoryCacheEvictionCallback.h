#ifndef __MEMORY_CACHE_EVICTION_CALLBACK_H__
#define __MEMORY_CACHE_EVICTION_CALLBACK_H__
#include "BaseLib/Bool.h"


struct SMemoryCacheDescriptor;
class CMemoryCacheEvictionCallback
{
public:
	virtual BOOL CacheDataEvicted(void* pvData, SMemoryCacheDescriptor* psDescriptor) =0;
};


#endif // !__MEMORY_CACHE_EVICTION_CALLBACK_H__

