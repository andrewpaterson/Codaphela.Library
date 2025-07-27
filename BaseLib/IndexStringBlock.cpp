#include "IndexStringBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexStringBlock::Get(char* szKey, char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Get((uint8*)szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexStringBlock::Get(const char* szKey, const char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Get((uint8*)szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::Get(char* szKey, void** ppvData, size* piDataSize, char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Get((uint8*)szKey, iKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::Get(const char* szKey, void** ppvData, size* piDataSize, const char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Get((uint8*)szKey, iKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexStringBlock::GetLongestPartial(char* szKey, char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::GetLongestPartial((uint8*)szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::GetLongestPartial(char* szKey, void** ppvData, size* piDataSize, char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::GetLongestPartial((uint8*)szKey, iKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexStringBlock::Put(char* szKey, size iDataSize, char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Put((uint8*)szKey, iKeySize, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::Put(char* szKey, void* pvData, size iDataSize, char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Put((uint8*)szKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexStringBlock::Put(const char* szKey, size iDataSize, const char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Put((uint8*)szKey, iKeySize, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::Put(const char* szKey, void* pvData, size iDataSize, const char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Put((uint8*)szKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::HasKey(char* szKey, char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::HasKey((uint8*)szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::HasKey(const char* szKey, const char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::HasKey((uint8*)szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::Remove(char* szKey, char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Remove((uint8*)szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexStringBlock::Remove(const char* szKey, const char* szLastCharInclusive)
{
	size iKeySize = StrLen(szKey, szLastCharInclusive);
	return CIndexBlock::Remove((uint8*)szKey, iKeySize);
}



