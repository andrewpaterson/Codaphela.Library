#ifndef __SCRATCH_PAD_ALLOCATOR_H__
#define __SCRATCH_PAD_ALLOCATOR_H__
#include "ScratchPad.h"
#include "LocalMallocator.h"
#include "LifeCycle.h"


class CScratchPadAllocator : public CLocalMallocator
{
CONSTRUCTABLE(CScratchPadAllocator)
protected:
	CScratchPad		mcScratchPad;

public:
	void			Init(void);
	void			Init(size uiChunkSize);
	void			Kill(void);

	void*			Malloc(lsize uiSize);
	void*			Realloc(void* pv, lsize uiSize);
	bool			Free(void* pv);

	CScratchPad*	GetScratchPad(void);

	const char*		GetName(void);

	bool			Read(CFileReader* pcFileReader);
	bool			Write(CFileWriter* pcFileWriter);

	size			SizeOffset(void);

public:
	static CLifeInit<CMallocator> Create(void);
	static CLifeInit<CMallocator> Create(size uiChunkSize);
};


#endif // __SCRATCH_PAD_ALLOCATOR_H__

