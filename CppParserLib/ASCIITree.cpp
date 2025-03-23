/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "ASCIINameIndex.h"
#include "ASCIITree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::Init(void)
{
	mcIndex.Init();
	mlliID = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::Kill(void)
{
	mcIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CASCIITree::Add(char* szText, char* szLastCharInclusive)
{
	bool				bResult;
	size				iDataSize;
	SASCIINameIndex*	psData;
	int64				lliID;

	bResult = mcIndex.Get(szText, (void**)&psData, &iDataSize, szLastCharInclusive);
	if (!bResult)
	{
		iDataSize = SASCIINameIndex::Size(szText, szLastCharInclusive);
		psData = (SASCIINameIndex*)mcIndex.Put(szText, iDataSize, szLastCharInclusive);
		if (psData != NULL)
		{
			lliID = mlliID;
			mlliID++;
			psData->Init(lliID, szText, szLastCharInclusive);
			return lliID;
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Cannot add key [", StringToString(szText, szLastCharInclusive), "] with ID [", LongToString(mlliID), "].", NULL);
			return false;
		}
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Cannot add key [", StringToString(szText, szLastCharInclusive), "] with ID [", LongToString(mlliID), "].  Key already exists.", NULL);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SASCIINameIndex* CASCIITree::Get(char* szText, char* szLastCharInclusive, bool bExact)
{
	if (bExact)
	{
		return (SASCIINameIndex*)mcIndex.Get(szText, szLastCharInclusive);
	}
	else
	{
		return (SASCIINameIndex*)mcIndex.GetLongestPartial(szText, szLastCharInclusive);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CASCIITree::Remove(char* szText, char* szLastCharInclusive)
{
	return mcIndex.Remove(szText, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::NumElements(void)
{
	return mcIndex.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CASCIITree::StartIteration(SIndexTreeMemoryUnsafeIterator* psIter)
{
	bool				bResult;
	SASCIINameIndex*	psNameIndex;

	bResult = mcIndex.StartIteration(psIter, (void**)&psNameIndex, NULL, NULL, NULL, 0);
	if (bResult)
	{
		return psNameIndex->mlliID;
	}
	else
	{
		return -1LL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CASCIITree::Iterate(SIndexTreeMemoryUnsafeIterator* psIter)
{
	SASCIINameIndex*	psNameIndex;
	bool				bResult;

	bResult = mcIndex.Iterate(psIter, (void**)&psNameIndex, NULL, NULL, NULL, 0);
	if (bResult)
	{
		return psNameIndex->mlliID;
	}
	else
	{
		return -1LL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CASCIITree::IsEmpty(void)
{
	return mcIndex.NumElements() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CASCIITree::Contains(char* szText)
{
	return mcIndex.HasKey(szText);
}

