#ifdef LINUX_GNU_32
#include <strings.h>
#endif // LINUX_GNU_32
#include "MapStringBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::Init(int iChunkSize, BOOL bCaseSensitive, BOOL bOverwrite)
{
	Init(&gcSystemAllocator, iChunkSize, bCaseSensitive, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::Init(CMallocator* pcMallocator, int iChunkSize, BOOL bCaseSensitive, BOOL bOverwrite)
{
	CompareFunc	CaseFunc;

	CaseFunc = CalculateCompareFunc(bCaseSensitive);
	CMapBlock::Init(pcMallocator, iChunkSize, CaseFunc, bOverwrite);
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

	pvData = CMapBlock::Get((void*)szKey);
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

	bFound = CMapBlock::Get((void*)szKey, &pvData, piDataSize);
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

	if (szKey)
	{
		iStrLen = strlen(szKey);
		pvData = CMapBlock::Put(szKey, iStrLen + 1, iDataSize);
		return (void*)pvData;
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
BOOL CMapStringBlock::Put(char* szKey, void* psData, int iDataSize)
{
	int	iStrLen;

	if (szKey)
	{
		iStrLen = strlen(szKey);
		return CMapBlock::Put(szKey, iStrLen + 1, psData, iDataSize);
	}
	else
	{
		return FALSE;
	}
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
	return CMapBlock::Remove(szKey);
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
CompareFunc CMapStringBlock::ReadCaseSensitivity(CFileReader* pcFileReader)
{
	BOOL			bCaseSensitive;
	CompareFunc		CaseFunc;

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
	CompareFunc		CaseFunc;
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

	bResult = StartIteration(&sIter, &pvKey, &pvData);
	while (bResult)
	{
		if (pvData == psData)
		{
			return (char*)pvKey;
		}
		bResult = Iterate(&sIter, &pvKey, &pvData);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringBlock::IsCaseSensitive(void)
{
	return Func == (CompareFunc)&strcmp;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::SetCaseSensitive(BOOL bCaseSensitive)
{
	Func = CalculateCompareFunc(bCaseSensitive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CompareFunc CMapStringBlock::CalculateCompareFunc(BOOL bCaseSensitive)
{
	if (bCaseSensitive)
	{
		return (CompareFunc)&strcmp;
	}
	else
	{
#ifdef _MSC_VER
		return (CompareFunc)&_stricmp;
#endif // _MSC_VER		

#ifdef LINUX_GNU_32
		return (CompareFunc)&strcasecmp;
#endif // LINUX_GNU_32
	}
}

