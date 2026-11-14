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
bool CCPP65Generator::TemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration)
{
	CCPPTemplate*					pcTemplate;
	CCPPSyntax*						pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPSyntaxTemplateDeclaration*	pcClassTemplate;
	CCPPSyntaxClassSpecifier*		pcSpecifier;
	CCPPSyntaxName*					pcName;
	CCPPSyntaxNameComponent*		pcClassName;
	CCPPTemplate*					pcOwnerTemplate;
	bool							bExplicit;

	pcDeclaration = pcTemplateDeclaration->mpcDeclaration;
	if (pcTemplateDeclaration->mbInstantiation && !pcTemplateDeclaration->mbExtern && pcDeclaration)
	{
		return ExplicitInstantiation(pcDeclaration);
	}
	if (pcTemplateDeclaration->mbInstantiation || !pcDeclaration)
	{
		return true;
	}
	bExplicit = pcTemplateDeclaration->macpParameters.NumElements() == 0;

	//template<class M> template<class N> N SBox<M>::Mul(N n) { ... }
	pcClassTemplate = NULL;
	if ((pcDeclaration->GetKind() == CPPS_TemplateDeclaration) && !bExplicit)
	{
		pcClassTemplate = pcTemplateDeclaration;
		pcTemplateDeclaration = (CCPPSyntaxTemplateDeclaration*)pcDeclaration;
		pcDeclaration = pcTemplateDeclaration->mpcDeclaration;
		if (pcTemplateDeclaration->mbInstantiation || (pcTemplateDeclaration->macpParameters.NumElements() == 0) || !pcDeclaration || (pcDeclaration->GetKind() != CPPS_FunctionDefinition))
		{
			return true;
		}
	}

	pcTemplate = NULL;
	if ((pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && !pcClassTemplate)
	{
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
		if (pcSimple->mpcSpecifiers && pcSimple->mpcSpecifiers->mpcType && (pcSimple->mpcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
		{
			//template<class T> struct SAlign<T*> { ... } and template<> struct SAlign<char> { ... } specialise SAlign.
			pcSpecifier = (CCPPSyntaxClassSpecifier*)pcSimple->mpcSpecifiers->mpcType;
			pcClassName = pcSpecifier->mpcName ? pcSpecifier->mpcName->GetLast() : NULL;
			if (pcClassName && pcClassName->mbTemplateArguments)
			{
				Specialisation(pcSpecifier->mpcSymbol, pcTemplateDeclaration, pcSpecifier, pcClassName);
			}
			else if (!bExplicit)
			{
				NewTemplate(pcSpecifier->mpcSymbol, pcTemplateDeclaration, pcSpecifier, false);
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
			pcClassName = pcName->GetQualifier();
			pcOwnerTemplate = GetComponentTemplate(pcClassName);
			if (pcOwnerTemplate && pcClassTemplate)
			{
				pcTemplate = NewTemplate(pcOwnerTemplate->mpcSymbol, pcTemplateDeclaration, pcFunction, true);
				pcTemplate->mbMemberTemplate = true;
				pcTemplate->mpcClassDeclaration = pcClassTemplate;
			}
			else if (pcOwnerTemplate)
			{
				pcTemplate = NewTemplate(pcOwnerTemplate->mpcSymbol, pcTemplateDeclaration, pcFunction, true);
				pcTemplate->mpcOwner = OutsideMemberOwner(pcOwnerTemplate, pcClassName, pcTemplateDeclaration);
			}
			else if (pcClassName->mpcSymbol && (pcClassName->mpcSymbol->meKind == CPPSK_Class))
			{
				//template<class T> int CValue::Size(T* p) { ... }
				pcTemplate = NewTemplate(pcClassName->mpcSymbol, pcTemplateDeclaration, pcFunction, true);
				pcTemplate->mbMemberTemplate = true;
			}
			if (pcTemplate && pcTemplate->mbMemberTemplate)
			{
				pcTemplate->mlliName = GetFunctionName(pcName->GetLast());
			}
		}
		else if (pcName && (pcName->macpComponents.NumElements() == 1) && bExplicit && !pcClassTemplate)
		{
			//template<> int Size<char>(char c) { ... } and template<> int Size(short s) { ... }
			Specialisation(pcName->mpcSymbol, pcTemplateDeclaration, pcFunction, pcName->GetLast());
		}
		else if (pcName && !pcClassTemplate && !bExplicit && (pcName->mpcSymbol || (pcName->GetLast()->meKind == CPPNC_Operator)))
		{
			//An operator template has no symbol; it is found by its operator (FindFreeOperator).
			NewTemplate(pcName->mpcSymbol, pcTemplateDeclaration, pcFunction, false);
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
CCPPTemplate* CCPP65Generator::Specialisation(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, CCPPSyntaxNameComponent* pcPattern)
{
	CCPPTemplate*	pcPrimary;
	CCPPTemplate*	pcTemplate;
	int64			lliName;

	//template<> int Size(short s) names a new function; its template is found by name.
	pcPrimary = pcSymbol ? GetTemplate(pcSymbol) : NULL;
	lliName = pcPattern ? GetComponentName(pcPattern) : -1;
	if (!pcPrimary && (lliName != -1))
	{
		pcPrimary = GetTemplateByName(lliName, pcDefinition->GetKind(), ComponentLookup(pcPattern));
	}
	if (!pcPrimary || (pcPrimary->mpcDefinition->GetKind() != pcDefinition->GetKind()))
	{
		return NULL;
	}
	pcTemplate = NewTemplate(pcPrimary->mpcSymbol, pcDeclaration, pcDefinition, false);
	pcTemplate->mpcPrimary = pcPrimary;
	pcTemplate->mpcPattern = pcPattern;
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// The class template a member function defined outside it belongs to: the primary
// when the class's arguments are the declaration's parameters in order
// (CArrayTemplate<M>), else the partial specialisation with the same arguments
// written the same way (SAlign<T*>).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPP65Generator::OutsideMemberOwner(CCPPTemplate* pcPrimary, CCPPSyntaxNameComponent* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	CCPPTemplate*		pcTemplate;
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
		return pcPrimary;
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
		return pcPrimary;
	}

	szClass.Init();
	pcClass->PrettyPrint(&szClass, 0);
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if ((pcTemplate->mpcPrimary != pcPrimary) || (pcTemplate == pcPrimary) || !pcTemplate->mpcPattern)
		{
			continue;
		}
		szPattern.Init();
		pcTemplate->mpcPattern->PrettyPrint(&szPattern, 0);
		if (szPattern.Equals(&szClass))
		{
			szPattern.Kill();
			szClass.Kill();
			return pcTemplate;
		}
		szPattern.Kill();
	}
	szClass.Kill();
	return pcPrimary;
}


//////////////////////////////////////////////////////////////////////////
//
// A new template, kept until the generator is killed.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPP65Generator::NewTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember)
{
	CCPPTemplate*	pcTemplate;

	pcTemplate = NewMalloc<CCPPTemplate>();
	pcTemplate->Init(pcSymbol, pcDeclaration, pcDefinition, bMember, mcContext.GetNamespace());
	mapcTemplates.Add(pcTemplate);
	return pcTemplate;
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
	CCPPClass*					pcClass;
	CCPPFunction*					pcFunction;
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
	pcClass = GetNameClass(pcName, pcDeclaration);
	if (!pcClass || !CompleteClass(pcClass, pcDeclaration))
	{
		return false;
	}
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if (pcFunction->mpcDefinition || pcFunction->mbImplicit)
		{
			RequireFunction(pcFunction);
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
CCPPTemplate* CCPP65Generator::GetTemplate(CCPPSymbol* pcSymbol)
{
	CCPPTemplate*	pcTemplate;
	CCPPSymbol*		pcDeclaration;
	size			i;

	for (pcDeclaration = pcSymbol; pcDeclaration; pcDeclaration = pcDeclaration->mpcNext)
	{
		if ((pcDeclaration != pcSymbol) && (pcDeclaration->meKind != pcSymbol->meKind))
		{
			continue;
		}
		for (i = 0; i < mapcTemplates.NumElements(); i++)
		{
			pcTemplate = mapcTemplates.GetPtr(i);
			if (!pcTemplate->mbMember && (pcTemplate->mpcPrimary == pcTemplate) && (pcTemplate->mpcSymbol == pcDeclaration))
			{
				return pcTemplate;
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
CCPPTemplate* CCPP65Generator::GetTemplateByName(int64 lliName, ECPPSyntax eDefinition, CCPPScope* pcLookup)
{
	CCPPTemplate*	pcTemplate;
	CCPPTemplate*	pcFound;
	size			i;

	pcFound = NULL;
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (!pcTemplate->mbMember && (pcTemplate->mpcPrimary == pcTemplate) && pcTemplate->mpcSymbol && (pcTemplate->mpcSymbol->mlliName == lliName) && (pcTemplate->mpcDefinition->GetKind() == eDefinition))
		{
			if (pcTemplate->mpcSymbol->mpcScope->Encloses(pcLookup))
			{
				return pcTemplate;
			}
			pcFound = pcFound ? pcFound : pcTemplate;
		}
	}
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
//
// The class template a name's component names.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPP65Generator::GetComponentTemplate(CCPPSyntaxNameComponent* pcComponent)
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
CCPPTemplateArguments* CCPP65Generator::BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode, CArrayInt64* palliDeduced)
{
	CCPPTemplateArguments*			pcBindings;
	SCPPBinding*			psBinding;
	CCPPSyntax*				pcParameter;
	CCPPSyntax*				pcArgument;
	CCPPSyntaxTypeParameter*	pcType;
	CCPPSyntaxParameter*	pcValue;
	SCPP65Type*				psType;
	size					i;
	size					j;
	bool					bResult;

	pcBindings = NewBindings();
	bResult = true;
	for (i = 0; i < pcTemplate->macpParameters.NumElements(); i++)
	{
		pcParameter = pcTemplate->macpParameters.GetPtr(i);
		pcArgument = (papcArguments && (i < papcArguments->NumElements())) ? papcArguments->GetPtr(i) : NULL;
		psBinding = pcBindings->macBindings.Add();
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
					psType = (pcArgument->GetKind() == CPPS_TypeId) ? mcDeclarators.GetTypeIdType((CCPPSyntaxTypeId*)pcArgument) : NULL;
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
					mcMangler.AppendType(pszArguments, psBinding->papsPack->GetPtr(j));
				}
				pszArguments->Append('E');
				break;
			}
			if (pcArgument && (pcArgument->GetKind() == CPPS_TypeId))
			{
				//const int is a different argument to int (its type is const).
				psBinding->psType = mcDeclarators.GetTypeIdType((CCPPSyntaxTypeId*)pcArgument);
			}
			else if (!pcArgument && papsDeduced && (i < papsDeduced->NumElements()) && papsDeduced->GetPtr(i))
			{
				psBinding->psType = papsDeduced->GetPtr(i);
			}
			else if (!pcArgument && pcType->mpcDefault)
			{
				//A default can use the parameters before it.
				pcBindings->mpcOuter = mcContext.GetBindings();
				mcContext.EnterBindings(pcBindings);
				psBinding->psType = mcDeclarators.GetTypeIdType(pcType->mpcDefault);
				mcContext.LeaveBindings();
				pcBindings->mpcOuter = NULL;
			}
			if (!psBinding->psType)
			{
				bResult = Error(pcArgument ? pcArgument : pcNode, "The template argument is not a type the compiler supports");
				break;
			}
			pszArguments->Append('_');
			mcMangler.AppendType(pszArguments, psBinding->psType);
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
	return bResult ? pcBindings : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a template parameter in what is being generated.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPP65Generator::FindBinding(CCPPSymbol* pcParameter)
{
	return mcContext.GetBindings() ? mcContext.GetBindings()->Find(pcParameter) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The binding of the template parameter pack a function parameter pack (Args ...
// args) is declared with, or NULL if the parameter is not a pack.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPP65Generator::PackBinding(CCPPSyntaxParameter* pcParameter)
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
// The instance of a class template for the arguments, made if it is new.  It is
// laid out when it is first needed whole, from the most specialised definition
// that matches the arguments.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPP65Generator::InstantiateClass(CCPPTemplate* pcTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPTemplateArguments*	pcSpecialised;
	CCPPTemplate*	pcSpecialisation;
	CCPPClass*	pcClass;
	CChars			szArguments;
	size			i;

	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, papcArguments, NULL, &szArguments, pcNode);
	if (!pcBindings)
	{
		szArguments.Kill();
		return NULL;
	}

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if (pcClass->mpcTemplate && (pcClass->mpcTemplate->mpcPrimary == pcTemplate) && pcClass->mszArguments.Equals(&szArguments))
		{
			szArguments.Kill();
			return pcClass;
		}
	}

	pcSpecialisation = FindClassSpecialisation(pcTemplate, pcBindings, &pcSpecialised, pcNode);
	if (pcSpecialisation)
	{
		pcTemplate = pcSpecialisation;
		pcBindings = pcSpecialised;
	}

	//An instance used before the template was defined (UndefinedInstance) becomes this instance.
	pcClass = NULL;
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		if (mapcClasses.GetPtr(i)->IsInstanceNamed(pcTemplate->mpcSymbol) && mapcClasses.GetPtr(i)->mszArguments.Equals(&szArguments))
		{
			pcClass = mapcClasses.GetPtr(i);
			pcClass->mpcSymbol = pcTemplate->mpcSymbol;
			pcClass->mszArguments.Kill();
			pcClass->mszArguments.Init();
			break;
		}
	}
	if (!pcClass)
	{
		pcClass = NewClass(pcTemplate->mpcSymbol, pcTemplate->mpcSymbol->mlliName);
	}
	pcClass->mpcTemplate = pcTemplate;
	pcClass->mpcSpecifier = (CCPPSyntaxClassSpecifier*)pcTemplate->mpcDefinition;
	pcClass->mpcBindings = pcBindings;
	pcBindings->mpcClass = pcClass;
	pcClass->mszArguments.Append(&szArguments);
	szArguments.Kill();
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// An instance of a class template that has only been declared: an incomplete class (it can be pointed to) with the
// instance's name.  It becomes the instance if the template is defined later (InstantiateClass).
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPP65Generator::UndefinedInstance(CCPPSymbol* pcSymbol, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPSyntax*		pcDeclaration;
	CCPPTemplateArguments*	pcBindings;
	CCPPClass*	pcClass;
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
	pcBindings = BindArguments((CCPPSyntaxTemplateDeclaration*)pcDeclaration, papcArguments, NULL, &szArguments, pcNode);
	if (!pcBindings)
	{
		szArguments.Kill();
		return NULL;
	}
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if (pcClass->IsInstanceNamed(pcSymbol) && pcClass->mszArguments.Equals(&szArguments))
		{
			szArguments.Kill();
			return pcClass;
		}
	}
	pcClass = NewClass(pcSymbol, pcSymbol->mlliName);
	pcClass->mszArguments.Append(&szArguments);
	szArguments.Kill();
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The most specialised specialisation of a class template that matches the
// arguments (psArguments), with its own parameters bound in *ppsBindings.  An
// explicit specialisation is chosen first; of the partial ones, the one whose
// arguments match more of the types' structure.  NULL if none matches.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPP65Generator::FindClassSpecialisation(CCPPTemplate* pcPrimary, CCPPTemplateArguments* pcArguments, CCPPTemplateArguments** ppcBindings, CCPPSyntax* pcNode)
{
	CCPPSpecialisationMatch	cMatch;
	CCPPTemplate*			pcTemplate;
	CCPPTemplate*			pcBest;
	CCPPTemplateArguments*	pcDeduced;
	CCPPTemplateArguments*	pcBestDeduced;
	size					i;
	int						iScore;
	int						iBest;
	bool					bMatches;
	bool					bAmbiguous;

	pcBest = NULL;
	pcBestDeduced = NULL;
	iBest = -1;
	bAmbiguous = false;
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if ((pcTemplate->mpcPrimary != pcPrimary) || (pcTemplate == pcPrimary) || !pcTemplate->mpcPattern)
		{
			continue;
		}

		cMatch.Init(this, pcTemplate);
		bMatches = cMatch.Match(pcArguments);
		pcDeduced = cMatch.GetDeduced();
		iScore = cMatch.GetScore();
		cMatch.Kill();
		if (!bMatches)
		{
			continue;
		}
		if (iScore > iBest)
		{
			pcBest = pcTemplate;
			pcBestDeduced = pcDeduced;
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
	*ppcBindings = pcBestDeduced;
	return pcBest;
}


//////////////////////////////////////////////////////////////////////////
//
// New empty bindings, kept until the generator is killed.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPP65Generator::NewBindings(void)
{
	CCPPTemplateArguments*	pcBindings;

	pcBindings = NewMalloc<CCPPTemplateArguments>();
	pcBindings->Init();
	mapcAllBindings.Add(pcBindings);
	return pcBindings;
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
CCPPFunction* CCPP65Generator::InstantiateFunction(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPTemplate*					pcSpecialisation;
	CCPPTemplateArguments*					pcBindings;
	CCPPFunction*					pcFunction;
	CArrayCPP65Types				apsArguments;
	CCPPDeduction				cDeduction;
	CChars							szArguments;
	CCPPSyntaxFunctionSuffix*		pcInClass;
	CCPPSyntaxParameter*			pcParameter;
	size							i;

	if (pcTemplate->mpcClass && !DefineMemberTemplate(pcTemplate, pcNode))
	{
		return NULL;
	}
	pcDefinition = (CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition;

	apsArguments.Init();
	for (i = 0; papcArguments && (i < papcArguments->NumElements()); i++)
	{
		apsArguments.Add(TypeOf(papcArguments->GetPtr(i)));
	}
	cDeduction.Init(this, pcTemplate);
	cDeduction.Deduce(&apsArguments, papcArguments);
	apsArguments.Kill();

	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, papcExplicit, cDeduction.GetTypes(), &szArguments, pcNode, cDeduction.GetValues());
	cDeduction.Kill();
	if (!pcBindings)
	{
		szArguments.Kill();
		return NULL;
	}

	for (i = 0; i < pcTemplate->mapcInstances.NumElements(); i++)
	{
		pcFunction = pcTemplate->mapcInstances.GetPtr(i);
		if (pcFunction->mszArguments.Equals(&szArguments))
		{
			szArguments.Kill();
			return pcFunction;
		}
	}

	pcSpecialisation = FindFunctionSpecialisation(pcTemplate, &szArguments, pcNode);
	if (pcSpecialisation)
	{
		pcDefinition = (CCPPSyntaxFunctionDefinition*)pcSpecialisation->mpcDefinition;
	}

	//A member template's instance also sees its class's template arguments.
	if (pcTemplate->mpcClass)
	{
		pcBindings->mpcOuter = pcTemplate->mpcClassDeclaration ? RenameBindings(pcTemplate->mpcClass, pcTemplate->mpcClassDeclaration) : pcTemplate->mpcClass->mpcBindings;
	}
	else
	{
		pcBindings->mpcOuter = mcContext.GetBindings();
	}
	mcContext.EnterBindings(pcBindings);
	mcContext.EnterNamespace(pcTemplate->mpcNamespace);
	pcFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, pcTemplate->mpcClass, false, pcTemplate, &szArguments);
	mcContext.LeaveNamespace();
	mcContext.LeaveBindings();
	if (!pcTemplate->mpcClass)
	{
		pcBindings->mpcOuter = NULL;
	}
	szArguments.Kill();
	if (pcFunction)
	{
		pcFunction->mpcDefinition = pcDefinition;
		pcFunction->mpcBindings = pcBindings;
		pcFunction->mbInline = true;
		pcFunction->mbStatic |= pcTemplate->mbStatic;

		//A member template defined outside its class has its default arguments in the class.  [dcl.fct.default]
		pcInClass = (pcTemplate->mpcDeclarator && (pcTemplate->mpcDeclarator != pcDefinition->mpcDeclarator)) ? pcTemplate->mpcDeclarator->GetFunctionSuffix() : NULL;
		if (pcInClass && (pcInClass->macpParameters.NumElements() == pcFunction->mapcDefaults.NumElements()))
		{
			for (i = 0; i < pcInClass->macpParameters.NumElements(); i++)
			{
				pcParameter = (CCPPSyntaxParameter*)pcInClass->macpParameters.GetPtr(i);
				if (!pcFunction->mapcDefaults.GetPtr(i) && pcParameter->mpcDefault)
				{
					pcFunction->mapcDefaults.SetPtr(i, pcParameter->mpcDefault);
				}
			}
		}
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The explicit specialisation of a function template for the mangled arguments,
// or NULL.  A specialisation's arguments are its explicit ones (Size<char>) and
// those deduced from its parameters' types (Size(short)), worked out once.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPP65Generator::FindFunctionSpecialisation(CCPPTemplate* pcPrimary, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPTemplate*				pcTemplate;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	SCPP65Type*					psType;
	CArrayCPP65Types			apsParameters;
	CCPPDeduction				cDeduction;
	size						i;
	size						j;

	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if ((pcTemplate->mpcPrimary != pcPrimary) || (pcTemplate == pcPrimary) || (pcTemplate->mpcDefinition->GetKind() != CPPS_FunctionDefinition))
		{
			continue;
		}
		if (!pcTemplate->mbArguments)
		{
			pcTemplate->mbArguments = true;
			mcContext.EnterBindings(NULL);
			BeginQuiet();
			apsParameters.Init();
			pcSuffix = ((CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
			for (j = 0; pcSuffix && (j < pcSuffix->macpParameters.NumElements()); j++)
			{
				pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(j);
				psType = mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
				apsParameters.Add((psType && psType->IsReference()) ? psType->psBase : psType);
			}
			cDeduction.Init(this, pcPrimary);
			cDeduction.Deduce(&apsParameters, NULL);
			if (!BindArguments(pcPrimary->mpcDeclaration, (pcTemplate->mpcPattern && pcTemplate->mpcPattern->mbTemplateArguments) ? &pcTemplate->mpcPattern->macpTemplateArguments : NULL, cDeduction.GetTypes(), &pcTemplate->mszArguments, pcNode, cDeduction.GetValues()))
			{
				pcTemplate->mszArguments.Clear();
			}
			cDeduction.Kill();
			apsParameters.Kill();
			EndQuiet();
			mcContext.LeaveBindings();
		}
		if (!pcTemplate->mszArguments.Empty() && pcTemplate->mszArguments.Equals(pszArguments))
		{
			return pcTemplate;
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
CCPPTemplateArguments* CCPP65Generator::RenameBindings(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPSyntax*		pcParameter;
	size			i;

	pcBindings = NewBindings();
	pcBindings->mpcOuter = pcClass->mpcBindings;
	pcBindings->mpcClass = pcClass;
	for (i = 0; (i < pcDeclaration->macpParameters.NumElements()) && (i < pcClass->mpcBindings->macBindings.NumElements()); i++)
	{
		pcParameter = pcDeclaration->macpParameters.GetPtr(i);
		*pcBindings->macBindings.Add() = *pcClass->mpcBindings->macBindings.Get(i);
		pcBindings->macBindings.Tail()->pcParameter = (pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol;
	}
	return pcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps a member template of a class being laid out.  Its instances are made when
// it is called (InstantiateFunction).  Member class templates, friend templates and
// constructor templates are left out.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::MemberTemplate(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration)
{
	CCPPSyntax*						pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxName*					pcName;
	CCPPTemplate*					pcTemplate;
	int64							lliName;

	pcDeclaration = pcTemplateDeclaration->mpcDeclaration;
	if (pcTemplateDeclaration->mbInstantiation || (pcTemplateDeclaration->macpParameters.NumElements() == 0) || !pcDeclaration)
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
	if ((lliName == -1) || (lliName == pcClass->mlliName) || (pcName->GetLast()->meKind == CPPNC_Destructor))
	{
		return true;
	}

	pcTemplate = NewTemplate(pcName->mpcSymbol, pcTemplateDeclaration, (pcDeclaration->GetKind() == CPPS_FunctionDefinition) ? pcDeclaration : NULL, true);
	pcTemplate->mbMemberTemplate = true;
	pcTemplate->mpcClass = pcClass;
	pcTemplate->mlliName = lliName;
	pcTemplate->mpcDeclarator = pcDeclarator;
	pcTemplate->mbStatic = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_STATIC);
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
// A member template of pcClass (or of a base) named lliName that takes uiArguments
// arguments.  Of two that do, the one whose const matches the object's (iConstObject
// 1 for a const object, 0 for one that is not, -1 for none) is chosen.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPP65Generator::FindMemberTemplate(CCPPClass* pcClass, int64 lliName, size uiArguments, int iConstObject)
{
	CArrayCPPTemplates	apcCandidates;
	CCPPTemplate*			pcTemplate;

	mpcStatistics->Count(CPPCC_FindMemberTemplate);
	apcCandidates.Init();
	GetMemberTemplates(pcClass, lliName, uiArguments, &apcCandidates);
	pcTemplate = ChooseTemplate(&apcCandidates, iConstObject, NULL, NULL);
	apcCandidates.Kill();
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// The member template of pcClass (or of a base) named lliName that pcCall, with
// the explicit template arguments papcExplicit, uses (ChooseTemplate).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPP65Generator::ChooseMemberTemplate(CCPPClass* pcClass, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcExplicit, int iConstObject)
{
	CArrayCPPTemplates	apcCandidates;
	CCPPTemplate*			pcTemplate;

	mpcStatistics->Count(CPPCC_ChooseMemberTemplate);
	apcCandidates.Init();
	GetMemberTemplates(pcClass, lliName, pcCall->macpArguments.NumElements(), &apcCandidates);
	pcTemplate = ChooseTemplate(&apcCandidates, iConstObject, papcExplicit, pcCall);
	apcCandidates.Kill();
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds pcClass's member templates named lliName that take uiArguments arguments,
// or else those of its first base that has any.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::GetMemberTemplates(CCPPClass* pcClass, int64 lliName, size uiArguments, CArrayCPPTemplates* papcCandidates)
{
	CCPPTemplate*				pcTemplate;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						i;

	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (!pcTemplate->mbMemberTemplate || (pcTemplate->mpcClass != pcClass) || (pcTemplate->mlliName != lliName))
		{
			continue;
		}
		pcSuffix = pcTemplate->mpcDeclarator->GetFunctionSuffix();
		if (pcSuffix && TakesArguments(pcSuffix, uiArguments))
		{
			papcCandidates->Add(pcTemplate);
		}
	}

	for (i = 0; (papcCandidates->NumElements() == 0) && (i < pcClass->macBases.NumElements()); i++)
	{
		GetMemberTemplates(pcClass->macBases.Get(i)->pcClass, lliName, uiArguments, papcCandidates);
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
CCPPTemplate* CCPP65Generator::GetCallTemplate(CCPPSymbol* pcSymbol, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcExplicit)
{
	CArrayCPPTemplates	apcCandidates;
	CArrayScopes			apcNamespaces;
	CCPPTemplate*			pcTemplate;
	size					i;

	mpcStatistics->Count(CPPCC_GetCallTemplate);
	apcCandidates.Init();
	apcNamespaces.Init();
	if (pcCall->mpcCallee->GetKind() == CPPS_IdExpression)
	{
		GetCallNamespaces(((CCPPSyntaxIdExpression*)pcCall->mpcCallee)->mpcName, lliName, &pcCall->macpArguments, &apcNamespaces);
	}
	for (i = 0; i < apcNamespaces.NumElements(); i++)
	{
		AddCallTemplates(apcNamespaces.GetPtr(i)->FindHere(lliName), pcCall, &apcCandidates);
	}
	if (apcNamespaces.NumElements() == 0)
	{
		AddCallTemplates(pcSymbol, pcCall, &apcCandidates);
	}
	pcTemplate = NULL;
	if (apcCandidates.NumElements() > 0)
	{
		pcTemplate = ChooseTemplate(&apcCandidates, -1, papcExplicit, pcCall);
	}
	else if (pcSymbol)
	{
		pcTemplate = GetTemplate(pcSymbol);
	}
	apcNamespaces.Kill();
	apcCandidates.Kill();
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the function templates of pcSymbol's declarations that take the call's
// number of arguments.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddCallTemplates(CCPPSymbol* pcSymbol, CCPPSyntaxCallExpression* pcCall, CArrayCPPTemplates* papcCandidates)
{
	CCPPTemplate*				pcTemplate;
	CCPPSymbol*					pcDeclaration;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						i;

	for (pcDeclaration = pcSymbol; pcDeclaration; pcDeclaration = pcDeclaration->mpcNext)
	{
		if (pcDeclaration->meKind != CPPSK_FunctionTemplate)
		{
			continue;
		}
		for (i = 0; i < mapcTemplates.NumElements(); i++)
		{
			pcTemplate = mapcTemplates.GetPtr(i);
			if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || (pcTemplate->mpcSymbol != pcDeclaration) || !pcTemplate->mpcDefinition || (pcTemplate->mpcDefinition->GetKind() != CPPS_FunctionDefinition))
			{
				continue;
			}
			pcSuffix = ((CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
			if (pcSuffix && TakesArguments(pcSuffix, pcCall->macpArguments.NumElements()))
			{
				papcCandidates->Add(pcTemplate);
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
CCPPTemplate* CCPP65Generator::ChooseTemplate(CArrayCPPTemplates* papcCandidates, int iConstObject, CArraySyntax* papcExplicit, CCPPSyntaxCallExpression* pcCall)
{
	CArrayCPPTemplates	apcTaken;
	CArrayCPPTemplates	apcBest;
	CArrayCPPTemplates*	papcChosen;
	CCPPTemplate*			pcTemplate;
	CArrayInt				aiRanks;
	size					uiArguments;
	size					uiBest;
	size					i;

	mpcStatistics->Count(CPPCC_ChooseTemplate);
	apcTaken.Init();
	apcBest.Init();
	aiRanks.Init();
	uiArguments = pcCall ? pcCall->macpArguments.NumElements() : 0;
	for (i = 0; pcCall && (papcCandidates->NumElements() > 1) && (i < papcCandidates->NumElements()); i++)
	{
		pcTemplate = papcCandidates->GetPtr(i);
		aiRanks.Resize(aiRanks.NumElements() + uiArguments);
		if (TemplateTakes(pcTemplate, papcExplicit, &pcCall->macpArguments, pcCall, (uiArguments > 0) ? aiRanks.Get(aiRanks.NumElements() - uiArguments) : NULL))
		{
			apcTaken.Add(pcTemplate);
		}
		else
		{
			aiRanks.Resize(aiRanks.NumElements() - uiArguments);
		}
	}

	//Only the templates whose arguments no other's convert better.
	for (i = 0; (uiArguments > 0) && (i < apcTaken.NumElements()); i++)
	{
		for (uiBest = 0; uiBest < apcTaken.NumElements(); uiBest++)
		{
			if (CompareRanks(aiRanks.Get(uiBest * uiArguments), aiRanks.Get(i * uiArguments), uiArguments) > 0)
			{
				break;
			}
		}
		if (uiBest == apcTaken.NumElements())
		{
			apcBest.Add(apcTaken.GetPtr(i));
		}
	}
	if (apcBest.NumElements() > 0)
	{
		papcChosen = &apcBest;
	}
	else
	{
		papcChosen = (apcTaken.NumElements() > 0) ? &apcTaken : papcCandidates;
	}

	pcTemplate = (papcChosen->NumElements() > 0) ? papcChosen->GetPtr(0) : NULL;
	for (i = 0; (iConstObject >= 0) && (i < papcChosen->NumElements()); i++)
	{
		if (IsConstSuffix(papcChosen->GetPtr(i)->mpcDeclarator->GetFunctionSuffix()) == (iConstObject == 1))
		{
			pcTemplate = papcChosen->GetPtr(i);
			break;
		}
	}
	aiRanks.Kill();
	apcBest.Kill();
	apcTaken.Kill();
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call with papcArguments (and the explicit template arguments
// papcExplicit) can use pcTemplate: its parameters can be deduced and bound, each
// parameter whose argument is a pointer to a member function is of that
// argument's type, and every other argument converts to its parameter.  aiRanks
// (one per argument, may be NULL) gets each conversion's rank (ConversionRank).
// Nothing is reported; a template the call can't use is passed over.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::TemplateTakes(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int* aiRanks)
{
	CArrayCPP65Types			apsArguments;
	CCPPDeduction				cDeduction;
	CChars						szArguments;
	CCPPTemplateArguments*				pcBindings;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	SCPP65Type*					psArgument;
	SCPP65Type*					psParameter;
	bool						bTakes;
	bool						bPack;
	int							iRank;
	size						i;

	for (i = 0; aiRanks && (i < papcArguments->NumElements()); i++)
	{
		aiRanks[i] = CPPCR_Exact;
	}

	//A failure here is not one of the quiet code around it.
	BeginTrial();
	apsArguments.Init();
	pcBindings = NULL;
	bTakes = !pcTemplate->mpcClass || DefineMemberTemplate(pcTemplate, pcNode);
	if (bTakes)
	{
		for (i = 0; i < papcArguments->NumElements(); i++)
		{
			apsArguments.Add(TypeOf(papcArguments->GetPtr(i)));
		}
		cDeduction.Init(this, pcTemplate);
		cDeduction.Deduce(&apsArguments, papcArguments);
		szArguments.Init();
		pcBindings = BindArguments(pcTemplate->mpcDeclaration, papcExplicit, cDeduction.GetTypes(), &szArguments, pcNode, cDeduction.GetValues());
		szArguments.Kill();
		cDeduction.Kill();
		bTakes = pcBindings != NULL;
	}

	if (bTakes)
	{
		//The parameters' types are worked out as InstantiateFunction declares them.
		if (pcTemplate->mpcClass)
		{
			pcBindings->mpcOuter = pcTemplate->mpcClassDeclaration ? RenameBindings(pcTemplate->mpcClass, pcTemplate->mpcClassDeclaration) : pcTemplate->mpcClass->mpcBindings;
		}
		else
		{
			pcBindings->mpcOuter = mcContext.GetBindings();
		}
		mcContext.EnterBindings(pcBindings);
		pcSuffix = ((CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
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
			psParameter = mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcNode);
			psArgument = psArgument->WithoutReference();
			if (psArgument->IsMemberPointer() && psArgument->psBase && psArgument->psBase->IsFunction())
			{
				bTakes = psParameter && psParameter->Equals(psArgument);
			}
			else if (psParameter)
			{
				iRank = ConversionRank(psParameter, apsArguments.GetPtr(i), papcArguments->GetPtr(i));
				bTakes = iRank != CPPCR_None;
				if (aiRanks)
				{
					aiRanks[i] = iRank;
				}
			}
		}
		mcContext.LeaveBindings();
		if (!pcTemplate->mpcClass)
		{
			pcBindings->mpcOuter = NULL;
		}
	}
	apsArguments.Kill();
	bTakes = EndTrial() && bTakes;
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
bool CCPP65Generator::DefineMemberTemplate(CCPPTemplate* pcTemplate, CCPPSyntax* pcNode)
{
	CCPPTemplate*				pcOutside;
	CCPPTemplate*				pcFound;
	CCPPSymbol*					pcClassSymbol;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxFunctionSuffix*	pcOutsideSuffix;
	CCPPSymbolKey				cKey;
	CChars						szSignature;
	CChars						szOutside;
	size						i;
	bool						bSame;

	if (pcTemplate->mpcDefinition)
	{
		return true;
	}
	pcClassSymbol = pcTemplate->mpcClass->mpcTemplate ? pcTemplate->mpcClass->mpcTemplate->mpcSymbol : pcTemplate->mpcClass->mpcSymbol;
	pcSuffix = pcTemplate->mpcDeclarator->GetFunctionSuffix();
	cKey.Init(mpcTokeniser, mpcTree);
	szSignature.Init();
	if (pcSuffix)
	{
		cKey.GetSignature(pcSuffix, &szSignature);
	}
	pcFound = NULL;
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcOutside = mapcTemplates.GetPtr(i);
		if (!pcOutside->mbMemberTemplate || pcOutside->mpcClass || (pcOutside->mpcSymbol != pcClassSymbol) || (pcOutside->mlliName != pcTemplate->mlliName))
		{
			continue;
		}
		pcOutsideSuffix = ((CCPPSyntaxFunctionDefinition*)pcOutside->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
		if ((pcOutsideSuffix ? pcOutsideSuffix->NumParameters() : 0) != (pcSuffix ? pcSuffix->NumParameters() : 0))
		{
			continue;
		}
		pcFound = pcFound ? pcFound : pcOutside;
		szOutside.Init();
		if (pcSuffix && pcOutsideSuffix)
		{
			cKey.GetSignature(pcOutsideSuffix, &szOutside);
		}
		bSame = pcSuffix && pcOutsideSuffix && szOutside.Equals(&szSignature);
		szOutside.Kill();
		if (bSame)
		{
			pcFound = pcOutside;
			break;
		}
	}
	szSignature.Kill();
	cKey.Kill();

	if (!pcFound)
	{
		return Error(pcNode, "The member template has not been defined");
	}
	pcTemplate->mpcDeclaration = pcFound->mpcDeclaration;
	pcTemplate->mpcDefinition = pcFound->mpcDefinition;
	pcTemplate->mpcClassDeclaration = pcFound->mpcClassDeclaration;
	return true;
}
