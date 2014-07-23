#ifndef __MEMORY_ALLOCATOR_H__
#define __MEMORY_ALLOCATOR_H__
#include "Memory.h"
#include "Mallocator.h"



class CMemoryAllocator : public CMallocator
{
protected:
	CMemory*	mpcMemory;
	char*		mszName;
	BOOL		mbGlobal;

public:
	void	Init(CMemory* pcMemory, char* szName, BOOL bGlobal = FALSE);

	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t tSize);
	void	Free(void* pv);

	char*	GetName(void);

	BOOL	Read(CFileReader* pcFileReader);
	BOOL	Write(CFileWriter* pcFileReader);
};


#endif // __MEMORY_ALLOCATOR_H__

