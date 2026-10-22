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
void CCPPSyntaxInitDeclarator::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	mpcDeclarator = NULL;
	mpcBitField = NULL;
	mpcInitializer = NULL;
	meInitializer = CPPI_None;
	mpcSymbol = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxInitDeclarator::Kill(void)
{
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxInitDeclarator::GetType(void)
{
	return "InitDeclarator";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxInitDeclarator::GetKind(void)
{
	return CPPS_InitDeclarator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxInitDeclarator::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintNode(pszDest, mpcDeclarator, iDepth);
	if (mpcBitField)
	{
		pszDest->Append(" : ");
		mpcBitField->PrettyPrint(pszDest, iDepth);
	}

	if (meInitializer == CPPI_Equals)
	{
		pszDest->Append(" = ");
		PrintNode(pszDest, mpcInitializer, iDepth);
	}
	else if (meInitializer == CPPI_Parentheses)
	{
		PrintNode(pszDest, mpcInitializer, iDepth);
	}
	else if (meInitializer == CPPI_Braces)
	{
		PrintNode(pszDest, mpcInitializer, iDepth);
	}
	else if (meInitializer == CPPI_Default)
	{
		pszDest->Append(" = default");
	}
	else if (meInitializer == CPPI_Delete)
	{
		pszDest->Append(" = delete");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxInitDeclarator::NumChildren(void)
{
	return 1 + 1 + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxInitDeclarator::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcDeclarator;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcBitField;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcInitializer;
	}
	iIndex--;

	return NULL;
}

