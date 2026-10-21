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
void CCPPSyntaxRangeForStatement::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxStatement::Init(pcTree, pcParent);
	mpcSpecifiers = NULL;
	mpcDeclarator = NULL;
	mpcRange = NULL;
	mpcBody = NULL;
	mpcScope = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxRangeForStatement::Kill(void)
{
	CCPPSyntaxStatement::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxRangeForStatement::GetType(void)
{
	return "RangeForStatement";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxRangeForStatement::GetKind(void)
{
	return CPPS_RangeForStatement;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxRangeForStatement::PrettyPrint(CChars* pszDest, int iDepth)
{
	Indent(pszDest, iDepth);
	pszDest->Append("for (");
	PrintNode(pszDest, mpcSpecifiers, iDepth);
	pszDest->Append(' ');
	PrintNode(pszDest, mpcDeclarator, iDepth);
	pszDest->Append(" : ");
	PrintNode(pszDest, mpcRange, iDepth);
	pszDest->Append(")\n");
	PrintSubStatement(pszDest, mpcBody, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxRangeForStatement::NumChildren(void)
{
	return 1 + 1 + 1 + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxRangeForStatement::GetChild(size iIndex)
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

	if (iIndex == 0)
	{
		return mpcRange;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcBody;
	}
	iIndex--;

	return NULL;
}

