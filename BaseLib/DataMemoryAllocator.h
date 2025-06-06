#ifndef __DATA_MEMORY_ALLOCATOR_H__
#define __DATA_MEMORY_ALLOCATOR_H__
#include "DataMemory.h"
#include "LocalMallocator.h"
#include "LifeCycle.h"


class CDataMemoryAllocator : public CLocalMallocator
{
CONSTRUCTABLE(CDataMemoryAllocator);
protected:
	CDataMemory		mcMemory;

public:
	void			Init(void);
	void			Init(CMemoryFreeListParams* pcFreeListParams);
	void			Kill(void);

	void*			Malloc(size uiSize) override;
	void*			Malloc(size uiSize, char(**pacDebugName)[4]) override;
	void*			Realloc(void* pv, size uiSize);
	bool			Free(void* pv) override;
	size			FreeMultiple(CArrayVoidPtr* pav) override;

	const char*		GetName(void);

	bool			Read(CFileReader* pcFileReader) override;
	bool			Write(CFileWriter* pcFileWriter) override;

	CDataMemory*	GetMemory(void);
	size			SizeOffset(void);

public:
	static CLifeInit<CMallocator> Create(void);
	static CLifeInit<CMallocator> Create(CMemoryFreeListParams* pcFreeListParams);
};


#endif // __DATA_MEMORY_ALLOCATOR_H__

