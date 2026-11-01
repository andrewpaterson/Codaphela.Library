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
void CCPPSyntaxFunctionSuffix::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	macpParameters.Init();
	mbVariadic = false;
	mbVariadicComma = false;
	macpQualifiers.Init();
	mpcExceptionSpecification = NULL;
	macpAttributes.Init();
	mpcTrailingReturn = NULL;
	macpVirtSpecifiers.Init();
	mpcScope = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxFunctionSuffix::Kill(void)
{
	macpParameters.Kill();
	macpQualifiers.Kill();
	macpAttributes.Kill();
	macpVirtSpecifiers.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxFunctionSuffix::GetKind(void)
{
	return CPPS_FunctionSuffix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxFunctionSuffix::PrettyPrint(CChars* pszDest, int iDepth)
{
	pszDest->Append('(');
	PrintList(pszDest, &macpParameters, ", ", iDepth);
	if (mbVariadic)
	{
		if (mbVariadicComma)
		{
			pszDest->Append(", ");
		}
		pszDest->Append("...");
	}
	pszDest->Append(')');
	if (macpQualifiers.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpQualifiers, " ", iDepth);
	}
	if (mpcExceptionSpecification)
	{
		pszDest->Append(' ');
		mpcExceptionSpecification->PrettyPrint(pszDest, iDepth);
	}
	if (macpAttributes.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpAttributes, " ", iDepth);
	}
	if (mpcTrailingReturn)
	{
		pszDest->Append(" -> ");
		mpcTrailingReturn->PrettyPrint(pszDest, iDepth);
	}
	if (macpVirtSpecifiers.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpVirtSpecifiers, " ", iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxFunctionSuffix::NumChildren(void)
{
	return macpParameters.NumElements() + macpQualifiers.NumElements() + 1 + macpAttributes.NumElements() + 1 + macpVirtSpecifiers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxFunctionSuffix::GetChild(size iIndex)
{
	CCPPSyntax*	pcChild;

	if (IsChild(&iIndex, &macpParameters, &pcChild) || IsChild(&iIndex, &macpQualifiers, &pcChild) || IsChild(&iIndex, mpcExceptionSpecification, &pcChild) || IsChild(&iIndex, &macpAttributes, &pcChild) || IsChild(&iIndex, mpcTrailingReturn, &pcChild) || IsChild(&iIndex, &macpVirtSpecifiers, &pcChild))
	{
		return pcChild;
	}
	return NULL;
}

