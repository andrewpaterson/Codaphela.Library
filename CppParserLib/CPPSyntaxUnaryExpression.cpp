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
void CCPPSyntaxUnaryExpression::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression::Init(pcTree, pcParent);
	meOperator = CT_Subtract;
	mbPostfix = false;
	mpcOperand = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxUnaryExpression::Kill(void)
{
	CCPPSyntaxExpression::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxUnaryExpression::GetType(void)
{
	return "UnaryExpression";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxUnaryExpression::GetKind(void)
{
	return CPPS_UnaryExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxUnaryExpression::PrettyPrint(CChars* pszDest, int iDepth)
{
	if (mbPostfix)
	{
		PrintNode(pszDest, mpcOperand, iDepth);
		PrintKeyword(pszDest, meOperator);
	}
	else
	{
		PrintKeyword(pszDest, meOperator);
		PrintNode(pszDest, mpcOperand, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxUnaryExpression::NumChildren(void)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxUnaryExpression::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcOperand;
	}
	iIndex--;

	return NULL;
}

