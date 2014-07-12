#ifndef __GLOBAL_MEMORY_H__
#define __GLOBAL_MEMORY_H__
#include "Memory.h"
#include "MemoryAllocator.h"

extern CMemory				gcMemory;
extern CMemoryAllocator		gcMemoryAllocator;
extern BOOL					gbMemory;


void MemoryInit(void);
void MemoryInit(int iDefaultAlignment, BOOL bDefaultFreeListParams = TRUE);
void MemoryKill(void);


#endif // __GLOBAL_MEMORY_H__

