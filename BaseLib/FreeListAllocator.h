#ifndef __FREE_LIST_ALLOCATOR_H__
#define __FREE_LIST_ALLOCATOR_H__
#include "Mallocator.h"
#include "FreeListBlock.h"


class CFreeListAllocator : public CMallocator
{
protected:
	CFreeListBlock*	mpcFreeList;

public:
	void	Init(CFreeListBlock* pcFreeList);

	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t iMemSize);
	void	Free(void* pv);
};


#endif // __FREE_LIST_ALLOCATOR_H__

