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
#include "CPPSymbolKey.h"


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
	psClass->bTypeInfo = false;
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
// A class (not a template instance) by name, for a qualifier the parser
// couldn't resolve: one visible from pcLookup, else any.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::FindClass(int64 lliName, CCPPScope* pcLookup)
{
	SCPP65Class*	psClass;
	SCPP65Class*	psFound;
	size			i;

	psFound = NULL;
	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if ((psClass->lliName == lliName) && !psClass->psTemplate)
		{
			if (psClass->pcSymbol && psClass->pcSymbol->mpcScope->Encloses(pcLookup))
			{
				return psClass;
			}
			psFound = psFound ? psFound : psClass;
		}
	}
	return psFound;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psClass is an instance (of any arguments) of the class template
// pcSymbol names, and not one of the same name in another scope.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsInstanceNamed(SCPP65Class* psClass, CCPPSymbol* pcSymbol)
{
	return !psClass->psTemplate && (psClass->lliName == pcSymbol->mlliName) && (!psClass->pcSymbol || (psClass->pcSymbol->mpcScope == pcSymbol->mpcScope));
}


//////////////////////////////////////////////////////////////////////////
//
// Where a name's component is declared if the parser resolved it, else where
// the name was looked up: what to choose between things of its name with.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPP65Generator::ComponentLookup(CCPPSyntaxNameComponent* pcComponent)
{
	if (pcComponent->mpcSymbol)
	{
		return pcComponent->mpcSymbol->mpcScope;
	}
	return (pcComponent->mpcParent && (pcComponent->mpcParent->GetKind() == CPPS_Name)) ? ((CCPPSyntaxName*)pcComponent->mpcParent)->mpcLookup : NULL;
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
// The strictest alignment the alignas(n) and alignas(type) specifiers in
// papcSpecifiers ask for: 0 for none (alignas(0) asks for nothing), -1 after
// an error.  It is kept whatever #pragma pack says, as MSVC does.  [dcl.align]
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::AlignasAlignment(CArraySyntax* papcSpecifiers)
{
	CCPPSyntax*		pcSpecifier;
	CCPPSyntax*		pcAlignment;
	SCPP65Type*		psType;
	int64			lliAlign;
	int				iAlign;
	size			i;

	iAlign = 0;
	for (i = 0; i < papcSpecifiers->NumElements(); i++)
	{
		pcSpecifier = papcSpecifiers->GetPtr(i);
		if (pcSpecifier->GetKind() != CPPS_AlignasSpecifier)
		{
			continue;
		}
		pcAlignment = ((CCPPSyntaxAlignasSpecifier*)pcSpecifier)->mpcAlignment;
		if (pcAlignment->GetKind() == CPPS_TypeId)
		{
			psType = GetTypeIdType((CCPPSyntaxTypeId*)pcAlignment);
			if (!psType || (psType->IsClass() && !CompleteClass(psType->psClass, pcAlignment)))
			{
				return -1;
			}
			lliAlign = TypeAlignment(psType);
		}
		else if (!Constant(pcAlignment, &lliAlign))
		{
			Error(pcAlignment, "The alignment must be a constant");
			return -1;
		}
		if ((lliAlign < 0) || (lliAlign > 0x10000) || ((lliAlign & (lliAlign - 1)) != 0))
		{
			Error(pcAlignment, "The alignment must be a power of two");
			return -1;
		}
		if (lliAlign > iAlign)
		{
			iAlign = (int)lliAlign;
		}
	}
	return iAlign;
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
		psTemplate = GetComponentTemplate(pcComponent);
		if (psTemplate)
		{
			pcSymbol = psTemplate->pcSymbol;
		}
		else if (lliName != -1)
		{
			return FindClass(lliName, ComponentLookup(pcComponent));
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
	CCPPScope*						pcEnclosing;
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
	int								iClassAlign;
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

	//Its friends are of its namespace, whatever is being generated when it is completed.
	pcEnclosing = mpcDeclaringNamespace;
	if (psClass->pcSymbol)
	{
		mpcDeclaringNamespace = NamespaceOf(psClass->pcSymbol->mpcScope);
	}
	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		bResult &= ClassMember(psClass, pcSpecifier->macpMembers.GetPtr(i));
	}
	mpcDeclaringNamespace = pcEnclosing;

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
		iFieldAlign = (psField->iAlign > iFieldAlign) ? psField->iAlign : iFieldAlign;
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
	iClassAlign = AlignasAlignment(&pcSpecifier->macpAttributes);
	if (iClassAlign < 0)
	{
		bResult = false;
	}
	iAlign = (iClassAlign > iAlign) ? iClassAlign : iAlign;
	iBytes = (iBytes + iAlign - 1) & ~(iAlign - 1);

	psClass->bEmpty = (psClass->macFields.NumElements() == 0) && (psClass->iVtableOffset == -1);
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psClass->bEmpty &= psClass->macBases.Get(i)->psClass->bEmpty;
	}
	psClass->iAlign = iAlign;
	mcTypes.SetClassBytes(psClass->psType, (iBytes > 0) ? iBytes : 1);
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
	if (pcUsing->mbTypename || !pcName || !pcName->IsQualifiedByClass())
	{
		return;
	}
	lliName = GetFunctionName(pcName->GetLast());
	if (lliName == -1)
	{
		return;
	}
	BeginQuiet();
	psBase = GetComponentClass(pcName->GetQualifier(), NULL, pcUsing, psClass);
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
	for (psOwn = FirstFunctionOfName(lliName); psOwn; psOwn = psOwn->psNextOfName)
	{
		if ((psOwn->psClass != psClass) || (psOwn->eKind != psFunction->eKind) || psOwn->psTemplate || (psOwn->mapsParameters.NumElements() != psFunction->mapsParameters.NumElements()))
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
	int								iAlign;

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
				psField->iAlign = 0;
				psField->pcInitialiser = NULL;
				psField->bMutable = false;
				return true;
			}
		}

		bResult = true;
		iAlign = pcSpecifiers ? AlignasAlignment(&pcSpecifiers->macpSpecifiers) : 0;
		if (iAlign < 0)
		{
			return false;
		}
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
				psVariable->pcInClassInitialiser = pcInit->mpcInitializer;
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
			psField->iAlign = iAlign;
			psField->pcInitialiser = pcInit->mpcInitializer;
			psField->bMutable = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_MUTABLE);
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
	CArrayCPP65VtableEntries	asEntries;
	SCPP65VtableEntry*			psEntry;
	size						i;
	CChars						szLabel;

	if (psClass->bVtable)
	{
		return;
	}
	psClass->bVtable = true;

	//Each unit that makes one of a class's objects writes its vtable in a COMDAT section
	//(the linker keeps one); a class without a name has its own.
	szLabel.Init();
	AppendVtableLabel(&szLabel, psClass);
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
	mszData.Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
	mszData.AppendNewLine();

	//The label is after the first part's header.
	asEntries.Init();
	VtableEntries(psClass, &asEntries);
	for (i = 0; i < asEntries.NumElements(); i++)
	{
		psEntry = asEntries.Get(i);
		if (psEntry->eKind == CPP65VE_Header)
		{
			AppendVtableHeader(psClass, psEntry->iVtablePointer);
		}
		else
		{
			AppendVtableEntry(psEntry->psFunction, psEntry->iAdjust);
		}
		if (i == 0)
		{
			mszData.Append(szLabel);
			mszData.Append(':');
			mszData.AppendNewLine();
		}
	}
	asEntries.Kill();
	szLabel.Kill();

	EnsureTypeInfo(psClass);
}


