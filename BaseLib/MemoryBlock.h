#ifndef __MEMORY_BLOCK_H__
#define __MEMORY_BLOCK_H__
#include "Define.h"
#include "Mallocator.h"


class CMemoryBlock
{
protected:
	CMallocator*	mpcMalloc;
	void*			mpvMem;
	size_t			mtSize;

public:
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);
	void*	GetMemory(void);
	void*	Allocate(size_t tSize);
	size_t	GetSize(void);

protected:	
	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t tSize);
	void	Free(void* pv);
};


#endif // __MEMORY_BLOCK_H__

