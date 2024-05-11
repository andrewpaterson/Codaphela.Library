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
	void			Init(void);
	void			Init(int iDefaultAlignment, bool bDefaultFreeListParams = true);
	void			Kill(void);

	void*			Malloc(size uiSize);
	void*			Malloc(size uiSize, char(**pacDebugName)[4]);
	void*			Realloc(void* pv, size uiSize);
	bool			Free(void* pv);
	size			FreeMultiple(CArrayVoidPtr* pav);

	const char*		GetName(void);

	bool			Read(CFileReader* pcFileReader);
	bool			Write(CFileWriter* pcFileWriter);

	CGeneralMemory*	GetMemory(void);
	size			SizeOffset(void);

public:
	static CLifeInit<CMallocator> Create(void);
	static CLifeInit<CMallocator> Create(int iDefaultAlignment, bool bDefaultFreeListParams = true);
};


#endif // __MEMORY_ALLOCATOR_H__

