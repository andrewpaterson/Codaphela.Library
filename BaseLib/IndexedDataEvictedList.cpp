#include "IndexedDataEvictedList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataEvictedList::Init(void)
{
	mcKeys.Init();
	mcDatas.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataEvictedList::Kill(void)
{
	mcDatas.Kill();
	mcKeys.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataEvictedList::IndexEvicted(OIndex oi, void* pvData, int iDataSize)
{
	mcKeys.Add(oi);
	mcDatas.Add(pvData, iDataSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedDataEvictedList::NumElements(void)
{
	return mcKeys.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedDataEvictedList::GetKey(int iIndex)
{
	return mcKeys.GetValue(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexedDataEvictedList::GetData(int iIndex, int* piDataSize)
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
void CIndexedDataEvictedList::Clear(void)
{
	mcKeys.ReInit();
	mcDatas.ReInit();
}
