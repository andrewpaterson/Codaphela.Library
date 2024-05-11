#ifndef __MEMORY_CACHE_DESCRIPTOR_H__
#define __MEMORY_CACHE_DESCRIPTOR_H__
#include "PrimitiveTypes.h"


struct SMemoryCacheDescriptor
{
	SMemoryCacheDescriptor* psNext;
	SMemoryCacheDescriptor* psPrev;
	size					uiSize;  //Not including the sizeof this.
};


#endif // __MEMORY_CACHE_DESCRIPTOR_H__

