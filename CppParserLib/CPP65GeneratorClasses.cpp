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

	psClass = (SCPP65Class*)malloc(sizeof(SCPP65Class));
	psClass->pcSymbol = pcSymbol;
	psClass->lliName = lliName;
	psClass->psType = mcTypes.Class(psClass);
	psClass->psBase = NULL;
	psClass->macBases.Init();
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
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	SCPP65Class*	psClass;
	size			i;

	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if (!psClass->pcSymbol && (psClass->pcSpecifier == pcSpecifier) && !psClass->psTemplate)
		{
			return psClass;
		}
	}
	psClass = NewClass(NULL, -1);
	psClass->pcSpecifier = pcSpecifier;
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
// The class a name component names: a class, a typedef of one, an instance of a
// class template (Name<Arguments>), or a class template's own name inside it.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	SCPP65Template*	psTemplate;
	SCPP65Bindings*	psBindings;
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
	if (pcSymbol->meKind == CPPSK_ClassTemplate)
	{
		psTemplate = GetTemplate(pcSymbol);
		if (!psTemplate)
		{
			Error(pcNode, "The class template has not been defined");
			return NULL;
		}
		if (pcComponent && pcComponent->mbTemplateArguments)
		{
			return InstantiateClass(psTemplate, &pcComponent->macpTemplateArguments, pcNode);
		}

		//The template's own name inside it is the instance being made.
		for (psBindings = mpsBindings; psBindings; psBindings = psBindings->psOuter)
		{
			if (psBindings->psClass && (psBindings->psClass->psTemplate == psTemplate))
			{
				return psBindings->psClass;
			}
		}
		Error(pcNode, "A class template needs template arguments");
		return NULL;
	}
	return NULL;
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
// Records a template declaration: a class template, a function template, or a
// member function of a class template defined outside it.  Explicit specialisations
// and instantiations are not supported yet and are ignored until used.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::TemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplate)
{
	SCPP65Template*					psTemplate;
	CCPPSyntax*						pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPSyntaxName*					pcName;
	CCPPSyntaxNameComponent*		pcClass;
	SCPP65Template*					psClass;

	pcDeclaration = pcTemplate->mpcDeclaration;
	if (pcTemplate->mbInstantiation && !pcTemplate->mbExtern && pcDeclaration)
	{
		return ExplicitInstantiation(pcDeclaration);
	}
	if (pcTemplate->mbInstantiation || (pcTemplate->macpParameters.NumElements() == 0) || !pcDeclaration)
	{
		return true;
	}

	psTemplate = NULL;
	if (pcDeclaration->GetKind() == CPPS_SimpleDeclaration)
	{
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
		if (pcSimple->mpcSpecifiers && pcSimple->mpcSpecifiers->mpcType && (pcSimple->mpcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
		{
			psTemplate = (SCPP65Template*)malloc(sizeof(SCPP65Template));
			psTemplate->pcSymbol = ((CCPPSyntaxClassSpecifier*)pcSimple->mpcSpecifiers->mpcType)->mpcSymbol;
			psTemplate->pcDefinition = pcSimple->mpcSpecifiers->mpcType;
			psTemplate->bMember = false;
		}
	}
	else if (pcDeclaration->GetKind() == CPPS_FunctionDefinition)
	{
		pcFunction = (CCPPSyntaxFunctionDefinition*)pcDeclaration;
		pcName = pcFunction->mpcDeclarator->GetName();
		if (pcName && (pcName->macpComponents.NumElements() == 2))
		{
			//template<class M> M* CArrayTemplate<M>::Get(size iIndex) { ... }
			pcClass = (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0);
			psClass = GetTemplateByName(GetComponentName(pcClass));
			if (psClass)
			{
				psTemplate = (SCPP65Template*)malloc(sizeof(SCPP65Template));
				psTemplate->pcSymbol = psClass->pcSymbol;
				psTemplate->pcDefinition = pcFunction;
				psTemplate->bMember = true;
			}
		}
		else if (pcName && (pcName->mpcSymbol || (pcName->GetLast()->meKind == CPPNC_Operator)))
		{
			//An operator template has no symbol; it is found by its operator (FindFreeOperator).
			psTemplate = (SCPP65Template*)malloc(sizeof(SCPP65Template));
			psTemplate->pcSymbol = pcName->mpcSymbol;
			psTemplate->pcDefinition = pcFunction;
			psTemplate->bMember = false;
		}
	}

	if (psTemplate)
	{
		psTemplate->pcDeclaration = pcTemplate;
		mapsTemplates.Add(psTemplate);
	}
	return true;
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
		return Error(pcDeclaration, "Only explicit instantiations of class templates are supported by the 65C64 generator yet");
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
//
//////////////////////////////////////////////////////////////////////////
SCPP65Template* CCPP65Generator::GetTemplate(CCPPSymbol* pcSymbol)
{
	SCPP65Template*	psTemplate;
	size			i;

	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if (!psTemplate->bMember && (psTemplate->pcSymbol == pcSymbol))
		{
			return psTemplate;
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
		if (!psTemplate->bMember && psTemplate->pcSymbol && (psTemplate->pcSymbol->mlliName == lliName) && (psTemplate->pcDefinition->GetKind() == CPPS_ClassSpecifier))
		{
			return psTemplate;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Binds a template's parameters to its arguments, worked out with the current
// bindings.  Parameters without an argument take their defaults.  The arguments
// are mangled into pszArguments.  Returns NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Bindings* CCPP65Generator::BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode)
{
	SCPP65Bindings*			psBindings;
	SCPP65Binding*			psBinding;
	CCPPSyntax*				pcParameter;
	CCPPSyntax*				pcArgument;
	CCPPSyntaxTypeParameter*	pcType;
	CCPPSyntaxParameter*	pcValue;
	SCPP65Bindings*			psSaved;
	size					i;
	bool					bResult;

	psBindings = (SCPP65Bindings*)malloc(sizeof(SCPP65Bindings));
	psBindings->macBindings.Init();
	psBindings->psOuter = NULL;
	psBindings->psClass = NULL;
	mapsAllBindings.Add(psBindings);

	bResult = true;
	for (i = 0; i < pcTemplate->macpParameters.NumElements(); i++)
	{
		pcParameter = pcTemplate->macpParameters.GetPtr(i);
		pcArgument = (papcArguments && (i < papcArguments->NumElements())) ? papcArguments->GetPtr(i) : NULL;
		psBinding = psBindings->macBindings.Add();
		psBinding->psType = NULL;
		psBinding->lliValue = 0;
		psBinding->pcParameter = NULL;

		if (pcParameter->GetKind() == CPPS_TypeParameter)
		{
			pcType = (CCPPSyntaxTypeParameter*)pcParameter;
			psBinding->pcParameter = pcType->mpcSymbol;
			if (pcType->mbPack)
			{
				bResult = Error(pcNode, "Variadic templates are not supported by the 65C64 generator yet");
				break;
			}
			if (pcArgument && (pcArgument->GetKind() == CPPS_TypeId))
			{
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
				bResult = Error(pcArgument ? pcArgument : pcNode, "The template argument is not a type the 65C64 generator supports");
				break;
			}
			pszArguments->Append('_');
			AppendMangledType(pszArguments, psBinding->psType);
		}
		else if (pcParameter->GetKind() == CPPS_Parameter)
		{
			pcValue = (CCPPSyntaxParameter*)pcParameter;
			psBinding->pcParameter = pcValue->mpcSymbol;
			pcArgument = pcArgument ? pcArgument : pcValue->mpcDefault;
			if (!pcArgument || !Constant(pcArgument, &psBinding->lliValue))
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
			bResult = Error(pcParameter, "This template parameter is not supported by the 65C64 generator yet");
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
// The instance of a class template for the arguments, made if it is new.  It is
// laid out when it is first needed whole.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::InstantiateClass(SCPP65Template* psTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	SCPP65Bindings*	psBindings;
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
		if ((psClass->psTemplate == psTemplate) && psClass->mszArguments.Equals(&szArguments))
		{
			szArguments.Kill();
			return psClass;
		}
	}

	psClass = NewClass(psTemplate->pcSymbol, psTemplate->pcSymbol->mlliName);
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
// The instance of a function template for explicit arguments and the types of the
// call's arguments.  Template parameters without an explicit argument are deduced
// from parameters declared as T, T*, T** ... or T&.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::InstantiateFunction(SCPP65Template* psTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxFunctionSuffix*		pcSuffix;
	CCPPSyntaxParameter*			pcParameter;
	CCPPSyntaxTypeSpecifier*		pcType;
	CCPPSyntaxName*					pcName;
	SCPP65Bindings*					psBindings;
	SCPP65Bindings*					psSaved;
	SCPP65Function*					psFunction;
	SCPP65Type*						psArgument;
	CArrayCPP65Types				apsDeduced;
	CChars							szArguments;
	size							i;
	size							j;
	size							uiPointers;

	pcDefinition = (CCPPSyntaxFunctionDefinition*)psTemplate->pcDefinition;
	pcSuffix = pcDefinition->mpcDeclarator->GetFunctionSuffix();

	//Deduction: a parameter whose type names a template parameter takes the argument's type.
	apsDeduced.Init();
	for (i = 0; i < psTemplate->pcDeclaration->macpParameters.NumElements(); i++)
	{
		apsDeduced.Add(NULL);
	}
	for (i = 0; pcSuffix && papcArguments && (i < pcSuffix->macpParameters.NumElements()) && (i < papcArguments->NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		pcType = pcParameter->mpcSpecifiers ? pcParameter->mpcSpecifiers->mpcType : NULL;
		if (!pcType || (pcType->GetKind() != CPPS_NamedTypeSpecifier))
		{
			continue;
		}
		pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
		for (j = 0; j < psTemplate->pcDeclaration->macpParameters.NumElements(); j++)
		{
			if ((psTemplate->pcDeclaration->macpParameters.GetPtr(j)->GetKind() == CPPS_TypeParameter) && (((CCPPSyntaxTypeParameter*)psTemplate->pcDeclaration->macpParameters.GetPtr(j))->mpcSymbol == pcName->mpcSymbol))
			{
				psArgument = TypeOf(papcArguments->GetPtr(i));
				uiPointers = pcParameter->mpcDeclarator ? pcParameter->mpcDeclarator->macpPrefix.NumElements() : 0;
				if (uiPointers && pcParameter->mpcDeclarator && (((CCPPSyntaxPointerOperator*)pcParameter->mpcDeclarator->macpPrefix.GetPtr(0))->meKind != CPPP_Pointer))
				{
					uiPointers = 0;
				}
				for (; psArgument && uiPointers > 0; uiPointers--)
				{
					psArgument = psArgument->IsPointer() ? psArgument->psBase : NULL;
				}
				if (psArgument && !apsDeduced.GetPtr(j))
				{
					apsDeduced.SetPtr(j, psArgument);
				}
			}
		}
	}

	szArguments.Init();
	psBindings = BindArguments(psTemplate->pcDeclaration, papcExplicit, &apsDeduced, &szArguments, pcNode);
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

	psSaved = mpsBindings;
	psBindings->psOuter = mpsBindings;
	mpsBindings = psBindings;
	psFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, NULL, false, psTemplate, &szArguments);
	mpsBindings = psSaved;
	psBindings->psOuter = NULL;
	szArguments.Kill();
	if (psFunction)
	{
		psFunction->pcDefinition = pcDefinition;
		psFunction->psBindings = psBindings;
		psFunction->bInline = true;
	}
	return psFunction;
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
			bResult = Error(pcBaseSpecifier, "Virtual inheritance is not supported by the 65C64 generator yet");
			continue;
		}
		psBaseClass = GetNameClass(pcBaseSpecifier->mpcName, pcBaseSpecifier);
		if (!psBaseClass)
		{
			bResult = Error(pcBaseSpecifier, "This base class is not supported by the 65C64 generator yet");
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
	size							j;
	int								iBytes;
	int								iAlign;
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
			if (!psMember->bMember || (psMember->pcSymbol != psClass->psTemplate->pcSymbol))
			{
				continue;
			}
			psMemberBindings = (SCPP65Bindings*)malloc(sizeof(SCPP65Bindings));
			psMemberBindings->macBindings.Init();
			psMemberBindings->psOuter = psClass->psBindings;
			psMemberBindings->psClass = psClass;
			mapsAllBindings.Add(psMemberBindings);
			for (j = 0; (j < psMember->pcDeclaration->macpParameters.NumElements()) && (j < psClass->psBindings->macBindings.NumElements()); j++)
			{
				*psMemberBindings->macBindings.Add() = *psClass->psBindings->macBindings.Get(j);
				psMemberBindings->macBindings.Tail()->pcParameter = (psMember->pcDeclaration->macpParameters.GetPtr(j)->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)psMember->pcDeclaration->macpParameters.GetPtr(j))->mpcSymbol : ((CCPPSyntaxParameter*)psMember->pcDeclaration->macpParameters.GetPtr(j))->mpcSymbol;
			}

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

	//The bases, the vtable pointer, then the fields.
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
		iBytes = (iBytes + psBase->psClass->iAlign - 1) & ~(psBase->psClass->iAlign - 1);
		psBase->iOffset = iBytes;
		iBytes += psBase->psClass->psType->iBytes;
		iAlign = (psBase->psClass->iAlign > iAlign) ? psBase->psClass->iAlign : iAlign;
	}
	if (psClass->psBase && (psClass->psBase->iVtableOffset != -1) && (psClass->macBases.Get(0)->iOffset == 0))
	{
		psClass->iVtableOffset = psClass->psBase->iVtableOffset;
	}
	else if (psClass->mapsVirtuals.NumElements() > 0)
	{
		iBytes = (iBytes + 3) & ~3;
		psClass->iVtableOffset = iBytes;
		iBytes += 4;
		iAlign = 4;
	}
	LayoutSecondaryVtables(psClass);
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		iFieldAlign = psField->psType->IsClass() ? psField->psType->psClass->iAlign : ((psField->psType->iBytes >= 4) || psField->psType->IsArray()) ? 4 : psField->psType->iBytes;
		iFieldAlign = (iFieldAlign < 1) ? 1 : iFieldAlign;
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
	case CPPS_TemplateDeclaration:
	case CPPS_UsingDeclaration:
	case CPPS_AliasDeclaration:
		//Member templates are not supported yet; they are left out until used.
		return true;

	case CPPS_FunctionDefinition:
		pcFunction = (CCPPSyntaxFunctionDefinition*)pcMember;
		BeginQuiet();
		bFriend = pcFunction->mpcSpecifiers && (pcFunction->mpcSpecifiers->muiFlags & CPPDS_FRIEND);
		psFunction = DeclareFunction(pcFunction->mpcSpecifiers, pcFunction->mpcDeclarator, true, bFriend ? NULL : psClass, false);
		EndQuiet();
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
			GetClass(((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType)->mpcSymbol, pcMember)->pcSpecifier = (CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType;
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
				if (!psFunction)
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
		return Error(pcMember, "This member is not supported by the 65C64 generator yet");
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
	bool			bEmit;

	if (psClass->bVtable)
	{
		return;
	}
	psClass->bVtable = true;

	//Each class's vtable is written by the first unit that makes one of its objects.
	szLabel.Init();
	AppendClassLabel(&szLabel, psClass, "2");
	bEmit = Emit(&szLabel);
	szLabel.Kill();
	if (!bEmit)
	{
		return;
	}
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
			mszThunks.Append(szLabel);
			mszThunks.Append(':');
			mszThunks.AppendNewLine();
			mszThunks.Append("\t\tadd X, ");
			mszThunks.Append(iAdjust);
			mszThunks.AppendNewLine();
			mszThunks.Append("\t\tjmp ");
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
// A field of the class or of its bases.  *piOffset is set to its offset in psClass.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Field* CCPP65Generator::FindField(SCPP65Class* psClass, int64 lliName, int* piOffset)
{
	SCPP65Field*	psField;
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
// Overload resolution, simplified: the functions of the name (in the class, then its
// bases if bBases) that take the number of arguments (default arguments make the
// rest), scored by how well each argument's type matches its parameter (the same
// type, then integer to integer and related pointers).  A reference parameter
// matches its referred type.  Returns NULL with an error if more than one is best,
// or none matches functions of the name; NULL without an error if there are none.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psBest;
	SCPP65Type*			apsArguments[32];
	SCPP65Type*			psParameter;
	SCPP65Type*			psArgument;
	size				uiArguments;
	size				uiRequired;
	size				i;
	size				j;
	int					iScore;
	int					iBest;
	int					iMatch;
	bool				bAmbiguous;
	bool				bFound;
	int64				lliValue;

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

	psBest = NULL;
	iBest = -1;
	bAmbiguous = false;
	bFound = false;
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass != psClass) || (psFunction->lliName != lliName) || (psFunction->eKind != eKind) || psFunction->psTemplate)
		{
			continue;
		}
		bFound = true;
		uiRequired = psFunction->mapsParameters.NumElements();
		while ((uiRequired > 0) && psFunction->mapcDefaults.GetPtr(uiRequired - 1))
		{
			uiRequired--;
		}
		if ((uiArguments < uiRequired) || ((uiArguments > psFunction->mapsParameters.NumElements()) && !psFunction->bVariadic))
		{
			continue;
		}

		iScore = 0;
		for (j = 0; (j < uiArguments) && (j < psFunction->mapsParameters.NumElements()); j++)
		{
			psParameter = psFunction->mapsParameters.GetPtr(j);
			psParameter = psParameter->IsReference() ? psParameter->psBase : psParameter;
			psArgument = apsArguments[j];
			if (psParameter->Equals(psArgument))
			{
				iMatch = 3;
			}
			else if (psParameter->IsInteger() && psArgument->IsInteger())
			{
				iMatch = 2;
			}
			else if (psParameter->IsPointer() && psArgument->IsPointer())
			{
				iMatch = (psParameter->psBase->IsVoid() || (psParameter->psBase->IsClass() && psArgument->psBase->IsClass() && IsDerived(psArgument->psBase->psClass, psParameter->psBase->psClass, NULL))) ? 1 : -1;
			}
			else if (psParameter->IsPointer() && psArgument->IsInteger() && Constant(papcArguments->GetPtr(j), &lliValue) && (lliValue == 0))
			{
				iMatch = 1;
			}
			else if (psParameter->IsClass() && psArgument->IsClass() && IsDerived(psArgument->psClass, psParameter->psClass, NULL))
			{
				iMatch = 1;
			}
			else if (psParameter->IsFloat() && (psArgument->IsFloat() || psArgument->IsInteger()))
			{
				iMatch = 1;
			}
			else
			{
				iMatch = -1;
			}
			if (iMatch < 0)
			{
				iScore = -1;
				break;
			}
			iScore += iMatch;
		}

		if (iScore > iBest)
		{
			psBest = psFunction;
			iBest = iScore;
			bAmbiguous = false;
		}
		else if ((iScore == iBest) && (iScore >= 0))
		{
			bAmbiguous = true;
		}
	}

	if (psBest && !bAmbiguous && (iBest >= 0))
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
		Error(pcNode, "No function matches the arguments");
		return NULL;
	}
	if (bBases && psClass)
	{
		for (i = 0; i < psClass->macBases.NumElements(); i++)
		{
			psFunction = FindFunction(psClass->macBases.Get(i)->psClass, lliName, eKind, papcArguments, true, pcNode);
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

	if (psFunction->psClass)
	{
		AppendClassLabel(psz, psFunction->psClass, szName.Text());
	}
	else
	{
		//A static function is the unit's own.
		if (psFunction->bStatic)
		{
			psz->Append(&mszUnit);
		}
		psz->Append('_');
		psz->Append(&szName);
	}
	szName.Kill();

	if (psFunction->mszArguments.Length() > 0)
	{
		psz->Append("__T");
		psz->Append(&psFunction->mszArguments);
	}
	if (psFunction->iOverload > 0)
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
		//A class without a name is the unit's own.
		psz->Append(&mszUnit);
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

	switch (psType->eKind)
	{
	case CPP65TK_Void:
		psz->Append('v');
		break;
	case CPP65TK_Bool:
		psz->Append('b');
		break;
	case CPP65TK_Integer:
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
// The start of a constructor: the bases (from the member initialiser list or their
// default constructors), the vtable pointer, then the fields from the member
// initialisers, the default member initialisers or their own constructors.  The
// body follows.  pcFunction is NULL for a constructor the generator writes.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ConstructorBody(SCPP65Function* psFunction, CCPPSyntaxFunctionDefinition* pcFunction)
{
	SCPP65Class*					psClass;
	CCPPSyntaxMemberInitializer*	pcMember;
	CCPPSyntaxMemberInitializer*	pcFound;
	CArraySyntax*					papcArguments;
	SCPP65Field*					psField;
	SCPP65Base*						psBase;
	SCPP65Place						sPlace;
	CArraySyntax					apcNone;
	CCPPSyntax*						pcInitialiser;
	SCPP65Class*					psNamed;
	size							i;
	size							j;
	bool							bResult;
	int								iOffset;

	psClass = psFunction->psClass;
	bResult = true;
	apcNone.Init();

	sPlace.eKind = CPP65PK_This;
	sPlace.iOffset = 0;
	sPlace.lliName = -1;
	sPlace.iString = -1;
	sPlace.bInternal = false;
	sPlace.psClass = NULL;

	//The bases, in order.
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		pcFound = NULL;
		for (j = 0; pcFunction && (j < pcFunction->macpMemberInitializers.NumElements()); j++)
		{
			pcMember = (CCPPSyntaxMemberInitializer*)pcFunction->macpMemberInitializers.GetPtr(j);
			psNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : GetNameClass(pcMember->mpcName, pcMember);
			if (psNamed == psBase->psClass)
			{
				pcFound = pcMember;
			}
		}
		sPlace.psType = psBase->psClass->psType;
		sPlace.iOffset = psBase->iOffset;
		papcArguments = &apcNone;
		if (pcFound && pcFound->mpcInitializer)
		{
			if (pcFound->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer)
			{
				papcArguments = &((CCPPSyntaxParenthesisedInitializer*)pcFound->mpcInitializer)->macpElements;
			}
			else if (pcFound->mpcInitializer->GetKind() == CPPS_InitializerList)
			{
				papcArguments = &((CCPPSyntaxInitializerList*)pcFound->mpcInitializer)->macpElements;
			}
		}
		bResult &= ConstructClass(&sPlace, psBase->psClass, papcArguments, pcFound ? (CCPPSyntax*)pcFound : (CCPPSyntax*)pcFunction);
	}

	bResult &= SetVtable(NULL, psClass);

	//The fields, in order.
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		pcFound = NULL;
		for (j = 0; pcFunction && (j < pcFunction->macpMemberInitializers.NumElements()); j++)
		{
			pcMember = (CCPPSyntaxMemberInitializer*)pcFunction->macpMemberInitializers.GetPtr(j);
			if ((pcMember->mpcName->mpcSymbol == psField->pcSymbol) || (!pcMember->mpcName->mpcSymbol && (GetComponentName(pcMember->mpcName->GetLast()) == psField->lliName)))
			{
				pcFound = pcMember;
			}
		}

		sPlace.eKind = CPP65PK_This;
		sPlace.psType = psField->psType;
		sPlace.iOffset = psField->iOffset;
		pcInitialiser = pcFound ? pcFound->mpcInitializer : psField->pcInitialiser;
		if (pcInitialiser || psField->psType->IsClass() || psField->psType->IsArray())
		{
			bResult &= Construct(&sPlace, NULL, pcInitialiser, pcFound ? (CCPPSyntax*)pcFound : (CCPPSyntax*)pcFunction);
		}
	}

	//Member initialisers that name nothing.
	for (j = 0; pcFunction && (j < pcFunction->macpMemberInitializers.NumElements()); j++)
	{
		pcMember = (CCPPSyntaxMemberInitializer*)pcFunction->macpMemberInitializers.GetPtr(j);
		psNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : GetNameClass(pcMember->mpcName, pcMember);
		if (!(psNamed && IsDerived(psClass, psNamed, NULL)) && !FindField(psClass, GetComponentName(pcMember->mpcName->GetLast()), &iOffset))
		{
			bResult &= Error(pcMember, "The member initialiser is not a base or a field of the class");
		}
	}

	apcNone.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The end of a destructor, after its body: the fields in reverse, then the bases in
// reverse.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::DestructorBody(SCPP65Function* psFunction)
{
	SCPP65Class*	psClass;
	SCPP65Field*	psField;
	SCPP65Base*		psBase;
	SCPP65Place		sPlace;
	size			i;
	bool			bResult;

	psClass = psFunction->psClass;
	bResult = true;
	sPlace.eKind = CPP65PK_This;
	sPlace.lliName = -1;
	sPlace.iString = -1;
	sPlace.bInternal = false;
	sPlace.psClass = NULL;
	for (i = psClass->macFields.NumElements(); i > 0; i--)
	{
		psField = psClass->macFields.Get(i - 1);
		sPlace.psType = psField->psType;
		sPlace.iOffset = psField->iOffset;
		bResult &= Destroy(&sPlace);
	}
	for (i = psClass->macBases.NumElements(); i > 0; i--)
	{
		psBase = psClass->macBases.Get(i - 1);
		sPlace.psType = psBase->psClass->psType;
		sPlace.iOffset = psBase->iOffset;
		bResult &= Destroy(&sPlace);
	}
	return bResult;
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

	if (pcInitialiser)
	{
		switch (pcInitialiser->GetKind())
		{
		case CPPS_ParenthesisedInitializer:
			papcArguments = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
			pcInitialiser = NULL;
			break;
		case CPPS_InitializerList:
			papcArguments = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
			pcInitialiser = NULL;
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
			return Error(pcNode, "This array initialiser is not supported by the 65C64 generator yet");
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
			bResult = Convert(Expression(pcInitialiser), psType, pcInitialiser);
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

	bResult = ConstructClass(psPlace, psClass, papcArguments, pcNode);
	apcArguments.Kill();
	return bResult;
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
// Calls the destructor of the object (or each object of an array, last first) at a
// place.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Destroy(SCPP65Place* psPlace)
{
	SCPP65Place		sElement;
	SCPP65Type*		psType;
	int				i;
	bool			bResult;

	psType = psPlace->psType;
	if (psType->IsArray())
	{
		if (!psType->psBase->IsClass() || !psType->psBase->psClass->psDestructor)
		{
			return true;
		}
		bResult = true;
		sElement = *psPlace;
		sElement.psType = psType->psBase;
		for (i = psType->iCount; i > 0; i--)
		{
			sElement.iOffset = psPlace->iOffset + (i - 1) * psType->psBase->iBytes;
			bResult &= Destroy(&sElement);
		}
		return bResult;
	}
	if (!psType->IsClass() || !psType->psClass->psDestructor)
	{
		return true;
	}
	return CallMember(psType->psClass->psDestructor, psPlace, NULL, false, NULL);
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
	CCPPSyntax*					pcObject;
	SCPP65Class*				psClass;
	SCPP65Function*				psFunction;
	SCPP65Type*					psType;
	SCPP65Place					sObject;
	int64						lliName;
	int							iBytes;
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
			psType = psType->IsPointer() ? psType->psBase : NULL;
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

	lliName = GetFunctionName(pcName->GetLast());
	if (!psClass || (lliName == -1))
	{
		Error(pcCallee, "This call is not supported by the 65C64 generator yet");
		return NULL;
	}
	uiErrors = mcErrors.NumErrors();
	psFunction = FindFunction(psClass, lliName, CPP65FK_Function, &pcCall->macpArguments, true, pcCall);
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
				if (!Expression(pcObject))
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
			Error(pcArgument, "Objects cannot be passed by value by the 65C64 generator yet; pass a pointer or a reference");
			return -1;
		}
		else
		{
			Error(pcArgument, "This variable argument is not supported by the 65C64 generator yet");
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

		psType = Expression(pcArgument);
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
	SCPP65Place		sObject;
	size			uiErrors;
	int				iBytes;
	int				iOffset;

	psType = TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		Error(pcNode, "An operator call needs an object");
		return NULL;
	}
	psClass = psType->psClass;
	uiErrors = mcErrors.NumErrors();
	psFunction = FindFunction(psClass, GetOperatorName(eOperator, false), CPP65FK_Function, papcArguments, true, pcNode);
	if (!psFunction)
	{
		if (mcErrors.NumErrors() == uiErrors)
		{
			Error(pcNode, "The class has no operator for this");
		}
		return NULL;
	}

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
			Line("wth Y, ")->Append(iChunk)->AppendNewLine();
		}
		Line("cpy Y, [")->Append(iOffset)->Append(" + B]")->AppendNewLine();
		Line("cpy [")->Append(iOffset)->Append(" + A], Y")->AppendNewLine();
		if (iChunk < 4)
		{
			Line("wth Y, 4")->AppendNewLine();
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
		psType = Expression(pcMember->mpcObject);
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
// Points the vtable pointer of this (psPlace is NULL) at psClass's vtable, in its
// constructors and destructor.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::SetVtable(SCPP65Place* psPlace, SCPP65Class* psClass)
{
	if (!IsPolymorphic(psClass))
	{
		return true;
	}
	EnsureThis();
	StoreVtables(psClass, "A", "X");
	return true;
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
