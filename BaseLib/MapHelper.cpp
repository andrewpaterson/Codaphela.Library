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
BOOL CopyString(char* pvDest, char* pvSource, size_t iSourceLength, size_t iMaxDestLength)
{
	if (pvDest)
	{
		memcpy(pvDest, pvSource, MinDataSize(iSourceLength, iMaxDestLength));
		if ((iSourceLength < iMaxDestLength) || (iMaxDestLength == 0))
		{
			pvDest[iSourceLength] = '\0';
			return TRUE;
		}
		else
		{
			pvDest[iMaxDestLength - 1] = '\0';
			return FALSE;
		}
	}
	return TRUE;
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

