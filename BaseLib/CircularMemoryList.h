#ifndef __CIRCULAR_MEMORY_LIST_H__
#define __CIRCULAR_MEMORY_LIST_H__
#include "Define.h"
#include "MemoryCacheDescriptor.h"


struct SCircularMemoryList
{
	size						muiCacheSize;

	SMemoryCacheDescriptor*		mpsTail;
	SMemoryCacheDescriptor*		mpsHead;
};


class CCircularMemoryList
{
protected:
	SCircularMemoryList*		mpsDetail;
	SMemoryCacheDescriptor*		mpvCache;

	size						miDescriptorSize;

public:
	void						Init(void* pvCache, size uiByteSize, size iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void						InitExisting(void* pvCache, size uiByteSize, size iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void						Kill(void);

	//These methods need to be renamed to something sane.
	void						Remap(void* pvNewCache, size uiByteSize);
	void						Remap(SCircularMemoryList* pvNewCache, size uiByteSize);

	void						Clear(void);

	size						GetSize(void* pvData);
	size						NumElements(void);

	size						GetDescriptorSize(void);

	void						Deallocate(void* pvData);

	bool						IsEmpty(void);
	size						NumElements(size iSize);
	size						GetCacheSize(void);
	size						GetAllocatedSize(void);

	bool						ValidateCache(void);
	void						Dump(void);

protected:
	SMemoryCacheDescriptor*		OneAllocation(void);
	SMemoryCacheDescriptor*		InsertNext(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	bool						Overlaps(SMemoryCacheDescriptor* pvCacheBasedNew, size uiNewSize, SMemoryCacheDescriptor* psExisting);

	size						RemainingAfterTail(void);
	size						RemainingAfter(SMemoryCacheDescriptor* psCacheBasedDescriptor);

	void						RemapDifferentMemory(void* pvNewCache, size uiCacheSize);
	void						RemapSameMemory(size uiCacheSize);

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


