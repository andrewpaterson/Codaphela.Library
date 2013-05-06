/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "GeneralToken.h"
#include "DefineMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::Init(int iIndex, int uiID, CDefineMap* pcDefineMap)
{
	CPPLine	cLine;

	memcpy(&mcReplacement, &cLine, sizeof(CPPLine));

	mcReplacement.Init(-1, -1);
	mcArguments.Init(2);
	miIndex = iIndex;
	miFlags = ((pcDefineMap == NULL) ? 0: DEFINE_FLAGS_IN_MAP);
	muiID = uiID;
	mpcDefineMap = pcDefineMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::Kill(void)
{
	mcArguments.Kill();
	mcReplacement.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::AddArgument(CExternalString* pcName)
{
	mcArguments.Add(pcName->msz, 0, pcName->miLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::AddReplacmentToken(CPPToken* pcToken)
{
	mcReplacement.mcTokens.Add(&pcToken);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDefine::IsBacketed(void)
{
	return miFlags & DEFINE_FLAGS_BRACKETED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDefine::IsSpecial(void)
{
	return miFlags & DEFINE_FLAGS_SPECIAL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDefine::IsInMap(void)
{
	return miFlags & DEFINE_FLAGS_IN_MAP;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::SetSpecial(BOOL b)
{
	SetFlag(&miFlags, DEFINE_FLAGS_SPECIAL, b);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::SetBracketed(BOOL b)
{
	SetFlag(&miFlags, DEFINE_FLAGS_BRACKETED, b);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::SetDefineMap(CDefineMap* pcDefineMap)
{
	if (pcDefineMap)
	{
		SetFlag(&miFlags, DEFINE_FLAGS_IN_MAP, TRUE);
	}
	else
	{
		SetFlag(&miFlags, DEFINE_FLAGS_IN_MAP, FALSE);
	}
	mpcDefineMap = pcDefineMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDefine::Equals(CDefine* pcOther)
{
	if (pcOther)
	{
		if (IsBacketed() == pcOther->IsBacketed())
		{
			if (mcReplacement.Equals(&pcOther->mcReplacement))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CDefine::GetName(void)
{
	return mpcDefineMap->GetName(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::Dump(void)
{
	CChars	sz;
	int		i;
	CChars*	psz;

	sz.Init();

	if (IsInMap())
	{
		sz.Append(GetName());
	}
	else
	{
		sz.Append("*Not In Map*");
	}

	if (IsBacketed())
	{
		sz.Append('(');
		for (i = 0; i < mcArguments.NumElements(); i++)
		{
			psz = mcArguments.Get(i);
			sz.Append(psz->Text());

			if (i != mcArguments.NumElements()-1)
			{
				sz.Append(", ");
			}
		}
		sz.Append(')');
	}

	if (mcReplacement.TokenLength() > 0)
	{
		sz.Append(' ');
		mcReplacement.Append(&sz);
	}
	else
	{
		sz.AppendNewLine();
	}

	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Init(int iChunkSize)
{
	mcDefinesArray.Init(iChunkSize);
	mcDefinesTree.Init();
	muiID = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Kill(void)
{
	int			i;
	CDefine*	pcDefine;

	for (i = 0; i < mcDefinesArray.NumElements(); i++)
	{
		pcDefine = mcDefinesArray.Get(i);
		pcDefine->Kill();
	}

	mcDefinesArray.Kill();
	mcDefinesTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::AddDefine(CExternalString* pcName)
{
	int			iIndex;
	CDefine*	pcDefine;

	iIndex = mcDefinesTree.Add(pcName->msz, pcName->EndInclusive());
	if (iIndex != -1)
	{
		pcDefine = mcDefinesArray.GrowToAtLeastNumElements(iIndex+1, TRUE, 0);
		pcDefine->Init(iIndex, muiID, this);
		muiID++;
		return pcDefine;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::AddDefine(CExternalString* pcName, CDefine* pcSource)
{
	int			iIndex;
	CDefine*	pcDefine;

	iIndex = mcDefinesTree.Add(pcName->msz, pcName->EndInclusive());
	if (iIndex != -1)
	{
		//The define must have been setup correctly before hand.
		//Other processes will have depended on it.
		if ((iIndex == pcSource->miIndex) && (muiID == pcSource->muiID))
		{
			pcDefine = mcDefinesArray.GrowToAtLeastNumElements(iIndex+1, TRUE, 0);

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
	int			iIndex;
	CDefine*	pcDefine;

	iIndex = mcDefinesTree.Add(szName);
	if (iIndex != -1)
	{
		pcDefine = mcDefinesArray.GrowToAtLeastNumElements(iIndex+1, TRUE, 0);
		pcDefine->Init(iIndex, muiID, this);
		muiID++;
		return pcDefine;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::GetDefine(CExternalString* pcName)
{
	int			iIndex;
	CDefine*	pcDefine;

	iIndex = mcDefinesTree.GetIndex(pcName->msz, pcName->EndInclusive(), TRUE);
	if (iIndex == -1)
	{
		return NULL;
	}
	pcDefine = mcDefinesArray.Get(iIndex);
	return pcDefine;
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
	int			iIndex;
	CDefine*	pcDefine;

	iIndex = mcDefinesTree.GetIndex(pcName->msz, pcName->EndInclusive(), TRUE);
	if (iIndex != -1)
	{
		pcDefine = mcDefinesArray.Get(iIndex);
		pcDefine->Kill();
		mcDefinesTree.Remove(ATRS_OnlyEndOfWordMarker, pcName->msz, pcName->EndInclusive());
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::RemoveDefine(char* szName)
{
	int			iIndex;
	CDefine*	pcDefine;

	iIndex = mcDefinesTree.GetIndex(szName, NULL, TRUE);
	if (iIndex != -1)
	{
		pcDefine = mcDefinesArray.Get(iIndex);
		pcDefine->Kill();
		mcDefinesTree.Remove(ATRS_OnlyEndOfWordMarker, szName, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CDefineMap::GetName(CDefine* pcDefine)
{
	return mcDefinesTree.GetWord(pcDefine->miIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Dump(void)
{
	SASCIITreeIter	sIter;
	int				iWordEnd;
	CDefine*		pcDefine;

	iWordEnd = (int)mcDefinesTree.StartIteration(&sIter);
	while (iWordEnd != -1)
	{
		pcDefine = mcDefinesArray.Get(iWordEnd);
		pcDefine->Dump();

		iWordEnd = (int)mcDefinesTree.Iterate(&sIter);
	}
}

