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
void CCPPSyntaxLinkageSpecification::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxDeclaration::Init(pcTree, pcParent);
	miLanguage = -1;
	mbBraced = false;
	macpDeclarations.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxLinkageSpecification::Kill(void)
{
	macpDeclarations.Kill();
	CCPPSyntaxDeclaration::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxLinkageSpecification::GetKind(void)
{
	return CPPS_LinkageSpecification;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxLinkageSpecification::PrettyPrint(CChars* pszDest, int iDepth)
{
	Indent(pszDest, iDepth);
	pszDest->Append("extern ");
	AppendToken(pszDest, miLanguage);
	if (mbBraced)
	{
		pszDest->AppendNewLine();
		Indent(pszDest, iDepth);
		pszDest->Append("{\n");
		PrintDeclarations(pszDest, &macpDeclarations, iDepth + 1);
		Indent(pszDest, iDepth);
		pszDest->Append("}\n");
	}
	else
	{
		pszDest->Append(' ');
		PrintDeclarations(pszDest, &macpDeclarations, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxLinkageSpecification::NumChildren(void)
{
	return macpDeclarations.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxLinkageSpecification::GetChild(size iIndex)
{
	CCPPSyntax*	pcChild;

	if (IsChild(&iIndex, &macpDeclarations, &pcChild))
	{
		return pcChild;
	}
	return NULL;
}

