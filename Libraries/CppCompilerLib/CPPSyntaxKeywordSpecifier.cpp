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
void CCPPSyntaxKeywordSpecifier::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	meKeyword = CT_int;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxKeywordSpecifier::PrettyPrint(CChars* pszDest, int iDepth)
{
	AppendToken(pszDest, miFirstToken);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxKeywordSpecifier::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Enum(&meKeyword);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a list of nodes (a declarator's qualifiers) has a keyword.
//
//////////////////////////////////////////////////////////////////////////
bool CPPHasKeyword(CArraySyntax* pacpNodes, ECppTokens eKeyword)
{
	return CPPHasKeyword(pacpNodes, eKeyword, eKeyword);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a list of nodes has a keyword that is spelled two ways (const and
// __const__).
//
//////////////////////////////////////////////////////////////////////////
bool CPPHasKeyword(CArraySyntax* pacpNodes, ECppTokens eKeyword, ECppTokens eAlsoSpelled)
{
	CCPPSyntax*	pcNode;
	ECppTokens	eNode;
	size		i;

	for (i = 0; i < pacpNodes->NumElements(); i++)
	{
		pcNode = pacpNodes->GetPtr(i);
		if (pcNode->GetKind() == CPPS_KeywordSpecifier)
		{
			eNode = ((CCPPSyntaxKeywordSpecifier*)pcNode)->meKeyword;
			if ((eNode == eKeyword) || (eNode == eAlsoSpelled))
			{
				return true;
			}
		}
	}
	return false;
}

