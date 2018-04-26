#ifndef __TRACKING_ALLOCATOR_H__
#define __TRACKING_ALLOCATOR_H__
#include "Mallocator.h"
#include "MapPtrPrimitiveTemplate.h"


class CTrackingAllocator : public CMallocator
{
protected:
	CMallocator*						mpcAlloc;
	CMapPtrPrimitiveTemplate<size_t>	mmpiSizes;

public:
	void		Init(CMallocator* pcAlloc);
	void		Kill(void);

	size_t		AllocatedSize(void);

	void*		Malloc(size_t tSize);
	void*		Realloc(void* pv, size_t tSize);
	void		Free(void* pv);

	const char*	GetName(void);

	BOOL		IsLocal(void);
};


#endif // __TRACKING_ALLOCATOR_H__

