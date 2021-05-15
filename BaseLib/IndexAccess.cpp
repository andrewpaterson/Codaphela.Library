#include <string.h>
#include "StringHelper.h"
#include "DataTypes.h"
#include "IndexAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexAccess::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::Kill(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutStringPtr(char* pszKey, void* pvPointer)
{
	return Put(pszKey, StrKeySize(pszKey), &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutStringInt(char* pszKey, int iIndex)
{
	return Put(pszKey, StrKeySize(pszKey), &iIndex, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutStringChar(char* pszKey, char cData)
{
	return Put(pszKey, StrKeySize(pszKey), &cData, sizeof(char));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutStringLong(char* pszKey, int64 lliIndex)
{
	return Put(pszKey, StrKeySize(pszKey), &lliIndex, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutStringData(char* pszKey, void* pvData, size_t iDataSize)
{
	return Put(pszKey, StrKeySize(pszKey), pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutLongPtr(int64 lliKey, void* pvPointer)
{
	return Put(&lliKey, sizeof(int64), &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutLongInt(int64 lliKey, int iData)
{
	return Put(&lliKey, sizeof(int64), &iData, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutLongLong(int64 lliKey, int64 lliData)
{
	return Put(&lliKey, sizeof(int64), &lliData, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutLongChar(int64 lliKey, char cData)
{
	return Put(&lliKey, sizeof(int64), &cData, sizeof(char));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutLongString(int64 lliKey, char* pszData)
{
	return Put(&lliKey, sizeof(int64), pszData, StrDataSize(pszData));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutLongData(int64 lliKey, void* pvData, size_t iDataSize)
{
	return Put(&lliKey, sizeof(int64), pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutIntPtr(int iKey, void* pvPointer)
{
	return Put(&iKey, sizeof(int), &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutIntInt(int iKey, int iData)
{
	return Put(&iKey, sizeof(int), &iData, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutIntLong(int iKey, int64 lliData)
{
	return Put(&iKey, sizeof(int), &lliData, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutIntChar(int iKey, char cData)
{
	return Put(&iKey, sizeof(int), &cData, sizeof(char));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutIntString(int iKey, char* pszData)
{
	return Put(&iKey, sizeof(int), pszData, StrDataSize(pszData));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutIntData(int iKey, void* pvData, size_t iDataSize)
{
	return Put(&iKey, sizeof(int), pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutStringString(char* pszKey, char* pszData)
{
	return Put(pszKey, StrKeySize(pszKey), pszData, StrDataSize(pszData));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutKeyData(void* pvKey, int iKeySize, void* pvData, size_t iDataSize)
{
	return Put(pvKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutKeyInt(void* pvKey, int iKeySize, int iData)
{
	return Put(pvKey, iKeySize, &iData, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::PutKeyString(void* pvKey, int iKeySize, char* pszData)
{
	return Put(pvKey, iKeySize, pszData, StrDataSize(pszData));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexAccess::GetLongString(int64 lliKey, char* pszDest, size_t uiMaxDataSize)
{
	return GetString(&lliKey, sizeof(int64), pszDest, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexAccess::GetLongInt(int64 lliKey, int iNullValue)
{
	int		iKeySize;
	BOOL	bResult;
	int		i;
	uint16	uiDataSize;

	iKeySize = sizeof(int64);
	uiDataSize = DataSize(&lliKey, iKeySize);
	if (uiDataSize != sizeof(int))
	{
		return iNullValue;
	}

	bResult = Get(&lliKey, iKeySize, &i, NULL, sizeof(int));
	if (bResult)
	{
		return i;
	}
	else
	{
		return iNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexAccess::GetLongLong(int64 lliKey, int64 lliNullValue)
{
	int		iKeySize;
	BOOL	bResult;
	int64	lli;
	uint16	uiDataSize;

	iKeySize = sizeof(int64);
	uiDataSize = DataSize(&lliKey, iKeySize);
	if (uiDataSize != sizeof(int64))
	{
		return lliNullValue;
	}

	bResult = Get(&lliKey, iKeySize, &lli, NULL, sizeof(int64));
	if (bResult)
	{
		return lli;
	}
	else
	{
		return lliNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CIndexAccess::GetLongChar(int64 lliKey, char cNullValue)
{
	int		iKeySize;
	BOOL	bResult;
	char	c;
	uint16	uiDataSize;

	iKeySize = sizeof(int64);
	uiDataSize = DataSize(&lliKey, iKeySize);
	if (uiDataSize != sizeof(char))
	{
		return cNullValue;
	}

	bResult = Get(&lliKey, iKeySize, &c, NULL, sizeof(char));
	if (bResult)
	{
		return c;
	}
	else
	{
		return cNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::GetLongData(int64 lliKey, void* pvData, size_t* piDataSize, size_t uiMaxDataSize)
{
	return Get(&lliKey, sizeof(int64), pvData, piDataSize, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexAccess::GetLongPtr(int64 lliKey)
{
	void**	pv;
	BOOL	bResult;

	bResult = Get(&lliKey, sizeof(int64), &pv, NULL, sizeof(void*));
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
void* CIndexAccess::GetIntPtr(int iKey)
{
	void**	pv;
	BOOL	bResult;

	bResult = Get(&iKey, sizeof(int), &pv, NULL, sizeof(void*));
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
char* CIndexAccess::GetIntString(int iKey, char* pszDest, size_t uiMaxDataSize)
{
	BOOL	bResult;

	bResult = Get(&iKey, sizeof(int), pszDest, NULL, uiMaxDataSize);
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
int CIndexAccess::GetIntInt(int iKey, int iNullValue)
{
	int				iKeySize;
	BOOL			bResult;
	int				i;
	uint16	uiDataSize;

	iKeySize = sizeof(int);
	uiDataSize = DataSize(&iKey, iKeySize);
	if (uiDataSize != sizeof(int))
	{
		return iNullValue;
	}

	bResult = Get(&iKey, iKeySize, &i, NULL, sizeof(int));
	if (bResult)
	{
		return i;
	}
	else
	{
		return iNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexAccess::GetIntLong(int iKey, int64 lliNullValue)
{
	int				iKeySize;
	BOOL			bResult;
	int64			lli;
	uint16	uiDataSize;

	iKeySize = sizeof(int);
	uiDataSize = DataSize(&iKey, iKeySize);
	if (uiDataSize != sizeof(int64))
	{
		return lliNullValue;
	}

	bResult = Get(&iKey, iKeySize, &lli, NULL, sizeof(int64));
	if (bResult)
	{
		return lli;
	}
	else
	{
		return lliNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CIndexAccess::GetIntChar(int iKey, char cNullValue)
{
	int				iKeySize;
	BOOL			bResult;
	char			c;
	uint16	uiDataSize;

	iKeySize = sizeof(int);
	uiDataSize = DataSize(&iKey, iKeySize);
	if (uiDataSize != sizeof(char))
	{
		return cNullValue;
	}

	bResult = Get(&iKey, iKeySize, &c, NULL, sizeof(char));
	if (bResult)
	{
		return c;
	}
	else
	{
		return cNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::GetIntData(int iKey, void* pvData, size_t* piDataSize, size_t uiMaxDataSize)
{
	return Get(&iKey, sizeof(int), pvData, piDataSize, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::GetStringData(char* pszKey, void* pvData, size_t* piDataSize, size_t uiMaxDataSize)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = (int)strlen(pszKey);

	return Get(pszKey, iKeySize, pvData, piDataSize, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexAccess::GetStringPtr(char* pszKey)
{
	size_t		iKeySize;
	void**	pv;
	BOOL	bResult;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	bResult = Get(pszKey, iKeySize, &pv, NULL, sizeof(void*));
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
BOOL CIndexAccess::GetKeyData(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize)
{
	return Get(pvKey, iKeySize, pvData, piDataSize, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexAccess::GetKeyInt(void* pvKey, int iKeySize, int iNullValue)
{
	int		i;
	BOOL	bResult;

	bResult = Get(pvKey, iKeySize, &i, NULL, sizeof(int));
	if (bResult)
	{
		return i;
	}
	else
	{
		return iNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexAccess::GetString(void* pvKey, int iKeySize, char* pszDest, size_t uiMaxDataSize)
{
	BOOL	bResult;

	bResult = Get(pvKey, iKeySize, pszDest, NULL, uiMaxDataSize);
	if (!bResult)
	{
		if (pszDest)
		{
			pszDest[0] = 0;
		}
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
char* CIndexAccess::GetKeyString(void* pvKey, int iKeySize, char* pszDest, size_t uiMaxDataSize)
{
	return GetString(pvKey, iKeySize, pszDest, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexAccess::GetStringString(char* pszKey, char* pszDest, size_t uiMaxDataSize)
{
	return GetString(pszKey, StrKeySize(pszKey), pszDest, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars CIndexAccess::GetStringString(char* pszKey, size_t uiMaxDataSize)
{
	size_t			iKeySize;
	BOOL			bResult;
	CChars			sz;
	unsigned int	uiDataSize;

	if (StrEmpty(pszKey))
	{
		sz.Init();
		return sz;
	}

	iKeySize = strlen(pszKey);

	uiDataSize = DataSize(pszKey, iKeySize);
	if (uiDataSize != 0)
	{
		sz.InitLength(uiDataSize);
		bResult = Get(pszKey, iKeySize, sz.Text(), NULL, uiMaxDataSize);
		return sz;
	}
	else
	{
		sz.Init();
		return sz;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexAccess::GetStringInt(char* pszKey, int iNullValue)
{
	BOOL	bResult;
	int		i;
	size_t	uiDataSize;

	bResult = Get(pszKey, StrKeySize(pszKey), &i, &uiDataSize, sizeof(int));
	if (bResult && (uiDataSize == sizeof(int)))
	{
		return i;
	}
	else
	{
		return iNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CIndexAccess::GetStringChar(char* pszKey, char iNullValue)
{
	BOOL	bResult;
	char	c;
	size_t	uiDataSize;

	bResult = Get(pszKey, StrKeySize(pszKey), &c, &uiDataSize, sizeof(char));
	if (bResult && (uiDataSize == sizeof(char)))
	{
		return c;
	}
	else
	{
		return iNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexAccess::GetStringLong(char* pszKey, int64 iNullValue)
{
	BOOL			bResult;
	int64			lli;
	size_t	uiDataSize;

	bResult = Get(pszKey, StrKeySize(pszKey), &lli, &uiDataSize, sizeof(int64));
	if (bResult)
	{
		return lli;
	}
	else
	{
		return iNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::HasString(char* pszKey)
{
	return Has(pszKey, StrKeySize(pszKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::HasLong(int64 lliKey)
{
	return Has(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::HasInt(int iKey)
{
	return Has(&iKey, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::HasKey(void* pvKey, int iKeySize)
{
	return Has(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::DeleteLong(int64 lliKey)
{
	return Remove(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::DeleteString(char* pszKey)
{
	return Remove(pszKey, StrKeySize(pszKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::DeleteKey(void* pvKey, int iKeySize)
{
	return Remove(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::FlushLong(int64 lliKey)
{
	return Flush(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::FlushString(char* pszKey)
{
	return Flush(pszKey, StrKeySize(pszKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::FlushKey(void* pvKey, int iKeySize)
{
	return Flush(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::EvictLong(int64 lliKey)
{
	return Evict(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::EvictInt(int iKey)
{
	return Evict(&iKey, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::EvictString(char* pszKey)
{
	return Evict(pszKey, StrKeySize(pszKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::EvictKey(void* pvKey, int iKeySize)
{
	return Evict(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexAccess::DataSizeLong(int64 lliKey)
{
	return DataSize(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexAccess::DataSizeString(char* pszKey)
{
	return DataSize(pszKey, StrKeySize(pszKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexAccess::DataSizeKey(void* pvKey, int iKeySize)
{
	return DataSize(pvKey, iKeySize);
}

