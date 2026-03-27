#ifndef __ARRAY_TEMPLATE_POINTER_H__
#define __ARRAY_TEMPLATE_POINTER_H__
#include "PointerFunctions.h"
#include "ArrayTemplate.h"


template<class M>
class CArrayTemplatePtr : public CArrayTemplate<M*>
{
public:
	void	Init(void);
	void	Kill(void);
	
	void	Add(M* pv);
	
	bool	Get(size iIndex, M** pv);
	M**		Get(size iIndex);
	M*		GetPtr(size iIndex);
	M*		SafeGetPtr(size iIndex);

	void	SetPtr(size iIndex, M* pv);

	void	QuickSort(void);
	void	TimSort(void);
	void	BubbleSort(void);

	int32	Find(M* pv);
	void 	Push(M* pv);
	M*		Pop(void);

	bool	ContainsVoidPtr(void* pv);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePtr<M>::Init(void)
{
	CArrayTemplate<M*>::Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePtr<M>::Kill(void)
{
	CArrayTemplate<M*>::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePtr<M>::Add(M* pv)
{
	M** pvTemp;

	pvTemp = CArrayTemplate<M*>::Add();
	*pvTemp = pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplatePtr<M>::Get(size iIndex, M** pv)
{
	M** pvTemp;

	pvTemp = CArrayTemplate<M*>::SafeGet(iIndex);
	if (pvTemp)
	{
		*pv = *pvTemp;
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M** CArrayTemplatePtr<M>::Get(size iIndex)
{
	return CArrayTemplate<M*>::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplatePtr<M>::GetPtr(size iIndex)
{
	return *CArrayTemplate<M*>::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplatePtr<M>::SafeGetPtr(size iIndex)
{
	M** ppv;

	ppv = CArrayTemplate<M*>::SafeGet(iIndex);
	if (ppv)
	{
		return *ppv;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePtr<M>::SetPtr(size iIndex, M* pv)
{
	CArrayTemplate<M*>::Set(iIndex, &pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePtr<M>::QuickSort(void)
{
	CArrayTemplate<M*>::QuickSort(&ComparePtrPtr);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePtr<M>::TimSort(void)
{
	CArrayTemplate<M*>::TimSort(&ComparePtrPtr);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePtr<M>::BubbleSort(void)
{
	CArrayTemplate<M*>::BubbleSort(&ComparePtrPtr);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int32 CArrayTemplatePtr<M>::Find(M* pv)
{
	size	i;
	M**		pvData;

	pvData = GetData();
	for (i = 0; i < NumElements(); i++)
	{
		if (pvData[i] == pv)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePtr<M>::Push(M* pv)
{
	Add(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplatePtr<M>::Pop(void)
{
	M* pv;

	CArrayTemplate<M*>::Pop(&pv);
	return pv;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplatePtr<M>::ContainsVoidPtr(void* pv)
{
	size	uiNumElements;
	size	ui;
	M*		pvObject;

	uiNumElements = NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pvObject = GetPtr(ui);
		if (pvObject == pv)
		{
			return true;
		}
	}
	return false;
}

#endif // __ARRAY_TEMPLATE_POINTER_H__

