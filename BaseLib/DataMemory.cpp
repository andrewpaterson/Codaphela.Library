#include "DataMemory.h"


CMemoryFreeListParams gcDataMemoryFreeListParams;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::Init(void)
{
	Init(&gcDataMemoryFreeListParams);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::Init(CMemoryFreeListParams* pcFreeListParams)
{
	mpcFreeListParams = pcFreeListParams;
	mcFreeLists.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::Kill(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataMemoryInit(void)
{
	gcDataMemoryFreeListParams.Init(sizeof(SDataMemoryAllocation), TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataMemoryKill(void)
{
	gcDataMemoryFreeListParams.Kill();
}


