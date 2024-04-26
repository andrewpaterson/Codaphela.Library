#ifndef __MEMORY_CACHE_ALLOCATION_H__
#define __MEMORY_CACHE_ALLOCATION_H__
#include "ArrayVoidPtr.h"
#include "MemoryCacheDescriptor.h"

class CMemoryCacheAllocation
{
friend class CMemoryCache;
protected:
	uint32						muiSize;
	CArrayVoidPtr				mapEvictedCacheDescriptors; 
	SMemoryCacheDescriptor*		mpsDescriptor;
	uint32						miCachedSize;

public:
	void						Init(int uiSize);
	void						Kill(void);

	bool						HasOverlaps(void);
	SMemoryCacheDescriptor*		Get(int iIndex);
	int							NumElements(void);

	CArrayVoidPtr*				GetEvictedArray(void);

	void						Dump(int iDescriptorSize);
};


#endif // __MEMORY_CACHE_ALLOCATION_H__

