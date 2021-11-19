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
	muiID = 0;
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
	int			iIndex;
	CDefine		cDefine;

	mcNameToIDIndex.Add(muiID, pcName->msz, pcName->EndInclusive());
	mcIDToDefineIndex.Put(muiID, &cDefine);
	cDefine.Init(pcName, muiID, this);
	muiID++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::AddDefine(CExternalString* pcName, CDefine* pcSource)
{
	int			iIndex;
	CDefine*	pcDefine;

	iIndex = mcNameToIDIndex.AddIndex(pcName->msz, pcName->EndInclusive());
	if (iIndex != -1)
	{
		//The define must have been setup correctly before hand.
		//Other processes will have depended on it.
		if ((iIndex == pcSource->miIndex) && (muiID == pcSource->muiID))
		{
			pcDefine = (CDefine*)mcIDToDefineIndex.GrowToAtLeastNumElements(iIndex+1, TRUE, 0);

			//It is safe to just overwrite mcReplacement and mcArguments provided that source isn't killed, just ignored.
			memcpy(pcDefine, pcSource, sizeof(CDefine));

			pcDefine->muiID = muiID;
			pcDefine->miIndex = iIndex;
			muiID++;
			pcDefine->SetDefineMap(this);
			return pcDefine;
		}
	}

	//If NULL is returned then source must still be killed.
	return NULL;
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
CDefine* CDefineMap::GetDefine(CExternalString* pcName)
{
	int64		lliID;
	CDefine*	pcDefine;

	lliID = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive());
	if (lliID != -1)
	{
		pcDefine = mcIDToDefineIndex.Get(lliID);
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
	return GetDefine(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::GetDefine(char* szName)
{
	CExternalString	cExternalString;
	int				iLen;

	iLen = (int)strlen(szName);
	cExternalString.Init(szName, iLen);
	return GetDefine(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::RemoveDefine(CExternalString* pcName)
{
	int64		lliID;
	CDefine*	pcDefine;

	lliID = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive());
	if (lliID != -1)
	{
		pcDefine = mcIDToDefineIndex.Get(lliID);
		if (pcDefine)
		{
			pcDefine->Kill();
			mcNameToIDIndex.Remove(pcName->msz, pcName->EndInclusive());
			mcIDToDefineIndex.Remove(lliID);
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
char* CDefineMap::GetName(CDefine* pcDefine)
{
	return mcNameToIDIndex.GetWord(pcDefine->miIndex);
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

