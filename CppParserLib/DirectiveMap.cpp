/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "DirectiveMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectiveMap::Init(void)
{
	mcIDToDirectiveIndex.Init();
	mcNameToIDIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectiveMap::Kill(void)
{
	CPreprocessorDirective*				pcDirective;
	SIndexTreeMemoryUnsafeIterator		sIter;
	int64								lli;
	bool								bHasNext;

	bHasNext = mcIDToDirectiveIndex.StartIteration(&sIter, &pcDirective, &lli);
	while (bHasNext)
	{
		pcDirective->Kill();
		bHasNext = mcIDToDirectiveIndex.Iterate(&sIter, &pcDirective, &lli);
	}

	mcIDToDirectiveIndex.Kill();
	mcNameToIDIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPreprocessorDirective* CDirectiveMap::AddDirective(CExternalString* pcName, EPreprocessorDirective eDirective)
{
	CPreprocessorDirective		cDirective;
	CPreprocessorDirective*		pcDirective;
	int64						lliID;
	bool						bResult;

	lliID = mcNameToIDIndex.Add(pcName->msz, pcName->EndInclusive());
	if (lliID != -1)
	{
		bResult = mcIDToDirectiveIndex.Put(lliID, &cDirective);
		if (bResult)
		{
			pcDirective = mcIDToDirectiveIndex.Get(lliID);
			pcDirective->Init(pcName, eDirective, lliID, this);
			return pcDirective;
		}
		else
		{
			mcNameToIDIndex.Remove(pcName->msz, pcName->EndInclusive());
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
CPreprocessorDirective* CDirectiveMap::AddDirective(char* szName, EPreprocessorDirective eDirective)
{
	CExternalString		cExternalString;
	int					iLen;

	iLen = (int)strlen(szName);
	cExternalString.Init(szName, iLen);

	return AddDirective(&cExternalString, eDirective);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPreprocessorDirective* CDirectiveMap::GetDirective(CExternalString* pcName, bool bExact)
{
	SASCIINameIndex* psNameIndex;
	CPreprocessorDirective* pcDirective;

	psNameIndex = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive(), bExact);
	if (psNameIndex)
	{
		pcDirective = mcIDToDirectiveIndex.Get(psNameIndex->mlliID);
		return pcDirective;
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
CPreprocessorDirective* CDirectiveMap::GetDirective(CChars* pszName, bool bExact)
{
	CExternalString	cExternalString;

	cExternalString.Init(pszName->Text(), pszName->Length());
	return GetDirective(&cExternalString, bExact);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPreprocessorDirective* CDirectiveMap::GetDirective(char* szName, bool bExact)
{
	CExternalString	cExternalString;
	int				iLen;

	iLen = (int)strlen(szName);
	cExternalString.Init(szName, iLen);
	return GetDirective(&cExternalString, bExact);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectiveMap::RemoveDirective(CExternalString* pcName)
{
	SASCIINameIndex* psNameIndex;
	CPreprocessorDirective* pcDirective;

	psNameIndex = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive(), true);
	if (psNameIndex)
	{
		pcDirective = mcIDToDirectiveIndex.Get(psNameIndex->mlliID);
		if (pcDirective)
		{
			pcDirective->Kill();
			mcNameToIDIndex.Remove(pcName->msz, pcName->EndInclusive());
			mcIDToDirectiveIndex.Remove(psNameIndex->mlliID);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectiveMap::RemoveDirective(char* szName)
{
	CExternalString	cExternalString;
	int				iLen;

	iLen = (int)strlen(szName);
	cExternalString.Init(szName, iLen);

	RemoveDirective(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectiveMap::Dump(void)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	int								iWordEnd;
	CPreprocessorDirective*			pcDirective;

	iWordEnd = (int)mcNameToIDIndex.StartIteration(&sIter);
	while (iWordEnd != -1)
	{
		pcDirective = mcIDToDirectiveIndex.Get(iWordEnd);
		pcDirective->Dump();

		iWordEnd = (int)mcNameToIDIndex.Iterate(&sIter);
	}
}

