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
#include "DefineMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Init(void)
{
	mcIDToDefineIndex.Init();
	mcNameToIDIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Kill(void)
{
	CDefine* pcDefine;
	SIndexTreeMemoryUnsafeIterator		sIter;
	int64								lli;
	BOOL								bHasNext;

	bHasNext = mcIDToDefineIndex.StartIteration(&sIter, &pcDefine, &lli);
	while (bHasNext)
	{
		pcDefine->Kill();
		bHasNext = mcIDToDefineIndex.Iterate(&sIter, &pcDefine, &lli);
	}

	mcIDToDefineIndex.Kill();
	mcNameToIDIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::AddDefine(CExternalString* pcName)
{
	CDefine				cDefine;
	CDefine* pcDefine;
	int64				lliID;
	BOOL				bResult;
	SASCIINameIndex* psNameIndex;

	psNameIndex = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive(), TRUE);
	if (psNameIndex == NULL)
	{
		lliID = mcNameToIDIndex.Add(pcName->msz, pcName->EndInclusive());
		if (lliID != -1)
		{
			bResult = mcIDToDefineIndex.Put(lliID, &cDefine);
			if (bResult)
			{
				pcDefine = mcIDToDefineIndex.Get(lliID);
				pcDefine->Init(pcName, lliID, this);
				return pcDefine;
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
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::AddDefine(char* szName)
{
	CExternalString		cExternalString;
	int					iLen;

	iLen = (int)strlen(szName);
	cExternalString.Init(szName, iLen);

	return AddDefine(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::GetDefine(CExternalString* pcName, BOOL bExact)
{
	SASCIINameIndex*	psNameIndex;
	CDefine*			pcDefine;

	psNameIndex = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive(), bExact);
	if (psNameIndex)
	{
		pcDefine = mcIDToDefineIndex.Get(psNameIndex->mlliID);
		return pcDefine;
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
CDefine* CDefineMap::GetDefine(CChars* pszName)
{
	CExternalString	cExternalString;

	cExternalString.Init(pszName->Text(), pszName->Length());
	return GetDefine(&cExternalString, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::GetDefine(char* szName, BOOL bExact)
{
	CExternalString	cExternalString;
	int				iLen;

	iLen = (int)strlen(szName);
	cExternalString.Init(szName, iLen);
	return GetDefine(&cExternalString, bExact);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::RemoveDefine(CExternalString* pcName)
{
	SASCIINameIndex*	psNameIndex;
	CDefine*			pcDefine;

	psNameIndex = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive(), TRUE);
	if (psNameIndex)
	{
		pcDefine = mcIDToDefineIndex.Get(psNameIndex->mlliID);
		if (pcDefine)
		{
			pcDefine->Kill();
			mcNameToIDIndex.Remove(pcName->msz, pcName->EndInclusive());
			mcIDToDefineIndex.Remove(psNameIndex->mlliID);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::RemoveDefine(char* szName)
{
	CExternalString	cExternalString;
	int				iLen;

	iLen = (int)strlen(szName);
	cExternalString.Init(szName, iLen);

	RemoveDefine(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Dump(void)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	int								iWordEnd;
	CDefine*						pcDefine;

	iWordEnd = (int)mcNameToIDIndex.StartIteration(&sIter);
	while (iWordEnd != -1)
	{
		pcDefine = mcIDToDefineIndex.Get(iWordEnd);
		pcDefine->Dump();

		iWordEnd = (int)mcNameToIDIndex.Iterate(&sIter);
	}
}

