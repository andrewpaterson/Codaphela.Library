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
#include <stdlib.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "CTIdentifier.h"
#include "CTReservedWord.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::NewClass(CCPPSymbol* pcSymbol, int64 lliName)
{
	SCPP65Class*	psClass;

	psClass = NewMalloc<SCPP65Class>();
	psClass->pcSymbol = pcSymbol;
	psClass->lliName = lliName;
	psClass->psType = mcTypes.Class(psClass);
	psClass->psBase = NULL;
	psClass->macBases.Init();
	psClass->macUsings.Init();
	psClass->macFields.Init();
	psClass->mapsVirtuals.Init();
	psClass->iVtableOffset = -1;
	psClass->macVtables.Init();
	psClass->bComplete = false;
	psClass->bLayingOut = false;
	psClass->bBases = false;
	psClass->bEmpty = false;
	psClass->iAlign = 1;
	psClass->bUnion = false;
	psClass->bVtable = false;
	psClass->bConstructors = false;
	psClass->psInheritedBase = NULL;
	psClass->psDestructor = NULL;
	psClass->pcSpecifier = NULL;
	psClass->psTemplate = NULL;
	psClass->psBindings = NULL;
	psClass->mszArguments.Init();
	mapsClasses.Add(psClass);
	return psClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class (not a template instance) a symbol names.  A class used before it is
// defined, or only through pointers, is laid out when it is first needed whole.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	SCPP65Class*	psClass;
	size			i;

	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if ((psClass->pcSymbol == pcSymbol) && !psClass->psTemplate)
		{
			return psClass;
		}
	}
	return NewClass(pcSymbol, pcSymbol->mlliName);
}


//////////////////////////////////////////////////////////////////////////
//
// A class without a name (typedef struct { ... } SName;) is known by its definition.
// One in a class template is a different class in each instance (its bindings).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	SCPP65Class*	psClass;
	SCPP65Bindings*	psBindings;
	CCPPSyntax*		pcParent;
	size			i;

	for (psBindings = mpsBindings; psBindings; psBindings = psBindings->psOuter)
	{
		pcParent = pcSpecifier->mpcParent;
		while (pcParent && (!psBindings->psClass || (pcParent != psBindings->psClass->pcSpecifier)))
		{
			pcParent = pcParent->mpcParent;
		}
		if (pcParent)
		{
			break;
		}
	}

	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if (!psClass->pcSymbol && (psClass->pcSpecifier == pcSpecifier) && !psClass->psTemplate && (psClass->psBindings == psBindings))
		{
			return psClass;
		}
	}
	psClass = NewClass(NULL, -1);
	psClass->pcSpecifier = pcSpecifier;
	psClass->psBindings = psBindings;
	return psClass;
}


