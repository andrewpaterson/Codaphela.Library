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
void CCPPSyntaxPointerOperator::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	meKind = CPPP_Pointer;
	mpcClass = NULL;
	macpQualifiers.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxPointerOperator::Kill(void)
{
	macpQualifiers.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxPointerOperator::GetType(void)
{
	return "PointerOperator";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxPointerOperator::GetKind(void)
{
	return CPPS_PointerOperator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxPointerOperator::PrettyPrint(CChars* pszDest, int iDepth)
{
	if (meKind == CPPP_MemberPointer)
	{
		PrintNode(pszDest, mpcClass, iDepth);
		pszDest->Append("::*");
	}
	else if (meKind == CPPP_Pointer)
	{
		pszDest->Append('*');
	}
	else if (meKind == CPPP_Reference)
	{
		pszDest->Append('&');
	}
	else if (meKind == CPPP_RValueReference)
	{
		pszDest->Append("&&");
	}
	if (macpQualifiers.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpQualifiers, " ", iDepth);
		pszDest->Append(' ');
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxPointerOperator::NumChildren(void)
{
	return 1 + macpQualifiers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxPointerOperator::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcClass;
	}
	iIndex--;

	if (iIndex < macpQualifiers.NumElements())
	{
		return macpQualifiers.GetPtr(iIndex);
	}
	iIndex -= macpQualifiers.NumElements();

	return NULL;
}

