#ifndef __MEMORY_CACHE_ALLOCATION_H__
#define __MEMORY_CACHE_ALLOCATION_H__
#include "ArrayVoidPtr.h"


struct SMemoryCacheDescriptor
{
	SMemoryCacheDescriptor*		psNext;
	SMemoryCacheDescriptor*		psPrev;
	unsigned int				iDataSize;  //Not including the sizeof this.
	unsigned int				iFlags;
};


class CMemoryCacheAllocation
{
friend class CMemoryCache;
protected:
	int							miDataSize;
	CArrayVoidPtr				mapEvictedCacheDescriptors; 
	SMemoryCacheDescriptor*		mpsCacheDesc;
	unsigned int				miCachedSize;

public:
	void						Init(int iDataSize);
	void						Kill(void);

	BOOL						HasOverlaps(void);
	SMemoryCacheDescriptor*		Get(int iIndex);
	int							NumElements(void);

	CArrayVoidPtr*				GetEvictedArray(void);
};


#endif // __MEMORY_CACHE_ALLOCATION_H__

