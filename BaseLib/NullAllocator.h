#ifndef __NULL_ALLOCATOR_H__
#define __NULL_ALLOCATOR_H__
#include "Mallocator.h"


class CNullAllocator : public CMallocator
{
public:
	void*		Malloc(size_t tSize);
	void*		Realloc(void* pv, size_t iMemSize);
	void		Free(void* pv);

	const char*	GetName(void);

	BOOL		IsLocal(void);
	size_t		SizeOffset(void);
};


extern CNullAllocator	gcNullAllocator;


#endif // __NULL_ALLOCATOR_H__

