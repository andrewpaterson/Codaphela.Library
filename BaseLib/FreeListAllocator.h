#ifndef __FREE_LIST_ALLOCATOR_H__
#define __FREE_LIST_ALLOCATOR_H__
#include "LocalMallocator.h"
#include "FreeList.h"


class CFreeListAllocator : public CLocalMallocator
{
protected:
	CFreeList*	mpcFreeList;

public:
	void	Init(CFreeList* pcFreeList);

	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t iMemSize);
	void	Free(void* pv);

	char*	GetName(void);

	BOOL	Read(CFileReader* pcFileReader);
	BOOL	Write(CFileWriter* pcFileWriter);
};


#endif // __FREE_LIST_ALLOCATOR_H__

