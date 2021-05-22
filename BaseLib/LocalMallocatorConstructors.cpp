#include "FreeListAllocator.h"
#include "MemoryAllocator.h"
#include "ScratchPadAllocator.h"
#include "LocalMallocatorConstructors.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LocalMallocatorsInit(CConstructors* pcConstructors, CMallocators* pcMallocators)
{
	CFreeListAllocator*		pcFreeListAllocator;
	CMemoryAllocator*		pcMemoryAllocator;
	CScratchPadAllocator*	pcScratchPadAllocator;

	pcFreeListAllocator = pcConstructors->Add<CFreeListAllocator>();
	pcMemoryAllocator = pcConstructors->Add<CMemoryAllocator>();
	pcScratchPadAllocator = pcConstructors->Add<CScratchPadAllocator>();

	pcMallocators->Add(pcFreeListAllocator);
	pcMallocators->Add(pcMemoryAllocator);
	pcMallocators->Add(pcScratchPadAllocator);
}

