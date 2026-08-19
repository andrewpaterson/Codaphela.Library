#ifndef __CIRCULAR_MEMORY_LIST_H__
#define __CIRCULAR_MEMORY_LIST_H__
#include "Define.h"
#include "ArrayVoidPtr.h"
#include "MemoryCacheDescriptor.h"


struct SCircularMemoryList
{
	size						muiCacheSize;

	SMemoryCacheDescriptor*		mpsTail;
	SMemoryCacheDescriptor*		mpsHead;
};


class CCircularMemoryList
{
friend class CMemoryCache;
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
	size						NumElements(size iSize);
	bool						IsEmpty(void);

	void*						StartIteration(void);
	void*						Iterate(void* psCurrent);

	void*						GetFirst(void);
	void*						GetLast(void);
	void*						GetNext(void* psCurrent);
	void*						GetPrev(void* psCurrent);

	size						GetDescriptorSize(void);

	void						Deallocate(void* pvData);

	size						GetCacheSize(void);
	size						GetAllocatedSize(void);

	bool						ValidateCache(void);
	bool						ValidateEnds(void);
	void						Dump(void);

protected:
	SMemoryCacheDescriptor*		OneAllocation(void);
	SMemoryCacheDescriptor*		InsertNext(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	bool						Overlaps(SMemoryCacheDescriptor* pvCacheBasedNew, size uiNewSize, SMemoryCacheDescriptor* psExisting);

	size						RemainingAfterTail(void);
	size						RemainingAfter(SMemoryCacheDescriptor* psCacheBasedDescriptor);

	void						RemapDifferentMemory(void* pvNewCache, size uiCacheSize);
	void						RemapSameMemory(size uiCacheSize);

	SMemoryCacheDescriptor*		StartDescriptorIteration(void);
	SMemoryCacheDescriptor*		IterateDescriptor(SMemoryCacheDescriptor* psCacheBasedDescriptor);

	SMemoryCacheDescriptor*		GetFirstDescriptor(void);
	SMemoryCacheDescriptor*		GetNextDescriptor(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		GetPrevDescriptor(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		GetLastDescriptor(void);

	void*						GetData(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		GetDescriptorNoRemap(void* pvData);
	SMemoryCacheDescriptor*		GetCache(void);
	bool						CanCache(size uiDataSize);

	void						Deallocate(SMemoryCacheDescriptor* psCacheBasedDescriptor);

	SMemoryCacheDescriptor*		MapFromZeroBasedToCacheBased(SMemoryCacheDescriptor* psZeroBasedDescriptor);
	SMemoryCacheDescriptor*		MapFromCacheBasedToZeroBased(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		MapFromZeroBasedToCacheBased(void* pvCache, SMemoryCacheDescriptor* psZeroBasedDescriptor);
	SMemoryCacheDescriptor*		MapFromCacheBasedToZeroBased(void* pvCache, SMemoryCacheDescriptor* psCacheBasedDescriptor);

	void						SetEndsForPostAllocate(SMemoryCacheDescriptor* psCacheBasedDescriptor, SMemoryCacheDescriptor* psFirstOverlap, SMemoryCacheDescriptor* psLastOverlap);

	bool						IsFirst(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	bool						IsLast(SMemoryCacheDescriptor* psCacheBasedDescriptor);

protected:
	void	FindOverlapping(SMemoryCacheDescriptor* psCachedBasedNew, size uiNewSize, CArrayVoidPtr* pasOverlappingCacheDescriptors);
};


#endif // __CIRCULAR_MEMORY_LIST_H__


