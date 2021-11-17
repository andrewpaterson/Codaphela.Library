#include "IndexStringTemplate.h"


template<class D>
class CIndexStringPrimitive : public CIndexStringTemplate<D>
{
public:
	D		Get(char* szKey, D sNullValue = -1, char* szLastCharInclusive = NULL);
	D		Get(const char* szKey, D sNullValue = -1, char* szLastCharInclusive = NULL);

	BOOL	Put(char* szKey, D sData, char* szLastCharInclusive = NULL);
	BOOL	Put(const char* szKey, D sData, const char* szLastCharInclusive = NULL);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D CIndexStringPrimitive<D>::Get(char* szKey, D sNullValue, char* szLastCharInclusive)
{
	//D* psData;

	//psData = (CIndexStringTemplate<D>::Get(szKey, szLastCharInclusive);
	//if (psData)
	//{
	//	return *psData;
	//}
	//else
	//{
	//	return sNullValue;
	//}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D CIndexStringPrimitive<D>::Get(const char* szKey, D sNullValue, char* szLastCharInclusive)
{
	D* psData;

	psData = CIndexStringTemplate<D>::Get(szKey, szLastCharInclusive);
	if (psData)
	{
		return *psData;
	}
	else
	{
		return sNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringPrimitive<D>::Put(char* szKey, D sData, char* szLastCharInclusive)
{
	return CIndexStringTemplate<D>::Put(szKey, &sData, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringPrimitive<D>::Put(const char* szKey, D sData, const char* szLastCharInclusive)
{
	return CIndexStringTemplate<D>::Put(szKey, &sData, szLastCharInclusive);
}

