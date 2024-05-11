#ifndef __GLOBAL_MEMORY_H__
#define __GLOBAL_MEMORY_H__
#include "GeneralMemory.h"
#include "GlobalMemoryAllocator.h"
#include "Mallocators.h"
#include "Constructors.h"


extern CMallocators				gcMallocators;
extern CConstructors			gcConstructors;
extern bool						gbMemory;


void MemoryInit(void);
void MemoryInit(uint16 iDefaultAlignment, bool bDefaultFreeListParams = true);
void MemoryKill(void);
bool MemoryValidate(void);


#endif // __GLOBAL_MEMORY_H__

