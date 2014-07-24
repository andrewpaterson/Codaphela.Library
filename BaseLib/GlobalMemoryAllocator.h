#ifndef __GLOBAL_MEMORY_ALLOCATOR_H__
#define __GLOBAL_MEMORY_ALLOCATOR_H__
#include "MemoryAllocator.h"
#include "Memory.h"


class CGlobalMemoryAllocator : public CMemoryAllocator
{
public:
	char*	GetName(void);

	BOOL	IsLocal(void);
};

extern CMemory					gcMemory;
extern CGlobalMemoryAllocator	gcMemoryAllocator;


#endif // __GLOBAL_MEMORY_ALLOCATOR_H__

