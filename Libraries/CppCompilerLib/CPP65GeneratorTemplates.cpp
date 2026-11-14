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
	pcTemplate->Init(pcSymbol, pcDeclaration, pcDefinition, bMember, mpcDeclaringNamespace);
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
	CCPPTemplateArguments*			pcSaved;
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
				pcSaved = mpcBindings;
				pcBindings->mpcOuter = mpcBindings;
				mpcBindings = pcBindings;
				psBinding->psType = GetTypeIdType(pcType->mpcDefault);
				mpcBindings = pcSaved;
				pcBindings->mpcOuter = NULL;
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
	return bResult ? pcBindings : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a template parameter in what is being generated.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPP65Generator::FindBinding(CCPPSymbol* pcParameter)
{
	return mpcBindings ? mpcBindings->Find(pcParameter) : NULL;
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
	CCPPTemplate*	pcTemplate;
	CCPPTemplate*	pcBest;
	CCPPTemplateArguments*	pcDeduced;
	CCPPTemplateArguments*	pcBestDeduced;
	SCPPBinding*	psBinding;
	CCPPSyntax*		pcParameter;
	size			i;
	size			j;
	int				iScore;
	int				iBest;
	bool			bAmbiguous;

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

		//The specialisation's parameters, deduced as its arguments are matched.
		pcDeduced = NewBindings();
		for (j = 0; j < pcTemplate->mpcDeclaration->macpParameters.NumElements(); j++)
		{
			pcParameter = pcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
			psBinding = pcDeduced->macBindings.Add();
			psBinding->pcParameter = (pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol;
			psBinding->psType = NULL;
			psBinding->lliValue = 0;
			psBinding->papsPack = NULL;
		}

		iScore = (pcTemplate->mpcDeclaration->macpParameters.NumElements() == 0) ? 1000 : 0;
		if (!MatchPattern(pcTemplate, &pcTemplate->mpcPattern->macpTemplateArguments, pcArguments, pcDeduced, &iScore))
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
// Whether a specialisation's template arguments (papcPattern) match an instance's
// arguments.  Its parameters are deduced into psDeduced.  *piScore counts how much
// of the arguments' structure the pattern names.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::MatchPattern(CCPPTemplate* pcSpecialisation, CArraySyntax* papcPattern, CCPPTemplateArguments* pcArguments, CCPPTemplateArguments* pcDeduced, int* piScore)
{
	CCPPSyntax*		pcPattern;
	SCPPBinding*	psArgument;
	SCPPBinding*	psBinding;
	CCPPSymbol*		pcSymbol;
	size			i;
	int64			lliValue;

	if (papcPattern->NumElements() != pcArguments->macBindings.NumElements())
	{
		return false;
	}
	for (i = 0; i < papcPattern->NumElements(); i++)
	{
		pcPattern = papcPattern->GetPtr(i);
		psArgument = pcArguments->macBindings.Get(i);
		if (psArgument->psType)
		{
			if ((pcPattern->GetKind() != CPPS_TypeId) || !MatchType(pcSpecialisation, (CCPPSyntaxTypeId*)pcPattern, psArgument, pcDeduced, piScore))
			{
				return false;
			}
			continue;
		}

		//A non-type argument: a parameter of the specialisation takes it, a constant must equal it.
		pcSymbol = (pcPattern->GetKind() == CPPS_IdExpression) ? ((CCPPSyntaxIdExpression*)pcPattern)->mpcName->mpcSymbol : NULL;
		psBinding = pcSymbol ? pcDeduced->FindOwn(pcSymbol) : NULL;
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
	for (i = 0; i < pcDeduced->macBindings.NumElements(); i++)
	{
		pcSymbol = pcDeduced->macBindings.Get(i)->pcParameter;
		if (pcSymbol && (pcSymbol->meKind != CPPSK_ValueParameter) && !pcDeduced->macBindings.Get(i)->psType)
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
bool CCPP65Generator::MatchType(CCPPTemplate* pcSpecialisation, CCPPSyntaxTypeId* pcPattern, SCPPBinding* psArgument, CCPPTemplateArguments* pcDeduced, int* piScore)
{
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxPointerOperator*		pcOperator;
	CCPPSyntaxNamedTypeSpecifier*	pcNamed;
	CCPPSyntaxNameComponent*		pcComponent;
	SCPPBinding*					psBinding;
	CCPPTemplate*					pcTemplate;
	SCPP65Type*						psType;
	SCPP65Type*						psNamed;
	CCPPTemplateArguments*					pcSaved;
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
	psBinding = pcNamed ? pcDeduced->FindOwn(pcNamed->mpcName->mpcSymbol) : NULL;
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
		pcTemplate = pcComponent->mpcSymbol ? GetTemplate(pcComponent->mpcSymbol) : GetComponentTemplate(pcComponent);
		if (!pcTemplate || !psType->IsClass() || !psType->pcClass->mpcTemplate || (psType->pcClass->mpcTemplate->mpcPrimary != pcTemplate))
		{
			return false;
		}
		(*piScore)++;
		return MatchPattern(pcSpecialisation, &pcComponent->macpTemplateArguments, psType->pcClass->mpcBindings, pcDeduced, piScore);
	}

	//A type naming none of the parameters must be the same type.
	pcSaved = mpcBindings;
	mpcBindings = NULL;
	BeginQuiet();
	psNamed = GetTypeIdType(pcPattern);
	EndQuiet();
	mpcBindings = pcSaved;
	bResult = psNamed && psNamed->Same(psArgument->psType);
	*piScore += 2;
	return bResult;
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
	CCPPTemplateArguments*					pcSaved;
	CCPPFunction*					pcFunction;
	CArrayCPP65Types				apsArguments;
	CArrayCPP65Types				apsDeduced;
	CArrayInt64						alliDeduced;
	CChars							szArguments;
	CCPPScope*						pcSavedNamespace;
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
	apsDeduced.Init();
	alliDeduced.Init();
	DeduceArguments(pcTemplate, &apsArguments, papcArguments, &apsDeduced, &alliDeduced);
	apsArguments.Kill();

	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, papcExplicit, &apsDeduced, &szArguments, pcNode, &alliDeduced);
	alliDeduced.Kill();
	apsDeduced.Kill();
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
	pcSaved = mpcBindings;
	if (pcTemplate->mpcClass)
	{
		pcBindings->mpcOuter = pcTemplate->mpcClassDeclaration ? RenameBindings(pcTemplate->mpcClass, pcTemplate->mpcClassDeclaration) : pcTemplate->mpcClass->mpcBindings;
	}
	else
	{
		pcBindings->mpcOuter = mpcBindings;
	}
	mpcBindings = pcBindings;
	pcSavedNamespace = mpcDeclaringNamespace;
	mpcDeclaringNamespace = pcTemplate->mpcNamespace;
	pcFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, pcTemplate->mpcClass, false, pcTemplate, &szArguments);
	mpcDeclaringNamespace = pcSavedNamespace;
	mpcBindings = pcSaved;
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
// Deduces a function template's parameters from the types of a call's arguments
// (an entry may be NULL).  papcArguments are the arguments, or NULL if only their
// types are known.  papsDeduced gets an entry for each template parameter, NULL if
// it is not deduced, and palliDeduced the non-type parameters' values
// (DeduceParameter).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DeduceArguments(CCPPTemplate* pcTemplate, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						i;

	pcSuffix = ((CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
	for (i = 0; i < pcTemplate->mpcDeclaration->macpParameters.NumElements(); i++)
	{
		papsDeduced->Add(NULL);
		palliDeduced->Add(0);
	}
	DeduceParameters(pcTemplate, pcSuffix, papsArguments, papcArguments, papsDeduced, palliDeduced);
}


//////////////////////////////////////////////////////////////////////////
//
// Deduces from each of a function suffix's parameters that has an argument
// type in papsArguments, until a parameter pack takes the rest.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DeduceParameters(CCPPTemplate* pcTemplate, CCPPSyntaxFunctionSuffix* pcSuffix, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
{
	CCPPSyntaxParameter*	pcParameter;
	size					i;

	for (i = 0; pcSuffix && (i < pcSuffix->macpParameters.NumElements()) && (i < papsArguments->NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		if (!DeduceParameter(pcTemplate, pcParameter, papsArguments, papcArguments, i, papsDeduced, palliDeduced))
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
bool CCPP65Generator::DeduceParameter(CCPPTemplate* pcTemplate, CCPPSyntaxParameter* pcParameter, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, size i, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
{
	CCPPSyntaxTypeSpecifier*	pcType;
	CCPPSyntaxName*				pcName;
	CCPPSyntax*					pcTemplateParameter;
	SCPP65Type*					psArgument;
	size						j;
	size						k;
	size						uiPointers;
	bool						bForwarding;

	DeduceMemberPointer(pcTemplate, pcParameter->mpcDeclarator, papsArguments->GetPtr(i), papsDeduced, palliDeduced);
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
		DeduceFromInstance(pcTemplate, pcName->GetLast(), psArgument, papsDeduced, palliDeduced);
		return true;
	}
	for (j = 0; j < pcTemplate->mpcDeclaration->macpParameters.NumElements(); j++)
	{
		pcTemplateParameter = pcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
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
void CCPP65Generator::DeduceMemberPointer(CCPPTemplate* pcTemplate, CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type* psArgument, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
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
			for (j = 0; pcSymbol && (j < pcTemplate->mpcDeclaration->macpParameters.NumElements()); j++)
			{
				pcTemplateParameter = pcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
				if ((pcTemplateParameter->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)pcTemplateParameter)->mpcSymbol == pcSymbol) && !papsDeduced->GetPtr(j))
				{
					papsDeduced->SetPtr(j, psArgument->pcClass->mpsType);
				}
			}

			//The member function's parameters follow the parentheses around (M::*f).
			pcSuffix = (pcOuter && (pcOuter->macpSuffixes.NumElements() > 0)) ? pcOuter->macpSuffixes.GetPtr(0) : NULL;
			if (pcSuffix && (pcSuffix->GetKind() == CPPS_FunctionSuffix) && psArgument->psBase && psArgument->psBase->IsFunction())
			{
				DeduceParameters(pcTemplate, (CCPPSyntaxFunctionSuffix*)pcSuffix, psArgument->psBase->papsParameters, NULL, papsDeduced, palliDeduced);
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
void CCPP65Generator::DeduceFromInstance(CCPPTemplate* pcTemplate, CCPPSyntaxNameComponent* pcComponent, SCPP65Type* psArgument, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
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
		for (j = 0; pcSymbol && (j < pcTemplate->mpcDeclaration->macpParameters.NumElements()); j++)
		{
			pcTemplateParameter = pcTemplate->mpcDeclaration->macpParameters.GetPtr(j);
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
// pcClass if it is an instance of the class template pcTemplate names, else the
// first of its bases that is.  NULL if none is.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPP65Generator::FindInstanceBase(CCPPClass* pcClass, CCPPSymbol* pcTemplate)
{
	CCPPClass*	pcFound;
	size			i;

	if (pcClass->mpcTemplate && (pcClass->mpcTemplate->mpcPrimary->mpcSymbol == pcTemplate))
	{
		return pcClass;
	}
	if (!pcClass->HasBases())
	{
		BeginQuiet();
		ResolveBases(pcClass, NULL);
		EndQuiet();
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
CCPPTemplate* CCPP65Generator::FindFunctionSpecialisation(CCPPTemplate* pcPrimary, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPTemplate*				pcTemplate;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPTemplateArguments*				pcSaved;
	SCPP65Type*					psType;
	CArrayCPP65Types			apsParameters;
	CArrayCPP65Types			apsDeduced;
	CArrayInt64					alliDeduced;
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
			pcSaved = mpcBindings;
			mpcBindings = NULL;
			BeginQuiet();
			apsParameters.Init();
			pcSuffix = ((CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
			for (j = 0; pcSuffix && (j < pcSuffix->macpParameters.NumElements()); j++)
			{
				pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(j);
				psType = GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
				apsParameters.Add((psType && psType->IsReference()) ? psType->psBase : psType);
			}
			apsDeduced.Init();
			alliDeduced.Init();
			DeduceArguments(pcPrimary, &apsParameters, NULL, &apsDeduced, &alliDeduced);
			if (!BindArguments(pcPrimary->mpcDeclaration, (pcTemplate->mpcPattern && pcTemplate->mpcPattern->mbTemplateArguments) ? &pcTemplate->mpcPattern->macpTemplateArguments : NULL, &apsDeduced, &pcTemplate->mszArguments, pcNode, &alliDeduced))
			{
				pcTemplate->mszArguments.Clear();
			}
			alliDeduced.Kill();
			apsDeduced.Kill();
			apsParameters.Kill();
			EndQuiet();
			mpcBindings = pcSaved;
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
	CArrayCPP65Types			apsDeduced;
	CArrayInt64					alliDeduced;
	CChars						szArguments;
	CCPPTemplateArguments*				pcBindings;
	CCPPTemplateArguments*				pcSaved;
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
		aiRanks[i] = CPPCR_Exact;
	}

	//A failure here is not one of the quiet code around it.
	bQuietFailed = mbQuietFailed;
	BeginQuiet();
	apsArguments.Init();
	pcBindings = NULL;
	bTakes = !pcTemplate->mpcClass || DefineMemberTemplate(pcTemplate, pcNode);
	if (bTakes)
	{
		for (i = 0; i < papcArguments->NumElements(); i++)
		{
			apsArguments.Add(TypeOf(papcArguments->GetPtr(i)));
		}
		apsDeduced.Init();
		alliDeduced.Init();
		DeduceArguments(pcTemplate, &apsArguments, papcArguments, &apsDeduced, &alliDeduced);
		szArguments.Init();
		pcBindings = BindArguments(pcTemplate->mpcDeclaration, papcExplicit, &apsDeduced, &szArguments, pcNode, &alliDeduced);
		szArguments.Kill();
		alliDeduced.Kill();
		apsDeduced.Kill();
		bTakes = pcBindings != NULL;
	}

	if (bTakes)
	{
		//The parameters' types are worked out as InstantiateFunction declares them.
		pcSaved = mpcBindings;
		if (pcTemplate->mpcClass)
		{
			pcBindings->mpcOuter = pcTemplate->mpcClassDeclaration ? RenameBindings(pcTemplate->mpcClass, pcTemplate->mpcClassDeclaration) : pcTemplate->mpcClass->mpcBindings;
		}
		else
		{
			pcBindings->mpcOuter = mpcBindings;
		}
		mpcBindings = pcBindings;
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
			psParameter = GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcNode);
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
		mpcBindings = pcSaved;
		if (!pcTemplate->mpcClass)
		{
			pcBindings->mpcOuter = NULL;
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
