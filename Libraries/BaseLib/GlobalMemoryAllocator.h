#ifndef __GLOBAL_MEMORY_ALLOCATOR_H__
#define __GLOBAL_MEMORY_ALLOCATOR_H__
#include "MemoryAllocator.h"
#include "GeneralMemory.h"


class CGlobalMemoryAllocator : public CMallocator
{
CONSTRUCTABLE(CGlobalMemoryAllocator)
public:
	void		Kill(void) override;

	void*		Malloc(lsize uiSize) override;
	void*		Realloc(void* pv, lsize uiSize) override;
	bool		Free(void* pv) override;

	bool		IsLocal(void) override;
	size		SizeOffset(void) override;

	const char*	ShortName(void) override;
};


extern CGeneralMemory			gcMemory;
extern CGlobalMemoryAllocator	gcMemoryAllocator;


#endif // __GLOBAL_MEMORY_ALLOCATOR_H__

