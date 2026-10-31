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
void CCPPSyntaxStaticAssert::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxDeclaration::Init(pcTree, pcParent);
	mpcCondition = NULL;
	mpcMessage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxStaticAssert::GetKind(void)
{
	return CPPS_StaticAssert;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxStaticAssert::PrettyPrint(CChars* pszDest, int iDepth)
{
	Indent(pszDest, iDepth);
	pszDest->Append("static_assert(");
	PrintNode(pszDest, mpcCondition, iDepth);
	if (mpcMessage)
	{
		pszDest->Append(", ");
		mpcMessage->PrettyPrint(pszDest, iDepth);
	}
	pszDest->Append(");\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxStaticAssert::NumChildren(void)
{
	return 1 + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxStaticAssert::GetChild(size iIndex)
{
	CCPPSyntax*	pcChild;

	if (IsChild(&iIndex, mpcCondition, &pcChild) || IsChild(&iIndex, mpcMessage, &pcChild))
	{
		return pcChild;
	}
	return NULL;
}

