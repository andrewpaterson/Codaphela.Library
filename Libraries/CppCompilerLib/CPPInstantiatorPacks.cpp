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
#include "CPPSyntaxCopier.h"
#include "CPPUnit.h"
#include "CPPInstantiator.h"


//Finds the parameter packs a pack expansion's pattern names: the packs of the bindings in use whose parameters it
//names, and the function parameter packs it names, of those the function being generated can expand
//(CCPPInstantiator::FindFunctionPack).  Not in a pack expansion written inside the pattern, or in sizeof...: what is named
//there is expanded there, as a whole (Pair<Count<Ts...>, Ts>... expands the second Ts).  [temp.variadic] 5
class CCPPPackFinder : public CCPPSyntaxChildVisitor
{
public:
	CCPPInstantiator*		mpcInstantiator;
	CArrayCPPBindingPtrs*	mpapsPacks;
	CArrayCPPSymbols*		mpapcFunctionPacks;

	void	Visit(CCPPSyntax* pcChild) override
	{
		if (!IsExpansion(pcChild))
		{
			pcChild->VisitFields(this);
		}
	}

	static bool	IsExpansion(CCPPSyntax* pcNode)
	{
		switch (pcNode->GetKind())
		{
		case CPPS_PackExpansionExpression:
			return true;
		case CPPS_TypeId:
			return ((CCPPSyntaxTypeId*)pcNode)->mbPack;
		case CPPS_SizeofExpression:
			return ((CCPPSyntaxSizeofExpression*)pcNode)->mbPack;
		default:
			return false;
		}
	}

