#ifndef __GLOBAL_MEMORY_ALLOCATOR_H__
#define __GLOBAL_MEMORY_ALLOCATOR_H__
#include "MemoryAllocator.h"
#include "GeneralMemory.h"


class CGlobalMemoryAllocator : public CMallocator
{
public:
	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t tSize);
	void	Free(void* pv);

	char*	GetName(void);

	BOOL	IsLocal(void);
};


extern CGeneralMemory			gcMemory;
extern CGlobalMemoryAllocator	gcMemoryAllocator;


#endif // __GLOBAL_MEMORY_ALLOCATOR_H__

