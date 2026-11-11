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
void CCPPSyntaxBaseSpecifier::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	macpAttributes.Init(NodeArrayMallocator());
	meAccess = CPPA_None;
	mbVirtual = false;
	mbPack = false;
	mpcName = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBaseSpecifier::Kill(void)
{
	macpAttributes.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBaseSpecifier::PrettyPrint(CChars* pszDest, int iDepth)
{
	if (macpAttributes.NumElements() > 0)
	{
		PrintList(pszDest, &macpAttributes, " ", iDepth);
		pszDest->Append(' ');
	}
	if (mbVirtual)
	{
		pszDest->Append("virtual ");
	}
	if (meAccess == CPPA_Public)
	{
		pszDest->Append("public ");
	}
	else if (meAccess == CPPA_Protected)
	{
		pszDest->Append("protected ");
	}
	else if (meAccess == CPPA_Private)
	{
		pszDest->Append("private ");
	}
	PrintNode(pszDest, mpcName, iDepth);
	if (mbPack)
	{
		pszDest->Append("...");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBaseSpecifier::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Children(&macpAttributes);
	pcFields->Enum(&meAccess);
	pcFields->Bool(&mbVirtual);
	pcFields->Bool(&mbPack);
	pcFields->Child(&mpcName);
}

