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
void CCPPSyntaxDeclarator::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	macpPrefix.Init(NodeArrayMallocator());
	mbPack = false;
	mpcName = NULL;
	mpcNested = NULL;
	macpSuffixes.Init(NodeArrayMallocator());
	macpAttributes.Init(NodeArrayMallocator());
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
	CCPPSyntax*					pcSuffix;
	CCPPSyntaxFunctionSuffix*	pcNested;

	//The function suffix that applies to the declared name: (*pf)(int) declares a pointer, not a function.
	//(*Pick())(int) declares Pick, a function that returns one; and (f)(int) is f(int).
	if (mpcNested && mpcNested->GetName())
	{
		pcNested = mpcNested->GetFunctionSuffix();
		if (pcNested || !mpcNested->IsParenthesisedName())
		{
			return pcNested;
		}
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
// Whether this is only a name, in as many brackets as it likes: no pointers
// or suffixes change what the declarator outside it declares.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxDeclarator::IsParenthesisedName(void)
{
	if ((macpPrefix.NumElements() != 0) || (macpSuffixes.NumElements() != 0))
	{
		return false;
	}
	return mpcNested ? mpcNested->IsParenthesisedName() : (mpcName != NULL);
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


//////////////////////////////////////////////////////////////////////////
//
// The pointer operators (*, &, &&, Class::*) in the prefix, without the
// calling conventions and attributes, as in void* __cdecl f(void).
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxDeclarator::NumPointers(void)
{
	size	i;
	size	uiPointers;

	uiPointers = 0;
	for (i = 0; i < macpPrefix.NumElements(); i++)
	{
		if (macpPrefix.GetPtr(i)->GetKind() == CPPS_PointerOperator)
		{
			uiPointers++;
		}
	}
	return uiPointers;
}


//////////////////////////////////////////////////////////////////////////
//
// The uiIndex'th pointer operator in the prefix (NumPointers), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxPointerOperator* CCPPSyntaxDeclarator::GetPointer(size uiIndex)
{
	size		i;
	CCPPSyntax*	pcPrefix;

	for (i = 0; i < macpPrefix.NumElements(); i++)
	{
		pcPrefix = macpPrefix.GetPtr(i);
		if (pcPrefix->GetKind() == CPPS_PointerOperator)
		{
			if (uiIndex == 0)
			{
				return (CCPPSyntaxPointerOperator*)pcPrefix;
			}
			uiIndex--;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclarator::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Children(&macpPrefix);
	pcFields->Bool(&mbPack);
	pcFields->Child(&mpcName);
	pcFields->Child(&mpcNested);
	pcFields->Children(&macpSuffixes);
	pcFields->Children(&macpAttributes);
}

