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
void CCPPSyntaxDeclSpecifiers::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	macpSpecifiers.Init();
	muiFlags = 0;
	muiTypeKeywords = 0;
	miLongs = 0;
	mpcType = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::Kill(void)
{
	macpSpecifiers.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxDeclSpecifiers::GetType(void)
{
	return "DeclSpecifiers";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxDeclSpecifiers::GetKind(void)
{
	return CPPS_DeclSpecifiers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintList(pszDest, &macpSpecifiers, " ", iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxDeclSpecifiers::NumChildren(void)
{
	return macpSpecifiers.NumElements() + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxDeclSpecifiers::GetChild(size iIndex)
{
	if (iIndex < macpSpecifiers.NumElements())
	{
		return macpSpecifiers.GetPtr(iIndex);
	}
	iIndex -= macpSpecifiers.NumElements();

	if (iIndex == 0)
	{
		return mpcType;
	}
	iIndex--;

	return NULL;
}

