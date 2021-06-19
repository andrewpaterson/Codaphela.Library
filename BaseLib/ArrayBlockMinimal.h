#ifndef __ARRAY_BLOCK_MINIMAL_H__
#define __ARRAY_BLOCK_MINIMAL_H__
#include "Define.h"
#include "PointerRemapper.h"
#include "PointerFunctions.h"
#include "ErrorHandler.h"
#include "FileIO.h"
#include "Mallocator.h"
#include "Malloc.h"


class CArrayBlockMinimal : public CMalloc
{
protected:
	int 			miUsedElements;

public:
	void Init(void);
	void Init(CMallocator *pcMalloc);
	void Kill(void);

	BOOL WriteHeader(CFileWriter *pcFileWriter);
	BOOL WriteAllocatorAndHeader(CFileWriter *pcFileWriter);
	BOOL ReadAllocator(CFileReader* pcFileReader);
};


#endif // !__ARRAY_BLOCK_MINIMAL_H__

