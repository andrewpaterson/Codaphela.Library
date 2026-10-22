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
void CCPPSyntaxExceptionSpecification::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	mbNoexcept = false;
	mbParentheses = false;
	mpcCondition = NULL;
	macpTypes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxExceptionSpecification::Kill(void)
{
	macpTypes.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxExceptionSpecification::GetType(void)
{
	return "ExceptionSpecification";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxExceptionSpecification::GetKind(void)
{
	return CPPS_ExceptionSpecification;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxExceptionSpecification::PrettyPrint(CChars* pszDest, int iDepth)
{
	if (mbNoexcept)
	{
		pszDest->Append("noexcept");
		if (mbParentheses)
		{
			pszDest->Append('(');
			PrintNode(pszDest, mpcCondition, iDepth);
			pszDest->Append(')');
		}
	}
	else
	{
		pszDest->Append("throw(");
		PrintList(pszDest, &macpTypes, ", ", iDepth);
		pszDest->Append(')');
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxExceptionSpecification::NumChildren(void)
{
	return 1 + macpTypes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxExceptionSpecification::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcCondition;
	}
	iIndex--;

	if (iIndex < macpTypes.NumElements())
	{
		return macpTypes.GetPtr(iIndex);
	}
	iIndex -= macpTypes.NumElements();

	return NULL;
}

