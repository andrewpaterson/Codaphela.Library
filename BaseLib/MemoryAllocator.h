#ifndef __MEMORY_ALLOCATOR_H__
#define __MEMORY_ALLOCATOR_H__
#include "GeneralMemory.h"
#include "LocalMallocator.h"


class CMemoryAllocator : public CLocalMallocator
{
CONSTRUCTABLE(CMemoryAllocator);
protected:
	CGeneralMemory		mcMemory;

public:
	void			Init(void);
	void			Init(int iDefaultAlignment, BOOL bDefaultFreeListParams = TRUE);
	void			Kill(void);

	void*			Malloc(size_t tSize);
	void*			Realloc(void* pv, size_t tSize);
	void			Free(void* pv);

	const char*		GetName(void);

	BOOL			Read(CFileReader* pcFileReader);
	BOOL			Write(CFileWriter* pcFileWriter);

	CGeneralMemory*	GetMemory(void);
	size_t			SizeOffset(void);
};


#endif // __MEMORY_ALLOCATOR_H__

