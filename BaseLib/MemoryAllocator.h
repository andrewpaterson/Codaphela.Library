#ifndef __MEMORY_ALLOCATOR_H__
#define __MEMORY_ALLOCATOR_H__
#include "Memory.h"
#include "LocalMallocator.h"


class CMemoryAllocator : public CLocalMallocator
{
protected:
	CMemory*	mpcMemory;

public:
	void	Init(CMemory* pcMemory);

	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t tSize);
	void	Free(void* pv);

	char*	GetName(void);

	BOOL	Read(CFileReader* pcFileReader);
	BOOL	Write(CFileWriter* pcFileWriter);
};


#endif // __MEMORY_ALLOCATOR_H__

