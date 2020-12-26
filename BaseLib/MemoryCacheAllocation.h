#ifndef __MEMORY_CACHE_ALLOCATION_H__
#define __MEMORY_CACHE_ALLOCATION_H__
#include "ArrayVoidPtr.h"
#include "MemoryCacheDescriptor.h"

class CMemoryCacheAllocation
{
friend class CMemoryCache;
protected:
	unsigned int				muiSize;
	int							miDescriptorSize;
	CArrayVoidPtr				mapEvictedCacheDescriptors; 
	SMemoryCacheDescriptor*		mpsDescriptor;
	unsigned int				miCachedSize;

public:
	void						Init(int uiSize, int iDescriptorSize);
	void						Kill(void);

	BOOL						HasOverlaps(void);
	SMemoryCacheDescriptor*		Get(int iIndex);
	int							NumElements(void);

	CArrayVoidPtr*				GetEvictedArray(void);

	void						Dump(void);
};


#endif // __MEMORY_CACHE_ALLOCATION_H__

