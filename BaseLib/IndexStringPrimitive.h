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

	bool	HasKey(char* szKey, D sNullValue = -1, char* szLastCharInclusive = NULL);
	bool	HasKey(const char* szKey, D sNullValue = -1, char* szLastCharInclusive = NULL);

	bool	Put(char* szKey, D sData, char* szLastCharInclusive = NULL);
	bool	Put(const char* szKey, D sData, const char* szLastCharInclusive = NULL);

	bool	StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, D* pvData, void* pvDestKey, size* puiKeySize, size uiMaxKeySize);
	bool	Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, D* pvData, void* pvDestKey, size* puiKeySize, size uiMaxKeySize);
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
bool CIndexStringPrimitive<D>::HasKey(char* szKey, D sNullValue, char* szLastCharInclusive)
{

	return CIndexStringTemplate<D>::HasKey(szKey, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexStringPrimitive<D>::HasKey(const char* szKey, D sNullValue, char* szLastCharInclusive)
{
	return CIndexStringTemplate<D>::Get(szKey, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexStringPrimitive<D>::Put(char* szKey, D sData, char* szLastCharInclusive)
{
	return CIndexStringTemplate<D>::Put(szKey, &sData, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexStringPrimitive<D>::Put(const char* szKey, D sData, const char* szLastCharInclusive)
{
	return CIndexStringTemplate<D>::Put(szKey, &sData, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexStringPrimitive<D>::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, D* pvData, void* pvDestKey, size* puiKeySize, size uiMaxKeySize)
{
	D*		pvResult;
	bool	bResult;

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
bool CIndexStringPrimitive<D>::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, D* pvData, void* pvDestKey, size* puiKeySize, size uiMaxKeySize)
{
	D*		pvResult;
	bool	bResult;

	bResult = CIndexBlock::Iterate(psIterator, (void**)&pvResult, NULL, pvDestKey, puiKeySize, uiMaxKeySize);
	if (bResult)
	{
		*pvData = *pvResult;
	}
	return bResult;
}



#endif // __INDEX_STRING_PRIMITIVE_H__

