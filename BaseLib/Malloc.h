#ifndef __MALLOC_H__
#define __MALLOC_H__
#include "Mallocator.h"


class CMalloc
{
protected:
	CMallocator*	mpcMalloc;

public:
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);

	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t iMemSize);
	void	Free(void* pv);

	bool	IsMallocInitialised(void);
	bool	IsMalloc(CMallocator* pcMalloc);
};


#endif // __MALLOC_H__


