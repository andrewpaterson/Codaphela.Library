#ifndef __FREE_LIST_ALLOCATOR_H__
#define __FREE_LIST_ALLOCATOR_H__
#include "LocalMallocator.h"
#include "FreeList.h"


class CFreeListAllocator : public CLocalMallocator
{
protected:
	CFreeList	mcFreeList;

public:
	void		Init(int iChunkSize, int iElementSize);
	void		Init(int iChunkSize, int iElementSize, int iAlignment);
	void		Init(int iChunkSize, int iElementSize, int iAlignment, int iOffset);
	void		Kill(void);

	void*		Malloc(size_t tSize);
	void*		Realloc(void* pv, size_t tSize);
	void		Free(void* pv);

	CFreeList*	GetFreeList(void);

	char*		GetName(void);

	BOOL		Read(CFileReader* pcFileReader);
	BOOL		Write(CFileWriter* pcFileWriter);
};


#endif // __FREE_LIST_ALLOCATOR_H__

