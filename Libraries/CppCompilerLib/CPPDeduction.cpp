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
#include "CPPUnit.h"
#include "CPPDeduction.h"


//////////////////////////////////////////////////////////////////////////
//
// Nothing is deduced yet for any of the template's parameters.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::Init(CCPPUnit* pcUnit, CCPPTemplate* pcTemplate)
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

	pcSuffix = mpcTemplate->FunctionDeclarator() ? mpcTemplate->FunctionDeclarator()->GetFunctionSuffix() : NULL;
	DeduceParameters(pcSuffix, papsArguments, papcArguments);
}


//////////////////////////////////////////////////////////////////////////
//
// Deduces a conversion function template's parameter from the type it is
// to convert to: operator T() to long has T long, operator T*() to char*
// T char.  Converting to what isn't a reference, the type is taken without
// its reference and its own const.  [temp.deduct.conv]
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::DeduceConversion(CCPPSyntaxTypeId* pcType, SCPP65Type* psTo)
{
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntaxTypeSpecifier*	pcSpecifier;
	CCPPSymbol*					pcSymbol;
	CCPPSyntax*					pcParameter;
	SCPP65Type*					psType;
	ECPPPointer					eOuter;
	size						j;

	pcDeclarator = pcType->mpcDeclarator;
	pcSpecifier = pcType->mpcSpecifiers ? pcType->mpcSpecifiers->mpcType : NULL;
	if (!pcSpecifier || (pcSpecifier->GetKind() != CPPS_NamedTypeSpecifier))
	{
		return;
	}
	pcSymbol = ((CCPPSyntaxNamedTypeSpecifier*)pcSpecifier)->mpcName->mpcSymbol;

	eOuter = (pcDeclarator && (pcDeclarator->NumPointers() > 0)) ? pcDeclarator->GetPointer(pcDeclarator->NumPointers() - 1)->meKind : CPPP_Pointer;
	psType = ((eOuter == CPPP_Reference) || (eOuter == CPPP_RValueReference)) ? psTo : psTo->WithoutReference()->Unqualified();
	psType = pcDeclarator ? StripDeclarator(pcDeclarator, psType) : psType;
	if (!psType)
	{
		return;
	}
	psType = WithoutQualifiersOf(pcType->mpcSpecifiers, psType);
	for (j = 0; j < mpcTemplate->mpcDeclaration->macpParameters.NumElements(); j++)
	{
		pcParameter = mpcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
		if ((pcParameter->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol == pcSymbol) && !mapsDeduced.GetPtr(j))
		{
			mapsDeduced.SetPtr(j, psType);
		}
	}
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
	SCPP65Type*					psArray;
	size						j;
	size						k;
	size						uiPointers;
	bool						bForwarding;

	DeduceMemberPointer(pcParameter->mpcDeclarator, papsArguments->GetPtr(i));
	pcType = pcParameter->mpcSpecifiers ? pcParameter->mpcSpecifiers->mpcType : NULL;
	psArray = NULL;
	if (pcParameter->mpcDeclarator && HasArray(pcParameter->mpcDeclarator) && papsArguments->GetPtr(i))
	{
		//T (&a)[N] of an array: N is its bound and T its element's type.  [temp.deduct.type] 8
		//The argument as the array it is, not the pointer it decays to.
		psArgument = papcArguments ? mpcUnit->PlaceTypeOf(papcArguments->GetPtr(i)) : NULL;
		psArray = StripDeclarator(pcParameter->mpcDeclarator, psArgument ? psArgument : papsArguments->GetPtr(i));
		if (!psArray)
		{
			return true;
		}
	}
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
		if (psArray)
		{
			psArray = WithoutQualifiersOf(pcParameter->mpcSpecifiers, psArray);
			if (!mapsDeduced.GetPtr(j))
			{
				mapsDeduced.SetPtr(j, psArray);
			}
			return true;
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
		if (psArgument)
		{
			psArgument = WithoutQualifiersOf(pcParameter->mpcSpecifiers, psArgument);
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
// const T (const T*, const T&) takes T without the const it has: psType
// without the const and volatile the specifiers have.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeduction::WithoutQualifiersOf(CCPPSyntaxDeclSpecifiers* pcSpecifiers, SCPP65Type* psType)
{
	uint32	uiFlags;

	uiFlags = pcSpecifiers ? pcSpecifiers->muiFlags : 0;
	if (!(uiFlags & (CPPDS_CONST | CPPDS_VOLATILE)))
	{
		return psType;
	}
	return mpcUnit->GetTypes()->Qualified(psType, psType->bConst && !(uiFlags & CPPDS_CONST), psType->bVolatile && !(uiFlags & CPPDS_VOLATILE));
}


//////////////////////////////////////////////////////////////////////////
//
// True if the declarator, or one nested in it, has an array suffix.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeduction::HasArray(CCPPSyntaxDeclarator* pcDeclarator)
{
	size	i;

	for (i = 0; i < pcDeclarator->macpSuffixes.NumElements(); i++)
	{
		if (pcDeclarator->macpSuffixes.GetPtr(i)->GetKind() == CPPS_ArraySuffix)
		{
			return true;
		}
	}
	return pcDeclarator->mpcNested && HasArray(pcDeclarator->mpcNested);
}


//////////////////////////////////////////////////////////////////////////
//
// The type a parameter's specifiers name, from psType, the type of its
// argument: what its declarator adds taken off outside in (a nested
// declarator, the array suffixes, then the pointers and references, last
// first), each array bound a template parameter names deduced (DeduceBound).
// A reference takes nothing off an argument that isn't one (an lvalue).
// NULL if the argument doesn't have that form.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeduction::StripDeclarator(CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type* psType)
{
	CCPPSyntaxPointerOperator*	pcOperator;
	CCPPSyntax*					pcSuffix;
	size						i;

	if (pcDeclarator->mpcNested)
	{
		psType = StripDeclarator(pcDeclarator->mpcNested, psType);
	}
	for (i = 0; psType && (i < pcDeclarator->macpSuffixes.NumElements()); i++)
	{
		pcSuffix = pcDeclarator->macpSuffixes.GetPtr(i);
		psType = psType->WithoutReference();
		if ((pcSuffix->GetKind() != CPPS_ArraySuffix) || !psType->IsArray())
		{
			return NULL;
		}
		DeduceBound(((CCPPSyntaxArraySuffix*)pcSuffix)->mpcSize, psType->iCount);
		psType = psType->psBase;
	}
	for (i = pcDeclarator->NumPointers(); psType && (i > 0); i--)
	{
		pcOperator = pcDeclarator->GetPointer(i - 1);
		if (pcOperator->meKind == CPPP_Pointer)
		{
			psType = psType->IsPointer() ? psType->psBase : NULL;
		}
		else if ((pcOperator->meKind == CPPP_Reference) || (pcOperator->meKind == CPPP_RValueReference))
		{
			psType = psType->WithoutReference();
		}
		else
		{
			return NULL;
		}
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// An array bound the template's non-type parameter is the name of takes
// the argument's bound (iCount; an unknown one deduces nothing).
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::DeduceBound(CCPPSyntax* pcSize, int iCount)
{
	CCPPSymbol*	pcSymbol;
	CCPPSyntax*	pcTemplateParameter;
	size		j;

	pcSymbol = (pcSize && (iCount > 0)) ? GetArgumentSymbol(pcSize) : NULL;
	for (j = 0; pcSymbol && (j < mpcTemplate->mpcDeclaration->macpParameters.NumElements()); j++)
	{
		pcTemplateParameter = mpcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
		if ((pcTemplateParameter->GetKind() == CPPS_Parameter) && (((CCPPSyntaxParameter*)pcTemplateParameter)->mpcSymbol == pcSymbol) && !mapsDeduced.GetPtr(j))
		{
			mapsDeduced.SetPtr(j, &mpcUnit->GetTypes()->msInt);
			malliDeduced.SetValue(j, iCount);
		}
	}
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
// What T is for an argument of type psArgument (not a reference) to T&&:
// U& for what has a name (so T&& is U&, and the argument binds as it is,
// with its own const), and U for a value or what is about to go (so T&&
// binds to it).  [temp.deduct.call] 3.  Not as the standard has it: an
// array or a function is a const pointer to it, so an array of each
// length isn't an instance of its own, and a string literal's characters
// are const, or Init("x") would choose Init(char*) over Init(const char*).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeduction::ForwardedType(SCPP65Type* psArgument, CCPPSyntax* pcArgument)
{
	CCPP65Types*	pcTypes;
	SCPP65Type*		psPlace;

	pcTypes = mpcUnit->GetTypes();
	if (psArgument->IsArray())
	{
		return pcTypes->Qualified(pcTypes->Pointer(psArgument->psBase), true, false);
	}
	if (psArgument->IsFunction())
	{
		return pcTypes->Qualified(pcTypes->Pointer(psArgument), true, false);
	}
	if (!pcArgument)
	{
		return psArgument;
	}
	if (mpcUnit->IsStringLiteral(mpcUnit->SkipParentheses(pcArgument)))
	{
		return pcTypes->Qualified(pcTypes->Pointer(pcTypes->Qualified(psArgument->psBase, true, false)), true, false);
	}
	if (mpcUnit->IsRvalue(pcArgument))
	{
		return psArgument->Unqualified();
	}

	//An array's or a function's type here is already the pointer it becomes, but its place is the array or the function.
	psPlace = mpcUnit->PlaceOf(pcArgument);
	if (psPlace && (psPlace->WithoutReference()->IsArray() || psPlace->WithoutReference()->IsFunction()))
	{
		return pcTypes->Qualified(psArgument, true, psArgument->bVolatile);
	}
	return pcTypes->Reference(psArgument);
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
				mapsDeduced.SetPtr(j, &mpcUnit->GetTypes()->msInt);
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


//////////////////////////////////////////////////////////////////////////
//
// None of the specialisation's parameters is deduced yet.  A full specialisation (template <>) is more special
// than any partial one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSpecialisationMatch::Init(CCPPUnit* pcUnit, CCPPTemplate* pcSpecialisation)
{
	CCPPSyntax*		pcParameter;
	SCPPBinding*	psBinding;
	size			i;

	mpcUnit = pcUnit;
	mpcSpecialisation = pcSpecialisation;
	mpcDeduced = pcUnit->NewBindings();
	masDeferred.Init();
	for (i = 0; i < pcSpecialisation->mpcDeclaration->macpParameters.NumElements(); i++)
	{
		pcParameter = pcSpecialisation->mpcDeclaration->macpParameters.GetPtr(i);
		psBinding = mpcDeduced->macBindings.Add();
		memset(psBinding, 0, sizeof(SCPPBinding));
		psBinding->pcParameter = (pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol;
	}
	miScore = (pcSpecialisation->mpcDeclaration->macpParameters.NumElements() == 0) ? 1000 : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSpecialisationMatch::Kill(void)
{
	masDeferred.Kill();
	mpcDeduced = NULL;
	mpcSpecialisation = NULL;
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the specialisation's pattern matches pcArguments, with every one
// of its type parameters deduced, and what it deduced them from left (the
// types its parameters are only used in: MatchDeferred) the arguments' too.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::Match(CCPPTemplateArguments* pcArguments)
{
	CCPPSymbol*		pcSymbol;
	SCPPBinding*	psBinding;
	size			i;

	if (!MatchPattern(&mpcSpecialisation->mpcPattern->macpTemplateArguments, pcArguments))
	{
		return false;
	}
	for (i = 0; i < mpcDeduced->macBindings.NumElements(); i++)
	{
		psBinding = mpcDeduced->macBindings.Get(i);
		pcSymbol = psBinding->pcParameter;
		if (pcSymbol && (pcSymbol->meKind != CPPSK_ValueParameter) && !psBinding->psType && !psBinding->IsPack())
		{
			return false;
		}
	}
	return MatchDeferred();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a specialisation's template arguments (papcPattern) match an
// instance's arguments, each of a pack's elements an argument of its own
// (FlattenArguments).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchPattern(CArraySyntax* papcPattern, CCPPTemplateArguments* pcArguments)
{
	CArrayCPPBindings	asArguments;
	bool				bResult;

	asArguments.Init();
	FlattenArguments(pcArguments, &asArguments);
	bResult = MatchArguments(papcPattern, &asArguments);
	asArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a list of patterns (a specialisation's template arguments, or
// the parameters of a function type in one) matches the arguments, one
// each.  The specialisation's parameters are deduced as they are met; a
// last pattern that expands one of its parameter packs (T...) takes the
// rest of the arguments (MatchPack).  The score counts how much of the
// arguments' structure the patterns name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchArguments(CArraySyntax* papcPattern, CArrayCPPBindings* pasArguments)
{
	CCPPSyntax*		pcPattern;
	SCPPBinding*	psArgument;
	SCPPBinding*	psBinding;
	SCPPBinding*	psPack;
	CCPPSymbol*		pcSymbol;
	size			i;
	size			k;
	int64			lliValue;

	for (i = 0, k = 0; i < papcPattern->NumElements(); i++)
	{
		pcPattern = papcPattern->GetPtr(i);
		if (IsPackExpansion(pcPattern))
		{
			psPack = ExpandedParameter(pcPattern);
			return psPack && (i + 1 == papcPattern->NumElements()) && MatchPack(psPack, pasArguments, k);
		}
		if (k >= pasArguments->NumElements())
		{
			return false;
		}
		psArgument = pasArguments->Get(k++);
		if (psArgument->psType)
		{
			if (!MatchType(pcPattern, psArgument->psType))
			{
				return false;
			}
			continue;
		}

		//A non-type argument: a parameter of the specialisation takes it, a constant must equal it.
		pcSymbol = (pcPattern->GetKind() == CPPS_IdExpression) ? ((CCPPSyntaxIdExpression*)pcPattern)->mpcName->mpcSymbol : NULL;
		psBinding = pcSymbol ? mpcDeduced->FindOwn(pcSymbol) : NULL;
		if (psBinding)
		{
			psBinding->lliValue = psArgument->lliValue;
			psBinding->psType = NULL;
			continue;
		}
		if (!mpcUnit->Constant(pcPattern, &lliValue) || (lliValue != psArgument->lliValue))
		{
			return false;
		}
		miScore++;
	}
	return k == pasArguments->NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// An instance's arguments one per element: a pack's types or values are
// each an argument.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSpecialisationMatch::FlattenArguments(CCPPTemplateArguments* pcArguments, CArrayCPPBindings* pasArguments)
{
	SCPPBinding*	psBinding;
	SCPPBinding*	psElement;
	size			i;
	size			j;

	for (i = 0; i < pcArguments->macBindings.NumElements(); i++)
	{
		psBinding = pcArguments->macBindings.Get(i);
		if (!psBinding->IsPack())
		{
			*pasArguments->Add() = *psBinding;
			continue;
		}
		for (j = 0; j < psBinding->PackSize(); j++)
		{
			psElement = pasArguments->Add();
			*psElement = *psBinding;
			psElement->papsPack = NULL;
			psElement->palliPack = NULL;
			psElement->psType = psBinding->papsPack ? psBinding->papsPack->GetPtr(j) : NULL;
			psElement->lliValue = psBinding->palliPack ? psBinding->palliPack->GetValue(j) : 0;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The specifiers and the declarator (NULL if it has none) of a pattern
// for a type: a template argument that is a type-id, or a parameter of a
// function type.  False if the pattern is neither.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::PatternParts(CCPPSyntax* pcPattern, CCPPSyntaxDeclSpecifiers** ppcSpecifiers, CCPPSyntaxDeclarator** ppcDeclarator)
{
	if (pcPattern->GetKind() == CPPS_TypeId)
	{
		*ppcSpecifiers = ((CCPPSyntaxTypeId*)pcPattern)->mpcSpecifiers;
		*ppcDeclarator = ((CCPPSyntaxTypeId*)pcPattern)->mpcDeclarator;
		return *ppcSpecifiers != NULL;
	}
	if (pcPattern->GetKind() == CPPS_Parameter)
	{
		*ppcSpecifiers = ((CCPPSyntaxParameter*)pcPattern)->mpcSpecifiers;
		*ppcDeclarator = ((CCPPSyntaxParameter*)pcPattern)->mpcDeclarator;
		return *ppcSpecifiers != NULL;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a pattern is a pack expansion: T..., N... or, of a function
// type's parameters, A... (the declarator has the dots).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::IsPackExpansion(CCPPSyntax* pcPattern)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxDeclarator*		pcDeclarator;

	if (pcPattern->GetKind() == CPPS_PackExpansionExpression)
	{
		return true;
	}
	if (!PatternParts(pcPattern, &pcSpecifiers, &pcDeclarator))
	{
		return false;
	}
	return ((pcPattern->GetKind() == CPPS_TypeId) && ((CCPPSyntaxTypeId*)pcPattern)->mbPack) || (pcDeclarator && pcDeclarator->mbPack);
}


//////////////////////////////////////////////////////////////////////////
//
// The binding of the specialisation's parameter pack a pack expansion
// expands, if that is all it is (T..., not T*...), or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPPSpecialisationMatch::ExpandedParameter(CCPPSyntax* pcPattern)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntax*					pcExpression;

	if (PatternParts(pcPattern, &pcSpecifiers, &pcDeclarator))
	{
		if (!NamedType(pcSpecifiers) || (pcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)) ||
			(pcDeclarator && ((pcDeclarator->NumPointers() > 0) || pcDeclarator->mpcNested || (pcDeclarator->macpSuffixes.NumElements() > 0))))
		{
			return NULL;
		}
		return mpcDeduced->FindOwn(NamedType(pcSpecifiers)->mpcName->mpcSymbol);
	}
	pcExpression = ((CCPPSyntaxPackExpansionExpression*)pcPattern)->mpcPattern->WithoutParentheses();
	if (pcExpression->GetKind() != CPPS_IdExpression)
	{
		return NULL;
	}
	return mpcDeduced->FindOwn(((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol);
}


//////////////////////////////////////////////////////////////////////////
//
// The type specifiers name by a name (T, X<T>, typename T::Type), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxNamedTypeSpecifier* CCPPSpecialisationMatch::NamedType(CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	return (pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_NamedTypeSpecifier)) ? (CCPPSyntaxNamedTypeSpecifier*)pcSpecifiers->mpcType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The parameter pack psPack takes the arguments from uiFirst on: all
// types for a type pack, all values for a non-type pack.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchPack(SCPPBinding* psPack, CArrayCPPBindings* pasArguments, size uiFirst)
{
	SCPPBinding*	psArgument;
	bool			bTypes;
	size			k;

	bTypes = psPack->pcParameter && (psPack->pcParameter->meKind != CPPSK_ValueParameter);
	if (bTypes)
	{
		psPack->papsPack = mpcUnit->NewPack();
	}
	else
	{
		psPack->palliPack = mpcUnit->NewValuePack();
	}
	for (k = uiFirst; k < pasArguments->NumElements(); k++)
	{
		psArgument = pasArguments->Get(k);
		if ((psArgument->psType != NULL) != bTypes)
		{
			return false;
		}
		if (bTypes)
		{
			psPack->papsPack->Add(psArgument->psType);
		}
		else
		{
			psPack->palliPack->Add(psArgument->lliValue);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an array bound in a pattern matches an array's (iCount, 0 for an
// unknown bound): none matches only an unknown bound; one of the
// specialisation's parameters takes a known one; a constant must equal it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchBound(CCPPSyntax* pcSize, int iCount)
{
	CCPPSymbol*		pcSymbol;
	SCPPBinding*	psBinding;
	int64			lliValue;

	if (!pcSize)
	{
		return iCount == 0;
	}
	if (iCount == 0)
	{
		return false;
	}
	pcSize = mpcUnit->SkipParentheses(pcSize);
	pcSymbol = (pcSize->GetKind() == CPPS_IdExpression) ? ((CCPPSyntaxIdExpression*)pcSize)->mpcName->mpcSymbol : NULL;
	psBinding = pcSymbol ? mpcDeduced->FindOwn(pcSymbol) : NULL;
	if (psBinding)
	{
		psBinding->lliValue = iCount;
		psBinding->psType = NULL;
		return true;
	}
	return mpcUnit->Constant(pcSize, &lliValue) && (lliValue == iCount);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a pattern for a type matches a type argument: T, const T, a
// class template's instance X<T>, and what a declarator makes of one (T*,
// T&, T[N], T[], R(A...), R (*)(A), M C::*, R (C::*)(A...) const), each of
// the specialisation's parameters taking what stands where it is written.
// A type that names none of them where one can be deduced from (int,
// typename T::Type, an alias template's Void<T>) is the argument once they
// are known (MatchDeferred).  [temp.deduct.type] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchType(CCPPSyntax* pcPattern, SCPP65Type* psArgument)
{
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxNamedTypeSpecifier*	pcNamed;
	CCPPSyntaxNameComponent*		pcComponent;
	SCPPBinding*					psBinding;
	SCPPDeferredPattern*			psDeferred;
	CCPPTemplate*					pcTemplate;
	SCPP65Type*						psType;

	psType = psArgument;
	if (!PatternParts(pcPattern, &pcSpecifiers, &pcDeclarator) || (pcDeclarator && !MatchDeclarator(pcDeclarator, &psType)))
	{
		return false;
	}
	if (pcSpecifiers->muiFlags & CPPDS_CONST)
	{
		//const T matches a const type, T the type without its const.
		if (!psType->bConst)
		{
			return false;
		}
		psType = mpcUnit->GetTypes()->Qualified(psType, false, psType->bVolatile);
		miScore++;
	}

	//T takes the type.
	pcNamed = NamedType(pcSpecifiers);
	psBinding = pcNamed ? mpcDeduced->FindOwn(pcNamed->mpcName->mpcSymbol) : NULL;
	if (psBinding)
	{
		return Deduce(psBinding, psType);
	}

	//X<T> matches an instance of the class template X with matching arguments.
	pcComponent = pcNamed ? pcNamed->mpcName->GetLast() : NULL;
	pcTemplate = (pcComponent && pcComponent->mbTemplateArguments && pcComponent->mpcSymbol && (pcComponent->mpcSymbol->meKind == CPPSK_ClassTemplate)) ? mpcUnit->GetTemplate(pcComponent->mpcSymbol) : NULL;
	if (pcTemplate)
	{
		if (!psType->IsClass() || !psType->pcClass->mpcTemplate || (psType->pcClass->mpcTemplate->mpcPrimary != pcTemplate))
		{
			return false;
		}
		miScore++;
		return MatchPattern(&pcComponent->macpTemplateArguments, psType->pcClass->mpcBindings);
	}

	psDeferred = masDeferred.Add();
	psDeferred->pcPattern = pcPattern;
	psDeferred->psArgument = psArgument;
	miScore += 2;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// One of the specialisation's parameters takes the type, which every
// place it is written must agree on (X<T, T> isn't X<int, char>).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::Deduce(SCPPBinding* psBinding, SCPP65Type* psType)
{
	if (psBinding->psType)
	{
		return psBinding->psType->Same(psType);
	}
	psBinding->psType = psType;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes what a pattern's declarator makes of its type off the argument's
// (*ppsType), outside in as the declarator applies it last (CCPPDeclarators::
// GetDeclaratorType): a nested declarator, the array and function suffixes
// from the first, then the pointers and references from the last.  False
// if the argument isn't made so.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchDeclarator(CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type** ppsType)
{
	CCPPSyntax*		pcSuffix;
	SCPP65Type*		psType;
	size			i;

	if (pcDeclarator->mpcNested && !MatchDeclarator(pcDeclarator->mpcNested, ppsType))
	{
		return false;
	}
	psType = *ppsType;
	for (i = 0; i < pcDeclarator->macpSuffixes.NumElements(); i++)
	{
		pcSuffix = pcDeclarator->macpSuffixes.GetPtr(i);
		if (pcSuffix->GetKind() == CPPS_ArraySuffix)
		{
			//T[N] matches an array, N taking its bound, and T[] an array of unknown bound.
			if (!psType->IsArray() || !MatchBound(((CCPPSyntaxArraySuffix*)pcSuffix)->mpcSize, psType->iCount))
			{
				return false;
			}
		}
		else if ((pcSuffix->GetKind() != CPPS_FunctionSuffix) || !MatchFunction((CCPPSyntaxFunctionSuffix*)pcSuffix, psType))
		{
			return false;
		}
		psType = psType->psBase;
		miScore++;
	}
	for (i = pcDeclarator->NumPointers(); i > 0; i--)
	{
		if (!MatchPointer(pcDeclarator->GetPointer(i - 1), psType))
		{
			return false;
		}
		psType = psType->psBase;
		miScore++;
	}
	*ppsType = psType;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the type is what a pointer operator of a pattern makes: a
// pointer, a reference of its kind, or a pointer to a member of the class
// the operator names (MatchClass).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchPointer(CCPPSyntaxPointerOperator* pcOperator, SCPP65Type* psType)
{
	switch (pcOperator->meKind)
	{
	case CPPP_Pointer:
		return psType->IsPointer();
	case CPPP_Reference:
		return psType->IsLvalueReference();
	case CPPP_RValueReference:
		return psType->IsRvalueReference();
	case CPPP_MemberPointer:
		return psType->IsMemberPointer() && pcOperator->mpcClass && MatchClass(pcOperator->mpcClass, psType->pcClass);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the class a pointer to a member is of in a pattern (C::*) is
// pcClass: one of the specialisation's parameters takes it, a class's name
// must be its.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchClass(CCPPSyntaxName* pcName, CCPPClass* pcClass)
{
	SCPPBinding*	psBinding;
	CCPPClass*		pcNamed;

	psBinding = mpcDeduced->FindOwn(pcName->mpcSymbol);
	if (psBinding)
	{
		return Deduce(psBinding, pcClass->mpsType);
	}
	mpcUnit->GetContext()->EnterBindings(NULL);
	mpcUnit->BeginQuiet();
	pcNamed = mpcUnit->GetNameClass(pcName, pcName);
	mpcUnit->EndQuiet();
	mpcUnit->GetContext()->LeaveBindings();
	miScore += 2;
	return pcNamed == pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the type is a function's with the parameters of a pattern's
// function suffix: as many (a pack takes the rest), each matching, with
// the ellipsis, const and volatile the suffix has.  What it returns is
// the pattern's next.  A parameter is matched as it is written: one that
// is declared an array, or const, isn't adjusted first ([dcl.fct] 5).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchFunction(CCPPSyntaxFunctionSuffix* pcSuffix, SCPP65Type* psType)
{
	CArrayCPPBindings	asParameters;
	SCPPBinding*		psParameter;
	size				i;
	bool				bResult;

	if (!psType->IsFunction() || (pcSuffix->mbVariadic != psType->bVariadic) || (pcSuffix->IsConst() != psType->bConstFunction) || (pcSuffix->IsVolatile() != psType->bVolatileFunction))
	{
		return false;
	}
	asParameters.Init();
	for (i = 0; psType->papsParameters && (i < psType->papsParameters->NumElements()); i++)
	{
		psParameter = asParameters.Add();
		memset(psParameter, 0, sizeof(SCPPBinding));
		psParameter->psType = psType->papsParameters->GetPtr(i);
	}
	bResult = MatchArguments(&pcSuffix->macpParameters, &asParameters);
	asParameters.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether each pattern that was left until the specialisation's
// parameters were known (MatchType) is the type of its argument with them:
// Void<typename T::Type> is void for a T that has a Type, and no type at
// all, so no match, for one that hasn't.  [temp.class.spec.match] 2,
// [temp.deduct] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchDeferred(void)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxDeclarator*		pcDeclarator;
	SCPPDeferredPattern*		psDeferred;
	SCPP65Type*					psType;
	size						i;
	bool						bResult;

	bResult = true;
	mpcUnit->GetContext()->EnterBindings(mpcDeduced);
	mpcUnit->BeginQuiet();
	for (i = 0; bResult && (i < masDeferred.NumElements()); i++)
	{
		psDeferred = masDeferred.Get(i);
		PatternParts(psDeferred->pcPattern, &pcSpecifiers, &pcDeclarator);
		psType = mpcUnit->GetDeclarators()->GetType(pcSpecifiers, pcDeclarator, psDeferred->pcPattern);
		bResult = psType && psType->Same(psDeferred->psArgument);
	}
	mpcUnit->EndQuiet();
	mpcUnit->GetContext()->LeaveBindings();
	return bResult;
}


CArrayCPP65Types* CCPPDeduction::GetTypes(void) { return &mapsDeduced; }
CArrayInt64* CCPPDeduction::GetValues(void) { return &malliDeduced; }
CCPPTemplateArguments* CCPPSpecialisationMatch::GetDeduced(void) const { return mpcDeduced; }
int CCPPSpecialisationMatch::GetScore(void) const { return miScore; }
