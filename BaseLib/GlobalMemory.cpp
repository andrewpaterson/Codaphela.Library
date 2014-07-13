#include "Mallocators.h"
#include "GlobalMemory.h"

CMemory					gcMemory;
CMemoryAllocator		gcMemoryAllocator;
CMallocators			gcMallocators;
BOOL					gbMemory = FALSE;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MemoryInit(void)
{
	gcMemory.Init();
	gcMemoryAllocator.Init(&gcMemory, "CMemoryAllocator.Global");
	gbMemory = TRUE;

	gcMallocators.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MemoryInit(int iDefaultAlignment, BOOL bDefaultFreeListParams)
{
	gcMemory.Init(iDefaultAlignment, bDefaultFreeListParams);
	gcMemoryAllocator.Init(&gcMemory, "CMemoryAllocator.Global");
	gbMemory = TRUE;

	gcMallocators.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MemoryKill(void)
{
	gcMallocators.Kill();

	gcMemory.Kill();
	gbMemory = FALSE;
}

