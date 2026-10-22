/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "CPPSyntaxes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumSpecifier::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeSpecifier::Init(pcTree, pcParent);
	mbScoped = false;
	meScopeKey = CT_class;
	mpcName = NULL;
	mpcBase = NULL;
	mbBody = false;
	mbTrailingComma = false;
	macpEnumerators.Init();
	mpcScope = NULL;
	mpcSymbol = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumSpecifier::Kill(void)
{
	macpEnumerators.Kill();
	CCPPSyntaxTypeSpecifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxEnumSpecifier::GetType(void)
{
	return "EnumSpecifier";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxEnumSpecifier::GetKind(void)
{
	return CPPS_EnumSpecifier;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumSpecifier::PrettyPrint(CChars* pszDest, int iDepth)
{
	pszDest->Append("enum");
	if (mbScoped)
	{
		pszDest->Append(' ');
		PrintKeyword(pszDest, meScopeKey);
	}
	if (mpcName)
	{
		pszDest->Append(' ');
		mpcName->PrettyPrint(pszDest, iDepth);
	}
	if (mpcBase)
	{
		pszDest->Append(" : ");
		mpcBase->PrettyPrint(pszDest, iDepth);
	}
	if (mbBody)
	{
		pszDest->AppendNewLine();
		Indent(pszDest, iDepth);
		pszDest->Append("{\n");
		PrintEnumerators(pszDest, iDepth + 1);
		Indent(pszDest, iDepth);
		pszDest->Append('}');
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxEnumSpecifier::NumChildren(void)
{
	return 1 + 1 + macpEnumerators.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxEnumSpecifier::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcName;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcBase;
	}
	iIndex--;

	if (iIndex < macpEnumerators.NumElements())
	{
		return macpEnumerators.GetPtr(iIndex);
	}
	iIndex -= macpEnumerators.NumElements();

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumSpecifier::PrintEnumerators(CChars* pszDest, int iDepth)
{
	size	i;
	size	uiNumElements;

	uiNumElements = macpEnumerators.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		Indent(pszDest, iDepth);
		macpEnumerators.GetPtr(i)->PrettyPrint(pszDest, iDepth);
		if ((i + 1 < uiNumElements) || mbTrailingComma)
		{
			pszDest->Append(',');
		}
		pszDest->AppendNewLine();
	}
}

