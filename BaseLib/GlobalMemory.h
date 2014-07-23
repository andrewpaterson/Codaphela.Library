#ifndef __GLOBAL_MEMORY_H__
#define __GLOBAL_MEMORY_H__
#include "Memory.h"
#include "MemoryAllocator.h"
#include "Mallocators.h"
#include "Constructors.h"


extern CMemory				gcMemory;
extern CMemoryAllocator		gcMemoryAllocator;
extern CMallocators			gcMallocators;
extern CConstructors		gcConstructors;
extern BOOL					gbMemory;


void MemoryInit(void);
void MemoryInit(int iDefaultAlignment, BOOL bDefaultFreeListParams = TRUE);
void MemoryKill(void);


#endif // __GLOBAL_MEMORY_H__

