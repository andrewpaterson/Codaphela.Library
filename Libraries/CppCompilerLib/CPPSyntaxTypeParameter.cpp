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
void CCPPSyntaxTypeParameter::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	mbTypename = false;
	mbPack = false;
	miName = -1;
	mpcDefault = NULL;
	mpcSymbol = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxTypeParameter::PrettyPrint(CChars* pszDest, int iDepth)
{
	pszDest->Append(mbTypename ? "typename" : "class");
	if (mbPack)
	{
		pszDest->Append("...");
	}
	if (miName != -1)
	{
		pszDest->Append(' ');
		AppendToken(pszDest, miName);
	}
	if (mpcDefault)
	{
		pszDest->Append(" = ");
		mpcDefault->PrettyPrint(pszDest, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxTypeParameter::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Bool(&mbTypename);
	pcFields->Bool(&mbPack);
	pcFields->Token(&miName);
	pcFields->Child(&mpcDefault);
	pcFields->Symbol(&mpcSymbol);
}

