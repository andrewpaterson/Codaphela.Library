#ifndef __CIRCULAR_MEMORY_LIST_H__
#define __CIRCULAR_MEMORY_LIST_H__
#include "Define.h"
#include "MemoryCacheDescriptor.h"


struct SCircularMemoryList
{
	size_t						muiCacheSize;

	SMemoryCacheDescriptor*		mpsTail;
	SMemoryCacheDescriptor*		mpsHead;
};


class CCircularMemoryList
{
protected:
	SCircularMemoryList*		mpsDetail;
	SMemoryCacheDescriptor*		mpvCache;

	int							miDescriptorSize;

public:
	void						Init(void* pvCache, size_t uiByteSize, int iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void						InitExisting(void* pvCache, size_t uiByteSize, int iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void						Kill(void);

	//These methods need to be renamed to something sane.
	void						Remap(void* pvNewCache, size_t uiByteSize);
	void						Remap(SCircularMemoryList* pvNewCache, size_t uiByteSize);

	void						Clear(void);

	size_t						GetSize(void* pvData);
	int							NumElements(void);

	int							GetDescriptorSize(void);

	void						Deallocate(void* pvData);

	bool						IsEmpty(void);
	int							NumElements(int iSize);
	size_t						GetCacheSize(void);
	size_t						GetAllocatedSize(void);

	bool						ValidateCache(void);
	void						Dump(void);

protected:
	SMemoryCacheDescriptor*		OneAllocation(void);
	SMemoryCacheDescriptor*		InsertNext(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	bool						Overlaps(SMemoryCacheDescriptor* pvCacheBasedNew, size_t uiNewSize, SMemoryCacheDescriptor* psExisting);

	size_t						RemainingAfterTail(void);
	size_t						RemainingAfter(SMemoryCacheDescriptor* psCacheBasedDescriptor);

	void						RemapDifferentMemory(void* pvNewCache, size_t uiCacheSize);
	void						RemapSameMemory(size_t uiCacheSize);

	SMemoryCacheDescriptor*		StartIteration(void);
	SMemoryCacheDescriptor*		Iterate(SMemoryCacheDescriptor* psCacheBasedDescriptor);

	SMemoryCacheDescriptor*		GetFirst(void);
	SMemoryCacheDescriptor*		GetNext(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		GetPrev(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		GetLast(void);

	void*						GetData(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		GetDescriptorNoRemap(void* pvData);

	void						Deallocate(SMemoryCacheDescriptor* psCacheBasedDescriptor);

	SMemoryCacheDescriptor*		MapFromZeroBasedToCacheBased(SMemoryCacheDescriptor* psZeroBasedDescriptor);
	SMemoryCacheDescriptor*		MapFromCacheBasedToZeroBased(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		MapFromZeroBasedToCacheBased(void* pvCache, SMemoryCacheDescriptor* psZeroBasedDescriptor);
	SMemoryCacheDescriptor*		MapFromCacheBasedToZeroBased(void* pvCache, SMemoryCacheDescriptor* psCacheBasedDescriptor);

	bool						IsFirst(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	bool						IsLast(SMemoryCacheDescriptor* psCacheBasedDescriptor);
};


#endif // __CIRCULAR_MEMORY_LIST_H__


