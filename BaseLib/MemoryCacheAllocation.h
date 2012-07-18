#ifndef __MEMORY_CACHE_ALLOCATION_H__
#define __MEMORY_CACHE_ALLOCATION_H__
#include "ArrayPointer.h"


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
	CArrayPointer				mapEvictedCacheDescriptors;
	SMemoryCacheDescriptor*		mpsCacheDesc;
	unsigned int				miCachedSize;
	unsigned int				miRemaining;

public:
	void						Init(int iDataSize);
	void						Kill(void);

	BOOL						HasOverlaps(void);
	SMemoryCacheDescriptor*		Get(int iIndex);
	int							NumElements(void);

	CArrayPointer*				GetEvictedArray(void);  //This only exists because I haven't finished factoring it out.
};


#endif // __MEMORY_CACHE_ALLOCATION_H__

