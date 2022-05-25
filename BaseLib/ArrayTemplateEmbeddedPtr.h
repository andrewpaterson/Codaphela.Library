#ifndef __ARRAY_TEMPLATE_EMBEDDED_POINTER_H__
#define __ARRAY_TEMPLATE_EMBEDDED_POINTER_H__
#include "PointerFunctions.h"
#include "ArrayTemplateEmbedded.h"


template<class M, int I>
class CArrayTemplateEmbeddedPtr : public CArrayTemplateEmbedded<M*, I>
{
public:
	void	Init(void);
	void	Kill(void);
	void	Add(M* pv);
	M*		GetPtr(int iIndex);
	BOOL	Get(int iIndex, M** pv);
	M**		Get(int iIndex);
	void	QuickSort(void);
	int		Find(M* pv);
	void 	Push(M* pv);
	M*		Pop(void);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbeddedPtr<M, I>::Init(void)
{
	CArrayTemplateEmbedded<M*, I>::Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbeddedPtr<M, I>::Kill(void)
{
	CArrayTemplateEmbedded<M*, I>::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
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
template<class M, int I>
BOOL CArrayTemplateEmbeddedPtr<M, I>::Get(int iIndex, M** pv)
{
	M** pvTemp;

	pvTemp = CArrayTemplateEmbedded<M*, I>::SafeGet(iIndex);
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
template<class M, int I>
M** CArrayTemplateEmbeddedPtr<M, I>::Get(int iIndex)
{
	return CArrayTemplateEmbedded<M*, I>::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
M* CArrayTemplateEmbeddedPtr<M, I>::GetPtr(int iIndex)
{
	return *CArrayTemplateEmbedded<M*, I>::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbeddedPtr<M, I>::QuickSort(void)
{
	CArrayTemplateEmbedded<M*, I>::QuickSort(&ComparePtrPtr);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
int CArrayTemplateEmbeddedPtr<M, I>::Find(M* pv)
{
	int		i;
	M** pvData;

	pvData = GetData();
	for (i = 0; i < NumElements(); i++)
	{
		if (pvData[i] == pv)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbeddedPtr<M, I>::Push(M* pv)
{
	Add(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
M* CArrayTemplateEmbeddedPtr<M, I>::Pop(void)
{
	M* pv;

	CArrayTemplateEmbedded<M*, I>::Pop(&pv);
	return pv;
}


#endif // !__ARRAY_TEMPLATE_EMBEDDED_POINTER_H__

