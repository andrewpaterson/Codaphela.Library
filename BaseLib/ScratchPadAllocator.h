#ifndef __SCRATCH_PAD_ALLOCATOR_H__
#define __SCRATCH_PAD_ALLOCATOR_H__
#include "ScratchPad.h"
#include "LocalMallocator.h"


class CScratchPadAllocator : public CLocalMallocator
{
CONSTRUCTABLE(CScratchPadAllocator);
protected:
	CScratchPad		mcScratchPad;

public:
	void			Init(void);
	void			Init(int iChunkSize);
	void			Kill(void);

	void*			Malloc(size_t tSize);
	void*			Realloc(void* pv, size_t tSize);
	void			Free(void* pv);

	CScratchPad*	GetScratchPad(void);

	const char*		GetName(void);

	BOOL			Read(CFileReader* pcFileReader);
	BOOL			Write(CFileWriter* pcFileWriter);

	size_t			SizeOffset(void);
};


#endif // __SCRATCH_PAD_ALLOCATOR_H__

