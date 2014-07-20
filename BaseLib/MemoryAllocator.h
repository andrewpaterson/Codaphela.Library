#ifndef __MEMORY_ALLOCATOR_H__
#define __MEMORY_ALLOCATOR_H__
#include "Memory.h"
#include "Mallocator.h"



class CMemoryAllocator : public CMallocator
{
protected:
	CMemory*	mpcMemory;
	char*		mszName;

public:
	void	Init(CMemory* pcMemory, char* szName);
	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t tSize);
	void	Free(void* pv);
	char*	GetName(void);
};


#endif // __MEMORY_ALLOCATOR_H__
