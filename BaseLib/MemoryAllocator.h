#ifndef __MEMORY_ALLOCATOR_H__
#define __MEMORY_ALLOCATOR_H__
#include "GeneralMemory.h"
#include "LocalMallocator.h"
#include "LifeCycle.h"


class CMemoryAllocator : public CLocalMallocator
{
CONSTRUCTABLE(CMemoryAllocator);
protected:
	CGeneralMemory		mcMemory;

public:
	void				Init(void);
	void				Init(int iDefaultAlignment, bool bDefaultFreeListParams = true);
	void				Kill(void);

	void*				Malloc(size uiSize) override;
	void*				Malloc(size uiSize, char(**pacDebugName)[4]) override;
	void*				Realloc(void* pv, size uiSize) override;
	bool				Free(void* pv) override;
	size				FreeMultiple(CArrayVoidPtr* pav) override;

	const char*			GetName(void);

	bool				Read(CFileReader* pcFileReader) override;
	bool				Write(CFileWriter* pcFileWriter) override;

	CGeneralMemory*		GetMemory(void);
	size				SizeOffset(void);

public:
	static CLifeInit<CMallocator> Create(void);
	static CLifeInit<CMallocator> Create(int iDefaultAlignment, bool bDefaultFreeListParams = true);
};


#endif // __MEMORY_ALLOCATOR_H__

