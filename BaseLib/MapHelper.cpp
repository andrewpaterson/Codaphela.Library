#include <string.h>
#include "StringHelper.h"
#include "MapHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t MinDataSize(size_t uiDataSize, size_t uiMaxDataSize)
{
	if (uiMaxDataSize == 0)
	{
		return uiDataSize;
	}
	else if (uiMaxDataSize > uiDataSize)
	{
		return uiDataSize;
	}
	else
	{
		return uiMaxDataSize;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CopyString(char* pvDest, char* pvSource, size_t iSourceLength, size_t iMaxDestLength)
{
	if (pvDest)
	{
		memcpy(pvDest, pvSource, MinDataSize(iSourceLength, iMaxDestLength));
		if ((iSourceLength < iMaxDestLength) || (iMaxDestLength == 0))
		{
			pvDest[iSourceLength] = '\0';
			return true;
		}
		else
		{
			pvDest[iMaxDestLength - 1] = '\0';
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CopyData(char* pvDest, char* pvSource, size_t iSourceLength, size_t iMaxDestLength)
{
	if (pvDest)
	{
		memcpy(pvDest, pvSource, MinDataSize(iSourceLength, iMaxDestLength));
		if ((iSourceLength < iMaxDestLength) || (iMaxDestLength == 0))
		{
			pvDest[iSourceLength] = '\0';
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t StrDataSize(char* pszData)
{
	size_t uiDataSize;

	if (StrEmpty(pszData))
	{
		return 0;
	}
	uiDataSize = strlen(pszData) + 1;
	return uiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t StrKeySize(char* pszKey)
{
	size_t		iKeySize;

	if (StrEmpty(pszKey))
	{
		return 0;
	}

	iKeySize = (int)strlen(pszKey);
	return iKeySize;
}