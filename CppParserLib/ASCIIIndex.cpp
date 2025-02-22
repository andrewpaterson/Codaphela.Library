#include "ASCIIIndex.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CASCIIIndex::Init(void)
{
	mcNameToIDIndex.Init();
	mcIDToNameIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CASCIIIndex::Kill(void)
{
	mcNameToIDIndex.Kill();
	mcIDToNameIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int64 CASCIIIndex::Add(char* sz, char* szLastCharInclusive)
{
	int64	lliID;
	bool	bResult;

	lliID = mcNameToIDIndex.Add(sz, szLastCharInclusive);
	if (lliID == -1)
	{
		bResult = mcIDToNameIndex.Put(lliID, sz, szLastCharInclusive);
		if (!bResult)
		{
			mcNameToIDIndex.Remove(sz, szLastCharInclusive);
			lliID = -1;
		}
	}
	return lliID;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CASCIIIndex::NumElements(void)
{
	return mcNameToIDIndex.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CASCIIIndex::Get(int64 lliID, int* piLength)
{
	char*	psz;
	size	iLength;
	bool	bResult;

	bResult = mcIDToNameIndex.Get(lliID, &psz, &iLength);
	if (bResult)
	{
		SafeAssign(piLength, iLength);
		return psz;
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
int64 CASCIIIndex::Get(char* szText, char* szLastCharInclusive, bool bExact)
{
	SASCIINameIndex*	psNameIndex;

	psNameIndex = mcNameToIDIndex.Get(szText, szLastCharInclusive, bExact);
	if (psNameIndex)
	{
		return psNameIndex->mlliID;
	}
	else
	{
		return -1;
	}
}

