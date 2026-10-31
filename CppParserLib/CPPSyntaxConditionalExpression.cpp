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
void CCPPSyntaxConditionalExpression::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression::Init(pcTree, pcParent);
	mpcCondition = NULL;
	mpcTrue = NULL;
	mpcFalse = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxConditionalExpression::GetKind(void)
{
	return CPPS_ConditionalExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxConditionalExpression::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintNode(pszDest, mpcCondition, iDepth);
	pszDest->Append(" ? ");
	PrintNode(pszDest, mpcTrue, iDepth);
	pszDest->Append(" : ");
	PrintNode(pszDest, mpcFalse, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxConditionalExpression::NumChildren(void)
{
	return 1 + 1 + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxConditionalExpression::GetChild(size iIndex)
{
	CCPPSyntax*	pcChild;

	if (IsChild(&iIndex, mpcCondition, &pcChild) || IsChild(&iIndex, mpcTrue, &pcChild) || IsChild(&iIndex, mpcFalse, &pcChild))
	{
		return pcChild;
	}
	return NULL;
}

