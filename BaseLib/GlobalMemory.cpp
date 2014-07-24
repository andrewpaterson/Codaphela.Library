#include "Mallocators.h"
#include "GlobalMemory.h"

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

