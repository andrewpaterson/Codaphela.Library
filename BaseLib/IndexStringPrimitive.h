#ifndef __INDEX_STRING_PRIMITIVE_H__
#define __INDEX_STRING_PRIMITIVE_H__
#include "IndexBlock.h"
#include "IndexStringTemplate.h"


template<class D>
class CIndexStringPrimitive : public CIndexStringTemplate<D>
{
public:
	D		Get(char* szKey, D sNullValue = -1, char* szLastCharInclusive = NULL);
	D		Get(const char* szKey, D sNullValue = -1, char* szLastCharInclusive = NULL);

	BOOL	HasKey(char* szKey, D sNullValue = -1, char* szLastCharInclusive = NULL);
	BOOL	HasKey(const char* szKey, D sNullValue = -1, char* szLastCharInclusive = NULL);

	BOOL	Put(char* szKey, D sData, char* szLastCharInclusive = NULL);
	BOOL	Put(const char* szKey, D sData, const char* szLastCharInclusive = NULL);

	BOOL	StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, D* pvData, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize);
	BOOL	Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, D* pvData, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D CIndexStringPrimitive<D>::Get(char* szKey, D sNullValue, char* szLastCharInclusive)
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
BOOL CIndexStringPrimitive<D>::HasKey(char* szKey, D sNullValue, char* szLastCharInclusive)
{

	return CIndexStringTemplate<D>::HasKey(szKey, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringPrimitive<D>::HasKey(const char* szKey, D sNullValue, char* szLastCharInclusive)
{
	return CIndexStringTemplate<D>::Get(szKey, szLastCharInclusive);
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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringPrimitive<D>::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, D* pvData, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize)
{
	D*		pvResult;
	BOOL	bResult;

	bResult = CIndexBlock::StartIteration(psIterator, (void**)&pvResult, NULL, pvDestKey, puiKeySize, uiMaxKeySize);
	if (bResult)
	{
		*pvData = *pvResult;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringPrimitive<D>::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, D* pvData, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize)
{
	D*		pvResult;
	BOOL	bResult;

	bResult = CIndexBlock::Iterate(psIterator, (void**)&pvResult, NULL, pvDestKey, puiKeySize, uiMaxKeySize);
	if (bResult)
	{
		*pvData = *pvResult;
	}
	return bResult;
}



#endif //!__INDEX_STRING_PRIMITIVE_H__

