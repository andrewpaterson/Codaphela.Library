#include "FreeListAllocator.h"
#include "MemoryAllocator.h"
#include "ScratchPadAllocator.h"
#include "LocalMallocatorConstructors.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LocalMallocatorsInit(CConstructors* pcConstructors, CMallocators* pcMallocatos)
{
	CFreeListAllocator*		pcFreeListAllocator;
	CMemoryAllocator*		pcMemoryAllocator;
	CScratchPadAllocator*	pcScratchPadAllocator;

	pcFreeListAllocator = pcConstructors->Add<CFreeListAllocator>();
	pcMemoryAllocator = pcConstructors->Add<CMemoryAllocator>();
	pcScratchPadAllocator = pcConstructors->Add<CScratchPadAllocator>();

	pcMallocatos->AddMallocator(pcFreeListAllocator);
	pcMallocatos->AddMallocator(pcMemoryAllocator);
	pcMallocatos->AddMallocator(pcScratchPadAllocator);
}

