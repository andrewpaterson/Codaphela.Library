#include "IndexedEvictedList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedEvictedList::Init(void)
{
	mcKeys.Init();
	mcDatas.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedEvictedList::Kill(void)
{
	mcDatas.Kill();
	mcKeys.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedEvictedList::IndexEvicted(OIndex oi, void* pvData, int iDataSize)
{
	mcKeys.Add(oi);
	mcDatas.Add(pvData, iDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedEvictedList::NumElements(void)
{
	return mcKeys.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedEvictedList::GetKey(int iIndex)
{
	return mcKeys.GetValue(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexedEvictedList::GetData(int iIndex, int* piDataSize)
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
void CIndexedEvictedList::Clear(void)
{
	mcKeys.ReInit();
	mcDatas.ReInit();
}

