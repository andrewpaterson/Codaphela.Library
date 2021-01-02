#ifndef __CIRCULAR_MEMORY_LIST_H__
#define __CIRCULAR_MEMORY_LIST_H__
#include "Define.h"
#include "MemoryCacheDescriptor.h"


class CCircularMemoryList
{
protected:
	void*						mpvCache;
	size_t						muiCacheSize;

	SMemoryCacheDescriptor*		mpsTail;
	SMemoryCacheDescriptor*		mpsHead;

	int							miDescriptorSize;

public:
	void						Init(void* pvCache, size_t uiCacheSize, int iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void						Kill(void);

	void						Remap(void* pvNewCache, size_t uiCacheSize);
	void						Clear(void);

	SMemoryCacheDescriptor*		GetFirst(void);
	SMemoryCacheDescriptor*		GetNext(SMemoryCacheDescriptor* psCurrent);
	SMemoryCacheDescriptor*		GetPrev(SMemoryCacheDescriptor* psCurrent);
	SMemoryCacheDescriptor*		GetLast(void);
	
	int							NumElements(void);

	int							GetDescriptorSize(void);
	void*						GetData(SMemoryCacheDescriptor* psCacheDesc);
	SMemoryCacheDescriptor*		GetDescriptor(void* pvData);

	void						Deallocate(void* pvData);
	void						Deallocate(SMemoryCacheDescriptor* psDescriptor);

	BOOL						IsEmpty(void);
	int							NumElements(int iSize);
	size_t						GetCacheSize(void);
	size_t						GetAllocatedSize(void);

	SMemoryCacheDescriptor*		StartIteration(void);
	SMemoryCacheDescriptor*		Iterate(SMemoryCacheDescriptor* psCurrent);

	BOOL						ValidateCache(void);
	void						Dump(void);

protected:
	SMemoryCacheDescriptor*		OneAllocation(void);
	SMemoryCacheDescriptor*		InsertNext(SMemoryCacheDescriptor* psDescriptor);
	BOOL						Overlaps(void* pvNew, size_t uiNewSize, SMemoryCacheDescriptor* psExisting);

	size_t						RemainingAfterTail(void);
	size_t						RemainingAfter(SMemoryCacheDescriptor* psDescriptor);

	void						RemapDifferentMemory(void* pvNewCache, size_t uiCacheSize);
	void						RemapSameMemory(void* pvNewCache, size_t uiCacheSize);
};


#endif // __CIRCULAR_MEMORY_LIST_H__


