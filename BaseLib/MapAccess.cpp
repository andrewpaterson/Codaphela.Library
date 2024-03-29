#include "MapHelper.h"
#include "MapAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapAccess::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void** CMapAccess::PutLongPtr(int64 lliKey, void* pvPointer)
{
	return (void**)Put(&lliKey, sizeof(int64), &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int* CMapAccess::PutLongInt(int64 lliKey, int iData)
{
	return (int*)Put(&lliKey, sizeof(int64), &iData, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64* CMapAccess::PutLongLong(int64 lliKey, int64 lliData)
{
	return (int64*)Put(&lliKey, sizeof(int64), &lliData, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::PutLongChar(int64 lliKey, char cData)
{
	return (char*)Put(&lliKey, sizeof(int64), &cData, sizeof(char));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::PutLongString(int64 lliKey, char* pszData)
{

	return (char*)Put(&lliKey, sizeof(int64), pszData, StrDataSize(pszData));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapAccess::PutLongData(int64 lliKey, void* pvData, size_t iDataSize)
{
	return Put(&lliKey, sizeof(int64), pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void** CMapAccess::PutIntPtr(int iKey, void* pvPointer)
{
	return (void**)Put(&iKey, sizeof(int), &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int* CMapAccess::PutIntInt(int iKey, int iData)
{
	return (int*)Put(&iKey, sizeof(int), &iData, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64* CMapAccess::PutIntLong(int iKey, int64 lliData)
{
	return (int64*)Put(&iKey, sizeof(int), &lliData, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::PutIntChar(int iKey, char cData)
{
	return (char*)Put(&iKey, sizeof(int), &cData, sizeof(char));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::PutIntString(int iKey, char* pszData)
{
	return (char*)Put(&iKey, sizeof(int), pszData, StrDataSize(pszData));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapAccess::PutIntData(int iKey, void* pvData, size_t iDataSize)
{
	return (void*)Put(&iKey, sizeof(int), pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void** CMapAccess::PutStringPtr(char* pszKey, void* pvPointer)
{
	return (void**)Put(pszKey, StrKeySize(pszKey), &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapAccess::PutStringData(char* pszKey, void* pvData, size_t iDataSize)
{
	return (void*)Put(pszKey, StrKeySize(pszKey), pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::PutStringChar(char* pszKey, char cData)
{
	return (char*)Put(pszKey, StrKeySize(pszKey), &cData, sizeof(char));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int* CMapAccess::PutStringInt(char* pszKey, int iData)
{
	return (int*)Put(pszKey, StrKeySize(pszKey), &iData, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64* CMapAccess::PutStringLong(char* pszKey, int64 lliData)
{
	return (int64*)Put(pszKey, StrKeySize(pszKey), &lliData, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::PutStringString(char* pszKey, char* pszData)
{
	return (char*)Put(pszKey, StrKeySize(pszKey), pszData, StrDataSize(pszData));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapAccess::PutKeyData(void* pvKey, int iKeySize, void* pvData, size_t iDataSize)
{
	return Put(pvKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int* CMapAccess::PutKeyInt(void* pvKey, int iKeySize, int iData)
{
	return (int*)Put(pvKey, iKeySize, &iData, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::PutKeyString(void* pvKey, int iKeySize, char* pszData)
{
	return (char*)Put(pvKey, iKeySize, pszData, StrDataSize(pszData));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::GetString(void* pvKey, int iKeySize)
{
	return (char*)Get(pvKey, iKeySize, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::GetLongString(int64 lliKey)
{
	return GetString(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMapAccess::GetLongInt(int64 lliKey, int iNullValue)
{
	int*	pi;
	
	pi = (int*)Get(&lliKey, sizeof(int64), NULL);
	if (pi)
	{
		return *pi;
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
int64 CMapAccess::GetLongLong(int64 lliKey, int64 lliNullValue)
{
	int64* plli;

	plli = (int64*)Get(&lliKey, sizeof(int64), NULL);
	if (plli)
	{
		return *plli;
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
char CMapAccess::GetLongChar(int64 lliKey, char cNullValue)
{
	char* pc;

	pc = (char*)Get(&lliKey, sizeof(int64), NULL);
	if (pc)
	{
		return *pc;
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
void* CMapAccess::GetLongData(int64 lliKey, size_t* piDataSize)
{
	return Get(&lliKey, sizeof(int64), piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapAccess::GetLongPtr(int64 lliKey)
{
	void*	pv;

	pv = Get(&lliKey, sizeof(int64), NULL);
	if (pv)
	{
		return pv;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::GetIntString(int iKey)
{
	return GetString(&iKey, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMapAccess::GetIntInt(int iKey, int iNullValue)
{
	int* pi;

	pi = (int*)Get(&iKey, sizeof(int), NULL);
	if (pi)
	{
		return *pi;
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
int64 CMapAccess::GetIntLong(int iKey, int64 lliNullValue)
{
	int64* plli;

	plli = (int64*)Get(&iKey, sizeof(int), NULL);
	if (plli)
	{
		return *plli;
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
char CMapAccess::GetIntChar(int iKey, char cNullValue)
{
	char* pc;

	pc = (char*)Get(&iKey, sizeof(int), NULL);
	if (pc)
	{
		return *pc;
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
void* CMapAccess::GetIntData(int iKey, size_t* piDataSize)
{
	return Get(&iKey, sizeof(int), piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapAccess::GetIntPtr(int iKey)
{
	void* pv;

	pv = Get(&iKey, sizeof(int), NULL);
	if (pv)
	{
		return pv;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapAccess::GetStringData(char* pszKey, size_t* piDataSize)
{
	return Get(pszKey, StrKeySize(pszKey), piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::GetStringString(char* pszKey)
{
	return GetString(pszKey, StrKeySize(pszKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CMapAccess::GetStringChar(char* pszKey, char cNullValue)
{
	char*	pc;

	pc = (char*)Get(pszKey, StrKeySize(pszKey), NULL);
	if (pc)
	{
		return *pc;
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
int CMapAccess::GetStringInt(char* pszKey, int iNullValue)
{
	int* pi;

	pi = (int*)Get(pszKey, StrKeySize(pszKey), NULL);
	if (pi)
	{
		return *pi;
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
int64 CMapAccess::GetStringLong(char* pszKey, int64 lliNullValue)
{
	int64* plli;

	plli = (int64*)Get(pszKey, StrKeySize(pszKey), NULL);
	if (plli)
	{
		return *plli;
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
void* CMapAccess::GetStringPtr(char* pszKey)
{
	void* pv;

	pv = Get(pszKey, StrKeySize(pszKey), NULL);
	if (pv)
	{
		return pv;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMapAccess::GetKeyString(void* pvKey, int iKeySize)
{
	return GetString(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapAccess::GetKeyData(void* pvKey, int iKeySize, size_t* piDataSize)
{
	return Get(pvKey, iKeySize, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMapAccess::GetKeyInt(void* pvKey, int iKeySize, int iNullValue)
{
	int*	pi;

	pi = (int*)Get(pvKey, iKeySize, NULL);
	if (pi)
	{
		return *pi;
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
int64 CMapAccess::GetKeyLong(void* pvKey, int iKeySize, int64 lliNullValue)
{
	int64* plli;

	plli = (int64*)Get(pvKey, iKeySize, NULL);
	if (plli)
	{
		return *plli;
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
bool CMapAccess::HasLong(int64 lliKey)
{
	return Has(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapAccess::HasInt(int iKey)
{
	return Has(&iKey, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapAccess::HasString(char* pszKey)
{
	return Has(pszKey, StrKeySize(pszKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapAccess::HasKey(void* pvKey, int iKeySize)
{
	return Has(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapAccess::DeleteLong(int64 lliKey)
{
	return Remove(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapAccess::DeleteInt(int iKey)
{
	return Remove(&iKey, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapAccess::DeleteString(char* pszKey)
{
	return Remove(pszKey, StrKeySize(pszKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapAccess::DeleteKey(void* pvKey, int iKeySize)
{
	return Remove(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMapAccess::DataSizeLong(int64 lliKey)
{
	return DataSize(&lliKey, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMapAccess::DataSizeString(char* pszKey)
{
	return DataSize(pszKey, StrKeySize(pszKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMapAccess::DataSizeKey(void* pvKey, int iKeySize)
{
	return DataSize(pvKey, iKeySize);
}