	void	SymbolField(CCPPSymbol** ppcSymbol) override
	{
		SCPPBinding*	psBinding;

		if (*ppcSymbol && mpcInstantiator->FindFunctionPack(*ppcSymbol))
		{
			if (mpapcFunctionPacks && (mpapcFunctionPacks->Find(*ppcSymbol) == ARRAY_ELEMENT_NOT_FOUND))
			{
				mpapcFunctionPacks->Add(*ppcSymbol);
			}
			return;
		}
		psBinding = *ppcSymbol ? mpcInstantiator->FindBinding(*ppcSymbol) : NULL;
		if (psBinding && psBinding->IsPack() && (mpapsPacks->Find(psBinding) == ARRAY_ELEMENT_NOT_FOUND))
		{
			mpapsPacks->Add(psBinding);
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A new empty list of a pack's types, kept until the unit is killed.
//
//////////////////////////////////////////////////////////////////////////
CArrayCPP65Types* CCPPInstantiator::NewPack(void)
{
	CArrayCPP65Types*	papsPack;

	papsPack = NewMalloc<CArrayCPP65Types>();
	papsPack->Init();
	mapapsPacks.Add(papsPack);
	return papsPack;
}


//////////////////////////////////////////////////////////////////////////
//
// The binding of the template parameter pack a function parameter pack (Args ...
// args) is declared with, or NULL if the parameter is not a pack.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPPInstantiator::PackBinding(CCPPSyntaxParameter* pcParameter)
{
	CCPPSyntaxTypeSpecifier*	pcType;
	SCPPBinding*				psBinding;

	if (!pcParameter->mpcDeclarator || !pcParameter->mpcDeclarator->mbPack || !pcParameter->mpcSpecifiers)
	{
		return NULL;
	}
	pcType = pcParameter->mpcSpecifiers->mpcType;
	if (!pcType || (pcType->GetKind() != CPPS_NamedTypeSpecifier))
	{
		return NULL;
	}
	psBinding = FindBinding(((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName->mpcSymbol);
	return (psBinding && psBinding->papsPack) ? psBinding : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol for element k of a function's parameter pack, which is one of
// the elements of the function being generated from here: a variable with
// the pack's name, kept until the unit is killed.  It is the same one
// each time the function's parameters are made (for what it returns, to
// check it, to write it), as its argument lists are expanded once
// (ExpandArguments) and name it.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPInstantiator::FunctionPackElement(CCPPFunction* pcFunction, CCPPSymbol* pcPack, size k)
{
	SCPPPackElement*	psElement;
	CCPPSymbol*			pcSymbol;
	size				i;

	for (i = 0; i < masPackElements.NumElements(); i++)
	{
		psElement = masPackElements.Get(i);
		if ((psElement->pcFunction == pcFunction) && (psElement->pcPack == pcPack) && (psElement->uiIndex == k))
		{
			mpcUnit->GetFunctionPacks()->Add(pcPack)->apcElements.Add(psElement->pcElement);
			return psElement->pcElement;
		}
	}

	pcSymbol = NewMalloc<CCPPSymbol>();
	pcSymbol->Init(pcPack->mlliName, CPPSK_Variable, pcPack->mpcDeclaration, pcPack->mpcScope);
	mapcPackSymbols.Add(pcSymbol);
	mpcUnit->GetFunctionPacks()->Add(pcPack)->apcElements.Add(pcSymbol);

	psElement = masPackElements.Add();
	psElement->pcBindings = NULL;
	psElement->pcFunction = pcFunction;
	psElement->pcPack = pcPack;
	psElement->uiIndex = k;
	psElement->pcElement = pcSymbol;
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call's arguments expand a pack (f(args...)).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::HasPackExpansion(CArraySyntax* papcArguments)
{
	size	i;

	for (i = 0; i < papcArguments->NumElements(); i++)
	{
		if (ExpansionPattern(papcArguments->GetPtr(i)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// What a pack expansion in a list expands: the pattern of a...,  or a
// mem-initialiser T()... itself.  NULL if pcNode isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPInstantiator::ExpansionPattern(CCPPSyntax* pcNode)
{
	if (pcNode->GetKind() == CPPS_PackExpansionExpression)
	{
		return ((CCPPSyntaxPackExpansionExpression*)pcNode)->mpcPattern;
	}
	if ((pcNode->GetKind() == CPPS_MemberInitializer) && ((CCPPSyntaxMemberInitializer*)pcNode)->mbPack)
	{
		return pcNode;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A pack of values for a non-type parameter pack, kept until the unit
// is killed.
//
//////////////////////////////////////////////////////////////////////////
CArrayInt64* CCPPInstantiator::NewValuePack(void)
{
	CArrayInt64*	palliPack;

	palliPack = NewMalloc<CArrayInt64>();
	palliPack->Init();
	mapalliPacks.Add(palliPack);
	return palliPack;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the template parameter packs pcPattern names (CCPPPackFinder), and
// to papcFunctionPacks, if it is given, the function parameter packs it
// names.
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::FindPacks(CCPPSyntax* pcPattern, CArrayCPPBindingPtrs* papsPacks, CArrayCPPSymbols* papcFunctionPacks)
{
	CCPPPackFinder	cFinder;

	cFinder.mpcInstantiator = this;
	cFinder.mpapsPacks = papsPacks;
	cFinder.mpapcFunctionPacks = papcFunctionPacks;
	pcPattern->VisitFields(&cFinder);
}


//////////////////////////////////////////////////////////////////////////
//
// How many elements a pattern's packs have (FindPacks found them): the
// same for each, and one pack at least.  An error at pcNode, and false, if
// not.  [temp.variadic] 5, 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::PackExpansionLength(CArrayCPPBindingPtrs* papsPacks, CArrayCPPSymbols* papcFunctionPacks, CCPPSyntax* pcNode, size* puiElements)
{
	size	i;
	bool	bFound;

	bFound = false;
	*puiElements = 0;
	for (i = 0; papcFunctionPacks && (i < papcFunctionPacks->NumElements()); i++)
	{
		if (bFound && (FindFunctionPack(papcFunctionPacks->GetPtr(i))->apcElements.NumElements() != *puiElements))
		{
			return mpcUnit->GetDiagnostics()->Error(pcNode, "The packs the pack expansion expands have different lengths");
		}
		*puiElements = FindFunctionPack(papcFunctionPacks->GetPtr(i))->apcElements.NumElements();
		bFound = true;
	}
	for (i = 0; i < papsPacks->NumElements(); i++)
	{
		if (bFound && (papsPacks->GetPtr(i)->PackSize() != *puiElements))
		{
			return mpcUnit->GetDiagnostics()->Error(pcNode, "The packs the pack expansion expands have different lengths");
		}
		*puiElements = papsPacks->GetPtr(i)->PackSize();
		bFound = true;
	}
	return bFound || mpcUnit->GetDiagnostics()->Error(pcNode, "The pack expansion names no parameter pack");
}


//////////////////////////////////////////////////////////////////////////
//
// The function parameter pack of the symbol, with its elements, if the
// function being generated can expand it: its own, or in a lambda's body
// one of a function the lambda is in.  NULL if the symbol isn't one.
//
//////////////////////////////////////////////////////////////////////////
SCPPFunctionPack* CCPPInstantiator::FindFunctionPack(CCPPSymbol* pcSymbol)
{
	return mpcUnit->GetFunctionPacks()->Find(pcSymbol);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a copy of pcPattern, pcParent's child, for each element of the
// packs it names (FindPacks): each copy names the element where the
// pattern names the pack.  A function parameter pack's elements are its
// variables (FunctionPackElement), a template parameter pack's are parameters
// of their own bound to its types or values (PackElement).  An error if
// the pattern names no pack or packs of different lengths (function
// parameter packs among them: a lambda's own and one it captured).
// [temp.variadic] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::ExpandPattern(CCPPSyntax* pcPattern, CCPPSyntax* pcParent, CArraySyntax* papcExpanded)
{
	CArrayCPPBindingPtrs	apsFound;
	CArrayCPPBindings		asPacks;
	CArrayCPPSymbols		apcFunctionPacks;
	CCPPSyntaxCopier		cCopier;
	SCPPFunctionPack*		psFunctionPack;
	CCPPSymbol*				pcElement;
	size					uiElements;
	size					i;
	size					k;
	bool					bResult;

	//The packs as they are: the elements' bindings are added to the bindings in use, which may have the packs.
	apsFound.Init();
	asPacks.Init();
	apcFunctionPacks.Init();
	FindPacks(pcPattern, &apsFound, &apcFunctionPacks);
	bResult = PackExpansionLength(&apsFound, &apcFunctionPacks, pcPattern, &uiElements);
	for (i = 0; i < apsFound.NumElements(); i++)
	{
		*asPacks.Add() = *apsFound.GetPtr(i);
	}
	apsFound.Kill();

	for (k = 0; bResult && (k < uiElements); k++)
	{
		cCopier.Init(mpcUnit->GetTree()->GetMemory());
		for (i = 0; i < apcFunctionPacks.NumElements(); i++)
		{
			psFunctionPack = FindFunctionPack(apcFunctionPacks.GetPtr(i));
			cCopier.Substitute(psFunctionPack->pcPack, psFunctionPack->apcElements.GetPtr(k));
		}
		for (i = 0; bResult && (i < asPacks.NumElements()); i++)
		{
			pcElement = PackElement(asPacks.Get(i), k);
			bResult = (pcElement != NULL) || mpcUnit->GetDiagnostics()->Error(pcPattern, "The pack expansion is not where the compiler can expand it yet");
			cCopier.Substitute(asPacks.Get(i)->pcParameter, pcElement);
		}
		if (bResult)
		{
			mpcUnit->GetStatistics()->Count(CPPCC_PackExpansionNodes, 3);
			papcExpanded->Add(cCopier.Copy(pcPattern, pcParent));
		}
		cCopier.Kill();
	}
	apcFunctionPacks.Kill();
	asPacks.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter of its own for element k of a template parameter pack, bound
// in the bindings in use to the element's type or value, made the first
// time it is wanted.  NULL where no bindings are in use.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPInstantiator::PackElement(SCPPBinding* psPack, size k)
{
	CCPPTemplateArguments*	pcBindings;
	SCPPPackElement*		psElement;
	SCPPBinding*			psBinding;
	CCPPSymbol*				pcSymbol;
	size					i;

	pcBindings = mpcUnit->GetContext()->GetBindings();
	if (!pcBindings || !psPack->pcParameter)
	{
		return NULL;
	}
	for (i = 0; i < masPackElements.NumElements(); i++)
	{
		psElement = masPackElements.Get(i);
		if ((psElement->pcBindings == pcBindings) && !psElement->pcFunction && (psElement->pcPack == psPack->pcParameter) && (psElement->uiIndex == k))
		{
			return psElement->pcElement;
		}
	}

	pcSymbol = NewMalloc<CCPPSymbol>();
	pcSymbol->Init(psPack->pcParameter->mlliName, psPack->pcParameter->meKind, psPack->pcParameter->mpcDeclaration, psPack->pcParameter->mpcScope);
	mapcPackSymbols.Add(pcSymbol);
	psBinding = pcBindings->macBindings.Add();
	psBinding->pcParameter = pcSymbol;
	psBinding->psType = psPack->papsPack ? psPack->papsPack->GetPtr(k) : NULL;
	psBinding->lliValue = psPack->palliPack ? psPack->palliPack->GetValue(k) : 0;
	psBinding->papsPack = NULL;
	psBinding->palliPack = NULL;
	psBinding->bSelf = false;
	psBinding->pcArgument = NULL;
	psBinding->pcTemplate = NULL;
	psBinding->pcAlias = NULL;

	psElement = masPackElements.Add();
	psElement->pcBindings = pcBindings;
	psElement->pcFunction = NULL;
	psElement->pcPack = psPack->pcParameter;
	psElement->uiIndex = k;
	psElement->pcElement = pcSymbol;
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// An expanded template argument written as pcArgument: an element of a
// pack expansion (bExpanded) or the argument as it is, its type or value
// to be given.
//
//////////////////////////////////////////////////////////////////////////
static void ExpandedArgument(SCPPExpandedArgument* psExpanded, CCPPSyntax* pcArgument, bool bExpanded)
{
	psExpanded->pcArgument = pcArgument;
	psExpanded->psType = NULL;
	psExpanded->lliValue = 0;
	psExpanded->bExpanded = bExpanded;
}


//////////////////////////////////////////////////////////////////////////
//
// A template argument list with its pack expansions expanded: T... and
// T*... for a type parameter pack T are each of T's types (or pointers to
// them), N... and N + 1... for a non-type pack N each of N's values (plus
// one), each pattern worked out with its packs' parameters bound to one of
// their elements at a time.  Other arguments are as they are written.
// [temp.variadic] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::ExpandTemplateArguments(CArraySyntax* papcArguments, CArrayCPPExpandedArguments* pasExpanded)
{
	CArrayCPPBindingPtrs	apsPacks;
	SCPPExpandedArgument*	psExpanded;
	CCPPPackElements		cElements;
	CCPPSyntax*				pcArgument;
	CCPPSyntax*				pcPattern;
	size					uiElements;
	size					i;
	size					k;
	bool					bResult;

	bResult = true;
	apsPacks.Init();
	for (i = 0; bResult && (i < papcArguments->NumElements()); i++)
	{
		pcArgument = papcArguments->GetPtr(i);
		pcPattern = NULL;
		if ((pcArgument->GetKind() == CPPS_TypeId) && ((CCPPSyntaxTypeId*)pcArgument)->mbPack)
		{
			pcPattern = pcArgument;
		}
		else if (pcArgument->GetKind() == CPPS_PackExpansionExpression)
		{
			pcPattern = ((CCPPSyntaxPackExpansionExpression*)pcArgument)->mpcPattern;
		}
		if (!pcPattern)
		{
			ExpandedArgument(pasExpanded->Add(), pcArgument, false);
			continue;
		}

		apsPacks.ReInit();
		FindPacks(pcPattern, &apsPacks, NULL);
		bResult = PackExpansionLength(&apsPacks, NULL, pcArgument, &uiElements);
		cElements.Init(&apsPacks);
		for (k = 0; bResult && (k < uiElements); k++)
		{
			cElements.Enter(k);
			psExpanded = pasExpanded->Add();
			ExpandedArgument(psExpanded, pcArgument, true);
			if (pcPattern->GetKind() == CPPS_TypeId)
			{
				psExpanded->psType = mpcUnit->GetDeclarators()->GetTypeIdType((CCPPSyntaxTypeId*)pcPattern);
				bResult = (psExpanded->psType != NULL) || mpcUnit->GetDiagnostics()->Error(pcArgument, "The template argument is not a type the compiler supports");
			}
			else
			{
				bResult = mpcUnit->GetFolder()->Constant(pcPattern, &psExpanded->lliValue) || mpcUnit->GetFolder()->ConstantError(pcArgument, "The template argument must be a constant");
			}
		}
		cElements.Kill();
	}
	apsPacks.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an expanded template argument for a type parameter: an
// element's type, or the type a type-id names.  NULL if it is neither.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPInstantiator::ExpandedArgumentType(SCPPExpandedArgument* psArgument)
{
	if (psArgument->bExpanded)
	{
		return psArgument->psType;
	}
	return (psArgument->pcArgument->GetKind() == CPPS_TypeId) ? mpcUnit->GetDeclarators()->GetTypeIdType((CCPPSyntaxTypeId*)psArgument->pcArgument) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of an expanded template argument for a non-type parameter:
// an element's value, or the constant an expression is.  An error if it is
// neither.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::ExpandedArgumentValue(SCPPExpandedArgument* psArgument, int64* plliValue)
{
	if (psArgument->bExpanded)
	{
		*plliValue = psArgument->lliValue;
		return !psArgument->psType || mpcUnit->GetDiagnostics()->Error(psArgument->pcArgument, "The template argument must be a constant");
	}
	return mpcUnit->GetFolder()->Constant(psArgument->pcArgument, plliValue) || mpcUnit->GetFolder()->ConstantError(psArgument->pcArgument, "The template argument must be a constant");
}


//////////////////////////////////////////////////////////////////////////
//
// sizeof...(pcPack): how many elements the parameter pack has, a template
// parameter pack or a function parameter pack (the one being generated's
// elements, as a generic lambda's auto... args has no template pack; or
// else by its template pack).  [expr.sizeof] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::PackSize(CCPPSymbol* pcPack, int64* plliValue)
{
	SCPPBinding*		psBinding;
	SCPPFunctionPack*	psFunctionPack;

	psFunctionPack = pcPack ? FindFunctionPack(pcPack) : NULL;
	if (psFunctionPack)
	{
		*plliValue = (int64)psFunctionPack->apcElements.NumElements();
		return true;
	}
	psBinding = pcPack ? FindBinding(pcPack) : NULL;
	if (!psBinding && pcPack && pcPack->mpcDeclaration && (pcPack->mpcDeclaration->GetKind() == CPPS_Parameter))
	{
		psBinding = PackBinding((CCPPSyntaxParameter*)pcPack->mpcDeclaration);
	}
	if (!psBinding || !psBinding->IsPack())
	{
		return false;
	}
	*plliValue = (int64)psBinding->PackSize();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An argument list with its pack expansions expanded (ExpandPattern): f(a...),
// P(Forward<A>(a)...) and : Base<T...>(t...), and a constructor's
// mem-initialisers : T()....  The list itself if it has none.
// Each list is expanded once for the bindings and function in use, and
// kept, as the type checker and the IR front end must see the same nodes.
// NULL, with an error, if one can't be expanded.
//
//////////////////////////////////////////////////////////////////////////
CArraySyntax* CCPPInstantiator::ExpandArguments(CArraySyntax* papcArguments)
{
	SCPPExpandedArguments*	psExpanded;
	CArraySyntax*			papcExpanded;
	CCPPSyntax*				pcArgument;
	CCPPSyntax*				pcPattern;
	size					i;
	bool					bResult;

	if (!HasPackExpansion(papcArguments))
	{
		return papcArguments;
	}
	for (i = 0; i < masExpandedArguments.NumElements(); i++)
	{
		psExpanded = masExpandedArguments.Get(i);
		if ((psExpanded->papcArguments == papcArguments) && (psExpanded->pcBindings == mpcUnit->GetContext()->GetBindings()) && (psExpanded->pcIn == mpcUnit->GetContext()->GetFunction()))
		{
			return psExpanded->papcExpanded;
		}
	}

	mpcUnit->GetStatistics()->Count(CPPCC_PackExpansions);
	papcExpanded = NewMalloc<CArraySyntax>();
	papcExpanded->Init();
	bResult = true;
	for (i = 0; bResult && (i < papcArguments->NumElements()); i++)
	{
		pcArgument = papcArguments->GetPtr(i);
		pcPattern = ExpansionPattern(pcArgument);
		if (!pcPattern)
		{
			papcExpanded->Add(pcArgument);
		}
		else
		{
			bResult = ExpandPattern(pcPattern, pcArgument->mpcParent, papcExpanded);
		}
	}
	if (!bResult)
	{
		//Not kept: asked again where errors are said, it says why.
		papcExpanded->Kill();
		free(papcExpanded);
		return NULL;
	}
	psExpanded = masExpandedArguments.Add();
	psExpanded->papcArguments = papcArguments;
	psExpanded->pcBindings = mpcUnit->GetContext()->GetBindings();
	psExpanded->pcIn = mpcUnit->GetContext()->GetFunction();
	psExpanded->papcExpanded = papcExpanded;
	return papcExpanded;
}


//////////////////////////////////////////////////////////////////////////
//
// The elements of an initialiser list, in braces or parentheses, with
// their pack expansions expanded ({ a... }, (t...)), as ExpandArguments
// expands them.  If one can't be, after its error, the list's own.
//
//////////////////////////////////////////////////////////////////////////
CArraySyntax* CCPPInstantiator::ListElements(CCPPSyntax* pcList)
{
	CArraySyntax*	papcElements;
	CArraySyntax*	papcExpanded;

	papcElements = (pcList->GetKind() == CPPS_ParenthesisedInitializer) ? &((CCPPSyntaxParenthesisedInitializer*)pcList)->macpElements : &((CCPPSyntaxInitializerList*)pcList)->macpElements;
	papcExpanded = ExpandArguments(papcElements);
	return papcExpanded ? papcExpanded : papcElements;
}
