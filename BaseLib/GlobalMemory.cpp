#include "Mallocators.h"
#include "GlobalMemory.h"

CMemory					gcMemory;
CMemoryAllocator		gcMemoryAllocator;
CMallocators			gcMallocators;
CConstructors			gcConstructors;
BOOL					gbMemory = FALSE;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MemoryInit(void)
{
	gcMemory.Init();
	gcMemoryAllocator.Init(&gcMemory, "CMemoryAllocator", TRUE);
	gbMemory = TRUE;

	gcMallocators.Init();
	gcConstructors.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MemoryInit(int iDefaultAlignment, BOOL bDefaultFreeListParams)
{
	gcMemory.Init(iDefaultAlignment, bDefaultFreeListParams);
	gcMemoryAllocator.Init(&gcMemory, "CMemoryAllocator", TRUE);
	gbMemory = TRUE;

	gcMallocators.Init();
	gcConstructors.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MemoryKill(void)
{
	gcConstructors.Kill();
	gcMallocators.Kill();

	gcMemory.Kill();
	gbMemory = FALSE;
}

