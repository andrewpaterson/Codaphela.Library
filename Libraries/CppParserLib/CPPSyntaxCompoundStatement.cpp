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
void CCPPSyntaxCompoundStatement::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxStatement::Init(pcTree, pcParent);
	macpStatements.Init(NodeArrayMallocator());
	mpcScope = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCompoundStatement::Kill(void)
{
	macpStatements.Kill();
	CCPPSyntaxStatement::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxCompoundStatement::GetKind(void)
{
	return CPPS_CompoundStatement;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCompoundStatement::PrettyPrint(CChars* pszDest, int iDepth)
{
	Indent(pszDest, iDepth);
	pszDest->Append("{\n");
	PrintStatements(pszDest, &macpStatements, iDepth + 1);
	Indent(pszDest, iDepth);
	pszDest->Append("}\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCompoundStatement::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Children(&macpStatements);
	pcFields->Scope(&mpcScope);
}

