#ifndef __GLOBAL_MEMORY_ALLOCATOR_H__
#define __GLOBAL_MEMORY_ALLOCATOR_H__
#include "MemoryAllocator.h"
#include "GeneralMemory.h"


class CGlobalMemoryAllocator : public CMallocator
{
CONSTRUCTABLE(CGlobalMemoryAllocator);
public:
	void		Kill(void);

	void*		Malloc(size uiSize);
	void*		Realloc(void* pv, size uiSize);
	bool		Free(void* pv);

	bool		IsLocal(void);
	size		SizeOffset(void);

	const char*	ShortName(void);
};


extern CGeneralMemory			gcMemory;
extern CGlobalMemoryAllocator	gcMemoryAllocator;


#endif // __GLOBAL_MEMORY_ALLOCATOR_H__

