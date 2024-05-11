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

	void*	Malloc(size uiSize);
	void*	Realloc(void* pv, size iMemSize);
	void	Free(void* pv);

	bool	IsMallocInitialised(void);
	bool	IsMalloc(CMallocator* pcMalloc);
};


#endif // __MALLOC_H__


