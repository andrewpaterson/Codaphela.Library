#ifndef __MEMORY_CACHE_EVICTION_CALLBACK_H__
#define __MEMORY_CACHE_EVICTION_CALLBACK_H__


struct SMemoryCacheDescriptor;
class CMemoryCacheEvictionCallback
{
public:
	virtual bool CacheDataEvicted(void* pvData, SMemoryCacheDescriptor* psDescriptor) =0;
};


#endif // __MEMORY_CACHE_EVICTION_CALLBACK_H__

