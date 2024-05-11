#ifndef __COUNTING_ALLOCATOR_H__
#define __COUNTING_ALLOCATOR_H__
#include "Mallocator.h"
#include "PrimitiveTypes.h"


struct SCountingMemoryAllocation
{
	size			uiSize;
};


class CCountingAllocator : public CMallocator
{
protected:
	CMallocator*	mpcAlloc;
	size			mtUserSize;
	size			mtSystemSize;

public:
	void			Init(CMallocator* pcAlloc);
	void			Kill(void);

	size			AllocatedUserSize(void);
	size			AllocatedSystemSize(void);

	void*			Malloc(size uiSize);
	void*			Realloc(void* pv, size uiSize);
	bool			Free(void* pv);

	//This looks so very wrong.  CONSTRUCTABLE(CCountingAllocator); should be used!
	const char*		ClassName(void);
	size			ClassSize(void);

	bool			IsLocal(void);
	size			SizeOffset(void);

	//This looks so very wrong.  CONSTRUCTABLE(CCountingAllocator); should be used!
	const char*		ShortName(void);
};


#endif // __COUNTING_ALLOCATOR_H__

