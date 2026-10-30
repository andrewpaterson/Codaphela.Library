#ifndef __CIRCULAR_MEMORY_LIST_H__
#define __CIRCULAR_MEMORY_LIST_H__
#include "Define.h"
#include "ArrayVoidPtr.h"
#include "MemoryCacheDescriptor.h"


struct SCircularMemoryList
{
	lsize						muiCacheSize;

	SMemoryCacheDescriptor*		mpsTail;
	SMemoryCacheDescriptor*		mpsHead;
};


class CCircularMemoryList
{
friend class CMemoryCache;
protected:
	SCircularMemoryList*		mpsDetail;
	SMemoryCacheDescriptor*		mpvCache;

	lsize						miDescriptorSize;

public:
	void						Init(void* pvCache, lsize uiByteSize, lsize iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void						InitExisting(void* pvCache, lsize uiByteSize, lsize iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void						Kill(void);

	//These methods need to be renamed to something sane.
	void						Remap(void* pvNewCache, lsize uiByteSize);
	void						Remap(SCircularMemoryList* pvNewCache, lsize uiByteSize);

	void						Clear(void);

	lsize						GetSize(void* pvData);
	lsize						NumElements(void);
	lsize						NumElements(lsize iSize);
	bool						IsEmpty(void);

	void*						StartIteration(void);
	void*						Iterate(void* psCurrent);

	void*						GetFirst(void);
	void*						GetLast(void);
	void*						GetNext(void* psCurrent);
	void*						GetPrev(void* psCurrent);

	lsize						GetDescriptorSize(void);

	void						Deallocate(void* pvData);

	lsize						GetCacheSize(void);
	lsize						GetAllocatedSize(void);

	bool						ValidateCache(void);
	bool						ValidateEnds(void);
	void						Dump(void);

protected:
	SMemoryCacheDescriptor*		OneAllocation(void);
	SMemoryCacheDescriptor*		InsertNext(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	bool						Overlaps(SMemoryCacheDescriptor* pvCacheBasedNew, lsize uiNewSize, SMemoryCacheDescriptor* psExisting);

	lsize						RemainingAfterTail(void);
	lsize						RemainingAfter(SMemoryCacheDescriptor* psCacheBasedDescriptor);

	void						RemapDifferentMemory(void* pvNewCache, lsize uiCacheSize);
	void						RemapSameMemory(lsize uiCacheSize);

	SMemoryCacheDescriptor*		StartDescriptorIteration(void);
	SMemoryCacheDescriptor*		IterateDescriptor(SMemoryCacheDescriptor* psCacheBasedDescriptor);

	SMemoryCacheDescriptor*		GetFirstDescriptor(void);
	SMemoryCacheDescriptor*		GetNextDescriptor(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		GetPrevDescriptor(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		GetLastDescriptor(void);

	void*						GetData(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		GetDescriptorNoRemap(void* pvData);
	SMemoryCacheDescriptor*		GetCache(void);
	bool						CanCache(lsize uiDataSize);

	void						Deallocate(SMemoryCacheDescriptor* psCacheBasedDescriptor);

	SMemoryCacheDescriptor*		MapFromZeroBasedToCacheBased(SMemoryCacheDescriptor* psZeroBasedDescriptor);
	SMemoryCacheDescriptor*		MapFromCacheBasedToZeroBased(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	SMemoryCacheDescriptor*		MapFromZeroBasedToCacheBased(void* pvCache, SMemoryCacheDescriptor* psZeroBasedDescriptor);
	SMemoryCacheDescriptor*		MapFromCacheBasedToZeroBased(void* pvCache, SMemoryCacheDescriptor* psCacheBasedDescriptor);

	void						SetEndsForPostAllocate(SMemoryCacheDescriptor* psCacheBasedDescriptor, SMemoryCacheDescriptor* psFirstOverlap, SMemoryCacheDescriptor* psLastOverlap);

	bool						IsFirst(SMemoryCacheDescriptor* psCacheBasedDescriptor);
	bool						IsLast(SMemoryCacheDescriptor* psCacheBasedDescriptor);

protected:
	size						FindOverlapping(SMemoryCacheDescriptor* psCachedBasedNew, lsize uiNewSize, CArrayVoidPtr* pasOverlappingCacheDescriptors);
};


#endif // __CIRCULAR_MEMORY_LIST_H__


