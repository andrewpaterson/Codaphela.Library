/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "BaseLib/Log.h"
#include "ASCIINameIndex.h"
#include "ASCIITree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::Init(void)
{
	mcIndex.Init();
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
BOOL CASCIITree::Add(int64 lliID, char* szText, char* szLastCharInclusive)
{
	int64				lli;
	BOOL				bResult;
	int					iDataSize;
	SASCIINameIndex*	psData;

	bResult = mcIndex.Get(szText, (void**)&psData, &iDataSize, szLastCharInclusive);
	if (!bResult)
	{
		iDataSize = SASCIINameIndex::Size(szText, szLastCharInclusive);
		psData = (SASCIINameIndex*)mcIndex.Put(szText, iDataSize, szLastCharInclusive);
		if (psData != NULL)
		{
			psData->Init(xxx);
			return bResult;
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Cannot add key [", StringToString(szText, szLastCharInclusive), "] with ID [", LongLongToString(lliID), "].", NULL);
			return FALSE;
		}
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Cannot add key [", StringToString(szText, szLastCharInclusive), "] with ID [", LongLongToString(lliID), "].  Key already exists.", NULL);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CASCIITree::Get(char* szText, char* szLastCharInclusive /*, BOOL bExact */)
{
	return mcIndex.Get(szText, -1LL, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::Remove(char* szText, char* szLastCharInclusive)
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
	int64	lli;
	BOOL	bResult;

	bResult = mcIndex.StartIteration(psIter, &lli, NULL, NULL, 0);
	if (bResult)
	{
		return lli;
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
	int64	lli;
	BOOL	bResult;

	bResult = mcIndex.Iterate(psIter, &lli, NULL, NULL, 0);
	if (bResult)
	{
		return lli;
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
BOOL CASCIITree::IsEmpty(void)
{
	return mcIndex.NumElements() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::Contains(char* szText)
{
	return mcIndex.HasKey(szText);
}

