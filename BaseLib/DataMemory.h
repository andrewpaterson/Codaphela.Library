#ifndef  __DATA_MEMORY_H__
#define __DATA_MEMORY_H__
#include "MemoryFreeListParams.h"
#include "FreeListMaybe.h"

extern CMemoryFreeListParams gcDataMemoryFreeListParams;


struct SDataMemoryAllocation
{
	SFNode*			psFreeListNode;
	unsigned int	uiSize;
};


class CDataMemory
{
private:
	CLinkListFreeListMaybe	mcFreeLists;

	CMemoryFreeListParams*	mpcFreeListParams;

public:
	void Init(void);
	void Init(CMemoryFreeListParams* pcFreeListParams);
	void Kill(void);
};


void DataMemoryInit(void);
void DataMemoryKill(void);


#endif // __DATA_MEMORY_H__

