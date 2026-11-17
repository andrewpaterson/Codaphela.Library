#ifndef __SYSTEM_ALLOCATOR_H__
#define __SYSTEM_ALLOCATOR_H__
#include "Mallocator.h"


class CSystemAllocator : public CMallocator
{
CONSTRUCTABLE(CSystemAllocator)
public:
	void		Kill(void) override;

	void*		Malloc(lsize uiSize) override;
	void*		Realloc(void* pv, lsize iMemSize) override;
	bool		Free(void* pv) override;

	bool		IsLocal(void) const override;
	size		SizeOffset(void) override;

	const char*	ShortName(void) override;
};


extern CSystemAllocator		gcSystemAllocator;


#endif // __SYSTEM_ALLOCATOR_H__

