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
void CCPPSyntaxCallExpression::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression::Init(pcTree, pcParent);
	mpcCallee = NULL;
	macpArguments.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCallExpression::Kill(void)
{
	macpArguments.Kill();
	CCPPSyntaxExpression::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxCallExpression::GetType(void)
{
	return "CallExpression";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxCallExpression::GetKind(void)
{
	return CPPS_CallExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCallExpression::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintNode(pszDest, mpcCallee, iDepth);
	pszDest->Append('(');
	PrintList(pszDest, &macpArguments, ", ", iDepth);
	pszDest->Append(')');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxCallExpression::NumChildren(void)
{
	return 1 + macpArguments.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxCallExpression::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcCallee;
	}
	iIndex--;

	if (iIndex < macpArguments.NumElements())
	{
		return macpArguments.GetPtr(iIndex);
	}
	iIndex -= macpArguments.NumElements();

	return NULL;
}

