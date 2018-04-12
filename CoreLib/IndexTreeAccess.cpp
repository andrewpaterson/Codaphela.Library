#include <string.h>
#include "BaseLib/StringHelper.h"
#include "BaseLib/DataTypes.h"
#include "IndexTreeAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeAccess::Init(void)
{
	mcTemp.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeAccess::Kill(void)
{
	mcTemp.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutStringPtr(char* pszKey, void* pvPointer)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);

	return Put(pszKey, iKeySize, &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutStringLong(char* pszKey, int64 lliIndex)
{
	int		iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);

	return Put(pszKey, iKeySize, &lliIndex, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutStringData(char* pszKey, void* pvObject, int iDataSize)
{
	int		iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	if (iDataSize > MAX_UCHAR)
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Put(pszKey, iKeySize, pvObject, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutLongPtr(int64 lliKey, void* pvPointer)
{
	return Put(&lliKey, sizeof(int64), &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutLongString(int64 lliKey, char* pszData)
{
	int iDataSize;

	iDataSize = strlen(pszData) + 1;
	if (iDataSize > MAX_UCHAR)
	{
		return FALSE;
	}

	return Put(&lliKey, sizeof(int64), pszData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutLongData(int64 lliKey, void* pvObject, int iDataSize)
{
	if (iDataSize > MAX_UCHAR)
	{
		return FALSE;
	}

	return Put(&lliKey, sizeof(int64), pvObject, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutStringString(char* pszKey, char* pszData)
{
	int iKeySize;
	int iDataSize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	iDataSize = strlen(pszData) + 1;
	if (iDataSize > MAX_UCHAR)
	{
		return FALSE;
	}

	return Put(pszKey, iKeySize, pszData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::Put(void* pvKey, int iKeySize, unsigned char uiDataSize)
{
	mcTemp.Size(uiDataSize);
	return Put(pvKey, iKeySize, NULL, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::GetLongString(int64 lliKey, char* pszDest)
{
	BOOL	bResult;

	bResult = Get(&lliKey, sizeof(int64), pszDest, NULL);
	if (!bResult)
	{
		pszDest[0] = 0;
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::GetStringData(char* pszKey, void* pvObject, int* piDataSize)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);

	return Get(pszKey, iKeySize, pvObject, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::DeleteLong(int64 lliKey)
{
	return Remove(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::DeleteString(char* pszKey)
{
	int		iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Remove(pszKey, iKeySize);
}
