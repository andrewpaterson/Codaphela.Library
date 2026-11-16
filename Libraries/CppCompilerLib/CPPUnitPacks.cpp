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


//Finds the parameter packs a pack expansion's pattern names: the packs of the bindings in use whose parameters it
//names, other than one already being expanded (a type pack whose type is one element for now).
class CCPPPackFinder : public CCPPSyntaxChildVisitor
{
public:
	CCPPUnit*				mpcUnit;
	CArrayCPPBindingPtrs*	mpapsPacks;

	void	Visit(CCPPSyntax* pcChild) override
	{
		pcChild->VisitFields(this);
	}

	void	SymbolField(CCPPSymbol** ppcSymbol) override
	{
		SCPPBinding*	psBinding;

		psBinding = *ppcSymbol ? mpcUnit->FindBinding(*ppcSymbol) : NULL;
		if (psBinding && psBinding->IsPack() && !(psBinding->papsPack && psBinding->psType) && (mpapsPacks->Find(psBinding) == ARRAY_ELEMENT_NOT_FOUND))
		{
			mpapsPacks->Add(psBinding);
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A new empty list of a pack's types, kept until the generator is killed.
//
//////////////////////////////////////////////////////////////////////////
CArrayCPP65Types* CCPPUnit::NewPack(void)
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
SCPPBinding* CCPPUnit::PackBinding(CCPPSyntaxParameter* pcParameter)
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
// A symbol for an element of the function parameter pack being generated: a
// variable with the pack's name, kept until the generator is killed.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPUnit::NewPackElement(CCPPSymbol* pcPack)
{
	CCPPSymbol*	pcSymbol;

	pcSymbol = NewMalloc<CCPPSymbol>();
	pcSymbol->Init(pcPack->mlliName, CPPSK_Variable, pcPack->mpcDeclaration, pcPack->mpcScope);
	mapcPackSymbols.Add(pcSymbol);
	mapcPackElements.Add(pcSymbol);
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call's arguments expand a pack (f(args...)).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HasPackExpansion(CArraySyntax* papcArguments)
{
	size	i;

	for (i = 0; i < papcArguments->NumElements(); i++)
	{
		if (papcArguments->GetPtr(i)->GetKind() == CPPS_PackExpansionExpression)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A pack of values for a non-type parameter pack, kept until the generator
// is killed.
//
//////////////////////////////////////////////////////////////////////////
CArrayInt64* CCPPUnit::NewValuePack(void)
{
	CArrayInt64*	palliPack;

	palliPack = NewMalloc<CArrayInt64>();
	palliPack->Init();
	mapalliPacks.Add(palliPack);
	return palliPack;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the parameter packs pcPattern names (CCPPPackFinder).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::FindPacks(CCPPSyntax* pcPattern, CArrayCPPBindingPtrs* papsPacks)
{
	CCPPPackFinder	cFinder;

	cFinder.mpcUnit = this;
	cFinder.mpapsPacks = papsPacks;
	pcPattern->VisitFields(&cFinder);
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
bool CCPPUnit::ExpandTemplateArguments(CArraySyntax* papcArguments, CArrayCPPExpandedArguments* pasExpanded)
{
	CArrayCPPBindingPtrs	apsPacks;
	SCPPExpandedArgument*	psExpanded;
	SCPPBinding*			psPack;
	CCPPSyntax*				pcArgument;
	CCPPSyntax*				pcPattern;
	size					uiElements;
	size					i;
	size					j;
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
			psExpanded = pasExpanded->Add();
			psExpanded->pcArgument = pcArgument;
			psExpanded->psType = NULL;
			psExpanded->lliValue = 0;
			psExpanded->bExpanded = false;
			continue;
		}

		apsPacks.ReInit();
		FindPacks(pcPattern, &apsPacks);
		if (apsPacks.NumElements() == 0)
		{
			bResult = Error(pcArgument, "The pack expansion names no parameter pack");
			break;
		}
		uiElements = apsPacks.GetPtr(0)->PackSize();
		for (j = 1; j < apsPacks.NumElements(); j++)
		{
			if (apsPacks.GetPtr(j)->PackSize() != uiElements)
			{
				bResult = Error(pcArgument, "The packs the pack expansion expands have different lengths");
				break;
			}
		}
		for (k = 0; bResult && (k < uiElements); k++)
		{
			for (j = 0; j < apsPacks.NumElements(); j++)
			{
				psPack = apsPacks.GetPtr(j);
				if (psPack->papsPack)
				{
					psPack->psType = psPack->papsPack->GetPtr(k);
				}
				else
				{
					psPack->lliValue = psPack->palliPack->GetValue(k);
				}
			}
			psExpanded = pasExpanded->Add();
			psExpanded->pcArgument = pcArgument;
			psExpanded->psType = NULL;
			psExpanded->lliValue = 0;
			psExpanded->bExpanded = true;
			if (pcPattern->GetKind() == CPPS_TypeId)
			{
				psExpanded->psType = mcDeclarators.GetTypeIdType((CCPPSyntaxTypeId*)pcPattern);
				bResult = (psExpanded->psType != NULL) || Error(pcArgument, "The template argument is not a type the compiler supports");
			}
			else
			{
				bResult = Constant(pcPattern, &psExpanded->lliValue) || ConstantError(pcArgument, "The template argument must be a constant");
			}
		}
		for (j = 0; j < apsPacks.NumElements(); j++)
		{
			apsPacks.GetPtr(j)->psType = NULL;
			apsPacks.GetPtr(j)->lliValue = 0;
		}
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
SCPP65Type* CCPPUnit::ExpandedArgumentType(SCPPExpandedArgument* psArgument)
{
	if (psArgument->bExpanded)
	{
		return psArgument->psType;
	}
	return (psArgument->pcArgument->GetKind() == CPPS_TypeId) ? mcDeclarators.GetTypeIdType((CCPPSyntaxTypeId*)psArgument->pcArgument) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of an expanded template argument for a non-type parameter:
// an element's value, or the constant an expression is.  An error if it is
// neither.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ExpandedArgumentValue(SCPPExpandedArgument* psArgument, int64* plliValue)
{
	if (psArgument->bExpanded)
	{
		*plliValue = psArgument->lliValue;
		return !psArgument->psType || Error(psArgument->pcArgument, "The template argument must be a constant");
	}
	return Constant(psArgument->pcArgument, plliValue) || ConstantError(psArgument->pcArgument, "The template argument must be a constant");
}


//////////////////////////////////////////////////////////////////////////
//
// sizeof...(pcPack): how many elements the parameter pack has, a template
// parameter pack or a function parameter pack (by its template pack).
// [expr.sizeof] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::PackSize(CCPPSymbol* pcPack, int64* plliValue)
{
	SCPPBinding*	psBinding;

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
