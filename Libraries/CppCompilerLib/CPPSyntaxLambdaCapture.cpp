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
void CCPPSyntaxLambdaCapture::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	meKind = CPPLC_Copy;
	mpcName = NULL;
	miName = -1;
	mpcInitializer = NULL;
	mpcSymbol = NULL;
	mbPack = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxLambdaCapture::PrettyPrint(CChars* pszDest, int iDepth)
{
	if (meKind == CPPLC_This)
	{
		pszDest->Append("this");
		return;
	}
	if (meKind == CPPLC_Reference)
	{
		pszDest->Append('&');
	}
	if (mpcName)
	{
		PrintNode(pszDest, mpcName, iDepth);
	}
	else
	{
		AppendToken(pszDest, miName);
	}
	if (mbPack)
	{
		pszDest->Append("...");
	}
	if (mpcInitializer)
	{
		pszDest->Append(" = ");
		PrintNode(pszDest, mpcInitializer, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxLambdaCapture::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Enum(&meKind);
	pcFields->Child(&mpcName);
	pcFields->Token(&miName);
	pcFields->Child(&mpcInitializer);
	pcFields->Symbol(&mpcSymbol);
}
