#ifndef __INDEX_STRING_TEMPLATE_H__
#define __INDEX_STRING_TEMPLATE_H__
#include "IndexStringBlock.h"


template<class D>
class CIndexStringTemplate : public CIndexStringBlock
{
public:
	D*		Get(char* szKey, char* szLastCharInclusive = NULL);
	D*		Get(const char* szKey, char* szLastCharInclusive = NULL);

	D*		Put(char* szKey, char* szLastCharInclusive = NULL);
	BOOL	Put(char* szKey, D* psData, char* szLastCharInclusive = NULL);
	D*		Put(const char* szKey, const char* szLastCharInclusive = NULL);
	BOOL	Put(const char* szKey, D* psData, const char* szLastCharInclusive = NULL);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CIndexStringTemplate<D>::Get(char* szKey, char* szLastCharInclusive)
{
	return (D*)CIndexStringBlock::Get(szKey, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CIndexStringTemplate<D>::Get(const char* szKey, char* szLastCharInclusive)
{
	return (D*)CIndexStringBlock::Get((void*)szKey, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CIndexStringTemplate<D>::Put(char* szKey, char* szLastCharInclusive)
{
	return (D*)CIndexStringBlock::Put(szKey, sizeof(D), szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringTemplate<D>::Put(char* szKey, D* psData, char* szLastCharInclusive)
{
	return CIndexStringBlock::Put(szKey, psData, sizeof(D), szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CIndexStringTemplate<D>::Put(const char* szKey, const char* szLastCharInclusive)
{
	return (D*)CIndexStringBlock::Put(szKey, iKeySize, sizeof(D), szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringTemplate<D>::Put(const char* szKey, D* psData, const char* szLastCharInclusive)
{
	return CIndexStringBlock::Put((void*)szKey, iKeySize, psData, sizeof(D), szLastCharInclusive);
}


#endif // !__INDEX_STRING_TEMPLATE_H__
