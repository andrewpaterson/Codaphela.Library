#include <string.h>
#include "StringHelper.h"
#include "DataTypes.h"
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
BOOL CIndexTreeAccess::Kill(void)
{
	mcTemp.Kill();
	return TRUE;
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
BOOL CIndexTreeAccess::PutStringInt(char* pszKey, int iIndex)
{
	int		iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);

	return Put(pszKey, iKeySize, &iIndex, sizeof(int));
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
BOOL CIndexTreeAccess::PutStringData(char* pszKey, void* pvObject, unsigned int uiDataSize)
{
	int		iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	if (uiDataSize > MAX_UCHAR)
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Put(pszKey, iKeySize, pvObject, uiDataSize);
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
	int uiDataSize;

	uiDataSize = strlen(pszData) + 1;
	if (uiDataSize > MAX_UCHAR)
	{
		return FALSE;
	}

	return Put(&lliKey, sizeof(int64), pszData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutLongData(int64 lliKey, void* pvObject, unsigned int uiDataSize)
{
	if (uiDataSize > MAX_UCHAR)
	{
		return FALSE;
	}

	return Put(&lliKey, sizeof(int64), pvObject, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutStringString(char* pszKey, char* pszData)
{
	int iKeySize;
	int uiDataSize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	uiDataSize = strlen(pszData) + 1;
	if (uiDataSize > MAX_UCHAR)
	{
		return FALSE;
	}

	return Put(pszKey, iKeySize, pszData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutKeyData(void* pvKey, int iKeySize, void* pvObject, unsigned int uiDataSize)
{
	if (uiDataSize > MAX_UCHAR)
	{
		return FALSE;
	}

	return Put(pvKey, iKeySize, pvObject, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutKeyString(void* pvKey, int iKeySize, char* pszData)
{
	int uiDataSize;

	uiDataSize = strlen(pszData) + 1;
	if (uiDataSize > MAX_UCHAR)
	{
		return FALSE;
	}

	return Put(pvKey, iKeySize, pszData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeAccess::GetLongString(int64 lliKey, char* pszDest)
{
	BOOL	bResult;

	bResult = Get(&lliKey, sizeof(int64), pszDest, NULL);
	if (!bResult)
	{
		pszDest[0] = 0;
		return NULL;
	}
	else
	{
		return pszDest;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::GetStringData(char* pszKey, void* pvObject, unsigned int* puiDataSize)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);

	return Get(pszKey, iKeySize, pvObject, puiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeAccess::GetStringPtr(char* pszKey)
{
	int		iKeySize;
	void**	pv;
	BOOL	bResult;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	bResult = Get(pszKey, iKeySize, &pv, NULL);
	if (bResult)
	{
		return pv;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::GetKeyData(void* pvKey, int iKeySize, void* pvObject, unsigned int* puiDataSize)
{
	return Get(pvKey, iKeySize, pvObject, puiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeAccess::GetKeyString(void* pvKey, int iKeySize, char* pszDest)
{
	BOOL	bResult;

	bResult = Get(pvKey, iKeySize, pszDest, NULL);
	if (!bResult)
	{
		pszDest[0] = 0;
		return NULL;
	}
	else
	{
		return pszDest;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeAccess::GetStringString(char* pszKey, char* pszDest)
{
	int		iKeySize;
	BOOL	bResult;

	if (StrEmpty(pszKey))
	{
		pszDest[0] = 0;
		return NULL;
	}

	iKeySize = strlen(pszKey);

	bResult = Get(pszKey, iKeySize, pszDest, NULL);
	if (!bResult)
	{
		pszDest[0] = 0;
		return NULL;
	}
	else
	{
		return pszDest;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeAccess::GetStringInt(char* pszKey)
{
	int				iKeySize;
	BOOL			bResult;
	int				i;
	unsigned short	uiDataSize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);

	uiDataSize = DataSize(pszKey, iKeySize);
	if (uiDataSize != sizeof(int))
	{
		return 0;
	}
	bResult = Get(pszKey, iKeySize, &i, NULL);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexTreeAccess::GetStringLong(char* pszKey)
{
	int				iKeySize;
	BOOL			bResult;
	int64			lli;
	unsigned short	uiDataSize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);

	uiDataSize = DataSize(pszKey, iKeySize);
	if (uiDataSize != sizeof(int64))
	{
		return 0;
	}
	bResult = Get(pszKey, iKeySize, &lli, NULL);
	return lli;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::HasString(char* pszKey)
{
	return GetStringData(pszKey, NULL, NULL);
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

