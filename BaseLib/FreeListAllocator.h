#ifndef __FREE_LIST_ALLOCATOR_H__
#define __FREE_LIST_ALLOCATOR_H__
#include "Mallocator.h"
#include "FreeList.h"


class CFreeListAllocator : public CMallocator
{
protected:
	CFreeList*	mpcFreeList;
	char*		mszName;

public:
	void	Init(CFreeList* pcFreeList, char* szName);

	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t iMemSize);
	void	Free(void* pv);
	char*	GetName(void);
};


#endif // __FREE_LIST_ALLOCATOR_H__

