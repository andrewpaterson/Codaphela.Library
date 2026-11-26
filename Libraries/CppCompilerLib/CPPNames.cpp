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
#include "CPPNames.h"


//////////////////////////////////////////////////////////////////////////
//
// The unit's diagnostics, declarators and context, asked for once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPNames::Init(CCPPUnit* pcUnit, CCPPBinder* pcBinder)
{
	mpcUnit = pcUnit;
	mpcBinder = pcBinder;
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mpcDeclarators = pcUnit->GetDeclarators();
	mpcContext = pcUnit->GetContext();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPNames::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The class a (possibly qualified) name's last component names.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPNames::GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSymbol*	pcSymbol;
	CCPPClass*	pcClass;

	//P<char>::Q<long>: P<char>'s member class template's instance.
	pcSymbol = pcName->mpcSymbol;
	if (pcName->GetQualifier() && pcSymbol && (pcSymbol->meKind == CPPSK_ClassTemplate) && pcSymbol->mpcScope && (pcSymbol->mpcScope->meKind == CPPSC_Class))
	{
		return QualifiedClass(pcName, pcNode);
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Class) && pcName->IsQualifiedByInstance())
	{
		//P<char>::Inner: that instance's member class.  [temp.mem.class]
		mpcDiagnostics->BeginSilent();
		pcClass = QualifiedClass(pcName, pcNode);
		mpcDiagnostics->EndSilent();
		if (pcClass)
		{
			return pcClass;
		}
	}
	return GetComponentClass(pcName->GetLast(), pcSymbol, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// The class a qualified name's qualifiers name.  Of a name of two
// components, it is the class the parser found its qualifier to be, if it
// found one (GetComponentClass).  Of three or more, the qualifiers are
// walked from the first, each found in the class before it
// (QualifiersClass): in W<int>::Type::N the parser found W's Type,
// whatever W's T is, where W<int>'s is meant, and a member class named
// through an instance is that instance's own (Outer<char>::Inner::Count()).
// A qualifier the parser couldn't resolve is walked to as well
// (TEXT::SizePolicy::Copy(...)).  [class.qual] 1, [temp.mem.class]
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPNames::NameQualifierClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSyntaxNameComponent*	pcQualifier;

	pcQualifier = pcName->GetQualifier();
	if (pcQualifier->mpcSymbol && (pcName->macpComponents.NumElements() == 2))
	{
		return GetComponentClass(pcQualifier, NULL, pcNode);
	}
	return QualifiersClass(pcName, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// The class pcName's qualifiers (its components before the last) name.  A component after a class is the member
// typedef, class or class template instance of that name of the class (MemberClass): in
// TEMPLATE::SizePolicy, SizePolicy is a typedef of the class TEMPLATE is bound to, or of one of its bases (the parser
// leaves it unresolved), and in W<int>::Type, Type is W<int>'s typedef, whatever W's T is (the parser found W's).
// NULL if one isn't a class.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPNames::QualifiersClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPClass*					pcClass;
	size						i;

	pcClass = NULL;
	for (i = 0; i + 1 < pcName->macpComponents.NumElements(); i++)
	{
		pcComponent = (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(i);
		if (pcComponent->mpcSymbol && (pcComponent->mpcSymbol->meKind == CPPSK_Namespace))
		{
			pcClass = NULL;
			continue;
		}
		pcClass = pcClass ? MemberClass(pcClass, pcComponent, pcNode) : GetComponentClass(pcComponent, NULL, pcNode);
		if (!pcClass)
		{
			return NULL;
		}
	}
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a qualified name the parser couldn't resolve names: typename
// C::In or typename C::template In<long>, with C bound to a class, is that
// class's member (MemberClass).  NULL if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPNames::QualifiedClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPClass*	pcClass;

	pcClass = (pcName->macpComponents.NumElements() > 1) ? QualifiersClass(pcName, pcNode) : NULL;
	return pcClass ? MemberClass(pcClass, pcName->GetLast(), pcNode) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The member typedef or class of pcClass a name component names, worked out
// with the bindings of the class that declares it, or the instance of its
// member class template (In<long>, its arguments as they are where the name
// is), or the base of pcClass it names.  NULL if it is none of them.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPNames::MemberClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
{
	CCPPClass*		pcDeclaring;
	CCPPClass*		pcBase;
	CCPPSymbol*		pcMember;
	CCPPTemplate*	pcTemplate;
	SCPP65Type*		psType;
	int64			lliName;

	if (!mpcUnit->GetClasses()->CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	lliName = mpcUnit->GetComponentName(pcComponent);
	pcDeclaring = NULL;
	pcMember = pcClass->FindMemberSymbol(lliName, CPPSK_ClassTemplate, &pcDeclaring);
	pcTemplate = (pcMember && pcComponent->mbTemplateArguments) ? mpcUnit->GetInstantiator()->MemberClassTemplate(pcMember, pcDeclaring) : NULL;
	if (pcTemplate)
	{
		return mpcUnit->GetInstantiator()->InstantiateClass(pcTemplate, &pcComponent->macpTemplateArguments, pcNode);
	}
	pcMember = pcMember ? NULL : pcClass->FindMemberSymbol(lliName, CPPSK_Typedef, &pcDeclaring);
	if (!pcMember)
	{
		pcMember = pcClass->FindMemberSymbol(lliName, CPPSK_Class, &pcDeclaring);
	}
	if (!pcMember || (pcMember == pcDeclaring->mpcSymbol))
	{
		//C::X<char>::Type, where X<char> is C's base: a base's name is found in the class (its injected class name).
		//[class.qual] 1, [temp.local] 1
		pcMember = pcComponent->mpcSymbol;
		pcBase = (pcMember && ((pcMember->meKind == CPPSK_Class) || (pcMember->meKind == CPPSK_ClassTemplate))) ? GetComponentClass(pcComponent, NULL, pcNode) : NULL;
		return (pcBase && mpcUnit->GetClasses()->IsDerived(pcClass, pcBase, NULL)) ? pcBase : NULL;
	}
	mpcContext->EnterBindings(pcDeclaring->mpcBindings);
	if (pcMember->meKind == CPPSK_Typedef)
	{
		psType = mpcDeclarators->GetSymbolType(pcMember, pcNode);
		pcClass = (psType && psType->IsClass()) ? psType->pcClass : NULL;
	}
	else
	{
		pcClass = GetComponentClass(pcComponent, pcMember, pcNode);
	}
	mpcContext->LeaveBindings();
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a name component names: a class, a typedef of one, an instance of a
// class template (Name<Arguments>), a class template's own name inside it, or the
// type of decltype(e)::'s e.  A base template's name without arguments is psContext's
// (or else the current member function's class's) base instance of it.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPNames::GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcContext)
{
	CCPPTemplate*	pcTemplate;
	CCPPTemplateArguments*	pcBindings;
	SCPPBinding*	psBinding;
	CCPPClass*		pcClass;
	SCPP65Type*		psType;
	int64			lliName;

	if (!pcSymbol && pcComponent)
	{
		pcSymbol = pcComponent->mpcSymbol;
	}
	if (pcComponent && (pcComponent->meKind == CPPNC_Decltype))
	{
		//[dcl.type.simple] 4: a class as it is, not a reference to one.
		psType = mpcDeclarators->GetTypeIdType(pcComponent->mpcType);
		return (psType && psType->IsClass()) ? psType->pcClass : NULL;
	}
	if (!pcSymbol && pcComponent)
	{
		//Names in templates are not always resolved by the parser.
		lliName = mpcUnit->GetComponentName(pcComponent);
		pcTemplate = mpcUnit->GetInstantiator()->GetComponentTemplate(pcComponent);
		if (pcTemplate)
		{
			pcSymbol = pcTemplate->mpcSymbol;
		}
		else if (lliName != -1)
		{
			return mpcUnit->GetClasses()->FindClass(lliName, ComponentLookup(pcComponent));
		}
	}
	if (!pcSymbol)
	{
		return NULL;
	}

	if (pcSymbol->meKind == CPPSK_Class)
	{
		return mpcUnit->GetClasses()->GetClass(pcSymbol, pcNode);
	}
	if (pcSymbol->meKind == CPPSK_Typedef)
	{
		psType = mpcDeclarators->GetSymbolType(pcSymbol, pcNode);
		return (psType && psType->IsClass()) ? psType->pcClass : NULL;
	}
	if (pcSymbol->meKind == CPPSK_TypeParameter)
	{
		psBinding = mpcUnit->GetInstantiator()->FindBinding(pcSymbol);
		return (psBinding && psBinding->psType && psBinding->psType->IsClass()) ? psBinding->psType->pcClass : NULL;
	}
	if (pcSymbol->meKind == CPPSK_TemplateTemplateParameter)
	{
		//C<T>, C bound to a class template: its instance.
		psBinding = mpcUnit->GetInstantiator()->FindBinding(pcSymbol);
		if (!psBinding || !psBinding->pcTemplate || !pcComponent || !pcComponent->mbTemplateArguments)
		{
			mpcDiagnostics->Error(pcNode, "A template template parameter used outside an instantiation");
			return NULL;
		}
		return mpcUnit->GetInstantiator()->InstantiateClass(psBinding->pcTemplate, &pcComponent->macpTemplateArguments, pcNode);
	}
	if (pcSymbol->meKind == CPPSK_ClassTemplate)
	{
		pcTemplate = mpcUnit->GetInstantiator()->GetTemplate(pcSymbol);
		if (!pcTemplate)
		{
			//A pointer to an instance of a template that is only declared needs no definition.
			pcClass = (pcComponent && pcComponent->mbTemplateArguments) ? mpcUnit->GetInstantiator()->UndefinedInstance(pcSymbol, &pcComponent->macpTemplateArguments, pcNode) : NULL;
			if (!pcClass)
			{
				mpcDiagnostics->Error(pcNode, "The class template has not been defined");
			}
			return pcClass;
		}
		if (pcComponent && pcComponent->mbTemplateArguments)
		{
			return mpcUnit->GetInstantiator()->InstantiateClass(pcTemplate, &pcComponent->macpTemplateArguments, pcNode);
		}

		//The template's own name inside it is the instance being made, and a base's name
		//in a class derived from an instance is that instance.
		for (pcBindings = mpcContext->GetBindings(); pcBindings; pcBindings = pcBindings->mpcOuter)
		{
			if (pcBindings->mpcClass && pcBindings->mpcClass->mpcTemplate && (pcBindings->mpcClass->mpcTemplate->mpcPrimary == pcTemplate))
			{
				return pcBindings->mpcClass;
			}
		}
		if (!pcContext && mpcUnit->GetClosures()->OuterFunction())
		{
			pcContext = mpcUnit->GetClosures()->OuterFunction()->mpcClass;
		}
		pcClass = pcContext ? FindBaseInstance(pcContext, pcTemplate) : NULL;
		if (pcClass)
		{
			return pcClass;
		}
		mpcDiagnostics->Error(pcNode, "A class template needs template arguments");
		return NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a name's component is declared if the parser resolved it, else where
// the name was looked up: what to choose between things of its name with.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPNames::ComponentLookup(CCPPSyntaxNameComponent* pcComponent)
{
	if (pcComponent->mpcSymbol)
	{
		return pcComponent->mpcSymbol->mpcScope;
	}
	return (pcComponent->mpcParent && (pcComponent->mpcParent->GetKind() == CPPS_Name)) ? ((CCPPSyntaxName*)pcComponent->mpcParent)->mpcLookup : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a class template that is pcClass or one of its bases, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPNames::FindBaseInstance(CCPPClass* pcClass, CCPPTemplate* pcTemplate)
{
	CCPPClass*		pcBase;
	size			i;

	mpcUnit->GetStatistics()->Count(CPPCC_BaseWalks);
	if (pcClass->mpcTemplate && (pcClass->mpcTemplate->mpcPrimary == pcTemplate))
	{
		return pcClass;
	}
	mpcDiagnostics->BeginQuiet();
	mpcUnit->GetClasses()->ResolveBases(pcClass, NULL);
	mpcDiagnostics->EndQuiet();
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcBase = FindBaseInstance(pcClass->macBases.Get(i)->pcClass, pcTemplate);
		if (pcBase)
		{
			return pcBase;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class template's instance, or the member class of one, that a symbol
// is a member of with the bindings in use, or NULL if it is a member of
// neither.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPNames::EnclosingInstance(CCPPSymbol* pcSymbol)
{
	CCPPClass*	pcClass;

	pcClass = MemberScopeClass(pcSymbol);
	return (pcClass && (pcClass->mpcTemplate || pcClass->mpcEnclosing)) ? pcClass : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a member's symbol is declared in, where the member is named in
// its class but not in a member function (sizeof(t(0)) in an enumerator):
// for a class template, its instance in the bindings in use.  NULL if the
// symbol isn't a member, or its class template's instance isn't in use.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPNames::MemberScopeClass(CCPPSymbol* pcSymbol)
{
	CCPPScope*				pcScope;
	CCPPTemplateArguments*	pcBindings;

	pcScope = pcSymbol ? pcSymbol->mpcScope : NULL;
	if (!pcScope || (pcScope->meKind != CPPSC_Class) || !pcScope->mpcOwner)
	{
		return NULL;
	}
	for (pcBindings = mpcContext->GetBindings(); pcBindings; pcBindings = pcBindings->mpcOuter)
	{
		if (pcBindings->mpcClass && (pcBindings->mpcClass->mpcSymbol == pcScope->mpcOwner))
		{
			return pcBindings->mpcClass;
		}
	}
	return mpcUnit->GetInstantiator()->GetTemplate(pcScope->mpcOwner) ? NULL : mpcUnit->GetClasses()->GetClass(pcScope->mpcOwner, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// The class a qualified member name (object.Qualifier::f) names: pcClass or one of its bases.  A class
// template's name without arguments is pcClass's base instance of it.  A qualifier the context doesn't
// find is looked up in the object's class: p->Base::v, Base a member typedef.  [basic.lookup.classref] 4
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPNames::QualifierClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
{
	CCPPClass*	pcQualifier;
	CCPPSymbol*	pcSymbol;
	int64		lliName;

	pcSymbol = NULL;
	if (pcComponent && !pcComponent->mpcSymbol && (pcComponent->meKind == CPPNC_Identifier) && pcClass->mpcSymbol && pcClass->mpcSymbol->mpcMembers)
	{
		lliName = mpcUnit->GetComponentName(pcComponent);
		pcSymbol = (lliName != -1) ? pcClass->mpcSymbol->mpcMembers->FindMember(lliName, CPPL_Type) : NULL;
	}
	pcQualifier = GetComponentClass(pcComponent, pcSymbol, pcNode, pcClass);
	if (!pcQualifier || !mpcUnit->GetClasses()->IsDerived(pcClass, pcQualifier, NULL))
	{
		mpcDiagnostics->Error(pcNode, "The qualifier is not the object's class or one of its bases");
		return NULL;
	}
	return pcQualifier;
}


//////////////////////////////////////////////////////////////////////////
//
// The member of the kind eKind Class::Name names, where Class is a class, an instance of a class template, a typedef of
// one or a template parameter, and in ppsClass the class that declares it: a member of an instance can depend on its
// template arguments, so it is worked out with that class's bindings.  NULL if it isn't one.  The class is the
// qualifiers' (NameQualifierClass): TEMPLATE::SizePolicy::NotFound and W<int>::Type::N.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPNames::QualifiedMember(CCPPSyntaxName* pcName, ECPPSymbolKind eKind, CCPPSyntax* pcNode, CCPPClass** ppcClass)
{
	CCPPSyntaxNameComponent*	pcQualifier;
	CCPPClass*					pcClass;
	CCPPSymbol*					pcSymbol;
	bool						bAmbiguous;

	if (pcName->macpComponents.NumElements() < 2)
	{
		return NULL;
	}
	pcQualifier = pcName->GetQualifier();
	if (!pcQualifier)
	{
		return NULL;
	}
	if (pcQualifier->mpcSymbol)
	{
		switch (pcQualifier->mpcSymbol->meKind)
		{
		case CPPSK_Class:
		case CPPSK_ClassTemplate:
		case CPPSK_Typedef:
		case CPPSK_TypeParameter:
			break;
		default:
			return NULL;
		}
	}
	else if ((pcName->macpComponents.NumElements() < 3) && (pcQualifier->meKind != CPPNC_Decltype))
	{
		return NULL;
	}

	mpcDiagnostics->BeginSilent();
	pcClass = NameQualifierClass(pcName, pcNode);
	pcSymbol = (pcClass && mpcUnit->GetClasses()->CompleteClass(pcClass, pcNode)) ? pcClass->FindMemberSymbol(mpcUnit->GetComponentName(pcName->GetLast()), eKind, ppcClass) : NULL;
	bAmbiguous = pcSymbol && (pcClass->macBases.NumElements() > 1) && IsAmbiguousInBases(pcClass, mpcUnit->GetComponentName(pcName->GetLast()));
	mpcDiagnostics->EndSilent();
	if (bAmbiguous)
	{
		//C::T, where two of C's bases each have a T.  [class.member.lookup] 6
		mpcDiagnostics->ReportOnce(pcNode, "The name is a member of more than one of the class's bases");
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// The enumerator Class::Name names.  See QualifiedMember.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPNames::QualifiedEnumerator(CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPClass** ppcClass)
{
	CCPPSymbol*	pcSymbol;

	pcSymbol = QualifiedMember(pcName, CPPSK_Enumerator, pcNode, ppcClass);
	if (!pcSymbol)
	{
		*ppcClass = NULL;
		pcSymbol = DecltypeEnumerator(pcName);
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// decltype(e)::name where e is of an enumeration: the enumeration's
// enumerator of the name, or NULL.  [dcl.type.simple] 4, [dcl.enum] 11
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPNames::DecltypeEnumerator(CCPPSyntaxName* pcName)
{
	SCPP65Type*	psType;
	CCPPSymbol*	pcEnum;
	CCPPSymbol*	pcSymbol;

	if (!pcName->IsQualifiedByDecltype() || (pcName->macpComponents.NumElements() != 2))
	{
		return NULL;
	}
	mpcDiagnostics->BeginSilent();
	psType = mpcDeclarators->GetTypeIdType(((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0))->mpcType);
	mpcDiagnostics->EndSilent();
	pcEnum = psType ? psType->WithoutReference()->pcEnum : NULL;
	if (!pcEnum)
	{
		return NULL;
	}

	//A scoped enumeration's enumerators are its members; one that isn't scoped has them in the scope it is in too.
	pcSymbol = pcEnum->mpcMembers ? pcEnum->mpcMembers->FindHere(mpcUnit->GetComponentName(pcName->GetLast())) : NULL;
	pcSymbol = pcSymbol ? pcSymbol : (pcEnum->mpcScope ? pcEnum->mpcScope->FindHere(mpcUnit->GetComponentName(pcName->GetLast())) : NULL);
	psType = (pcSymbol && (pcSymbol->meKind == CPPSK_Enumerator)) ? mpcUnit->GetEnums()->EnumeratorType(pcSymbol) : NULL;
	return (psType && (psType->pcEnum == pcEnum)) ? pcSymbol : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a name that isn't a function's is ambiguous: lookup finds two
// different things where it finds it (CCPPScope::IsAmbiguous, or for a
// name qualified by a namespace IsAmbiguousMember).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPNames::IsAmbiguousName(CCPPSyntaxName* pcName)
{
	CCPPSyntaxNameComponent*	pcQualifier;
	int64						lliName;

	if (!pcName->mpcSymbol || pcName->mpcSymbol->IsFunction() || (pcName->GetLast()->meKind != CPPNC_Identifier))
	{
		return false;
	}
	lliName = pcName->mpcSymbol->mlliName;
	pcQualifier = pcName->GetQualifier();
	if (pcQualifier)
	{
		return pcQualifier->mpcSymbol && (pcQualifier->mpcSymbol->meKind == CPPSK_Namespace) && pcQualifier->mpcSymbol->mpcMembers && pcQualifier->mpcSymbol->mpcMembers->IsAmbiguousMember(lliName);
	}
	return !pcName->mbGlobal && pcName->mpcLookup && pcName->mpcLookup->IsAmbiguous(lliName);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the class or a base has a member function of the name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPNames::HasMember(CCPPClass* pcClass, int64 lliName)
{
	CCPPFunction*	pcFunction;
	size			i;

	if (!mpcUnit->GetClasses()->CompleteClass(pcClass, NULL))
	{
		return false;
	}
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->mpcClass == pcClass) && (pcFunction->meKind == CPPFK_Function))
		{
			return true;
		}
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		if (HasMember(pcClass->macBases.Get(i)->pcClass, lliName))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcClass (or a base) has an operator function named lliName: a member function, or a member template taking
// uiArguments.  The class is laid out to find out, if it hasn't been: its members and its using-declarations are
// known only then.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPNames::HasOperator(CCPPClass* pcClass, int64 lliName, size uiArguments)
{
	size	i;

	if (lliName == mpcUnit->GetOperatorName(CT_Assignment, false))
	{
		//A base's operator= is hidden by the copy assignment operator every class declares: only the class's own
		//are, and those a using-declaration in it names (using Base::operator=;).  [over.ass] 1, [namespace.udecl] 4
		if (!mpcUnit->GetClasses()->CompleteClass(pcClass, NULL))
		{
			return false;
		}
		for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
		{
			if (pcClass->mapcMembers.GetPtr(i)->mlliName == lliName)
			{
				return true;
			}
		}
		for (i = 0; i < pcClass->macUsings.NumElements(); i++)
		{
			if (pcClass->macUsings.Get(i)->lliName == lliName)
			{
				return true;
			}
		}
		return mpcBinder->HasOwnMemberTemplate(pcClass, lliName, uiArguments);
	}
	return HasMember(pcClass, lliName) || mpcBinder->HasMemberTemplate(pcClass, lliName, uiArguments);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a name looked up in a class is found as different things in
// its bases, neither hidden by one of the class's own (or of a class
// nearer than both): struct C : A, B { } where A and B each have a T, or
// are two instances of the template that has it.  The same class's member
// reached through two bases is one thing, and one in a virtual base of
// another class that has the name is hidden by that class's (there is one
// of the base).  [class.member.lookup] 3 - 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPNames::IsAmbiguousInBases(CCPPClass* pcClass, int64 lliName)
{
	CArrayCPPClasses	apcFound;
	CCPPClass*			pcFound;
	size				uiMeanings;
	size				i;
	size				j;
	bool				bHidden;

	mpcUnit->GetStatistics()->Count(CPPCC_BaseWalks);
	if (pcClass->macBases.NumElements() == 0)
	{
		return false;
	}
	apcFound.Init();
	FindInBases(pcClass, lliName, &apcFound);
	uiMeanings = 0;
	for (i = 0; i < apcFound.NumElements(); i++)
	{
		pcFound = apcFound.GetPtr(i);
		bHidden = false;
		for (j = 0; !bHidden && (j < apcFound.NumElements()); j++)
		{
			bHidden = (j != i) && apcFound.GetPtr(j)->GetVirtualBase(pcFound);
		}
		uiMeanings += bHidden ? 0 : 1;
	}
	apcFound.Kill();
	return uiMeanings > 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The classes whose declaration of a name lookup in a class finds: the
// class itself, whose own hides its bases', or else each base's.
//
//////////////////////////////////////////////////////////////////////////
void CCPPNames::FindInBases(CCPPClass* pcClass, int64 lliName, CArrayCPPClasses* papcFound)
{
	CCPPScope*	pcScope;
	size		i;

	mpcUnit->GetStatistics()->Count(CPPCC_BaseWalks);
	pcScope = pcClass->mpcSpecifier ? pcClass->mpcSpecifier->mpcScope : ((pcClass->mpcSymbol) ? pcClass->mpcSymbol->mpcMembers : NULL);
	if (pcScope && pcScope->FindHere(lliName))
	{
		if (papcFound->Find(pcClass) == ARRAY_ELEMENT_NOT_FOUND)
		{
			papcFound->Add(pcClass);
		}
		return;
	}
	if (!mpcUnit->GetClasses()->ResolveBases(pcClass, NULL))
	{
		return;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		FindInBases(pcClass->macBases.Get(i)->pcClass, lliName, papcFound);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A name not qualified, in a member function, that the parser found as a
// member of a base of the function's class (a type, a static member, an
// enumerator): an error if more than one of the class's bases has it, as
// lookup in the class then finds more than one.  Fields and functions are
// checked where they are bound (CCPPBinder::HasOneMeaning).
// [class.member.lookup] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPNames::CheckBaseMemberName(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSymbol*		pcSymbol;
	CCPPScope*		pcScope;
	CCPPFunction*	pcFunction;
	CCPPClass*		pcClass;

	pcSymbol = pcName->mpcSymbol;
	pcFunction = mpcContext->GetFunction();
	pcClass = pcFunction ? pcFunction->mpcClass : NULL;
	if (!pcSymbol || !pcClass || (pcName->macpComponents.NumElements() != 1) || !pcSymbol->mpcScope || (pcClass->macBases.NumElements() < 2))
	{
		return true;
	}
	pcScope = pcSymbol->mpcScope;
	if ((pcScope->meKind == CPPSC_Enum) && pcScope->mpcParent)
	{
		//An unscoped enum's enumerator is its class's member.
		pcScope = pcScope->mpcParent;
	}
	if ((pcScope->meKind != CPPSC_Class) || !pcScope->mpcOwner || (pcScope->mpcOwner->meKind != CPPSK_Class) || (pcClass->mpcSymbol == pcScope->mpcOwner))
	{
		return true;
	}
	return !IsAmbiguousInBases(pcClass, pcSymbol->mlliName) || mpcDiagnostics->Error(pcNode, "The name is a member of more than one of the class's bases");
}
