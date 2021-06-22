#include "ListCharsMinimal.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CListCharsMinimal::TotalSize(int iNumElements, int iRowLength)
{
	return __CListCharsMinimal::TotalSize(iNumElements, iRowLength + iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListCharsMinimal::Init(int iNumElements, int iRowLength)
{
	__CListCharsMinimal::Init(iNumElements, iRowLength);
	miTotalSize = TotalSize(iNumElements, iRowLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CListCharsMinimal::IndexOf(char* szString)
{
	size_t	i;

	for (i = 0; i < miUsedElements; i++)
	{
		if (StringCompare(szString, Get(i)) == 0)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListCharsMinimal::Add(char* sz)
{
	Add(sz, strlen(sz));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListCharsMinimal::Add(char* sz, size_t iSize)
{
	char*	szDest;

	szDest = __CListCharsMinimal::Add(sz, iSize + 1);
	szDest[iSize] = 0;
}

