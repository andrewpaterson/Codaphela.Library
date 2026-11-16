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

	//An array's type here is already the pointer it decays to, but its place is the array.
	psPlace = mpcUnit->PlaceOf(pcArgument);
	if (psPlace && psPlace->WithoutReference()->IsArray())
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
	for (i = 0; i < pcSpecialisation->mpcDeclaration->macpParameters.NumElements(); i++)
	{
		pcParameter = pcSpecialisation->mpcDeclaration->macpParameters.GetPtr(i);
		psBinding = mpcDeduced->macBindings.Add();
		psBinding->pcParameter = (pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol;
		psBinding->psType = NULL;
		psBinding->lliValue = 0;
		psBinding->papsPack = NULL;
		psBinding->palliPack = NULL;
		psBinding->bSelf = false;
		psBinding->pcArgument = NULL;
		psBinding->pcTemplate = NULL;
	}
	miScore = (pcSpecialisation->mpcDeclaration->macpParameters.NumElements() == 0) ? 1000 : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSpecialisationMatch::Kill(void)
{
	mpcDeduced = NULL;
	mpcSpecialisation = NULL;
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the specialisation's pattern matches pcArguments, with every one
// of its type parameters deduced.
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a specialisation's template arguments (papcPattern) match an
// instance's arguments, each of a pack's elements an argument of its own
// (FlattenArguments).  The specialisation's parameters are deduced as they
// are met; a last pattern that expands one of its parameter packs (T...)
// takes the rest of the arguments (MatchPack).  The score counts how much
// of the arguments' structure the pattern names.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchPattern(CArraySyntax* papcPattern, CCPPTemplateArguments* pcArguments)
{
	CArrayCPPBindings	asArguments;
	CCPPSyntax*			pcPattern;
	SCPPBinding*		psArgument;
	SCPPBinding*		psBinding;
	SCPPBinding*		psPack;
	CCPPSymbol*			pcSymbol;
	size				i;
	size				k;
	int64				lliValue;
	bool				bResult;

	asArguments.Init();
	FlattenArguments(pcArguments, &asArguments);
	bResult = true;
	for (i = 0, k = 0; bResult && (i < papcPattern->NumElements()); i++)
	{
		pcPattern = papcPattern->GetPtr(i);
		if (IsPackExpansion(pcPattern))
		{
			psPack = ExpandedParameter(pcPattern);
			bResult = psPack && (i + 1 == papcPattern->NumElements()) && MatchPack(psPack, &asArguments, k);
			k = asArguments.NumElements();
			continue;
		}
		if (k >= asArguments.NumElements())
		{
			bResult = false;
			break;
		}
		psArgument = asArguments.Get(k++);
		if (psArgument->psType)
		{
			bResult = (pcPattern->GetKind() == CPPS_TypeId) && MatchType((CCPPSyntaxTypeId*)pcPattern, psArgument);
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
		bResult = mpcUnit->Constant(pcPattern, &lliValue) && (lliValue == psArgument->lliValue);
		miScore++;
	}
	bResult = bResult && (k == asArguments.NumElements());
	asArguments.Kill();
	return bResult;
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
// Whether a pattern is a pack expansion: T... or N....
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::IsPackExpansion(CCPPSyntax* pcPattern)
{
	return ((pcPattern->GetKind() == CPPS_TypeId) && ((CCPPSyntaxTypeId*)pcPattern)->mbPack) || (pcPattern->GetKind() == CPPS_PackExpansionExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// The binding of the specialisation's parameter pack a pack expansion
// expands, if that is all it is (T..., not T*...), or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPPSpecialisationMatch::ExpandedParameter(CCPPSyntax* pcPattern)
{
	CCPPSyntaxTypeId*		pcTypeId;
	CCPPSyntaxTypeSpecifier*	pcType;
	CCPPSyntax*				pcExpression;

	if (pcPattern->GetKind() == CPPS_TypeId)
	{
		pcTypeId = (CCPPSyntaxTypeId*)pcPattern;
		pcType = pcTypeId->mpcSpecifiers ? pcTypeId->mpcSpecifiers->mpcType : NULL;
		if (!pcType || (pcType->GetKind() != CPPS_NamedTypeSpecifier) || (pcTypeId->mpcDeclarator && !pcTypeId->mpcDeclarator->IsEmpty()) || (pcTypeId->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)))
		{
			return NULL;
		}
		return mpcDeduced->FindOwn(((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName->mpcSymbol);
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
// Whether a type pattern (T, T*, const T, T&, T[N], T[], a class template instance
// X<T>, or a type naming none of the specialisation's parameters) matches a type
// argument.  Array bounds, pointers and references are matched outside in, then
// const at the top level, then the type named.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSpecialisationMatch::MatchType(CCPPSyntaxTypeId* pcPattern, SCPPBinding* psArgument)
{
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxPointerOperator*		pcOperator;
	CCPPSyntax*						pcSuffix;
	CCPPSyntaxNamedTypeSpecifier*	pcNamed;
	CCPPSyntaxNameComponent*		pcComponent;
	SCPPBinding*					psBinding;
	CCPPTemplate*					pcTemplate;
	SCPP65Type*						psType;
	SCPP65Type*						psNamed;
	size							i;
	bool							bResult;

	pcSpecifiers = pcPattern->mpcSpecifiers;
	pcDeclarator = pcPattern->mpcDeclarator;
	if (!pcSpecifiers || (pcDeclarator && pcDeclarator->mpcNested))
	{
		return false;
	}

	//T[N] matches an array, N taking its bound, and T[] an array of unknown bound.  Outermost first.
	psType = psArgument->psType;
	for (i = 0; pcDeclarator && (i < pcDeclarator->macpSuffixes.NumElements()); i++)
	{
		pcSuffix = pcDeclarator->macpSuffixes.GetPtr(i);
		if ((pcSuffix->GetKind() != CPPS_ArraySuffix) || !psType->IsArray() || !MatchBound(((CCPPSyntaxArraySuffix*)pcSuffix)->mpcSize, psType->iCount))
		{
			return false;
		}
		psType = psType->psBase;
		miScore++;
	}
	for (i = pcDeclarator ? pcDeclarator->NumPointers() : 0; i > 0; i--)
	{
		pcOperator = pcDeclarator->GetPointer(i - 1);
		if ((pcOperator->meKind == CPPP_Pointer) && psType->IsPointer())
		{
			psType = psType->psBase;
		}
		else if (((pcOperator->meKind == CPPP_Reference) && psType->IsLvalueReference()) || ((pcOperator->meKind == CPPP_RValueReference) && psType->IsRvalueReference()))
		{
			psType = psType->psBase;
		}
		else
		{
			return false;
		}
		miScore++;
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
	pcNamed = (pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_NamedTypeSpecifier)) ? (CCPPSyntaxNamedTypeSpecifier*)pcSpecifiers->mpcType : NULL;
	psBinding = pcNamed ? mpcDeduced->FindOwn(pcNamed->mpcName->mpcSymbol) : NULL;
	if (psBinding)
	{
		if (psBinding->psType)
		{
			return psBinding->psType->Same(psType);
		}
		psBinding->psType = psType;
		return true;
	}

	//X<T> matches an instance of X with matching arguments.
	pcComponent = pcNamed ? pcNamed->mpcName->GetLast() : NULL;
	if (pcComponent && pcComponent->mbTemplateArguments)
	{
		pcTemplate = pcComponent->mpcSymbol ? mpcUnit->GetTemplate(pcComponent->mpcSymbol) : mpcUnit->GetComponentTemplate(pcComponent);
		if (!pcTemplate || !psType->IsClass() || !psType->pcClass->mpcTemplate || (psType->pcClass->mpcTemplate->mpcPrimary != pcTemplate))
		{
			return false;
		}
		miScore++;
		return MatchPattern(&pcComponent->macpTemplateArguments, psType->pcClass->mpcBindings);
	}

	//A type naming none of the parameters must be the same type.
	mpcUnit->GetContext()->EnterBindings(NULL);
	mpcUnit->BeginQuiet();
	psNamed = mpcUnit->GetDeclarators()->GetTypeIdType(pcPattern);
	mpcUnit->EndQuiet();
	mpcUnit->GetContext()->LeaveBindings();
	bResult = psNamed && psNamed->Same(psArgument->psType);
	miScore += 2;
	return bResult;
}

