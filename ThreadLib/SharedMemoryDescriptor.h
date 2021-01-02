#ifndef __SHARED_MEMORY_DESCRIPTOR_H__
#define __SHARED_MEMORY_DESCRIPTOR_H__
#include "BaseLib/PrimitiveTypes.h"


#define SHARED_MEMORY_MAGIC		0xe749c0a8d3c51108
#define SHARED_MEMORY_VALID		0
#define SHARED_MEMORY_INVALID	-1


struct SSharedMemoryDescriptor
{
	uint64	uiMagic;
	uint64	uiSize;
	int		iInvalid;
	int		iMapCount;
	char	szName[64];
};


struct SSharedMemoryResize
{
	void*	pvMemory;
	size_t	uiSize;

	SSharedMemoryResize(void* pvMemory, size_t uiSize)
	{
		this->pvMemory = pvMemory;
		this->uiSize = uiSize;
	}
};


#endif // !__SHARED_MEMORY_DESCRIPTOR_H__

