/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/ConstructorCall.h"
#include "ASCIINameIndex.h"
#include "GeneralToken.h"
#include "SpecialOperatorMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpecialOperatorMap::Init(void)
{
	mcIDToSpecialOperatorIndex.Init();
	mcNameToIDIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpecialOperatorMap::Kill(void)
{
	CSpecialOperator* pcSpecialOperator;
	SIndexTreeMemoryUnsafeIterator		sIter;
	int64								lli;
	BOOL								bHasNext;

	bHasNext = mcIDToSpecialOperatorIndex.StartIteration(&sIter, &pcSpecialOperator, &lli);
	while (bHasNext)
	{
		pcSpecialOperator->Kill();
		bHasNext = mcIDToSpecialOperatorIndex.Iterate(&sIter, &pcSpecialOperator, &lli);
	}

	mcIDToSpecialOperatorIndex.Kill();
	mcNameToIDIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSpecialOperator* CSpecialOperatorMap::AddSpecialOperator(char* szName, EPreprocessorSpecialOperator eType)
{

	CSpecialOperator	cSpecialOperator;
	CSpecialOperator*	pcSpecialOperator;
	int64				lliID;
	BOOL				bResult;
	SASCIINameIndex*	psNameIndex;

	psNameIndex = mcNameToIDIndex.Get(szName, NULL, TRUE);
	if (psNameIndex == NULL)
	{
		lliID = mcNameToIDIndex.Add(szName);
		if (lliID != -1)
		{
			bResult = mcIDToSpecialOperatorIndex.Put(lliID, &cSpecialOperator);
			if (bResult)
			{
				pcSpecialOperator = mcIDToSpecialOperatorIndex.Get(lliID);
				pcSpecialOperator->Init(szName, eType, lliID);
				return pcSpecialOperator;
			}
			else
			{
				mcNameToIDIndex.Remove(szName);
				return NULL;
			}
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
//
//
//////////////////////////////////////////////////////////////////////////
CSpecialOperator* CSpecialOperatorMap::GetSpecialOperator(CExternalString* pcName)
{
	SASCIINameIndex* psNameIndex;
	CSpecialOperator* pcSpecialOperator;

	psNameIndex = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive(), TRUE);
	if (psNameIndex)
	{
		pcSpecialOperator = mcIDToSpecialOperatorIndex.Get(psNameIndex->mlliID);
		return pcSpecialOperator;
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
CSpecialOperator* CSpecialOperatorMap::GetSpecialOperator(CChars* pszName)
{
	CExternalString	cExternalString;

	cExternalString.Init(pszName->Text(), pszName->Length());
	return GetSpecialOperator(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSpecialOperator* CSpecialOperatorMap::GetSpecialOperator(char* szName)
{
	CExternalString	cExternalString;
	int				iLen;

	iLen = (int)strlen(szName);
	cExternalString.Init(szName, iLen);
	return GetSpecialOperator(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpecialOperatorMap::RemoveSpecialOperator(CExternalString* pcName)
{
	SASCIINameIndex* psNameIndex;
	CSpecialOperator* pcSpecialOperator;

	psNameIndex = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive(), TRUE);
	if (psNameIndex)
	{
		pcSpecialOperator = mcIDToSpecialOperatorIndex.Get(psNameIndex->mlliID);
		if (pcSpecialOperator)
		{
			pcSpecialOperator->Kill();
			mcNameToIDIndex.Remove(pcName->msz, pcName->EndInclusive());
			mcIDToSpecialOperatorIndex.Remove(psNameIndex->mlliID);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpecialOperatorMap::RemoveSpecialOperator(char* szName)
{
	CExternalString	cExternalString;
	int				iLen;

	iLen = (int)strlen(szName);
	cExternalString.Init(szName, iLen);

	RemoveSpecialOperator(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpecialOperatorMap::Dump(void)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	int								iWordEnd;
	CSpecialOperator* pcSpecialOperator;

	iWordEnd = (int)mcNameToIDIndex.StartIteration(&sIter);
	while (iWordEnd != -1)
	{
		pcSpecialOperator = mcIDToSpecialOperatorIndex.Get(iWordEnd);
		pcSpecialOperator->Dump();

		iWordEnd = (int)mcNameToIDIndex.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSpecialOperatorsIndex* CSpecialOperatorMap::GetIDToSpecialOperatorIndex(void)
{
	return &mcIDToSpecialOperatorIndex;
}

