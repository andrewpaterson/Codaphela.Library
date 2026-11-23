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
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mpcTemplate = pcTemplate;
	mapsDeduced.Init();
	malliDeduced.Init();
	masPacks.Init();
	mbConflict = false;
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
	masPacks.Kill();
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
	SCPP65Type*					psType;
	ECPPPointer					eOuter;

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
	DeduceType(pcSymbol, WithoutQualifiersOf(pcType->mpcSpecifiers, psType));
}


//////////////////////////////////////////////////////////////////////////
//
// The template's type parameter of the symbol, if it has one that isn't
// deduced yet, is the type.  A template template parameter's is a class
// that is an instance of its template (CCPPUnit::BindArguments).
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::DeduceType(CCPPSymbol* pcSymbol, SCPP65Type* psType)
{
	CCPPSyntax*	pcParameter;
	size		j;

	for (j = 0; j < mpcTemplate->mpcDeclaration->macpParameters.NumElements(); j++)
	{
		pcParameter = mpcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
		if ((pcParameter->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol == pcSymbol))
		{
			if (!mapsDeduced.GetPtr(j))
			{
				mapsDeduced.SetPtr(j, psType);
			}
			else if (psType && IsPlainType(psType) && IsPlainType(mapsDeduced.GetPtr(j)) && !psType->Unqualified()->Same(mapsDeduced.GetPtr(j)->Unqualified()))
			{
				//One parameter deduced as two types: no deduction.  [temp.deduct.type] 2
				mbConflict = true;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The template's non-type parameter of the symbol, if it has one that
// isn't deduced yet, is the value.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::DeduceValue(CCPPSymbol* pcSymbol, int64 lliValue)
{
	CCPPSyntax*	pcParameter;
	size		j;

	for (j = 0; j < mpcTemplate->mpcDeclaration->macpParameters.NumElements(); j++)
	{
		pcParameter = mpcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
		if ((pcParameter->GetKind() == CPPS_Parameter) && (((CCPPSyntaxParameter*)pcParameter)->mpcSymbol == pcSymbol) && !mapsDeduced.GetPtr(j))
		{
			mapsDeduced.SetPtr(j, &mpcUnit->GetTypes()->msInt);
			malliDeduced.SetValue(j, lliValue);
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
	size					uiArgument;
	size					uiKnown;

	uiArgument = 0;
	for (i = 0; pcSuffix && (i < pcSuffix->macpParameters.NumElements()) && (uiArgument < papsArguments->NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		if (KnownPackSize(pcParameter, &uiKnown))
		{
			//A... a of the class's pack A, known already: as many arguments as A has, none deduced from.
			//[temp.deduct.call] 1
			uiArgument += uiKnown;
			continue;
		}
		if (!DeduceParameter(pcParameter, papsArguments, papcArguments, uiArgument))
		{
			return;
		}
		uiArgument++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a parameter is a pack expansion of a pack that isn't the
// template's own, but bound already (the pack of the class a member
// template is in), and how many it expands to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeduction::KnownPackSize(CCPPSyntaxParameter* pcParameter, size* puiSize)
{
	CCPPSyntaxTypeSpecifier*	pcType;
	CCPPSymbol*					pcSymbol;
	SCPPBinding*				psBinding;
	CCPPTemplateArguments*		pcOuter;
	size						j;

	if (!pcParameter->mpcDeclarator || !pcParameter->mpcDeclarator->mbPack || !pcParameter->mpcSpecifiers)
	{
		return false;
	}
	pcType = pcParameter->mpcSpecifiers->mpcType;
	if (!pcType || (pcType->GetKind() != CPPS_NamedTypeSpecifier))
	{
		return false;
	}
	pcSymbol = ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName->mpcSymbol;
	for (j = 0; pcSymbol && (j < mpcTemplate->mpcDeclaration->macpParameters.NumElements()); j++)
	{
		if ((mpcTemplate->mpcDeclaration->macpParameters.GetPtr(j)->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)mpcTemplate->mpcDeclaration->macpParameters.GetPtr(j))->mpcSymbol == pcSymbol))
		{
			return false;
		}
	}
	//A member template's class's pack is in its class's bindings, which deduction may be made outside of.
	pcOuter = (pcSymbol && mpcTemplate->mpcClass && mpcTemplate->mpcClass->mpcBindings) ? mpcUnit->GetInstantiator()->MemberTemplateClassBindings(mpcTemplate) : NULL;
	if (pcOuter)
	{
		mpcUnit->GetContext()->EnterBindings(pcOuter);
	}
	psBinding = pcSymbol ? mpcUnit->GetInstantiator()->FindBinding(pcSymbol) : NULL;
	if (pcOuter)
	{
		mpcUnit->GetContext()->LeaveBindings();
	}
	if (!psBinding || !psBinding->IsPack())
	{
		return false;
	}
	*puiSize = psBinding->PackSize();
	return true;
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

	if (pcParameter->mpcDeclarator && !pcParameter->mpcDeclarator->mbPack && !HasArray(pcParameter->mpcDeclarator) && NeedsPattern(pcParameter->mpcDeclarator) && papsArguments->GetPtr(i))
	{
		//R (*)(A) and M C::* are matched as the forms of a type, where they can be; what that doesn't deduce, the
		//rest below may.  [temp.deduct.type] 8
		DeduceByPattern(pcParameter, papsArguments->GetPtr(i));
	}
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
	if (pcName->GetLast() && pcName->GetLast()->mbTemplateArguments && !papsArguments->GetPtr(i) && papcArguments && (papcArguments->GetPtr(i)->GetKind() == CPPS_InitializerList))
	{
		DeduceFromList(pcName, papcArguments->GetPtr(i));
		return true;
	}
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
		else if (psArgument && IsPlainType(psArgument) && IsPlainType(mapsDeduced.GetPtr(j)) && !psArgument->Unqualified()->Same(mapsDeduced.GetPtr(j)->Unqualified()))
		{
			//F(T, T) of 1 and 2L: T can't be both.  [temp.deduct.type] 2
			mbConflict = true;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A number or an object: a type two arguments deduce one parameter as only
// if they are both it (an array or a function might be a pointer instead).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeduction::IsPlainType(SCPP65Type* psType)
{
	return psType->IsInteger() || psType->IsFloat() || psType->IsClass();
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

	pcSymbol = (pcSize && (iCount > 0)) ? GetArgumentSymbol(pcSize) : NULL;
	if (pcSymbol)
	{
		DeduceValue(pcSymbol, iCount);
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
	size						i;

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
			if (pcPointer->mpcClass && pcPointer->mpcClass->mpcSymbol)
			{
				DeduceType(pcPointer->mpcClass->mpcSymbol, psArgument->pcClass->mpsType);
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
// Name<M, J>, from the template arguments psArgument has for Name
// (ArgumentsFor): each that is just a parameter's name takes the argument
// in its place.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::DeduceFromInstance(CCPPSyntaxNameComponent* pcComponent, SCPP65Type* psArgument)
{
	CCPPTemplateArguments*	pcArguments;
	SCPPBinding*			psBinding;
	CCPPSymbol*				pcSymbol;
	size					i;

	pcArguments = (psArgument && pcComponent->mpcSymbol) ? ArgumentsFor(pcComponent->mpcSymbol, psArgument) : NULL;
	for (i = 0; pcArguments && (i < pcComponent->macpTemplateArguments.NumElements()) && (i < pcArguments->macBindings.NumElements()); i++)
	{
		if (DeducePack(pcComponent->macpTemplateArguments.GetPtr(i), pcArguments, i))
		{
			//The pack expansion takes the rest of the arguments.
			break;
		}
		pcSymbol = GetArgumentSymbol(pcComponent->macpTemplateArguments.GetPtr(i));
		psBinding = pcArguments->macBindings.Get(i);
		if (pcSymbol && psBinding->psType)
		{
			DeduceType(pcSymbol, psBinding->psType);
		}
		else if (pcSymbol)
		{
			DeduceValue(pcSymbol, psBinding->lliValue);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The template arguments a type has for the template a parameter names
// (Name of Name<M, J>), or NULL if it has none:
// - a class template's: those of the type's class if it is an instance of
//   it, or else of the first of its bases that is.  Only instances of the
//   primary template are looked in.
// - a template template parameter's (C of C<T>): those of the type's class,
//   whose class template C then is.  [temp.deduct.type] 8
// - an alias template's: those that make its type the type (int for
//   template<class T> using Ptr = T*; and int*).  [temp.alias] 2
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPPDeduction::ArgumentsFor(CCPPSymbol* pcTemplate, SCPP65Type* psType)
{
	CCPPClass*	pcInstance;

	if (pcTemplate->meKind == CPPSK_AliasTemplate)
	{
		return AliasArguments(pcTemplate, psType);
	}
	if (!psType->IsClass())
	{
		return NULL;
	}
	if (pcTemplate->meKind == CPPSK_TemplateTemplateParameter)
	{
		pcInstance = psType->pcClass->mpcTemplate ? psType->pcClass : NULL;
	}
	else
	{
		pcInstance = FindInstanceBase(psType->pcClass, pcTemplate);
	}
	if (!pcInstance || (pcInstance->mpcTemplate->mpcPrimary != pcInstance->mpcTemplate) || !pcInstance->mpcBindings)
	{
		return NULL;
	}
	if (pcTemplate->meKind == CPPSK_TemplateTemplateParameter)
	{
		DeduceType(pcTemplate, pcInstance->mpsType);
	}
	return pcInstance->mpcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// The arguments of an alias template that make its type psType, or NULL
// if none do.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPPDeduction::AliasArguments(CCPPSymbol* pcTemplate, SCPP65Type* psType)
{
	CCPPSyntaxAliasDeclaration*	pcAlias;
	CCPPPatternMatch			cMatch;
	CCPPTemplateArguments*		pcArguments;

	pcAlias = (CCPPSyntaxAliasDeclaration*)pcTemplate->mpcDeclaration;
	if (!pcAlias || (pcAlias->GetKind() != CPPS_AliasDeclaration) || !pcAlias->mpcParent || (pcAlias->mpcParent->GetKind() != CPPS_TemplateDeclaration))
	{
		return NULL;
	}
	cMatch.Init(mpcUnit, (CCPPSyntaxTemplateDeclaration*)pcAlias->mpcParent);
	pcArguments = cMatch.MatchTypeId(pcAlias->mpcType, psType) ? cMatch.GetDeduced() : NULL;
	cMatch.Kill();
	return pcArguments;
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
		mpcDiagnostics->BeginQuiet();
		mpcUnit->ResolveBases(pcClass, NULL);
		mpcDiagnostics->EndQuiet();
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
// Whether a parameter's declarator has a form DeduceParameter doesn't take
// apart itself: parentheses around one (a pointer to a function or to an
// array, R (*)(A)) or a pointer to a member (M C::*).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeduction::NeedsPattern(CCPPSyntaxDeclarator* pcDeclarator)
{
	size	i;

	if (pcDeclarator->mpcNested && ((pcDeclarator->mpcNested->NumPointers() > 0) || pcDeclarator->mpcNested->mpcNested))
	{
		return true;
	}
	for (i = 0; i < pcDeclarator->NumPointers(); i++)
	{
		if (pcDeclarator->GetPointer(i)->meKind == CPPP_MemberPointer)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Deduces the template parameters a parameter's type is made of from its
// argument's type (CCPPPatternMatch), the argument a function's pointer if
// it is a function and the parameter isn't a reference.  [temp.deduct.call]
// 2, [temp.deduct.type] 8
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::DeduceByPattern(CCPPSyntaxParameter* pcParameter, SCPP65Type* psArgument)
{
	CCPPPatternMatch		cMatch;
	CCPPTemplateArguments*	pcDeduced;
	SCPPBinding*			psBinding;
	size					i;

	psArgument = psArgument->WithoutReference();
	if (psArgument->IsFunction())
	{
		psArgument = mpcUnit->GetTypes()->Pointer(psArgument);
	}
	cMatch.Init(mpcUnit, mpcTemplate->mpcDeclaration);
	if (cMatch.MatchParameter(pcParameter, psArgument))
	{
		pcDeduced = cMatch.GetDeduced();
		for (i = 0; i < pcDeduced->macBindings.NumElements(); i++)
		{
			psBinding = pcDeduced->macBindings.Get(i);
			if (psBinding->psType && psBinding->pcParameter)
			{
				DeduceType(psBinding->pcParameter, psBinding->psType);
			}
		}
	}
	cMatch.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter std::initializer_list<T> (or a reference to one) and a
// braced list for it: T is its elements' type.  [temp.deduct.call] 1
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeduction::DeduceFromList(CCPPSyntaxName* pcName, CCPPSyntax* pcList)
{
	CCPPTemplate*	pcInitializerList;
	CArraySyntax*	papcElements;
	CCPPSymbol*		pcSymbol;
	SCPP65Type*		psElement;
	size			i;

	pcInitializerList = mpcUnit->InitializerListTemplate();
	if (!pcInitializerList || (pcName->mpcSymbol != pcInitializerList->mpcSymbol) || (pcName->GetLast()->macpTemplateArguments.NumElements() != 1))
	{
		return;
	}
	pcSymbol = GetArgumentSymbol(pcName->GetLast()->macpTemplateArguments.GetPtr(0));
	papcElements = mpcUnit->ListElements(pcList);
	for (i = 0; pcSymbol && (i < papcElements->NumElements()); i++)
	{
		psElement = mpcUnit->TypeOf(papcElements->GetPtr(i));
		if (psElement)
		{
			DeduceType(pcSymbol, psElement->WithoutReference()->Unqualified());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A template argument that is a pack expansion of one of the template's
// parameter packs alone (T... or I...) deduces the pack from the instance's
// arguments from uiFirst on: each one's type or value, and a pack's
// elements.  True if the argument is one.  [temp.deduct.type] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeduction::DeducePack(CCPPSyntax* pcArgument, CCPPTemplateArguments* pcArguments, size uiFirst)
{
	CCPPSyntax*			pcPattern;
	CCPPSymbol*			pcSymbol;
	CCPPSyntax*			pcParameter;
	SCPPDeducedPack*	psPack;
	SCPPBinding*		psBinding;
	size				j;
	size				k;
	size				uiParameter;
	bool				bType;

	if ((pcArgument->GetKind() == CPPS_TypeId) && ((CCPPSyntaxTypeId*)pcArgument)->mbPack)
	{
		pcPattern = pcArgument;
	}
	else if (pcArgument->GetKind() == CPPS_PackExpansionExpression)
	{
		pcPattern = ((CCPPSyntaxPackExpansionExpression*)pcArgument)->mpcPattern;
	}
	else
	{
		return false;
	}
	pcSymbol = GetArgumentSymbol(pcPattern);
	uiParameter = mpcTemplate->mpcDeclaration->macpParameters.NumElements();
	bType = false;
	for (j = 0; pcSymbol && (j < mpcTemplate->mpcDeclaration->macpParameters.NumElements()); j++)
	{
		pcParameter = mpcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
		if ((pcParameter->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol == pcSymbol) && ((CCPPSyntaxTypeParameter*)pcParameter)->mbPack)
		{
			uiParameter = j;
			bType = true;
		}
		else if ((pcParameter->GetKind() == CPPS_Parameter) && (((CCPPSyntaxParameter*)pcParameter)->mpcSymbol == pcSymbol) && ((CCPPSyntaxParameter*)pcParameter)->mpcDeclarator && ((CCPPSyntaxParameter*)pcParameter)->mpcDeclarator->mbPack)
		{
			uiParameter = j;
		}
	}
	if (uiParameter == mpcTemplate->mpcDeclaration->macpParameters.NumElements())
	{
		return true;
	}
	for (j = 0; j < masPacks.NumElements(); j++)
	{
		if (masPacks.Get(j)->uiParameter == uiParameter)
		{
			return true;
		}
	}
	psPack = masPacks.Add();
	psPack->uiParameter = uiParameter;
	psPack->papsTypes = bType ? mpcUnit->NewPack() : NULL;
	psPack->palliValues = bType ? NULL : mpcUnit->NewValuePack();
	for (j = uiFirst; j < pcArguments->macBindings.NumElements(); j++)
	{
		psBinding = pcArguments->macBindings.Get(j);
		if (psBinding->papsPack || psBinding->palliPack)
		{
			for (k = 0; k < psBinding->PackSize(); k++)
			{
				if (bType && psBinding->papsPack)
				{
					psPack->papsTypes->Add(psBinding->papsPack->GetPtr(k));
				}
				else if (!bType && psBinding->palliPack)
				{
					psPack->palliValues->Add(psBinding->palliPack->GetValue(k));
				}
			}
		}
		else if (bType && psBinding->psType)
		{
			psPack->papsTypes->Add(psBinding->psType);
		}
		else if (!bType)
		{
			psPack->palliValues->Add(psBinding->lliValue);
		}
	}
	return true;
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
// None of the declaration's template parameters is deduced yet.  What has none (template <>, an explicit
// specialisation) is more special than any that has.
//
//////////////////////////////////////////////////////////////////////////
void CCPPPatternMatch::Init(CCPPUnit* pcUnit, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	SCPPBinding*	psBinding;
	size			i;

	mpcUnit = pcUnit;
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mpcDeduced = pcUnit->GetInstantiator()->NewBindings();
	masDeferred.Init();
	for (i = 0; i < pcDeclaration->macpParameters.NumElements(); i++)
	{
		psBinding = mpcDeduced->macBindings.Add();
		memset(psBinding, 0, sizeof(SCPPBinding));
		psBinding->pcParameter = pcDeclaration->ParameterSymbol(i);
	}
	miScore = (pcDeclaration->macpParameters.NumElements() == 0) ? 1000 : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPPatternMatch::Kill(void)
{
	masDeferred.Kill();
	mpcDeduced = NULL;
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a specialisation's template arguments (papcPattern, X<T*, int>'s)
// match pcArguments, an instance's, with every one of its type parameters
// deduced (IsDeduced).  [temp.class.spec.match]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPatternMatch::Match(CArraySyntax* papcPattern, CCPPTemplateArguments* pcArguments)
{
	return MatchPattern(papcPattern, pcArguments) && IsDeduced();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a type written with the parameters (an alias template's, T* of
// template<class T> using Ptr = T*;) is psType for some arguments.  A
// parameter the type doesn't show (T of template<class T, class U> using
// Second = V<U>;) is left without one.  [temp.alias] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPatternMatch::MatchTypeId(CCPPSyntaxTypeId* pcPattern, SCPP65Type* psType)
{
	return MatchType(pcPattern, psType) && MatchDeferred();
}


//////////////////////////////////////////////////////////////////////////
//
// The same for a function template's parameter (its specifiers and
// declarator) and its argument's type.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPatternMatch::MatchParameter(CCPPSyntaxParameter* pcParameter, SCPP65Type* psType)
{
	return MatchType(pcParameter, psType) && MatchDeferred();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether every type parameter was deduced, and the types the parameters
// are only used in (left until they were known: MatchDeferred) are their
// arguments'.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPatternMatch::IsDeduced(void)
{
	CCPPSymbol*		pcSymbol;
	SCPPBinding*	psBinding;
	size			i;

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
bool CCPPPatternMatch::MatchPattern(CArraySyntax* papcPattern, CCPPTemplateArguments* pcArguments)
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
// each.  The parameters are deduced as they are met; a
// last pattern that expands one of its parameter packs (T...) takes the
// rest of the arguments (MatchPack).  The score counts how much of the
// arguments' structure the patterns name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPatternMatch::MatchArguments(CArraySyntax* papcPattern, CArrayCPPBindings* pasArguments)
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
		if (!mpcUnit->GetFolder()->Constant(pcPattern, &lliValue) || (lliValue != psArgument->lliValue))
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
void CCPPPatternMatch::FlattenArguments(CCPPTemplateArguments* pcArguments, CArrayCPPBindings* pasArguments)
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
bool CCPPPatternMatch::PatternParts(CCPPSyntax* pcPattern, CCPPSyntaxDeclSpecifiers** ppcSpecifiers, CCPPSyntaxDeclarator** ppcDeclarator)
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
bool CCPPPatternMatch::IsPackExpansion(CCPPSyntax* pcPattern)
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
// The binding of the parameter pack a pack expansion
// expands, if that is all it is (T..., not T*...), or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPPPatternMatch::ExpandedParameter(CCPPSyntax* pcPattern)
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
CCPPSyntaxNamedTypeSpecifier* CCPPPatternMatch::NamedType(CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	return (pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_NamedTypeSpecifier)) ? (CCPPSyntaxNamedTypeSpecifier*)pcSpecifiers->mpcType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The parameter pack psPack takes the arguments from uiFirst on: all
// types for a type pack, all values for a non-type pack.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPatternMatch::MatchPack(SCPPBinding* psPack, CArrayCPPBindings* pasArguments, size uiFirst)
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
bool CCPPPatternMatch::MatchBound(CCPPSyntax* pcSize, int iCount)
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
	return mpcUnit->GetFolder()->Constant(pcSize, &lliValue) && (lliValue == iCount);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a pattern for a type matches a type argument: T, const T, a
// class template's instance X<T>, and what a declarator makes of one (T*,
// T&, T[N], T[], R(A...), R (*)(A), M C::*, R (C::*)(A...) const), each of
// the parameters taking what stands where it is written.
// A type that names none of them where one can be deduced from (int,
// typename T::Type, an alias template's Void<T>) is the argument once they
// are known (MatchDeferred).  [temp.deduct.type] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPatternMatch::MatchType(CCPPSyntax* pcPattern, SCPP65Type* psArgument)
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
	pcTemplate = (pcComponent && pcComponent->mbTemplateArguments && pcComponent->mpcSymbol && (pcComponent->mpcSymbol->meKind == CPPSK_ClassTemplate)) ? mpcUnit->GetInstantiator()->GetTemplate(pcComponent->mpcSymbol) : NULL;
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
// One of the parameters takes the type, which every
// place it is written must agree on (X<T, T> isn't X<int, char>).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPatternMatch::Deduce(SCPPBinding* psBinding, SCPP65Type* psType)
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
bool CCPPPatternMatch::MatchDeclarator(CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type** ppsType)
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
bool CCPPPatternMatch::MatchPointer(CCPPSyntaxPointerOperator* pcOperator, SCPP65Type* psType)
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
// pcClass: one of the parameters takes it, a class's name
// must be its.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPatternMatch::MatchClass(CCPPSyntaxName* pcName, CCPPClass* pcClass)
{
	SCPPBinding*	psBinding;
	CCPPClass*		pcNamed;

	psBinding = mpcDeduced->FindOwn(pcName->mpcSymbol);
	if (psBinding)
	{
		return Deduce(psBinding, pcClass->mpsType);
	}
	mpcUnit->GetContext()->EnterBindings(NULL);
	mpcDiagnostics->BeginQuiet();
	pcNamed = mpcUnit->GetNameClass(pcName, pcName);
	mpcDiagnostics->EndQuiet();
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
bool CCPPPatternMatch::MatchFunction(CCPPSyntaxFunctionSuffix* pcSuffix, SCPP65Type* psType)
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
// Whether each pattern that was left until the parameters were known
// (MatchType) is the type of its argument with them:
// Void<typename T::Type> is void for a T that has a Type, and no type at
// all, so no match, for one that hasn't.  [temp.class.spec.match] 2,
// [temp.deduct] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPatternMatch::MatchDeferred(void)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxDeclarator*		pcDeclarator;
	SCPPDeferredPattern*		psDeferred;
	SCPP65Type*					psType;
	size						i;
	bool						bResult;

	bResult = true;
	mpcUnit->GetContext()->EnterBindings(mpcDeduced);
	mpcDiagnostics->BeginQuiet();
	for (i = 0; bResult && (i < masDeferred.NumElements()); i++)
	{
		psDeferred = masDeferred.Get(i);
		PatternParts(psDeferred->pcPattern, &pcSpecifiers, &pcDeclarator);
		psType = mpcUnit->GetDeclarators()->GetType(pcSpecifiers, pcDeclarator, psDeferred->pcPattern);
		bResult = psType && psType->Same(psDeferred->psArgument);
	}
	mpcDiagnostics->EndQuiet();
	mpcUnit->GetContext()->LeaveBindings();
	return bResult;
}


CArrayCPP65Types* CCPPDeduction::GetTypes(void) { return &mapsDeduced; }
CArrayInt64* CCPPDeduction::GetValues(void) { return &malliDeduced; }
CArrayCPPDeducedPacks* CCPPDeduction::GetPacks(void) { return &masPacks; }
bool CCPPDeduction::HasConflict(void) const { return mbConflict; }
CCPPTemplateArguments* CCPPPatternMatch::GetDeduced(void) const { return mpcDeduced; }
int CCPPPatternMatch::GetScore(void) const { return miScore; }
