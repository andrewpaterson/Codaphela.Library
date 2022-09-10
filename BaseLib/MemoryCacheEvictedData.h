#ifndef __MEMORY_CACHE_EVICTED_DATA_H__
#define __MEMORY_CACHE_EVICTED_DATA_H__
#include "MemoryCacheDescriptor.h"
#include "MemoryCacheEvictionCallback.h"
#include "ListVariable.h"


class CMemoryCacheEvictedData : public CMemoryCacheEvictionCallback
{
protected:
	CListVariable	mcEvictedData;

public:
	void	Init(void);
	void	Kill(void);

	bool	CacheDataEvicted(void* pvData, SMemoryCacheDescriptor* psDescriptor);

	int		NumElements(void);
	void*	Get(int iIndex, size_t* puiDataSize = NULL);
};


#endif // !__MEMORY_CACHE_EVICTED_DATA_H__

