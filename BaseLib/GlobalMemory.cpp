#include "Mallocators.h"
#include "Logger.h"
#include "GlobalMemory.h"


CMallocators			gcMallocators;
CConstructors			gcConstructors;
bool					gbMemory = false;


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
	gbMemory = true;

	gcMallocators.Init();
	gcConstructors.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MemoryInit(int iDefaultAlignment, bool bDefaultFreeListParams)
{
	if (gbMemory)
	{
		gcLogger.Error("Global Memory has already been initialised.");
	}

	gcMemory.Init(iDefaultAlignment, bDefaultFreeListParams);
	gbMemory = true;

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
	gbMemory = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool MemoryValidate(void)
{
	if (!gbMemory)
	{
		gcLogger.Error("Global Memory has not been initialised.  Call MemoryInit().");
		return false;
	}
	else
	{
		return true;
	}
}

