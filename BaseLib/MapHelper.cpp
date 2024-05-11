#include <string.h>
#include "StringHelper.h"
#include "MapHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size MinDataSize(size uiDataSize, size uiMaxDataSize)
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
bool CopyString(char* pvDest, char* pvSource, size iSourceLength, size iMaxDestLength)
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
void CopyData(void* pvDest, void* pvSource, size iSourceLength, size iMaxDestLength)
{
	if (pvDest)
	{
		memcpy(pvDest, pvSource, MinDataSize(iSourceLength, iMaxDestLength));
		if ((iSourceLength < iMaxDestLength) || (iMaxDestLength == 0))
		{
			((char*)pvDest)[iSourceLength] = '\0';
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size StrDataSize(char* pszData)
{
	size uiDataSize;

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
size StrKeySize(char* pszKey)
{
	size		iKeySize;

	if (StrEmpty(pszKey))
	{
		return 0;
	}

	iKeySize = strlen(pszKey);
	return iKeySize;
}