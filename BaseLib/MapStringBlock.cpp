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
	DataCompare	fCaseFunc;

	fCaseFunc = CalculateCompareFunc(bCaseSensitive);
	CMapBlock::Init(pcMalloc, fCaseFunc, bOverwrite);
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
	size		iStrLen;

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
void* CMapStringBlock::Get(const char* szKey, size* piDataSize)
{
	void*	pvData;
	bool	bFound;
	size	iStrLen;

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
void* CMapStringBlock::Put(char* szKey, size iDataSize)
{
	void*	pvData;
	size		iStrLen;

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
bool CMapStringBlock::Put(char* szKey, void* psData, size iDataSize)
{
	size	iStrLen;

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
void* CMapStringBlock::Put(const char* szKey, size iDataSize)
{
	return Put((char*)szKey, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::Put(const char* szKey, void* psData, size iDataSize)
{
	return Put((char*)szKey, psData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::Remove(char* szKey)
{
	size	iStrLen;

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
	size	iStrLen;

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
	DataCompare		fCaseFunc;

	if (!pcFileReader->ReadBool(&bCaseSensitive))
	{
		return NULL;
	}

	fCaseFunc = CalculateCompareFunc(bCaseSensitive);
	return fCaseFunc;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringBlock::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	DataCompare		fCaseFunc;
	bool			bResult;

	fCaseFunc = ReadCaseSensitivity(pcFileReader);
	if (fCaseFunc == NULL)
	{
		return false;
	}
	bResult = CMapBlock::Read(pcFileReader, fCaseFunc);
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
	size				iStrLen;
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

