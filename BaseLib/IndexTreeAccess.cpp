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

	iKeySize = (int)strlen(pszKey);

	return Put(pszKey, iKeySize, &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutStringInt(char* pszKey, int iIndex)
{
	int	iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = (int)strlen(pszKey);

	return Put(pszKey, iKeySize, &iIndex, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutStringChar(char* pszKey, char cData)
{
	int	iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = (int)strlen(pszKey);

	return Put(pszKey, iKeySize, &cData, sizeof(char));
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

	iKeySize = (int)strlen(pszKey);

	return Put(pszKey, iKeySize, &lliIndex, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutStringData(char* pszKey, void* pvData, size_t iDataSize)
{
	int		iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = (int)strlen(pszKey);

	return Put(pszKey, iKeySize, pvData, iDataSize);
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
BOOL CIndexTreeAccess::PutLongInt(int64 lliKey, int iData)
{
	return Put(&lliKey, sizeof(int64), &iData, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutLongChar(int64 lliKey, char cData)
{
	return Put(&lliKey, sizeof(int64), &cData, sizeof(char));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutLongString(int64 lliKey, char* pszData)
{
	size_t iDataSize;

	iDataSize = strlen(pszData) + 1;
	return Put(&lliKey, sizeof(int64), pszData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutLongData(int64 lliKey, void* pvData, size_t iDataSize)
{
	return Put(&lliKey, sizeof(int64), pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutIntPtr(int iKey, void* pvPointer)
{
	return Put(&iKey, sizeof(int), &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutIntInt(int iKey, int iData)
{
	return Put(&iKey, sizeof(int), &iData, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutIntLong(int iKey, int64 lliData)
{
	return Put(&iKey, sizeof(int), &lliData, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutIntChar(int iKey, char cData)
{
	return Put(&iKey, sizeof(int), &cData, sizeof(char));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutIntString(int iKey, char* pszData)
{
	size_t iDataSize;

	iDataSize = strlen(pszData) + 1;
	return Put(&iKey, sizeof(int), pszData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutIntData(int iKey, void* pvData, size_t iDataSize)
{
	return Put(&iKey, sizeof(int), pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutStringString(char* pszKey, char* pszData)
{
	int		iKeySize;
	size_t	iDataSize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = (int)strlen(pszKey);
	iDataSize = strlen(pszData) + 1;

	return Put(pszKey, iKeySize, pszData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutKeyData(void* pvKey, int iKeySize, void* pvData, size_t iDataSize)
{
	return Put(pvKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutKeyInt(void* pvKey, int iKeySize, int iData)
{
	return Put(pvKey, iKeySize, &iData, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::PutKeyString(void* pvKey, int iKeySize, char* pszData)
{
	size_t uiDataSize;

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
char* CIndexTreeAccess::GetLongString(int64 lliKey, char* pszDest, size_t uiMaxDataSize)
{
	BOOL	bResult;

	bResult = Get(&lliKey, sizeof(int64), pszDest, NULL, uiMaxDataSize);
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
int CIndexTreeAccess::GetLongInt(int64 lliKey, int iNullValue)
{
	int				iKeySize;
	BOOL			bResult;
	int				i;
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
int64 CIndexTreeAccess::GetLongLong(int64 lliKey, int64 lliNullValue)
{
	int				iKeySize;
	BOOL			bResult;
	int				i;
	uint16	uiDataSize;

	iKeySize = sizeof(int64);
	uiDataSize = DataSize(&lliKey, iKeySize);
	if (uiDataSize != sizeof(int64))
	{
		return lliNullValue;
	}

	bResult = Get(&lliKey, iKeySize, &i, NULL, sizeof(int64));
	if (bResult)
	{
		return i;
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
char CIndexTreeAccess::GetLongChar(int64 lliKey, char cNullValue)
{
	int				iKeySize;
	BOOL			bResult;
	char			c;
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
BOOL CIndexTreeAccess::GetLongData(int64 lliKey, void* pvData, size_t* piDataSize, size_t uiMaxDataSize)
{
	return Get(&lliKey, sizeof(int64), pvData, piDataSize, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeAccess::GetLongPtr(int64 lliKey)
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
char* CIndexTreeAccess::GetIntString(int iKey, char* pszDest, size_t uiMaxDataSize)
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
int CIndexTreeAccess::GetIntInt(int iKey, int iNullValue)
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
int64 CIndexTreeAccess::GetIntLong(int iKey, int64 lliNullValue)
{
	int				iKeySize;
	BOOL			bResult;
	int				i;
	uint16	uiDataSize;

	iKeySize = sizeof(int);
	uiDataSize = DataSize(&iKey, iKeySize);
	if (uiDataSize != sizeof(int64))
	{
		return lliNullValue;
	}

	bResult = Get(&iKey, iKeySize, &i, NULL, sizeof(int64));
	if (bResult)
	{
		return i;
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
char CIndexTreeAccess::GetIntChar(int iKey, char cNullValue)
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
BOOL CIndexTreeAccess::GetIntData(int iKey, void* pvData, size_t* piDataSize, size_t uiMaxDataSize)
{
	return Get(&iKey, sizeof(int), pvData, piDataSize, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::GetStringData(char* pszKey, void* pvData, size_t* piDataSize, size_t uiMaxDataSize)
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
void* CIndexTreeAccess::GetStringPtr(char* pszKey)
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
BOOL CIndexTreeAccess::GetKeyData(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize)
{
	return Get(pvKey, iKeySize, pvData, piDataSize, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeAccess::GetKeyString(void* pvKey, int iKeySize, char* pszDest, size_t uiMaxDataSize)
{
	BOOL	bResult;

	bResult = Get(pvKey, iKeySize, pszDest, NULL, uiMaxDataSize);
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
char* CIndexTreeAccess::GetStringString(char* pszKey, char* pszDest, size_t uiMaxDataSize)
{
	size_t		iKeySize;
	BOOL	bResult;

	if (StrEmpty(pszKey))
	{
		pszDest[0] = 0;
		return NULL;
	}

	iKeySize = strlen(pszKey);

	bResult = Get(pszKey, iKeySize, pszDest, NULL, uiMaxDataSize);
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
CChars CIndexTreeAccess::GetStringString(char* pszKey, size_t uiMaxDataSize)
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
int CIndexTreeAccess::GetStringInt(char* pszKey, int iNullValue)
{
	size_t			iKeySize;
	BOOL			bResult;
	int				i;
	uint16	uiDataSize;

	if (StrEmpty(pszKey))
	{
		return iNullValue;
	}

	iKeySize = strlen(pszKey);
	uiDataSize = DataSize(pszKey, iKeySize);
	if (uiDataSize != sizeof(int))
	{
		return iNullValue;
	}

	bResult = Get(pszKey, iKeySize, &i, NULL, sizeof(int));
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
char CIndexTreeAccess::GetStringChar(char* pszKey, char iNullValue)
{
	size_t			iKeySize;
	BOOL			bResult;
	char			c;
	uint16	uiDataSize;

	if (StrEmpty(pszKey))
	{
		return iNullValue;
	}

	iKeySize = strlen(pszKey);
	uiDataSize = DataSize(pszKey, iKeySize);
	if (uiDataSize != sizeof(char))
	{
		return iNullValue;
	}

	bResult = Get(pszKey, iKeySize, &c, NULL, sizeof(char));
	if (bResult)
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
int64 CIndexTreeAccess::GetStringLong(char* pszKey, int64 iNullValue)
{
	size_t			iKeySize;
	BOOL			bResult;
	int64			lli;
	uint16	uiDataSize;

	if (StrEmpty(pszKey))
	{
		return iNullValue;
	}

	iKeySize = strlen(pszKey);
	uiDataSize = DataSize(pszKey, iKeySize);
	if (uiDataSize != sizeof(int64))
	{
		return iNullValue;
	}
	bResult = Get(pszKey, iKeySize, &lli, NULL, sizeof(int64));
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
BOOL CIndexTreeAccess::HasString(char* pszKey)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);

	return Has(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::HasLong(int64 lliKey)
{
	return Has(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::HasInt(int iKey)
{
	return Has(&iKey, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::HasKey(void* pvKey, int iKeySize)
{
	return Has(pvKey, iKeySize);
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
	size_t		iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Remove(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::DeleteKey(void* pvKey, int iKeySize)
{
	return Remove(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::FlushLong(int64 lliKey)
{
	return Flush(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::FlushString(char* pszKey)
{
	size_t		iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Flush(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::FlushKey(void* pvKey, int iKeySize)
{
	return Flush(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::EvictLong(int64 lliKey)
{
	return Evict(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::EvictInt(int iKey)
{
	return Evict(&iKey, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::EvictString(char* pszKey)
{
	size_t		iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Evict(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeAccess::EvictKey(void* pvKey, int iKeySize)
{
	return Evict(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeAccess::DataSizeLong(int64 lliKey)
{
	return DataSize(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeAccess::DataSizeString(char* pszKey)
{
	size_t		iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return DataSize(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeAccess::DataSizeKey(void* pvKey, int iKeySize)
{
	return DataSize(pvKey, iKeySize);
}

