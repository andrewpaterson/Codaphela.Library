#ifndef __COUNTING_ALLOCATOR_H__
#define __COUNTING_ALLOCATOR_H__
#include "Mallocator.h"
#include "PrimitiveTypes.h"


struct SCountingMemoryAllocation
{
	size_t			tSize;
};


class CCountingAllocator : public CMallocator
{
protected:
	CMallocator*	mpcAlloc;
	size_t			mtSize;

public:
	void		Init(CMallocator* pcAlloc);
	void		Kill(void);

	size_t		AllocatedSize(void);

	void*		Malloc(size_t tSize);
	void*		Realloc(void* pv, size_t tSize);
	void		Free(void* pv);

	const char*	GetName(void);

	BOOL		IsLocal(void);
	size_t		SizeOffset(void);
};


#endif // __COUNTING_ALLOCATOR_H__

