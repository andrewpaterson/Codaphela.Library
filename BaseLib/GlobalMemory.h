#ifndef __GLOBAL_MEMORY_H__
#define __GLOBAL_MEMORY_H__
#include "Memory.h"
#include "GlobalMemoryAllocator.h"
#include "Mallocators.h"
#include "Constructors.h"


extern CMallocators				gcMallocators;
extern CConstructors			gcConstructors;
extern BOOL						gbMemory;


void MemoryInit(void);
void MemoryInit(int iDefaultAlignment, BOOL bDefaultFreeListParams = TRUE);
void MemoryKill(void);
BOOL MemoryValidate(void);


#endif // __GLOBAL_MEMORY_H__

