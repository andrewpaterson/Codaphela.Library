#include "EvictedList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEvictedList::Init(void)
{
	mcKeys.Init();
	mcDatas.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEvictedList::Kill(void)
{
	mcDatas.Kill();
	mcKeys.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CEvictedList::NodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	mcKeys.Add(pvKey, iKeySize + 1);
	mcDatas.Add(pvData, iDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEvictedList::NumElements(void)
{
	return mcKeys.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CEvictedList::GetKey(int iIndex, int* piKeySize)
{
	if (piKeySize)
	{
		*piKeySize = mcKeys.GetSize(iIndex);
	}
	return (unsigned char*)mcKeys.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CEvictedList::GetData(int iIndex, int* piDataSize)
{
	if (piDataSize)
	{
		*piDataSize = mcDatas.GetSize(iIndex);
	}
	return mcDatas.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEvictedList::Clear(void)
{
	mcKeys.ReInit();
	mcDatas.ReInit();
}
