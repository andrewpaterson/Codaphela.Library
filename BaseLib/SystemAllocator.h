#ifndef __SYSTEM_ALLOCATOR_H__
#define __SYSTEM_ALLOCATOR_H__
#include "Mallocator.h"


class CSystemAllocator : public CMallocator
{
public:
	void*		Malloc(size_t tSize);
	void*		Realloc(void* pv, size_t iMemSize);
	void		Free(void* pv);

	const char*	GetName(void);

	BOOL		IsLocal(void);
	size_t		SizeOffset(void);
};


extern CSystemAllocator		gcSystemAllocator;


#endif // __SYSTEM_ALLOCATOR_H__

