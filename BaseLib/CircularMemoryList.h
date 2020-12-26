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

	void						Dump(void);
};


#endif // __CIRCULAR_MEMORY_LIST_H__


