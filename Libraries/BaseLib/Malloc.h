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

	void*	Malloc(lsize uiSize);
	void*	Realloc(void* pv, lsize iMemSize);
	void	Free(void* pv);

	bool	IsMallocInitialised(void) const;
	bool	IsMalloc(CMallocator* pcMalloc) const;

	CMallocator*	GetMallocator(void) const;
};


#endif // __MALLOC_H__


