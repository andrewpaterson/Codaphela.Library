#ifdef LINUX_GNU_32
#include <strings.h>
#endif // LINUX_GNU_32
#include "MapStringBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::Init(BOOL bCaseSensitive, BOOL bOverwrite)
{
	Init(&gcSystemAllocator, bCaseSensitive, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::Init(CMallocator* pcMalloc, BOOL bCaseSensitive, BOOL bOverwrite)
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
	BOOL	bFound;
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
BOOL CMapStringBlock::Put(char* szKey, void* psData, int iDataSize)
{
	int	iStrLen;

	if (StrEmpty(szKey))
	{
		return FALSE;
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
BOOL CMapStringBlock::Put(const char* szKey, void* psData, int iDataSize)
{
	return Put((char*)szKey, psData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringBlock::Remove(char* szKey)
{
	int	iStrLen;

	if (StrEmpty(szKey))
	{
		return FALSE;
	}

	iStrLen = strlen(szKey) + 1;
	return CMapBlock::Remove(szKey, iStrLen);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringBlock::WriteCaseSensitivity(CFileWriter* pcFileWriter)
{
	BOOL	bCaseSensitive;

	bCaseSensitive = IsCaseSensitive();
	return pcFileWriter->WriteBool(bCaseSensitive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringBlock::Write(CFileWriter* pcFileWriter)
{
	BOOL	bResult;

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
	BOOL			bCaseSensitive;
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
BOOL CMapStringBlock::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	DataCompare		CaseFunc;
	BOOL			bResult;

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
	BOOL			bResult;

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
BOOL CMapStringBlock::IsCaseSensitive(void)
{
	return fKeyCompare == (DataCompare)&strcmp;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::SetCaseSensitive(BOOL bCaseSensitive)
{
	fKeyCompare = CalculateCompareFunc(bCaseSensitive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
DataCompare CMapStringBlock::CalculateCompareFunc(BOOL bCaseSensitive)
{
	if (bCaseSensitive)
	{
		return (DataCompare)&strcmp;
	}
	else
	{
#ifdef _MSC_VER
		return (DataCompare)&_stricmp;
#endif // _MSC_VER		

#ifdef LINUX_GNU_32
		return (DataCompare)&strcasecmp;
#endif // LINUX_GNU_32
	}
}

