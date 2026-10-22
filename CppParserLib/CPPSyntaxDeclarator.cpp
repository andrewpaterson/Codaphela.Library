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
void CCPPSyntaxDeclarator::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	macpPrefix.Init();
	mbPack = false;
	mpcName = NULL;
	mpcNested = NULL;
	macpSuffixes.Init();
	macpAttributes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclarator::Kill(void)
{
	macpPrefix.Kill();
	macpSuffixes.Kill();
	macpAttributes.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxDeclarator::GetType(void)
{
	return "Declarator";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxDeclarator::GetKind(void)
{
	return CPPS_Declarator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclarator::PrettyPrint(CChars* pszDest, int iDepth)
{
	size		i;
	CCPPSyntax*	pcPrefix;

	//Calling conventions and attributes in the prefix, as in void* __cdecl f(void), are followed by a space.
	for (i = 0; i < macpPrefix.NumElements(); i++)
	{
		pcPrefix = macpPrefix.GetPtr(i);
		pcPrefix->PrettyPrint(pszDest, iDepth);
		if (pcPrefix->GetKind() != CPPS_PointerOperator)
		{
			pszDest->Append(' ');
		}
	}
	if (mbPack)
	{
		pszDest->Append("...");
	}
	if (mpcNested)
	{
		pszDest->Append('(');
		mpcNested->PrettyPrint(pszDest, iDepth);
		pszDest->Append(')');
	}
	else if (mpcName)
	{
		mpcName->PrettyPrint(pszDest, iDepth);
	}
	PrintList(pszDest, &macpSuffixes, "", iDepth);
	if (macpAttributes.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpAttributes, " ", iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxDeclarator::NumChildren(void)
{
	return macpPrefix.NumElements() + 1 + 1 + macpSuffixes.NumElements() + macpAttributes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxDeclarator::GetChild(size iIndex)
{
	if (iIndex < macpPrefix.NumElements())
	{
		return macpPrefix.GetPtr(iIndex);
	}
	iIndex -= macpPrefix.NumElements();

	if (iIndex == 0)
	{
		return mpcName;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcNested;
	}
	iIndex--;

	if (iIndex < macpSuffixes.NumElements())
	{
		return macpSuffixes.GetPtr(iIndex);
	}
	iIndex -= macpSuffixes.NumElements();

	if (iIndex < macpAttributes.NumElements())
	{
		return macpAttributes.GetPtr(iIndex);
	}
	iIndex -= macpAttributes.NumElements();

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxDeclarator::IsEmpty(void)
{
	return (macpPrefix.NumElements() == 0) && !mbPack && (mpcName == NULL) && (mpcNested == NULL) && (macpSuffixes.NumElements() == 0) && (macpAttributes.NumElements() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxFunctionSuffix* CCPPSyntaxDeclarator::GetFunctionSuffix(void)
{
	CCPPSyntax*	pcSuffix;

	//The function suffix that applies to the declared name: (*pf)(int) declares a pointer, not a function.
	if (mpcNested && mpcNested->GetName())
	{
		if (mpcNested->macpPrefix.NumElements() == 0)
		{
			return mpcNested->GetFunctionSuffix();
		}
		return NULL;
	}

	if (macpSuffixes.NumElements() > 0)
	{
		pcSuffix = macpSuffixes.GetPtr(0);
		if (pcSuffix->GetKind() == CPPS_FunctionSuffix)
		{
			return (CCPPSyntaxFunctionSuffix*)pcSuffix;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxDeclarator::IsFunction(void)
{
	return GetFunctionSuffix() != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxName* CCPPSyntaxDeclarator::GetName(void)
{
	if (mpcName)
	{
		return mpcName;
	}
	if (mpcNested)
	{
		return mpcNested->GetName();
	}
	return NULL;
}

