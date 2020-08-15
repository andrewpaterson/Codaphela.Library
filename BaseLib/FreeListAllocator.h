#ifndef __FREE_LIST_ALLOCATOR_H__
#define __FREE_LIST_ALLOCATOR_H__
#include "LocalMallocator.h"
#include "FreeList.h"
#include "LifeCycle.h"


class CFreeListAllocator : public CLocalMallocator
{
CONSTRUCTABLE(CFreeListAllocator);
protected:
	CFreeList	mcFreeList;

public:
	void			Init(int iElementSize);
	void			Init(int iElementSize, int iAlignment);
	void			Init(int iElementSize, int iAlignment, int iOffset);
	void			Kill(void);

	void*			Malloc(size_t tSize);
	void*			Realloc(void* pv, size_t tSize);
	void			Free(void* pv);

	CFreeList*		GetFreeList(void);

	const char*		GetName(void);

	BOOL			Read(CFileReader* pcFileReader);
	BOOL			Write(CFileWriter* pcFileWriter);

	size_t			SizeOffset(void);

public:
	static CLifeInit<CMallocator> Create(int iElementSize);
	static CLifeInit<CMallocator> Create(int iElementSize, int iAlignment);
	static CLifeInit<CMallocator> Create(int iElementSize, int iAlignment, int iOffset);
};


#endif // __FREE_LIST_ALLOCATOR_H__

