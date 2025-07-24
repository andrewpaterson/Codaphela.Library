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

	void*			Malloc(size uiSize);
	void*			Realloc(void* pv, size uiSize);
	bool			Free(void* pv);

	//This looks very wrong.  CONSTRUCTABLE(CCountingAllocator); should be used!
	const char*		ClassName(void);
	size			ClassSize(void);

	bool			IsLocal(void);
	size			SizeOffset(void);

	//This looks very wrong. 
	const char*		ShortName(void);
};


#endif // __TRACKING_ALLOCATOR_H__

