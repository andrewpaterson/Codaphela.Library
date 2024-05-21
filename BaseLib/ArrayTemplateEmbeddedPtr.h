#ifndef __ARRAY_TEMPLATE_EMBEDDED_POINTER_H__
#define __ARRAY_TEMPLATE_EMBEDDED_POINTER_H__
#include "PrimitiveTypes.h"
#include "PointerFunctions.h"
#include "ArrayTemplateEmbedded.h"


template<class M, uint I>
class CArrayTemplateEmbeddedPtr : public CArrayTemplateEmbedded<M*, I>
{
public:
	void	Init(void);
	void	Kill(void);
	void	Add(M* pv);
	M*		GetPtr(size iIndex);
	bool	Get(size iIndex, M** pv);
	M**		Get(size iIndex);
	void	QuickSort(void);
	size	Find(M* pv);
	void 	Push(M* pv);
	M*		Pop(void);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, uint I>
void CArrayTemplateEmbeddedPtr<M, I>::Init(void)
{
	CArrayTemplateEmbedded<M*, I>::Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, uint I>
void CArrayTemplateEmbeddedPtr<M, I>::Kill(void)
{
	CArrayTemplateEmbedded<M*, I>::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, uint I>
void CArrayTemplateEmbeddedPtr<M, I>::Add(M* pv)
{
	M** pvTemp;

	pvTemp = CArrayTemplateEmbedded<M*, I>::Add();
	*pvTemp = pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, uint I>
bool CArrayTemplateEmbeddedPtr<M, I>::Get(size iIndex, M** pv)
{
	M** pvTemp;

	pvTemp = CArrayTemplateEmbedded<M*, I>::SafeGet(iIndex);
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
template<class M, uint I>
M** CArrayTemplateEmbeddedPtr<M, I>::Get(size iIndex)
{
	return CArrayTemplateEmbedded<M*, I>::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, uint I>
M* CArrayTemplateEmbeddedPtr<M, I>::GetPtr(size iIndex)
{
	return *CArrayTemplateEmbedded<M*, I>::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, uint I>
void CArrayTemplateEmbeddedPtr<M, I>::QuickSort(void)
{
	CArrayTemplateEmbedded<M*, I>::QuickSort(&ComparePtrPtr);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, uint I>
size CArrayTemplateEmbeddedPtr<M, I>::Find(M* pv)
{
	size	i;
	M**		pvData;
	size	uiNumElements;

	pvData = GetData();
	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
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
template<class M, uint I>
void CArrayTemplateEmbeddedPtr<M, I>::Push(M* pv)
{
	Add(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, uint I>
M* CArrayTemplateEmbeddedPtr<M, I>::Pop(void)
{
	M* pv;

	CArrayTemplateEmbedded<M*, I>::Pop(&pv);
	return pv;
}


#endif // __ARRAY_TEMPLATE_EMBEDDED_POINTER_H__

