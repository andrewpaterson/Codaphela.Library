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
void CCPPSyntaxNamedCastExpression::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression::Init(pcTree, pcParent);
	meCast = CT_static_cast;
	mpcType = NULL;
	mpcOperand = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxNamedCastExpression::Kill(void)
{
	CCPPSyntaxExpression::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxNamedCastExpression::GetType(void)
{
	return "NamedCastExpression";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxNamedCastExpression::GetKind(void)
{
	return CPPS_NamedCastExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxNamedCastExpression::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintKeyword(pszDest, meCast);
	pszDest->Append('<');
	PrintNode(pszDest, mpcType, iDepth);
	pszDest->Append(">(");
	PrintNode(pszDest, mpcOperand, iDepth);
	pszDest->Append(')');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxNamedCastExpression::NumChildren(void)
{
	return 1 + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxNamedCastExpression::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcType;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcOperand;
	}
	iIndex--;

	return NULL;
}

