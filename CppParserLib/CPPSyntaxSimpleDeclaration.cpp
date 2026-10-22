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
void CCPPSyntaxSimpleDeclaration::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxDeclaration::Init(pcTree, pcParent);
	mpcSpecifiers = NULL;
	macpDeclarators.Init();
	mbNoSemicolon = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxSimpleDeclaration::Kill(void)
{
	macpDeclarators.Kill();
	CCPPSyntaxDeclaration::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxSimpleDeclaration::GetType(void)
{
	return "SimpleDeclaration";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxSimpleDeclaration::GetKind(void)
{
	return CPPS_SimpleDeclaration;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxSimpleDeclaration::PrettyPrint(CChars* pszDest, int iDepth)
{
	Indent(pszDest, iDepth);
	PrintNode(pszDest, mpcSpecifiers, iDepth);
	if (mpcSpecifiers && (mpcSpecifiers->macpSpecifiers.NumElements() > 0) && (macpDeclarators.NumElements() > 0))
	{
		pszDest->Append(' ');
	}
	PrintList(pszDest, &macpDeclarators, ", ", iDepth);
	if (!mbNoSemicolon)
	{
		pszDest->Append(";\n");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxSimpleDeclaration::NumChildren(void)
{
	return 1 + macpDeclarators.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxSimpleDeclaration::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcSpecifiers;
	}
	iIndex--;

	if (iIndex < macpDeclarators.NumElements())
	{
		return macpDeclarators.GetPtr(iIndex);
	}
	iIndex -= macpDeclarators.NumElements();

	return NULL;
}

