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
void CCPPSyntaxEnumerator::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	miName = -1;
	macpAttributes.Init(NodeArrayMallocator());
	mpcValue = NULL;
	mpcSymbol = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumerator::Kill(void)
{
	macpAttributes.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumerator::PrettyPrint(CChars* pszDest, int iDepth)
{
	AppendToken(pszDest, miName);
	if (macpAttributes.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpAttributes, " ", iDepth);
	}
	if (mpcValue)
	{
		pszDest->Append(" = ");
		mpcValue->PrettyPrint(pszDest, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxEnumerator::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Token(&miName);
	pcFields->Children(&macpAttributes);
	pcFields->Child(&mpcValue);
	pcFields->Symbol(&mpcSymbol);
}

