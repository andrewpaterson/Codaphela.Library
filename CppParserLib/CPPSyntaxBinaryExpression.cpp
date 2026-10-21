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
void CCPPSyntaxBinaryExpression::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression::Init(pcTree, pcParent);
	meOperator = CT_Add;
	mpcLeft = NULL;
	mpcRight = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBinaryExpression::Kill(void)
{
	CCPPSyntaxExpression::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxBinaryExpression::GetType(void)
{
	return "BinaryExpression";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxBinaryExpression::GetKind(void)
{
	return CPPS_BinaryExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBinaryExpression::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintNode(pszDest, mpcLeft, iDepth);
	if (meOperator == CT_Comma)
	{
		pszDest->Append(", ");
	}
	else if ((meOperator == CT_MemberFunctionPointerIndirection) || (meOperator == CT_MemberByPointerFunctionPointerIndirection))
	{
		PrintKeyword(pszDest, meOperator);
	}
	else
	{
		pszDest->Append(' ');
		PrintKeyword(pszDest, meOperator);
		pszDest->Append(' ');
	}
	PrintNode(pszDest, mpcRight, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxBinaryExpression::NumChildren(void)
{
	return 1 + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxBinaryExpression::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcLeft;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcRight;
	}
	iIndex--;

	return NULL;
}

