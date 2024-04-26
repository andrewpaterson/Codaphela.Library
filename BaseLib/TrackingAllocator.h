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
	void			Kill(void);

	int				AllocatedCount(void);

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


#endif // __TRACKING_ALLOCATOR_H__

