#ifndef __TRACKING_ALLOCATOR_H__
#define __TRACKING_ALLOCATOR_H__
#include "Mallocator.h"
#include "MapPtrPrimitiveTemplate.h"


class CTrackingAllocator : public CMallocator
{
protected:
	CMallocator*						mpcAlloc;
	CArrayBlockSorted					mapv;

public:
	void			Init(CMallocator* pcAlloc);
	void			Kill(void);

	int				AllocatedCount(void);

	void*			Malloc(size_t tSize);
	void*			Realloc(void* pv, size_t tSize);
	BOOL			Free(void* pv);

	//This looks so very wrong.  CONSTRUCTABLE(CCountingAllocator); should be used!
	const char*		ClassName(void);
	unsigned int	ClassSize(void);

	BOOL			IsLocal(void);
	size_t			SizeOffset(void);
};


#endif // __TRACKING_ALLOCATOR_H__

