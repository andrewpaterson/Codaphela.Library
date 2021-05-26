#include "Mallocators.h"
#include "Logger.h"
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
	if (gbMemory)
	{
		gcLogger.Error("Global Memory has already been initialised.");
	}

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
	if (gbMemory)
	{
		gcLogger.Error("Global Memory has already been initialised.");
	}

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
	if (!gbMemory)
	{
		gcLogger.Error("Global Memory has already been Killed.");
	}

	gcConstructors.Kill();
	gcMallocators.Kill();

	gcMemory.Kill();
	gbMemory = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL MemoryValidate(void)
{
	if (!gbMemory)
	{
		gcLogger.Error("Global Memory has not been initialised.  Call MemoryInit().");
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

