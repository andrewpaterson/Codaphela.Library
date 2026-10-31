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
void CCPPSyntaxForStatement::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxStatement::Init(pcTree, pcParent);
	mpcInitialiser = NULL;
	mpcCondition = NULL;
	mpcIncrement = NULL;
	mpcBody = NULL;
	mpcScope = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxForStatement::GetKind(void)
{
	return CPPS_ForStatement;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxForStatement::PrettyPrint(CChars* pszDest, int iDepth)
{
	CChars	szInitialiser;

	Indent(pszDest, iDepth);
	pszDest->Append("for (");
	if (mpcInitialiser)
	{
		szInitialiser.Init();
		mpcInitialiser->PrettyPrint(&szInitialiser, 0);
		szInitialiser.StripWhitespace(true);
		pszDest->Append(szInitialiser);
		szInitialiser.Kill();
	}
	else
	{
		pszDest->Append(';');
	}
	if (mpcCondition)
	{
		pszDest->Append(' ');
		PrintCondition(pszDest, mpcCondition, iDepth);
	}
	pszDest->Append(';');
	if (mpcIncrement)
	{
		pszDest->Append(' ');
		mpcIncrement->PrettyPrint(pszDest, iDepth);
	}
	pszDest->Append(")\n");
	PrintSubStatement(pszDest, mpcBody, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxForStatement::NumChildren(void)
{
	return 1 + 1 + 1 + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxForStatement::GetChild(size iIndex)
{
	CCPPSyntax*	pcChild;

	if (IsChild(&iIndex, mpcInitialiser, &pcChild) || IsChild(&iIndex, mpcCondition, &pcChild) || IsChild(&iIndex, mpcIncrement, &pcChild) || IsChild(&iIndex, mpcBody, &pcChild))
	{
		return pcChild;
	}
	return NULL;
}

