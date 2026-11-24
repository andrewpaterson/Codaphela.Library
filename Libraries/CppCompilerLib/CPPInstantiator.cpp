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
#include <ctype.h>
#include "CPPUnit.h"
#include "CPPSyntaxCopier.h"
#include "CPPSymbolKey.h"
#include "CPPInstantiator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::Kill(void)
{
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the declaration defines a static data member of a class
// template: its declarator's name is qualified by the class template.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::IsStaticMemberDefinition(CCPPSyntaxSimpleDeclaration* pcSimple)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxName*				pcName;

	if ((pcSimple->macpDeclarators.NumElements() != 1) || (pcSimple->macpDeclarators.GetPtr(0)->GetKind() != CPPS_InitDeclarator))
	{
		return false;
	}
	pcInit = (CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0);
	pcName = pcInit->mpcDeclarator ? pcInit->mpcDeclarator->GetName() : NULL;
	return pcName && !pcInit->mpcDeclarator->IsFunction() && pcName->IsQualifiedByClass() && pcName->GetQualifier() && pcName->GetQualifier()->mbTemplateArguments && GetComponentTemplate(pcName->GetQualifier());
}


//////////////////////////////////////////////////////////////////////////
//
// The data of templates' instances, defined when the unit's functions are
// written (as an initialiser that isn't a constant is a start up routine):
// class template instances' static members (DefineInstanceStatics) and
// variable templates' instances (DefineVariableInstances), until none is
// left, as one's may need more.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::DefineInstanceData(void)
{
	bool	bResult;
	bool	bDefined;

	bResult = true;
	do
	{
		bDefined = DefineInstanceStatics(&bResult);
		bDefined |= DefineVariableInstances(&bResult);
		if (bDefined)
		{
			mpcUnit->GenerateQueued();
		}
	} while (bDefined);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Defines each class template instance's static data members that a
// template declaration outside the class defines and that aren't yet, as
// the instance's data (every unit that has the instance has it, and the
// link keeps one), with the template's parameters its arguments: those the
// unit's code uses (their labels asked for).  True if it defined one;
// *pbResult false after an error.  [temp.static], [temp.inst] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::DefineInstanceStatics(bool* pbResult)
{
	CCPPSyntaxTemplateDeclaration*	pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxName*					pcName;
	CCPPTemplate*					pcOwner;
	CCPPClass*						pcClass;
	CCPPVariable*					pcVariable;
	size							i;
	size							j;
	bool							bDefined;

	bDefined = false;
	for (i = 0; i < mpcUnit->masOutsideDefinitions.NumElements(); i++)
	{
		if (mpcUnit->masOutsideDefinitions.Get(i)->eKind != CPPOD_StaticMember)
		{
			continue;
		}
		pcDeclaration = mpcUnit->masOutsideDefinitions.Get(i)->pcDeclaration;
		pcSimple = mpcUnit->masOutsideDefinitions.Get(i)->pcSimple;
		pcName = mpcUnit->masOutsideDefinitions.Get(i)->pcName;
		pcOwner = GetComponentTemplate(pcName->GetQualifier());
		for (j = 0; j < mpcUnit->mapcClasses.NumElements(); j++)
		{
			pcClass = mpcUnit->mapcClasses.GetPtr(j);
			if (!pcClass->mpcTemplate || (pcClass->mpcTemplate != pcOwner) || !pcClass->IsComplete() || pcClass->mbExternInstance || IsInstanceStaticDefined(pcClass, pcName))
			{
				continue;
			}
			pcVariable = OwnStatic(pcClass, pcName->mpcSymbol);
			if (pcVariable && !pcVariable->mbUsed)
			{
				//Only a member that is used is instantiated, its initialiser with it.  [temp.inst] 1, 2
				continue;
			}
			mpcUnit->mcContext.EnterBindings(RenameBindings(pcClass, pcDeclaration));
			*pbResult &= mpcUnit->GlobalDeclaration(pcSimple, pcVariable);
			mpcUnit->mcContext.LeaveBindings();
			bDefined = true;
		}
	}
	return bDefined;
}


//////////////////////////////////////////////////////////////////////////
//
// The initialiser a template declaration outside a class template gives
// the static data member of the symbol (template<class T> const int W<T>::k
// = 3;), for the instance pcClass, with the bindings it is worked out in
// (*ppcBindings: the declaration's parameters as the instance's
// arguments).  NULL if none does.  [temp.static]
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPInstantiator::InstanceStaticInitialiser(CCPPClass* pcClass, CCPPSymbol* pcSymbol, CCPPTemplateArguments** ppcBindings)
{
	SCPPOutsideDefinition*	psDefinition;
	size					i;

	for (i = 0; i < mpcUnit->masOutsideDefinitions.NumElements(); i++)
	{
		psDefinition = mpcUnit->masOutsideDefinitions.Get(i);
		if ((psDefinition->eKind == CPPOD_StaticMember) && (psDefinition->pcName->mpcSymbol == pcSymbol) && psDefinition->pcInit->mpcInitializer && (GetComponentTemplate(psDefinition->pcName->GetQualifier()) == pcClass->mpcTemplate))
		{
			*ppcBindings = RenameBindings(pcClass, psDefinition->pcDeclaration);
			return psDefinition->pcInit->mpcInitializer;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Defines each variable template instance that isn't yet, as its own data
// (every unit that has the instance has it, and the link keeps one), with
// its template's parameters its arguments, or as its explicit
// specialisation.  True if it defined one; *pbResult false after an error.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::DefineVariableInstances(bool* pbResult)
{
	CCPPVariable*	pcVariable;
	size			i;
	bool			bDefined;

	bDefined = false;
	for (i = 0; i < mpcUnit->mapcVariableInstances.NumElements(); i++)
	{
		pcVariable = mpcUnit->mapcVariableInstances.GetPtr(i);
		if (pcVariable->mbDefined)
		{
			continue;
		}
		mpcUnit->mcContext.EnterBindings(pcVariable->mpcBindings);
		*pbResult &= mpcUnit->GlobalDeclaration((CCPPSyntaxSimpleDeclaration*)pcVariable->mpcTemplate->mpcDefinition, pcVariable);
		mpcUnit->mcContext.LeaveBindings();
		pcVariable->mbDefined = true;
		bDefined = true;
	}
	return bDefined;
}


//////////////////////////////////////////////////////////////////////////
//
// The name a variable template's declaration declares (pi in
// template<class T> T pi = T(3);, size<char> in an explicit
// specialisation), or NULL if the declaration isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxName* CCPPInstantiator::VariableTemplateName(CCPPSyntaxSimpleDeclaration* pcSimple)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxName*				pcName;

	if ((pcSimple->macpDeclarators.NumElements() != 1) || (pcSimple->macpDeclarators.GetPtr(0)->GetKind() != CPPS_InitDeclarator))
	{
		return NULL;
	}
	pcInit = (CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0);
	pcName = pcInit->mpcDeclarator ? pcInit->mpcDeclarator->GetName() : NULL;
	if (!pcName || pcInit->mpcDeclarator->IsFunction() || !pcName->mpcSymbol || (pcName->mpcSymbol->meKind != CPPSK_VariableTemplate) || (pcName->macpComponents.NumElements() != 1))
	{
		return NULL;
	}
	return pcName;
}


//////////////////////////////////////////////////////////////////////////
//
// The name a declaration outside a class gives a variable template that is
// the class's static member, where it defines or specialises it with an
// initialiser (template<class T> const int S::size = sizeof(T);), or NULL if
// it doesn't.  [temp.static] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxName* CCPPInstantiator::MemberVariableTemplateName(CCPPSyntaxSimpleDeclaration* pcSimple)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxName*				pcName;

	if ((pcSimple->macpDeclarators.NumElements() != 1) || (pcSimple->macpDeclarators.GetPtr(0)->GetKind() != CPPS_InitDeclarator))
	{
		return NULL;
	}
	pcInit = (CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0);
	pcName = pcInit->mpcDeclarator ? pcInit->mpcDeclarator->GetName() : NULL;
	if (!pcName || !pcInit->mpcInitializer || !pcName->IsQualifiedByClass() || !pcName->mpcSymbol || (pcName->mpcSymbol->meKind != CPPSK_VariableTemplate))
	{
		return NULL;
	}
	return pcName;
}


//////////////////////////////////////////////////////////////////////////
//
// The variable template of the symbol, or NULL.  One that is a class's
// static member is known once its class is laid out (MemberTemplate), and
// is the class's own, of the one symbol, where the class is a class
// template's instance: pcClass's (the class its name is qualified by), or
// the class's the bindings in use are of, or a base's of that class
// (ClassVariableTemplate).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPInstantiator::VariableTemplate(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcClass)
{
	CCPPTemplate*	pcTemplate;
	CCPPClass*		pcDeclaring;

	pcClass = pcClass ? pcClass : mpcUnit->MemberScopeClass(pcSymbol);
	if (!pcClass)
	{
		return GetTemplate(pcSymbol);
	}
	if (!mpcUnit->CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	pcTemplate = ClassVariableTemplate(pcSymbol, pcClass);
	if (!pcTemplate && pcClass->FindMemberSymbol(pcSymbol->mlliName, CPPSK_VariableTemplate, &pcDeclaring))
	{
		pcTemplate = ClassVariableTemplate(pcSymbol, pcDeclaring);
	}
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// The variable template of the symbol that pcClass declares, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPInstantiator::ClassVariableTemplate(CCPPSymbol* pcSymbol, CCPPClass* pcClass)
{
	CCPPTemplate*	pcTemplate;
	size			i;

	for (i = 0; i < mpcUnit->mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
		if (!pcTemplate->mbMember && (pcTemplate->mpcPrimary == pcTemplate) && (pcTemplate->mpcSymbol == pcSymbol) && (pcTemplate->mpcClass == pcClass))
		{
			return pcTemplate;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A class template's instance's member variable template (pcTemplate, made
// from its declaration in the class) takes its definition outside the
// class, if one has been seen (template<class C> template<class T> T
// W<C>::value = T(2);), which names the class's template parameters its
// own way (mpcClassDeclaration).  [temp.static] 1
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::DefineMemberVariableTemplate(CCPPTemplate* pcTemplate)
{
	SCPPOutsideDefinition*	psDefinition;
	size					i;

	for (i = 0; i < mpcUnit->masOutsideDefinitions.NumElements(); i++)
	{
		psDefinition = mpcUnit->masOutsideDefinitions.Get(i);
		if ((psDefinition->eKind == CPPOD_MemberVariableTemplate) && (psDefinition->pcName->mpcSymbol == pcTemplate->mpcSymbol))
		{
			pcTemplate->mpcDeclaration = psDefinition->pcMember;
			pcTemplate->mpcDefinition = psDefinition->pcSimple;
			pcTemplate->mpcClassDeclaration = psDefinition->pcDeclaration;
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A class template's instance's member variable template (pcPrimary) takes
// a partial specialisation written outside the class (pcOutside: template
// <class C> template<class T> constexpr bool W<C>::same<T*> = true;) if it
// is of its symbol, naming the class's parameters its own way as the
// definition outside does (mpcClassDeclaration).  [temp.class.spec] 5
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::SpecialiseMemberVariableTemplate(CCPPTemplate* pcPrimary, CCPPSyntaxTemplateDeclaration* pcOutside)
{
	CCPPSyntaxTemplateDeclaration*	pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxName*					pcName;
	CCPPTemplate*					pcTemplate;

	pcDeclaration = (CCPPSyntaxTemplateDeclaration*)pcOutside->mpcDeclaration;
	pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcDeclaration;
	pcName = MemberVariableTemplateName(pcSimple);
	if (!pcName || (pcName->mpcSymbol != pcPrimary->mpcSymbol))
	{
		return;
	}
	pcTemplate = NewTemplate(pcPrimary->mpcSymbol, pcDeclaration, pcSimple, false);
	pcTemplate->mpcPrimary = pcPrimary;
	pcTemplate->mpcPattern = pcName->GetLast();
	pcTemplate->mpcClass = pcPrimary->mpcClass;
	pcTemplate->mpcClassDeclaration = pcOutside;
}


//////////////////////////////////////////////////////////////////////////
//
// The init-declarator a variable is declared by: its symbol's, or for a
// variable template's instance its template's (or explicit
// specialisation's), whose symbol all its instances share.  NULL if it
// has none.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxInitDeclarator* CCPPInstantiator::VariableDeclarator(CCPPVariable* pcVariable)
{
	CCPPSyntax*	pcDeclaration;

	if (pcVariable->mpcTemplate)
	{
		return (CCPPSyntaxInitDeclarator*)((CCPPSyntaxSimpleDeclaration*)pcVariable->mpcTemplate->mpcDefinition)->macpDeclarators.GetPtr(0);
	}
	pcDeclaration = pcVariable->mpcSymbol ? pcVariable->mpcSymbol->mpcDeclaration : NULL;
	return (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator)) ? (CCPPSyntaxInitDeclarator*)pcDeclaration : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The variable a variable template's name with its arguments (pi<int>)
// names: its instance for the arguments, made the first time it is named,
// of the specialisation that matches them best if one does (an explicit
// one, or a partial one with its own parameters deduced: FindSpecialisation).
// Defined with the unit's data (DefineVariableInstances).  NULL after an
// error.  A class's static member's is the class's the name is qualified
// by, and sees that class's template arguments (W<char>::size<long>).
// [temp.variable], [temp.class.spec]
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPInstantiator::VariableInstance(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	return ClassVariableInstance(pcName->mpcSymbol, pcName->GetLast(), pcName->IsQualifiedByClass() ? mpcUnit->NameQualifierClass(pcName, pcNode) : NULL, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// VariableInstance for the variable template of the symbol, named by
// pcComponent with its arguments: pcClass's member (W<char>::size<long>,
// object.size<long>), or with no class the one the symbol is where it is
// named (VariableTemplate).
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPInstantiator::ClassVariableInstance(CCPPSymbol* pcSymbol, CCPPSyntaxNameComponent* pcComponent, CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPTemplate*				pcTemplate;
	CCPPTemplate*				pcSource;
	CCPPTemplateArguments*		pcBindings;
	CCPPTemplateArguments*		pcSpecialised;
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPVariable*				pcVariable;
	SCPP65Type*					psType;
	CChars						szArguments;
	size						i;

	pcTemplate = VariableTemplate(pcSymbol, pcNode, pcClass);
	if (!pcTemplate || !pcComponent || !pcComponent->mbTemplateArguments)
	{
		mpcUnit->mcDiagnostics.Error(pcNode, "A variable template is named with its template arguments");
		return NULL;
	}
	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, &pcComponent->macpTemplateArguments, NULL, &szArguments, pcNode);
	pcVariable = NULL;
	for (i = 0; pcBindings && !pcVariable && (i < mpcUnit->mapcVariableInstances.NumElements()); i++)
	{
		if ((mpcUnit->mapcVariableInstances.GetPtr(i)->mpcSymbol == pcSymbol) && (mpcUnit->mapcVariableInstances.GetPtr(i)->mpcClass == pcTemplate->mpcClass) && mpcUnit->mapcVariableInstances.GetPtr(i)->mszArguments.Equals(&szArguments))
		{
			pcVariable = mpcUnit->mapcVariableInstances.GetPtr(i);
		}
	}
	if (!pcBindings || pcVariable)
	{
		szArguments.Kill();
		return pcVariable;
	}

	pcSource = FindSpecialisation(pcTemplate, pcBindings, &pcSpecialised, pcNode);
	pcBindings = pcSource ? pcSpecialised : pcBindings;
	pcSource = pcSource ? pcSource : pcTemplate;
	if (pcTemplate->mpcClass && pcTemplate->mpcClass->mpcBindings && !pcBindings->mpcOuter)
	{
		//(A specialisation outside the class names the class's parameters its own way.)
		pcBindings->mpcOuter = MemberTemplateClassBindings(pcSource->mpcClassDeclaration ? pcSource : pcTemplate);
	}
	pcSimple = (CCPPSyntaxSimpleDeclaration*)pcSource->mpcDefinition;
	pcInit = (CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0);
	mpcUnit->mcContext.EnterBindings(pcBindings);
	psType = mpcUnit->mcDeclarators.GetType(pcSimple->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
	mpcUnit->mcContext.LeaveBindings();
	if (psType)
	{
		pcVariable = mpcUnit->AddVariable(&mpcUnit->mapcGlobals, pcSymbol, psType, CPPVS_Global, 0);
		pcVariable->mpcClass = pcTemplate->mpcClass;
		pcVariable->mpcTemplate = pcSource;
		pcVariable->mpcBindings = pcBindings;
		pcVariable->mszArguments.Append(&szArguments);
		mpcUnit->mapcVariableInstances.Add(pcVariable);
	}
	szArguments.Kill();
	return pcVariable;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the instance's static member the name is of is defined, or it
// has none of the name (it can't be).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::IsInstanceStaticDefined(CCPPClass* pcClass, CCPPSyntaxName* pcName)
{
	CCPPVariable*	pcVariable;

	pcVariable = OwnStatic(pcClass, pcName->mpcSymbol);
	return !pcVariable || pcVariable->mbDefined;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's own static data member of the symbol (each instance of a
// class template has one, all of the symbol), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPInstantiator::OwnStatic(CCPPClass* pcClass, CCPPSymbol* pcSymbol)
{
	CCPPVariable*	pcVariable;
	size			i;

	for (i = 0; i < mpcUnit->mapcGlobals.NumElements(); i++)
	{
		pcVariable = mpcUnit->mapcGlobals.GetPtr(i);
		if ((pcVariable->mpcClass == pcClass) && (pcVariable->mpcSymbol == pcSymbol))
		{
			return pcVariable;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Records a template declaration: a class template, a function template, a member
// function of a class template defined outside it, a member template defined outside
// its class (template<class M> template<class N> ... for a class template), or an
// explicit or partial specialisation of a class or function template, or an
// explicit specialisation of a class template's member function
// (MemberSpecialisation).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::TemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration)
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
	size							i;
	bool							bExplicit;

	pcDeclaration = pcTemplateDeclaration->mpcDeclaration;
	if (!pcDeclaration)
	{
		return true;
	}
	if (pcTemplateDeclaration->mbInstantiation)
	{
		return ExplicitInstantiation(pcDeclaration, pcTemplateDeclaration->mbExtern);
	}
	if (mpcUnit->mcContext.IsCLinkage() && (pcTemplateDeclaration->macpParameters.NumElements() > 0))
	{
		//[temp] 4
		return mpcUnit->mcDiagnostics.Error(pcTemplateDeclaration, "A template has no C linkage: it is declared outside extern \"C\"");
	}
	bExplicit = pcTemplateDeclaration->macpParameters.NumElements() == 0;
	if (bExplicit && (pcDeclaration->GetKind() == CPPS_TemplateDeclaration) && (((CCPPSyntaxTemplateDeclaration*)pcDeclaration)->macpParameters.NumElements() == 0) &&
		((CCPPSyntaxTemplateDeclaration*)pcDeclaration)->mpcDeclaration && (((CCPPSyntaxTemplateDeclaration*)pcDeclaration)->mpcDeclaration->GetKind() == CPPS_FunctionDefinition))
	{
		//template<> template<> int S<int>::f<char>(char) { ... }: a member template's explicit specialisation for one
		//instance of its class template, found when that instance's member template is instantiated.  [temp.expl.spec] 16
		mpcUnit->AddOutsideDefinition(CPPOD_MemberTemplateSpecialisation, (CCPPSyntaxTemplateDeclaration*)pcDeclaration, NULL, ((CCPPSyntaxTemplateDeclaration*)pcDeclaration)->mpcDeclaration);
		return true;
	}

	//template<class M> template<class N> N SBox<M>::Mul(N n) { ... }
	pcClassTemplate = NULL;
	if ((pcDeclaration->GetKind() == CPPS_TemplateDeclaration) && !bExplicit)
	{
		pcClassTemplate = pcTemplateDeclaration;
		pcTemplateDeclaration = (CCPPSyntaxTemplateDeclaration*)pcDeclaration;
		pcDeclaration = pcTemplateDeclaration->mpcDeclaration;
		if (pcTemplateDeclaration->mbInstantiation || (pcTemplateDeclaration->macpParameters.NumElements() == 0) || !pcDeclaration)
		{
			return true;
		}
		if (IsClassTemplateDefinition(pcDeclaration))
		{
			//template<class T> template<class U> struct Outer<T>::Pair { ... }; defines each instance's member class template
			//(MemberTemplate).
			mpcUnit->AddOutsideDefinition(CPPOD_MemberClass, pcClassTemplate, pcTemplateDeclaration, pcDeclaration);
			return true;
		}
		if ((pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && MemberVariableTemplateName((CCPPSyntaxSimpleDeclaration*)pcDeclaration) && MemberVariableTemplateName((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->GetLast()->mbTemplateArguments)
		{
			//template<class C> template<class T> constexpr bool W<C>::same<T*> = true; partially specialises each instance's
			//member variable template: those laid out already, and the others' as they are (MemberTemplate).
			//[temp.class.spec] 5
			mpcUnit->AddOutsideDefinition(CPPOD_MemberVariableSpecialisation, pcClassTemplate, pcTemplateDeclaration, pcDeclaration);
			pcName = MemberVariableTemplateName((CCPPSyntaxSimpleDeclaration*)pcDeclaration);
			for (i = 0; i < mpcUnit->mapcTemplates.NumElements(); i++)
			{
				pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
				if (!pcTemplate->mbMember && pcTemplate->mpcClass && (pcTemplate->mpcSymbol == pcName->mpcSymbol) && (pcTemplate->mpcPrimary == pcTemplate))
				{
					SpecialiseMemberVariableTemplate(pcTemplate, pcClassTemplate);
				}
			}
			return true;
		}
		if ((pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && MemberVariableTemplateName((CCPPSyntaxSimpleDeclaration*)pcDeclaration) && !MemberVariableTemplateName((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->GetLast()->mbTemplateArguments)
		{
			//template<class C> template<class T> T W<C>::value = T(2); defines each instance's member variable template:
			//those of the instances laid out already, and the others' as they are (MemberTemplate).
			mpcUnit->AddOutsideDefinition(CPPOD_MemberVariableTemplate, pcClassTemplate, pcTemplateDeclaration, pcDeclaration);
			pcName = MemberVariableTemplateName((CCPPSyntaxSimpleDeclaration*)pcDeclaration);
			for (i = 0; i < mpcUnit->mapcTemplates.NumElements(); i++)
			{
				pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
				if (!pcTemplate->mbMember && pcTemplate->mpcClass && (pcTemplate->mpcSymbol == pcName->mpcSymbol))
				{
					DefineMemberVariableTemplate(pcTemplate);
				}
			}
			return true;
		}
		if (pcDeclaration->GetKind() != CPPS_FunctionDefinition)
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
			else if (!bExplicit && pcSpecifier->mpcName->IsQualifiedByClass() && pcSpecifier->mpcSymbol && (pcSpecifier->mpcSymbol->meKind == CPPSK_Class))
			{
				//template<class T> struct Outer<T>::Inner { ... }; defines each instance's member class (FindSpecifier).
				mpcUnit->AddOutsideDefinition(CPPOD_MemberClass, pcTemplateDeclaration, NULL, pcSimple);
			}
			else if (!bExplicit)
			{
				DefineClassTemplate(pcSpecifier->mpcSymbol, pcTemplateDeclaration, pcSpecifier);
			}
		}
		else if (!bExplicit && pcSimple->mpcSpecifiers && pcSimple->mpcSpecifiers->mpcType && (pcSimple->mpcSpecifiers->mpcType->GetKind() == CPPS_ElaboratedTypeSpecifier) && (pcSimple->macpDeclarators.NumElements() == 0))
		{
			//template<class... T> struct Tuple; is a template its specialisations can specialise.
			DeclareClassTemplate((CCPPSyntaxElaboratedTypeSpecifier*)pcSimple->mpcSpecifiers->mpcType, pcTemplateDeclaration);
		}
		else if (!bExplicit && IsStaticMemberDefinition(pcSimple))
		{
			//template<class T> int S<T>::count = 3; is each instance's (DefineInstanceStatics).
			mpcUnit->AddOutsideDefinition(CPPOD_StaticMember, pcTemplateDeclaration, NULL, pcSimple);
		}
		else if (bExplicit && IsStaticMemberDefinition(pcSimple))
		{
			//template<> int S<int>::count = 5; defines that instance's, which DefineInstanceStatics then leaves be.  [temp.expl.spec] 13
			return mpcUnit->GlobalDeclaration(pcSimple);
		}
		else if (MemberVariableTemplateName(pcSimple))
		{
			//A class's static member that is a variable template, outside the class: template<class T> int S::kind<T*> = 2;
			//and template<> int S::kind<char> = 3; specialise it, and template<class T> const int S::size = sizeof(T);
			//defines it (its instances are made from this).
			pcName = MemberVariableTemplateName(pcSimple);
			pcTemplate = VariableTemplate(pcName->mpcSymbol, pcSimple, NULL);
			if (pcTemplate && (bExplicit || pcName->GetLast()->mbTemplateArguments))
			{
				Specialisation(pcName->mpcSymbol, pcTemplateDeclaration, pcSimple, pcName->GetLast());
			}
			else if (pcTemplate)
			{
				pcTemplate->mpcDeclaration = pcTemplateDeclaration;
				pcTemplate->mpcDefinition = pcSimple;
			}
			pcTemplate = NULL;
		}
		else if (VariableTemplateName(pcSimple))
		{
			//template<class T> T pi = T(3);, template<> int size<char> = 9; and template<class T> int size<T*> = 2;
			pcName = VariableTemplateName(pcSimple);
			if (bExplicit || pcName->GetLast()->mbTemplateArguments)
			{
				Specialisation(pcName->mpcSymbol, pcTemplateDeclaration, pcSimple, pcName->GetLast());
			}
			else
			{
				NewTemplate(pcName->mpcSymbol, pcTemplateDeclaration, pcSimple, false);
			}
		}
		else if (!bExplicit)
		{
			//template<class T> T Twice(T v);
			DeclareFunctionTemplate(pcTemplateDeclaration, pcSimple);
		}
	}
	else if (pcDeclaration->GetKind() == CPPS_FunctionDefinition)
	{
		pcFunction = (CCPPSyntaxFunctionDefinition*)pcDeclaration;
		pcName = pcFunction->mpcDeclarator->GetName();
		if (pcName && pcName->IsQualifiedByClass() && !bExplicit)
		{
			//template<class M> M* CArrayTemplate<M>::Get(size iIndex) { ... }
			NoDefaultArguments(pcFunction->mpcDeclarator, "A template's member is given its default arguments in its class");
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
				mpcUnit->DefineOutsideMembers(pcTemplate);
			}
			else if (pcClassName->mpcSymbol && (pcClassName->mpcSymbol->meKind == CPPSK_Class) && pcName->IsQualifiedByInstance() && !pcClassTemplate)
			{
				//template<class T> int Outer<T>::Inner::Size() { ... }: of each instance's member class (IsOutsideMemberOf).
				pcTemplate = NewTemplate(pcClassName->mpcSymbol, pcTemplateDeclaration, pcFunction, true);
				mpcUnit->DefineOutsideMembers(pcTemplate);
			}
			else if (pcClassName->mpcSymbol && (pcClassName->mpcSymbol->meKind == CPPSK_Class))
			{
				//template<class T> int CValue::Size(T* p) { ... }
				pcTemplate = NewTemplate(pcClassName->mpcSymbol, pcTemplateDeclaration, pcFunction, true);
				pcTemplate->mbMemberTemplate = true;
			}
			if (pcTemplate && pcTemplate->mbMemberTemplate)
			{
				pcTemplate->mlliName = mpcUnit->GetFunctionName(pcName->GetLast());
			}
		}
		else if (pcName && pcName->IsQualifiedByClass() && bExplicit && !pcClassTemplate)
		{
			//template<> int S<char>::f() { ... }
			NoDefaultArguments(pcFunction->mpcDeclarator, "An explicit specialisation is given no default argument");
			mpcUnit->MemberSpecialisation(pcFunction);
		}
		else if (pcName && (pcName->macpComponents.NumElements() == 1) && bExplicit && !pcClassTemplate)
		{
			//template<> int Size<char>(char c) { ... } and template<> int Size(short s) { ... }
			NoDefaultArguments(pcFunction->mpcDeclarator, "An explicit specialisation is given no default argument");
			pcTemplate = Specialisation(pcName->mpcSymbol, pcTemplateDeclaration, pcFunction, pcName->GetLast());
			for (i = 0; pcTemplate && (i < pcTemplate->mpcPrimary->mapcInstances.NumElements()); i++)
			{
				if (FindFunctionSpecialisation(pcTemplate->mpcPrimary, &pcTemplate->mpcPrimary->mapcInstances.GetPtr(i)->mszArguments, pcFunction) == pcTemplate)
				{
					//The template's own instance for its arguments was made by a use before it.  [temp.expl.spec] 6
					mpcUnit->mcDiagnostics.Error(pcFunction->mpcDeclarator, "The explicit specialisation is declared after a use of it that made the template's instance");
					break;
				}
			}
			pcTemplate = NULL;
		}
		else if (pcName && !pcClassTemplate && !bExplicit && (pcName->mpcSymbol || (pcName->GetLast()->meKind == CPPNC_Operator) || (pcName->GetLast()->meKind == CPPNC_Literal)))
		{
			//An operator template (and a literal operator template) has no symbol; it is found by its operator (FindFreeOperator).
			pcTemplate = FindFunctionTemplate(pcName->mpcSymbol, pcTemplateDeclaration, pcFunction->mpcDeclarator);
			if (pcTemplate && pcTemplate->mpcDefinition && (pcTemplate->mpcDefinition != pcFunction) && (pcTemplate->mpcDefinition->GetKind() == CPPS_FunctionDefinition) &&
				SameReturnWritten(((CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition)->mpcSpecifiers, pcFunction->mpcSpecifiers) && pcTemplate->mpcDeclaration &&
				(pcTemplate->mpcDeclaration->macpParameters.NumElements() == pcTemplateDeclaration->macpParameters.NumElements()) && TemplateParametersMatch(pcTemplate->mpcDeclaration, pcTemplateDeclaration))
			{
				//[temp.over.link] 6, [basic.def.odr] 1
				return mpcUnit->mcDiagnostics.Error(pcFunction->mpcDeclarator, "The function template is defined twice: one with an equivalent signature is defined already");
			}
			if (pcTemplate && !pcTemplate->mpcDefinition)
			{
				NoDefaultArguments(pcFunction->mpcDeclarator, "A function template is given its default arguments where it is first declared");
				DefineFunctionTemplate(pcTemplate, pcTemplateDeclaration, pcFunction);
			}
			else
			{
				NewTemplate(pcName->mpcSymbol, pcTemplateDeclaration, pcFunction, false);
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// template<class T> T Twice(T v); declares a function template: its
// instances are declarations (what sizeof, decltype and noexcept ask of a
// call needs no more) until it is defined.  Declared again, or after its
// definition, it is the same template, and is given no more default
// arguments: only a function that isn't a template is.  [temp.fct],
// [basic.def.odr] 4, [dcl.fct.default] 4
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::DeclareFunctionTemplate(CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxSimpleDeclaration* pcSimple)
{
	CCPPSyntaxDeclarator*	pcDeclarator;
	CCPPSyntaxName*			pcName;
	CCPPTemplate*			pcTemplate;

	pcDeclarator = (pcSimple->macpDeclarators.NumElements() == 1) ? ((CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0))->mpcDeclarator : NULL;
	//(An operator template has no symbol: it is found by its operator, FindFreeOperator.)
	pcName = (pcDeclarator && pcDeclarator->GetFunctionSuffix()) ? pcDeclarator->GetName() : NULL;
	if (!pcName || (pcName->macpComponents.NumElements() != 1) || (pcSimple->mpcSpecifiers && (pcSimple->mpcSpecifiers->muiFlags & (CPPDS_FRIEND | CPPDS_TYPEDEF))))
	{
		return;
	}
	if (pcName->mpcSymbol ? (pcName->mpcSymbol->meKind != CPPSK_FunctionTemplate) : ((pcName->GetLast()->meKind != CPPNC_Operator) && (pcName->GetLast()->meKind != CPPNC_Literal)))
	{
		return;
	}
	if (FindFunctionTemplate(pcName->mpcSymbol, pcDeclaration, pcDeclarator))
	{
		NoDefaultArguments(pcDeclarator, "A function template is given its default arguments where it is first declared");
		return;
	}
	pcTemplate = NewTemplate(pcName->mpcSymbol, pcDeclaration, NULL, false);
	pcTemplate->mpcDeclarator = pcDeclarator;
	pcTemplate->mpcSpecifiers = pcSimple->mpcSpecifiers;
}


//////////////////////////////////////////////////////////////////////////
//
// A function template declared before is defined: the template is the
// definition's from here on, whose parameters' names its body uses.  The
// instances made of its declaration were declarations only, which nothing
// that is evaluated could call: they are left behind, and the next use of
// one makes it from the definition.
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::DefineFunctionTemplate(CCPPTemplate* pcTemplate, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxFunctionDefinition* pcDefinition)
{
	CArrayCPPFunctions	apcDeclared;
	CCPPFunction*		pcDeclared;
	CCPPFunction*		pcFunction;
	size				i;

	DeclaredDefaultArguments(pcTemplate->mpcDeclaration, pcTemplate->mpcDeclarator, pcDeclaration, pcDefinition->mpcDeclarator);
	apcDeclared.Init();
	apcDeclared.Copy(&pcTemplate->mapcInstances);
	pcTemplate->mpcDeclaration = pcDeclaration;
	pcTemplate->mpcDefinition = pcDefinition;
	pcTemplate->mpcDeclarator = NULL;
	pcTemplate->mpcSpecifiers = NULL;
	pcTemplate->mapcInstances.ReInit();

	//An instance that was called while the template was only declared is an instance of its definition: it is
	//written.  [temp.point] 8
	for (i = 0; i < apcDeclared.NumElements(); i++)
	{
		pcDeclared = apcDeclared.GetPtr(i);
		pcFunction = pcDeclared->mbCalled ? InstanceOfFunction(pcTemplate, RenamedBindings(pcDeclared->mpcBindings, pcDeclaration), &pcDeclared->mszArguments, pcDefinition) : NULL;
		if (pcFunction)
		{
			mpcUnit->RequireFunction(pcFunction);
		}
	}
	apcDeclared.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A declaration that may give no default argument (a template's after its
// first, a template's member's outside its class, an explicit
// specialisation's): szError at each it gives.  [dcl.fct.default] 4 and 6,
// [temp.expl.spec] 21
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::NoDefaultArguments(CCPPSyntaxDeclarator* pcDeclarator, const char* szError)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	size						i;

	pcSuffix = pcDeclarator->GetFunctionSuffix();
	for (i = 0; pcSuffix && (i < pcSuffix->macpParameters.NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		if ((pcParameter->GetKind() == CPPS_Parameter) && pcParameter->mpcDefault)
		{
			mpcUnit->mcDiagnostics.Error(pcParameter->mpcDefault, szError);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The default arguments a function template's declaration gave its
// parameters are its definition's too, where that gives none
// (template<class T> T Add(T a, T b = 4); template<class T> T Add(T a, T b)
// { ... }): each is copied to the definition's parameter, naming the
// definition's template parameters.  [dcl.fct.default] 4
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::DeclaredDefaultArguments(CCPPSyntaxTemplateDeclaration* pcDeclared, CCPPSyntaxDeclarator* pcDeclaredDeclarator, CCPPSyntaxTemplateDeclaration* pcDefined, CCPPSyntaxDeclarator* pcDefinedDeclarator)
{
	CCPPSyntaxFunctionSuffix*	pcFrom;
	CCPPSyntaxFunctionSuffix*	pcTo;
	CCPPSyntaxParameter*		pcFromParameter;
	CCPPSyntaxParameter*		pcToParameter;
	CCPPSyntaxCopier			cCopier;
	size						i;
	size						j;

	pcFrom = pcDeclaredDeclarator ? pcDeclaredDeclarator->GetFunctionSuffix() : NULL;
	pcTo = pcDefinedDeclarator->GetFunctionSuffix();
	if (!pcFrom || !pcTo || (pcFrom->macpParameters.NumElements() != pcTo->macpParameters.NumElements()))
	{
		return;
	}
	for (i = 0; i < pcFrom->macpParameters.NumElements(); i++)
	{
		pcFromParameter = (CCPPSyntaxParameter*)pcFrom->macpParameters.GetPtr(i);
		pcToParameter = (CCPPSyntaxParameter*)pcTo->macpParameters.GetPtr(i);
		if ((pcFromParameter->GetKind() != CPPS_Parameter) || (pcToParameter->GetKind() != CPPS_Parameter) || !pcFromParameter->mpcDefault || pcToParameter->mpcDefault)
		{
			continue;
		}
		cCopier.Init(mpcUnit->mpcTree->GetMemory());
		for (j = 0; (j < pcDeclared->macpParameters.NumElements()) && (j < pcDefined->macpParameters.NumElements()); j++)
		{
			cCopier.Substitute(pcDeclared->ParameterSymbol(j), pcDefined->ParameterSymbol(j));
		}
		pcToParameter->mpcDefault = cCopier.Copy(pcFromParameter->mpcDefault, pcToParameter);
		cCopier.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function template of the name pcSymbol is, in its scope, that has
// this signature (FunctionTemplateSignature): the one a declaration
// declares again, or a definition defines.  NULL if there is none, and for
// an operator template, which has no symbol.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPInstantiator::FindFunctionTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPTemplate*	pcTemplate;
	CCPPTemplate*	pcFound;
	CChars			szSignature;
	CChars			szOther;
	int64			lliName;
	size			i;

	lliName = mpcUnit->GetFunctionName(pcDeclarator->GetName()->GetLast());
	szSignature.Init();
	FunctionTemplateSignature(pcDeclaration, pcDeclarator, &szSignature);
	pcFound = NULL;
	for (i = 0; !pcFound && (i < mpcUnit->mapcTemplates.NumElements()); i++)
	{
		pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
		if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->IsFunction() || ((pcTemplate->mpcSymbol != NULL) != (pcSymbol != NULL)))
		{
			continue;
		}
		if (pcSymbol ? ((pcTemplate->mpcSymbol->mlliName != pcSymbol->mlliName) || (pcTemplate->mpcSymbol->mpcScope != pcSymbol->mpcScope)) :
			((pcTemplate->mpcNamespace != mpcUnit->mcContext.GetNamespace()) || (mpcUnit->GetFunctionName(pcTemplate->FunctionDeclarator()->GetName()->GetLast()) != lliName)))
		{
			//An operator template, which has no symbol, is one of its operator in its namespace.
			continue;
		}
		szOther.Init();
		FunctionTemplateSignature(pcTemplate->mpcDeclaration, pcTemplate->FunctionDeclarator(), &szOther);
		pcFound = szOther.Equals(&szSignature) ? pcTemplate : NULL;
		szOther.Kill();
	}
	szSignature.Kill();
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
//
// A function template's parameters as they are written, after the number of
// its template parameters and with each of those named by its place: what
// template<class T> T f(T) and template<class U> U f(U u) { ... } have the
// same, and another overload of f hasn't.
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::FunctionTemplateSignature(CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxDeclarator* pcDeclarator, CChars* pszDest)
{
	CCPPSymbolKey	cKey;
	CChars			szWritten;
	CCPPSyntax*		pcParameter;
	CCPPSymbol*		pcSymbol;
	char*			szText;
	char*			szName;
	size			uiStart;
	size			uiEnd;
	size			i;
	int				iLength;
	bool			bNamed;

	szWritten.Init();
	cKey.Init(mpcUnit->mpcTokeniser, mpcUnit->mpcTree);
	cKey.GetSignature(pcDeclarator->GetFunctionSuffix(), &szWritten);
	cKey.Kill();

	pszDest->Append((int32)pcDeclaration->macpParameters.NumElements());
	szText = szWritten.Text();
	for (uiStart = 0; uiStart < szWritten.Length(); uiStart = uiEnd)
	{
		//A word, or one character that isn't of one.
		for (uiEnd = uiStart; (uiEnd < szWritten.Length()) && (isalnum((unsigned char)szText[uiEnd]) || (szText[uiEnd] == '_')); uiEnd++)
		{
		}
		if (uiEnd == uiStart)
		{
			uiEnd++;
			pszDest->Append(szText[uiStart]);
			continue;
		}
		bNamed = false;
		for (i = 0; !bNamed && (i < pcDeclaration->macpParameters.NumElements()); i++)
		{
			pcParameter = pcDeclaration->macpParameters.GetPtr(i);
			pcSymbol = (pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((pcParameter->GetKind() == CPPS_Parameter) ? ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol : NULL);
			szName = pcSymbol ? mpcUnit->mpcTokeniser->GetIdentifier(pcSymbol->mlliName, &iLength) : NULL;
			if (szName && ((size)iLength == uiEnd - uiStart) && (memcmp(szName, &szText[uiStart], iLength) == 0))
			{
				pszDest->Append('$');
				pszDest->Append((int32)i);
				bNamed = true;
			}
		}
		if (!bNamed)
		{
			pszDest->Append(&szText[uiStart], uiEnd - uiStart);
		}
	}
	szWritten.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// template<class... T> struct Tuple; declares a class template: one its
// specialisations can specialise before it is defined, or if it never is,
// as a variadic template is often written.  Declared again, it is the
// same template.
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::DeclareClassTemplate(CCPPSyntaxElaboratedTypeSpecifier* pcSpecifier, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	CCPPSymbol*	pcSymbol;

	pcSymbol = (pcSpecifier->mpcName && (pcSpecifier->mpcName->macpComponents.NumElements() == 1)) ? pcSpecifier->mpcName->mpcSymbol : NULL;
	if (pcSymbol && !GetTemplate(pcSymbol))
	{
		NewTemplate(pcSymbol, pcDeclaration, pcSpecifier, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A class template's definition: the template it declared before, if it
// was, now with the definition's parameters and body, or else a new one.
// Its instances made before (A<int>* p;) are of the definition, their
// arguments bound to its parameters.
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::DefineClassTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPTemplate*	pcTemplate;
	CCPPClass*		pcClass;
	size			i;

	pcTemplate = pcSymbol ? GetTemplate(pcSymbol) : NULL;
	if (!pcTemplate || !pcTemplate->IsDeclaredClass())
	{
		NewTemplate(pcSymbol, pcDeclaration, pcSpecifier, false);
		return;
	}

	pcTemplate->mpcDeclaration = pcDeclaration;
	pcTemplate->mpcDefinition = pcSpecifier;
	for (i = 0; i < mpcUnit->mapcClasses.NumElements(); i++)
	{
		pcClass = mpcUnit->mapcClasses.GetPtr(i);
		if ((pcClass->mpcTemplate == pcTemplate) && !pcClass->mpcSpecifier)
		{
			pcClass->mpcSpecifier = pcSpecifier;
			pcClass->mpcBindings = RenameBindings(pcClass, pcDeclaration);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Records a specialisation of the template pcSymbol names, which must be declared
// before it.  pcPattern holds its template arguments (a function specialisation's
// may have none: they are deduced from its parameters).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPInstantiator::Specialisation(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, CCPPSyntaxNameComponent* pcPattern)
{
	CCPPTemplate*	pcPrimary;
	CCPPTemplate*	pcTemplate;
	int64			lliName;

	//template<> int Size(short s) names a new function; its template is found by name.
	pcPrimary = pcSymbol ? GetTemplate(pcSymbol) : NULL;
	lliName = pcPattern ? mpcUnit->GetComponentName(pcPattern) : -1;
	if (!pcPrimary && (lliName != -1))
	{
		pcPrimary = GetTemplateByName(lliName, pcDefinition->GetKind(), mpcUnit->ComponentLookup(pcPattern));
	}
	if (!pcPrimary || (pcPrimary->DefinitionKind() != pcDefinition->GetKind()))
	{
		return NULL;
	}
	if (mpcUnit->mcContext.GetNamespace() && !mpcUnit->mcContext.GetNamespace()->Encloses(pcPrimary->mpcNamespace))
	{
		//[temp.expl.spec] 2, [temp.class.spec] 6
		mpcUnit->mcDiagnostics.Error(pcDefinition, "A specialisation is declared in a namespace that encloses its template");
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
CCPPTemplate* CCPPInstantiator::OutsideMemberOwner(CCPPTemplate* pcPrimary, CCPPSyntaxNameComponent* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration)
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
	for (i = 0; i < mpcUnit->mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
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
// A new template, kept until the unit is killed.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPInstantiator::NewTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember)
{
	CCPPTemplate*	pcTemplate;

	pcTemplate = NewMalloc<CCPPTemplate>();
	pcTemplate->Init(pcSymbol, pcDeclaration, pcDefinition, bMember, mpcUnit->mcContext.GetNamespace());
	mpcUnit->mapcTemplates.Add(pcTemplate);
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// template class Name<Arguments>; generates every member function of the instance
// that has a definition, and template int Max<int>(int, int); (or Max(int, int))
// the function template's instance.  extern template ... (bExtern) generates
// neither: the unit uses the instance another unit's explicit instantiation
// generates (IsInstantiatedElsewhere).  An explicit instantiation after an extern
// one in the same unit generates the instance.  [temp.explicit]  The functions
// are generated in the order they are defined, as a class's that isn't a
// template are, also those waiting already: what the inliner makes of them
// depends on it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::ExplicitInstantiation(CCPPSyntax* pcDeclaration, bool bExtern)
{
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxTypeSpecifier*		pcType;
	CCPPSyntaxName*					pcName;
	CCPPClass*					pcClass;
	CCPPFunction*					pcFunction;
	CArrayCPPFunctions				apcDefined;
	size							i;
	size							j;

	pcType = NULL;
	pcSimple = NULL;
	if (pcDeclaration->GetKind() == CPPS_SimpleDeclaration)
	{
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
		pcType = pcSimple->mpcSpecifiers ? pcSimple->mpcSpecifiers->mpcType : NULL;
	}
	if (pcSimple && (pcSimple->macpDeclarators.NumElements() == 1))
	{
		return ExplicitFunctionInstantiation((CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0), bExtern);
	}
	if (!pcType || (pcSimple->macpDeclarators.NumElements() != 0) || ((pcType->GetKind() != CPPS_ElaboratedTypeSpecifier) && (pcType->GetKind() != CPPS_NamedTypeSpecifier)))
	{
		return mpcUnit->mcDiagnostics.Error(pcDeclaration, "Only explicit instantiations of class templates and function templates are supported by the compiler yet");
	}
	pcName = (pcType->GetKind() == CPPS_ElaboratedTypeSpecifier) ? ((CCPPSyntaxElaboratedTypeSpecifier*)pcType)->mpcName : ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
	pcClass = mpcUnit->GetNameClass(pcName, pcDeclaration);
	if (!pcClass)
	{
		return false;
	}
	if (bExtern)
	{
		//Its members are declared when it is completed, wherever that is.
		pcClass->mbExternInstance = true;
		return true;
	}
	pcClass->mbExternInstance = false;
	if (!ExplicitlyInstantiatedOnce(pcClass, pcDeclaration) || !mpcUnit->CompleteClass(pcClass, pcDeclaration))
	{
		return false;
	}
	//The defined functions by where their definitions are (an insertion sort: it keeps the order of equals), then those
	//the compiler writes.
	apcDefined.Init();
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if (pcFunction->mpcDefinition)
		{
			for (j = apcDefined.NumElements(); (j > 0) && (apcDefined.GetPtr(j - 1)->mpcDefinition->miFirstToken > pcFunction->mpcDefinition->miFirstToken); j--)
			{
			}
			*apcDefined.InsertAt(j) = pcFunction;
		}
	}
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if (!pcFunction->mpcDefinition && pcFunction->mbImplicit)
		{
			apcDefined.Add(pcFunction);
		}
	}
	//One already waiting (called before its definition was seen) waits again in its place in that order.
	for (i = 0; i < apcDefined.NumElements(); i++)
	{
		pcFunction = apcDefined.GetPtr(i);
		if (pcFunction->mbQueued && !pcFunction->mbGenerated)
		{
			for (j = mpcUnit->muiQueueNext; j < mpcUnit->mapcQueue.NumElements(); j++)
			{
				if (mpcUnit->mapcQueue.GetPtr(j) == pcFunction)
				{
					mpcUnit->mapcQueue.RemoveAt(j);
					break;
				}
			}
			pcFunction->mbQueued = false;
		}
		mpcUnit->RequireFunction(pcFunction);
	}
	apcDefined.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A specialisation (pvInstance, a class or a function) is explicitly
// instantiated once in a unit, not extern; an error (and false) the
// second time.  [temp.spec] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::ExplicitlyInstantiatedOnce(void* pvInstance, CCPPSyntax* pcNode)
{
	char	c;

	if (mpcUnit->mcExplicitInstances.Get((int64)(size)pvInstance))
	{
		return mpcUnit->mcDiagnostics.Error(pcNode, "The specialisation is explicitly instantiated twice");
	}
	c = 1;
	mpcUnit->mcExplicitInstances.Put((int64)(size)pvInstance, &c);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// template int B<int>::K(); names a member function of a class template's
// instance, of the declarator's type where it has several of the name: it
// is generated unless bExtern.  [temp.explicit] 1, 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::ExplicitMemberInstantiation(CCPPSyntaxInitDeclarator* pcInit, bool bExtern)
{
	CCPPSyntaxName*		pcName;
	CCPPClass*			pcClass;
	CCPPFunction*		pcFunction;
	CCPPFunction*		pcFound;
	SCPP65Type*			psType;
	int64				lliName;
	size				i;

	pcName = pcInit->mpcDeclarator->GetName();
	pcClass = mpcUnit->GetComponentClass(pcName->GetQualifier(), NULL, pcInit);
	if (!pcClass || !pcClass->mpcTemplate || !mpcUnit->CompleteClass(pcClass, pcInit))
	{
		return mpcUnit->mcDiagnostics.Error(pcInit, "The explicit instantiation names no member of a class template's instance");
	}
	lliName = mpcUnit->GetFunctionName(pcName->GetLast());
	mpcUnit->mcContext.EnterBindings(pcClass->mpcBindings);
	mpcUnit->mcDiagnostics.BeginQuiet();
	psType = mpcUnit->mcDeclarators.GetType(NULL, pcInit->mpcDeclarator, pcInit);
	mpcUnit->mcDiagnostics.EndQuiet();
	mpcUnit->mcContext.LeaveBindings();
	pcFound = NULL;
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if ((pcFunction->mlliName != lliName) || pcFunction->mpcTemplate)
		{
			continue;
		}
		if (!pcFound || (psType && psType->IsFunction() && psType->papsParameters && (psType->papsParameters->NumElements() == pcFunction->mapsParameters.NumElements()) && mpcUnit->GetFunctionDeclarer()->FunctionType(pcFunction)->papsParameters && SameParameters(psType, mpcUnit->GetFunctionDeclarer()->FunctionType(pcFunction))))
		{
			pcFound = pcFunction;
		}
	}
	if (!pcFound)
	{
		return mpcUnit->mcDiagnostics.Error(pcInit, "The explicit instantiation names no member function of the instance");
	}
	pcFound->mbExternInstance = bExtern;
	if (!bExtern && pcFound->mpcDefinition)
	{
		mpcUnit->RequireFunction(pcFound);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two function types have the same parameter types.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::SameParameters(SCPP65Type* psFirst, SCPP65Type* psSecond)
{
	size	i;

	if (!psFirst->papsParameters || !psSecond->papsParameters || (psFirst->papsParameters->NumElements() != psSecond->papsParameters->NumElements()))
	{
		return false;
	}
	for (i = 0; i < psFirst->papsParameters->NumElements(); i++)
	{
		if (!psFirst->papsParameters->GetPtr(i)->Same(psSecond->papsParameters->GetPtr(i)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// template int Max<int>(int a, int b); and template int Max(int, int); name the
// instance of the function template Max whose arguments are the explicit ones
// and those deduced from the parameters' types.  Generated unless bExtern.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::ExplicitFunctionInstantiation(CCPPSyntaxInitDeclarator* pcInit, bool bExtern)
{
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPTemplate*				pcTemplate;
	CCPPTemplateArguments*		pcBindings;
	CCPPFunction*				pcFunction;
	SCPP65Type*					psType;
	CArrayCPP65Types			apsParameters;
	CCPPDeduction				cDeduction;
	CChars						szArguments;
	int64						lliName;
	size						i;

	pcName = pcInit->mpcDeclarator ? pcInit->mpcDeclarator->GetName() : NULL;
	pcSuffix = pcInit->mpcDeclarator ? pcInit->mpcDeclarator->GetFunctionSuffix() : NULL;
	if (pcName && pcSuffix && pcName->IsQualifiedByClass() && !pcName->GetLast()->mbTemplateArguments)
	{
		return ExplicitMemberInstantiation(pcInit, bExtern);
	}
	if (!pcName || !pcSuffix || pcName->IsQualifiedByClass())
	{
		return mpcUnit->mcDiagnostics.Error(pcInit, "Only explicit instantiations of class templates, their member functions and function templates are supported by the compiler yet");
	}
	pcComponent = pcName->GetLast();
	pcTemplate = pcName->mpcSymbol ? GetTemplate(pcName->mpcSymbol) : NULL;
	if (!pcTemplate || !pcTemplate->IsFunction())
	{
		lliName = mpcUnit->GetComponentName(pcComponent);
		pcTemplate = (lliName != -1) ? GetTemplateByName(lliName, CPPS_FunctionDefinition, mpcUnit->ComponentLookup(pcComponent)) : NULL;
	}
	if (!pcTemplate)
	{
		return mpcUnit->mcDiagnostics.Error(pcInit, "The explicit instantiation names no function template");
	}

	//The parameters' types are the deduced arguments' (as FindFunctionSpecialisation deduces them).
	mpcUnit->mcContext.EnterBindings(NULL);
	apsParameters.Init();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = mpcUnit->mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
		apsParameters.Add(psType ? psType->WithoutReference() : NULL);
	}
	cDeduction.Init(mpcUnit, pcTemplate);
	cDeduction.Deduce(&apsParameters, NULL);
	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, pcComponent->mbTemplateArguments ? &pcComponent->macpTemplateArguments : NULL, cDeduction.GetTypes(), &szArguments, pcInit, cDeduction.GetValues());
	cDeduction.Kill();
	apsParameters.Kill();
	mpcUnit->mcContext.LeaveBindings();

	pcFunction = pcBindings ? InstanceOfFunction(pcTemplate, pcBindings, &szArguments, pcInit) : NULL;
	szArguments.Kill();
	if (!pcFunction)
	{
		return false;
	}
	if (!bExtern && !ExplicitlyInstantiatedOnce(pcFunction, pcInit))
	{
		return false;
	}
	pcFunction->mbExternInstance = bExtern;
	if (!bExtern)
	{
		mpcUnit->RequireFunction(pcFunction);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The function is generated by the unit with its explicit instantiation, not by
// this one: an instance extern template names (of a function template, or a member
// of a class template defined outside the class) that isn't inline, constexpr or
// written by the compiler.  [temp.explicit] 10: those still are generated where
// used.  The index's and the IDE's checks check it as ever.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::IsInstantiatedElsewhere(CCPPFunction* pcFunction)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxName*					pcName;

	pcDefinition = pcFunction->mpcDefinition;
	if (mpcUnit->mpcSemanticPass || !pcDefinition || pcFunction->mbImplicit)
	{
		return false;
	}
	if (pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & (CPPDS_INLINE | CPPDS_CONSTEXPR)))
	{
		return false;
	}
	if (pcFunction->mbExternInstance)
	{
		return true;
	}

	//A member template's instance isn't the class's to instantiate.  A member defined in the class is inline.
	if (!pcFunction->mpcClass || !pcFunction->mpcClass->mbExternInstance || pcFunction->mpcTemplate)
	{
		return false;
	}
	pcName = pcDefinition->mpcDeclarator ? pcDefinition->mpcDeclarator->GetName() : NULL;
	return pcName && pcName->IsQualifiedByClass();
}


//////////////////////////////////////////////////////////////////////////
//
// The member class template of the symbol that pcClass has, or if it is
// NULL the class it is named in (MemberScopeClass: for a class template's
// member, its instance in use), the class laid out first, as it keeps its
// member templates then.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPInstantiator::MemberClassTemplate(CCPPSymbol* pcSymbol, CCPPClass* pcClass)
{
	CCPPTemplate*	pcTemplate;
	size			i;

	pcClass = pcClass ? pcClass : mpcUnit->MemberScopeClass(pcSymbol);
	if (!pcClass || !mpcUnit->CompleteClass(pcClass, NULL))
	{
		return NULL;
	}
	for (i = 0; i < mpcUnit->mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
		if (!pcTemplate->mbMember && (pcTemplate->mpcPrimary == pcTemplate) && (pcTemplate->mpcClass == pcClass) && (pcTemplate->mpcSymbol == pcSymbol))
		{
			return pcTemplate;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The template pcSymbol declares.  A function template declared before it is
// defined has a symbol for each declaration (chained by mpcNext); the template is
// recorded under the definition's.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPInstantiator::GetTemplate(CCPPSymbol* pcSymbol)
{
	CCPPTemplate*	pcTemplate;
	CCPPSymbol*		pcDeclaration;
	size			i;

	//A member class template is its class's: the class in use where it is named.
	if (pcSymbol && (pcSymbol->meKind == CPPSK_ClassTemplate) && pcSymbol->mpcScope && (pcSymbol->mpcScope->meKind == CPPSC_Class))
	{
		return MemberClassTemplate(pcSymbol, NULL);
	}
	for (pcDeclaration = pcSymbol; pcDeclaration; pcDeclaration = pcDeclaration->mpcNext)
	{
		if ((pcDeclaration != pcSymbol) && (pcDeclaration->meKind != pcSymbol->meKind))
		{
			continue;
		}
		for (i = 0; i < mpcUnit->mapcTemplates.NumElements(); i++)
		{
			pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
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
CCPPTemplate* CCPPInstantiator::GetTemplateByName(int64 lliName, ECPPSyntax eDefinition, CCPPScope* pcLookup)
{
	CCPPTemplate*	pcTemplate;
	CCPPTemplate*	pcFound;
	size			i;

	pcFound = NULL;
	for (i = 0; i < mpcUnit->mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
		if (!pcTemplate->mbMember && (pcTemplate->mpcPrimary == pcTemplate) && pcTemplate->mpcSymbol && (pcTemplate->mpcSymbol->mlliName == lliName) && (pcTemplate->DefinitionKind() == eDefinition))
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
CCPPTemplate* CCPPInstantiator::GetComponentTemplate(CCPPSyntaxNameComponent* pcComponent)
{
	int64	lliName;

	lliName = mpcUnit->GetComponentName(pcComponent);
	return (lliName != -1) ? GetTemplateByName(lliName, CPPS_ClassSpecifier, mpcUnit->ComponentLookup(pcComponent)) : NULL;
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
CCPPTemplateArguments* CCPPInstantiator::BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode, CArrayInt64* palliDeduced, CArrayCPPDeducedPacks* pasPacks)
{
	CArrayCPPExpandedArguments	asArguments;
	CCPPTemplateArguments*		pcBindings;
	SCPPBinding*				psBinding;
	SCPPExpandedArgument*		psArgument;
	CCPPSyntax*					pcParameter;
	CCPPSyntaxTypeParameter*	pcType;
	CCPPSyntaxParameter*		pcValue;
	CCPPTemplate*				pcDeduced;
	size						i;
	bool						bResult;

	asArguments.Init();
	bResult = !papcArguments || mpcUnit->ExpandTemplateArguments(papcArguments, &asArguments);
	if (bResult && (asArguments.NumElements() > pcTemplate->macpParameters.NumElements()) && !IsPackParameter(pcTemplate, pcTemplate->macpParameters.NumElements() - 1))
	{
		//[temp.arg] 1, [temp.deduct] 2
		bResult = mpcUnit->mcDiagnostics.Error(pcNode, "More template arguments than the template has parameters");
	}
	pcBindings = NewBindings();
	for (i = 0; bResult && (i < pcTemplate->macpParameters.NumElements()); i++)
	{
		pcParameter = pcTemplate->macpParameters.GetPtr(i);
		psArgument = (i < asArguments.NumElements()) ? asArguments.Get(i) : NULL;
		psBinding = pcBindings->macBindings.Add();
		psBinding->psType = NULL;
		psBinding->lliValue = 0;
		psBinding->pcParameter = NULL;
		psBinding->papsPack = NULL;
		psBinding->palliPack = NULL;
		psBinding->bSelf = false;
		psBinding->pcArgument = NULL;
		psBinding->pcTemplate = NULL;
		psBinding->pcAlias = NULL;

		if (pcParameter->GetKind() == CPPS_TypeParameter)
		{
			pcType = (CCPPSyntaxTypeParameter*)pcParameter;
			psBinding->pcParameter = pcType->mpcSymbol;
			if (pcType->mbPack && (i >= asArguments.NumElements()) && BindDeducedPack(psBinding, pasPacks, i, pszArguments))
			{
				//A pack deduced whole (from Types<T...>), which needn't be the last parameter.
				continue;
			}
			if (pcType->mbPack)
			{
				//A pack (the last parameter) takes the rest of the explicit arguments, or else the deduced types.
				bResult = BindTypePack(psBinding, &asArguments, i, papsDeduced, pszArguments);
				break;
			}
			if (pcType->mpcTemplate)
			{
				//Deduced, it is the class template of the instance deduced for it (CCPPDeduction).
				pcDeduced = (!psArgument && papsDeduced && (i < papsDeduced->NumElements()) && papsDeduced->GetPtr(i)) ? papsDeduced->GetPtr(i)->pcClass->mpcTemplate->mpcPrimary : NULL;
				bResult = BindTemplate(psBinding, psArgument ? psArgument->pcArgument : pcType->mpcDefault, pcDeduced, pcNode, pszArguments);
				if (bResult && psBinding->pcTemplate && psBinding->pcTemplate->mpcDeclaration && !TemplateParametersMatch(pcType->mpcTemplate, psBinding->pcTemplate->mpcDeclaration))
				{
					//In C++14 a template template argument's parameters are the parameter's, kind for kind: one with a
					//defaulted parameter more isn't a template<class> class.  [temp.arg.template] 3
					bResult = mpcUnit->mcDiagnostics.Error(psArgument ? psArgument->pcArgument : pcNode, "The template argument's parameters aren't the template template parameter's");
				}
				if (!bResult)
				{
					break;
				}
				continue;
			}
			if (psArgument)
			{
				//const int is a different argument to int (its type is const).
				psBinding->psType = mpcUnit->ExpandedArgumentType(psArgument);
			}
			else if (papsDeduced && (i < papsDeduced->NumElements()) && papsDeduced->GetPtr(i))
			{
				psBinding->psType = papsDeduced->GetPtr(i);
			}
			else if (pcType->mpcDefault)
			{
				//A default can use the parameters before it.
				pcBindings->mpcOuter = mpcUnit->mcContext.GetBindings();
				mpcUnit->mcContext.EnterBindings(pcBindings);
				psBinding->psType = mpcUnit->mcDeclarators.GetTypeIdType(pcType->mpcDefault);
				mpcUnit->mcContext.LeaveBindings();
				pcBindings->mpcOuter = NULL;
			}
			if (!psBinding->psType)
			{
				bResult = mpcUnit->mcDiagnostics.Error(psArgument ? psArgument->pcArgument : pcNode, "The template argument is not a type the compiler supports");
				break;
			}
			pszArguments->Append('_');
			mpcUnit->mcMangler.AppendType(pszArguments, psBinding->psType);
		}
		else if (pcParameter->GetKind() == CPPS_Parameter)
		{
			pcValue = (CCPPSyntaxParameter*)pcParameter;
			psBinding->pcParameter = pcValue->mpcSymbol;
			if (pcValue->mpcDeclarator && pcValue->mpcDeclarator->mbPack && (i >= asArguments.NumElements()) && BindDeducedPack(psBinding, pasPacks, i, pszArguments))
			{
				continue;
			}
			if (pcValue->mpcDeclarator && pcValue->mpcDeclarator->mbPack)
			{
				bResult = BindValuePack(psBinding, &asArguments, i, pszArguments);
				break;
			}
			if (psArgument && IsEntityParameter(pcValue))
			{
				bResult = BindEntity(psBinding, psArgument, pszArguments);
				if (!bResult)
				{
					break;
				}
				continue;
			}
			if (psArgument)
			{
				bResult = mpcUnit->ExpandedArgumentValue(psArgument, &psBinding->lliValue) && ValueArgumentFits(pcValue, psArgument, psBinding->lliValue, pcBindings);
			}
			else if (palliDeduced && papsDeduced && (i < papsDeduced->NumElements()) && papsDeduced->GetPtr(i))
			{
				psBinding->lliValue = palliDeduced->GetValue(i);
			}
			else if (!pcValue->mpcDefault)
			{
				bResult = mpcUnit->mcDiagnostics.Error(pcNode, "The template argument must be a constant");
			}
			else
			{
				//A default can use the parameters before it: int N = sizeof(T).  [basic.scope.temp] 1
				pcBindings->mpcOuter = mpcUnit->mcContext.GetBindings();
				mpcUnit->mcContext.EnterBindings(pcBindings);
				bResult = mpcUnit->mcFolder.Constant(pcValue->mpcDefault, &psBinding->lliValue);
				mpcUnit->mcContext.LeaveBindings();
				pcBindings->mpcOuter = NULL;
				if (!bResult)
				{
					mpcUnit->mcFolder.ConstantError(pcNode, "The template argument must be a constant");
				}
			}
			if (!bResult)
			{
				break;
			}
			pszArguments->Append("_L");
			AppendMangledValue(pszArguments, psBinding->lliValue);
		}
		else
		{
			bResult = mpcUnit->mcDiagnostics.Error(pcParameter, "This template parameter is not supported by the compiler yet");
		}
	}
	asArguments.Kill();
	return bResult ? pcBindings : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a template argument written for a parameter of an integer type
// is a value of that type, as a converted constant expression must be:
// Byte<300> for an unsigned char isn't, nor Count<-1> for an unsigned int.
// An error (and false) if not.  The parameter's type is worked out with the
// arguments before it (pcBindings); one that can't be yet isn't checked.
// [temp.arg.nontype] 1, [expr.const] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::ValueArgumentFits(CCPPSyntaxParameter* pcParameter, SCPPExpandedArgument* psArgument, int64 lliValue, CCPPTemplateArguments* pcBindings)
{
	SCPP65Type*	psType;

	if (psArgument->bExpanded)
	{
		return true;
	}
	mpcUnit->mcContext.EnterBindings(pcBindings);
	mpcUnit->mcDiagnostics.BeginSilent();
	psType = mpcUnit->mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
	mpcUnit->mcDiagnostics.EndSilent();
	mpcUnit->mcContext.LeaveBindings();
	if (!psType || ConstantFits(psType, lliValue))
	{
		return true;
	}
	//Said again where the instance is named for good (CCPP65TypeChecker::ConstantValue): this may only be a try.
	mpcUnit->mbValueArgumentRefused = true;
	return mpcUnit->mcDiagnostics.ReportOnce(psArgument->pcArgument, "The constant's value isn't one of the type it is converted to");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a constant's value is one of an integer type's (any is a bool's,
// and an enum's isn't asked).  [expr.const] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::ConstantFits(SCPP65Type* psType, int64 lliValue)
{
	return !psType->IsInteger() || psType->pcEnum || (psType->eKind == CPP65TK_Bool) || (mpcUnit->mcFolder.ConstantOfType(psType, lliValue) == lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The type a non-type template parameter (its symbol) is declared with,
// with the bindings there are; psOtherwise if it can't be worked out.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPInstantiator::ValueParameterType(CCPPSymbol* pcParameter, SCPP65Type* psOtherwise)
{
	CCPPSyntaxParameter*	pcDeclaration;
	SCPP65Type*				psType;

	pcDeclaration = pcParameter ? (CCPPSyntaxParameter*)pcParameter->mpcDeclaration : NULL;
	if (!pcDeclaration || (pcDeclaration->GetKind() != CPPS_Parameter))
	{
		return psOtherwise;
	}
	psType = mpcUnit->mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcDeclaration->mpcDeclarator, pcDeclaration);
	return psType ? psType : psOtherwise;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a non-type template parameter is a pointer or a reference
// (template<int* P>, template<int (*F)(int)>, template<int& R>): its
// argument names an object or a function, not a value.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::IsEntityParameter(CCPPSyntaxParameter* pcParameter)
{
	return pcParameter->mpcDeclarator && ((pcParameter->mpcDeclarator->NumPointers() > 0) || pcParameter->mpcDeclarator->mpcNested);
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer or reference parameter takes its argument as it is written:
// &g or f (a pointer: the address of a global or a function) or g (a
// reference), which its name then stands for.  Mangled as _X, A for an
// address, the global's label or the function's name, E.  [temp.arg.nontype]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::BindEntity(SCPPBinding* psBinding, SCPPExpandedArgument* psArgument, CChars* pszArguments)
{
	CCPPSyntax*		pcEntity;
	CCPPSymbol*		pcSymbol;
	CCPPVariable*	pcVariable;
	CCPPClass*		pcClass;
	bool			bAddress;

	pcEntity = psArgument->bExpanded ? NULL : mpcUnit->SkipParentheses(psArgument->pcArgument);
	bAddress = pcEntity && (pcEntity->GetKind() == CPPS_UnaryExpression) && (((CCPPSyntaxUnaryExpression*)pcEntity)->meOperator == CT_BitwiseAnd);
	if (bAddress)
	{
		pcEntity = mpcUnit->SkipParentheses(((CCPPSyntaxUnaryExpression*)pcEntity)->mpcOperand);
	}
	pcSymbol = (pcEntity && (pcEntity->GetKind() == CPPS_IdExpression)) ? ((CCPPSyntaxIdExpression*)pcEntity)->mpcName->mpcSymbol : NULL;
	if (bAddress && pcSymbol && (pcSymbol->meKind == CPPSK_Field) && ((CCPPSyntaxIdExpression*)pcEntity)->mpcName->IsQualifiedByClass())
	{
		//&S::m for a pointer to a member: mangled as _XM, the class, the member's name, E.  [temp.arg.nontype] 1
		pcClass = mpcUnit->GetComponentClass(((CCPPSyntaxIdExpression*)pcEntity)->mpcName->GetQualifier(), NULL, pcEntity);
		if (!pcClass)
		{
			return mpcUnit->mcDiagnostics.Error(psArgument->pcArgument, "The template argument must name a member of a class");
		}
		psBinding->pcArgument = psArgument->pcArgument;
		pszArguments->Append("_XM");
		mpcUnit->mcMangler.AppendType(pszArguments, pcClass->mpsType);
		mpcUnit->mcMangler.AppendFunctionName(pszArguments, mpcUnit->GetComponentName(((CCPPSyntaxIdExpression*)pcEntity)->mpcName->GetLast()));
		pszArguments->Append('E');
		return true;
	}
	if (!bAddress && pcEntity && mpcUnit->GetTypeChecker()->TypeOf(pcEntity) && mpcUnit->GetTypeChecker()->TypeOf(pcEntity)->IsNullPointer())
	{
		//nullptr for a pointer: a null pointer of the parameter's type, mangled as _XN.  [temp.arg.nontype] 1
		psBinding->pcArgument = psArgument->pcArgument;
		pszArguments->Append("_XNE");
		return true;
	}
	pcVariable = pcSymbol ? mpcUnit->GetVariable(pcSymbol) : NULL;
	if (!(pcVariable && (pcVariable->meStorage == CPPVS_Global)) && !(pcSymbol && (pcSymbol->meKind == CPPSK_Function)))
	{
		return mpcUnit->mcDiagnostics.Error(psArgument->pcArgument, "The template argument must name a global or a function");
	}
	psBinding->pcArgument = psArgument->pcArgument;
	pszArguments->Append(bAddress ? "_XA" : "_X");
	if (pcVariable)
	{
		mpcUnit->mcMangler.AppendGlobalLabel(pszArguments, pcVariable);
	}
	else
	{
		mpcUnit->mcMangler.AppendFunctionName(pszArguments, pcSymbol->mlliName);
	}
	pszArguments->Append('E');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A template template parameter takes the class template its argument (or
// default) names: Use<Box, long>'s C is Box; or, with no argument, the one
// a call's arguments deduced (pcDeduced).  Mangled as _Y, the template's
// scopes and name, E.  [temp.arg.template]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::BindTemplate(SCPPBinding* psBinding, CCPPSyntax* pcArgument, CCPPTemplate* pcDeduced, CCPPSyntax* pcNode, CChars* pszArguments)
{
	CCPPSyntaxTypeId*			pcTypeId;
	CCPPSyntaxTypeSpecifier*	pcType;
	CCPPSyntaxName*				pcName;
	CCPPSymbol*					pcSymbol;
	char*						szName;
	int							iLength;

	pcTypeId = (pcArgument && (pcArgument->GetKind() == CPPS_TypeId)) ? (CCPPSyntaxTypeId*)pcArgument : NULL;
	pcType = (pcTypeId && pcTypeId->mpcSpecifiers) ? pcTypeId->mpcSpecifiers->mpcType : NULL;
	pcName = (pcType && (pcType->GetKind() == CPPS_NamedTypeSpecifier) && (!pcTypeId->mpcDeclarator || pcTypeId->mpcDeclarator->IsEmpty())) ? ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName : NULL;
	if (pcDeduced)
	{
		psBinding->pcTemplate = pcDeduced;
	}
	else if (pcName && pcName->mpcSymbol && (pcName->mpcSymbol->meKind == CPPSK_TemplateTemplateParameter))
	{
		//A template template parameter passed on: what it is bound to.
		psBinding->pcTemplate = FindBinding(pcName->mpcSymbol) ? FindBinding(pcName->mpcSymbol)->pcTemplate : NULL;
		psBinding->pcAlias = FindBinding(pcName->mpcSymbol) ? FindBinding(pcName->mpcSymbol)->pcAlias : NULL;
	}
	else if (pcName && pcName->mpcSymbol && (pcName->mpcSymbol->meKind == CPPSK_ClassTemplate) && !pcName->GetLast()->mbTemplateArguments)
	{
		psBinding->pcTemplate = GetTemplate(pcName->mpcSymbol);
	}
	else if (pcName && pcName->mpcSymbol && (pcName->mpcSymbol->meKind == CPPSK_AliasTemplate) && !pcName->GetLast()->mbTemplateArguments)
	{
		//template<class T> using Ptr = T*; is an argument too: TT<char> is then Ptr<char>.  [temp.arg.template] 1
		psBinding->pcAlias = pcName->mpcSymbol;
	}
	if (!psBinding->pcTemplate && !psBinding->pcAlias)
	{
		return mpcUnit->mcDiagnostics.Error(pcArgument ? pcArgument : pcNode, "The template argument must be a class template or an alias template");
	}
	pszArguments->Append("_Y");
	pcSymbol = psBinding->pcTemplate ? psBinding->pcTemplate->mpcSymbol : psBinding->pcAlias;
	mpcUnit->mcMangler.AppendScopeNames(pszArguments, pcSymbol->mpcScope);
	szName = mpcUnit->mpcTokeniser->GetIdentifier(pcSymbol->mlliName, &iLength);
	pszArguments->Append(szName, iLength);
	pszArguments->Append('E');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A type parameter pack takes the expanded arguments from uiFirst on, all
// types, or if there are none the deduced types from there.  Mangled as
// _J, its types, E.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::BindTypePack(SCPPBinding* psBinding, CArrayCPPExpandedArguments* pasArguments, size uiFirst, CArrayCPP65Types* papsDeduced, CChars* pszArguments)
{
	SCPP65Type*	psType;
	size		j;

	psBinding->papsPack = mpcUnit->NewPack();
	for (j = uiFirst; j < pasArguments->NumElements(); j++)
	{
		psType = mpcUnit->ExpandedArgumentType(pasArguments->Get(j));
		if (!psType)
		{
			return mpcUnit->mcDiagnostics.Error(pasArguments->Get(j)->pcArgument, "The template argument is not a type the compiler supports");
		}
		psBinding->papsPack->Add(psType);
	}
	for (j = uiFirst; (uiFirst >= pasArguments->NumElements()) && papsDeduced && (j < papsDeduced->NumElements()); j++)
	{
		if (papsDeduced->GetPtr(j))
		{
			psBinding->papsPack->Add(papsDeduced->GetPtr(j));
		}
	}
	pszArguments->Append("_J");
	for (j = 0; j < psBinding->papsPack->NumElements(); j++)
	{
		mpcUnit->mcMangler.AppendType(pszArguments, psBinding->papsPack->GetPtr(j));
	}
	pszArguments->Append('E');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two function declarations' return types are written the same
// (template<class T> int F(T) and long F(T) are two templates).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::SameReturnWritten(CCPPSyntaxDeclSpecifiers* pcFirst, CCPPSyntaxDeclSpecifiers* pcSecond)
{
	CChars	szFirst;
	CChars	szSecond;
	bool	bSame;

	if (!pcFirst || !pcSecond)
	{
		return pcFirst == pcSecond;
	}
	szFirst.Init();
	szSecond.Init();
	pcFirst->PrettyPrint(&szFirst, 0);
	pcSecond->PrettyPrint(&szSecond, 0);
	bSame = szFirst.Equals(&szSecond);
	szSecond.Kill();
	szFirst.Kill();
	return bSame;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a template template parameter's parameters (pcParameter) take
// the template argument's (pcArgument) as they are: as many, each of the
// same kind, unless the parameter's last is a pack, which takes the rest.
// [temp.arg.template] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::TemplateParametersMatch(CCPPSyntaxTemplateDeclaration* pcParameter, CCPPSyntaxTemplateDeclaration* pcArgument)
{
	size	i;
	size	uiParameters;

	if (!pcParameter)
	{
		return true;
	}
	uiParameters = pcParameter->macpParameters.NumElements();
	if ((uiParameters > 0) && IsPackParameter(pcParameter, uiParameters - 1))
	{
		return true;
	}
	if (uiParameters != pcArgument->macpParameters.NumElements())
	{
		return IsPackParameter(pcArgument, pcArgument->macpParameters.NumElements() - 1) && (pcArgument->macpParameters.NumElements() <= uiParameters + 1);
	}
	for (i = 0; i < uiParameters; i++)
	{
		if (pcParameter->macpParameters.GetPtr(i)->GetKind() != pcArgument->macpParameters.GetPtr(i)->GetKind())
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a template's parameter uiIndex (-1 for none) is a pack.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::IsPackParameter(CCPPSyntaxTemplateDeclaration* pcTemplate, size uiIndex)
{
	CCPPSyntax*	pcParameter;

	if (uiIndex >= pcTemplate->macpParameters.NumElements())
	{
		return false;
	}
	pcParameter = pcTemplate->macpParameters.GetPtr(uiIndex);
	if (pcParameter->GetKind() == CPPS_TypeParameter)
	{
		return ((CCPPSyntaxTypeParameter*)pcParameter)->mbPack;
	}
	return (pcParameter->GetKind() == CPPS_Parameter) && ((CCPPSyntaxParameter*)pcParameter)->mpcDeclarator && ((CCPPSyntaxParameter*)pcParameter)->mpcDeclarator->mbPack;
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter pack deduced whole (CCPPDeduction::DeducePack) for the
// parameter uiParameter, if there is one: its types or values, mangled as
// an explicit pack's are.  False if there is none.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::BindDeducedPack(SCPPBinding* psBinding, CArrayCPPDeducedPacks* pasPacks, size uiParameter, CChars* pszArguments)
{
	SCPPDeducedPack*	psPack;
	size				i;
	size				j;

	for (i = 0; pasPacks && (i < pasPacks->NumElements()); i++)
	{
		psPack = pasPacks->Get(i);
		if (psPack->uiParameter != uiParameter)
		{
			continue;
		}
		pszArguments->Append("_J");
		if (psPack->papsTypes)
		{
			psBinding->papsPack = mpcUnit->NewPack();
			for (j = 0; j < psPack->papsTypes->NumElements(); j++)
			{
				psBinding->papsPack->Add(psPack->papsTypes->GetPtr(j));
				mpcUnit->mcMangler.AppendType(pszArguments, psPack->papsTypes->GetPtr(j));
			}
		}
		else
		{
			psBinding->palliPack = mpcUnit->NewValuePack();
			for (j = 0; j < psPack->palliValues->NumElements(); j++)
			{
				psBinding->palliPack->Add(psPack->palliValues->GetValue(j));
				pszArguments->Append('L');
				AppendMangledValue(pszArguments, psPack->palliValues->GetValue(j));
				pszArguments->Append('E');
			}
		}
		pszArguments->Append('E');
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A non-type parameter pack takes the expanded arguments from uiFirst on,
// all constants.  Mangled as _J, L value E for each, E.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::BindValuePack(SCPPBinding* psBinding, CArrayCPPExpandedArguments* pasArguments, size uiFirst, CChars* pszArguments)
{
	int64	lliValue;
	size	j;

	psBinding->palliPack = mpcUnit->NewValuePack();
	pszArguments->Append("_J");
	for (j = uiFirst; j < pasArguments->NumElements(); j++)
	{
		if (!mpcUnit->ExpandedArgumentValue(pasArguments->Get(j), &lliValue))
		{
			return false;
		}
		psBinding->palliPack->Add(lliValue);
		pszArguments->Append('L');
		AppendMangledValue(pszArguments, lliValue);
		pszArguments->Append('E');
	}
	pszArguments->Append('E');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A template argument's value as it is mangled: n before a negative one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::AppendMangledValue(CChars* pszArguments, int64 lliValue)
{
	pszArguments->Append((lliValue < 0) ? "n" : "");
	pszArguments->Append((lliValue < 0) ? -lliValue : lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a template parameter in what is being generated.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPPInstantiator::FindBinding(CCPPSymbol* pcParameter)
{
	return mpcUnit->mcContext.GetBindings() ? mpcUnit->mcContext.GetBindings()->Find(pcParameter) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a class template for the arguments, made if it is new.  It is
// laid out when it is first needed whole, from the most specialised definition
// that matches the arguments.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPInstantiator::InstantiateClass(CCPPTemplate* pcTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPClass*				pcClass;
	CChars					szArguments;

	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, papcArguments, NULL, &szArguments, pcNode);
	pcClass = pcBindings ? InstanceOfClass(pcTemplate, pcBindings, &szArguments, pcNode) : NULL;
	szArguments.Kill();
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a class template for arguments that have been bound
// (pszArguments is them mangled), made if it is new.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPInstantiator::InstanceOfClass(CCPPTemplate* pcTemplate, CCPPTemplateArguments* pcBindings, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcSpecialised;
	CCPPTemplate*	pcSpecialisation;
	CCPPClass*	pcClass;
	size			i;

	for (i = 0; i < mpcUnit->mapcClasses.NumElements(); i++)
	{
		pcClass = mpcUnit->mapcClasses.GetPtr(i);
		if (pcClass->mpcTemplate && (pcClass->mpcTemplate->mpcPrimary == pcTemplate) && pcClass->mszArguments.Equals(pszArguments))
		{
			return pcClass;
		}
	}

	pcSpecialised = NULL;
	pcSpecialisation = pcBindings->mbSelf ? NULL : FindSpecialisation(pcTemplate, pcBindings, &pcSpecialised, pcNode);
	if (pcSpecialisation)
	{
		pcTemplate = pcSpecialisation;
		pcBindings = pcSpecialised;
	}
	if (pcTemplate->mpcPrimary->mpcClass && !pcBindings->mpcOuter)
	{
		//A member class template's instance sees the template arguments of the class it is in.
		pcBindings->mpcOuter = MemberTemplateClassBindings(pcTemplate->mpcPrimary);
	}

	//An instance used before the template was defined (UndefinedInstance) becomes this instance.
	pcClass = NULL;
	for (i = 0; i < mpcUnit->mapcClasses.NumElements(); i++)
	{
		if (mpcUnit->mapcClasses.GetPtr(i)->IsInstanceNamed(pcTemplate->mpcSymbol) && mpcUnit->mapcClasses.GetPtr(i)->mszArguments.Equals(pszArguments))
		{
			pcClass = mpcUnit->mapcClasses.GetPtr(i);
			pcClass->mpcSymbol = pcTemplate->mpcSymbol;
			pcClass->mszArguments.Kill();
			pcClass->mszArguments.Init();
			break;
		}
	}
	if (!pcClass)
	{
		pcClass = mpcUnit->NewClass(pcTemplate->mpcSymbol, pcTemplate->mpcSymbol->mlliName);
	}
	pcClass->mpcTemplate = pcTemplate;
	pcClass->mpcSpecifier = pcTemplate->IsDeclaredClass() ? NULL : (CCPPSyntaxClassSpecifier*)pcTemplate->mpcDefinition;
	pcClass->mpcBindings = pcBindings;
	pcBindings->mpcClass = pcClass;
	pcClass->mszArguments.Append(pszArguments);
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class that stands for a template's type parameter where the
// template is checked as itself: one for each parameter, complete, with
// nothing in it, and written as the parameter's name (CCPPTypeWriter).
// A value of it can be declared, copied, pointed to and passed; what is
// asked of it that only an argument would have (a member, an operator, a
// conversion) isn't there, so an expression that needs one has no type.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPInstantiator::PlaceholderClass(CCPPSymbol* pcParameter)
{
	CCPPClass*	pcClass;
	size		i;

	for (i = 0; i < mpcUnit->mapcClasses.NumElements(); i++)
	{
		pcClass = mpcUnit->mapcClasses.GetPtr(i);
		if (pcClass->mbPlaceholder && (pcClass->mpcSymbol == pcParameter))
		{
			return pcClass;
		}
	}
	pcClass = mpcUnit->NewClass(pcParameter, pcParameter->mlliName);
	pcClass->mbPlaceholder = true;
	pcClass->mbEmpty = true;
	pcClass->meState = CPPCS_Complete;
	mpcUnit->mcTypes.SetClassBytes(pcClass->mpsType, 1);
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// A template's parameters bound to themselves: each type parameter to
// its placeholder class, each value parameter to a value that isn't
// known (it is 1 where one is needed), a pack to nothing.  NULL if the
// template has a parameter that can't be (a template for a parameter).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPPInstantiator::BindAsItself(CCPPSyntaxTemplateDeclaration* pcDeclaration, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*		pcBindings;
	CArrayCPP65Types			apsTypes;
	CArrayInt64					alliValues;
	CCPPSyntax*					pcParameter;
	CCPPSyntaxTypeParameter*	pcType;
	size						i;
	bool						bBound;

	apsTypes.Init();
	alliValues.Init();
	bBound = true;
	for (i = 0; bBound && (i < pcDeclaration->macpParameters.NumElements()); i++)
	{
		pcParameter = pcDeclaration->macpParameters.GetPtr(i);
		if (pcParameter->GetKind() == CPPS_TypeParameter)
		{
			pcType = (CCPPSyntaxTypeParameter*)pcParameter;
			if (pcType->mbPack)
			{
				break;
			}
			bBound = pcType->mpcSymbol && !pcType->mpcTemplate;
			if (bBound)
			{
				apsTypes.Add(PlaceholderClass(pcType->mpcSymbol)->mpsType);
				alliValues.Add(0);
			}
		}
		else if (pcParameter->GetKind() == CPPS_Parameter)
		{
			if (((CCPPSyntaxParameter*)pcParameter)->mpcDeclarator && ((CCPPSyntaxParameter*)pcParameter)->mpcDeclarator->mbPack)
			{
				break;
			}
			apsTypes.Add(&mpcUnit->mcTypes.msInt);
			alliValues.Add(1);
		}
		else
		{
			bBound = false;
		}
	}

	pcBindings = NULL;
	if (bBound)
	{
		mpcUnit->mcDiagnostics.BeginQuiet();
		pcBindings = BindArguments(pcDeclaration, NULL, &apsTypes, pszArguments, pcNode, &alliValues);
		mpcUnit->mcDiagnostics.EndQuiet();
	}
	if (pcBindings)
	{
		pcBindings->mbSelf = true;
		for (i = 0; i < pcBindings->macBindings.NumElements(); i++)
		{
			pcBindings->macBindings.Get(i)->bSelf = true;
		}
	}
	alliValues.Kill();
	apsTypes.Kill();
	return pcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// A class template as itself: its instance with its parameters standing
// for themselves, which CArray<M> written in the template is too.  NULL
// if it can't be made.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPInstantiator::InstantiateClassAsItself(CCPPTemplate* pcTemplate)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPClass*				pcClass;
	CChars					szArguments;

	szArguments.Init();
	pcBindings = BindAsItself(pcTemplate->mpcDeclaration, &szArguments, pcTemplate->mpcDefinition);
	pcClass = pcBindings ? InstanceOfClass(pcTemplate, pcBindings, &szArguments, pcTemplate->mpcDefinition) : NULL;
	szArguments.Kill();
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// A function template as itself, a member template of a class too: its
// instance with its parameters standing for themselves.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPInstantiator::InstantiateFunctionAsItself(CCPPTemplate* pcTemplate)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPFunction*			pcFunction;
	CChars					szArguments;

	if (pcTemplate->mpcClass)
	{
		mpcUnit->mcDiagnostics.BeginQuiet();
		DefineMemberTemplate(pcTemplate, pcTemplate->mpcDeclaration);
		mpcUnit->mcDiagnostics.EndQuiet();
	}
	if (!pcTemplate->mpcDefinition || (pcTemplate->mpcDefinition->GetKind() != CPPS_FunctionDefinition))
	{
		return NULL;
	}
	szArguments.Init();
	pcBindings = BindAsItself(pcTemplate->mpcDeclaration, &szArguments, pcTemplate->mpcDefinition);
	pcFunction = pcBindings ? InstanceOfFunction(pcTemplate, pcBindings, &szArguments, pcTemplate->mpcDefinition) : NULL;
	szArguments.Kill();
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// An instance of a class template that has only been declared: an incomplete class (it can be pointed to) with the
// instance's name.  It becomes the instance if the template is defined later (InstantiateClass).
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPInstantiator::UndefinedInstance(CCPPSymbol* pcSymbol, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
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
	for (i = 0; i < mpcUnit->mapcClasses.NumElements(); i++)
	{
		pcClass = mpcUnit->mapcClasses.GetPtr(i);
		if (pcClass->IsInstanceNamed(pcSymbol) && pcClass->mszArguments.Equals(&szArguments))
		{
			szArguments.Kill();
			return pcClass;
		}
	}
	pcClass = mpcUnit->NewClass(pcSymbol, pcSymbol->mlliName);
	pcClass->mszArguments.Append(&szArguments);
	szArguments.Kill();
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The most specialised specialisation of a class template or a variable
// template that matches the arguments (pcArguments), with its own parameters
// bound in *ppcBindings.  An explicit specialisation is chosen first; of the
// partial ones, the one whose arguments match more of the types' structure.
// NULL if none matches.  [temp.class.spec.match]
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPInstantiator::FindSpecialisation(CCPPTemplate* pcPrimary, CCPPTemplateArguments* pcArguments, CCPPTemplateArguments** ppcBindings, CCPPSyntax* pcNode)
{
	CCPPPatternMatch		cMatch;
	CCPPTemplate*			pcTemplate;
	CCPPTemplate*			pcBest;
	CCPPTemplateArguments*	pcDeduced;
	CCPPTemplateArguments*	pcBestDeduced;
	CArrayCPPTemplates		apcMatched;
	CArrayTemplatePtr<CCPPTemplateArguments>	apcDeduced;
	CArrayInt				aiScores;
	size					i;
	size					j;
	int						iScore;
	int						iBest;
	bool					bMatches;
	bool					bAmbiguous;
	bool					bLess;
	bool					bFirst;
	bool					bSecond;
	bool					bKnownFirst;
	bool					bKnownSecond;

	pcBest = NULL;
	pcBestDeduced = NULL;
	iBest = -1;
	bAmbiguous = false;
	apcMatched.Init();
	apcDeduced.Init();
	aiScores.Init();
	for (i = 0; i < mpcUnit->mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
		if ((pcTemplate->mpcPrimary != pcPrimary) || (pcTemplate == pcPrimary) || !pcTemplate->mpcPattern)
		{
			continue;
		}

		cMatch.Init(mpcUnit, pcTemplate->mpcDeclaration);
		bMatches = cMatch.Match(&pcTemplate->mpcPattern->macpTemplateArguments, pcArguments);
		pcDeduced = cMatch.GetDeduced();
		iScore = cMatch.GetScore();
		cMatch.Kill();
		if (bMatches)
		{
			apcMatched.Add(pcTemplate);
			apcDeduced.Add(pcDeduced);
			aiScores.Add(iScore);
		}
	}

	//The one no other is more specialised than: one whose arguments, its parameters standing for themselves, the
	//other's pattern matches, and not the other way round (by the matches' scores where that can't be worked out).
	//[temp.class.order] 1
	for (i = 0; i < apcMatched.NumElements(); i++)
	{
		bLess = false;
		for (j = 0; !bLess && (j < apcMatched.NumElements()); j++)
		{
			if (j == i)
			{
				continue;
			}
			bFirst = SpecialisationAtLeastAs(apcMatched.GetPtr(j), apcMatched.GetPtr(i), pcPrimary, pcNode, &bKnownFirst);
			bSecond = SpecialisationAtLeastAs(apcMatched.GetPtr(i), apcMatched.GetPtr(j), pcPrimary, pcNode, &bKnownSecond);
			bLess = (bKnownFirst && bKnownSecond) ? (bFirst && !bSecond) : (aiScores.GetValue(j) > aiScores.GetValue(i));
		}
		if (bLess)
		{
			continue;
		}
		if (pcBest)
		{
			bAmbiguous = true;
			if (aiScores.GetValue(i) <= iBest)
			{
				continue;
			}
		}
		pcBest = apcMatched.GetPtr(i);
		pcBestDeduced = apcDeduced.GetPtr(i);
		iBest = aiScores.GetValue(i);
	}
	apcMatched.Kill();
	apcDeduced.Kill();
	aiScores.Kill();
	if (bAmbiguous)
	{
		//Kept (Defer): an instance is made once, and often where errors are dropped.
		mpcUnit->mcDiagnostics.Defer(pcNode ? pcNode->miFirstToken : -1, "More than one specialisation of the template matches, and none is more specialised than the others");
	}
	*ppcBindings = pcBestDeduced;
	return pcBest;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the partial specialisation pcFirst is at least as specialised as
// pcSecond: pcSecond's pattern matches pcFirst's arguments with pcFirst's
// parameters standing for types and values of their own.  *pbKnown false
// if that couldn't be worked out.  [temp.class.order] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::SpecialisationAtLeastAs(CCPPTemplate* pcFirst, CCPPTemplate* pcSecond, CCPPTemplate* pcPrimary, CCPPSyntax* pcNode, bool* pbKnown)
{
	CCPPPatternMatch		cMatch;
	CCPPTemplateArguments*	pcSelf;
	CCPPTemplateArguments*	pcArguments;
	CChars					szArguments;
	bool					bMatches;

	bMatches = false;
	pcArguments = NULL;
	szArguments.Init();
	mpcUnit->mcDiagnostics.BeginTrial();
	pcSelf = BindAsItself(pcFirst->mpcDeclaration, &szArguments, pcNode);
	szArguments.Clear();
	if (pcSelf)
	{
		pcSelf->mpcOuter = mpcUnit->mcContext.GetBindings();
		mpcUnit->mcContext.EnterBindings(pcSelf);
		pcArguments = BindArguments(pcPrimary->mpcDeclaration, &pcFirst->mpcPattern->macpTemplateArguments, NULL, &szArguments, pcNode);
		mpcUnit->mcContext.LeaveBindings();
		pcSelf->mpcOuter = NULL;
	}
	if (pcArguments)
	{
		cMatch.Init(mpcUnit, pcSecond->mpcDeclaration);
		bMatches = cMatch.Match(&pcSecond->mpcPattern->macpTemplateArguments, pcArguments);
		cMatch.Kill();
	}
	*pbKnown = mpcUnit->mcDiagnostics.EndTrial() && (pcArguments != NULL);
	szArguments.Kill();
	return bMatches;
}


//////////////////////////////////////////////////////////////////////////
//
// New empty bindings, kept until the unit is killed.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPPInstantiator::NewBindings(void)
{
	CCPPTemplateArguments*	pcBindings;

	pcBindings = NewMalloc<CCPPTemplateArguments>();
	pcBindings->Init();
	mpcUnit->mapcAllBindings.Add(pcBindings);
	return pcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a function template for explicit arguments and the types of the
// call's arguments.  Template parameters without an explicit argument are deduced
// (DeduceArguments).  An explicit specialisation for the arguments is used instead
// of the template's definition.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPInstantiator::InstantiateFunction(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPFunction*			pcFunction;
	CArrayCPP65Types		apsArguments;
	CCPPDeduction			cDeduction;
	CChars					szArguments;
	size					i;

	if (pcTemplate->mpcClass && !DefineMemberTemplate(pcTemplate, pcNode))
	{
		return NULL;
	}

	apsArguments.Init();
	for (i = 0; papcArguments && (i < papcArguments->NumElements()); i++)
	{
		apsArguments.Add(mpcUnit->GetTypeChecker()->TypeOf(papcArguments->GetPtr(i)));
	}
	cDeduction.Init(mpcUnit, pcTemplate);
	cDeduction.Deduce(&apsArguments, papcArguments);
	apsArguments.Kill();
	if (cDeduction.HasConflict() && !papcExplicit)
	{
		cDeduction.Kill();
		mpcUnit->mcDiagnostics.Error(pcNode, "The arguments deduce one template parameter as two types");
		return NULL;
	}

	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, papcExplicit, cDeduction.GetTypes(), &szArguments, pcNode, cDeduction.GetValues(), cDeduction.GetPacks());
	cDeduction.Kill();
	pcFunction = pcBindings ? InstanceOfFunction(pcTemplate, pcBindings, &szArguments, pcNode) : NULL;
	szArguments.Kill();
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a function template whose template arguments are
// deduced from psFunction, the type of the function wanted (a pointer to
// one is being initialised), or NULL, quietly, if they can't be or the
// instance isn't of that type.  [temp.deduct.funcaddr], [over.over] 2
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPInstantiator::InstantiateFunctionFor(CCPPTemplate* pcTemplate, SCPP65Type* psFunction, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPFunction*			pcFunction;
	CArrayCPP65Types		apsArguments;
	CCPPDeduction			cDeduction;
	CChars					szArguments;
	size					i;

	if (!psFunction->IsFunction() || (pcTemplate->mpcClass && !DefineMemberTemplate(pcTemplate, pcNode)))
	{
		return NULL;
	}
	mpcUnit->mcDiagnostics.BeginSilent();
	apsArguments.Init();
	for (i = 0; psFunction->papsParameters && (i < psFunction->papsParameters->NumElements()); i++)
	{
		apsArguments.Add(psFunction->papsParameters->GetPtr(i)->WithoutReference());
	}
	cDeduction.Init(mpcUnit, pcTemplate);
	cDeduction.Deduce(&apsArguments, NULL);
	apsArguments.Kill();
	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, NULL, cDeduction.GetTypes(), &szArguments, pcNode, cDeduction.GetValues(), cDeduction.GetPacks());
	cDeduction.Kill();
	pcFunction = pcBindings ? InstanceOfFunction(pcTemplate, pcBindings, &szArguments, pcNode) : NULL;
	szArguments.Kill();
	mpcUnit->mcDiagnostics.EndSilent();
	if (pcFunction && !psFunction->Equals(mpcUnit->GetFunctionDeclarer()->FunctionType(pcFunction)))
	{
		return NULL;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a function template for arguments that have been bound
// (pszArguments is them mangled), declared if it is new.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPInstantiator::InstanceOfFunction(CCPPTemplate* pcTemplate, CCPPTemplateArguments* pcBindings, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPTemplate*					pcSpecialisation;
	CCPPFunction*					pcFunction;
	CCPPSyntaxFunctionSuffix*		pcInClass;
	CCPPSyntaxParameter*			pcParameter;
	size							i;

	pcDefinition = (CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition;
	for (i = 0; i < pcTemplate->mapcInstances.NumElements(); i++)
	{
		pcFunction = pcTemplate->mapcInstances.GetPtr(i);
		if (pcFunction->mszArguments.Equals(pszArguments))
		{
			return pcFunction;
		}
	}

	pcSpecialisation = pcBindings->mbSelf ? NULL : FindFunctionSpecialisation(pcTemplate, pszArguments, pcNode);
	if (pcSpecialisation)
	{
		pcDefinition = (CCPPSyntaxFunctionDefinition*)pcSpecialisation->mpcDefinition;
	}

	//A member template's instance also sees its class's template arguments.
	if (pcTemplate->mpcClass)
	{
		pcBindings->mpcOuter = MemberTemplateClassBindings(pcTemplate);
	}
	else
	{
		pcBindings->mpcOuter = mpcUnit->mcContext.GetBindings();
	}
	//A member template only declared has instances only declared.
	pcDeclarator = pcDefinition ? pcDefinition->mpcDeclarator : pcTemplate->FunctionDeclarator();
	pcSpecifiers = pcDefinition ? pcDefinition->mpcSpecifiers : pcTemplate->FunctionSpecifiers();
	mpcUnit->mcContext.EnterBindings(pcBindings);
	mpcUnit->mcContext.EnterNamespace(pcTemplate->mpcNamespace);
	pcFunction = mpcUnit->GetFunctionDeclarer()->Declare(pcSpecifiers, pcDeclarator, pcDefinition != NULL, pcTemplate->mpcClass, false, pcTemplate, pszArguments);
	mpcUnit->mcContext.LeaveNamespace();
	mpcUnit->mcContext.LeaveBindings();
	if (!pcTemplate->mpcClass)
	{
		pcBindings->mpcOuter = NULL;
	}
	if (pcFunction)
	{
		//template<class T> int Exact(T) = delete;: each instance is deleted.  [dcl.fct.def.delete]
		pcFunction->mbDeleted = pcDefinition && (pcDefinition->meBody == CPPFB_Delete);
		pcFunction->mpcDefinition = pcFunction->mbDeleted ? NULL : pcDefinition;
		pcFunction->mpcBindings = pcBindings;
		pcFunction->mbInline = true;
		pcFunction->mbStatic |= pcTemplate->mbStatic;

		//A member template defined outside its class has its default arguments in the class.  [dcl.fct.default]
		pcInClass = (pcTemplate->mpcDeclarator && (pcTemplate->mpcDeclarator != pcDeclarator)) ? pcTemplate->mpcDeclarator->GetFunctionSuffix() : NULL;
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
// The explicit specialisations template<> template<> R S<A>::f<T>(...)
// written for a class template instance's member template (pcPrimary, of
// the instance), made its specialisations once.  [temp.expl.spec] 16
//
//////////////////////////////////////////////////////////////////////////
void CCPPInstantiator::AddMemberTemplateSpecialisations(CCPPTemplate* pcPrimary)
{
	CCPPSyntaxTemplateDeclaration*	pcInner;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxName*					pcName;
	CCPPTemplate*					pcTemplate;
	size							i;
	size							j;
	bool							bMade;

	for (i = 0; i < mpcUnit->masOutsideDefinitions.NumElements(); i++)
	{
		if (mpcUnit->masOutsideDefinitions.Get(i)->eKind != CPPOD_MemberTemplateSpecialisation)
		{
			continue;
		}
		pcInner = mpcUnit->masOutsideDefinitions.Get(i)->pcDeclaration;
		pcDefinition = mpcUnit->masOutsideDefinitions.Get(i)->pcFunction;
		pcName = mpcUnit->masOutsideDefinitions.Get(i)->pcName;
		if (!pcName || (mpcUnit->GetFunctionName(pcName->GetLast()) != pcPrimary->mlliName))
		{
			continue;
		}
		bMade = false;
		for (j = 0; !bMade && (j < mpcUnit->mapcTemplates.NumElements()); j++)
		{
			pcTemplate = mpcUnit->mapcTemplates.GetPtr(j);
			bMade = (pcTemplate->mpcPrimary == pcPrimary) && (pcTemplate->mpcDefinition == pcDefinition);
		}
		if (bMade || !mpcUnit->QualifierIsInstance(pcName->GetQualifier(), pcPrimary->mpcClass, pcDefinition))
		{
			continue;
		}
		pcTemplate = NewTemplate(pcPrimary->mpcSymbol, pcInner, pcDefinition, false);
		pcTemplate->mpcPrimary = pcPrimary;
		pcTemplate->mpcPattern = pcName->GetLast();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The explicit specialisation of a function template for the mangled arguments,
// or NULL.  A specialisation's arguments are its explicit ones (Size<char>) and
// those deduced from its parameters' types (Size(short)), worked out once.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPInstantiator::FindFunctionSpecialisation(CCPPTemplate* pcPrimary, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPTemplate*				pcTemplate;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	SCPP65Type*					psType;
	CArrayCPP65Types			apsParameters;
	CCPPDeduction				cDeduction;
	size						i;
	size						j;

	if (pcPrimary->mbMemberTemplate && pcPrimary->mpcClass && pcPrimary->mpcClass->mpcTemplate)
	{
		AddMemberTemplateSpecialisations(pcPrimary);
	}
	for (i = 0; i < mpcUnit->mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
		if ((pcTemplate->mpcPrimary != pcPrimary) || (pcTemplate == pcPrimary) || (pcTemplate->mpcDefinition->GetKind() != CPPS_FunctionDefinition))
		{
			continue;
		}
		if (!pcTemplate->mbArguments)
		{
			pcTemplate->mbArguments = true;
			mpcUnit->mcContext.EnterBindings(NULL);
			mpcUnit->mcDiagnostics.BeginQuiet();
			apsParameters.Init();
			pcSuffix = ((CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
			for (j = 0; pcSuffix && (j < pcSuffix->macpParameters.NumElements()); j++)
			{
				pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(j);
				psType = mpcUnit->mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
				apsParameters.Add(psType ? psType->WithoutReference() : NULL);
			}
			cDeduction.Init(mpcUnit, pcPrimary);
			cDeduction.Deduce(&apsParameters, NULL);
			if (!BindArguments(pcPrimary->mpcDeclaration, (pcTemplate->mpcPattern && pcTemplate->mpcPattern->mbTemplateArguments) ? &pcTemplate->mpcPattern->macpTemplateArguments : NULL, cDeduction.GetTypes(), &pcTemplate->mszArguments, pcNode, cDeduction.GetValues()))
			{
				pcTemplate->mszArguments.Clear();
			}
			cDeduction.Kill();
			apsParameters.Kill();
			mpcUnit->mcDiagnostics.EndQuiet();
			mpcUnit->mcContext.LeaveBindings();
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
CCPPTemplateArguments* CCPPInstantiator::RenameBindings(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	CCPPTemplateArguments*	pcBindings;

	pcBindings = RenamedBindings(pcClass->mpcBindings, pcDeclaration);
	pcBindings->mpcOuter = pcClass->mpcBindings;
	pcBindings->mpcClass = pcClass;
	pcBindings->mbSelf = pcClass->mpcBindings->mbSelf;
	return pcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// Template arguments as another declaration of their template names its
// parameters: each parameter of pcDeclaration bound to what the one in its
// place is bound to.  (Each declaration of a template has parameters of its
// own: template<class T> T Twice(T); and template<class U> U Twice(U u) { }.)
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPPInstantiator::RenamedBindings(CCPPTemplateArguments* pcFrom, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	CCPPTemplateArguments*	pcBindings;
	size					i;

	pcBindings = NewBindings();
	for (i = 0; (i < pcDeclaration->macpParameters.NumElements()) && (i < pcFrom->macBindings.NumElements()); i++)
	{
		*pcBindings->macBindings.Add() = *pcFrom->macBindings.Get(i);
		pcBindings->macBindings.Tail()->pcParameter = pcDeclaration->ParameterSymbol(i);
	}
	return pcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a template declaration's declaration defines a class template:
// struct I { ... }; with a name and no template arguments (not a
// specialisation).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::IsClassTemplateDefinition(CCPPSyntax* pcDeclaration)
{
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxClassSpecifier*		pcSpecifier;

	if (pcDeclaration->GetKind() != CPPS_SimpleDeclaration)
	{
		return false;
	}
	pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
	if (!pcSimple->mpcSpecifiers || !pcSimple->mpcSpecifiers->mpcType || (pcSimple->mpcSpecifiers->mpcType->GetKind() != CPPS_ClassSpecifier))
	{
		return false;
	}
	pcSpecifier = (CCPPSyntaxClassSpecifier*)pcSimple->mpcSpecifiers->mpcType;
	return pcSpecifier->mpcSymbol && pcSpecifier->mpcName && !pcSpecifier->mpcName->GetLast()->mbTemplateArguments;
}


//////////////////////////////////////////////////////////////////////////
//
// The definition, outside its class template, of the member class or the
// member class template pcSymbol names (template<class T> struct
// Outer<T>::Inner { ... }; template<class T> template<class U> struct
// Outer<T>::Pair { ... };), or NULL if none has been seen.
// *ppcDeclaration: the template declaration it is in, whose parameters are
// the class template's under the definition's names.  [temp.mem.class] 1,
// [temp.mem] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxClassSpecifier* CCPPInstantiator::MemberClassDefinition(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration** ppcDeclaration)
{
	SCPPOutsideDefinition*	psDefinition;
	size					i;

	for (i = 0; i < mpcUnit->masOutsideDefinitions.NumElements(); i++)
	{
		psDefinition = mpcUnit->masOutsideDefinitions.Get(i);
		if ((psDefinition->eKind == CPPOD_MemberClass) && (psDefinition->pcSpecifier->mpcSymbol == pcSymbol))
		{
			*ppcDeclaration = psDefinition->pcDeclaration;
			return psDefinition->pcSpecifier;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The template arguments of the class a member template is in, which the
// template's instances see: under the names the template's definition
// outside the class gives the class's parameters, if it is defined there
// (mpcClassDeclaration).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPPInstantiator::MemberTemplateClassBindings(CCPPTemplate* pcTemplate)
{
	return pcTemplate->mpcClassDeclaration ? RenameBindings(pcTemplate->mpcClass, pcTemplate->mpcClassDeclaration) : pcTemplate->mpcClass->mpcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps a member template of a class being laid out.  A member function
// template's instances are made when it is called (InstantiateFunction), a
// member class template's when it is named (GetTemplate finds the class's).
// Friend templates and constructor templates are left out.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::MemberTemplate(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration)
{
	CCPPSyntax*						pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxName*					pcName;
	CCPPTemplate*					pcTemplate;
	CCPPSymbol*						pcSymbol;
	CCPPSyntaxClassSpecifier*		pcSpecifier;
	CCPPSyntaxTemplateDeclaration*	pcOutside;
	int64							lliName;
	size							i;

	pcDeclaration = pcTemplateDeclaration->mpcDeclaration;
	if (pcTemplateDeclaration->mbInstantiation || (pcTemplateDeclaration->macpParameters.NumElements() == 0) || !pcDeclaration)
	{
		return true;
	}

	if (IsClassTemplateDefinition(pcDeclaration))
	{
		//template<class T> struct I { ... }; in a class: its own for each class it is in.
		pcTemplate = NewTemplate(((CCPPSyntaxClassSpecifier*)((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->mpcSpecifiers->mpcType)->mpcSymbol, pcTemplateDeclaration, ((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->mpcSpecifiers->mpcType, false);
		pcTemplate->mpcClass = pcClass;
		return true;
	}
	pcSimple = (pcDeclaration->GetKind() == CPPS_SimpleDeclaration) ? (CCPPSyntaxSimpleDeclaration*)pcDeclaration : NULL;
	if (pcSimple && pcSimple->mpcSpecifiers && pcSimple->mpcSpecifiers->mpcType && (pcSimple->mpcSpecifiers->mpcType->GetKind() == CPPS_ElaboratedTypeSpecifier) && (pcSimple->macpDeclarators.NumElements() == 0))
	{
		//template<class U> struct Pair; in a class template, defined outside it (template<class T> template<class U>
		//struct Outer<T>::Pair { ... };): the definition's, which names the class's template parameters its own way.
		pcSymbol = ((CCPPSyntaxElaboratedTypeSpecifier*)pcSimple->mpcSpecifiers->mpcType)->mpcName->mpcSymbol;
		pcOutside = NULL;
		pcSpecifier = pcSymbol ? MemberClassDefinition(pcSymbol, &pcOutside) : NULL;
		if (pcSpecifier)
		{
			pcTemplate = NewTemplate(pcSymbol, (CCPPSyntaxTemplateDeclaration*)pcOutside->mpcDeclaration, pcSpecifier, false);
			pcTemplate->mpcClass = pcClass;
			pcTemplate->mpcClassDeclaration = pcOutside;
		}
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
	if ((pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && VariableTemplateName((CCPPSyntaxSimpleDeclaration*)pcDeclaration))
	{
		if (VariableTemplateName((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->GetLast()->mbTemplateArguments)
		{
			//A member variable template is specialised outside its class, as g++ has it.  [temp.class.spec] 5
			return mpcUnit->mcDiagnostics.Defer(pcDeclaration->miFirstToken, "A member variable template is specialised outside its class");
		}
		//template<class T> static const int size = sizeof(T); in a class: a variable template that is its static member.
		pcTemplate = NewTemplate(VariableTemplateName((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->mpcSymbol, pcTemplateDeclaration, pcDeclaration, false);
		pcTemplate->mpcClass = pcClass;
		DefineMemberVariableTemplate(pcTemplate);
		for (i = 0; i < mpcUnit->masOutsideDefinitions.NumElements(); i++)
		{
			if (mpcUnit->masOutsideDefinitions.Get(i)->eKind == CPPOD_MemberVariableSpecialisation)
			{
				SpecialiseMemberVariableTemplate(pcTemplate, mpcUnit->masOutsideDefinitions.Get(i)->pcDeclaration);
			}
		}
		return true;
	}
	pcName = pcDeclarator ? pcDeclarator->GetName() : NULL;
	if (!pcName || (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_FRIEND)))
	{
		return true;
	}
	lliName = mpcUnit->GetFunctionName(pcName->GetLast());
	if ((lliName == -1) || (pcName->GetLast()->meKind == CPPNC_Destructor))
	{
		return true;
	}

	//A constructor template is a constructor the class declares: it is no aggregate, and is given no default constructor.
	//[class.ctor] 4, [dcl.init.aggr] 1
	pcClass->mbConstructors |= (lliName == pcClass->mlliName);

	pcTemplate = NewTemplate(pcName->mpcSymbol, pcTemplateDeclaration, (pcDeclaration->GetKind() == CPPS_FunctionDefinition) ? pcDeclaration : NULL, true);
	pcTemplate->mbMemberTemplate = true;
	pcTemplate->mpcClass = pcClass;
	pcTemplate->mlliName = lliName;
	pcTemplate->mpcDeclarator = pcDeclarator;
	pcTemplate->mpcSpecifiers = pcSpecifiers;
	pcTemplate->mbStatic = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_STATIC);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The & or && after a member function's parameters.
//
//////////////////////////////////////////////////////////////////////////
ECPPRefQualifier CCPPInstantiator::RefQualifier(CCPPSyntaxFunctionSuffix* pcSuffix)
{
	if (CPPHasKeyword(&pcSuffix->macpQualifiers, CT_LogicalAnd))
	{
		return CPPRQ_Rvalue;
	}
	return CPPHasKeyword(&pcSuffix->macpQualifiers, CT_BitwiseAnd) ? CPPRQ_Lvalue : CPPRQ_None;
}


//////////////////////////////////////////////////////////////////////////
//
// A member template declared in its class but defined outside it takes the outside
// definition with its name and parameters (and its template parameters, which the
// definition uses): the one whose parameters are written the same, as overloads
// that differ only in their parameters' types are, or else the first with as many.
// The default arguments are the declaration's (DeclaredDefaultArguments).
// One with no definition stays only declared (its declarator and specifiers in the
// class: CCPPTemplate::FunctionDeclarator), as SFINAE's test functions are.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPInstantiator::DefineMemberTemplate(CCPPTemplate* pcTemplate, CCPPSyntax* pcNode)
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
	cKey.Init(mpcUnit->mpcTokeniser, mpcUnit->mpcTree);
	szSignature.Init();
	if (pcSuffix)
	{
		cKey.GetSignature(pcSuffix, &szSignature);
	}
	pcFound = NULL;
	for (i = 0; i < mpcUnit->mapcTemplates.NumElements(); i++)
	{
		pcOutside = mpcUnit->mapcTemplates.GetPtr(i);
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
		//Only declared: its instances are declarations, which only a call needs defined (CCPP65TypeChecker::Call).
		return true;
	}
	DeclaredDefaultArguments(pcTemplate->mpcDeclaration, pcTemplate->mpcDeclarator, pcFound->mpcDeclaration, ((CCPPSyntaxFunctionDefinition*)pcFound->mpcDefinition)->mpcDeclarator);
	pcTemplate->mpcDeclaration = pcFound->mpcDeclaration;
	pcTemplate->mpcDefinition = pcFound->mpcDefinition;
	pcTemplate->mpcClassDeclaration = pcFound->mpcClassDeclaration;
	return true;
}

