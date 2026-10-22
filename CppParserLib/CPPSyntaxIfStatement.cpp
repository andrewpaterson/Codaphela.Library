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
void CCPPSyntaxIfStatement::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxStatement::Init(pcTree, pcParent);
	mpcCondition = NULL;
	mpcThen = NULL;
	mpcElse = NULL;
	mpcScope = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxIfStatement::Kill(void)
{
	CCPPSyntaxStatement::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxIfStatement::GetType(void)
{
	return "IfStatement";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxIfStatement::GetKind(void)
{
	return CPPS_IfStatement;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxIfStatement::PrettyPrint(CChars* pszDest, int iDepth)
{
	Indent(pszDest, iDepth);
	pszDest->Append("if (");
	PrintCondition(pszDest, mpcCondition, iDepth);
	pszDest->Append(")\n");
	PrintSubStatement(pszDest, mpcThen, iDepth);
	if (mpcElse)
	{
		Indent(pszDest, iDepth);
		pszDest->Append("else\n");
		PrintSubStatement(pszDest, mpcElse, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxIfStatement::NumChildren(void)
{
	return 1 + 1 + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxIfStatement::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcCondition;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcThen;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcElse;
	}
	iIndex--;

	return NULL;
}

