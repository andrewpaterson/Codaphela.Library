#ifndef  __DATA_MEMORY_H__
#define __DATA_MEMORY_H__
#include "MemoryHeader.h"
#include "MemoryFreeListParams.h"
#include "FreeList.h"

extern CMemoryFreeListParams gcDataMemoryFreeListParams;


class CDataMemory
{
private:
	CLinkListFreeList		mcFreeLists;

	CMemoryFreeListParams*	mpcFreeListParams;

public:
	void Init(void);
	void Init(CMemoryFreeListParams* pcFreeListParams);
	void Kill(void);
};


void DataMemoryInit(void);
void DataMemoryKill(void);


#endif // __DATA_MEMORY_H__

