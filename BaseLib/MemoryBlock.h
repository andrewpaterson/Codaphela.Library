#ifndef __MEMORY_BLOCK_H__
#define __MEMORY_BLOCK_H__
#include "Define.h"
#include "Mallocator.h"
#include "Malloc.h"

class CMemoryBlock : public CMalloc
{
protected:
	void*			mpvMem;
	size			mtSize;

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);
	void*	GetMemory(void);
	void*	Allocate(size uiSize);
	size	GetSize(void);

protected:	
	void*	Malloc(size uiSize);
	void*	Realloc(void* pv, size uiSize);
	void	Free(void* pv);
};


#endif // __MEMORY_BLOCK_H__

