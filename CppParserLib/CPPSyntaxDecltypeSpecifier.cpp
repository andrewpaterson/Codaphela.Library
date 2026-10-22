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
void CCPPSyntaxDecltypeSpecifier::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeSpecifier::Init(pcTree, pcParent);
	meKeyword = CT_decltype;
	mpcOperand = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDecltypeSpecifier::Kill(void)
{
	CCPPSyntaxTypeSpecifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxDecltypeSpecifier::GetType(void)
{
	return "DecltypeSpecifier";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxDecltypeSpecifier::GetKind(void)
{
	return CPPS_DecltypeSpecifier;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDecltypeSpecifier::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintKeyword(pszDest, meKeyword);
	pszDest->Append('(');
	PrintNode(pszDest, mpcOperand, iDepth);
	pszDest->Append(')');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxDecltypeSpecifier::NumChildren(void)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxDecltypeSpecifier::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcOperand;
	}
	iIndex--;

	return NULL;
}

