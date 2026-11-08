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
#include "CPP65Generator.h"
#include "CPPSymbolKey.h"


//The generator's templates: declaring them, binding and deducing their arguments, and instantiating classes and functions.


//////////////////////////////////////////////////////////////////////////
//
// Records a template declaration: a class template, a function template, a member
// function of a class template defined outside it, a member template defined outside
// its class (template<class M> template<class N> ... for a class template), or an
// explicit or partial specialisation of a class or function template.  Explicit
// specialisations of members are ignored until used.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::TemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplate)
{
	SCPP65Template*					psTemplate;
	CCPPSyntax*						pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPSyntaxTemplateDeclaration*	pcClassTemplate;
	CCPPSyntaxClassSpecifier*		pcSpecifier;
	CCPPSyntaxName*					pcName;
	CCPPSyntaxNameComponent*		pcClass;
	SCPP65Template*					psClass;
	bool							bExplicit;

	pcDeclaration = pcTemplate->mpcDeclaration;
	if (pcTemplate->mbInstantiation && !pcTemplate->mbExtern && pcDeclaration)
	{
		return ExplicitInstantiation(pcDeclaration);
	}
	if (pcTemplate->mbInstantiation || !pcDeclaration)
	{
		return true;
	}
	bExplicit = pcTemplate->macpParameters.NumElements() == 0;

	//template<class M> template<class N> N SBox<M>::Mul(N n) { ... }
	pcClassTemplate = NULL;
	if ((pcDeclaration->GetKind() == CPPS_TemplateDeclaration) && !bExplicit)
	{
		pcClassTemplate = pcTemplate;
		pcTemplate = (CCPPSyntaxTemplateDeclaration*)pcDeclaration;
		pcDeclaration = pcTemplate->mpcDeclaration;
		if (pcTemplate->mbInstantiation || (pcTemplate->macpParameters.NumElements() == 0) || !pcDeclaration || (pcDeclaration->GetKind() != CPPS_FunctionDefinition))
		{
			return true;
		}
	}

	psTemplate = NULL;
	if ((pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && !pcClassTemplate)
	{
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
		if (pcSimple->mpcSpecifiers && pcSimple->mpcSpecifiers->mpcType && (pcSimple->mpcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
		{
			//template<class T> struct SAlign<T*> { ... } and template<> struct SAlign<char> { ... } specialise SAlign.
			pcSpecifier = (CCPPSyntaxClassSpecifier*)pcSimple->mpcSpecifiers->mpcType;
			pcClass = pcSpecifier->mpcName ? pcSpecifier->mpcName->GetLast() : NULL;
			if (pcClass && pcClass->mbTemplateArguments)
			{
				Specialisation(pcSpecifier->mpcSymbol, pcTemplate, pcSpecifier, pcClass);
			}
			else if (!bExplicit)
			{
				NewTemplate(pcSpecifier->mpcSymbol, pcTemplate, pcSpecifier, false);
			}
		}
	}
	else if (pcDeclaration->GetKind() == CPPS_FunctionDefinition)
	{
		pcFunction = (CCPPSyntaxFunctionDefinition*)pcDeclaration;
		pcName = pcFunction->mpcDeclarator->GetName();
		if (pcName && pcName->IsQualifiedByClass() && !bExplicit)
		{
			//template<class M> M* CArrayTemplate<M>::Get(size iIndex) { ... }
			pcClass = pcName->GetQualifier();
			psClass = GetComponentTemplate(pcClass);
			if (psClass && pcClassTemplate)
			{
				psTemplate = NewTemplate(psClass->pcSymbol, pcTemplate, pcFunction, true);
				psTemplate->bMemberTemplate = true;
				psTemplate->pcClassDeclaration = pcClassTemplate;
			}
			else if (psClass)
			{
				psTemplate = NewTemplate(psClass->pcSymbol, pcTemplate, pcFunction, true);
				psTemplate->psOwner = OutsideMemberOwner(psClass, pcClass, pcTemplate);
			}
			else if (pcClass->mpcSymbol && (pcClass->mpcSymbol->meKind == CPPSK_Class))
			{
				//template<class T> int CValue::Size(T* p) { ... }
				psTemplate = NewTemplate(pcClass->mpcSymbol, pcTemplate, pcFunction, true);
				psTemplate->bMemberTemplate = true;
			}
			if (psTemplate && psTemplate->bMemberTemplate)
			{
				psTemplate->lliName = GetFunctionName(pcName->GetLast());
			}
		}
		else if (pcName && (pcName->macpComponents.NumElements() == 1) && bExplicit && !pcClassTemplate)
		{
			//template<> int Size<char>(char c) { ... } and template<> int Size(short s) { ... }
			Specialisation(pcName->mpcSymbol, pcTemplate, pcFunction, pcName->GetLast());
		}
		else if (pcName && !pcClassTemplate && !bExplicit && (pcName->mpcSymbol || (pcName->GetLast()->meKind == CPPNC_Operator)))
		{
			//An operator template has no symbol; it is found by its operator (FindFreeOperator).
			NewTemplate(pcName->mpcSymbol, pcTemplate, pcFunction, false);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Records a specialisation of the template pcSymbol names, which must be declared
// before it.  pcPattern holds its template arguments (a function specialisation's
// may have none: they are deduced from its parameters).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::Specialisation(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, CCPPSyntaxNameComponent* pcPattern)
{
	SCPP65Template*	psPrimary;
	SCPP65Template*	psTemplate;
	int64			lliName;

	//template<> int Size(short s) names a new function; its template is found by name.
	psPrimary = pcSymbol ? GetTemplate(pcSymbol) : NULL;
	lliName = pcPattern ? GetComponentName(pcPattern) : -1;
	if (!psPrimary && (lliName != -1))
	{
		psPrimary = GetTemplateByName(lliName, pcDefinition->GetKind(), ComponentLookup(pcPattern));
	}
	if (!psPrimary || (psPrimary->pcDefinition->GetKind() != pcDefinition->GetKind()))
	{
		return NULL;
	}
	psTemplate = NewTemplate(psPrimary->pcSymbol, pcDeclaration, pcDefinition, false);
	psTemplate->psPrimary = psPrimary;
	psTemplate->pcPattern = pcPattern;
	return psTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// The class template a member function defined outside it belongs to: the primary
// when the class's arguments are the declaration's parameters in order
// (CArrayTemplate<M>), else the partial specialisation with the same arguments
// written the same way (SAlign<T*>).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::OutsideMemberOwner(SCPP65Template* psPrimary, CCPPSyntaxNameComponent* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	SCPP65Template*		psTemplate;
	CCPPSyntax*			pcArgument;
	CCPPSyntax*			pcParameter;
	CCPPSyntaxTypeId*	pcTypeId;
	CCPPSymbol*			pcSymbol;
	CChars				szClass;
	CChars				szPattern;
	size				i;
	bool				bPrimary;

	if (!pcClass->mbTemplateArguments)
	{
		return psPrimary;
	}
	bPrimary = pcClass->macpTemplateArguments.NumElements() == pcDeclaration->macpParameters.NumElements();
	for (i = 0; bPrimary && (i < pcClass->macpTemplateArguments.NumElements()); i++)
	{
		pcArgument = pcClass->macpTemplateArguments.GetPtr(i);
		pcParameter = pcDeclaration->macpParameters.GetPtr(i);
		pcSymbol = NULL;
		if (pcArgument->GetKind() == CPPS_TypeId)
		{
			pcTypeId = (CCPPSyntaxTypeId*)pcArgument;
			if (pcTypeId->mpcSpecifiers && pcTypeId->mpcSpecifiers->mpcType && (pcTypeId->mpcSpecifiers->mpcType->GetKind() == CPPS_NamedTypeSpecifier) && !(pcTypeId->mpcSpecifiers->muiFlags & CPPDS_CONST) && (!pcTypeId->mpcDeclarator || ((pcTypeId->mpcDeclarator->NumPointers() == 0) && (pcTypeId->mpcDeclarator->macpSuffixes.NumElements() == 0))))
			{
				pcSymbol = ((CCPPSyntaxNamedTypeSpecifier*)pcTypeId->mpcSpecifiers->mpcType)->mpcName->mpcSymbol;
			}
		}
		else if (pcArgument->GetKind() == CPPS_IdExpression)
		{
			pcSymbol = ((CCPPSyntaxIdExpression*)pcArgument)->mpcName->mpcSymbol;
		}
		bPrimary = pcSymbol && (pcSymbol == ((pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol));
	}
	if (bPrimary)
	{
		return psPrimary;
	}

	szClass.Init();
	pcClass->PrettyPrint(&szClass, 0);
	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if ((psTemplate->psPrimary != psPrimary) || (psTemplate == psPrimary) || !psTemplate->pcPattern)
		{
			continue;
		}
		szPattern.Init();
		psTemplate->pcPattern->PrettyPrint(&szPattern, 0);
		if (szPattern.Equals(&szClass))
		{
			szPattern.Kill();
			szClass.Kill();
			return psTemplate;
		}
		szPattern.Kill();
	}
	szClass.Kill();
	return psPrimary;
}


//////////////////////////////////////////////////////////////////////////
//
// A new template, kept until the generator is killed.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::NewTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember)
{
	SCPP65Template*	psTemplate;

	psTemplate = NewMalloc<SCPP65Template>();
	psTemplate->pcSymbol = pcSymbol;
	psTemplate->pcDeclaration = pcDeclaration;
	psTemplate->pcDefinition = pcDefinition;
	psTemplate->bMember = bMember;
	psTemplate->bMemberTemplate = false;
	psTemplate->psClass = NULL;
	psTemplate->lliName = -1;
	psTemplate->pcClassDeclaration = NULL;
	psTemplate->pcDeclarator = NULL;
	psTemplate->bStatic = false;
	psTemplate->psPrimary = psTemplate;
	psTemplate->pcPattern = NULL;
	psTemplate->psOwner = NULL;
	psTemplate->bArguments = false;
	psTemplate->mszArguments.Init();
	psTemplate->pcNamespace = mpcDeclaringNamespace;
	mapsTemplates.Add(psTemplate);
	return psTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// template class Name<Arguments>; generates every member function of the instance
// that has a definition.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ExplicitInstantiation(CCPPSyntax* pcDeclaration)
{
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxTypeSpecifier*		pcType;
	CCPPSyntaxName*					pcName;
	SCPP65Class*					psClass;
	SCPP65Function*					psFunction;
	size							i;

	pcType = NULL;
	if (pcDeclaration->GetKind() == CPPS_SimpleDeclaration)
	{
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
		pcType = pcSimple->mpcSpecifiers ? pcSimple->mpcSpecifiers->mpcType : NULL;
	}
	if (!pcType || ((pcType->GetKind() != CPPS_ElaboratedTypeSpecifier) && (pcType->GetKind() != CPPS_NamedTypeSpecifier)))
	{
		return Error(pcDeclaration, "Only explicit instantiations of class templates are supported by the compiler yet");
	}
	pcName = (pcType->GetKind() == CPPS_ElaboratedTypeSpecifier) ? ((CCPPSyntaxElaboratedTypeSpecifier*)pcType)->mpcName : ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
	psClass = GetNameClass(pcName, pcDeclaration);
	if (!psClass || !CompleteClass(psClass, pcDeclaration))
	{
		return false;
	}
	for (i = 0; i < psClass->mapsMembers.NumElements(); i++)
	{
		psFunction = psClass->mapsMembers.GetPtr(i);
		if (psFunction->pcDefinition || psFunction->bImplicit)
		{
			RequireFunction(psFunction);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The template pcSymbol declares.  A function template declared before it is
// defined has a symbol for each declaration (chained by mpcNext); the template is
// recorded under the definition's.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::GetTemplate(CCPPSymbol* pcSymbol)
{
	SCPP65Template*	psTemplate;
	CCPPSymbol*		pcDeclaration;
	size			i;

	for (pcDeclaration = pcSymbol; pcDeclaration; pcDeclaration = pcDeclaration->mpcNext)
	{
		if ((pcDeclaration != pcSymbol) && (pcDeclaration->meKind != pcSymbol->meKind))
		{
			continue;
		}
		for (i = 0; i < mapsTemplates.NumElements(); i++)
		{
			psTemplate = mapsTemplates.GetPtr(i);
			if (!psTemplate->bMember && (psTemplate->psPrimary == psTemplate) && (psTemplate->pcSymbol == pcDeclaration))
			{
				return psTemplate;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A primary template by name, of a class or a function (eDefinition): one
// visible from pcLookup, else any.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::GetTemplateByName(int64 lliName, ECPPSyntax eDefinition, CCPPScope* pcLookup)
{
	SCPP65Template*	psTemplate;
	SCPP65Template*	psFound;
	size			i;

	psFound = NULL;
	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if (!psTemplate->bMember && (psTemplate->psPrimary == psTemplate) && psTemplate->pcSymbol && (psTemplate->pcSymbol->mlliName == lliName) && (psTemplate->pcDefinition->GetKind() == eDefinition))
		{
			if (psTemplate->pcSymbol->mpcScope->Encloses(pcLookup))
			{
				return psTemplate;
			}
			psFound = psFound ? psFound : psTemplate;
		}
	}
	return psFound;
}


//////////////////////////////////////////////////////////////////////////
//
// The class template a name's component names.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::GetComponentTemplate(CCPPSyntaxNameComponent* pcComponent)
{
	int64	lliName;

	lliName = GetComponentName(pcComponent);
	return (lliName != -1) ? GetTemplateByName(lliName, CPPS_ClassSpecifier, ComponentLookup(pcComponent)) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Binds a template's parameters to its arguments, worked out with the current
// bindings.  Parameters without an argument take what was deduced (a type in
// papsDeduced, or for a non-type parameter its value in palliDeduced where
// papsDeduced has an entry) or else their defaults.  The arguments are mangled into
// pszArguments.  Returns NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Bindings* CCPP65Generator::BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode, CArrayInt64* palliDeduced)
{
	SCPP65Bindings*			psBindings;
	SCPP65Binding*			psBinding;
	CCPPSyntax*				pcParameter;
	CCPPSyntax*				pcArgument;
	CCPPSyntaxTypeParameter*	pcType;
	CCPPSyntaxParameter*	pcValue;
	SCPP65Bindings*			psSaved;
	SCPP65Type*				psType;
	size					i;
	size					j;
	bool					bResult;

	psBindings = NewBindings();
	bResult = true;
	for (i = 0; i < pcTemplate->macpParameters.NumElements(); i++)
	{
		pcParameter = pcTemplate->macpParameters.GetPtr(i);
		pcArgument = (papcArguments && (i < papcArguments->NumElements())) ? papcArguments->GetPtr(i) : NULL;
		psBinding = psBindings->macBindings.Add();
		psBinding->psType = NULL;
		psBinding->lliValue = 0;
		psBinding->pcParameter = NULL;
		psBinding->papsPack = NULL;

		if (pcParameter->GetKind() == CPPS_TypeParameter)
		{
			pcType = (CCPPSyntaxTypeParameter*)pcParameter;
			psBinding->pcParameter = pcType->mpcSymbol;
			if (pcType->mbPack)
			{
				//A pack (the last parameter) takes the rest of the explicit arguments, or else the deduced types.
				psBinding->papsPack = NewPack();
				pszArguments->Append("_J");
				for (j = i; papcArguments && (j < papcArguments->NumElements()); j++)
				{
					pcArgument = papcArguments->GetPtr(j);
					psType = (pcArgument->GetKind() == CPPS_TypeId) ? GetTypeIdType((CCPPSyntaxTypeId*)pcArgument) : NULL;
					if (!psType)
					{
						bResult = Error(pcArgument, "The template argument is not a type the compiler supports");
						break;
					}
					psBinding->papsPack->Add(psType);
				}
				for (j = i; (!papcArguments || (i >= papcArguments->NumElements())) && papsDeduced && (j < papsDeduced->NumElements()); j++)
				{
					if (papsDeduced->GetPtr(j))
					{
						psBinding->papsPack->Add(papsDeduced->GetPtr(j));
					}
				}
				for (j = 0; j < psBinding->papsPack->NumElements(); j++)
				{
					AppendMangledType(pszArguments, psBinding->papsPack->GetPtr(j));
				}
				pszArguments->Append('E');
				break;
			}
			if (pcArgument && (pcArgument->GetKind() == CPPS_TypeId))
			{
				//const int is a different argument to int (its type is const).
				psBinding->psType = GetTypeIdType((CCPPSyntaxTypeId*)pcArgument);
			}
			else if (!pcArgument && papsDeduced && (i < papsDeduced->NumElements()) && papsDeduced->GetPtr(i))
			{
				psBinding->psType = papsDeduced->GetPtr(i);
			}
			else if (!pcArgument && pcType->mpcDefault)
			{
				//A default can use the parameters before it.
				psSaved = mpsBindings;
				psBindings->psOuter = mpsBindings;
				mpsBindings = psBindings;
				psBinding->psType = GetTypeIdType(pcType->mpcDefault);
				mpsBindings = psSaved;
				psBindings->psOuter = NULL;
			}
			if (!psBinding->psType)
			{
				bResult = Error(pcArgument ? pcArgument : pcNode, "The template argument is not a type the compiler supports");
				break;
			}
			pszArguments->Append('_');
			AppendMangledType(pszArguments, psBinding->psType);
		}
		else if (pcParameter->GetKind() == CPPS_Parameter)
		{
			pcValue = (CCPPSyntaxParameter*)pcParameter;
			psBinding->pcParameter = pcValue->mpcSymbol;
			if (!pcArgument && palliDeduced && papsDeduced && (i < papsDeduced->NumElements()) && papsDeduced->GetPtr(i))
			{
				psBinding->lliValue = palliDeduced->GetValue(i);
			}
			else if (!pcArgument && !pcValue->mpcDefault)
			{
				bResult = Error(pcNode, "The template argument must be a constant");
				break;
			}
			else if (!Constant(pcArgument ? pcArgument : pcValue->mpcDefault, &psBinding->lliValue))
			{
				bResult = Error(pcArgument ? pcArgument : pcNode, "The template argument must be a constant");
				break;
			}
			pszArguments->Append("_L");
			pszArguments->Append((psBinding->lliValue < 0) ? "n" : "");
			pszArguments->Append((psBinding->lliValue < 0) ? -psBinding->lliValue : psBinding->lliValue);
		}
		else
		{
			bResult = Error(pcParameter, "This template parameter is not supported by the compiler yet");
			break;
		}
	}
	return bResult ? psBindings : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a template parameter in what is being generated.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Binding* CCPP65Generator::FindBinding(CCPPSymbol* pcParameter)
{
	SCPP65Bindings*	psBindings;
	SCPP65Binding*	psBinding;
	size			i;

	for (psBindings = mpsBindings; psBindings; psBindings = psBindings->psOuter)
	{
		for (i = 0; i < psBindings->macBindings.NumElements(); i++)
		{
			psBinding = psBindings->macBindings.Get(i);
			if (psBinding->pcParameter == pcParameter)
			{
				return psBinding;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The binding of the template parameter pack a function parameter pack (Args ...
// args) is declared with, or NULL if the parameter is not a pack.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Binding* CCPP65Generator::PackBinding(CCPPSyntaxParameter* pcParameter)
{
	CCPPSyntaxTypeSpecifier*	pcType;
	SCPP65Binding*				psBinding;

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
// The instance of a class template for the arguments, made if it is new.  It is
// laid out when it is first needed whole, from the most specialised definition
// that matches the arguments.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::InstantiateClass(SCPP65Template* psTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	SCPP65Bindings*	psBindings;
	SCPP65Bindings*	psSpecialised;
	SCPP65Template*	psSpecialisation;
	SCPP65Class*	psClass;
	CChars			szArguments;
	size			i;

	szArguments.Init();
	psBindings = BindArguments(psTemplate->pcDeclaration, papcArguments, NULL, &szArguments, pcNode);
	if (!psBindings)
	{
		szArguments.Kill();
		return NULL;
	}

	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if (psClass->psTemplate && (psClass->psTemplate->psPrimary == psTemplate) && psClass->mszArguments.Equals(&szArguments))
		{
			szArguments.Kill();
			return psClass;
		}
	}

	psSpecialisation = FindClassSpecialisation(psTemplate, psBindings, &psSpecialised, pcNode);
	if (psSpecialisation)
	{
		psTemplate = psSpecialisation;
		psBindings = psSpecialised;
	}

	//An instance used before the template was defined (UndefinedInstance) becomes this instance.
	psClass = NULL;
	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		if (IsInstanceNamed(mapsClasses.GetPtr(i), psTemplate->pcSymbol) && mapsClasses.GetPtr(i)->mszArguments.Equals(&szArguments))
		{
			psClass = mapsClasses.GetPtr(i);
			psClass->pcSymbol = psTemplate->pcSymbol;
			psClass->mszArguments.Kill();
			psClass->mszArguments.Init();
			break;
		}
	}
	if (!psClass)
	{
		psClass = NewClass(psTemplate->pcSymbol, psTemplate->pcSymbol->mlliName);
	}
	psClass->psTemplate = psTemplate;
	psClass->pcSpecifier = (CCPPSyntaxClassSpecifier*)psTemplate->pcDefinition;
	psClass->psBindings = psBindings;
	psBindings->psClass = psClass;
	psClass->mszArguments.Append(&szArguments);
	szArguments.Kill();
	return psClass;
}


//////////////////////////////////////////////////////////////////////////
//
// An instance of a class template that has only been declared: an incomplete class (it can be pointed to) with the
// instance's name.  It becomes the instance if the template is defined later (InstantiateClass).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::UndefinedInstance(CCPPSymbol* pcSymbol, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPSyntax*		pcDeclaration;
	SCPP65Bindings*	psBindings;
	SCPP65Class*	psClass;
	CChars			szArguments;
	size			i;

	for (pcDeclaration = pcSymbol->mpcDeclaration; pcDeclaration && (pcDeclaration->GetKind() != CPPS_TemplateDeclaration); pcDeclaration = pcDeclaration->mpcParent)
	{
	}
	if (!pcDeclaration)
	{
		return NULL;
	}

	szArguments.Init();
	psBindings = BindArguments((CCPPSyntaxTemplateDeclaration*)pcDeclaration, papcArguments, NULL, &szArguments, pcNode);
	if (!psBindings)
	{
		szArguments.Kill();
		return NULL;
	}
	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if (IsInstanceNamed(psClass, pcSymbol) && psClass->mszArguments.Equals(&szArguments))
		{
			szArguments.Kill();
			return psClass;
		}
	}
	psClass = NewClass(pcSymbol, pcSymbol->mlliName);
	psClass->mszArguments.Append(&szArguments);
	szArguments.Kill();
	return psClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The most specialised specialisation of a class template that matches the
// arguments (psArguments), with its own parameters bound in *ppsBindings.  An
// explicit specialisation is chosen first; of the partial ones, the one whose
// arguments match more of the types' structure.  NULL if none matches.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::FindClassSpecialisation(SCPP65Template* psPrimary, SCPP65Bindings* psArguments, SCPP65Bindings** ppsBindings, CCPPSyntax* pcNode)
{
	SCPP65Template*	psTemplate;
	SCPP65Template*	psBest;
	SCPP65Bindings*	psDeduced;
	SCPP65Bindings*	psBestDeduced;
	SCPP65Binding*	psBinding;
	CCPPSyntax*		pcParameter;
	size			i;
	size			j;
	int				iScore;
	int				iBest;
	bool			bAmbiguous;

	psBest = NULL;
	psBestDeduced = NULL;
	iBest = -1;
	bAmbiguous = false;
	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if ((psTemplate->psPrimary != psPrimary) || (psTemplate == psPrimary) || !psTemplate->pcPattern)
		{
			continue;
		}

		//The specialisation's parameters, deduced as its arguments are matched.
		psDeduced = NewBindings();
		for (j = 0; j < psTemplate->pcDeclaration->macpParameters.NumElements(); j++)
		{
			pcParameter = psTemplate->pcDeclaration->macpParameters.GetPtr(j);
			psBinding = psDeduced->macBindings.Add();
			psBinding->pcParameter = (pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol;
			psBinding->psType = NULL;
			psBinding->lliValue = 0;
			psBinding->papsPack = NULL;
		}

		iScore = (psTemplate->pcDeclaration->macpParameters.NumElements() == 0) ? 1000 : 0;
		if (!MatchPattern(psTemplate, &psTemplate->pcPattern->macpTemplateArguments, psArguments, psDeduced, &iScore))
		{
			continue;
		}
		if (iScore > iBest)
		{
			psBest = psTemplate;
			psBestDeduced = psDeduced;
			iBest = iScore;
			bAmbiguous = false;
		}
		else if (iScore == iBest)
		{
			bAmbiguous = true;
		}
	}
	if (bAmbiguous)
	{
		Error(pcNode, "More than one specialisation of the class template matches equally");
	}
	*ppsBindings = psBestDeduced;
	return psBest;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a specialisation's template arguments (papcPattern) match an instance's
// arguments.  Its parameters are deduced into psDeduced.  *piScore counts how much
// of the arguments' structure the pattern names.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::MatchPattern(SCPP65Template* psSpecialisation, CArraySyntax* papcPattern, SCPP65Bindings* psArguments, SCPP65Bindings* psDeduced, int* piScore)
{
	CCPPSyntax*		pcPattern;
	SCPP65Binding*	psArgument;
	SCPP65Binding*	psBinding;
	CCPPSymbol*		pcSymbol;
	size			i;
	int64			lliValue;

	if (papcPattern->NumElements() != psArguments->macBindings.NumElements())
	{
		return false;
	}
	for (i = 0; i < papcPattern->NumElements(); i++)
	{
		pcPattern = papcPattern->GetPtr(i);
		psArgument = psArguments->macBindings.Get(i);
		if (psArgument->psType)
		{
			if ((pcPattern->GetKind() != CPPS_TypeId) || !MatchType(psSpecialisation, (CCPPSyntaxTypeId*)pcPattern, psArgument, psDeduced, piScore))
			{
				return false;
			}
			continue;
		}

		//A non-type argument: a parameter of the specialisation takes it, a constant must equal it.
		pcSymbol = (pcPattern->GetKind() == CPPS_IdExpression) ? ((CCPPSyntaxIdExpression*)pcPattern)->mpcName->mpcSymbol : NULL;
		psBinding = pcSymbol ? DeducedBinding(psDeduced, pcSymbol) : NULL;
		if (psBinding)
		{
			psBinding->lliValue = psArgument->lliValue;
			psBinding->psType = NULL;
			continue;
		}
		if (!Constant(pcPattern, &lliValue) || (lliValue != psArgument->lliValue))
		{
			return false;
		}
		(*piScore)++;
	}

	//Every parameter must be deduced.
	for (i = 0; i < psDeduced->macBindings.NumElements(); i++)
	{
		pcSymbol = psDeduced->macBindings.Get(i)->pcParameter;
		if (pcSymbol && (pcSymbol->meKind != CPPSK_ValueParameter) && !psDeduced->macBindings.Get(i)->psType)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a type pattern (T, T*, const T, T&, a class template instance X<T>, or a
// type naming none of the specialisation's parameters) matches a type argument.
// Pointers and references are matched outside in, then const at the top level, then
// the type named.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::MatchType(SCPP65Template* psSpecialisation, CCPPSyntaxTypeId* pcPattern, SCPP65Binding* psArgument, SCPP65Bindings* psDeduced, int* piScore)
{
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxPointerOperator*		pcOperator;
	CCPPSyntaxNamedTypeSpecifier*	pcNamed;
	CCPPSyntaxNameComponent*		pcComponent;
	SCPP65Binding*					psBinding;
	SCPP65Template*					psTemplate;
	SCPP65Type*						psType;
	SCPP65Type*						psNamed;
	SCPP65Bindings*					psSaved;
	size							i;
	bool							bResult;

	pcSpecifiers = pcPattern->mpcSpecifiers;
	pcDeclarator = pcPattern->mpcDeclarator;
	if (!pcSpecifiers || (pcDeclarator && (pcDeclarator->macpSuffixes.NumElements() > 0 || pcDeclarator->mpcNested)))
	{
		return false;
	}

	psType = psArgument->psType;
	for (i = pcDeclarator ? pcDeclarator->NumPointers() : 0; i > 0; i--)
	{
		pcOperator = pcDeclarator->GetPointer(i - 1);
		if ((pcOperator->meKind == CPPP_Pointer) && psType->IsPointer())
		{
			psType = psType->psBase;
		}
		else if (((pcOperator->meKind == CPPP_Reference) || (pcOperator->meKind == CPPP_RValueReference)) && psType->IsReference())
		{
			psType = psType->psBase;
		}
		else
		{
			return false;
		}
		(*piScore)++;
	}
	if (pcSpecifiers->muiFlags & CPPDS_CONST)
	{
		//const T matches a const type, T the type without its const.
		if (!psType->bConst)
		{
			return false;
		}
		psType = mcTypes.Qualified(psType, false, psType->bVolatile);
		(*piScore)++;
	}

	//T takes the type.
	pcNamed = (pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_NamedTypeSpecifier)) ? (CCPPSyntaxNamedTypeSpecifier*)pcSpecifiers->mpcType : NULL;
	psBinding = pcNamed ? DeducedBinding(psDeduced, pcNamed->mpcName->mpcSymbol) : NULL;
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
		psTemplate = pcComponent->mpcSymbol ? GetTemplate(pcComponent->mpcSymbol) : GetComponentTemplate(pcComponent);
		if (!psTemplate || !psType->IsClass() || !psType->psClass->psTemplate || (psType->psClass->psTemplate->psPrimary != psTemplate))
		{
			return false;
		}
		(*piScore)++;
		return MatchPattern(psSpecialisation, &pcComponent->macpTemplateArguments, psType->psClass->psBindings, psDeduced, piScore);
	}

	//A type naming none of the parameters must be the same type.
	psSaved = mpsBindings;
	mpsBindings = NULL;
	BeginQuiet();
	psNamed = GetTypeIdType(pcPattern);
	EndQuiet();
	mpsBindings = psSaved;
	bResult = psNamed && psNamed->Same(psArgument->psType);
	*piScore += 2;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The binding of one of a specialisation's parameters, or NULL if pcParameter is
// not one.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Binding* CCPP65Generator::DeducedBinding(SCPP65Bindings* psDeduced, CCPPSymbol* pcParameter)
{
	SCPP65Binding*	psBinding;
	size			i;

	if (!pcParameter)
	{
		return NULL;
	}
	for (i = 0; i < psDeduced->macBindings.NumElements(); i++)
	{
		psBinding = psDeduced->macBindings.Get(i);
		if (psBinding->pcParameter == pcParameter)
		{
			return psBinding;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// New empty bindings, kept until the generator is killed.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Bindings* CCPP65Generator::NewBindings(void)
{
	SCPP65Bindings*	psBindings;

	psBindings = NewMalloc<SCPP65Bindings>();
	psBindings->macBindings.Init();
	psBindings->psOuter = NULL;
	psBindings->psClass = NULL;
	mapsAllBindings.Add(psBindings);
	return psBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// A new empty list of a pack's types, kept until the generator is killed.
//
//////////////////////////////////////////////////////////////////////////
CArrayCPP65Types* CCPP65Generator::NewPack(void)
{
	CArrayCPP65Types*	papsPack;

	papsPack = NewMalloc<CArrayCPP65Types>();
	papsPack->Init();
	mapapsPacks.Add(papsPack);
	return papsPack;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a function template for explicit arguments and the types of the
// call's arguments.  Template parameters without an explicit argument are deduced
// (DeduceArguments).  An explicit specialisation for the arguments is used instead
// of the template's definition.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::InstantiateFunction(SCPP65Template* psTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	SCPP65Template*					psSpecialisation;
	SCPP65Bindings*					psBindings;
	SCPP65Bindings*					psSaved;
	SCPP65Function*					psFunction;
	CArrayCPP65Types				apsArguments;
	CArrayCPP65Types				apsDeduced;
	CArrayInt64						alliDeduced;
	CChars							szArguments;
	CCPPScope*						pcSavedNamespace;
	CCPPSyntaxFunctionSuffix*		pcInClass;
	CCPPSyntaxParameter*			pcParameter;
	size							i;

	if (psTemplate->psClass && !DefineMemberTemplate(psTemplate, pcNode))
	{
		return NULL;
	}
	pcDefinition = (CCPPSyntaxFunctionDefinition*)psTemplate->pcDefinition;

	apsArguments.Init();
	for (i = 0; papcArguments && (i < papcArguments->NumElements()); i++)
	{
		apsArguments.Add(TypeOf(papcArguments->GetPtr(i)));
	}
	apsDeduced.Init();
	alliDeduced.Init();
	DeduceArguments(psTemplate, &apsArguments, papcArguments, &apsDeduced, &alliDeduced);
	apsArguments.Kill();

	szArguments.Init();
	psBindings = BindArguments(psTemplate->pcDeclaration, papcExplicit, &apsDeduced, &szArguments, pcNode, &alliDeduced);
	alliDeduced.Kill();
	apsDeduced.Kill();
	if (!psBindings)
	{
		szArguments.Kill();
		return NULL;
	}

	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psTemplate == psTemplate) && psFunction->mszArguments.Equals(&szArguments))
		{
			szArguments.Kill();
			return psFunction;
		}
	}

	psSpecialisation = FindFunctionSpecialisation(psTemplate, &szArguments, pcNode);
	if (psSpecialisation)
	{
		pcDefinition = (CCPPSyntaxFunctionDefinition*)psSpecialisation->pcDefinition;
	}

	//A member template's instance also sees its class's template arguments.
	psSaved = mpsBindings;
	if (psTemplate->psClass)
	{
		psBindings->psOuter = psTemplate->pcClassDeclaration ? RenameBindings(psTemplate->psClass, psTemplate->pcClassDeclaration) : psTemplate->psClass->psBindings;
	}
	else
	{
		psBindings->psOuter = mpsBindings;
	}
	mpsBindings = psBindings;
	pcSavedNamespace = mpcDeclaringNamespace;
	mpcDeclaringNamespace = psTemplate->pcNamespace;
	psFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, psTemplate->psClass, false, psTemplate, &szArguments);
	mpcDeclaringNamespace = pcSavedNamespace;
	mpsBindings = psSaved;
	if (!psTemplate->psClass)
	{
		psBindings->psOuter = NULL;
	}
	szArguments.Kill();
	if (psFunction)
	{
		psFunction->pcDefinition = pcDefinition;
		psFunction->psBindings = psBindings;
		psFunction->bInline = true;
		psFunction->bStatic |= psTemplate->bStatic;

		//A member template defined outside its class has its default arguments in the class.  [dcl.fct.default]
		pcInClass = (psTemplate->pcDeclarator && (psTemplate->pcDeclarator != pcDefinition->mpcDeclarator)) ? psTemplate->pcDeclarator->GetFunctionSuffix() : NULL;
		if (pcInClass && (pcInClass->macpParameters.NumElements() == psFunction->mapcDefaults.NumElements()))
		{
			for (i = 0; i < pcInClass->macpParameters.NumElements(); i++)
			{
				pcParameter = (CCPPSyntaxParameter*)pcInClass->macpParameters.GetPtr(i);
				if (!psFunction->mapcDefaults.GetPtr(i) && pcParameter->mpcDefault)
				{
					psFunction->mapcDefaults.SetPtr(i, pcParameter->mpcDefault);
				}
			}
		}
	}
	return psFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// Deduces a function template's parameters from the types of a call's arguments
// (an entry may be NULL).  papcArguments are the arguments, or NULL if only their
// types are known.  papsDeduced gets an entry for each template parameter, NULL if
// it is not deduced, and palliDeduced the non-type parameters' values
// (DeduceParameter).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DeduceArguments(SCPP65Template* psTemplate, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						i;

	pcSuffix = ((CCPPSyntaxFunctionDefinition*)psTemplate->pcDefinition)->mpcDeclarator->GetFunctionSuffix();
	for (i = 0; i < psTemplate->pcDeclaration->macpParameters.NumElements(); i++)
	{
		papsDeduced->Add(NULL);
		palliDeduced->Add(0);
	}
	DeduceParameters(psTemplate, pcSuffix, papsArguments, papcArguments, papsDeduced, palliDeduced);
}


//////////////////////////////////////////////////////////////////////////
//
// Deduces from each of a function suffix's parameters that has an argument
// type in papsArguments, until a parameter pack takes the rest.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DeduceParameters(SCPP65Template* psTemplate, CCPPSyntaxFunctionSuffix* pcSuffix, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
{
	CCPPSyntaxParameter*	pcParameter;
	size					i;

	for (i = 0; pcSuffix && (i < pcSuffix->macpParameters.NumElements()) && (i < papsArguments->NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		if (!DeduceParameter(psTemplate, pcParameter, papsArguments, papcArguments, i, papsDeduced, palliDeduced))
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
bool CCPP65Generator::DeduceParameter(SCPP65Template* psTemplate, CCPPSyntaxParameter* pcParameter, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, size i, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
{
	CCPPSyntaxTypeSpecifier*	pcType;
	CCPPSyntaxName*				pcName;
	CCPPSyntax*					pcTemplateParameter;
	SCPP65Type*					psArgument;
	size						j;
	size						k;
	size						uiPointers;
	bool						bForwarding;

	DeduceMemberPointer(psTemplate, pcParameter->mpcDeclarator, papsArguments->GetPtr(i), papsDeduced, palliDeduced);
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
		DeduceFromInstance(psTemplate, pcName->GetLast(), psArgument, papsDeduced, palliDeduced);
		return true;
	}
	for (j = 0; j < psTemplate->pcDeclaration->macpParameters.NumElements(); j++)
	{
		pcTemplateParameter = psTemplate->pcDeclaration->macpParameters.GetPtr(j);
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
					papsDeduced->SetPtr(j, psArgument);
				}
				else
				{
					papsDeduced->Add(psArgument);
				}
			}
			return false;
		}
		psArgument = papsArguments->GetPtr(i);
		if (bForwarding)
		{
			if (psArgument && !papsDeduced->GetPtr(j))
			{
				papsDeduced->SetPtr(j, ForwardedType(psArgument->WithoutReference(), papcArguments ? papcArguments->GetPtr(i) : NULL));
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
			psArgument = mcTypes.Qualified(psArgument, psArgument->bConst && !(pcParameter->mpcSpecifiers->muiFlags & CPPDS_CONST), psArgument->bVolatile && !(pcParameter->mpcSpecifiers->muiFlags & CPPDS_VOLATILE));
		}
		if (psArgument && !papsDeduced->GetPtr(j))
		{
			papsDeduced->SetPtr(j, psArgument);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the parameter is T&& (or Args&& ...), and not const T&&.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsForwardingReference(CCPPSyntaxParameter* pcParameter)
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
SCPP65Type* CCPP65Generator::ForwardedType(SCPP65Type* psArgument, CCPPSyntax* pcArgument)
{
	SCPP65Type*	psPlace;

	if (psArgument->IsArray())
	{
		return mcTypes.Qualified(mcTypes.Pointer(psArgument->psBase), true, false);
	}
	if (psArgument->IsFunction())
	{
		return mcTypes.Qualified(mcTypes.Pointer(psArgument), true, false);
	}
	if (psArgument->IsClass() || !pcArgument)
	{
		return psArgument;
	}
	if (SkipParentheses(pcArgument)->GetKind() == CPPS_StringExpression)
	{
		return mcTypes.Qualified(mcTypes.Pointer(mcTypes.Qualified(psArgument->psBase, true, false)), true, false);
	}

	//An array's type here is already the pointer it decays to, but its place is the array.
	psPlace = PlaceOf(pcArgument);
	if (psPlace && !psPlace->WithoutReference()->IsArray())
	{
		return psArgument;
	}
	return mcTypes.Qualified(psArgument, true, psArgument->bVolatile);
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter declared as a pointer to a member of a template parameter, void
// (M::*f)(A*, V), deduces M as the class of the argument's pointer to member
// (&CTarget::Changed), and A and V from the parameters of the argument's member
// function, as a call's arguments deduce (DeduceParameters).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DeduceMemberPointer(SCPP65Template* psTemplate, CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type* psArgument, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
{
	CCPPSyntaxPointerOperator*	pcPointer;
	CCPPSyntaxDeclarator*		pcOuter;
	CCPPSyntax*					pcSuffix;
	CCPPSyntax*					pcTemplateParameter;
	CCPPSymbol*					pcSymbol;
	size						i;
	size						j;

	psArgument = (psArgument && psArgument->IsReference()) ? psArgument->psBase : psArgument;
	if (!psArgument || !psArgument->IsMemberPointer() || !psArgument->psClass)
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
			for (j = 0; pcSymbol && (j < psTemplate->pcDeclaration->macpParameters.NumElements()); j++)
			{
				pcTemplateParameter = psTemplate->pcDeclaration->macpParameters.GetPtr(j);
				if ((pcTemplateParameter->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)pcTemplateParameter)->mpcSymbol == pcSymbol) && !papsDeduced->GetPtr(j))
				{
					papsDeduced->SetPtr(j, psArgument->psClass->psType);
				}
			}

			//The member function's parameters follow the parentheses around (M::*f).
			pcSuffix = (pcOuter && (pcOuter->macpSuffixes.NumElements() > 0)) ? pcOuter->macpSuffixes.GetPtr(0) : NULL;
			if (pcSuffix && (pcSuffix->GetKind() == CPPS_FunctionSuffix) && psArgument->psBase && psArgument->psBase->IsFunction())
			{
				DeduceParameters(psTemplate, (CCPPSyntaxFunctionSuffix*)pcSuffix, psArgument->psBase->papsParameters, NULL, papsDeduced, palliDeduced);
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
void CCPP65Generator::DeduceFromInstance(SCPP65Template* psTemplate, CCPPSyntaxNameComponent* pcComponent, SCPP65Type* psArgument, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
{
	SCPP65Class*	psInstance;
	SCPP65Binding*	psBinding;
	CCPPSyntax*		pcTemplateParameter;
	CCPPSymbol*		pcSymbol;
	size			i;
	size			j;

	if (!psArgument || !psArgument->IsClass() || !pcComponent->mpcSymbol)
	{
		return;
	}
	psInstance = FindInstanceBase(psArgument->psClass, pcComponent->mpcSymbol);
	if (!psInstance || (psInstance->psTemplate->psPrimary != psInstance->psTemplate) || !psInstance->psBindings)
	{
		return;
	}
	for (i = 0; (i < pcComponent->macpTemplateArguments.NumElements()) && (i < psInstance->psBindings->macBindings.NumElements()); i++)
	{
		pcSymbol = GetArgumentSymbol(pcComponent->macpTemplateArguments.GetPtr(i));
		psBinding = psInstance->psBindings->macBindings.Get(i);
		for (j = 0; pcSymbol && (j < psTemplate->pcDeclaration->macpParameters.NumElements()); j++)
		{
			pcTemplateParameter = psTemplate->pcDeclaration->macpParameters.GetPtr(j);
			if ((pcTemplateParameter->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)pcTemplateParameter)->mpcSymbol == pcSymbol) && psBinding->psType && !papsDeduced->GetPtr(j))
			{
				papsDeduced->SetPtr(j, psBinding->psType);
			}
			else if ((pcTemplateParameter->GetKind() == CPPS_Parameter) && (((CCPPSyntaxParameter*)pcTemplateParameter)->mpcSymbol == pcSymbol) && !psBinding->psType && !papsDeduced->GetPtr(j))
			{
				papsDeduced->SetPtr(j, &mcTypes.msInt);
				palliDeduced->SetValue(j, psBinding->lliValue);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// psClass if it is an instance of the class template pcTemplate names, else the
// first of its bases that is.  NULL if none is.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::FindInstanceBase(SCPP65Class* psClass, CCPPSymbol* pcTemplate)
{
	SCPP65Class*	psFound;
	size			i;

	if (psClass->psTemplate && (psClass->psTemplate->psPrimary->pcSymbol == pcTemplate))
	{
		return psClass;
	}
	if (!psClass->bBases)
	{
		BeginQuiet();
		ResolveBases(psClass, NULL);
		EndQuiet();
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psFound = FindInstanceBase(psClass->macBases.Get(i)->psClass, pcTemplate);
		if (psFound)
		{
			return psFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol a template argument is just the name of (J or T), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPP65Generator::GetArgumentSymbol(CCPPSyntax* pcArgument)
{
	CCPPSyntaxTypeId*			pcTypeId;
	CCPPSyntaxTypeSpecifier*	pcType;

	pcArgument = SkipParentheses(pcArgument);
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
// The explicit specialisation of a function template for the mangled arguments,
// or NULL.  A specialisation's arguments are its explicit ones (Size<char>) and
// those deduced from its parameters' types (Size(short)), worked out once.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::FindFunctionSpecialisation(SCPP65Template* psPrimary, CChars* pszArguments, CCPPSyntax* pcNode)
{
	SCPP65Template*				psTemplate;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	SCPP65Bindings*				psSaved;
	SCPP65Type*					psType;
	CArrayCPP65Types			apsParameters;
	CArrayCPP65Types			apsDeduced;
	CArrayInt64					alliDeduced;
	size						i;
	size						j;

	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if ((psTemplate->psPrimary != psPrimary) || (psTemplate == psPrimary) || (psTemplate->pcDefinition->GetKind() != CPPS_FunctionDefinition))
		{
			continue;
		}
		if (!psTemplate->bArguments)
		{
			psTemplate->bArguments = true;
			psSaved = mpsBindings;
			mpsBindings = NULL;
			BeginQuiet();
			apsParameters.Init();
			pcSuffix = ((CCPPSyntaxFunctionDefinition*)psTemplate->pcDefinition)->mpcDeclarator->GetFunctionSuffix();
			for (j = 0; pcSuffix && (j < pcSuffix->macpParameters.NumElements()); j++)
			{
				pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(j);
				psType = GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
				apsParameters.Add((psType && psType->IsReference()) ? psType->psBase : psType);
			}
			apsDeduced.Init();
			alliDeduced.Init();
			DeduceArguments(psPrimary, &apsParameters, NULL, &apsDeduced, &alliDeduced);
			if (!BindArguments(psPrimary->pcDeclaration, (psTemplate->pcPattern && psTemplate->pcPattern->mbTemplateArguments) ? &psTemplate->pcPattern->macpTemplateArguments : NULL, &apsDeduced, &psTemplate->mszArguments, pcNode, &alliDeduced))
			{
				psTemplate->mszArguments.Clear();
			}
			alliDeduced.Kill();
			apsDeduced.Kill();
			apsParameters.Kill();
			EndQuiet();
			mpsBindings = psSaved;
		}
		if (!psTemplate->mszArguments.Empty() && psTemplate->mszArguments.Equals(pszArguments))
		{
			return psTemplate;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A class template instance's bindings under the parameter names of a declaration
// outside the class (template<class N> ... CArrayTemplate<N>::...).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Bindings* CCPP65Generator::RenameBindings(SCPP65Class* psClass, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	SCPP65Bindings*	psBindings;
	CCPPSyntax*		pcParameter;
	size			i;

	psBindings = NewBindings();
	psBindings->psOuter = psClass->psBindings;
	psBindings->psClass = psClass;
	for (i = 0; (i < pcDeclaration->macpParameters.NumElements()) && (i < psClass->psBindings->macBindings.NumElements()); i++)
	{
		pcParameter = pcDeclaration->macpParameters.GetPtr(i);
		*psBindings->macBindings.Add() = *psClass->psBindings->macBindings.Get(i);
		psBindings->macBindings.Tail()->pcParameter = (pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol;
	}
	return psBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps a member template of a class being laid out.  Its instances are made when
// it is called (InstantiateFunction).  Member class templates, friend templates and
// constructor templates are left out.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::MemberTemplate(SCPP65Class* psClass, CCPPSyntaxTemplateDeclaration* pcTemplate)
{
	CCPPSyntax*						pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxName*					pcName;
	SCPP65Template*					psTemplate;
	int64							lliName;

	pcDeclaration = pcTemplate->mpcDeclaration;
	if (pcTemplate->mbInstantiation || (pcTemplate->macpParameters.NumElements() == 0) || !pcDeclaration)
	{
		return true;
	}

	pcSpecifiers = NULL;
	pcDeclarator = NULL;
	if (pcDeclaration->GetKind() == CPPS_FunctionDefinition)
	{
		pcSpecifiers = ((CCPPSyntaxFunctionDefinition*)pcDeclaration)->mpcSpecifiers;
		pcDeclarator = ((CCPPSyntaxFunctionDefinition*)pcDeclaration)->mpcDeclarator;
	}
	else if (pcDeclaration->GetKind() == CPPS_SimpleDeclaration)
	{
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
		pcInit = (pcSimple->macpDeclarators.NumElements() == 1) ? (CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0) : NULL;
		pcSpecifiers = pcSimple->mpcSpecifiers;
		pcDeclarator = (pcInit && pcInit->mpcDeclarator && pcInit->mpcDeclarator->IsFunction()) ? pcInit->mpcDeclarator : NULL;
	}
	pcName = pcDeclarator ? pcDeclarator->GetName() : NULL;
	if (!pcName || (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_FRIEND)))
	{
		return true;
	}
	lliName = GetFunctionName(pcName->GetLast());
	if ((lliName == -1) || (lliName == psClass->lliName) || (pcName->GetLast()->meKind == CPPNC_Destructor))
	{
		return true;
	}

	psTemplate = NewTemplate(pcName->mpcSymbol, pcTemplate, (pcDeclaration->GetKind() == CPPS_FunctionDefinition) ? pcDeclaration : NULL, true);
	psTemplate->bMemberTemplate = true;
	psTemplate->psClass = psClass;
	psTemplate->lliName = lliName;
	psTemplate->pcDeclarator = pcDeclarator;
	psTemplate->bStatic = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_STATIC);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function with these parameters takes uiArguments arguments: defaults
// make up the rest, and ... or a function parameter pack (Args ... args) take any
// number more.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::TakesArguments(CCPPSyntaxFunctionSuffix* pcSuffix, size uiArguments)
{
	CCPPSyntaxParameter*	pcLast;
	size					uiParameters;
	size					uiRequired;

	uiParameters = pcSuffix->NumParameters();
	uiRequired = pcSuffix->RequiredParameters();
	pcLast = (uiParameters > 0) ? (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(uiParameters - 1) : NULL;
	if (pcLast && pcLast->mpcDeclarator && pcLast->mpcDeclarator->mbPack)
	{
		return uiArguments + 1 >= uiRequired;
	}
	return (uiArguments >= uiRequired) && ((uiArguments <= uiParameters) || pcSuffix->mbVariadic);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function's parameters are followed by const: int Get(void) const.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstSuffix(CCPPSyntaxFunctionSuffix* pcSuffix)
{
	CCPPSyntax*	pcQualifier;
	size		i;

	for (i = 0; pcSuffix && (i < pcSuffix->macpQualifiers.NumElements()); i++)
	{
		pcQualifier = pcSuffix->macpQualifiers.GetPtr(i);
		if ((pcQualifier->GetKind() == CPPS_KeywordSpecifier) && ((((CCPPSyntaxKeywordSpecifier*)pcQualifier)->meKeyword == CT_const) || (((CCPPSyntaxKeywordSpecifier*)pcQualifier)->meKeyword == CT___const__)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A member template of psClass (or of a base) named lliName that takes uiArguments
// arguments.  Of two that do, the one whose const matches the object's (iConstObject
// 1 for a const object, 0 for one that is not, -1 for none) is chosen.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::FindMemberTemplate(SCPP65Class* psClass, int64 lliName, size uiArguments, int iConstObject)
{
	CArrayCPP65Templates	apsCandidates;
	SCPP65Template*			psTemplate;

	apsCandidates.Init();
	GetMemberTemplates(psClass, lliName, uiArguments, &apsCandidates);
	psTemplate = ChooseTemplate(&apsCandidates, iConstObject, NULL, NULL);
	apsCandidates.Kill();
	return psTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// The member template of psClass (or of a base) named lliName that pcCall, with
// the explicit template arguments papcExplicit, uses (ChooseTemplate).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::ChooseMemberTemplate(SCPP65Class* psClass, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcExplicit, int iConstObject)
{
	CArrayCPP65Templates	apsCandidates;
	SCPP65Template*			psTemplate;

	apsCandidates.Init();
	GetMemberTemplates(psClass, lliName, pcCall->macpArguments.NumElements(), &apsCandidates);
	psTemplate = ChooseTemplate(&apsCandidates, iConstObject, papcExplicit, pcCall);
	apsCandidates.Kill();
	return psTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds psClass's member templates named lliName that take uiArguments arguments,
// or else those of its first base that has any.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::GetMemberTemplates(SCPP65Class* psClass, int64 lliName, size uiArguments, CArrayCPP65Templates* papsCandidates)
{
	SCPP65Template*				psTemplate;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						i;

	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if (!psTemplate->bMemberTemplate || (psTemplate->psClass != psClass) || (psTemplate->lliName != lliName))
		{
			continue;
		}
		pcSuffix = psTemplate->pcDeclarator->GetFunctionSuffix();
		if (pcSuffix && TakesArguments(pcSuffix, uiArguments))
		{
			papsCandidates->Add(psTemplate);
		}
	}

	for (i = 0; (papsCandidates->NumElements() == 0) && (i < psClass->macBases.NumElements()); i++)
	{
		GetMemberTemplates(psClass->macBases.Get(i)->psClass, lliName, uiArguments, papsCandidates);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function template pcCall to pcSymbol, with the explicit template arguments
// papcExplicit, uses: of its overloads (the declarations chained by mpcNext) that
// take the call's arguments, the one ChooseTemplate chooses, or else
// GetTemplate's.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::GetCallTemplate(CCPPSymbol* pcSymbol, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcExplicit)
{
	CArrayCPP65Templates	apsCandidates;
	CArrayScopes			apcNamespaces;
	SCPP65Template*			psTemplate;
	size					i;

	apsCandidates.Init();
	apcNamespaces.Init();
	if (pcCall->mpcCallee->GetKind() == CPPS_IdExpression)
	{
		GetCallNamespaces(((CCPPSyntaxIdExpression*)pcCall->mpcCallee)->mpcName, lliName, &pcCall->macpArguments, &apcNamespaces);
	}
	for (i = 0; i < apcNamespaces.NumElements(); i++)
	{
		AddCallTemplates(apcNamespaces.GetPtr(i)->FindHere(lliName), pcCall, &apsCandidates);
	}
	if (apcNamespaces.NumElements() == 0)
	{
		AddCallTemplates(pcSymbol, pcCall, &apsCandidates);
	}
	psTemplate = NULL;
	if (apsCandidates.NumElements() > 0)
	{
		psTemplate = ChooseTemplate(&apsCandidates, -1, papcExplicit, pcCall);
	}
	else if (pcSymbol)
	{
		psTemplate = GetTemplate(pcSymbol);
	}
	apcNamespaces.Kill();
	apsCandidates.Kill();
	return psTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the function templates of pcSymbol's declarations that take the call's
// number of arguments.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddCallTemplates(CCPPSymbol* pcSymbol, CCPPSyntaxCallExpression* pcCall, CArrayCPP65Templates* papsCandidates)
{
	SCPP65Template*				psTemplate;
	CCPPSymbol*					pcDeclaration;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						i;

	for (pcDeclaration = pcSymbol; pcDeclaration; pcDeclaration = pcDeclaration->mpcNext)
	{
		if (pcDeclaration->meKind != CPPSK_FunctionTemplate)
		{
			continue;
		}
		for (i = 0; i < mapsTemplates.NumElements(); i++)
		{
			psTemplate = mapsTemplates.GetPtr(i);
			if (psTemplate->bMember || (psTemplate->psPrimary != psTemplate) || (psTemplate->pcSymbol != pcDeclaration) || !psTemplate->pcDefinition || (psTemplate->pcDefinition->GetKind() != CPPS_FunctionDefinition))
			{
				continue;
			}
			pcSuffix = ((CCPPSyntaxFunctionDefinition*)psTemplate->pcDefinition)->mpcDeclarator->GetFunctionSuffix();
			if (pcSuffix && TakesArguments(pcSuffix, pcCall->macpArguments.NumElements()))
			{
				papsCandidates->Add(psTemplate);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Of papsCandidates, the templates that take a call's number of arguments in the
// order they were found, the one the call uses.  When there are several, the ones
// the call can use (TemplateTakes) are chosen from, if there are any, as
// SMemberCall::Init's overloads differ only in their function's type; of those,
// the ones whose arguments convert best (as plain overloads are ranked), as
// CObjects::Get<M>(OIndex) and Get<M>(const char*) differ only in their
// parameter's type.  Then the first whose const matches the object's
// (iConstObject 1 for a const object, 0 for one that is not, -1 for none) is
// chosen, or else the first.  pcCall is NULL to choose by the number of
// arguments alone.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::ChooseTemplate(CArrayCPP65Templates* papsCandidates, int iConstObject, CArraySyntax* papcExplicit, CCPPSyntaxCallExpression* pcCall)
{
	CArrayCPP65Templates	apsTaken;
	CArrayCPP65Templates	apsBest;
	CArrayCPP65Templates*	papsChosen;
	SCPP65Template*			psTemplate;
	CArrayInt				aiRanks;
	size					uiArguments;
	size					uiBest;
	size					i;

	apsTaken.Init();
	apsBest.Init();
	aiRanks.Init();
	uiArguments = pcCall ? pcCall->macpArguments.NumElements() : 0;
	for (i = 0; pcCall && (papsCandidates->NumElements() > 1) && (i < papsCandidates->NumElements()); i++)
	{
		psTemplate = papsCandidates->GetPtr(i);
		aiRanks.Resize(aiRanks.NumElements() + uiArguments);
		if (TemplateTakes(psTemplate, papcExplicit, &pcCall->macpArguments, pcCall, (uiArguments > 0) ? aiRanks.Get(aiRanks.NumElements() - uiArguments) : NULL))
		{
			apsTaken.Add(psTemplate);
		}
		else
		{
			aiRanks.Resize(aiRanks.NumElements() - uiArguments);
		}
	}

	//Only the templates whose arguments no other's convert better.
	for (i = 0; (uiArguments > 0) && (i < apsTaken.NumElements()); i++)
	{
		for (uiBest = 0; uiBest < apsTaken.NumElements(); uiBest++)
		{
			if (CompareRanks(aiRanks.Get(uiBest * uiArguments), aiRanks.Get(i * uiArguments), uiArguments) > 0)
			{
				break;
			}
		}
		if (uiBest == apsTaken.NumElements())
		{
			apsBest.Add(apsTaken.GetPtr(i));
		}
	}
	if (apsBest.NumElements() > 0)
	{
		papsChosen = &apsBest;
	}
	else
	{
		papsChosen = (apsTaken.NumElements() > 0) ? &apsTaken : papsCandidates;
	}

	psTemplate = (papsChosen->NumElements() > 0) ? papsChosen->GetPtr(0) : NULL;
	for (i = 0; (iConstObject >= 0) && (i < papsChosen->NumElements()); i++)
	{
		if (IsConstSuffix(papsChosen->GetPtr(i)->pcDeclarator->GetFunctionSuffix()) == (iConstObject == 1))
		{
			psTemplate = papsChosen->GetPtr(i);
			break;
		}
	}
	aiRanks.Kill();
	apsBest.Kill();
	apsTaken.Kill();
	return psTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call with papcArguments (and the explicit template arguments
// papcExplicit) can use psTemplate: its parameters can be deduced and bound, each
// parameter whose argument is a pointer to a member function is of that
// argument's type, and every other argument converts to its parameter.  aiRanks
// (one per argument, may be NULL) gets each conversion's rank (ConversionRank).
// Nothing is reported; a template the call can't use is passed over.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::TemplateTakes(SCPP65Template* psTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int* aiRanks)
{
	CArrayCPP65Types			apsArguments;
	CArrayCPP65Types			apsDeduced;
	CArrayInt64					alliDeduced;
	CChars						szArguments;
	SCPP65Bindings*				psBindings;
	SCPP65Bindings*				psSaved;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	SCPP65Type*					psArgument;
	SCPP65Type*					psParameter;
	bool						bQuietFailed;
	bool						bTakes;
	bool						bPack;
	int							iRank;
	size						i;

	for (i = 0; aiRanks && (i < papcArguments->NumElements()); i++)
	{
		aiRanks[i] = CPP65CR_Exact;
	}

	//A failure here is not one of the quiet code around it.
	bQuietFailed = mbQuietFailed;
	BeginQuiet();
	apsArguments.Init();
	psBindings = NULL;
	bTakes = !psTemplate->psClass || DefineMemberTemplate(psTemplate, pcNode);
	if (bTakes)
	{
		for (i = 0; i < papcArguments->NumElements(); i++)
		{
			apsArguments.Add(TypeOf(papcArguments->GetPtr(i)));
		}
		apsDeduced.Init();
		alliDeduced.Init();
		DeduceArguments(psTemplate, &apsArguments, papcArguments, &apsDeduced, &alliDeduced);
		szArguments.Init();
		psBindings = BindArguments(psTemplate->pcDeclaration, papcExplicit, &apsDeduced, &szArguments, pcNode, &alliDeduced);
		szArguments.Kill();
		alliDeduced.Kill();
		apsDeduced.Kill();
		bTakes = psBindings != NULL;
	}

	if (bTakes)
	{
		//The parameters' types are worked out as InstantiateFunction declares them.
		psSaved = mpsBindings;
		if (psTemplate->psClass)
		{
			psBindings->psOuter = psTemplate->pcClassDeclaration ? RenameBindings(psTemplate->psClass, psTemplate->pcClassDeclaration) : psTemplate->psClass->psBindings;
		}
		else
		{
			psBindings->psOuter = mpsBindings;
		}
		mpsBindings = psBindings;
		pcSuffix = ((CCPPSyntaxFunctionDefinition*)psTemplate->pcDefinition)->mpcDeclarator->GetFunctionSuffix();
		bPack = false;
		for (i = 0; bTakes && !bPack && pcSuffix && (i < pcSuffix->macpParameters.NumElements()) && (i < apsArguments.NumElements()); i++)
		{
			pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
			bPack = pcParameter->mpcDeclarator && pcParameter->mpcDeclarator->mbPack;
			psArgument = apsArguments.GetPtr(i);
			if (bPack || !psArgument)
			{
				continue;
			}
			psParameter = GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcNode);
			psArgument = psArgument->WithoutReference();
			if (psArgument->IsMemberPointer() && psArgument->psBase && psArgument->psBase->IsFunction())
			{
				bTakes = psParameter && psParameter->Equals(psArgument);
			}
			else if (psParameter)
			{
				iRank = ConversionRank(psParameter, apsArguments.GetPtr(i), papcArguments->GetPtr(i));
				bTakes = iRank != CPP65CR_None;
				if (aiRanks)
				{
					aiRanks[i] = iRank;
				}
			}
		}
		mpsBindings = psSaved;
		if (!psTemplate->psClass)
		{
			psBindings->psOuter = NULL;
		}
	}
	apsArguments.Kill();
	bTakes = EndQuiet() && bTakes;
	mbQuietFailed = bQuietFailed;
	return bTakes;
}


//////////////////////////////////////////////////////////////////////////
//
// A member template declared in its class but defined outside it takes the outside
// definition with its name and parameters (and its template parameters, which the
// definition uses): the one whose parameters are written the same, as overloads
// that differ only in their parameters' types are, or else the first with as many.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::DefineMemberTemplate(SCPP65Template* psTemplate, CCPPSyntax* pcNode)
{
	SCPP65Template*				psOutside;
	SCPP65Template*				psFound;
	CCPPSymbol*					pcClassSymbol;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxFunctionSuffix*	pcOutsideSuffix;
	CCPPSymbolKey				cKey;
	CChars						szSignature;
	CChars						szOutside;
	size						i;
	bool						bSame;

	if (psTemplate->pcDefinition)
	{
		return true;
	}
	pcClassSymbol = psTemplate->psClass->psTemplate ? psTemplate->psClass->psTemplate->pcSymbol : psTemplate->psClass->pcSymbol;
	pcSuffix = psTemplate->pcDeclarator->GetFunctionSuffix();
	cKey.Init(mpcTokeniser, mpcTree);
	szSignature.Init();
	if (pcSuffix)
	{
		cKey.GetSignature(pcSuffix, &szSignature);
	}
	psFound = NULL;
	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		psOutside = mapsTemplates.GetPtr(i);
		if (!psOutside->bMemberTemplate || psOutside->psClass || (psOutside->pcSymbol != pcClassSymbol) || (psOutside->lliName != psTemplate->lliName))
		{
			continue;
		}
		pcOutsideSuffix = ((CCPPSyntaxFunctionDefinition*)psOutside->pcDefinition)->mpcDeclarator->GetFunctionSuffix();
		if ((pcOutsideSuffix ? pcOutsideSuffix->NumParameters() : 0) != (pcSuffix ? pcSuffix->NumParameters() : 0))
		{
			continue;
		}
		psFound = psFound ? psFound : psOutside;
		szOutside.Init();
		if (pcSuffix && pcOutsideSuffix)
		{
			cKey.GetSignature(pcOutsideSuffix, &szOutside);
		}
		bSame = pcSuffix && pcOutsideSuffix && szOutside.Equals(&szSignature);
		szOutside.Kill();
		if (bSame)
		{
			psFound = psOutside;
			break;
		}
	}
	szSignature.Kill();
	cKey.Kill();

	if (!psFound)
	{
		return Error(pcNode, "The member template has not been defined");
	}
	psTemplate->pcDeclaration = psFound->pcDeclaration;
	psTemplate->pcDefinition = psFound->pcDefinition;
	psTemplate->pcClassDeclaration = psFound->pcClassDeclaration;
	return true;
}
