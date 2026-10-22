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
void CCPPSyntaxNewExpression::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression::Init(pcTree, pcParent);
	mbGlobal = false;
	macpPlacement.Init();
	mbPlacement = false;
	mbParenthesisedType = false;
	mpcType = NULL;
	macpArraySizes.Init();
	mpcInitializer = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxNewExpression::Kill(void)
{
	macpPlacement.Kill();
	macpArraySizes.Kill();
	CCPPSyntaxExpression::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxNewExpression::GetType(void)
{
	return "NewExpression";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxNewExpression::GetKind(void)
{
	return CPPS_NewExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxNewExpression::PrettyPrint(CChars* pszDest, int iDepth)
{
	size	i;

	if (mbGlobal)
	{
		pszDest->Append("::");
	}
	pszDest->Append("new ");
	if (mbPlacement)
	{
		pszDest->Append('(');
		PrintList(pszDest, &macpPlacement, ", ", iDepth);
		pszDest->Append(") ");
	}
	if (mbParenthesisedType)
	{
		pszDest->Append('(');
	}
	PrintNode(pszDest, mpcType, iDepth);
	if (mbParenthesisedType)
	{
		pszDest->Append(')');
	}
	for (i = 0; i < macpArraySizes.NumElements(); i++)
	{
		pszDest->Append('[');
		macpArraySizes.GetPtr(i)->PrettyPrint(pszDest, iDepth);
		pszDest->Append(']');
	}
	PrintNode(pszDest, mpcInitializer, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxNewExpression::NumChildren(void)
{
	return macpPlacement.NumElements() + 1 + macpArraySizes.NumElements() + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxNewExpression::GetChild(size iIndex)
{
	if (iIndex < macpPlacement.NumElements())
	{
		return macpPlacement.GetPtr(iIndex);
	}
	iIndex -= macpPlacement.NumElements();

	if (iIndex == 0)
	{
		return mpcType;
	}
	iIndex--;

	if (iIndex < macpArraySizes.NumElements())
	{
		return macpArraySizes.GetPtr(iIndex);
	}
	iIndex -= macpArraySizes.NumElements();

	if (iIndex == 0)
	{
		return mpcInitializer;
	}
	iIndex--;

	return NULL;
}

