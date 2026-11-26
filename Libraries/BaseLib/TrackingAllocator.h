#ifndef __TRACKING_ALLOCATOR_H__
#define __TRACKING_ALLOCATOR_H__
#include "Mallocator.h"
#include "ArrayBlockSorted.h"


class CTrackingAllocator : public CMallocator
{
protected:
	CMallocator*		mpcAlloc;
	CArrayBlockSorted	mapv;

public:
	void			Init(CMallocator* pcAlloc);
	void			Kill(void) override;

	int				AllocatedCount(void);

	void*			Malloc(lsize uiSize) override;
	void*			Realloc(void* pv, lsize uiSize) override;
	bool			Free(void* pv) override;

	//This looks very wrong.  CONSTRUCTABLE(CCountingAllocator); should be used!
	const char*		ClassName(void) override;
	size			ClassSize(void) override;

	bool			IsLocal(void) const override;
	size			SizeOffset(void) override;

	//This looks very wrong.
	const char*		ShortName(void) override;
};


#endif // __TRACKING_ALLOCATOR_H__

