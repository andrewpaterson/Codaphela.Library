#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__
#include "BaseLib/PrimitiveTypes.h"


#define SHARED_MEMORY_MAGIC		0xe749c0a8d3c51108
#define SHARED_MEMORY_VALID		0
#define SHARED_MEMORY_INVALID	-1


struct SSharedMemory
{
	uint64	uiMagic;
	uint64	uiSize;
	int		iInvalid;
	int		iMapCount;
	char	szName[64];
};


#ifdef _MSC_VER
#include "WindowsSharedMemory.h"
#endif // _MSC_VER


#endif // __SHARED_MEMORY_H__



