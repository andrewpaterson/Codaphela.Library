#ifndef __NULL_ALLOCATOR_H__
#define __NULL_ALLOCATOR_H__
#include "Mallocator.h"


class CNullAllocator : public CMallocator
{
CONSTRUCTABLE(CNullAllocator)
public:
	void		Kill(void) override;

	void*		Malloc(lsize uiSize) override;
	void*		Realloc(void* pv, lsize iMemSize) override;
	bool		Free(void* pv) override;

	const char*	GetName(void);

	bool		IsLocal(void) override;
	size		SizeOffset(void) override;

	const char*	ShortName(void) override;
};


extern CNullAllocator	gcNullAllocator;


#endif // __NULL_ALLOCATOR_H__

