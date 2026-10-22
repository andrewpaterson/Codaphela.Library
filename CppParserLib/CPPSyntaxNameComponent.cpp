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
void CCPPSyntaxNameComponent::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	meKind = CPPNC_Identifier;
	mbTemplateKeyword = false;
	miName = -1;
	mbTemplateArguments = false;
	macpTemplateArguments.Init();
	mpcConversionType = NULL;
	mpcSymbol = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxNameComponent::Kill(void)
{
	macpTemplateArguments.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxNameComponent::GetType(void)
{
	return "NameComponent";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxNameComponent::GetKind(void)
{
	return CPPS_NameComponent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxNameComponent::PrettyPrint(CChars* pszDest, int iDepth)
{
	if (mbTemplateKeyword)
	{
		pszDest->Append("template ");
	}
	if (meKind == CPPNC_Destructor)
	{
		pszDest->Append('~');
	}

	if (meKind == CPPNC_Operator)
	{
		pszDest->Append("operator ");
		AppendTokens(pszDest, miName, miLastToken);
		return;
	}
	if (meKind == CPPNC_Conversion)
	{
		pszDest->Append("operator ");
		PrintNode(pszDest, mpcConversionType, iDepth);
		return;
	}

	AppendToken(pszDest, miName);
	if (mbTemplateArguments)
	{
		pszDest->Append('<');
		PrintList(pszDest, &macpTemplateArguments, ", ", iDepth);
		pszDest->Append('>');
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxNameComponent::NumChildren(void)
{
	return macpTemplateArguments.NumElements() + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxNameComponent::GetChild(size iIndex)
{
	if (iIndex < macpTemplateArguments.NumElements())
	{
		return macpTemplateArguments.GetPtr(iIndex);
	}
	iIndex -= macpTemplateArguments.NumElements();

	if (iIndex == 0)
	{
		return mpcConversionType;
	}
	iIndex--;

	return NULL;
}

