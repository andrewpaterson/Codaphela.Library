#ifndef __NULL_ALLOCATOR_H__
#define __NULL_ALLOCATOR_H__
#include "Mallocator.h"


class CNullAllocator : public CMallocator
{
CONSTRUCTABLE(CNullAllocator);
public:
	void		Kill(void);

	void*		Malloc(size uiSize);
	void*		Realloc(void* pv, size iMemSize);
	bool		Free(void* pv);

	const char*	GetName(void);

	bool		IsLocal(void);
	size		SizeOffset(void);

	const char*	ShortName(void);
};


extern CNullAllocator	gcNullAllocator;


#endif // __NULL_ALLOCATOR_H__

