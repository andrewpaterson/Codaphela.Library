#include "IndexStringBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexStringBlock::Get(char* szKey, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Get(szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexStringBlock::Get(const char* szKey, const char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Get((void*)szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexStringBlock::Get(char* szKey, void** ppvData, int* piDataSize, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Get(szKey, iKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexStringBlock::Get(const char* szKey, void** ppvData, int* piDataSize, const char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Get((void*)szKey, iKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexStringBlock::Put(char* szKey, int iDataSize, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Put(szKey, iKeySize, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexStringBlock::Put(char* szKey, void* pvData, int iDataSize, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Put(szKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexStringBlock::Put(const char* szKey, int iDataSize, const char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Put((void*)szKey, iKeySize, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexStringBlock::Put(const char* szKey, void* pvData, int iDataSize, const char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Put((void*)szKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexStringBlock::HasKey(char* szKey, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::HasKey(szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexStringBlock::HasKey(const char* szKey, const char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::HasKey((void*)szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexStringBlock::Remove(char* szKey, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Remove(szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexStringBlock::Remove(const char* szKey, const char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Remove((void*)szKey, iKeySize);
}



