#ifndef __FREE_LIST_ALLOCATOR_H__
#define __FREE_LIST_ALLOCATOR_H__
#include "LocalMallocator.h"
#include "FreeList.h"
#include "LifeCycle.h"


class CFreeListAllocator : public CLocalMallocator
{
CONSTRUCTABLE(CFreeListAllocator)
protected:
	CFreeList	mcFreeList;

public:
	void			Init(int iElementSize);
	void			Init(int iElementSize, int iAlignment);
	void			Init(int iElementSize, int iAlignment, int iOffset);
	void			Kill(void) override;

	void*			Malloc(lsize uiSize) override;
	void*			Realloc(void* pv, lsize uiSize) override;
	bool			Free(void* pv) override;

	CFreeList*		GetFreeList(void);

	const char*		GetName(void) const;

	bool			Read(CFileReader* pcFileReader) override;
	bool			Write(CFileWriter* pcFileWriter) override;

	size			SizeOffset(void) override;

public:
	static CLifeInit<CMallocator>	Create(int iElementSize);
	static CLifeInit<CMallocator>	Create(int iElementSize, int iAlignment);
	static CLifeInit<CMallocator>	Create(int iElementSize, int iAlignment, int iOffset);
};


#endif // __FREE_LIST_ALLOCATOR_H__

