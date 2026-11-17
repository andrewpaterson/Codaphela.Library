#ifndef __COUNTING_ALLOCATOR_H__
#define __COUNTING_ALLOCATOR_H__
#include "Mallocator.h"
#include "PrimitiveTypes.h"


struct SCountingMemoryAllocation
{
	lsize			uiSize;
};


class CCountingAllocator : public CMallocator
{
protected:
	CMallocator*	mpcAlloc;
	lsize			mtUserSize;
	lsize			mtSystemSize;

public:
	void			Init(CMallocator* pcAlloc);
	void			Kill(void) override;

	lsize			AllocatedUserSize(void);
	lsize			AllocatedSystemSize(void);

	void*			Malloc(lsize uiSize) override;
	void*			Realloc(void* pv, lsize uiSize) override;
	bool			Free(void* pv) override;

	//This looks so very wrong.  CONSTRUCTABLE(CCountingAllocator) should be used!
	const char*		ClassName(void) override;
	size			ClassSize(void) override;

	bool			IsLocal(void) const override;
	size			SizeOffset(void) override;

	//This looks so very wrong.  CONSTRUCTABLE(CCountingAllocator) should be used!
	const char*		ShortName(void) override;
};


#endif // __COUNTING_ALLOCATOR_H__

