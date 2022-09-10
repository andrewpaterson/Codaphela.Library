#ifdef LINUX_GNU_32
#include <strings.h>
#endif // LINUX_GNU_32
#include "MapStringBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::Init(bool bCaseSensitive, bool bOverwrite)
{
	Init(&gcSystemAllocator, bCaseSensitive, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::Init(CMallocator* pcMalloc, bool bCaseSensitive, bool bOverwrite)
{
	DataCompare	CaseFunc;

	CaseFunc = CalculateCompareFunc(bCaseSensitive);
	CMapBlock::Init(pcMalloc, CaseFunc, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::Kill(void)
{
	CMapBlock::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMapStringBlock::Get(const char* szKey)
{
	void*	pvData;
	int		iStrLen;

	if (!szKey)
	{
		return NULL;
	}

	iStrLen = strlen(szKey) + 1;
	pvData = CMapBlock::Get((void*)szKey, iStrLen);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMapStringBlock::Get(const char* szKey, int* piDataSize)
{
	void*	pvData;
	bool	bFound;
	int		iStrLen;

	if (!szKey)
	{
		return NULL;
	}

	iStrLen = strlen(szKey) + 1;
	bFound = CMapBlock::Get((void*)szKey, iStrLen, &pvData, piDataSize);
	if (bFound)
	{
		return pvData;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMapStringBlock::Put(char* szKey, int iDataSize)
{
	void*	pvData;
	int		iStrLen;

	if (StrEmpty(szKey))
	{
		return NULL;
	}

	iStrLen = strlen(szKey) + 1;
	pvData = CMapBlock::Put(szKey, iStrLen, iDataSize);
	return (void*)pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::Put(char* szKey, void* psData, int iDataSize)
{
	int	iStrLen;

	if (StrEmpty(szKey))
	{
		return false;
	}

	iStrLen = strlen(szKey) + 1;
	return CMapBlock::Put(szKey, iStrLen, psData, iDataSize);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMapStringBlock::Put(const char* szKey, int iDataSize)
{
	return Put((char*)szKey, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::Put(const char* szKey, void* psData, int iDataSize)
{
	return Put((char*)szKey, psData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::Remove(char* szKey)
{
	int	iStrLen;

	if (StrEmpty(szKey))
	{
		return false;
	}

	iStrLen = strlen(szKey) + 1;
	return CMapBlock::Remove(szKey, iStrLen);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::Remove(const char* szKey)
{
	return Remove((char*)szKey);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::HasKey(char* szKey)
{
	int	iStrLen;

	if (StrEmpty(szKey))
	{
		return false;
	}

	iStrLen = strlen(szKey) + 1;
	return CMapBlock::HasKey(szKey, iStrLen);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::HasKey(const char* szKey)
{
	return HasKey((char*)szKey);
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::WriteCaseSensitivity(CFileWriter* pcFileWriter)
{
	bool	bCaseSensitive;

	bCaseSensitive = IsCaseSensitive();
	return pcFileWriter->WriteBool(bCaseSensitive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::Write(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = WriteCaseSensitivity(pcFileWriter);
	bResult &= CMapBlock::Write(pcFileWriter);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
DataCompare CMapStringBlock::ReadCaseSensitivity(CFileReader* pcFileReader)
{
	bool			bCaseSensitive;
	DataCompare		CaseFunc;

	if (!pcFileReader->ReadBool(&bCaseSensitive))
	{
		return NULL;
	}

	CaseFunc = CalculateCompareFunc(bCaseSensitive);
	return CaseFunc;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	DataCompare		CaseFunc;
	bool			bResult;

	CaseFunc = ReadCaseSensitivity(pcFileReader);
	bResult = CMapBlock::Read(pcFileReader, CaseFunc);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CMapStringBlock::GetKeyForData(void* psData)
{
	SMapIterator	sIter;
	void*			pvData;
	void*			pvKey;
	bool			bResult;

	if (psData == NULL)
	{
		return NULL;
	}

	bResult = StartIteration(&sIter, &pvKey, NULL, &pvData, NULL);
	while (bResult)
	{
		if (pvData == psData)
		{
			return (char*)pvKey;
		}
		bResult = Iterate(&sIter, &pvKey, NULL, &pvData, NULL);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::IsCaseSensitive(void)
{
	return fKeyCompare == (DataCompare)&StringCompare;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::SetCaseSensitive(bool bCaseSensitive)
{
	fKeyCompare = CalculateCompareFunc(bCaseSensitive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
DataCompare CMapStringBlock::CalculateCompareFunc(bool bCaseSensitive)
{
	if (bCaseSensitive)
	{
		return (DataCompare)&StringCompare;
	}
	else
	{
		return (DataCompare)&StringInsensitiveCompare;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::GetKeysAsString(CChars* pszDest, char* szSeparator)
{
	SMapIterator	sIter;
	char*			szKey;
	int				iStrLen;
	bool			bHasNext;
	bool			bFirst;

	bFirst = true;
	bHasNext = StartIteration(&sIter, (void**)&szKey, &iStrLen, NULL, NULL);
	while (bHasNext)
	{
		if (bFirst)
		{
			bFirst = false;
		}
		else
		{
			pszDest->Append(szSeparator);
		}

		pszDest->Append(szKey, iStrLen - 1);

		bHasNext = Iterate(&sIter, (void**)&szKey, &iStrLen, NULL, NULL);
	}
}

