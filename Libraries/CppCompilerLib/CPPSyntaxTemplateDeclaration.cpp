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
void CCPPSyntaxTemplateDeclaration::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxDeclaration::Init(pcTree, pcParent);
	mbExtern = false;
	mbInstantiation = false;
	macpParameters.Init(NodeArrayMallocator());
	mpcDeclaration = NULL;
	mpcScope = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxTemplateDeclaration::Kill(void)
{
	macpParameters.Kill();
	CCPPSyntaxDeclaration::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxTemplateDeclaration::PrettyPrint(CChars* pszDest, int iDepth)
{
	Indent(pszDest, iDepth);
	if (mbExtern)
	{
		pszDest->Append("extern ");
	}
	pszDest->Append("template");
	if (!mbInstantiation)
	{
		pszDest->Append('<');
		PrintList(pszDest, &macpParameters, ", ", iDepth);
		pszDest->Append('>');
	}
	pszDest->AppendNewLine();
	PrintNode(pszDest, mpcDeclaration, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxTemplateDeclaration::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Bool(&mbExtern);
	pcFields->Bool(&mbInstantiation);
	pcFields->Children(&macpParameters);
	pcFields->Child(&mpcDeclaration);
	pcFields->Scope(&mpcScope);
}

