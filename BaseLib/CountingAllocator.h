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
	size_t			mtUserSize;
	size_t			mtSystemSize;

public:
	void			Init(CMallocator* pcAlloc);
	void			Kill(void);

	size_t			AllocatedUserSize(void);
	size_t			AllocatedSystemSize(void);

	void*			Malloc(size_t tSize);
	void*			Realloc(void* pv, size_t tSize);
	bool			Free(void* pv);

	//This looks so very wrong.  CONSTRUCTABLE(CCountingAllocator); should be used!
	const char*		ClassName(void);
	uint32	ClassSize(void);

	bool			IsLocal(void);
	size_t			SizeOffset(void);

	//This looks so very wrong.  CONSTRUCTABLE(CCountingAllocator); should be used!
	const char*		ShortName(void);
};


#endif // __COUNTING_ALLOCATOR_H__

