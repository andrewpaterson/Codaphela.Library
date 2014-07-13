#ifndef __GLOBAL_MEMORY_H__
#define __GLOBAL_MEMORY_H__
#include "Memory.h"
#include "MemoryAllocator.h"
#include "Mallocators.h"

extern CMemory				gcMemory;
extern CMemoryAllocator		gcMemoryAllocator;
extern CMallocators			gcMallocators;
extern BOOL					gbMemory;


void MemoryInit(void);
void MemoryInit(int iDefaultAlignment, BOOL bDefaultFreeListParams = TRUE);
void MemoryKill(void);


#endif // __GLOBAL_MEMORY_H__

