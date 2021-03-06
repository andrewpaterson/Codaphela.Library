#ifndef __DATA_FREE_H__
#define __DATA_FREE_H__
#include "DataCallback.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CDataFreeCallBack::Init(DataFree fDataFree)
{
	mDataFree = fDataFree;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CDataFreeCallBack::DataWillBeFreed(void* pvData)
{
	mDataFree(pvData);
}


#endif // !__DATA_FREE_H__

