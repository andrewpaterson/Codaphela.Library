#ifndef __ALLOC_H__
#define __ALLOC_H__
#include "Mallocator.h"


class CAlloc
{
protected:
	CMallocator* mpcMalloc;

public:
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);

	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t iMemSize);
	void	Free(void* pv);
};


#endif // __ALLOC_H__


