#include <strings.h>
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
void* CMapStringBlock::Get(char* szKey)
{
	void*	pvData;

	pvData = CMapBlock::Get(szKey);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMapStringBlock::Get(char* szKey, int* piDataSize)
{
	void*	pvData;
	BOOL	bFound;

	bFound = CMapBlock::Get(szKey, &pvData, piDataSize);
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
		return (CompareFunc)&strcasecmp;
	}
}

