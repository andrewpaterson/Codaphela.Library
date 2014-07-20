#include "MapStringBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::Init(int iChunkSize, BOOL bCaseSensitive)
{
	Init(&gcSystemAllocator, iChunkSize, bCaseSensitive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringBlock::Init(CMallocator* pcMallocator, int iChunkSize, BOOL bCaseSensitive)
{
	CompareFunc	CaseFunc;

	CaseFunc = CalculateCompareFunc(bCaseSensitive);
	CMapBlock::Init(pcMallocator, iChunkSize, CaseFunc);
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
	BOOL	bResult;

	if (szKey)
	{
		bResult = CMapBlock::Get(szKey, &pvData, NULL);
		if (bResult)
		{
			return ((void*)pvData);
		}
		else
		{
			return NULL;
		}
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
BOOL CMapStringBlock::Write(CFileWriter* pcFileWriter)
{
	BOOL	bResult;
	BOOL	bCaseSensitive;

	bCaseSensitive = IsCaseSensitive();
	if (!pcFileWriter->WriteBool(bCaseSensitive))
	{
		return FALSE;
	}

	bResult = CMapBlock::Write(pcFileWriter);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringBlock::Read(CFileReader* pcFileReader)
{
	BOOL			bCaseSensitive;
	CompareFunc		CaseFunc;
	BOOL			bResult;

	if (!pcFileReader->ReadBool(&bCaseSensitive))
	{
		return FALSE;
	}

	CaseFunc = CalculateCompareFunc(bCaseSensitive);
	bResult = CMapBlock::Read(pcFileReader, CaseFunc);
	return bResult;
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
		return (CompareFunc)&_stricmp;
	}
}

