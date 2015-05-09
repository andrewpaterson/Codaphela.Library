#include "ArrayVoidPtr.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareVoidPtr(const void* arg1, const void* arg2)
{
	arg1 = *((void**)arg1);
	arg2 = *((void**)arg2);

	if (arg1 < arg2)
	{
		return -1;
	}
	else if (arg1 > arg2)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayVoidPtr::Init(void)
{
	__CArrayVoidPtr::Init(64);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayVoidPtr::Kill(void)
{
	__CArrayVoidPtr::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayVoidPtr::Add(void* pv)
{
	void**	pvTemp;

	pvTemp = __CArrayVoidPtr::Add();
	*pvTemp = pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayVoidPtr::Get(int iIndex, void** pv)
{
	void**	pvTemp;

	pvTemp = __CArrayVoidPtr::SafeGet(iIndex);
	if (pvTemp)
	{
		*pv = *pvTemp;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void** CArrayVoidPtr::Get(int iIndex)
{
	return __CArrayVoidPtr::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayVoidPtr::GetPtr(int iIndex)
{
	return *__CArrayVoidPtr::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayVoidPtr::QuickSort(void)
{
	__CArrayVoidPtr::QuickSort(&CompareVoidPtr);
}

