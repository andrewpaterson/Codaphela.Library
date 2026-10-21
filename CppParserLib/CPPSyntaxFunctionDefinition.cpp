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
void CCPPSyntaxFunctionDefinition::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxDeclaration::Init(pcTree, pcParent);
	mpcSpecifiers = NULL;
	mpcDeclarator = NULL;
	macpMemberInitializers.Init();
	mpcBody = NULL;
	meBody = CPPFB_Body;
	miBodyStart = -1;
	miBodyEnd = -1;
	mpcScope = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxFunctionDefinition::Kill(void)
{
	macpMemberInitializers.Kill();
	CCPPSyntaxDeclaration::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxFunctionDefinition::GetType(void)
{
	return "FunctionDefinition";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxFunctionDefinition::GetKind(void)
{
	return CPPS_FunctionDefinition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxFunctionDefinition::PrettyPrint(CChars* pszDest, int iDepth)
{
	Indent(pszDest, iDepth);
	if (mpcSpecifiers && (mpcSpecifiers->macpSpecifiers.NumElements() > 0))
	{
		mpcSpecifiers->PrettyPrint(pszDest, iDepth);
		pszDest->Append(' ');
	}
	PrintNode(pszDest, mpcDeclarator, iDepth);
	if (macpMemberInitializers.NumElements() > 0)
	{
		pszDest->Append(" : ");
		PrintList(pszDest, &macpMemberInitializers, ", ", iDepth);
	}

	if (meBody == CPPFB_Default)
	{
		pszDest->Append(" = default;\n");
	}
	else if (meBody == CPPFB_Delete)
	{
		pszDest->Append(" = delete;\n");
	}
	else if (meBody == CPPFB_Skipped)
	{
		pszDest->AppendNewLine();
		Indent(pszDest, iDepth);
		AppendTokens(pszDest, miBodyStart, miBodyEnd);
		pszDest->AppendNewLine();
	}
	else
	{
		pszDest->AppendNewLine();
		PrintNode(pszDest, mpcBody, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxFunctionDefinition::NumChildren(void)
{
	return 1 + 1 + macpMemberInitializers.NumElements() + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxFunctionDefinition::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcSpecifiers;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcDeclarator;
	}
	iIndex--;

	if (iIndex < macpMemberInitializers.NumElements())
	{
		return macpMemberInitializers.GetPtr(iIndex);
	}
	iIndex -= macpMemberInitializers.NumElements();

	if (iIndex == 0)
	{
		return mpcBody;
	}
	iIndex--;

	return NULL;
}

