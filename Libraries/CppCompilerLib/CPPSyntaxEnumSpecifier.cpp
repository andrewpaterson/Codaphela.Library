/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "CPPSyntaxes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumSpecifier::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeSpecifier::Init(pcTree, pcParent);
	mbScoped = false;
	meScopeKey = CT_class;
	macpAttributes.Init(NodeArrayMallocator());
	mpcName = NULL;
	mpcBase = NULL;
	mbBody = false;
	mbTrailingComma = false;
	macpEnumerators.Init(NodeArrayMallocator());
	mpcScope = NULL;
	mpcSymbol = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumSpecifier::Kill(void)
{
	macpAttributes.Kill();
	macpEnumerators.Kill();
	CCPPSyntaxTypeSpecifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumSpecifier::PrettyPrint(CChars* pszDest, int iDepth)
{
	pszDest->Append("enum");
	if (mbScoped)
	{
		pszDest->Append(' ');
		PrintKeyword(pszDest, meScopeKey);
	}
	if (macpAttributes.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpAttributes, " ", iDepth);
	}
	if (mpcName)
	{
		pszDest->Append(' ');
		mpcName->PrettyPrint(pszDest, iDepth);
	}
	if (mpcBase)
	{
		pszDest->Append(" : ");
		mpcBase->PrettyPrint(pszDest, iDepth);
	}
	if (mbBody)
	{
		pszDest->AppendNewLine();
		Indent(pszDest, iDepth);
		pszDest->Append("{\n");
		PrintEnumerators(pszDest, iDepth + 1);
		Indent(pszDest, iDepth);
		pszDest->Append('}');
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumSpecifier::PrintEnumerators(CChars* pszDest, int iDepth)
{
	size	i;
	size	uiNumElements;

	uiNumElements = macpEnumerators.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		Indent(pszDest, iDepth);
		macpEnumerators.GetPtr(i)->PrettyPrint(pszDest, iDepth);
		if ((i + 1 < uiNumElements) || mbTrailingComma)
		{
			pszDest->Append(',');
		}
		pszDest->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumSpecifier::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Bool(&mbScoped);
	pcFields->Enum(&meScopeKey);
	pcFields->Children(&macpAttributes);
	pcFields->Child(&mpcName);
	pcFields->Child(&mpcBase);
	pcFields->Bool(&mbBody);
	pcFields->Bool(&mbTrailingComma);
	pcFields->Children(&macpEnumerators);
	pcFields->Scope(&mpcScope);
	pcFields->Symbol(&mpcSymbol);
}

