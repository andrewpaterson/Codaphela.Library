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
bool CIndexStringBlock::Get(char* szKey, void** ppvData, int* piDataSize, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Get(szKey, iKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::Get(const char* szKey, void** ppvData, int* piDataSize, const char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Get((void*)szKey, iKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexStringBlock::GetLongestPartial(char* szKey, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::GetLongestPartial(szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::GetLongestPartial(char* szKey, void** ppvData, int* piDataSize, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::GetLongestPartial(szKey, iKeySize, ppvData, piDataSize);
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
bool CIndexStringBlock::Put(char* szKey, void* pvData, int iDataSize, char* szLastCharInclusive)
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
bool CIndexStringBlock::Put(const char* szKey, void* pvData, int iDataSize, const char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Put((void*)szKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::HasKey(char* szKey, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::HasKey(szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::HasKey(const char* szKey, const char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::HasKey((void*)szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::Remove(char* szKey, char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Remove(szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::Remove(const char* szKey, const char* szLastCharInclusive)
{
	int iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Remove((void*)szKey, iKeySize);
}



