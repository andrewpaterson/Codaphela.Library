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
void CCPPSyntaxName::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	mbGlobal = false;
	macpComponents.Init();
	mpcSymbol = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxName::Kill(void)
{
	macpComponents.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxName::GetType(void)
{
	return "Name";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxName::GetKind(void)
{
	return CPPS_Name;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxName::PrettyPrint(CChars* pszDest, int iDepth)
{
	size			i;

	if (mbGlobal)
	{
		pszDest->Append("::");
	}
	for (i = 0; i < macpComponents.NumElements(); i++)
	{
		if (i > 0)
		{
			pszDest->Append("::");
		}
		macpComponents.GetPtr(i)->PrettyPrint(pszDest, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxName::NumChildren(void)
{
	return macpComponents.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxName::GetChild(size iIndex)
{
	if (iIndex < macpComponents.NumElements())
	{
		return macpComponents.GetPtr(iIndex);
	}
	iIndex -= macpComponents.NumElements();

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxNameComponent* CCPPSyntaxName::GetLast(void)
{
	size	uiNumElements;

	uiNumElements = macpComponents.NumElements();
	if (uiNumElements == 0)
	{
		return NULL;
	}
	return (CCPPSyntaxNameComponent*)macpComponents.GetPtr(uiNumElements - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxName::IsSimple(void)
{
	CCPPSyntaxNameComponent*	pcComponent;

	//An unqualified identifier.
	if (mbGlobal || (macpComponents.NumElements() != 1))
	{
		return false;
	}
	pcComponent = GetLast();
	return (pcComponent->meKind == CPPNC_Identifier) && !pcComponent->mbTemplateArguments;
}