//////////////////////////////////////////////////////////////////////////
//
// A class (not a template instance) by name, for qualified names.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::FindClass(int64 lliName)
{
	SCPP65Class*	psClass;
	size			i;

	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if ((psClass->lliName == lliName) && !psClass->psTemplate)
		{
			return psClass;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An alignment limited by #pragma pack(iPack) (0 for none).
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::PackAlignment(int iAlign, int iPack)
{
	return ((iPack > 0) && (iAlign > iPack)) ? iPack : iAlign;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a name component names: a class, a typedef of one, an instance of a
// class template (Name<Arguments>), or a class template's own name inside it.  A
// base template's name without arguments is psContext's (or else the current member
// function's class's) base instance of it.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, SCPP65Class* psContext)
{
	SCPP65Template*	psTemplate;
	SCPP65Bindings*	psBindings;
	SCPP65Binding*	psBinding;
	SCPP65Class*	psClass;
	SCPP65Type*		psType;
	int64			lliName;

	if (!pcSymbol && pcComponent)
	{
		pcSymbol = pcComponent->mpcSymbol;
	}
	if (!pcSymbol && pcComponent)
	{
		//Names in templates are not always resolved by the parser.
		lliName = GetComponentName(pcComponent);
		psTemplate = (lliName != -1) ? GetTemplateByName(lliName) : NULL;
		if (psTemplate)
		{
			pcSymbol = psTemplate->pcSymbol;
		}
		else if (lliName != -1)
		{
			return FindClass(lliName);
		}
	}
	if (!pcSymbol)
	{
		return NULL;
	}

	if (pcSymbol->meKind == CPPSK_Class)
	{
		return GetClass(pcSymbol, pcNode);
	}
	if (pcSymbol->meKind == CPPSK_Typedef)
	{
		psType = GetSymbolType(pcSymbol, pcNode);
		return (psType && psType->IsClass()) ? psType->psClass : NULL;
	}
	if (pcSymbol->meKind == CPPSK_TypeParameter)
	{
		psBinding = FindBinding(pcSymbol);
		return (psBinding && psBinding->psType && psBinding->psType->IsClass()) ? psBinding->psType->psClass : NULL;
	}
	if (pcSymbol->meKind == CPPSK_ClassTemplate)
	{
		psTemplate = GetTemplate(pcSymbol);
		if (!psTemplate)
		{
			//A pointer to an instance of a template that is only declared needs no definition.
			psClass = (pcComponent && pcComponent->mbTemplateArguments) ? UndefinedInstance(pcSymbol, &pcComponent->macpTemplateArguments, pcNode) : NULL;
			if (!psClass)
			{
				Error(pcNode, "The class template has not been defined");
			}
			return psClass;
		}
		if (pcComponent && pcComponent->mbTemplateArguments)
		{
			return InstantiateClass(psTemplate, &pcComponent->macpTemplateArguments, pcNode);
		}

		//The template's own name inside it is the instance being made, and a base's name
		//in a class derived from an instance is that instance.
		for (psBindings = mpsBindings; psBindings; psBindings = psBindings->psOuter)
		{
			if (psBindings->psClass && psBindings->psClass->psTemplate && (psBindings->psClass->psTemplate->psPrimary == psTemplate))
			{
				return psBindings->psClass;
			}
		}
		if (!psContext && mpsFunction)
		{
			psContext = mpsFunction->psClass;
		}
		psClass = psContext ? FindBaseInstance(psContext, psTemplate) : NULL;
		if (psClass)
		{
			return psClass;
		}
		Error(pcNode, "A class template needs template arguments");
		return NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A type's alignment in a class, as MSVC lays classes out (BaseLib reads and writes them as raw bytes): a
// scalar is aligned to its size (8 for long long and double), a pointer or reference to 4, an array as its
// element and a class to its largest member's alignment.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::TypeAlignment(SCPP65Type* psType)
{
	if (psType->IsClass())
	{
		return (psType->psClass->iAlign < 1) ? 1 : psType->psClass->iAlign;
	}
	if (psType->IsArray())
	{
		return TypeAlignment(psType->psBase);
	}
	if (psType->IsReference() || psType->IsPointer() || psType->IsMemberPointer())
	{
		return 4;
	}
	if (psType->iBytes >= 8)
	{
		return 8;
	}
	if (psType->iBytes >= 4)
	{
		return 4;
	}
	return (psType->iBytes < 1) ? 1 : psType->iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a class template that is psClass or one of its bases, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::FindBaseInstance(SCPP65Class* psClass, SCPP65Template* psTemplate)
{
	SCPP65Class*	psBase;
	size			i;

	if (psClass->psTemplate && (psClass->psTemplate->psPrimary == psTemplate))
	{
		return psClass;
	}
	BeginQuiet();
	ResolveBases(psClass, NULL);
	EndQuiet();
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = FindBaseInstance(psClass->macBases.Get(i)->psClass, psTemplate);
		if (psBase)
		{
			return psBase;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a qualified member name (object.Qualifier::f) names: psClass or one of its bases.  A class
// template's name without arguments is psClass's base instance of it.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::QualifierClass(SCPP65Class* psClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
{
	SCPP65Class*	psQualifier;

	psQualifier = GetComponentClass(pcComponent, NULL, pcNode, psClass);
	if (!psQualifier || !IsDerived(psClass, psQualifier, NULL))
	{
		Error(pcNode, "The qualifier is not the object's class or one of its bases");
		return NULL;
	}
	return psQualifier;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a (possibly qualified) name's last component names.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	return GetComponentClass(pcName->GetLast(), pcName->mpcSymbol, pcNode);
}


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
		if (pcName && (pcName->macpComponents.NumElements() == 2) && !bExplicit)
		{
			//template<class M> M* CArrayTemplate<M>::Get(size iIndex) { ... }
			pcClass = (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0);
			psClass = GetTemplateByName(GetComponentName(pcClass));
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
	size			i;
	int64			lliName;

	//template<> int Size(short s) names a new function; its template is found by name.
	psPrimary = pcSymbol ? GetTemplate(pcSymbol) : NULL;
	lliName = pcPattern ? GetComponentName(pcPattern) : -1;
	for (i = 0; !psPrimary && (lliName != -1) && (i < mapsTemplates.NumElements()); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if (!psTemplate->bMember && (psTemplate->psPrimary == psTemplate) && psTemplate->pcSymbol && (psTemplate->pcSymbol->mlliName == lliName) && (psTemplate->pcDefinition->GetKind() == pcDefinition->GetKind()))
		{
			psPrimary = psTemplate;
		}
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
			if (pcTypeId->mpcSpecifiers && pcTypeId->mpcSpecifiers->mpcType && (pcTypeId->mpcSpecifiers->mpcType->GetKind() == CPPS_NamedTypeSpecifier) && !(pcTypeId->mpcSpecifiers->muiFlags & CPPDS_CONST) && (!pcTypeId->mpcDeclarator || ((pcTypeId->mpcDeclarator->macpPrefix.NumElements() == 0) && (pcTypeId->mpcDeclarator->macpSuffixes.NumElements() == 0))))
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
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass == psClass) && (psFunction->pcDefinition || psFunction->bImplicit))
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
// A class template by name.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::GetTemplateByName(int64 lliName)
{
	SCPP65Template*	psTemplate;
	size			i;

	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if (!psTemplate->bMember && (psTemplate->psPrimary == psTemplate) && psTemplate->pcSymbol && (psTemplate->pcSymbol->mlliName == lliName) && (psTemplate->pcDefinition->GetKind() == CPPS_ClassSpecifier))
		{
			return psTemplate;
		}
	}
	return NULL;
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
	CCPPSyntaxTypeId*		pcTypeId;
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
		psBinding->bConst = false;
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
				//const int is a different argument to int; const char* is not const itself.
				pcTypeId = (CCPPSyntaxTypeId*)pcArgument;
				psBinding->psType = GetTypeIdType(pcTypeId);
				psBinding->bConst = pcTypeId->mpcSpecifiers && (pcTypeId->mpcSpecifiers->muiFlags & CPPDS_CONST) && (!pcTypeId->mpcDeclarator || (pcTypeId->mpcDeclarator->macpPrefix.NumElements() == 0));
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
			pszArguments->Append(psBinding->bConst ? "_K" : "_");
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
		if (!mapsClasses.GetPtr(i)->psTemplate && (mapsClasses.GetPtr(i)->lliName == psTemplate->pcSymbol->mlliName) && mapsClasses.GetPtr(i)->mszArguments.Equals(&szArguments))
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
		if (!psClass->psTemplate && (psClass->lliName == pcSymbol->mlliName) && psClass->mszArguments.Equals(&szArguments))
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
			psBinding->bConst = false;
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
	bool							bConst;
	bool							bResult;

	pcSpecifiers = pcPattern->mpcSpecifiers;
	pcDeclarator = pcPattern->mpcDeclarator;
	if (!pcSpecifiers || (pcDeclarator && (pcDeclarator->macpSuffixes.NumElements() > 0 || pcDeclarator->mpcNested)))
	{
		return false;
	}

	psType = psArgument->psType;
	bConst = psArgument->bConst;
	for (i = pcDeclarator ? pcDeclarator->macpPrefix.NumElements() : 0; i > 0; i--)
	{
		pcOperator = (CCPPSyntaxPointerOperator*)pcDeclarator->macpPrefix.GetPtr(i - 1);
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
		bConst = false;
		(*piScore)++;
	}
	if ((pcSpecifiers->muiFlags & CPPDS_CONST) && (!pcDeclarator || (pcDeclarator->macpPrefix.NumElements() == 0)))
	{
		if (!bConst)
		{
			return false;
		}
		bConst = false;
		(*piScore)++;
	}

	//T takes the type.
	pcNamed = (pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_NamedTypeSpecifier)) ? (CCPPSyntaxNamedTypeSpecifier*)pcSpecifiers->mpcType : NULL;
	psBinding = pcNamed ? DeducedBinding(psDeduced, pcNamed->mpcName->mpcSymbol) : NULL;
	if (psBinding)
	{
		if (psBinding->psType)
		{
			return psBinding->psType->Equals(psType) && (psBinding->bConst == bConst);
		}
		psBinding->psType = psType;
		psBinding->bConst = bConst;
		return true;
	}

	//X<T> matches an instance of X with matching arguments.
	pcComponent = pcNamed ? pcNamed->mpcName->GetLast() : NULL;
	if (pcComponent && pcComponent->mbTemplateArguments)
	{
		psTemplate = pcComponent->mpcSymbol ? GetTemplate(pcComponent->mpcSymbol) : GetTemplateByName(GetComponentName(pcComponent));
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
	bResult = psNamed && psNamed->Equals(psArgument->psType) && !bConst;
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
	DeduceArguments(psTemplate, &apsArguments, &apsDeduced, &alliDeduced);
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
	psFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, psTemplate->psClass, false, psTemplate, &szArguments);
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
// (an entry may be NULL): a parameter declared as T, T*, T** ... or T& takes the
// argument's type, less the pointers.  papsDeduced gets an entry for each template
// parameter, NULL if it is not deduced.  A function parameter pack (Args ... args)
// takes the rest of the arguments' types: the template parameter pack's entry is the
// first and the rest are added after it.  A parameter declared as Name<..., N, ...>
// (or a pointer or reference to one) deduces from the argument's instance of Name
// (DeduceFromInstance); palliDeduced gets the non-type parameters' values.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DeduceArguments(SCPP65Template* psTemplate, CArrayCPP65Types* papsArguments, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPSyntaxTypeSpecifier*	pcType;
	CCPPSyntaxName*				pcName;
	CCPPSyntax*					pcTemplateParameter;
	SCPP65Type*					psArgument;
	size						i;
	size						j;
	size						k;
	size						uiPointers;

	pcSuffix = ((CCPPSyntaxFunctionDefinition*)psTemplate->pcDefinition)->mpcDeclarator->GetFunctionSuffix();
	for (i = 0; i < psTemplate->pcDeclaration->macpParameters.NumElements(); i++)
	{
		papsDeduced->Add(NULL);
		palliDeduced->Add(0);
	}
	for (i = 0; pcSuffix && (i < pcSuffix->macpParameters.NumElements()) && (i < papsArguments->NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		DeduceMemberPointerClass(psTemplate, pcParameter->mpcDeclarator, papsArguments->GetPtr(i), papsDeduced);
		pcType = pcParameter->mpcSpecifiers ? pcParameter->mpcSpecifiers->mpcType : NULL;
		if (!pcType || (pcType->GetKind() != CPPS_NamedTypeSpecifier))
		{
			continue;
		}
		pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
		if (pcName->GetLast() && pcName->GetLast()->mbTemplateArguments)
		{
			psArgument = papsArguments->GetPtr(i);
			psArgument = (psArgument && psArgument->IsReference()) ? psArgument->psBase : psArgument;
			if (psArgument && psArgument->IsPointer() && pcParameter->mpcDeclarator && (pcParameter->mpcDeclarator->macpPrefix.NumElements() == 1) && (((CCPPSyntaxPointerOperator*)pcParameter->mpcDeclarator->macpPrefix.GetPtr(0))->meKind == CPPP_Pointer))
			{
				psArgument = psArgument->psBase;
			}
			DeduceFromInstance(psTemplate, pcName->GetLast(), psArgument, papsDeduced, palliDeduced);
			continue;
		}
		for (j = 0; j < psTemplate->pcDeclaration->macpParameters.NumElements(); j++)
		{
			pcTemplateParameter = psTemplate->pcDeclaration->macpParameters.GetPtr(j);
			if ((pcTemplateParameter->GetKind() != CPPS_TypeParameter) || (((CCPPSyntaxTypeParameter*)pcTemplateParameter)->mpcSymbol != pcName->mpcSymbol))
			{
				continue;
			}
			if (pcParameter->mpcDeclarator && pcParameter->mpcDeclarator->mbPack)
			{
				//Args ... args takes the types of the rest of the arguments (the pack is the last parameter).
				for (k = i; k < papsArguments->NumElements(); k++)
				{
					psArgument = papsArguments->GetPtr(k);
					psArgument = (psArgument && psArgument->IsReference()) ? psArgument->psBase : psArgument;
					if (k == i)
					{
						papsDeduced->SetPtr(j, psArgument);
					}
					else
					{
						papsDeduced->Add(psArgument);
					}
				}
				return;
			}
			psArgument = papsArguments->GetPtr(i);
			uiPointers = pcParameter->mpcDeclarator ? pcParameter->mpcDeclarator->macpPrefix.NumElements() : 0;
			if (uiPointers && (((CCPPSyntaxPointerOperator*)pcParameter->mpcDeclarator->macpPrefix.GetPtr(0))->meKind != CPPP_Pointer))
			{
				uiPointers = 0;
			}
			for (; psArgument && uiPointers > 0; uiPointers--)
			{
				psArgument = psArgument->IsPointer() ? psArgument->psBase : NULL;
			}
			if (psArgument && !papsDeduced->GetPtr(j))
			{
				papsDeduced->SetPtr(j, psArgument);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter declared as a pointer to a member of a template parameter, void
// (M::*f)(CUnknown*, void*), deduces M as the class of the argument's pointer to
// member (&CListener::Changed).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DeduceMemberPointerClass(SCPP65Template* psTemplate, CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type* psArgument, CArrayCPP65Types* papsDeduced)
{
	CCPPSyntaxPointerOperator*	pcPointer;
	CCPPSyntax*					pcTemplateParameter;
	CCPPSymbol*					pcSymbol;
	size						i;
	size						j;

	psArgument = (psArgument && psArgument->IsReference()) ? psArgument->psBase : psArgument;
	if (!psArgument || !psArgument->IsMemberPointer() || !psArgument->psClass)
	{
		return;
	}
	for (; pcDeclarator; pcDeclarator = pcDeclarator->mpcNested)
	{
		for (i = 0; i < pcDeclarator->macpPrefix.NumElements(); i++)
		{
			pcPointer = (CCPPSyntaxPointerOperator*)pcDeclarator->macpPrefix.GetPtr(i);
			pcSymbol = ((pcPointer->meKind == CPPP_MemberPointer) && pcPointer->mpcClass) ? pcPointer->mpcClass->mpcSymbol : NULL;
			for (j = 0; pcSymbol && (j < psTemplate->pcDeclaration->macpParameters.NumElements()); j++)
			{
				pcTemplateParameter = psTemplate->pcDeclaration->macpParameters.GetPtr(j);
				if ((pcTemplateParameter->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)pcTemplateParameter)->mpcSymbol == pcSymbol) && !papsDeduced->GetPtr(j))
				{
					papsDeduced->SetPtr(j, psArgument->psClass->psType);
				}
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
	if (pcTypeId->mpcDeclarator && ((pcTypeId->mpcDeclarator->macpPrefix.NumElements() > 0) || (pcTypeId->mpcDeclarator->macpSuffixes.NumElements() > 0)))
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
			DeduceArguments(psPrimary, &apsParameters, &apsDeduced, &alliDeduced);
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
// Finds a class's base classes without laying it out, so a pointer to it can be
// converted to a pointer to a base.  Offsets are worked out by LayoutClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ResolveBases(SCPP65Class* psClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxBaseSpecifier*	pcBaseSpecifier;
	SCPP65Class*				psBaseClass;
	SCPP65Base*					psBase;
	SCPP65Bindings*				psSaved;
	size						i;
	bool						bResult;

	if (psClass->bBases)
	{
		return true;
	}
	if (!psClass->pcSpecifier && psClass->pcSymbol && psClass->pcSymbol->mpcDeclaration && (psClass->pcSymbol->mpcDeclaration->GetKind() == CPPS_ClassSpecifier))
	{
		psClass->pcSpecifier = (CCPPSyntaxClassSpecifier*)psClass->pcSymbol->mpcDeclaration;
	}
	if (!psClass->pcSpecifier)
	{
		return Error(pcNode, "The class has not been defined");
	}

	psClass->bBases = true;
	psSaved = mpsBindings;
	mpsBindings = psClass->psBindings;
	bResult = true;
	for (i = 0; i < psClass->pcSpecifier->macpBases.NumElements(); i++)
	{
		pcBaseSpecifier = (CCPPSyntaxBaseSpecifier*)psClass->pcSpecifier->macpBases.GetPtr(i);
		if (pcBaseSpecifier->mbVirtual)
		{
			bResult = Error(pcBaseSpecifier, "Virtual inheritance is not supported by the compiler yet");
			continue;
		}
		psBaseClass = GetNameClass(pcBaseSpecifier->mpcName, pcBaseSpecifier);
		if (!psBaseClass)
		{
			bResult = Error(pcBaseSpecifier, "This base class is not supported by the compiler yet");
			continue;
		}
		bResult &= ResolveBases(psBaseClass, pcBaseSpecifier);
		psBase = psClass->macBases.Add();
		psBase->psClass = psBaseClass;
		psBase->iOffset = 0;
	}
	psClass->psBase = (psClass->macBases.NumElements() > 0) ? psClass->macBases.Get(0)->psClass : NULL;
	mpsBindings = psSaved;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out a class the first time it is needed whole.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CompleteClass(SCPP65Class* psClass, CCPPSyntax* pcNode)
{
	if (psClass->bComplete)
	{
		return true;
	}
	if (psClass->bLayingOut)
	{
		return Error(pcNode, "A class cannot contain itself");
	}
	if (!psClass->pcSpecifier && psClass->pcSymbol && psClass->pcSymbol->mpcDeclaration && (psClass->pcSymbol->mpcDeclaration->GetKind() == CPPS_ClassSpecifier))
	{
		psClass->pcSpecifier = (CCPPSyntaxClassSpecifier*)psClass->pcSymbol->mpcDeclaration;
	}
	if (!psClass->pcSpecifier)
	{
		return Error(pcNode, "The class has not been defined");
	}
	return LayoutClass(psClass, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out a class: the bases in order (an empty base takes no space), the vtable
// pointer if no base has one, then the fields.  Members are read first: functions
// are declared (in class definitions are generated when called) and fields are
// collected.  A template instance's members and its members defined outside it are
// worked out with its template arguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::LayoutClass(SCPP65Class* psClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxClassSpecifier*		pcSpecifier;
	CCPPSyntaxBaseSpecifier*		pcBaseSpecifier;
	CCPPSyntaxNameComponent*		pcComponent;
	SCPP65Template*					psMember;
	SCPP65Base*						psBase;
	SCPP65Field*					psField;
	SCPP65Function*					psFunction;
	SCPP65Bindings*					psSaved;
	SCPP65Bindings*					psMemberBindings;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	size							i;
	int								iBytes;
	int								iAlign;
	int								iPack;
	int								iFieldAlign;
	bool							bResult;

	pcSpecifier = psClass->pcSpecifier;
	psClass->bLayingOut = true;
	psSaved = mpsBindings;
	mpsBindings = psClass->psBindings;
	bResult = true;

	psClass->bUnion = pcSpecifier->meKey == CT_union;
	pcComponent = pcSpecifier->mpcName ? pcSpecifier->mpcName->GetLast() : NULL;
	if (pcComponent && !psClass->psTemplate)
	{
		psClass->lliName = GetComponentName(pcComponent);
	}

	bResult &= ResolveBases(psClass, pcSpecifier);
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		pcBaseSpecifier = (CCPPSyntaxBaseSpecifier*)pcSpecifier->macpBases.GetPtr(i);
		bResult &= CompleteClass(psClass->macBases.Get(i)->psClass, pcBaseSpecifier);
	}

	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		bResult &= ClassMember(psClass, pcSpecifier->macpMembers.GetPtr(i));
	}

	//A class template's members defined outside it.  Their template parameters are
	//new names for the class's.
	if (psClass->psTemplate)
	{
		for (i = 0; i < mapsTemplates.NumElements(); i++)
		{
			psMember = mapsTemplates.GetPtr(i);
			if (!psMember->bMember || psMember->bMemberTemplate || (psMember->psOwner != psClass->psTemplate))
			{
				continue;
			}
			psMemberBindings = RenameBindings(psClass, psMember->pcDeclaration);

			pcDefinition = (CCPPSyntaxFunctionDefinition*)psMember->pcDefinition;
			mpsBindings = psMemberBindings;
			BeginQuiet();
			psFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, psClass, false);
			EndQuiet();
			mpsBindings = psClass->psBindings;
			if (psFunction && (pcDefinition->meBody == CPPFB_Body))
			{
				psFunction->pcDefinition = pcDefinition;
				psFunction->psBindings = psMemberBindings;
				psFunction->bInline = true;
			}
		}
	}

	//A destructor is needed to destroy the bases or the fields.
	if (!psClass->psDestructor && NeedsDestructor(psClass))
	{
		psFunction = AddFunction(psClass, psClass->lliName, CPP65FK_Destructor, &mcTypes.msVoid);
		psFunction->bImplicit = true;
		psClass->psDestructor = psFunction;
	}
	bResult &= LayoutVirtuals(psClass, pcSpecifier);

	//The bases, the vtable pointer, then the fields.  #pragma pack(n) aligns each to at most n bytes.
	iPack = pcSpecifier->miPack;
	iBytes = 0;
	iAlign = 1;
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		if (psBase->psClass->bEmpty)
		{
			psBase->iOffset = 0;
			continue;
		}
		iFieldAlign = PackAlignment(psBase->psClass->iAlign, iPack);
		iBytes = (iBytes + iFieldAlign - 1) & ~(iFieldAlign - 1);
		psBase->iOffset = iBytes;
		iBytes += psBase->psClass->psType->iBytes;
		iAlign = (iFieldAlign > iAlign) ? iFieldAlign : iAlign;
	}
	if (psClass->psBase && (psClass->psBase->iVtableOffset != -1) && (psClass->macBases.Get(0)->iOffset == 0))
	{
		psClass->iVtableOffset = psClass->psBase->iVtableOffset;
	}
	else if (psClass->mapsVirtuals.NumElements() > 0)
	{
		iFieldAlign = PackAlignment(4, iPack);
		iBytes = (iBytes + iFieldAlign - 1) & ~(iFieldAlign - 1);
		psClass->iVtableOffset = iBytes;
		iBytes += 4;
		iAlign = (iFieldAlign > iAlign) ? iFieldAlign : iAlign;
	}
	LayoutSecondaryVtables(psClass);
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		iFieldAlign = PackAlignment(TypeAlignment(psField->psType), iPack);
		iBytes = psClass->bUnion ? 0 : (iBytes + iFieldAlign - 1) & ~(iFieldAlign - 1);
		psField->iOffset = iBytes;
		iBytes += psField->psType->iBytes;
		iAlign = (iFieldAlign > iAlign) ? iFieldAlign : iAlign;
		if (psClass->bUnion && (iBytes > psClass->psType->iBytes))
		{
			psClass->psType->iBytes = iBytes;
		}
	}
	if (psClass->bUnion)
	{
		iBytes = psClass->psType->iBytes;
	}
	iBytes = (iBytes + iAlign - 1) & ~(iAlign - 1);

	psClass->bEmpty = (psClass->macFields.NumElements() == 0) && (psClass->iVtableOffset == -1);
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psClass->bEmpty &= psClass->macBases.Get(i)->psClass->bEmpty;
	}
	psClass->iAlign = iAlign;
	psClass->psType->iBytes = (iBytes > 0) ? iBytes : 1;
	psClass->bComplete = true;
	psClass->bLayingOut = false;

	if (psClass->psInheritedBase)
	{
		InheritConstructors(psClass);
	}

	//A class that needs one but declares none gets a constructor written for it when it is called.
	if (!psClass->bConstructors && NeedsConstructor(psClass))
	{
		psFunction = AddFunction(psClass, psClass->lliName, CPP65FK_Constructor, &mcTypes.msVoid);
		psFunction->bImplicit = true;
		psFunction->bDefined = true;
	}
	mpsBindings = psSaved;
	return bResult;
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

	uiParameters = NumParameters(pcSuffix);
	uiRequired = uiParameters;
	while ((uiRequired > 0) && ((CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(uiRequired - 1))->mpcDefault)
	{
		uiRequired--;
	}
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
	SCPP65Template*				psTemplate;
	SCPP65Template*				psFound;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						i;

	psFound = NULL;
	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if (!psTemplate->bMemberTemplate || (psTemplate->psClass != psClass) || (psTemplate->lliName != lliName))
		{
			continue;
		}
		pcSuffix = psTemplate->pcDeclarator->GetFunctionSuffix();
		if (!pcSuffix || !TakesArguments(pcSuffix, uiArguments))
		{
			continue;
		}
		if ((iConstObject < 0) || (IsConstSuffix(pcSuffix) == (iConstObject == 1)))
		{
			return psTemplate;
		}
		if (!psFound)
		{
			psFound = psTemplate;
		}
	}
	if (psFound)
	{
		return psFound;
	}

	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psTemplate = FindMemberTemplate(psClass->macBases.Get(i)->psClass, lliName, uiArguments, iConstObject);
		if (psTemplate)
		{
			return psTemplate;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The function template a call to pcSymbol with uiArguments arguments uses: the
// first of its overloads (the declarations chained by mpcNext) that takes them, or
// else GetTemplate's.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::GetCallTemplate(CCPPSymbol* pcSymbol, size uiArguments)
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
			if (pcSuffix && TakesArguments(pcSuffix, uiArguments))
			{
				return psTemplate;
			}
		}
	}
	return GetTemplate(pcSymbol);
}


//////////////////////////////////////////////////////////////////////////
//
// The number of parameters a function suffix declares: none for (void).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Generator::NumParameters(CCPPSyntaxFunctionSuffix* pcSuffix)
{
	CCPPSyntaxParameter*	pcParameter;
	size					uiParameters;

	if (!pcSuffix)
	{
		return 0;
	}
	uiParameters = pcSuffix->macpParameters.NumElements();
	if (uiParameters == 1)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(0);
		//(void), but not void (M::*f)(...) or void (*f)(...).
		if (pcParameter->mpcSpecifiers && (pcParameter->mpcSpecifiers->muiTypeKeywords == CPPTK_VOID) && (!pcParameter->mpcDeclarator || ((pcParameter->mpcDeclarator->macpPrefix.NumElements() == 0) && !pcParameter->mpcDeclarator->mpcNested && (pcParameter->mpcDeclarator->macpSuffixes.NumElements() == 0))))
		{
			uiParameters = 0;
		}
	}
	return uiParameters;
}


//////////////////////////////////////////////////////////////////////////
//
// using Base::f; in a class makes Base's functions named f overloads of the class's
// own.  using Base::Base; inherits Base's constructors (see InheritConstructors).
// Other using declarations (of types) are left out.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ClassUsing(SCPP65Class* psClass, CCPPSyntaxUsingDeclaration* pcUsing)
{
	CCPPSyntaxName*	pcName;
	SCPP65Class*	psBase;
	SCPP65Using*	psUsing;
	int64			lliName;

	pcName = pcUsing->mpcName;
	if (pcUsing->mbTypename || !pcName || (pcName->macpComponents.NumElements() != 2))
	{
		return;
	}
	lliName = GetFunctionName(pcName->GetLast());
	if (lliName == -1)
	{
		return;
	}
	BeginQuiet();
	psBase = GetComponentClass((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0), NULL, pcUsing, psClass);
	EndQuiet();
	if (!psBase || (psBase == psClass))
	{
		return;
	}
	if (psBase->lliName == lliName)
	{
		psClass->psInheritedBase = psBase;
		return;
	}
	psUsing = psClass->macUsings.Add();
	psUsing->psBase = psBase;
	psUsing->lliName = lliName;
}


//////////////////////////////////////////////////////////////////////////
//
// using Base::Base;  The class gets a constructor for each of the base's (not its
// default, copy or move constructors, constructor templates, or ones with the same
// parameters as the class's own) that calls the base's with its arguments and makes
// the rest of the object as a default constructor does.  A class that declares no
// constructor of its own keeps its default constructor.  [class.inhctor.init]
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::InheritConstructors(SCPP65Class* psClass)
{
	SCPP65Class*	psBase;
	SCPP65Function*	psInherited;
	SCPP65Function*	psOwn;
	SCPP65Function*	psFunction;
	SCPP65Type*		psParameter;
	size			uiFunctions;
	size			i;
	size			j;
	size			k;
	bool			bSame;
	bool			bAdded;

	psBase = psClass->psInheritedBase;
	if (!CompleteClass(psBase, NULL))
	{
		return;
	}
	bAdded = false;
	uiFunctions = mapsFunctions.NumElements();
	for (i = 0; i < uiFunctions; i++)
	{
		psInherited = mapsFunctions.GetPtr(i);
		if ((psInherited->psClass != psBase) || (psInherited->eKind != CPP65FK_Constructor) || psInherited->psTemplate || psInherited->bImplicit || (psInherited->mapsParameters.NumElements() == 0))
		{
			continue;
		}
		psParameter = psInherited->mapsParameters.GetPtr(0);
		if ((psInherited->mapsParameters.NumElements() == 1) && psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->psClass == psBase))
		{
			continue;
		}

		bSame = false;
		for (j = 0; !bSame && (j < mapsFunctions.NumElements()); j++)
		{
			psOwn = mapsFunctions.GetPtr(j);
			if ((psOwn->psClass != psClass) || (psOwn->eKind != CPP65FK_Constructor) || (psOwn->mapsParameters.NumElements() != psInherited->mapsParameters.NumElements()))
			{
				continue;
			}
			bSame = true;
			for (k = 0; bSame && (k < psOwn->mapsParameters.NumElements()); k++)
			{
				bSame = psOwn->mapsParameters.GetPtr(k)->Equals(psInherited->mapsParameters.GetPtr(k));
			}
		}
		if (bSame)
		{
			continue;
		}

		psFunction = AddFunction(psClass, psClass->lliName, CPP65FK_Constructor, &mcTypes.msVoid);
		psInherited = mapsFunctions.GetPtr(i);
		for (k = 0; k < psInherited->mapsParameters.NumElements(); k++)
		{
			psFunction->mapsParameters.Add(psInherited->mapsParameters.GetPtr(k));
			psFunction->mapcDefaults.Add(psInherited->mapcDefaults.GetPtr(k));
		}
		psFunction->uiConstReferences = psInherited->uiConstReferences;
		psFunction->bExplicit = psInherited->bExplicit;
		psFunction->bImplicit = true;
		psFunction->bDefined = true;
		psFunction->psInherited = psInherited;
		bAdded = true;
	}

	if (bAdded && !psClass->bConstructors)
	{
		psFunction = AddFunction(psClass, psClass->lliName, CPP65FK_Constructor, &mcTypes.msVoid);
		psFunction->bImplicit = true;
		psFunction->bDefined = true;
		psClass->bConstructors = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psFunction, a function named lliName, is one of psClass's overloads: its
// own, or a base's brought in by a using declaration and not hidden by one of its
// own with the same parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsOverloadOf(SCPP65Class* psClass, SCPP65Function* psFunction, int64 lliName)
{
	SCPP65Function*	psOwn;
	SCPP65Using*	psUsing;
	size			i;
	size			j;
	bool			bUsed;

	if (psFunction->psClass == psClass)
	{
		return true;
	}
	if (!psClass || !psFunction->psClass)
	{
		return false;
	}
	bUsed = false;
	for (i = 0; i < psClass->macUsings.NumElements(); i++)
	{
		psUsing = psClass->macUsings.Get(i);
		if ((psUsing->psBase == psFunction->psClass) && (psUsing->lliName == lliName))
		{
			bUsed = true;
			break;
		}
	}
	if (!bUsed)
	{
		return false;
	}
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psOwn = mapsFunctions.GetPtr(i);
		if ((psOwn->psClass != psClass) || (psOwn->lliName != lliName) || (psOwn->eKind != psFunction->eKind) || psOwn->psTemplate || (psOwn->mapsParameters.NumElements() != psFunction->mapsParameters.NumElements()))
		{
			continue;
		}
		for (j = 0; j < psOwn->mapsParameters.NumElements(); j++)
		{
			if (!psOwn->mapsParameters.GetPtr(j)->Equals(psFunction->mapsParameters.GetPtr(j)))
			{
				break;
			}
		}
		if (j == psOwn->mapsParameters.NumElements())
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A member template declared in its class but defined outside it takes the outside
// definition with its name and number of parameters (and its template parameters,
// which the definition uses).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::DefineMemberTemplate(SCPP65Template* psTemplate, CCPPSyntax* pcNode)
{
	SCPP65Template*	psOutside;
	CCPPSymbol*		pcClassSymbol;
	size			i;
	size			uiParameters;

	if (psTemplate->pcDefinition)
	{
		return true;
	}
	pcClassSymbol = psTemplate->psClass->psTemplate ? psTemplate->psClass->psTemplate->pcSymbol : psTemplate->psClass->pcSymbol;
	uiParameters = NumParameters(psTemplate->pcDeclarator->GetFunctionSuffix());
	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		psOutside = mapsTemplates.GetPtr(i);
		if (psOutside->bMemberTemplate && !psOutside->psClass && (psOutside->pcSymbol == pcClassSymbol) && (psOutside->lliName == psTemplate->lliName) && (NumParameters(((CCPPSyntaxFunctionDefinition*)psOutside->pcDefinition)->mpcDeclarator->GetFunctionSuffix()) == uiParameters))
		{
			psTemplate->pcDeclaration = psOutside->pcDeclaration;
			psTemplate->pcDefinition = psOutside->pcDefinition;
			psTemplate->pcClassDeclaration = psOutside->pcClassDeclaration;
			return true;
		}
	}
	return Error(pcNode, "The member template has not been defined");
}


//////////////////////////////////////////////////////////////////////////
//
// Declares a member of a class being laid out.  Functions defined in the class are
// generated when they are called.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ClassMember(SCPP65Class* psClass, CCPPSyntax* pcMember)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxClassSpecifier*		pcClassSpecifier;
	SCPP65Field*					psField;
	SCPP65Variable*					psVariable;
	SCPP65Function*					psFunction;
	SCPP65Type*						psType;
	SCPP65Type*						psElement;
	size							i;
	bool							bPure;
	bool							bResult;
	bool							bFriend;
	int64							lliZero;

	switch (pcMember->GetKind())
	{
	case CPPS_AccessSpecifier:
	case CPPS_EmptyDeclaration:
	case CPPS_StaticAssert:
	case CPPS_AliasDeclaration:
		return true;

	case CPPS_UsingDeclaration:
		ClassUsing(psClass, (CCPPSyntaxUsingDeclaration*)pcMember);
		return true;

	case CPPS_TemplateDeclaration:
		return MemberTemplate(psClass, (CCPPSyntaxTemplateDeclaration*)pcMember);

	case CPPS_FunctionDefinition:
		pcFunction = (CCPPSyntaxFunctionDefinition*)pcMember;
		if ((pcFunction->meBody == CPPFB_Default) && pcFunction->mpcDeclarator->GetName() && (GetFunctionName(pcFunction->mpcDeclarator->GetName()->GetLast()) == GetOperatorName(CT_Assignment, false)))
		{
			//operator = (...) = default is the implicit copy assignment, which = does when the class declares none.
			return true;
		}
		BeginQuiet();
		bFriend = pcFunction->mpcSpecifiers && (pcFunction->mpcSpecifiers->muiFlags & CPPDS_FRIEND);
		psFunction = DeclareFunction(pcFunction->mpcSpecifiers, pcFunction->mpcDeclarator, true, bFriend ? NULL : psClass, false);
		EndQuiet();
		if (!psFunction && pcFunction->mpcSpecifiers && (pcFunction->mpcSpecifiers->muiFlags & CPPDS_VIRTUAL))
		{
			//Leaving out a virtual function would move the vtable's other entries.
			return Error(pcFunction, "This virtual function's declaration is not supported by the compiler yet");
		}
		if (!psFunction)
		{
			mapcFailed.Add(pcFunction->mpcDeclarator);
			return true;
		}
		SetDefinition(psFunction, pcFunction);
		return true;

	case CPPS_SimpleDeclaration:
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcMember;
		pcSpecifiers = pcDeclaration->mpcSpecifiers;
		if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_FRIEND))
		{
			//A friend function is a function of the namespace, not a member.
			for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
			{
				pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
				if (pcInit->mpcDeclarator && pcInit->mpcDeclarator->IsFunction())
				{
					BeginQuiet();
					psFunction = DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, NULL, false);
					EndQuiet();
					if (!psFunction)
					{
						mapcFailed.Add(pcInit->mpcDeclarator);
					}
				}
			}
			return true;
		}
		if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_TYPEDEF))
		{
			return true;
		}
		if (pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
		{
			//A nested class is laid out when it is used.
			pcClassSpecifier = (CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType;
			if (pcClassSpecifier->mpcSymbol)
			{
				GetClass(pcClassSpecifier->mpcSymbol, pcMember)->pcSpecifier = pcClassSpecifier;
			}
			else if (pcDeclaration->macpDeclarators.NumElements() == 0)
			{
				//union { ... }; is an unnamed field whose members are the class's (FindField looks in it).
				psType = GetUnnamedClass(pcClassSpecifier)->psType;
				if (!CompleteClass(psType->psClass, pcMember))
				{
					return false;
				}
				psField = psClass->macFields.Add();
				psField->pcSymbol = NULL;
				psField->lliName = -1;
				psField->psType = psType;
				psField->iOffset = 0;
				psField->pcInitialiser = NULL;
				return true;
			}
		}

		bResult = true;
		for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
		{
			pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
			if (pcInit->mpcDeclarator->IsFunction())
			{
				//virtual int f() = 0;
				bPure = pcInit->mpcInitializer && Constant(pcInit->mpcInitializer, &lliZero) && (lliZero == 0);
				BeginQuiet();
				psFunction = DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, psClass, bPure);
				EndQuiet();
				if (!psFunction && (pcSpecifiers->muiFlags & CPPDS_VIRTUAL))
				{
					//Leaving out a virtual function would move the vtable's other entries.
					bResult &= Error(pcInit, "This virtual function's declaration is not supported by the compiler yet");
				}
				else if (!psFunction)
				{
					mapcFailed.Add(pcInit->mpcDeclarator);
				}
				else if ((pcInit->meInitializer == CPPI_Default) && (psFunction->eKind != CPP65FK_Function))
				{
					psFunction->bImplicit = true;
				}
				continue;
			}
			if (!pcInit->mpcSymbol)
			{
				continue;
			}

			psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			if (!psType)
			{
				bResult = false;
				continue;
			}
			if (pcSpecifiers->muiFlags & CPPDS_STATIC)
			{
				psVariable = AddVariable(&mapsGlobals, pcInit->mpcSymbol, psType, CPP65S_Global, 0);
				psVariable->psClass = psClass;
				continue;
			}

			psElement = psType->IsArray() ? psType->psBase : psType;
			if (psElement->IsClass() && !CompleteClass(psElement->psClass, pcInit))
			{
				bResult = false;
				continue;
			}
			if (psType->IsArray() && psElement->IsClass())
			{
				psType = mcTypes.Array(psElement, psType->iCount);
			}
			psField = psClass->macFields.Add();
			psField->pcSymbol = pcInit->mpcSymbol;
			psField->lliName = pcInit->mpcSymbol->mlliName;
			psField->psType = psType;
			psField->iOffset = 0;
			psField->pcInitialiser = pcInit->mpcInitializer;
		}
		return bResult;

	default:
		return Error(pcMember, "This member is not supported by the compiler yet");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The vtable starts as the first base's.  A function with the name and parameters
// of a virtual function in it overrides it (and is virtual); other virtual functions
// are added on the end.  A destructor overrides a virtual base destructor.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::LayoutVirtuals(SCPP65Class* psClass, CCPPSyntax* pcNode)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psVirtual;
	size				i;
	size				j;
	bool				bOverride;

	if (psClass->psBase)
	{
		for (i = 0; i < psClass->psBase->mapsVirtuals.NumElements(); i++)
		{
			psClass->mapsVirtuals.Add(psClass->psBase->mapsVirtuals.GetPtr(i));
		}
	}

	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass != psClass) || psFunction->bStatic || (psFunction->eKind == CPP65FK_Constructor))
		{
			continue;
		}

		bOverride = false;
		for (j = 0; j < psClass->mapsVirtuals.NumElements(); j++)
		{
			psVirtual = psClass->mapsVirtuals.GetPtr(j);
			if (Overrides(psVirtual, psFunction))
			{
				psClass->mapsVirtuals.SetPtr(j, psFunction);
				psFunction->bVirtual = true;
				psFunction->iSlot = (int)j;
				bOverride = true;
				break;
			}
		}
		//A function that overrides one in a secondary vtable also gets a place in the class's own.
		if (!bOverride && (psFunction->bVirtual || OverridesBase(psClass, psFunction)))
		{
			psFunction->bVirtual = true;
			psFunction->iSlot = (int)psClass->mapsVirtuals.NumElements();
			psClass->mapsVirtuals.Add(psFunction);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if psFunction overrides psVirtual: destructors override destructors, other
// functions need the same name and parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Overrides(SCPP65Function* psVirtual, SCPP65Function* psFunction)
{
	return (psVirtual->eKind == psFunction->eKind) && ((psFunction->eKind == CPP65FK_Destructor) || ((psVirtual->lliName == psFunction->lliName) && SameParameters(psVirtual, psFunction)));
}


//////////////////////////////////////////////////////////////////////////
//
// True if psFunction overrides a virtual function in any of psClass's bases'
// vtables.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::OverridesBase(SCPP65Class* psClass, SCPP65Function* psFunction)
{
	SCPP65Class*	psBase;
	SCPP65Vtable*	psVtable;
	size			i;
	size			j;
	size			k;

	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i)->psClass;
		for (j = 0; j < psBase->mapsVirtuals.NumElements(); j++)
		{
			if (Overrides(psBase->mapsVirtuals.GetPtr(j), psFunction))
			{
				return true;
			}
		}
		for (j = 0; j < psBase->macVtables.NumElements(); j++)
		{
			psVtable = psBase->macVtables.Get(j);
			for (k = 0; k < psVtable->mapsFunctions.NumElements(); k++)
			{
				if (Overrides(psVtable->mapsFunctions.GetPtr(k), psFunction))
				{
					return true;
				}
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The vtables of the polymorphic base subobjects that do not share the class's
// vtable pointer: the bases' own (except the first base's when the class shares it)
// and their secondary vtables, with the class's functions overriding.  The base
// offsets must be known.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::LayoutSecondaryVtables(SCPP65Class* psClass)
{
	SCPP65Base*		psBase;
	SCPP65Vtable*	psVtable;
	bool			bShared;
	size			i;
	size			j;

	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		bShared = (i == 0) && (psClass->psBase == psBase->psClass) && (psBase->psClass->iVtableOffset != -1) && (psClass->iVtableOffset == psBase->psClass->iVtableOffset);
		if (!bShared && (psBase->psClass->iVtableOffset != -1))
		{
			AddSecondaryVtable(psClass, psBase->iOffset, psBase->psClass, &psBase->psClass->mapsVirtuals, NULL);
		}
		for (j = 0; j < psBase->psClass->macVtables.NumElements(); j++)
		{
			psVtable = psBase->psClass->macVtables.Get(j);
			AddSecondaryVtable(psClass, psBase->iOffset + psVtable->iOffset, psVtable->psOwner, &psVtable->mapsFunctions, &psVtable->maiAdjust);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A secondary vtable at iOffset, starting as a base's.  The class's own functions
// override; they need this moved back from the subobject to the class.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddSecondaryVtable(SCPP65Class* psClass, int iOffset, SCPP65Class* psOwner, CArrayCPP65FunctionPtrs* papsFunctions, CArrayInt* paiAdjust)
{
	SCPP65Vtable*		psVtable;
	SCPP65Function*		psFunction;
	size				i;
	size				j;

	psVtable = psClass->macVtables.Add();
	psVtable->iOffset = iOffset;
	psVtable->psOwner = psOwner;
	psVtable->mapsFunctions.Init();
	psVtable->maiAdjust.Init();
	for (i = 0; i < papsFunctions->NumElements(); i++)
	{
		psVtable->mapsFunctions.Add(papsFunctions->GetPtr(i));
		psVtable->maiAdjust.Add(paiAdjust ? paiAdjust->GetValue(i) : 0);
	}

	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass != psClass) || psFunction->bStatic || (psFunction->eKind == CPP65FK_Constructor))
		{
			continue;
		}
		for (j = 0; j < psVtable->mapsFunctions.NumElements(); j++)
		{
			if (Overrides(psVtable->mapsFunctions.GetPtr(j), psFunction))
			{
				psVtable->mapsFunctions.SetPtr(j, psFunction);
				psVtable->maiAdjust.SetValue(j, -iOffset);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if objects of the class have a vtable pointer, its own or a base's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsPolymorphic(SCPP65Class* psClass)
{
	return (psClass->iVtableOffset != -1) || (psClass->macVtables.NumElements() > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function in the class's vtables (its final overriders) is pure
// virtual: no object of the class can be made.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsAbstract(SCPP65Class* psClass)
{
	SCPP65Vtable*	psVtable;
	size			i;
	size			j;

	for (i = 0; i < psClass->mapsVirtuals.NumElements(); i++)
	{
		if (psClass->mapsVirtuals.GetPtr(i)->bPure)
		{
			return true;
		}
	}
	for (i = 0; i < psClass->macVtables.NumElements(); i++)
	{
		psVtable = psClass->macVtables.Get(i);
		for (j = 0; j < psVtable->mapsFunctions.NumElements(); j++)
		{
			if (psVtable->mapsFunctions.GetPtr(j)->bPure)
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__2: the addresses of the virtual functions, written the first time an
// object of the class is made.  Its functions are generated.  Pure virtual
// functions are R_Pure, which stops.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::EnsureVtable(SCPP65Class* psClass)
{
	SCPP65Vtable*	psVtable;
	size			i;
	size			j;
	CChars			szLabel;

	if (psClass->bVtable)
	{
		return;
	}
	psClass->bVtable = true;

	//Each unit that makes one of a class's objects writes its vtable in a COMDAT section
	//(the linker keeps one); a class without a name has its own.
	szLabel.Init();
	AppendClassLabel(&szLabel, psClass, "2");
	if (!Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	if (psClass->lliName < 0)
	{
		AppendSection(&mszData, "rodata", &szLabel, NULL);
	}
	else
	{
		AppendSection(&mszData, "rodata", &szLabel, ", comdat");
		AppendGlobal(&mszData, &szLabel);
	}
	szLabel.Kill();
	mszData.Append("\t\t.align 4");
	mszData.AppendNewLine();
	AppendClassLabel(&mszData, psClass, "2");
	mszData.Append(':');
	mszData.AppendNewLine();
	for (i = 0; i < psClass->mapsVirtuals.NumElements(); i++)
	{
		AppendVtableEntry(psClass->mapsVirtuals.GetPtr(i), 0);
	}
	for (i = 0; i < psClass->macVtables.NumElements(); i++)
	{
		psVtable = psClass->macVtables.Get(i);
		for (j = 0; j < psVtable->mapsFunctions.NumElements(); j++)
		{
			AppendVtableEntry(psVtable->mapsFunctions.GetPtr(j), psVtable->maiAdjust.GetValue(j));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable entry: the function, or a thunk that adds iAdjust to this and jumps to
// it.  Each thunk is written once.  Pure virtual functions are R_Pure.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVtableEntry(SCPP65Function* psFunction, int iAdjust)
{
	CChars	szLabel;

	mszData.Append("\t\t.long ");
	if (psFunction->bPure)
	{
		mszData.Append("R_Pure");
		mbPureVirtual = true;
		mszData.AppendNewLine();
		return;
	}

	RequireFunction(psFunction);
	szLabel.Init();
	AppendFunctionLabel(&szLabel, psFunction);
	if (iAdjust != 0)
	{
		szLabel.Append("__t");
		szLabel.Append(-iAdjust);
		if (Emit(&szLabel))
		{
			if (IsUnitFunction(psFunction))
			{
				AppendSection(&mszThunks, "text", &szLabel, NULL);
			}
			else
			{
				AppendSection(&mszThunks, "text", &szLabel, ", comdat");
				AppendGlobal(&mszThunks, &szLabel);
			}
			mszThunks.Append(szLabel);
			mszThunks.Append(':');
			mszThunks.AppendNewLine();
			if (mpsTarget->eTarget == CPPT_W65C816)
			{
				AppendAdjustThunk816(iAdjust);
				mszThunks.Append("\t\tjml ");
			}
			else
			{
				mszThunks.Append("\t\tadd X, ");
				mszThunks.Append(iAdjust);
				mszThunks.AppendNewLine();
				mszThunks.Append("\t\tjmp ");
			}
			AppendFunctionLabel(&mszThunks, psFunction);
			mszThunks.AppendNewLine();
			mszThunks.AppendNewLine();
		}
	}
	mszData.Append(szLabel);
	mszData.AppendNewLine();
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Points the object's vtable pointers (the object's address is in szObject) at the
// class's vtables, using szValue.  The secondary vtables follow the class's own.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::StoreVtables(SCPP65Class* psClass, const char* szValue, const char* szObject)
{
	SCPP65Vtable*	psVtable;
	size			i;
	int				iEntry;

	EnsureVtable(psClass);
	if (psClass->iVtableOffset != -1)
	{
		Line("cpy ")->Append(szValue)->Append(", ");
		AppendClassLabel(mpszOut, psClass, "2");
		mpszOut->AppendNewLine();
		Line("cpy [")->Append(psClass->iVtableOffset)->Append(" + ")->Append(szObject)->Append("], ")->Append(szValue)->AppendNewLine();
	}
	iEntry = (int)psClass->mapsVirtuals.NumElements();
	for (i = 0; i < psClass->macVtables.NumElements(); i++)
	{
		psVtable = psClass->macVtables.Get(i);
		Line("cpy ")->Append(szValue)->Append(", ");
		AppendClassLabel(mpszOut, psClass, "2");
		mpszOut->Append(" + ")->Append(4 * iEntry)->AppendNewLine();
		Line("cpy [")->Append(psVtable->iOffset + psVtable->psOwner->iVtableOffset)->Append(" + ")->Append(szObject)->Append("], ")->Append(szValue)->AppendNewLine();
		iEntry += (int)psVtable->mapsFunctions.NumElements();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A field of the class, of an anonymous union or struct in it, or of its bases.
// *piOffset is set to its offset in psClass.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Field* CCPP65Generator::FindField(SCPP65Class* psClass, int64 lliName, int* piOffset)
{
	SCPP65Field*	psField;
	SCPP65Field*	psMember;
	SCPP65Base*		psBase;
	size			i;
	int				iOffset;

	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		if (psField->lliName == lliName)
		{
			*piOffset = psField->iOffset;
			return psField;
		}
		if (!psField->pcSymbol && psField->psType->IsClass())
		{
			psMember = FindField(psField->psType->psClass, lliName, &iOffset);
			if (psMember)
			{
				*piOffset = psField->iOffset + iOffset;
				return psMember;
			}
		}
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		psField = FindField(psBase->psClass, lliName, &iOffset);
		if (psField)
		{
			*piOffset = psBase->iOffset + iOffset;
			return psField;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Field* CCPP65Generator::FindFieldSymbol(SCPP65Class* psClass, CCPPSymbol* pcSymbol, int* piOffset)
{
	SCPP65Field*	psField;
	SCPP65Field*	psMember;
	SCPP65Base*		psBase;
	size			i;
	int				iOffset;

	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		if (psField->pcSymbol == pcSymbol)
		{
			*piOffset = psField->iOffset;
			return psField;
		}
		if (!psField->pcSymbol && psField->psType->IsClass())
		{
			psMember = FindFieldSymbol(psField->psType->psClass, pcSymbol, &iOffset);
			if (psMember)
			{
				*piOffset = psField->iOffset + iOffset;
				return psMember;
			}
		}
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		psField = FindFieldSymbol(psBase->psClass, pcSymbol, &iOffset);
		if (psField)
		{
			*piOffset = psBase->iOffset + iOffset;
			return psField;
		}
	}

	//Names in templates are not always resolved to the instance's fields.
	if (psClass->psTemplate)
	{
		return FindField(psClass, pcSymbol->mlliName, piOffset);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A static data member of the class or of its bases.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Variable* CCPP65Generator::FindStatic(SCPP65Class* psClass, int64 lliName)
{
	SCPP65Variable*	psVariable;
	size			i;

	for (i = 0; i < mapsGlobals.NumElements(); i++)
	{
		psVariable = mapsGlobals.GetPtr(i);
		if ((psVariable->psClass == psClass) && (psVariable->pcSymbol->mlliName == lliName))
		{
			return psVariable;
		}
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psVariable = FindStatic(psClass->macBases.Get(i)->psClass, lliName);
		if (psVariable)
		{
			return psVariable;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The error for a call no function of the name matches: the arguments' types
// and, when there is only one function of the name, its parameters'.  An
// unsigned short* passed for an unsigned long* (a 16 bit size where a 32 bit
// lsize is wanted) says so.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::NoFunctionMatches(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, SCPP65Type** apsArguments, size uiArguments, CCPPSyntax* pcNode)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psOnly;
	CChars				szMessage;
	size				i;
	size				uiCandidates;

	psOnly = NULL;
	uiCandidates = 0;
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->lliName == lliName) && (psFunction->eKind == eKind) && !psFunction->psTemplate && IsOverloadOf(psClass, psFunction, lliName))
		{
			psOnly = psFunction;
			uiCandidates++;
		}
	}

	szMessage.Init("No function matches the arguments (");
	for (i = 0; i < uiArguments; i++)
	{
		if (i != 0)
		{
			szMessage.Append(", ");
		}
		apsArguments[i]->Append(&szMessage);
	}
	szMessage.Append(")");
	if (uiCandidates == 1)
	{
		szMessage.Append(": it takes (");
		for (i = 0; i < psOnly->mapsParameters.NumElements(); i++)
		{
			if (i != 0)
			{
				szMessage.Append(", ");
			}
			psOnly->mapsParameters.GetPtr(i)->Append(&szMessage);
		}
		szMessage.Append(psOnly->bVariadic ? ", ...)" : ")");
	}
	Error(pcNode, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Overload resolution: of the functions of the name (in the class, then its bases if
// bBases) that take the number of arguments (default arguments make the rest), the
// one whose every argument converts at least as well as for any other, and one
// better (see ConversionRank).  Returns NULL with an error if no one is best, or
// none matches functions of the name; NULL without an error if there are none.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, int iConstObject)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psBest;
	SCPP65Type*			apsArguments[32];
	int					aiRanks[32];
	int					aiBest[32];
	size				uiArguments;
	size				i;
	bool				bAmbiguous;
	bool				bFound;

	if (psClass && !CompleteClass(psClass, pcNode))
	{
		return NULL;
	}
	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	if (uiArguments > 32)
	{
		Error(pcNode, "Too many arguments");
		return NULL;
	}
	for (i = 0; i < uiArguments; i++)
	{
		apsArguments[i] = TypeOf(papcArguments->GetPtr(i));
		if (!apsArguments[i])
		{
			Expression(papcArguments->GetPtr(i));
			return NULL;
		}
	}

	//The best is found by comparing each viable function with the best so far, then
	//checked to be better than every other.
	psBest = NULL;
	bFound = false;
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->lliName != lliName) || (psFunction->eKind != eKind) || psFunction->psTemplate || !IsOverloadOf(psClass, psFunction, lliName))
		{
			continue;
		}
		bFound = true;
		if (!RankArguments(psFunction, apsArguments, papcArguments, aiRanks))
		{
			continue;
		}
		if (!psBest || (CompareRanks(aiRanks, aiBest, uiArguments) > 0) || ((CompareRanks(aiRanks, aiBest, uiArguments) == 0) && (CompareConstReferences(psFunction, psBest, papcArguments, iConstObject) > 0)))
		{
			psBest = psFunction;
			memcpy(aiBest, aiRanks, sizeof(int) * uiArguments);
		}
	}
	bAmbiguous = false;
	for (i = 0; psBest && (i < mapsFunctions.NumElements()); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction == psBest) || (psFunction->lliName != lliName) || (psFunction->eKind != eKind) || psFunction->psTemplate || !IsOverloadOf(psClass, psFunction, lliName))
		{
			continue;
		}
		if (RankArguments(psFunction, apsArguments, papcArguments, aiRanks) && (CompareRanks(aiBest, aiRanks, uiArguments) <= 0))
		{
			if ((CompareRanks(aiBest, aiRanks, uiArguments) < 0) || (CompareConstReferences(psBest, psFunction, papcArguments, iConstObject) <= 0))
			{
				bAmbiguous = true;
			}
		}
	}

	if (psBest && !bAmbiguous)
	{
		return psBest;
	}
	if (bAmbiguous)
	{
		Error(pcNode, "The call is ambiguous");
		return NULL;
	}
	if (bFound)
	{
		NoFunctionMatches(psClass, lliName, eKind, apsArguments, uiArguments, pcNode);
		return NULL;
	}
	if (bBases && psClass)
	{
		for (i = 0; i < psClass->macBases.NumElements(); i++)
		{
			psFunction = FindFunction(psClass->macBases.Get(i)->psClass, lliName, eKind, papcArguments, true, pcNode, iConstObject);
			if (psFunction)
			{
				return psFunction;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The rank of each argument's conversion to its parameter, in aiRanks.  Returns
// false if the function cannot take the arguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::RankArguments(SCPP65Function* psFunction, SCPP65Type** apsArguments, CArraySyntax* papcArguments, int* aiRanks)
{
	size	uiArguments;
	size	uiRequired;
	size	i;

	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	uiRequired = psFunction->mapsParameters.NumElements();
	while ((uiRequired > 0) && psFunction->mapcDefaults.GetPtr(uiRequired - 1))
	{
		uiRequired--;
	}
	if ((uiArguments < uiRequired) || ((uiArguments > psFunction->mapsParameters.NumElements()) && !psFunction->bVariadic))
	{
		return false;
	}
	for (i = 0; i < uiArguments; i++)
	{
		aiRanks[i] = (i < psFunction->mapsParameters.NumElements()) ? ConversionRank(psFunction->mapsParameters.GetPtr(i), apsArguments[i], papcArguments->GetPtr(i)) : CPP65CR_Ellipsis;
		if (aiRanks[i] == CPP65CR_None)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// How well an argument converts to a parameter: an exact match (the same type, an
// array or function to a pointer to it), a promotion (bool, char, short and enums to
// int, float to double), a conversion (between other arithmetic types, a derived class
// or its pointer to a base, a pointer to void*, 0 to a pointer, a pointer to bool),
// a user defined conversion (a constructor of the parameter's class, only when
// bUserDefined) or none.  A reference parameter matches its referred type.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::ConversionRank(SCPP65Type* psParameter, SCPP65Type* psArgument, CCPPSyntax* pcArgument, bool bUserDefined)
{
	int64			lliValue;
	bool			bReference;
	SCPP65Function*	psConversion;

	bReference = psParameter->IsReference();
	psParameter = bReference ? psParameter->psBase : psParameter;
	psArgument = psArgument->IsReference() ? psArgument->psBase : psArgument;
	if (psParameter->Equals(psArgument))
	{
		return CPP65CR_Exact;
	}
	if (psParameter->IsPointer() && (psArgument->IsArray() || psArgument->IsFunction()) && psParameter->psBase->Equals(psArgument->IsArray() ? psArgument->psBase : psArgument))
	{
		return CPP65CR_Exact;
	}
	if ((psParameter->IsInteger() || psParameter->IsFloat()) && (psArgument->IsInteger() || psArgument->IsFloat()))
	{
		if (psParameter->pcEnum)
		{
			//Only the same enum (an exact match) converts to an enum.
			return CPP65CR_None;
		}
		if ((psParameter == &mcTypes.msInt) && psArgument->IsInteger() && ((psArgument->iBytes < 4) || psArgument->pcEnum))
		{
			return CPP65CR_Promotion;
		}
		if (psParameter->IsFloat() && (psParameter->iBytes == 8) && !psParameter->bLongDouble && psArgument->IsFloat() && (psArgument->iBytes == 4))
		{
			return CPP65CR_Promotion;
		}
		return CPP65CR_Conversion;
	}
	if (psParameter->IsPointer() && psArgument->IsPointer())
	{
		if (psParameter->psBase->IsVoid() || (psParameter->psBase->IsClass() && psArgument->psBase->IsClass() && IsDerived(psArgument->psBase->psClass, psParameter->psBase->psClass, NULL)))
		{
			return CPP65CR_Conversion;
		}
		return CPP65CR_None;
	}
	if (psParameter->IsMemberPointer() && psArgument->IsMemberPointer())
	{
		//A pointer to a member of a base converts to a pointer to the same member of a derived class.
		if (psParameter->psClass && psArgument->psClass && psParameter->psBase->Equals(psArgument->psBase) && IsDerived(psParameter->psClass, psArgument->psClass, NULL))
		{
			return CPP65CR_Conversion;
		}
		return CPP65CR_None;
	}
	if ((psParameter->IsPointer() || psParameter->IsMemberPointer()) && psArgument->IsInteger() && pcArgument && Constant(pcArgument, &lliValue) && (lliValue == 0))
	{
		return CPP65CR_Conversion;
	}
	if ((psParameter->eKind == CPP65TK_Bool) && (psArgument->IsPointer() || psArgument->IsMemberPointer()))
	{
		return CPP65CR_Boolean;
	}
	if (psParameter->IsClass() && psArgument->IsClass() && IsDerived(psArgument->psClass, psParameter->psClass, NULL))
	{
		return CPP65CR_Conversion;
	}
	if (bUserDefined && psParameter->IsClass() && HasConvertingConstructor(psParameter->psClass, psArgument, pcArgument))
	{
		return CPP65CR_UserDefinedExact;
	}
	if (bUserDefined && !bReference && !psParameter->IsClass() && psArgument->IsClass())
	{
		//The conversion function's result is then converted to the parameter, which tells overloads apart.
		psConversion = FindConversion(psArgument->psClass, psParameter, false, pcArgument);
		if (psConversion)
		{
			return (ConversionRank(psParameter, psConversion->psReturn, NULL, false) >= CPP65CR_Promotion) ? CPP65CR_UserDefinedExact : CPP65CR_UserDefined;
		}
	}
	return CPP65CR_None;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psClass has a constructor that takes one argument of psArgument's type
// through a standard conversion (any other parameters have defaults).  It is not
// the class's copy constructor or explicit.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::HasConvertingConstructor(SCPP65Class* psClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument)
{
	SCPP65Function*	psFunction;
	SCPP65Type*		psParameter;
	size			i;

	if (!CompleteClass(psClass, pcArgument) || !psClass->bConstructors)
	{
		return false;
	}
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass != psClass) || (psFunction->eKind != CPP65FK_Constructor) || psFunction->psTemplate || psFunction->bExplicit || (psFunction->mapsParameters.NumElements() == 0))
		{
			continue;
		}
		if ((psFunction->mapsParameters.NumElements() > 1) && !psFunction->mapcDefaults.GetPtr(1))
		{
			continue;
		}
		psParameter = psFunction->mapsParameters.GetPtr(0);
		psParameter = psParameter->IsReference() ? psParameter->psBase : psParameter;
		if (psParameter->IsClass() && (psParameter->psClass == psClass))
		{
			continue;
		}
		if (ConversionRank(psFunction->mapsParameters.GetPtr(0), psArgument, pcArgument, false) != CPP65CR_None)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// 1 if the first function's arguments convert better (none worse, one better), -1
// if the second's do, 0 if neither.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::CompareRanks(int* aiFirst, int* aiSecond, size uiArguments)
{
	size	i;
	bool	bFirst;
	bool	bSecond;

	bFirst = false;
	bSecond = false;
	for (i = 0; i < uiArguments; i++)
	{
		bFirst |= aiFirst[i] > aiSecond[i];
		bSecond |= aiSecond[i] > aiFirst[i];
	}
	if (bFirst && !bSecond)
	{
		return 1;
	}
	if (bSecond && !bFirst)
	{
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Between two functions whose arguments convert equally well, which binds them to
// references better: an object made by the argument (a temporary) or a const object
// binds better to a const reference, others better to a reference that is not
// const.  The object a member function is called on (iConstObject 1 if it is const,
// 0 if not, -1 for none) is bound to this the same way.  1 if the first does, -1 if
// the second does, 0 if neither.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::CompareConstReferences(SCPP65Function* psFirst, SCPP65Function* psSecond, CArraySyntax* papcArguments, int iConstObject)
{
	uint32		uiDifferent;
	size		i;
	bool		bFirst;
	bool		bSecond;
	bool		bConstBetter;
	SCPP65Type*	psParameter;

	uiDifferent = psFirst->uiConstReferences ^ psSecond->uiConstReferences;
	bFirst = false;
	bSecond = false;
	if ((iConstObject >= 0) && (psFirst->bConstThis != psSecond->bConstThis))
	{
		if (psFirst->bConstThis == (iConstObject == 1))
		{
			bFirst = true;
		}
		else
		{
			bSecond = true;
		}
	}
	for (i = 0; uiDifferent && papcArguments && (i < papcArguments->NumElements()) && (i < 32); i++)
	{
		if (!(uiDifferent & (1u << i)) || (i >= psFirst->mapsParameters.NumElements()) || (i >= psSecond->mapsParameters.NumElements()))
		{
			continue;
		}
		psParameter = psFirst->mapsParameters.GetPtr(i);
		if (!psParameter->IsReference() || !psSecond->mapsParameters.GetPtr(i)->IsReference())
		{
			continue;
		}
		bConstBetter = (psParameter->psBase->IsClass() && IsMadeObject(papcArguments->GetPtr(i), psParameter->psBase->psClass)) || IsConstObject(papcArguments->GetPtr(i));
		if (((psFirst->uiConstReferences & (1u << i)) != 0) == bConstBetter)
		{
			bFirst = true;
		}
		else
		{
			bSecond = true;
		}
	}
	if (bFirst && !bSecond)
	{
		return 1;
	}
	if (bSecond && !bFirst)
	{
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is an object that is const: a variable, parameter or field
// declared const (const T or const T&), a field of a const object, a field in a
// const member function, or what a pointer to const points to (*p, p->field).
// Types do not record const, so this is worked out from the declarations.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstObject(CCPPSyntax* pcExpression)
{
	CCPPSyntaxMemberExpression*	pcMember;
	CCPPSyntaxUnaryExpression*	pcUnary;
	CCPPSymbol*					pcSymbol;
	SCPP65Type*					psType;
	SCPP65Field*				psField;
	int							iOffset;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
		pcSymbol = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol;
		if (!pcSymbol)
		{
			return false;
		}
		if ((pcSymbol->meKind == CPPSK_Variable) || (pcSymbol->meKind == CPPSK_Parameter))
		{
			return IsConstDeclared(pcSymbol, false);
		}
		if (pcSymbol->meKind == CPPSK_Field)
		{
			return IsConstDeclared(pcSymbol, false) || IsConstThis();
		}
		return false;
	case CPPS_MemberExpression:
		pcMember = (CCPPSyntaxMemberExpression*)pcExpression;
		pcSymbol = pcMember->mpcName->mpcSymbol;
		if (!pcSymbol)
		{
			//The parser leaves object.field unresolved; the field is the object's class's.
			psType = pcMember->mbArrow ? ArrowType(pcMember->mpcObject, pcMember) : TypeOf(pcMember->mpcObject);
			psType = (psType && (psType->IsPointer() || psType->IsReference())) ? psType->psBase : psType;
			psField = (psType && psType->IsClass()) ? FindField(psType->psClass, GetComponentName(pcMember->mpcName->GetLast()), &iOffset) : NULL;
			pcSymbol = psField ? psField->pcSymbol : NULL;
		}
		if (pcSymbol && (pcSymbol->meKind == CPPSK_Field) && IsConstDeclared(pcSymbol, false))
		{
			return true;
		}
		return pcMember->mbArrow ? IsConstPointer(pcMember->mpcObject) : IsConstObject(pcMember->mpcObject);
	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		return (pcUnary->meOperator == CT_Multiply) && IsConstPointer(pcUnary->mpcOperand);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is a pointer to const: this in a const member function, or
// a variable, parameter or field declared const T*.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstPointer(CCPPSyntax* pcExpression)
{
	CCPPSymbol*	pcSymbol;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}
	if (pcExpression->GetKind() == CPPS_ThisExpression)
	{
		return IsConstThis();
	}
	if (pcExpression->GetKind() != CPPS_IdExpression)
	{
		return false;
	}
	pcSymbol = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol;
	if (pcSymbol && ((pcSymbol->meKind == CPPSK_Variable) || (pcSymbol->meKind == CPPSK_Parameter) || (pcSymbol->meKind == CPPSK_Field)))
	{
		return IsConstDeclared(pcSymbol, true);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a variable, parameter or field is declared const T or const T& (or, when
// bPointer, const T*).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstDeclared(CCPPSymbol* pcSymbol, bool bPointer)
{
	CCPPSyntax*					pcDeclaration;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxDeclarator*		pcDeclarator;
	ECPPPointer					eKind;
	size						uiPrefix;

	pcDeclaration = pcSymbol ? pcSymbol->mpcDeclaration : NULL;
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator) && pcDeclaration->mpcParent && (pcDeclaration->mpcParent->GetKind() == CPPS_SimpleDeclaration))
	{
		pcSpecifiers = ((CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcParent)->mpcSpecifiers;
		pcDeclarator = ((CCPPSyntaxInitDeclarator*)pcDeclaration)->mpcDeclarator;
	}
	else if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_Parameter))
	{
		pcSpecifiers = ((CCPPSyntaxParameter*)pcDeclaration)->mpcSpecifiers;
		pcDeclarator = ((CCPPSyntaxParameter*)pcDeclaration)->mpcDeclarator;
	}
	else
	{
		return false;
	}
	if (!pcSpecifiers || !(pcSpecifiers->muiFlags & CPPDS_CONST))
	{
		return false;
	}
	uiPrefix = pcDeclarator ? pcDeclarator->macpPrefix.NumElements() : 0;
	eKind = (uiPrefix == 1) ? ((CCPPSyntaxPointerOperator*)pcDeclarator->macpPrefix.GetPtr(0))->meKind : CPPP_Pointer;
	if (bPointer)
	{
		return (uiPrefix == 1) && (eKind == CPPP_Pointer);
	}
	return (uiPrefix == 0) || ((uiPrefix == 1) && ((eKind == CPPP_Reference) || (eKind == CPPP_RValueReference)));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function being generated is a const member function.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstThis(void)
{
	return mpsFunction && mpsFunction->bConstThis;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psClass is psBase or derives from it.  *piOffset (if not NULL) is set to
// where psBase is in psClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsDerived(SCPP65Class* psClass, SCPP65Class* psBase, int* piOffset)
{
	SCPP65Base*	psDirect;
	size		i;
	int			iOffset;

	if ((psClass != psBase) && !psClass->bComplete)
	{
		//The bases are enough to know; their offsets need the class laid out.
		BeginQuiet();
		ResolveBases(psClass, NULL);
		EndQuiet();
	}
	if (psClass == psBase)
	{
		if (piOffset)
		{
			*piOffset = 0;
		}
		return true;
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psDirect = psClass->macBases.Get(i);
		if (IsDerived(psDirect->psClass, psBase, &iOffset))
		{
			if (piOffset)
			{
				*piOffset = psDirect->iOffset + iOffset;
			}
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether making an object of the class runs code: a constructor, a vtable pointer
// to set, a base or a field that needs it, or a default member initialiser.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::NeedsConstructor(SCPP65Class* psClass)
{
	SCPP65Field*	psField;
	SCPP65Type*		psType;
	size			i;

	if (psClass->bConstructors || IsPolymorphic(psClass))
	{
		return true;
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		if (NeedsConstructor(psClass->macBases.Get(i)->psClass))
		{
			return true;
		}
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		if (psField->pcInitialiser)
		{
			return true;
		}
		psType = psField->psType->IsArray() ? psField->psType->psBase : psField->psType;
		if (psType->IsClass() && NeedsConstructor(psType->psClass))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether destroying an object of the class runs code.  While the class is being
// laid out this asks whether its bases or fields need destroying.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::NeedsDestructor(SCPP65Class* psClass)
{
	SCPP65Field*	psField;
	SCPP65Type*		psType;
	size			i;

	if (psClass->psDestructor)
	{
		return true;
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		if (psClass->macBases.Get(i)->psClass->psDestructor)
		{
			return true;
		}
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		psType = psField->psType->IsArray() ? psField->psType->psBase : psField->psType;
		if (psType->IsClass() && psType->psClass->psDestructor)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Functions are _name, operators _opName.  Members are _Class__name, constructors
// _Class__0 and destructors _Class__1; the vtable is _Class__2.  A template
// instance's class is Class__T and its arguments, mangled.  An overload has __n on
// the end.  C++ names cannot contain __ and cannot start with a digit, so none can
// clash.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendFunctionLabel(CChars* psz, SCPP65Function* psFunction)
{
	CChars	szName;
	size	i;
	bool	bPlainName;

	szName.Init();
	if (psFunction->eKind == CPP65FK_Constructor)
	{
		szName.Append('0');
	}
	else if (psFunction->eKind == CPP65FK_Destructor)
	{
		szName.Append('1');
	}
	else
	{
		AppendFunctionName(&szName, psFunction->lliName);
	}

	//main, and the global operator new and delete the generator calls by name, are just their names.
	bPlainName = !psFunction->psClass && (szName.Equals("main") ||
			((psFunction->mapsParameters.NumElements() == 1) && (szName.Equals("opNew") || szName.Equals("opNewArray") || szName.Equals("opDelete") || szName.Equals("opDeleteArray"))));
	if (psFunction->psClass)
	{
		AppendClassLabel(psz, psFunction->psClass, szName.Text());
	}
	else
	{
		psz->Append('_');
		psz->Append(&szName);
	}
	szName.Kill();

	if (psFunction->mszArguments.Length() > 0)
	{
		psz->Append("__T");
		psz->Append(&psFunction->mszArguments);
	}
	if (((!psFunction->psClass && !psFunction->psTemplate) || (psFunction->mszArguments.Length() > 0)) && !psFunction->bCLinkage && !bPlainName)
	{
		//A function that is not a member has its parameters in its label, as units can declare different sets of
		//overloads.  Without parameters (only one such overload can exist) or with C linkage it is just its name.
		//So does an instance of a function template, as Add<M>(void) and Add<M>(size) are both Add__T_..., and a
		//const member function template's instance ends in K.
		if (psFunction->bConstThis)
		{
			psz->Append("__K");
		}
		if (psFunction->mapsParameters.NumElements() > 0)
		{
			psz->Append("__F");
			for (i = 0; i < psFunction->mapsParameters.NumElements(); i++)
			{
				if ((i < 32) && (psFunction->uiConstReferences & (1u << i)))
				{
					//const T& is KR, as T& and const T& are different overloads.
					psz->Append('K');
				}
				AppendMangledType(psz, psFunction->mapsParameters.GetPtr(i));
			}
		}
	}
	else if (psFunction->iOverload > 0)
	{
		psz->Append("__");
		psz->Append(psFunction->iOverload);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__part.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendClassLabel(CChars* psz, SCPP65Class* psClass, const char* szPart)
{
	psz->Append('_');
	AppendClassName(psz, psClass);
	psz->Append("__");
	psz->Append(szPart);
}


//////////////////////////////////////////////////////////////////////////
//
// Class, or Class__T and the template arguments.  A class without a name is the
// unit, Unnamed and its number.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendClassName(CChars* psz, SCPP65Class* psClass)
{
	char*	szName;
	int		iLength;

	if (psClass->lliName < 0)
	{
		//A class without a name is the unit's own (its labels are local to the object).
		psz->Append("Unnamed");
		psz->Append((int)mapsClasses.Find(psClass));
		return;
	}
	szName = mpcTokeniser->GetIdentifier(psClass->lliName, &iLength);
	psz->Append(szName, iLength);
	if (psClass->mszArguments.Length() > 0)
	{
		psz->Append("__T");
		psz->Append(&psClass->mszArguments);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A function's name: its identifier, or op and the operator's name.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendFunctionName(CChars* psz, int64 lliName)
{
	char*	szName;
	int		iLength;

	if (lliName == CPP65_CONVERSION_NAME)
	{
		psz->Append("opConvert");
		return;
	}
	if (lliName < 0)
	{
		psz->Append("op");
		psz->Append(GetOperatorLabel(lliName));
		return;
	}
	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	psz->Append(szName, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Types in labels: v void, b bool, c char, h unsigned char, s short, t unsigned
// short, i int, j unsigned, x long long, y unsigned long long, f float, d double,
// e long double, P pointer, R reference, A count _ array, F return parameters E
// function, and C, the length and the label of a class.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendMangledType(CChars* psz, SCPP65Type* psType)
{
	CChars	szClass;
	size	i;
	char*	szIdentifier;
	int		iLength;

	switch (psType->eKind)
	{
	case CPP65TK_Void:
		psz->Append('v');
		break;
	case CPP65TK_Bool:
		psz->Append('b');
		break;
	case CPP65TK_Integer:
		if (psType->pcEnum)
		{
			szIdentifier = mpcTokeniser->GetIdentifier(psType->pcEnum->mlliName, &iLength);
			psz->Append('N');
			psz->Append(iLength);
			psz->Append(szIdentifier, iLength);
			break;
		}
		switch (psType->iBytes)
		{
		case 1:		psz->Append(psType->bSigned ? 'c' : 'h');	break;
		case 2:		psz->Append(psType->bSigned ? 's' : 't');	break;
		case 8:		psz->Append(psType->bSigned ? 'x' : 'y');	break;
		default:	psz->Append(psType->bSigned ? 'i' : 'j');	break;
		}
		break;
	case CPP65TK_Float:
		psz->Append((psType->iBytes == 4) ? 'f' : (psType->bLongDouble ? 'e' : 'd'));
		break;
	case CPP65TK_Pointer:
		psz->Append('P');
		AppendMangledType(psz, psType->psBase);
		break;
	case CPP65TK_Reference:
		psz->Append('R');
		AppendMangledType(psz, psType->psBase);
		break;
	case CPP65TK_Array:
		psz->Append('A');
		psz->Append(psType->iCount);
		psz->Append('_');
		AppendMangledType(psz, psType->psBase);
		break;
	case CPP65TK_Function:
		psz->Append('F');
		AppendMangledType(psz, psType->psBase);
		for (i = 0; i < psType->papsParameters->NumElements(); i++)
		{
			AppendMangledType(psz, psType->papsParameters->GetPtr(i));
		}
		psz->Append('E');
		break;
	case CPP65TK_Class:
		szClass.Init();
		AppendClassName(&szClass, psType->psClass);
		psz->Append('C');
		psz->Append((int)szClass.Length());
		psz->Append(&szClass);
		szClass.Kill();
		break;
	case CPP65TK_MemberPointer:
		psz->Append('M');
		AppendMangledType(psz, psType->psClass->psType);
		AppendMangledType(psz, psType->psBase);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A function name component's name: its identifier, or a negative number for an
// operator (see GetOperatorLabel).  -1 if it is neither.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::GetFunctionName(CCPPSyntaxNameComponent* pcComponent)
{
	CCPPToken*	pcToken;
	ECppTokens	eToken;
	bool		bArray;

	if (pcComponent && (pcComponent->meKind == CPPNC_Conversion))
	{
		return CPP65_CONVERSION_NAME;
	}
	if (!pcComponent || (pcComponent->meKind != CPPNC_Operator))
	{
		return GetComponentName(pcComponent);
	}
	pcToken = mpcTokeniser->Get(pcComponent->miName);
	if (!pcToken || !pcToken->IsReservedWord())
	{
		return -1;
	}
	eToken = ((CCTReservedWord*)pcToken)->GetToken();
	bArray = (pcComponent->miLastToken > pcComponent->miName) && ((eToken == CT_new) || (eToken == CT_delete));
	return GetOperatorName(eToken, bArray);
}


//////////////////////////////////////////////////////////////////////////
//
// Operators are named by their token: -1 - token, and -1 - NUM_CPP_TOKENS - token
// for new[] and delete[].
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::GetOperatorName(ECppTokens eToken, bool bArray)
{
	return bArray ? (-1 - NUM_CPP_TOKENS - (int64)eToken) : (-1 - (int64)eToken);
}


//////////////////////////////////////////////////////////////////////////
//
// The operator's part of a label: _Class__opIndex for operator[].
//
//////////////////////////////////////////////////////////////////////////
const char* CCPP65Generator::GetOperatorLabel(int64 lliName)
{
	ECppTokens	eToken;
	bool		bArray;

	bArray = lliName < -1 - NUM_CPP_TOKENS;
	eToken = (ECppTokens)(bArray ? (-1 - NUM_CPP_TOKENS - lliName) : (-1 - lliName));
	switch (eToken)
	{
	case CT_OpenSquareBracket:			return "Index";
	case CT_OpenRoundBracket:			return "Call";
	case CT_Add:						return "Add";
	case CT_Subtract:					return "Subtract";
	case CT_Multiply:					return "Multiply";
	case CT_Divide:						return "Divide";
	case CT_Modulus:					return "Modulus";
	case CT_BitwiseAnd:					return "And";
	case CT_BitwiseOr:					return "Or";
	case CT_BitwiseXor:					return "Xor";
	case CT_BitwiseNot:					return "Not";
	case CT_LogicalNot:					return "LogicalNot";
	case CT_LogicalAnd:					return "LogicalAnd";
	case CT_LogicalOr:					return "LogicalOr";
	case CT_LeftShift:					return "ShiftLeft";
	case CT_RightShift:					return "ShiftRight";
	case CT_Assignment:					return "Assign";
	case CT_AssignmentAdd:				return "AddAssign";
	case CT_AssignmentSubtract:			return "SubtractAssign";
	case CT_AssignmentMultiply:			return "MultiplyAssign";
	case CT_AssignmentDivide:			return "DivideAssign";
	case CT_AssignmentModulus:			return "ModulusAssign";
	case CT_AssignmentBitwiseAnd:		return "AndAssign";
	case CT_AssignmentBitwiseOr:		return "OrAssign";
	case CT_AssignmentBitwiseXor:		return "XorAssign";
	case CT_AssignmentLeftShift:		return "ShiftLeftAssign";
	case CT_AssignmentRightShift:		return "ShiftRightAssign";
	case CT_EqualTo:					return "Equal";
	case CT_NotEqualTo:					return "NotEqual";
	case CT_LessThan:					return "Less";
	case CT_LessThanEqualTo:			return "LessEqual";
	case CT_GreaterThan:				return "Greater";
	case CT_GreaterThanEqualTo:			return "GreaterEqual";
	case CT_Increment:					return "Increment";
	case CT_Decrement:					return "Decrement";
	case CT_MemberByPointer:			return "Arrow";
	case CT_Comma:						return "Comma";
	case CT_new:						return bArray ? "NewArray" : "New";
	case CT_delete:						return bArray ? "DeleteArray" : "Delete";
	default:							return "Operator";
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Records a function's definition.  Functions defined in classes, inline,
// constexpr and template functions are generated when they are first called.
// Bodies that were skipped (in system headers) are not definitions; = default
// makes the generator write a constructor or destructor.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetDefinition(SCPP65Function* psFunction, CCPPSyntaxFunctionDefinition* pcDefinition)
{
	if ((pcDefinition->meBody == CPPFB_Default) && (psFunction->eKind != CPP65FK_Function))
	{
		psFunction->bImplicit = true;
		psFunction->bDefined = true;
		return;
	}
	if (pcDefinition->meBody != CPPFB_Body)
	{
		return;
	}
	psFunction->pcDefinition = pcDefinition;
	psFunction->bDefined = true;
	psFunction->bInline = true;
	if (!psFunction->psBindings && psFunction->psClass)
	{
		psFunction->psBindings = psFunction->psClass->psBindings;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Initialises the object at a place from a declaration's initialiser (or a member
// initialiser): nothing, (arguments), {arguments}, = value or = Class(arguments).
// Scalars are stored, arrays are initialised element by element, objects are
// constructed or copied.  papcArguments is used when pcInitialiser is NULL.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Construct(SCPP65Place* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode)
{
	SCPP65Type*						psType;
	SCPP65Type*						psSource;
	SCPP65Place						sElement;
	CArraySyntax					apcArguments;
	CCPPSyntaxConstructExpression*	pcConstruct;
	SCPP65Class*					psClass;
	int								i;
	bool							bResult;
	ECPP65Elide						eElide;
	int								iElideDepth;
	bool							bValueInitialise;

	psType = psPlace->psType;
	apcArguments.Init();
	if (!papcArguments)
	{
		papcArguments = &apcArguments;
	}

	//How an Address place can be reached again, from the caller (only for this place).
	eElide = (psPlace->eKind == CPP65PK_Address) ? meNextElide : CPP65E_Place;
	iElideDepth = miNextElideDepth;
	meNextElide = CPP65E_None;

	//A reference is bound to an object's address.
	if (psType->IsReference())
	{
		bResult = ConstructReference(psPlace, papcArguments, pcInitialiser, pcNode);
		apcArguments.Kill();
		return bResult;
	}

	bValueInitialise = false;
	if (pcInitialiser)
	{
		switch (pcInitialiser->GetKind())
		{
		case CPPS_ParenthesisedInitializer:
			papcArguments = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
			pcInitialiser = NULL;
			bValueInitialise = papcArguments->NumElements() == 0;
			break;
		case CPPS_InitializerList:
			papcArguments = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
			pcInitialiser = NULL;
			bValueInitialise = papcArguments->NumElements() == 0;
			break;
		default:
			break;
		}
	}

	if (psType->IsArray())
	{
		bResult = true;
		if (pcInitialiser || (papcArguments->NumElements() > (size)psType->iCount))
		{
			apcArguments.Kill();
			return Error(pcNode, "This array initialiser is not supported by the compiler yet");
		}
		sElement = *psPlace;
		sElement.psType = psType->psBase;
		for (i = 0; i < psType->iCount; i++)
		{
			if ((size)i < papcArguments->NumElements())
			{
				bResult &= Construct(&sElement, NULL, papcArguments->GetPtr(i), pcNode);
			}
			else if (psType->psBase->IsClass())
			{
				bResult &= Construct(&sElement, NULL, NULL, pcNode);
			}
			else
			{
				Line("cpy A, 0")->AppendNewLine();
				Store(&sElement, "A", NULL);
			}
			sElement.iOffset += psType->psBase->iBytes;
		}
		apcArguments.Kill();
		return bResult;
	}

	if (!psType->IsClass())
	{
		if (!pcInitialiser)
		{
			if (papcArguments->NumElements() > 1)
			{
				apcArguments.Kill();
				return Error(pcNode, "Expected one initialiser");
			}
			pcInitialiser = (papcArguments->NumElements() == 1) ? papcArguments->GetPtr(0) : NULL;
		}
		//An address in A waits in X.
		if (psPlace->eKind == CPP65PK_Address)
		{
			Push("A");
		}
		if (pcInitialiser)
		{
			bResult = Convert(ValueFor(pcInitialiser, psType, false), psType, pcInitialiser);
		}
		else
		{
			Line("cpy A, 0")->AppendNewLine();
			bResult = true;
		}
		if (psPlace->eKind == CPP65PK_Address)
		{
			Pull("X");
			ChangeX();
		}
		if (bResult)
		{
			Store(psPlace, "A", "X");
		}
		apcArguments.Kill();
		return bResult;
	}

	psClass = psType->psClass;
	if (!CompleteClass(psClass, pcNode))
	{
		apcArguments.Kill();
		return false;
	}

	//(object) copies it, as = object does.
	if (!pcInitialiser && (papcArguments->NumElements() == 1) && !psClass->bConstructors)
	{
		psSource = TypeOf(papcArguments->GetPtr(0));
		if (psSource && psSource->IsReference())
		{
			psSource = psSource->psBase;
		}
		if (psSource && psSource->IsClass() && IsDerived(psSource->psClass, psClass, NULL))
		{
			pcInitialiser = papcArguments->GetPtr(0);
		}
	}
	if (pcInitialiser)
	{
		//= Class(arguments) constructs in place.
		if ((pcInitialiser->GetKind() == CPPS_ConstructExpression))
		{
			pcConstruct = (CCPPSyntaxConstructExpression*)pcInitialiser;
			if ((GetTypeIdType(pcConstruct->mpcType) == psType) && pcConstruct->mpcInitializer && (pcConstruct->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer))
			{
				bResult = ConstructClass(psPlace, psClass, &((CCPPSyntaxParenthesisedInitializer*)pcConstruct->mpcInitializer)->macpElements, pcNode);
				apcArguments.Kill();
				return bResult;
			}
		}

		//= object copies it, = value calls a constructor that takes it.
		psSource = TypeOf(pcInitialiser);
		if (psSource && psSource->IsReference())
		{
			psSource = psSource->psBase;
		}
		if (psSource && psSource->IsClass() && IsDerived(psSource->psClass, psClass, NULL))
		{
			if ((eElide != CPP65E_None) && IsMadeObject(pcInitialiser, psClass))
			{
				//Made in place, not copied.
				bResult = ConstructFromMade(psPlace, pcInitialiser, eElide, iElideDepth, pcNode);
				apcArguments.Kill();
				return bResult;
			}
			bResult = CopyConstruct(psPlace, pcInitialiser, pcNode);
			apcArguments.Kill();
			return bResult;
		}
		apcArguments.Add(pcInitialiser);
		papcArguments = &apcArguments;
	}

	//() and {} value initialise: an object without a constructor of its own is zeroed first.
	if (bValueInitialise && !psClass->bConstructors)
	{
		ZeroObject(psPlace, psType->iBytes);
	}
	bResult = ConstructClass(psPlace, psClass, papcArguments, pcNode);
	apcArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Zeroes iBytes at a place (an Address place's address stays in A): small
// objects in place, bigger ones with memset.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ZeroObject(SCPP65Place* psPlace, int iBytes)
{
	int		iDepth;
	int		iOffset;
	int		iChunk;

	if (psPlace->eKind != CPP65PK_Address)
	{
		LoadAddress(psPlace);
	}
	Push("A");
	iDepth = miDepth;
	if (iBytes <= CPP65_INLINE_ZERO)
	{
		Line("cpy Y, 0")->AppendNewLine();
		for (iOffset = 0; iOffset < iBytes; iOffset += iChunk)
		{
			iChunk = (iBytes - iOffset >= 4) ? 4 : ((iBytes - iOffset >= 2) ? 2 : 1);
			if (iChunk < 4)
			{
				Line("wth Y, ")->Append(8 * iChunk)->AppendNewLine();
			}
			Line("cpy [")->Append(iOffset)->Append(" + A], Y")->AppendNewLine();
			if (iChunk < 4)
			{
				Line("wth Y, 32")->AppendNewLine();
			}
		}
	}
	else
	{
		Line("cpy A, ")->Append(iBytes)->AppendNewLine();
		Push("A");
		Line("cpy A, 0")->AppendNewLine();
		Push("A");
		Line("cpy A, [")->Append(1 + miDepth - iDepth)->Append(" + S]")->AppendNewLine();
		Push("A");
		CallRuntime("memset", 12);
	}
	Pull("A");
}


//////////////////////////////////////////////////////////////////////////
//
// Binds a reference: its place gets the address of the object it is initialised
// with.  Temporaries are not supported yet, so it must be an lvalue.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ConstructReference(SCPP65Place* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode)
{
	SCPP65Place		sObject;

	if (!pcInitialiser && (papcArguments->NumElements() == 1))
	{
		pcInitialiser = papcArguments->GetPtr(0);
	}
	if (!pcInitialiser)
	{
		return Error(pcNode, "A reference must be initialised with one object");
	}
	if (!Place(pcInitialiser, &sObject))
	{
		return false;
	}
	LoadAddress(&sObject);
	Store(psPlace, "A", "X");
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Calls the constructor of psClass that matches the arguments for the object at a
// place.  A class without constructors takes its fields from {arguments}, in order.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ConstructClass(SCPP65Place* psPlace, SCPP65Class* psClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	SCPP65Function*	psConstructor;
	SCPP65Field*	psField;
	SCPP65Place		sField;
	size			i;
	bool			bResult;

	if (!CompleteClass(psClass, pcNode))
	{
		return false;
	}
	if (psClass->bConstructors)
	{
		psConstructor = FindFunction(psClass, psClass->lliName, CPP65FK_Constructor, papcArguments, false, pcNode);
		if (!psConstructor)
		{
			return Error(pcNode, "No constructor matches the arguments");
		}
		return CallMember(psConstructor, psPlace, papcArguments, false, pcNode);
	}

	if (papcArguments->NumElements() == 0)
	{
		psConstructor = FindFunction(psClass, psClass->lliName, CPP65FK_Constructor, papcArguments, false, pcNode);
		if (psConstructor)
		{
			return CallMember(psConstructor, psPlace, papcArguments, false, pcNode);
		}
		return true;
	}

	//An aggregate: the fields in order, the rest zero.
	if ((psClass->macBases.NumElements() > 0) || IsPolymorphic(psClass) || (papcArguments->NumElements() > psClass->macFields.NumElements()))
	{
		return Error(pcNode, "These initialisers do not match the class");
	}
	bResult = true;
	sField = *psPlace;
	if (psPlace->eKind == CPP65PK_Address)
	{
		Push("A");
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		sField.psType = psField->psType;
		sField.iOffset = psPlace->iOffset + psField->iOffset;
		if (psPlace->eKind == CPP65PK_Address)
		{
			Line("cpy A, [1 + S]")->AppendNewLine();
		}
		bResult &= Construct(&sField, NULL, (i < papcArguments->NumElements()) ? papcArguments->GetPtr(i) : NULL, pcNode);
	}
	if (psPlace->eKind == CPP65PK_Address)
	{
		Line("add S, 4")->AppendNewLine();
		miDepth -= 4;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a member function: the arguments are pushed, X gets the object's address
// (from psObject, or this when it is NULL), moved on to the function's class if it
// is a base, and the function is called, through the vtable if bVirtual and the
// function is virtual.  An object whose address is in A is kept on the stack while
// the arguments are worked out.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CallMember(SCPP65Function* psFunction, SCPP65Place* psObject, CArraySyntax* papcArguments, bool bVirtual, CCPPSyntax* pcNode)
{
	SCPP65Class*	psObjectClass;
	int				iBytes;
	int				iOffset;
	bool			bPushed;

	bPushed = psObject && (psObject->eKind == CPP65PK_Address) && !psFunction->bStatic;
	if (bPushed)
	{
		Push("A");
	}
	iBytes = PushArguments(psFunction, papcArguments, pcNode);
	if (iBytes < 0)
	{
		return false;
	}

	if (!psFunction->bStatic)
	{
		if (bPushed)
		{
			Line("cpy X, [")->Append(1 + iBytes)->Append(" + S]")->AppendNewLine();
			ChangeX();
			if (psObject->iOffset != 0)
			{
				Line("add X, ")->Append(psObject->iOffset)->AppendNewLine();
			}
		}
		else if (psObject)
		{
			LoadAddress(psObject);
			Line("cpy X, A")->AppendNewLine();
			ChangeX();
		}
		else
		{
			if (miThisOffset == -1)
			{
				return Error(pcNode, "A member function needs an object");
			}
			EnsureThis();
		}

		psObjectClass = psObject ? (psObject->psType->IsClass() ? psObject->psType->psClass : NULL) : mpsFunction->psClass;
		if (psObjectClass && IsDerived(psObjectClass, psFunction->psClass, &iOffset) && (iOffset != 0))
		{
			Line("add X, ")->Append(iOffset)->AppendNewLine();
			ChangeX();
		}
	}

	Call(psFunction, bVirtual, iBytes);
	if (bPushed)
	{
		Line("add S, 4")->AppendNewLine();
		miDepth -= 4;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The label of the thunk a pointer to a virtual member function points to (as
// MSVC's vcall thunks): with the object in X it jumps to the function in the
// object's vtable.  Thunks are named by the vtable pointer's offset and the slot,
// __vcall_8_3, so every class and unit shares them (comdat).
//
//		__vcall_8_3:
//				cpy A, [8 + X]
//				jmp [12 + A]
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVirtualCallThunk(CChars* psz, SCPP65Function* psFunction)
{
	CChars	szLabel;

	szLabel.Init("__vcall_");
	szLabel.Append(psFunction->psClass->iVtableOffset);
	szLabel.Append('_');
	szLabel.Append(psFunction->iSlot);
	if (mpsTarget->eTarget == CPPT_W65C816)
	{
		if (Emit(&szLabel))
		{
			AppendSection(&mszThunks, "text", &szLabel, ", comdat");
			AppendGlobal(&mszThunks, &szLabel);
			mszThunks.Append(szLabel);
			mszThunks.Append(':');
			mszThunks.AppendNewLine();
			AppendVirtualCallThunk816(psFunction);
		}
	}
	else if (Emit(&szLabel))
	{
		AppendSection(&mszThunks, "text", &szLabel, ", comdat");
		AppendGlobal(&mszThunks, &szLabel);
		mszThunks.Append(szLabel);
		mszThunks.Append(':');
		mszThunks.AppendNewLine();
		mszThunks.Append("\t\tcpy A, [");
		mszThunks.Append(psFunction->psClass->iVtableOffset);
		mszThunks.Append(" + X]");
		mszThunks.AppendNewLine();
		mszThunks.Append("\t\tjmp [");
		mszThunks.Append(4 * psFunction->iSlot);
		mszThunks.Append(" + A]");
		mszThunks.AppendNewLine();
		mszThunks.AppendNewLine();
	}
	psz->Append(szLabel);
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A W65C816 this adjusting thunk's body before its jml: this is the first
// argument, pushed last, so it is above the return address at S + 4.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendAdjustThunk816(int iAdjust)
{
	mszThunks.Append("\t\t.width (A, X), 16\n");
	mszThunks.Append("\t\tclc\n");
	mszThunks.Append("\t\tlda [S + 4]\n");
	mszThunks.Append("\t\tadc 0x")->Append((uint32)iAdjust & 0xFFFF, 16)->AppendNewLine();
	mszThunks.Append("\t\tsta [S + 4]\n");
	mszThunks.Append("\t\tlda [S + 6]\n");
	mszThunks.Append("\t\tadc 0x")->Append(((uint32)iAdjust >> 16) & 0xFFFF, 16)->AppendNewLine();
	mszThunks.Append("\t\tsta [S + 6]\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A W65C816 virtual call thunk's body.  Four bytes are pushed below D (saved)
// and D pointed at them: D + 1 .. D + 4 is the vtable pointer, D + 5 the
// caller's D, D + 7 the return address and D + 10 this.  The function's
// address - 1 is put where the caller's D was, D restored, the bytes below
// it dropped and rtl jumps to the function with the stack as the caller left
// it.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVirtualCallThunk816(SCPP65Function* psFunction)
{
	mszThunks.Append("\t\t.width (A, X), 16\n");
	mszThunks.Append("\t\tphd\n");
	mszThunks.Append("\t\tpha\n");
	mszThunks.Append("\t\tpha\n");
	mszThunks.Append("\t\ttsc\n");
	mszThunks.Append("\t\ttcd\n");
	mszThunks.Append("\t\tldy ")->Append(psFunction->psClass->iVtableOffset)->AppendNewLine();
	mszThunks.Append("\t\tlda [[D + 10]:24 + Y]\n");
	mszThunks.Append("\t\tsta [D + 1]\n");
	mszThunks.Append("\t\tldy ")->Append(psFunction->psClass->iVtableOffset + 2)->AppendNewLine();
	mszThunks.Append("\t\tlda [[D + 10]:24 + Y]\n");
	mszThunks.Append("\t\tsta [D + 3]\n");
	mszThunks.Append("\t\tldy ")->Append(4 * psFunction->iSlot + 2)->AppendNewLine();
	mszThunks.Append("\t\tlda [[D + 1]:24 + Y]\n");
	mszThunks.Append("\t\ttax\n");
	mszThunks.Append("\t\tldy ")->Append(4 * psFunction->iSlot)->AppendNewLine();
	mszThunks.Append("\t\tlda [[D + 1]:24 + Y]\n");
	mszThunks.Append("\t\tdec A\n");
	mszThunks.Append("\t\tsta [D + 1]\n");
	mszThunks.Append("\t\tlda [D + 5]\n");
	mszThunks.Append("\t\ttay\n");
	mszThunks.Append("\t\tlda [D + 1]\n");
	mszThunks.Append("\t\tsta [D + 4]\n");
	mszThunks.Append("\t\ttxa\n");
	mszThunks.Append("\t\tsep m\n");
	mszThunks.Append("\t\tsta [D + 6]\n");
	mszThunks.Append("\t\tpla\n");
	mszThunks.Append("\t\trep m\n");
	mszThunks.Append("\t\tpla\n");
	mszThunks.Append("\t\ttya\n");
	mszThunks.Append("\t\ttcd\n");
	mszThunks.Append("\t\trtl\n");
	mszThunks.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a function whose arguments are pushed (and whose object is in X), then
// removes the arguments.  A virtual call goes through the object's vtable:
//
//		cpy A, [vtable + X]
//		jsr [4 * slot + A]
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Call(SCPP65Function* psFunction, bool bVirtual, int iBytes)
{
	RequireFunction(psFunction);
	if (bVirtual && psFunction->bVirtual)
	{
		Line("cpy A, [")->Append(psFunction->psClass->iVtableOffset)->Append(" + X]")->AppendNewLine();
		Line("jsr [")->Append(4 * psFunction->iSlot)->Append(" + A]")->AppendNewLine();
	}
	else
	{
		Line("jsr ");
		AppendFunctionLabel(mpszOut, psFunction);
		mpszOut->AppendNewLine();
	}
	ChangeX();
	if (iBytes > 0)
	{
		Line("add S, ")->Append(iBytes)->AppendNewLine();
		miDepth -= iBytes;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// object.f(), pointer->f(), f() and Class::f() in a member function.  Calls through a
// pointer or this are virtual; calls on an object variable and qualified calls are
// not.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::MemberCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee)
{
	CCPPSyntaxMemberExpression*	pcMember;
	CCPPSyntaxIdExpression*		pcId;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSyntax*					pcObject;
	SCPP65Class*				psClass;
	SCPP65Class*				psLookup;
	SCPP65Function*				psFunction;
	SCPP65Template*				psTemplate;
	SCPP65Type*					psType;
	int64						lliName;
	int							iOffset;
	size						uiErrors;
	bool						bVirtual;
	bool						bThis;

	psClass = NULL;
	pcObject = NULL;
	bVirtual = true;
	bThis = false;
	if (pcCallee->GetKind() == CPPS_MemberExpression)
	{
		pcMember = (CCPPSyntaxMemberExpression*)pcCallee;
		psType = TypeOf(pcMember->mpcObject);
		if (!psType)
		{
			Expression(pcMember->mpcObject);
			return NULL;
		}
		if (pcMember->mbArrow)
		{
			psType = ArrowType(pcMember->mpcObject, pcCallee);
			psType = (psType && psType->IsPointer()) ? psType->psBase : NULL;
		}
		if (!psType || !psType->IsClass())
		{
			Error(pcCallee, "Member functions need an object of a class");
			return NULL;
		}
		psClass = psType->psClass;
		if (!CompleteClass(psClass, pcCallee))
		{
			return NULL;
		}
		pcObject = pcMember->mpcObject;
		pcName = pcMember->mpcName;
		if (!pcMember->mbArrow && (SkipParentheses(pcObject)->GetKind() == CPPS_IdExpression))
		{
			bVirtual = false;
		}
	}
	else
	{
		pcId = (CCPPSyntaxIdExpression*)pcCallee;
		pcName = pcId->mpcName;
		if (pcName->macpComponents.NumElements() == 2)
		{
			psClass = GetComponentClass((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0), NULL, pcCallee);
			bVirtual = false;
		}
		else if (mpsFunction)
		{
			psClass = mpsFunction->psClass;
		}
		bThis = true;
	}

	//p->~X(), o.~X(), p->X::~X() (not virtual) or ~X() (this's): nothing if the class has no destructor.
	if (pcName->GetLast()->meKind == CPPNC_Destructor)
	{
		if (!psClass || (pcCall->macpArguments.NumElements() != 0) || !CompleteClass(psClass, pcCallee))
		{
			Error(pcCallee, "This destructor call is not supported by the compiler yet");
			return NULL;
		}
		if (!psClass->psDestructor)
		{
			return &mcTypes.msVoid;
		}
		if (pcName->macpComponents.NumElements() == 2)
		{
			bVirtual = false;
		}
		return CallMemberFunction(pcCall, pcCallee, pcObject, psClass, psClass->psDestructor, bVirtual);
	}

	lliName = GetFunctionName(pcName->GetLast());
	if (!psClass || (lliName == -1))
	{
		Error(pcCallee, "This call is not supported by the compiler yet");
		return NULL;
	}

	//object.Base::f() looks f up in Base (which can be a base template's name without its arguments) and is not
	//virtual.  psClass stays the object's class.
	psLookup = psClass;
	if (pcObject && (pcName->macpComponents.NumElements() == 2))
	{
		psLookup = QualifierClass(psClass, (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0), pcCallee);
		if (!psLookup)
		{
			return NULL;
		}
		bVirtual = false;
	}
	//object.pf() calls through a field that points to a function.
	if (pcObject && !HasMember(psLookup, lliName) && FindField(psLookup, lliName, &iOffset))
	{
		psType = TypeOf(pcCallee);
		if (psType && psType->IsPointer() && psType->psBase->IsFunction())
		{
			return PointerCall(pcCall, psType->psBase);
		}
	}

	//A member template is used when it has template arguments or no other function matches.
	uiErrors = mcErrors.NumErrors();
	pcComponent = pcName->GetLast();
	psTemplate = (CompleteClass(psLookup, pcCall) ? FindMemberTemplate(psLookup, lliName, pcCall->macpArguments.NumElements(), pcObject ? (((CCPPSyntaxMemberExpression*)pcCallee)->mbArrow ? IsConstPointer(pcObject) : IsConstObject(pcObject)) : IsConstThis()) : NULL);
	psFunction = NULL;
	if (!psTemplate || !pcComponent->mbTemplateArguments)
	{
		if (psTemplate)
		{
			BeginQuiet();
		}
		psFunction = FindFunction(psLookup, lliName, CPP65FK_Function, &pcCall->macpArguments, true, pcCall, pcObject ? (((CCPPSyntaxMemberExpression*)pcCallee)->mbArrow ? IsConstPointer(pcObject) : IsConstObject(pcObject)) : IsConstThis());
		if (psTemplate)
		{
			EndQuiet();
		}
	}
	if (!psFunction && psTemplate)
	{
		psFunction = InstantiateFunction(psTemplate, pcComponent->mbTemplateArguments ? &pcComponent->macpTemplateArguments : NULL, &pcCall->macpArguments, pcCall);
	}
	if (!psFunction)
	{
		if (mcErrors.NumErrors() == uiErrors)
		{
			Error(pcCallee, "The class has no member function of that name");
		}
		return NULL;
	}
	if (bThis && !psFunction->bStatic && (!mpsFunction || !mpsFunction->psClass || !IsDerived(mpsFunction->psClass, psFunction->psClass, NULL)))
	{
		Error(pcCallee, "A member function needs an object");
		return NULL;
	}

	return CallMemberFunction(pcCall, pcCallee, pcObject, psClass, psFunction, bVirtual);
}


//////////////////////////////////////////////////////////////////////////
//
// The end of MemberCall: psFunction (found in psClass, the object's class)
// is called on pcObject (this's when NULL) with pcCall's arguments.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::CallMemberFunction(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee, CCPPSyntax* pcObject, SCPP65Class* psClass, SCPP65Function* psFunction, bool bVirtual)
{
	SCPP65Place		sObject;
	int				iBytes;
	int				iOffset;

	//The arguments are pushed before the object's address is worked out, as that is
	//left in A.
	iBytes = PushArguments(psFunction, &pcCall->macpArguments, pcCall);
	if (iBytes < 0)
	{
		return NULL;
	}
	if (!psFunction->bStatic)
	{
		if (pcObject)
		{
			if (((CCPPSyntaxMemberExpression*)pcCallee)->mbArrow)
			{
				if (!ArrowPointer(pcObject, pcCallee))
				{
					return NULL;
				}
			}
			else
			{
				if (!Place(pcObject, &sObject))
				{
					return NULL;
				}
				LoadAddress(&sObject);
			}
			Line("cpy X, A")->AppendNewLine();
			ChangeX();
		}
		else
		{
			EnsureThis();
		}

		//A base's member function gets the base's address.
		if (IsDerived(pcObject ? psClass : mpsFunction->psClass, psFunction->psClass, &iOffset) && (iOffset != 0))
		{
			Line("add X, ")->Append(iOffset)->AppendNewLine();
			ChangeX();
		}
	}

	Call(psFunction, bVirtual, iBytes);
	return psFunction->psReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes a function's arguments.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::PushArguments(SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	return PushParameters(&psFunction->mapsParameters, &psFunction->mapcDefaults, psFunction->bVariadic, papcArguments, pcNode, psFunction->psReturn);
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the arguments last first, each converted to its parameter's type and at
// its size.  Missing arguments are the parameters' defaults.  A reference gets the
// argument's address.  A variadic function's extra arguments are pushed first (they
// are last), promoted: integers narrower than int are int, arrays and functions are
// pointers.  An object argument is copied onto the stack.  A function returning an
// object (psReturn) gets its temporary's address pushed last.  Returns the bytes
// pushed, or -1 after an error.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::PushParameters(CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, CArraySyntax* papcArguments, CCPPSyntax* pcNode, SCPP65Type* psReturn)
{
	size			uiArguments;
	size			uiParameters;
	size			i;
	SCPP65Type*		psType;
	SCPP65Type*		psParameter;
	CCPPSyntax*		pcArgument;
	SCPP65Place		sPlace;
	int				iBytes;

	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	uiParameters = papsParameters->NumElements();
	if ((uiArguments > uiParameters) && !bVariadic)
	{
		Error(pcNode, "The number of arguments does not match the function");
		return -1;
	}

	iBytes = 0;
	for (i = uiArguments; i > uiParameters; i--)
	{
		pcArgument = papcArguments->GetPtr(i - 1);
		psType = Expression(pcArgument);
		if (!psType)
		{
			return -1;
		}
		if (psType->IsReference())
		{
			psType = psType->psBase;
		}
		if (psType->eKind == CPP65TK_Bool)
		{
			psParameter = &mcTypes.msInt;
		}
		else if (psType->IsFloat())
		{
			psParameter = &mcTypes.msDouble;
		}
		else if (psType->IsInteger())
		{
			psParameter = mcTypes.Promote(psType);
		}
		else if (psType->IsArray())
		{
			psParameter = mcTypes.Pointer(psType->psBase);
		}
		else if (psType->IsFunction())
		{
			psParameter = mcTypes.Pointer(psType);
		}
		else if (psType->IsPointer())
		{
			psParameter = psType;
		}
		else if (psType->IsClass())
		{
			Error(pcArgument, "Objects cannot be passed by value by the compiler yet; pass a pointer or a reference");
			return -1;
		}
		else
		{
			Error(pcArgument, "This variable argument is not supported by the compiler yet");
			return -1;
		}
		if (!Convert(psType, psParameter, pcArgument))
		{
			return -1;
		}
		PushValue(psParameter);
		iBytes += psParameter->iBytes;
	}

	for (i = uiParameters; i > 0; i--)
	{
		pcArgument = (i <= uiArguments) ? papcArguments->GetPtr(i - 1) : (papcDefaults ? papcDefaults->GetPtr(i - 1) : NULL);
		if (!pcArgument)
		{
			Error(pcNode, "The number of arguments does not match the function");
			return -1;
		}
		psParameter = papsParameters->GetPtr(i - 1);
		if (psParameter->IsReference())
		{
			if (!Place(pcArgument, &sPlace))
			{
				return -1;
			}
			LoadAddress(&sPlace);
			Push("A");
			iBytes += 4;
			continue;
		}
		if (psParameter->IsClass())
		{
			//Made in its place on the stack.
			if (!CompleteClass(psParameter->psClass, pcArgument))
			{
				return -1;
			}
			Line("sub S, ")->Append(psParameter->iBytes)->AppendNewLine();
			miDepth += psParameter->iBytes;
			Line("cpy A, S")->AppendNewLine();
			Line("add A, 1")->AppendNewLine();
			meNextElide = CPP65E_Stack;
			miNextElideDepth = miDepth;
			sPlace.eKind = CPP65PK_Address;
			sPlace.psType = psParameter;
			sPlace.iOffset = 0;
			sPlace.lliName = -1;
			sPlace.iString = -1;
			sPlace.bInternal = false;
			sPlace.psClass = NULL;
			if (!Construct(&sPlace, NULL, pcArgument, pcArgument))
			{
				return -1;
			}
			iBytes += psParameter->iBytes;
			continue;
		}

		psType = ValueFor(pcArgument, psParameter, false);
		if (psType && psType->IsInteger() && (psParameter->eKind == CPP65TK_Integer) && (psParameter->iBytes < 4) && psType->IsSupported())
		{
			//Pushing a narrow value keeps only its low bytes, which is the conversion.
		}
		else if (!Convert(psType, psParameter, pcArgument))
		{
			return -1;
		}
		PushValue(psParameter);
		iBytes += psParameter->iBytes;
	}

	if (psReturn && psReturn->IsClass())
	{
		if (!CompleteClass(psReturn->psClass, pcNode) || !TemporaryAddress(pcNode, psReturn))
		{
			return -1;
		}
		Push("A");
		iBytes += 4;
	}
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// object op arguments: a call to the object's member operator.  The result is the
// operator's (a reference leaves its address in A).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	SCPP65Class*	psClass;
	SCPP65Function*	psFunction;
	size			uiErrors;

	psType = TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		Error(pcNode, "An operator call needs an object");
		return NULL;
	}
	psClass = psType->psClass;
	uiErrors = mcErrors.NumErrors();
	psFunction = FindFunction(psClass, GetOperatorName(eOperator, false), CPP65FK_Function, papcArguments, true, pcNode, IsConstObject(pcObject));
	if (!psFunction)
	{
		if (mcErrors.NumErrors() == uiErrors)
		{
			Error(pcNode, "The class has no operator for this");
		}
		return NULL;
	}

	return ObjectCall(pcObject, psClass, psFunction, papcArguments, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls psFunction, a member function of psClass or of one of its bases, on the
// object pcObject (of psClass).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ObjectCall(CCPPSyntax* pcObject, SCPP65Class* psClass, SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	SCPP65Place		sObject;
	int				iBytes;
	int				iOffset;

	iBytes = PushArguments(psFunction, papcArguments, pcNode);
	if (iBytes < 0)
	{
		return NULL;
	}
	if (!Place(pcObject, &sObject))
	{
		return NULL;
	}
	LoadAddress(&sObject);
	Line("cpy X, A")->AppendNewLine();
	ChangeX();
	if (IsDerived(psClass, psFunction->psClass, &iOffset) && (iOffset != 0))
	{
		Line("add X, ")->Append(iOffset)->AppendNewLine();
	}
	Call(psFunction, true, iBytes);
	return psFunction->psReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// The conversion function (operator T, its class's own or a base's) that converts
// an object of psClass to psTo best: by its result's conversion to psTo (see
// ConversionRank; a condition is a conversion to bool).  explicit ones only when
// bExplicit (a cast or a condition).  NULL, quietly, if none converts.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindConversion(SCPP65Class* psClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode)
{
	SCPP65Function*	psFunction;
	SCPP65Function*	psBest;
	int				iRank;
	int				iBest;
	size			i;

	if (!CompleteClass(psClass, pcNode))
	{
		return NULL;
	}
	psBest = NULL;
	iBest = CPP65CR_None;
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->lliName != CPP65_CONVERSION_NAME) || psFunction->psTemplate || (psFunction->bExplicit && !bExplicit) || !IsDerived(psClass, psFunction->psClass, NULL))
		{
			continue;
		}
		iRank = ConversionRank(psTo, psFunction->psReturn, NULL, false);
		if (iRank > iBest)
		{
			psBest = psFunction;
			iBest = iRank;
		}
	}
	return psBest;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of an expression that is converted to psTo: an object's conversion
// function's result when psTo is not a class or a reference and the object has one
// (explicit ones when bExplicit), else the expression's own value.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, bool bExplicit)
{
	SCPP65Function*	psFunction;
	SCPP65Class*	psClass;

	if (psTo && !psTo->IsClass() && !psTo->IsReference() && !psTo->IsVoid() && IsObject(pcExpression))
	{
		psClass = ObjectClass(pcExpression);
		psFunction = FindConversion(psClass, psTo, bExplicit, pcExpression);
		if (psFunction)
		{
			return ObjectCall(pcExpression, psClass, psFunction, NULL, pcExpression);
		}
	}
	return Expression(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the pointer on the left of ->: the object's operator->'s result when
// it is an object of a class.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ArrowType(CCPPSyntax* pcObject, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	SCPP65Function*	psFunction;
	CArraySyntax	apcNone;

	psType = TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		return psType;
	}
	if (!CompleteClass(psType->psClass, pcNode))
	{
		return NULL;
	}
	apcNone.Init();
	psFunction = FindFunction(psType->psClass, GetOperatorName(CT_MemberByPointer, false), CPP65FK_Function, &apcNone, true, pcNode, IsConstObject(pcObject));
	apcNone.Kill();
	return psFunction ? psFunction->psReturn : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer on the left of ->, left in A.  An object of a class calls its
// operator->, which must return a pointer.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ArrowPointer(CCPPSyntax* pcObject, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	CArraySyntax	apcNone;

	psType = TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		return Expression(pcObject);
	}
	apcNone.Init();
	psType = OperatorCall(pcObject, CT_MemberByPointer, &apcNone, pcNode);
	apcNone.Kill();
	if (psType && !psType->IsPointer())
	{
		Error(pcNode, "operator-> must return a pointer for the compiler");
		return NULL;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Copies an object (of the destination's class or one derived from it) a word at a
// time through Y.  The destination keeps its own class's vtable.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CopyObject(SCPP65Place* psDestination, CCPPSyntax* pcSource, CCPPSyntax* pcNode)
{
	SCPP65Place		sSource;
	SCPP65Class*	psClass;
	int				iBytes;
	int				iOffset;
	int				iChunk;

	psClass = psDestination->psType->psClass;
	if (psDestination->eKind == CPP65PK_Address)
	{
		Push("A");
	}
	if (!Place(pcSource, &sSource))
	{
		return false;
	}
	if (!sSource.psType->IsClass() || !IsDerived(sSource.psType->psClass, psClass, NULL))
	{
		return Error(pcNode, "The object cannot be copied to that class");
	}
	LoadAddress(&sSource);
	Line("cpy B, A")->AppendNewLine();
	if (psDestination->eKind == CPP65PK_Address)
	{
		Pull("A");
	}
	LoadAddress(psDestination);

	iBytes = psClass->psType->iBytes;
	for (iOffset = 0; iOffset < iBytes; iOffset += iChunk)
	{
		iChunk = (iBytes - iOffset >= 4) ? 4 : ((iBytes - iOffset >= 2) ? 2 : 1);
		if (iChunk < 4)
		{
			Line("wth Y, ")->Append(8 * iChunk)->AppendNewLine();
		}
		Line("cpy Y, [")->Append(iOffset)->Append(" + B]")->AppendNewLine();
		Line("cpy [")->Append(iOffset)->Append(" + A], Y")->AppendNewLine();
		if (iChunk < 4)
		{
			Line("wth Y, 32")->AppendNewLine();
		}
	}
	if (IsPolymorphic(psClass))
	{
		StoreVtables(psClass, "Y", "A");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// object.field and pointer->field.  A static member is a global.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::MemberPlace(CCPPSyntaxMemberExpression* pcMember, SCPP65Place* psPlace)
{
	SCPP65Type*		psType;
	SCPP65Field*	psField;
	SCPP65Variable*	psVariable;
	SCPP65Class*	psClass;
	int64			lliName;
	int				iOffset;

	if (pcMember->mbArrow)
	{
		psType = ArrowPointer(pcMember->mpcObject, pcMember);
		if (!psType)
		{
			return false;
		}
		if (!psType->IsPointer() || !psType->psBase->IsClass())
		{
			return Error(pcMember, "-> needs a pointer to an object");
		}
		psPlace->eKind = CPP65PK_Address;
		psPlace->psType = psType->psBase;
		psPlace->iOffset = 0;
		psPlace->lliName = -1;
		psPlace->iString = -1;
		psPlace->bInternal = false;
		psPlace->psClass = NULL;
	}
	else
	{
		if (!Place(pcMember->mpcObject, psPlace))
		{
			return false;
		}
		if (!psPlace->psType->IsClass())
		{
			return Error(pcMember, ". needs an object");
		}
	}

	psClass = psPlace->psType->psClass;
	if (!CompleteClass(psClass, pcMember))
	{
		return false;
	}
	lliName = GetComponentName(pcMember->mpcName->GetLast());
	psField = FindField(psClass, lliName, &iOffset);
	if (psField)
	{
		psPlace->psType = psField->psType;
		psPlace->iOffset += iOffset;
		if (psField->psType->IsReference())
		{
			DerefPlace(psPlace);
		}
		return true;
	}
	psVariable = FindStatic(psClass, lliName);
	if (psVariable)
	{
		return VariablePlace(psVariable, psPlace);
	}
	return Error(pcMember, "The class has no member of that name");
}


//////////////////////////////////////////////////////////////////////////
//
// Loads this into X from the frame if something else has used X.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::EnsureThis(void)
{
	if (!mbThisInX && (miThisOffset != -1))
	{
		Line("cpy X, [")->Append(1 + miThisOffset)->Append(" + V]")->AppendNewLine();
	}
	mbThisInX = true;
}


//////////////////////////////////////////////////////////////////////////
//
// X no longer holds this.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ChangeX(void)
{
	mbThisInX = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The identifier of a name component, or -1.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::GetComponentName(CCPPSyntaxNameComponent* pcComponent)
{
	CCPPToken*	pcToken;

	if (!pcComponent || (pcComponent->miName < 0))
	{
		return -1;
	}
	pcToken = mpcTokeniser->Get(pcComponent->miName);
	if (!pcToken->IsIdentifier())
	{
		return -1;
	}
	return ((CCTIdentifier*)pcToken)->mlliIdentifier;
}
