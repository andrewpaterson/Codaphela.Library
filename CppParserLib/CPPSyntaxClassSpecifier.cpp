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
void CCPPSyntaxClassSpecifier::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeSpecifier::Init(pcTree, pcParent);
	meKey = CT_class;
	macpAttributes.Init();
	mpcName = NULL;
	mbFinal = false;
	macpBases.Init();
	macpMembers.Init();
	mpcScope = NULL;
	mpcSymbol = NULL;
	miPack = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxClassSpecifier::Kill(void)
{
	macpAttributes.Kill();
	macpBases.Kill();
	macpMembers.Kill();
	CCPPSyntaxTypeSpecifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxClassSpecifier::GetKind(void)
{
	return CPPS_ClassSpecifier;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxClassSpecifier::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintKeyword(pszDest, meKey);
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
	if (mbFinal)
	{
		pszDest->Append(" final");
	}
	if (macpBases.NumElements() > 0)
	{
		pszDest->Append(" : ");
		PrintList(pszDest, &macpBases, ", ", iDepth);
	}
	pszDest->AppendNewLine();
	Indent(pszDest, iDepth);
	pszDest->Append("{\n");
	PrintDeclarations(pszDest, &macpMembers, iDepth + 1);
	Indent(pszDest, iDepth);
	pszDest->Append('}');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxClassSpecifier::NumChildren(void)
{
	return macpAttributes.NumElements() + 1 + macpBases.NumElements() + macpMembers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxClassSpecifier::GetChild(size iIndex)
{
	CCPPSyntax*	pcChild;

	if (IsChild(&iIndex, &macpAttributes, &pcChild) || IsChild(&iIndex, mpcName, &pcChild) || IsChild(&iIndex, &macpBases, &pcChild) || IsChild(&iIndex, &macpMembers, &pcChild))
	{
		return pcChild;
	}
	return NULL;
}

