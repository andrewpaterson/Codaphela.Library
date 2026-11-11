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
void CCPPSyntaxLazyDeclaration::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxDeclaration::Init(pcTree, pcParent);
	mpapcOwner = NULL;
	muiIndex = 0;
	mpsRecord = NULL;
	mpcScope = NULL;
	miPack = 0;
	mbLoaded = false;
	mpcMembers = NULL;
	mpcNextMember = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxLazyDeclaration::PrettyPrint(CChars* pszDest, int iDepth)
{
	//A declaration that was never needed prints as its tokens.
	Indent(pszDest, iDepth);
	AppendTokens(pszDest, miFirstToken, miLastToken);
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxLazyDeclaration::VisitFields(CCPPSyntaxFields* pcFields)
{
	//mpapcOwner and muiIndex aren't fields: where it is in its owner's list, set when it is made; nor is mpsRecord.
	pcFields->UsedScope(&mpcScope);
	pcFields->Integer(&miPack);
	pcFields->Bool(&mbLoaded);
	pcFields->Link(&mpcMembers);
	pcFields->Link(&mpcNextMember);
}
