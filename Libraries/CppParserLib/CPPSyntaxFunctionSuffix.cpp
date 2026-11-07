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
	macpParameters.Init(NodeArrayMallocator());
	mbVariadic = false;
	mbVariadicComma = false;
	macpQualifiers.Init(NodeArrayMallocator());
	mpcExceptionSpecification = NULL;
	macpAttributes.Init(NodeArrayMallocator());
	mpcTrailingReturn = NULL;
	macpVirtSpecifiers.Init(NodeArrayMallocator());
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
void CCPPSyntaxFunctionSuffix::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Children(&macpParameters);
	pcFields->Bool(&mbVariadic);
	pcFields->Bool(&mbVariadicComma);
	pcFields->Children(&macpQualifiers);
	pcFields->Child(&mpcExceptionSpecification);
	pcFields->Children(&macpAttributes);
	pcFields->Child(&mpcTrailingReturn);
	pcFields->Children(&macpVirtSpecifiers);
	pcFields->Scope(&mpcScope);
}

