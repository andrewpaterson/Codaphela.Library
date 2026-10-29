#ifndef __SYSTEM_ALLOCATOR_H__
#define __SYSTEM_ALLOCATOR_H__
#include "Mallocator.h"


class CSystemAllocator : public CMallocator
{
CONSTRUCTABLE(CSystemAllocator)
public:
	void		Kill(void);

	void*		Malloc(lsize uiSize);
	void*		Realloc(void* pv, lsize iMemSize);
	bool		Free(void* pv);

	bool		IsLocal(void);
	size		SizeOffset(void);

	const char*	ShortName(void);
};


extern CSystemAllocator		gcSystemAllocator;


#endif // __SYSTEM_ALLOCATOR_H__

