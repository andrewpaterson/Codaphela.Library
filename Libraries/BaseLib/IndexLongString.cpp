#include "IndexLongString.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexLongString::Get(int64 lli, char** pszData, size* piLength)
{
	bool	bResult;
	size	iLength;

	//The strings are stored with their terminator; the length doesn't count it.
	bResult = CIndexPrimitiveBlock<int64>::Get(lli, (void**)pszData, &iLength);
	if (bResult && piLength)
	{
		*piLength = (iLength > 0) ? iLength - 1 : 0;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CIndexLongString::Get(int64 lli)
{
	return (char*)CIndexPrimitiveBlock<int64>::Get(lli);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexLongString::Put(int64 lli, const char* szData, const char* szLastCharInclusive)
{
	size	iLength;
	char*	sz;
	bool	bResult;

	iLength = StrLen(szData, szLastCharInclusive);
	sz = (char*)malloc(iLength + 1);
	memcpy(sz, szData, iLength);
	sz[iLength] = '\0';
	bResult = CIndexPrimitiveBlock<int64>::Put(lli, sz, iLength + 1);
	free(sz);
	return bResult;
}

