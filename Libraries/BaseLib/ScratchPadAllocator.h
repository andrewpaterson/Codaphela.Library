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
	void			Kill(void) override;

	void*			Malloc(lsize uiSize) override;
	void*			Realloc(void* pv, lsize uiSize) override;
	bool			Free(void* pv) override;

	CScratchPad*	GetScratchPad(void);

	const char*		GetName(void) const;

	bool			Read(CFileReader* pcFileReader) override;
	bool			Write(CFileWriter* pcFileWriter) override;

	size			SizeOffset(void) override;

public:
	static CLifeInit<CMallocator>	Create(void);
	static CLifeInit<CMallocator>	Create(size uiChunkSize);
};


#endif // __SCRATCH_PAD_ALLOCATOR_H__