//////////////////////////////////////////////////////////////////////////
//
// The class's vtable, in order (see CPP65_VTABLE_ENTRY_BYTES): its own
// part's header (before the label) and functions, then each secondary
// vtable's header and functions.  The generator writes it (EnsureVtable)
// and the IR front end describes it and stores its parts' addresses from
// the same list.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::VtableEntries(SCPP65Class* psClass, CArrayCPP65VtableEntries* pasEntries)
{
	SCPP65VtableEntry*	psEntry;
	SCPP65Vtable*		psVtable;
	size				i;
	size				j;

	psEntry = pasEntries->Add();
	psEntry->eKind = CPP65VE_Header;
	psEntry->iVtablePointer = (psClass->iVtableOffset == -1) ? 0 : psClass->iVtableOffset;
	psEntry->psFunction = NULL;
	psEntry->iAdjust = 0;
	for (i = 0; i < psClass->mapsVirtuals.NumElements(); i++)
	{
		psEntry = pasEntries->Add();
		psEntry->eKind = CPP65VE_Function;
		psEntry->iVtablePointer = 0;
		psEntry->psFunction = psClass->mapsVirtuals.GetPtr(i);
		psEntry->iAdjust = 0;
	}
	for (i = 0; i < psClass->macVtables.NumElements(); i++)
	{
		psVtable = psClass->macVtables.Get(i);
		psEntry = pasEntries->Add();
		psEntry->eKind = CPP65VE_Header;
		psEntry->iVtablePointer = psVtable->iOffset + psVtable->psOwner->iVtableOffset;
		psEntry->psFunction = NULL;
		psEntry->iAdjust = 0;
		for (j = 0; j < psVtable->mapsFunctions.NumElements(); j++)
		{
			psEntry = pasEntries->Add();
			psEntry->eKind = CPP65VE_Function;
			psEntry->iVtablePointer = 0;
			psEntry->psFunction = psVtable->mapsFunctions.GetPtr(j);
			psEntry->iAdjust = psVtable->maiAdjust.GetValue(j);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The two words before each part of a vtable, for dynamic_cast: how far the
// object's start is from the vtable pointer using that part (iVtablePointer is
// where that pointer is in the object), then the class's type info.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVtableHeader(SCPP65Class* psClass, int iVtablePointer)
{
	mszData.Append("\t\t.long ");
	mszData.Append(-iVtablePointer);
	mszData.AppendNewLine();
	mszData.Append("\t\t.long ");
	AppendTypeInfoLabel(&mszData, psClass);
	mszData.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__3: the class's type info, for dynamic_cast.  Every subobject of an
// object of the class, the object itself first: the subobject class's type info
// and its offset in the object.  A zero ends it.  Each unit that needs it writes
// it in a COMDAT section (the linker keeps one) so its address identifies the
// class.  The type infos of the bases are written too.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::EnsureTypeInfo(SCPP65Class* psClass)
{
	CChars	szLabel;
	size	i;

	if (psClass->bTypeInfo)
	{
		return;
	}
	psClass->bTypeInfo = true;

	szLabel.Init();
	AppendTypeInfoLabel(&szLabel, psClass);
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
	AppendTypeInfoLabel(&mszData, psClass);
	mszData.Append(':');
	mszData.AppendNewLine();
	AppendTypeInfoBases(psClass, 0);
	mszData.Append("\t\t.long 0");
	mszData.AppendNewLine();

	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		EnsureTypeInfo(psClass->macBases.Get(i)->psClass);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type info entries for the class's subobject at iOffset and all its bases'.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendTypeInfoBases(SCPP65Class* psClass, int iOffset)
{
	SCPP65Base*	psBase;
	size		i;

	mszData.Append("\t\t.long ");
	AppendTypeInfoLabel(&mszData, psClass);
	mszData.AppendNewLine();
	mszData.Append("\t\t.long ");
	mszData.Append(iOffset);
	mszData.AppendNewLine();
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		AppendTypeInfoBases(psBase->psClass, iOffset + psBase->iOffset);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where a vtable pointer is in an object of a polymorphic class: its own, else
// the first secondary one's.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::VtablePointerOffset(SCPP65Class* psClass)
{
	SCPP65Vtable*	psVtable;

	if (psClass->iVtableOffset != -1)
	{
		return psClass->iVtableOffset;
	}
	psVtable = psClass->macVtables.Get(0);
	return psVtable->iOffset + psVtable->psOwner->iVtableOffset;
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
		AppendVtableEntryLabel(&mszData, psFunction, iAdjust);
		mbPureVirtual = true;
		mszData.AppendNewLine();
		return;
	}

	RequireFunction(psFunction);
	szLabel.Init();
	AppendVtableEntryLabel(&szLabel, psFunction, iAdjust);
	if (iAdjust != 0)
	{
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
// A vtable entry's label: R_Pure for a pure virtual function, else the
// function's, or its thunk's (__t and the adjustment's negation) when this
// is adjusted by iAdjust first.  AppendVtableEntry writes the thunk.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVtableEntryLabel(CChars* psz, SCPP65Function* psFunction, int iAdjust)
{
	if (psFunction->bPure)
	{
		psz->Append("R_Pure");
		return;
	}
	AppendFunctionLabel(psz, psFunction);
	if (iAdjust != 0)
	{
		psz->Append("__t");
		psz->Append(-iAdjust);
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
	for (psFunction = FirstFunctionOfName(lliName); psFunction; psFunction = psFunction->psNextOfName)
	{
		if ((psFunction->eKind == eKind) && !psFunction->psTemplate && IsOverloadOf(psClass, psFunction, lliName))
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
		if (apsArguments[i])
		{
			apsArguments[i]->Append(&szMessage);
		}
		else
		{
			szMessage.Append("overloaded function");
		}
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
SCPP65Function* CCPP65Generator::FindFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, int iConstObject, CArrayScopes* papcNamespaces)
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
		//An overloaded function's name has no type of its own (NULL): RankArguments matches it to a parameter
		//that points to one of its overloads.
		apsArguments[i] = TypeOf(papcArguments->GetPtr(i));
		if (!apsArguments[i] && !FunctionNameOf(papcArguments->GetPtr(i)))
		{
			//Its errors.
			mcTypeChecker.Value(papcArguments->GetPtr(i));
			return NULL;
		}
	}

	//The best is found by comparing each viable function with the best so far, then
	//checked to be better than every other.
	psBest = NULL;
	bFound = false;
	for (psFunction = FirstFunctionOfName(lliName); psFunction; psFunction = psFunction->psNextOfName)
	{
		if ((psFunction->eKind != eKind) || psFunction->psTemplate || !IsOverloadOf(psClass, psFunction, lliName) || (papcNamespaces && (papcNamespaces->Find(psFunction->pcNamespace) == ARRAY_ELEMENT_NOT_FOUND)))
		{
			continue;
		}
		bFound = true;
		if (!TakesObject(psFunction, iConstObject) || !RankArguments(psFunction, apsArguments, papcArguments, aiRanks))
		{
			continue;
		}
		if (!psBest || (CompareRanks(aiRanks, aiBest, uiArguments) > 0) || ((CompareRanks(aiRanks, aiBest, uiArguments) == 0) && (CompareQualifiers(psFunction, psBest, papcArguments, iConstObject) > 0)))
		{
			psBest = psFunction;
			memcpy(aiBest, aiRanks, sizeof(int) * uiArguments);
		}
	}
	bAmbiguous = false;
	for (psFunction = psBest ? FirstFunctionOfName(lliName) : NULL; psFunction; psFunction = psFunction->psNextOfName)
	{
		if ((psFunction == psBest) || (psFunction->eKind != eKind) || psFunction->psTemplate || !IsOverloadOf(psClass, psFunction, lliName) || (papcNamespaces && (papcNamespaces->Find(psFunction->pcNamespace) == ARRAY_ELEMENT_NOT_FOUND)))
		{
			continue;
		}
		if (TakesObject(psFunction, iConstObject) && RankArguments(psFunction, apsArguments, papcArguments, aiRanks) && (CompareRanks(aiBest, aiRanks, uiArguments) <= 0))
		{
			if ((CompareRanks(aiBest, aiRanks, uiArguments) < 0) || (CompareQualifiers(psBest, psFunction, papcArguments, iConstObject) <= 0))
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
// The namespaces as functions have them: NULL for the global one.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::NormaliseNamespaces(CArrayScopes* papcNamespaces)
{
	size	i;

	for (i = 0; i < papcNamespaces->NumElements(); i++)
	{
		papcNamespaces->SetPtr(i, NamespaceOf(papcNamespaces->GetPtr(i)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The namespaces (as the scopes they are) a name's function can be in
// (CCPPScope::GatherOverloadScopes).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::GetOverloadNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArrayScopes* papcNamespaces)
{
	if (pcName->mpcLookup)
	{
		pcName->mpcLookup->GatherOverloadScopes(lliName, pcName->mbGlobal || (pcName->GetQualifier() != NULL), papcNamespaces);
	}
	if ((papcNamespaces->NumElements() == 0) && pcName->mpcSymbol)
	{
		papcNamespaces->Add(pcName->mpcSymbol->mpcScope->GetEnclosingNamespace());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the namespace pcScope is in, if it isn't there already.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddNamespaceOf(CCPPScope* pcScope, CArrayScopes* papcNamespaces)
{
	CCPPScope*	pcNamespace;

	pcNamespace = pcScope->GetEnclosingNamespace();
	if (papcNamespaces->Find(pcNamespace) == ARRAY_ELEMENT_NOT_FOUND)
	{
		papcNamespaces->Add(pcNamespace);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the namespaces associated with a type: a class's, its bases' and its
// template arguments', an enum's, and those of what a pointer, reference, array
// or function type is made from.  [basic.lookup.argdep] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddAssociatedNamespaces(SCPP65Type* psType, CArrayScopes* papcNamespaces, int iDepth)
{
	SCPP65Class*	psClass;
	SCPP65Binding*	psBinding;
	size			i;
	size			j;

	if (!psType || (iDepth > CPP65_MAX_ASSOCIATED_DEPTH))
	{
		return;
	}
	switch (psType->eKind)
	{
	case CPP65TK_Class:
		psClass = psType->psClass;
		if (psClass->pcSymbol)
		{
			AddNamespaceOf(psClass->pcSymbol->mpcScope, papcNamespaces);
		}
		for (i = 0; i < psClass->macBases.NumElements(); i++)
		{
			AddAssociatedNamespaces(psClass->macBases.Get(i)->psClass->psType, papcNamespaces, iDepth + 1);
		}
		for (i = 0; psClass->psBindings && (i < psClass->psBindings->macBindings.NumElements()); i++)
		{
			psBinding = psClass->psBindings->macBindings.Get(i);
			AddAssociatedNamespaces(psBinding->psType, papcNamespaces, iDepth + 1);
			for (j = 0; psBinding->papsPack && (j < psBinding->papsPack->NumElements()); j++)
			{
				AddAssociatedNamespaces(psBinding->papsPack->GetPtr(j), papcNamespaces, iDepth + 1);
			}
		}
		break;
	case CPP65TK_Function:
		AddAssociatedNamespaces(psType->psBase, papcNamespaces, iDepth + 1);
		for (i = 0; psType->papsParameters && (i < psType->papsParameters->NumElements()); i++)
		{
			AddAssociatedNamespaces(psType->papsParameters->GetPtr(i), papcNamespaces, iDepth + 1);
		}
		break;
	case CPP65TK_MemberPointer:
		AddAssociatedNamespaces(psType->psClass->psType, papcNamespaces, iDepth + 1);
		AddAssociatedNamespaces(psType->psBase, papcNamespaces, iDepth + 1);
		break;
	case CPP65TK_Pointer:
	case CPP65TK_Reference:
	case CPP65TK_Array:
		AddAssociatedNamespaces(psType->psBase, papcNamespaces, iDepth + 1);
		break;
	default:
		if (psType->pcEnum)
		{
			AddNamespaceOf(psType->pcEnum->mpcScope, papcNamespaces);
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the namespaces associated with the types of a call's arguments.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddArgumentNamespaces(CArraySyntax* papcArguments, CArrayScopes* papcNamespaces)
{
	size	i;

	for (i = 0; papcArguments && (i < papcArguments->NumElements()); i++)
	{
		if (!FunctionNameOf(papcArguments->GetPtr(i)))
		{
			AddAssociatedNamespaces(TypeOf(papcArguments->GetPtr(i)), papcNamespaces, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The namespaces a call's function can be in: what the name finds, and for
// a name that isn't qualified the namespaces its arguments are associated with.
// [basic.lookup.argdep] 3
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::GetCallNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CArrayScopes* papcNamespaces)
{
	GetOverloadNamespaces(pcName, lliName, papcNamespaces);
	if (!pcName->mbGlobal && !pcName->GetQualifier())
	{
		AddArgumentNamespaces(papcArguments, papcNamespaces);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function a call of a name that isn't a member's means.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindNamedFunction(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CArrayScopes	apcNamespaces;
	SCPP65Function*	psFunction;

	apcNamespaces.Init();
	GetCallNamespaces(pcName, lliName, papcArguments, &apcNamespaces);
	NormaliseNamespaces(&apcNamespaces);
	psFunction = FindFunction(NULL, lliName, CPP65FK_Function, papcArguments, false, pcNode, -1, &apcNamespaces);
	apcNamespaces.Kill();
	return psFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The function or function template's instance a call of a name nothing
// declares where it is means: one only its arguments' namespaces have.  NULL
// if there is none.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindAssociatedFunction(CCPPSyntaxName* pcName, int64 lliName, CCPPSyntaxCallExpression* pcCall)
{
	SCPP65Function*	psFunction;
	SCPP65Template*	psTemplate;
	size			uiErrors;

	if ((lliName == -1) || pcName->mbGlobal || pcName->GetQualifier() || (pcCall->macpArguments.NumElements() == 0))
	{
		return NULL;
	}
	uiErrors = mcErrors.NumErrors();
	psFunction = FindNamedFunction(pcName, lliName, &pcCall->macpArguments, pcCall);
	if (psFunction || (mcErrors.NumErrors() != uiErrors))
	{
		return psFunction;
	}
	psTemplate = GetCallTemplate(NULL, lliName, pcCall, NULL);
	return psTemplate ? InstantiateFunction(psTemplate, NULL, &pcCall->macpArguments, pcCall) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The function a name's address is of: a static member of the class that
// qualifies it or of the class of the function being written, or a function
// it can name.  The one of psTarget's type, else the only one; *pbOverloaded is
// set when the name has several and psTarget doesn't pick one.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindAddressedFunction(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPSyntax* pcNode, bool* pbOverloaded)
{
	SCPP65Function*	psFunction;
	SCPP65Function*	psFound;
	SCPP65Class*	psClass;
	CArrayScopes	apcNamespaces;
	bool			bQualified;

	psFound = NULL;
	*pbOverloaded = false;
	psClass = NULL;
	bQualified = pcName && pcName->IsQualifiedByClass();
	if (bQualified)
	{
		BeginQuiet();
		psClass = GetComponentClass(pcName->GetQualifier(), NULL, pcNode);
		EndQuiet();
	}
	else if (mpsFunction && mpsFunction->psClass)
	{
		psClass = mpsFunction->psClass;
	}
	if (psClass && CompleteClass(psClass, pcNode))
	{
		psFound = FindStaticFunction(psClass, lliName, psTarget, pbOverloaded);
	}
	if (psFound || bQualified)
	{
		return psFound;
	}

	apcNamespaces.Init();
	if (pcName)
	{
		GetOverloadNamespaces(pcName, lliName, &apcNamespaces);
		NormaliseNamespaces(&apcNamespaces);
	}
	for (psFunction = FirstFunctionOfName(lliName); psFunction; psFunction = psFunction->psNextOfName)
	{
		if (!psFunction->psClass && !psFunction->psTemplate && (!pcName || (apcNamespaces.Find(psFunction->pcNamespace) != ARRAY_ELEMENT_NOT_FOUND)))
		{
			if (psTarget && psTarget->Equals(mcTypes.Function(psFunction->psReturn, &psFunction->mapsParameters, psFunction->bVariadic)))
			{
				psFound = psFunction;
				*pbOverloaded = false;
				break;
			}
			*pbOverloaded |= (psFound != NULL);
			psFound = psFunction;
		}
	}
	apcNamespaces.Kill();
	return psFound;
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
	size	i;

	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	if ((uiArguments < psFunction->RequiredParameters()) || ((uiArguments > psFunction->mapsParameters.NumElements()) && !psFunction->bVariadic))
	{
		return false;
	}
	for (i = 0; i < uiArguments; i++)
	{
		if (!apsArguments[i])
		{
			//An overloaded function's name (FindFunction) is an exact match for a pointer to one of its overloads.
			aiRanks[i] = ((i < psFunction->mapsParameters.NumElements()) && HasOverloadOfType(FunctionNameOf(papcArguments->GetPtr(i)), psFunction->mapsParameters.GetPtr(i))) ? CPP65CR_Exact : CPP65CR_None;
			if (aiRanks[i] == CPP65CR_None)
			{
				return false;
			}
			continue;
		}
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
	psArgument = psArgument->WithoutReference();
	if (bReference && !psParameter->bConst && !psParameter->IsClass() && pcArgument && !PlaceOf(pcArgument))
	{
		//A value with no address only binds to a const reference.
		return CPP65CR_None;
	}
	if (psParameter->IsPointer() && psArgument->IsPointer() && psParameter->psBase->Equals(psArgument->psBase) && pcArgument && (SkipParentheses(pcArgument)->GetKind() == CPPS_StringExpression))
	{
		//A string literal's characters are const: char* p = "x" is allowed, but const char* is the better match.
		return psParameter->psBase->bConst ? CPP65CR_Exact : CPP65CR_Qualification;
	}
	if (psParameter->Equals(psArgument))
	{
		if (bReference && psArgument->IsClass() && !psArgument->bConst && pcArgument && IsMadeObject(pcArgument, psArgument->psClass))
		{
			//An object made for the call binds best to a const reference (T& takes it too, as MSVC lets it).
			return psParameter->bConst ? CPP65CR_Exact : CPP65CR_Qualification;
		}
		//A copy doesn't care what its source's own const is; a reference does, as does what a pointer points to.
		return QualificationRank(psParameter, psArgument, bReference);
	}
	if (psParameter->IsPointer() && psArgument->IsFunction() && psParameter->psBase->Equals(psArgument))
	{
		return CPP65CR_Exact;
	}
	if (psParameter->IsPointer() && psArgument->IsArray() && psParameter->psBase->Equals(psArgument->psBase))
	{
		return QualificationRank(psParameter->psBase, psArgument->psBase, true);
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
		if (AddsQualifiers(psParameter->psBase, psArgument->psBase) &&
			(psParameter->psBase->IsVoid() || (psParameter->psBase->IsClass() && psArgument->psBase->IsClass() && IsDerived(psArgument->psBase->psClass, psParameter->psBase->psClass, NULL))))
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
		return (!bReference || AddsQualifiers(psParameter, psArgument)) ? CPP65CR_Conversion : CPP65CR_None;
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
// Whether a function can be called on the object (iConstObject 1 if it is
// const, 0 if not, -1 for none): a const object takes only const and static
// member functions.  [over.match.funcs] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::TakesObject(SCPP65Function* psFunction, int iConstObject)
{
	return (iConstObject != 1) || !psFunction->psClass || psFunction->bStatic || psFunction->bConstThis;
}


//////////////////////////////////////////////////////////////////////////
//
// How psFrom converts to psTo, the same type (Equals) but for const and
// volatile: exactly (Exact), adding const or volatile at a level with const
// on every level above it (Qualification), or not at all (None: one is
// taken away, as const T* to T*).  psTo's and psFrom's own qualifiers count
// only with bTopLevel (a reference bound, a pointer's target); a copy is
// made of a value.  [conv.qual]
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::QualificationRank(SCPP65Type* psTo, SCPP65Type* psFrom, bool bTopLevel)
{
	bool	bAdded;
	bool	bAllConst;

	bAdded = false;
	bAllConst = true;
	if (!bTopLevel)
	{
		if (!psTo->IsPointer() || !psFrom->IsPointer())
		{
			return CPP65CR_Exact;
		}
		psTo = psTo->psBase;
		psFrom = psFrom->psBase;
	}
	for (;;)
	{
		if (!AddsQualifiers(psTo, psFrom))
		{
			return CPP65CR_None;
		}
		if ((psTo->bConst != psFrom->bConst) || (psTo->bVolatile != psFrom->bVolatile))
		{
			if (!bAllConst)
			{
				return CPP65CR_None;
			}
			bAdded = true;
		}
		bAllConst &= psTo->bConst;
		if (!(psTo->IsPointer() && psFrom->IsPointer()) && !(psTo->IsArray() && psFrom->IsArray()))
		{
			return bAdded ? CPP65CR_Qualification : CPP65CR_Exact;
		}
		psTo = psTo->psBase;
		psFrom = psFrom->psBase;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psTo has every qualifier psFrom has (const T from T or const T,
// not T from const T).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::AddsQualifiers(SCPP65Type* psTo, SCPP65Type* psFrom)
{
	return (psTo->bConst || !psFrom->bConst) && (psTo->bVolatile || !psFrom->bVolatile);
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
		if (psFunction->RequiredParameters() > 1)
		{
			continue;
		}
		psParameter = psFunction->mapsParameters.GetPtr(0);
		psParameter = psParameter->WithoutReference();
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
// Between two functions whose arguments convert equally well, which qualifies them
// less: of T* and const T*, or T& and const T&, the one without const, but an object
// made by the argument (a temporary) or a const object binds better to a const
// reference.  The object a member function is called on (iConstObject 1 if it is const,
// 0 if not, -1 for none) is bound to this the same way.  1 if the first does, -1 if
// the second does, 0 if neither.  [over.ics.rank] 3.2.6
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::CompareQualifiers(SCPP65Function* psFirst, SCPP65Function* psSecond, CArraySyntax* papcArguments, int iConstObject)
{
	size		i;
	bool		bFirst;
	bool		bSecond;
	bool		bConstBetter;
	SCPP65Type*	psParameter;
	SCPP65Type*	psOther;

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
	for (i = 0; papcArguments && (i < papcArguments->NumElements()) && (i < psFirst->mapsParameters.NumElements()) && (i < psSecond->mapsParameters.NumElements()); i++)
	{
		psParameter = psFirst->mapsParameters.GetPtr(i);
		psOther = psSecond->mapsParameters.GetPtr(i);
		if (!((psParameter->IsPointer() && psOther->IsPointer()) || (psParameter->IsReference() && psOther->IsReference())) || (psParameter->psBase->bConst == psOther->psBase->bConst) || !psParameter->psBase->Equals(psOther->psBase))
		{
			continue;
		}
		bConstBetter = psParameter->IsReference() && ((psParameter->psBase->IsClass() && IsMadeObject(papcArguments->GetPtr(i), psParameter->psBase->psClass)) || IsConstObject(papcArguments->GetPtr(i)));
		if (psParameter->psBase->bConst == bConstBetter)
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
// Whether an expression is an object that is const.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstObject(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType && psType->bConst;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is a pointer to const.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstPointer(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType && psType->IsPointer() && psType->psBase->bConst;
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
// instance's class is Class__T and its arguments, mangled.  An overloaded member
// has its parameters on the end, as a function that is not a member always does,
// so adding an overload never moves another's label.  C++ names cannot contain __
// and cannot start with a digit, so none can clash.
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
	bPlainName = !psFunction->psClass && !psFunction->pcNamespace && (szName.Equals("main") ||
			((psFunction->mapsParameters.NumElements() == 1) && (szName.Equals("opNew") || szName.Equals("opNewArray") || szName.Equals("opDelete") || szName.Equals("opDeleteArray"))));
	if (psFunction->psClass)
	{
		AppendClassLabel(psz, psFunction->psClass, szName.Text());
	}
	else
	{
		psz->Append('_');
		if (!psFunction->bCLinkage)
		{
			AppendScopeNames(psz, psFunction->pcNamespace);
		}
		psz->Append(&szName);
	}
	szName.Kill();

	if (psFunction->mszArguments.Length() > 0)
	{
		psz->Append("__T");
		psz->Append(&psFunction->mszArguments);
	}
	if (((!psFunction->psClass && !psFunction->psTemplate) || (psFunction->mszArguments.Length() > 0) || IsOverloaded(psFunction)) && !psFunction->bCLinkage && !bPlainName)
	{
		//A function that is not a member has its parameters in its label, as units can declare different sets of
		//overloads.  Without parameters (only one such overload can exist) or with C linkage it is just its name.
		//So does an instance of a function template, as Add<M>(void) and Add<M>(size) are both Add__T_..., and an
		//overloaded member (hand-written assembly calls it by a label that doesn't depend on the order of its
		//overloads); a const one ends in K.
		if (psFunction->bConstThis)
		{
			psz->Append("__K");
		}
		if ((psFunction->mapsParameters.NumElements() > 0) || psFunction->bVariadic)
		{
			psz->Append("__F");
			for (i = 0; i < psFunction->mapsParameters.NumElements(); i++)
			{
				AppendMangledType(psz, psFunction->mapsParameters.GetPtr(i));
			}
			if (psFunction->bVariadic)
			{
				//f(int) and f(int, ...) are different overloads.
				psz->Append('z');
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Another function (not a template) of the same class (or none), kind and
// name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsOverloaded(SCPP65Function* psFunction)
{
	SCPP65Function*	psOther;

	for (psOther = FirstFunctionOfName(psFunction->lliName); psOther; psOther = psOther->psNextOfName)
	{
		if ((psOther != psFunction) && (psOther->psClass == psFunction->psClass) && (psOther->eKind == psFunction->eKind) && !psOther->psTemplate)
		{
			return true;
		}
	}
	return false;
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
// _Class__2, the class's vtable (CPP65_VTABLE_LABEL).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVtableLabel(CChars* psz, SCPP65Class* psClass)
{
	AppendClassLabel(psz, psClass, CPP65_VTABLE_LABEL);
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__3, the class's type info (CPP65_TYPE_INFO_LABEL).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendTypeInfoLabel(CChars* psz, SCPP65Class* psClass)
{
	AppendClassLabel(psz, psClass, CPP65_TYPE_INFO_LABEL);
}


//////////////////////////////////////////////////////////////////////////
//
// Class, or Class__T and the template arguments, after the classes and
// namespaces it is in (Outer__Class).  A class without a name is the unit,
// Unnamed and its number.
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
	if (psClass->pcSymbol)
	{
		AppendScopeNames(psz, psClass->pcSymbol->mpcScope);
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
// The named classes and namespaces pcScope is in, outermost first, each
// followed by __, so N::Outer::Inner's class is N__Outer__Inner and two
// Inners (of different classes) have different labels.  A namespace without
// a name is Unnamed and its number, so what it declares doesn't clash with
// the same name outside it.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendScopeNames(CChars* psz, CCPPScope* pcScope)
{
	char*		szName;
	int			iLength;
	CCPPSymbol*	pcOwner;
	size		uiUnnamed;

	if (!pcScope || ((pcScope->meKind != CPPSC_Class) && (pcScope->meKind != CPPSC_Namespace)))
	{
		return;
	}
	pcOwner = pcScope->mpcOwner;
	AppendScopeNames(psz, pcScope->mpcParent);
	if (pcOwner && (pcOwner->mlliName >= 0))
	{
		szName = mpcTokeniser->GetIdentifier(pcOwner->mlliName, &iLength);
		psz->Append(szName, iLength);
		psz->Append("__");
	}
	else if (pcScope->IsUnnamedNamespace())
	{
		uiUnnamed = mapcUnnamedNamespaces.Find(pcScope);
		if (uiUnnamed == ARRAY_ELEMENT_NOT_FOUND)
		{
			uiUnnamed = mapcUnnamedNamespaces.NumElements();
			mapcUnnamedNamespaces.Add(pcScope);
		}
		psz->Append("Unnamed");
		psz->Append((int)uiUnnamed);
		psz->Append("__");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The namespace pcScope is in (or is), and NULL for the global one.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPP65Generator::NamespaceOf(CCPPScope* pcScope)
{
	CCPPScope*	pcNamespace;

	pcNamespace = pcScope->GetEnclosingNamespace();
	return (pcNamespace && pcNamespace->mpcParent) ? pcNamespace : NULL;
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
// function, and C, the length and the label of a class.  A volatile type starts
// with V and a const one with K (VK both): const char* is PKc, const T& RK.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendMangledType(CChars* psz, SCPP65Type* psType)
{
	CChars	szClass;
	size	i;
	char*	szIdentifier;
	int		iLength;

	if (psType->bVolatile)
	{
		psz->Append('V');
	}
	if (psType->bConst)
	{
		psz->Append('K');
	}
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
			szClass.Init();
			AppendScopeNames(&szClass, psType->pcEnum->mpcScope);
			szIdentifier = mpcTokeniser->GetIdentifier(psType->pcEnum->mlliName, &iLength);
			szClass.Append(szIdentifier, iLength);
			psz->Append('N');
			psz->Append((int)szClass.Length());
			psz->Append(&szClass);
			szClass.Kill();
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
		if (psType->bVariadic)
		{
			psz->Append('z');
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
		mszThunks.Append(CPP65_VTABLE_ENTRY_BYTES * psFunction->iSlot);
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
	mszThunks.Append("\t\tldy ")->Append(CPP65_VTABLE_ENTRY_BYTES * psFunction->iSlot + 2)->AppendNewLine();
	mszThunks.Append("\t\tlda [[D + 1]:24 + Y]\n");
	mszThunks.Append("\t\ttax\n");
	mszThunks.Append("\t\tldy ")->Append(CPP65_VTABLE_ENTRY_BYTES * psFunction->iSlot)->AppendNewLine();
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

	if (!CompleteClass(psClass, pcNode))
	{
		return NULL;
	}
	psBest = NULL;
	iBest = CPP65CR_None;
	for (psFunction = FirstFunctionOfName(CPP65_CONVERSION_NAME); psFunction; psFunction = psFunction->psNextOfName)
	{
		if (psFunction->psTemplate || (psFunction->bExplicit && !bExplicit) || !IsDerived(psClass, psFunction->psClass, NULL))
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
	return ((CCTIdentifier*)pcToken)->miIdentifier;
}
