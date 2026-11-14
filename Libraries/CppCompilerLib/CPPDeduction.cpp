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
#include "CPP65Generator.h"
#include "CPPDeduction.h"


//////////////////////////////////////////////////////////////////////////
//
// Nothing is deduced yet for any of the template's parameters.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::Init(CCPP65Generator* pcUnit, CCPPTemplate* pcTemplate)
{
	size	i;

	mpcUnit = pcUnit;
	mpcTemplate = pcTemplate;
	mapsDeduced.Init();
	malliDeduced.Init();
	for (i = 0; i < pcTemplate->mpcDeclaration->macpParameters.NumElements(); i++)
	{
		mapsDeduced.Add(NULL);
		malliDeduced.Add(0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::Kill(void)
{
	malliDeduced.Kill();
	mapsDeduced.Kill();
	mpcTemplate = NULL;
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Deduces the function template's arguments from a call's, whose types are
// papsArguments (and papcArguments the expressions, or NULL: a forwarding
// reference takes an lvalue's type as a reference).  [temp.deduct.call]
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::Deduce(CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;

	pcSuffix = ((CCPPSyntaxFunctionDefinition*)mpcTemplate->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
	DeduceParameters(pcSuffix, papsArguments, papcArguments);
}


//////////////////////////////////////////////////////////////////////////
//
// Deduces from each of a function suffix's parameters that has an argument
// type in papsArguments, until a parameter pack takes the rest.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::DeduceParameters(CCPPSyntaxFunctionSuffix* pcSuffix, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments)
{
	CCPPSyntaxParameter*	pcParameter;
	size					i;

	for (i = 0; pcSuffix && (i < pcSuffix->macpParameters.NumElements()) && (i < papsArguments->NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		if (!DeduceParameter(pcParameter, papsArguments, papcArguments, i))
		{
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Deduces from the parameter i of a function, whose argument's type is
// papsArguments' i: a parameter declared as T, T*, T** ... or T& takes the
// argument's type, less the pointers, and less the const and volatile the
// parameter has (const T* of const int* is int) or, for T, the argument's
// own (a const int argument is int).  A function parameter pack (Args ... args)
// takes the rest of the arguments' types: the template parameter pack's entry is
// the first and the rest are added after it.  A parameter declared as
// Name<..., N, ...> (or a pointer or reference to one) deduces from the
// argument's instance of Name (DeduceFromInstance).  A pointer to a member
// deduces from the argument's pointer to member (DeduceMemberPointer).  T&& and
// Args&& ... take ForwardedType.  Returns false once a pack has taken the rest.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeduction::DeduceParameter(CCPPSyntaxParameter* pcParameter, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, size i)
{
	CCPPSyntaxTypeSpecifier*	pcType;
	CCPPSyntaxName*				pcName;
	CCPPSyntax*					pcTemplateParameter;
	SCPP65Type*					psArgument;
	size						j;
	size						k;
	size						uiPointers;
	bool						bForwarding;

	DeduceMemberPointer(pcParameter->mpcDeclarator, papsArguments->GetPtr(i));
	pcType = pcParameter->mpcSpecifiers ? pcParameter->mpcSpecifiers->mpcType : NULL;
	if (!pcType || (pcType->GetKind() != CPPS_NamedTypeSpecifier))
	{
		return true;
	}
	pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
	if (pcName->GetLast() && pcName->GetLast()->mbTemplateArguments)
	{
		psArgument = papsArguments->GetPtr(i);
		psArgument = (psArgument && psArgument->IsReference()) ? psArgument->psBase : psArgument;
		if (psArgument && psArgument->IsPointer() && pcParameter->mpcDeclarator && (pcParameter->mpcDeclarator->NumPointers() == 1) && (pcParameter->mpcDeclarator->GetPointer(0)->meKind == CPPP_Pointer))
		{
			psArgument = psArgument->psBase;
		}
		DeduceFromInstance(pcName->GetLast(), psArgument);
		return true;
	}
	for (j = 0; j < mpcTemplate->mpcDeclaration->macpParameters.NumElements(); j++)
	{
		pcTemplateParameter = mpcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
		if ((pcTemplateParameter->GetKind() != CPPS_TypeParameter) || (((CCPPSyntaxTypeParameter*)pcTemplateParameter)->mpcSymbol != pcName->mpcSymbol))
		{
			continue;
		}
		bForwarding = IsForwardingReference(pcParameter);
		if (pcParameter->mpcDeclarator && pcParameter->mpcDeclarator->mbPack)
		{
			//Args ... args takes the types of the rest of the arguments (the pack is the last parameter).
			for (k = i; k < papsArguments->NumElements(); k++)
			{
				psArgument = papsArguments->GetPtr(k);
				psArgument = (psArgument && psArgument->IsReference()) ? psArgument->psBase : psArgument;
				if (psArgument)
				{
					psArgument = bForwarding ? ForwardedType(psArgument, papcArguments ? papcArguments->GetPtr(k) : NULL) : psArgument->Unqualified();
				}
				if (k == i)
				{
					mapsDeduced.SetPtr(j, psArgument);
				}
				else
				{
					mapsDeduced.Add(psArgument);
				}
			}
			return false;
		}
		psArgument = papsArguments->GetPtr(i);
		if (bForwarding)
		{
			if (psArgument && !mapsDeduced.GetPtr(j))
			{
				mapsDeduced.SetPtr(j, ForwardedType(psArgument->WithoutReference(), papcArguments ? papcArguments->GetPtr(i) : NULL));
			}
			return true;
		}
		uiPointers = pcParameter->mpcDeclarator ? pcParameter->mpcDeclarator->NumPointers() : 0;
		if (uiPointers && (pcParameter->mpcDeclarator->GetPointer(0)->meKind != CPPP_Pointer))
		{
			uiPointers = 0;
		}
		if (psArgument && (!pcParameter->mpcDeclarator || (pcParameter->mpcDeclarator->NumPointers() == 0)))
		{
			//A parameter taken by value: the argument's own const and volatile don't count.  [temp.deduct.call] 2
			psArgument = psArgument->Unqualified();
		}
		for (; psArgument && uiPointers > 0; uiPointers--)
		{
			psArgument = psArgument->IsPointer() ? psArgument->psBase : NULL;
		}
		if (psArgument && pcParameter->mpcSpecifiers && (pcParameter->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)))
		{
			//const T (const T*, const T&) takes T without the const it has.
			psArgument = mpcUnit->mcTypes.Qualified(psArgument, psArgument->bConst && !(pcParameter->mpcSpecifiers->muiFlags & CPPDS_CONST), psArgument->bVolatile && !(pcParameter->mpcSpecifiers->muiFlags & CPPDS_VOLATILE));
		}
		if (psArgument && !mapsDeduced.GetPtr(j))
		{
			mapsDeduced.SetPtr(j, psArgument);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the parameter is T&& (or Args&& ...), and not const T&&.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeduction::IsForwardingReference(CCPPSyntaxParameter* pcParameter)
{
	CCPPSyntaxDeclarator*	pcDeclarator;

	pcDeclarator = pcParameter->mpcDeclarator;
	if (!pcDeclarator || (pcDeclarator->NumPointers() != 1) || (pcDeclarator->GetPointer(0)->meKind != CPPP_RValueReference))
	{
		return false;
	}
	return !pcParameter->mpcSpecifiers || !(pcParameter->mpcSpecifiers->muiFlags & CPPDS_CONST);
}


//////////////////////////////////////////////////////////////////////////
//
// What T is for an argument of type psArgument (not a reference) to T&&.
// && is a & here, not a type of its own, so only what passing arguments on
// needs is done (B10 in CppParserTodo.txt has the rest).  T is the argument's
// type with its own const, so an object or a variable binds as it is.  A
// value with no address is const, as only a const reference can bind to one,
// and an array or a function is a const pointer to it, so an array of each
// length isn't an instance of its own.  A string literal's characters are
// const here, as the standard has them, or Init("x") would choose Init(char*)
// over Init(const char*).  (The standard's T is not const for a value, and is
// an array for an array.)
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeduction::ForwardedType(SCPP65Type* psArgument, CCPPSyntax* pcArgument)
{
	SCPP65Type*	psPlace;

	if (psArgument->IsArray())
	{
		return mpcUnit->mcTypes.Qualified(mpcUnit->mcTypes.Pointer(psArgument->psBase), true, false);
	}
	if (psArgument->IsFunction())
	{
		return mpcUnit->mcTypes.Qualified(mpcUnit->mcTypes.Pointer(psArgument), true, false);
	}
	if (psArgument->IsClass() || !pcArgument)
	{
		return psArgument;
	}
	if (mpcUnit->SkipParentheses(pcArgument)->GetKind() == CPPS_StringExpression)
	{
		return mpcUnit->mcTypes.Qualified(mpcUnit->mcTypes.Pointer(mpcUnit->mcTypes.Qualified(psArgument->psBase, true, false)), true, false);
	}

	//An array's type here is already the pointer it decays to, but its place is the array.
	psPlace = mpcUnit->PlaceOf(pcArgument);
	if (psPlace && !psPlace->WithoutReference()->IsArray())
	{
		return psArgument;
	}
	return mpcUnit->mcTypes.Qualified(psArgument, true, psArgument->bVolatile);
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter declared as a pointer to a member of a template parameter, void
// (M::*f)(A*, V), deduces M as the class of the argument's pointer to member
// (&CTarget::Changed), and A and V from the parameters of the argument's member
// function, as a call's arguments deduce (DeduceParameters).
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::DeduceMemberPointer(CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type* psArgument)
{
	CCPPSyntaxPointerOperator*	pcPointer;
	CCPPSyntaxDeclarator*		pcOuter;
	CCPPSyntax*					pcSuffix;
	CCPPSyntax*					pcTemplateParameter;
	CCPPSymbol*					pcSymbol;
	size						i;
	size						j;

	psArgument = (psArgument && psArgument->IsReference()) ? psArgument->psBase : psArgument;
	if (!psArgument || !psArgument->IsMemberPointer() || !psArgument->pcClass)
	{
		return;
	}
	for (pcOuter = NULL; pcDeclarator; pcOuter = pcDeclarator, pcDeclarator = pcDeclarator->mpcNested)
	{
		for (i = 0; i < pcDeclarator->NumPointers(); i++)
		{
			pcPointer = pcDeclarator->GetPointer(i);
			if (pcPointer->meKind != CPPP_MemberPointer)
			{
				continue;
			}
			pcSymbol = pcPointer->mpcClass ? pcPointer->mpcClass->mpcSymbol : NULL;
			for (j = 0; pcSymbol && (j < mpcTemplate->mpcDeclaration->macpParameters.NumElements()); j++)
			{
				pcTemplateParameter = mpcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
				if ((pcTemplateParameter->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)pcTemplateParameter)->mpcSymbol == pcSymbol) && !mapsDeduced.GetPtr(j))
				{
					mapsDeduced.SetPtr(j, psArgument->pcClass->mpsType);
				}
			}

			//The member function's parameters follow the parentheses around (M::*f).
			pcSuffix = (pcOuter && (pcOuter->macpSuffixes.NumElements() > 0)) ? pcOuter->macpSuffixes.GetPtr(0) : NULL;
			if (pcSuffix && (pcSuffix->GetKind() == CPPS_FunctionSuffix) && psArgument->psBase && psArgument->psBase->IsFunction())
			{
				DeduceParameters((CCPPSyntaxFunctionSuffix*)pcSuffix, psArgument->psBase->papsParameters, NULL);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Deduces the template parameters named in a parameter's template arguments,
// Name<M, J>, from psArgument's class: an instance of the class template Name, or a
// class derived from one.  Only instances of the primary template are looked in.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::DeduceFromInstance(CCPPSyntaxNameComponent* pcComponent, SCPP65Type* psArgument)
{
	CCPPClass*	pcInstance;
	SCPPBinding*	psBinding;
	CCPPSyntax*		pcTemplateParameter;
	CCPPSymbol*		pcSymbol;
	size			i;
	size			j;

	if (!psArgument || !psArgument->IsClass() || !pcComponent->mpcSymbol)
	{
		return;
	}
	pcInstance = FindInstanceBase(psArgument->pcClass, pcComponent->mpcSymbol);
	if (!pcInstance || (pcInstance->mpcTemplate->mpcPrimary != pcInstance->mpcTemplate) || !pcInstance->mpcBindings)
	{
		return;
	}
	for (i = 0; (i < pcComponent->macpTemplateArguments.NumElements()) && (i < pcInstance->mpcBindings->macBindings.NumElements()); i++)
	{
		pcSymbol = GetArgumentSymbol(pcComponent->macpTemplateArguments.GetPtr(i));
		psBinding = pcInstance->mpcBindings->macBindings.Get(i);
		for (j = 0; pcSymbol && (j < mpcTemplate->mpcDeclaration->macpParameters.NumElements()); j++)
		{
			pcTemplateParameter = mpcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
			if ((pcTemplateParameter->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)pcTemplateParameter)->mpcSymbol == pcSymbol) && psBinding->psType && !mapsDeduced.GetPtr(j))
			{
				mapsDeduced.SetPtr(j, psBinding->psType);
			}
			else if ((pcTemplateParameter->GetKind() == CPPS_Parameter) && (((CCPPSyntaxParameter*)pcTemplateParameter)->mpcSymbol == pcSymbol) && !psBinding->psType && !mapsDeduced.GetPtr(j))
			{
				mapsDeduced.SetPtr(j, &mpcUnit->mcTypes.msInt);
				malliDeduced.SetValue(j, psBinding->lliValue);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pcClass if it is an instance of the class template pcTemplate names, else the
// first of its bases that is.  NULL if none is.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPDeduction::FindInstanceBase(CCPPClass* pcClass, CCPPSymbol* pcTemplate)
{
	CCPPClass*	pcFound;
	size			i;

	if (pcClass->mpcTemplate && (pcClass->mpcTemplate->mpcPrimary->mpcSymbol == pcTemplate))
	{
		return pcClass;
	}
	if (!pcClass->HasBases())
	{
		mpcUnit->BeginQuiet();
		mpcUnit->ResolveBases(pcClass, NULL);
		mpcUnit->EndQuiet();
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcFound = FindInstanceBase(pcClass->macBases.Get(i)->pcClass, pcTemplate);
		if (pcFound)
		{
			return pcFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol a template argument is just the name of (J or T), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPDeduction::GetArgumentSymbol(CCPPSyntax* pcArgument)
{
	CCPPSyntaxTypeId*			pcTypeId;
	CCPPSyntaxTypeSpecifier*	pcType;

	pcArgument = mpcUnit->SkipParentheses(pcArgument);
	if (pcArgument->GetKind() == CPPS_IdExpression)
	{
		return ((CCPPSyntaxIdExpression*)pcArgument)->mpcName->mpcSymbol;
	}
	if (pcArgument->GetKind() != CPPS_TypeId)
	{
		return NULL;
	}
	pcTypeId = (CCPPSyntaxTypeId*)pcArgument;
	if (pcTypeId->mpcDeclarator && ((pcTypeId->mpcDeclarator->NumPointers() > 0) || (pcTypeId->mpcDeclarator->macpSuffixes.NumElements() > 0)))
	{
		return NULL;
	}
	pcType = pcTypeId->mpcSpecifiers ? pcTypeId->mpcSpecifiers->mpcType : NULL;
	if (!pcType || (pcType->GetKind() != CPPS_NamedTypeSpecifier))
	{
		return NULL;
	}
	return ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName->mpcSymbol;
}

