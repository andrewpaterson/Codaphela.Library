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
#include <string.h>
#include "CPPUnit.h"
#include "CPPAccess.h"


//A member's access as a member of a class: the declared ones (ECPPAccess), and what a private member of a base is in
//the classes derived from it.  CPPA_None is not a member of the class at all.  They are in order: a smaller one lets
//more name the member.
#define CPPA_INACCESSIBLE	(CPPA_Private + 1)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPAccess::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mpcDiagnostics = pcUnit->GetDiagnostics();
}


//////////////////////////////////////////////////////////////////////////
//
// A function chosen for a call, an operator or a construction: an error
// (and false) if it is a member that may not be named at pcNode.  A
// function the compiler wrote, or inherited, has no declaration and is
// public.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::CheckFunction(CCPPFunction* pcFunction, CCPPClass* pcNaming, CCPPSyntax* pcNode)
{
	return !pcFunction || !pcFunction->mpcClass || Check(pcFunction->mpcDeclarator, pcNaming, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// A field, a static member, an enumerator or a type, by its symbol.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::CheckSymbol(CCPPSymbol* pcSymbol, CCPPClass* pcNaming, CCPPSyntax* pcNode)
{
	CCPPScope*	pcScope;

	//Most names are no class's: their scopes say so at once.  An enumerator's class is the one its enum is in.
	pcScope = pcSymbol ? pcSymbol->mpcScope : NULL;
	if (pcScope && (pcScope->meKind == CPPSC_Enum))
	{
		pcScope = pcScope->mpcParent;
	}
	return !pcScope || (pcScope->meKind != CPPSC_Class) || Check(pcSymbol->mpcDeclaration, pcNaming, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// What pcDeclaration declares, named at pcNode as a member of pcNaming:
// an error (and false) if it may not be.  What isn't a member of a class
// may always be named.  With no naming class, or one the member isn't
// reached from, it is named as a member of the class that declares it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::Check(CCPPSyntax* pcDeclaration, CCPPClass* pcNaming, CCPPSyntax* pcNode)
{
	CCPPSyntaxClassSpecifier*	pcDeclaring;
	CCPPSyntax*					pcUsing;
	int							iDeclared;
	bool						bAccessible;

	iDeclared = DeclaredAccess(pcDeclaration, &pcDeclaring);
	if (iDeclared == CPPA_None)
	{
		return true;
	}

	//A using-declaration of the member in the class it is named in (or a class between) gives it its own access.
	pcUsing = pcNaming ? FindUsingDeclaration(pcNaming, pcDeclaration, pcDeclaring) : NULL;
	if (pcUsing)
	{
		iDeclared = DeclaredAccess(pcUsing, &pcDeclaring);
	}

	if (pcNaming && pcNaming->mpcSpecifier && (AccessAs(pcNaming, pcDeclaring, iDeclared) != CPPA_None))
	{
		bAccessible = IsAccessible(pcNaming, pcDeclaring, iDeclared, pcNode);
	}
	else
	{
		bAccessible = IsAllowed(iDeclared, pcDeclaring, pcNode);
	}
	if (bAccessible)
	{
		return true;
	}
	if (iDeclared == CPPA_Public)
	{
		return mpcDiagnostics->ReportOnce(pcNode, "The member is in a base class that can't be reached from here");
	}
	return mpcDiagnostics->ReportOnce(pcNode, (iDeclared == CPPA_Private) ? "The member is private" : "The member is protected");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether every field of the class is public: one of what makes it an
// aggregate, which a list in braces fills field by field.  [dcl.init.aggr] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::HasOnlyPublicFields(CCPPClass* pcClass)
{
	CCPPSyntaxClassSpecifier*	pcDeclaring;
	SCPPField*					psField;
	size						i;

	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		if (psField->pcSymbol && psField->pcSymbol->mpcDeclaration && (DeclaredAccess(psField->pcSymbol->mpcDeclaration, &pcDeclaring) > CPPA_Public))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a pcClass may be used as its base pcBase at pcNode (a pointer or
// a reference to the one made one to the other): an error (and false) if
// not.  The base is reached where a public member of it could be named as
// a member of the class.  [class.access.base] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::CheckBase(CCPPClass* pcClass, CCPPClass* pcBase, CCPPSyntax* pcNode)
{
	if ((pcClass == pcBase) || !pcClass->mpcSpecifier || !pcBase->mpcSpecifier)
	{
		return true;
	}
	return IsAccessible(pcClass, pcBase->mpcSpecifier, CPPA_Public, pcNode) || mpcDiagnostics->ReportOnce(pcNode, "The base class is private or protected: it can't be reached from here");
}


//////////////////////////////////////////////////////////////////////////
//
// The access a declaration gives what it declares, and the class it is a
// member of (*ppcClass): CPPA_None if it isn't a class's member.  The
// member is the declaration, or what it is in, among the class's members;
// its access is the last access specifier before it, or the class's own.
// What is declared in a function's body or parameters is no member.
//
//////////////////////////////////////////////////////////////////////////
int CCPPAccess::DeclaredAccess(CCPPSyntax* pcDeclaration, CCPPSyntaxClassSpecifier** ppcClass)
{
	CCPPSyntax*					pcMember;
	CCPPSyntax*					pcOther;
	CCPPSyntaxClassSpecifier*	pcClass;
	int							iAccess;
	size						i;

	*ppcClass = NULL;
	for (pcMember = pcDeclaration; pcMember && pcMember->mpcParent && (pcMember->mpcParent->GetKind() != CPPS_ClassSpecifier); pcMember = pcMember->mpcParent)
	{
		if ((pcMember->GetKind() == CPPS_CompoundStatement) || (pcMember->GetKind() == CPPS_Parameter))
		{
			return CPPA_None;
		}
	}
	if (!pcMember || !pcMember->mpcParent)
	{
		return CPPA_None;
	}

	pcClass = (CCPPSyntaxClassSpecifier*)pcMember->mpcParent;
	iAccess = (pcClass->meKey == CT_class) ? CPPA_Private : CPPA_Public;
	for (i = 0; i < pcClass->macpMembers.NumElements(); i++)
	{
		pcOther = pcClass->macpMembers.GetPtr(i);
		if (pcOther == pcMember)
		{
			*ppcClass = pcClass;
			return iAccess;
		}
		if (pcOther->GetKind() == CPPS_AccessSpecifier)
		{
			iAccess = ((CCPPSyntaxAccessSpecifier*)pcOther)->meAccess;
		}
	}

	//A base of the class, or an attribute: not one of its members.
	return CPPA_None;
}


//////////////////////////////////////////////////////////////////////////
//
// The using-declaration of the member pcDeclaration declares, in pcClass or
// in a base of it on the way to pcDeclaring (the member's class), nearest
// first, or NULL: struct D : B { public: using B::p; } makes B's protected p
// public in D.  [namespace.udecl] 19
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPAccess::FindUsingDeclaration(CCPPClass* pcClass, CCPPSyntax* pcDeclaration, CCPPSyntaxClassSpecifier* pcDeclaring)
{
	CCPPSyntaxDeclarator*	pcDeclarator;
	CCPPSyntaxName*			pcName;
	CCPPSyntax*				pcMember;
	CCPPSyntax*				pcFound;
	int64					lliName;
	size					i;

	if (!pcClass->mpcSpecifier || (pcClass->mpcSpecifier == pcDeclaring))
	{
		return NULL;
	}
	pcDeclarator = NULL;
	if (pcDeclaration->GetKind() == CPPS_Declarator)
	{
		pcDeclarator = (CCPPSyntaxDeclarator*)pcDeclaration;
	}
	else if (pcDeclaration->GetKind() == CPPS_InitDeclarator)
	{
		pcDeclarator = ((CCPPSyntaxInitDeclarator*)pcDeclaration)->mpcDeclarator;
	}
	pcName = pcDeclarator ? pcDeclarator->GetName() : NULL;
	if (!pcName || !pcName->GetLast())
	{
		return NULL;
	}
	lliName = mpcUnit->GetComponentName(pcName->GetLast());
	for (i = 0; i < pcClass->mpcSpecifier->macpMembers.NumElements(); i++)
	{
		pcMember = pcClass->mpcSpecifier->macpMembers.GetPtr(i);
		pcName = (pcMember->GetKind() == CPPS_UsingDeclaration) ? ((CCPPSyntaxUsingDeclaration*)pcMember)->mpcName : NULL;
		if (pcName && pcName->GetLast() && (mpcUnit->GetComponentName(pcName->GetLast()) == lliName))
		{
			return pcMember;
		}
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcFound = FindUsingDeclaration(pcClass->macBases.Get(i)->pcClass, pcDeclaration, pcDeclaring);
		if (pcFound)
		{
			return pcFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What a member of pcDeclaring, declared iDeclared, is as a member of
// pcClass: itself in the class that declares it; through a base, what it
// is in the base made no more open than the base (a public member through
// a private base is private), and a base's private member is inaccessible.
// The most open of the ways there.  CPPA_None if pcDeclaring is neither
// the class nor a base of it.  [class.access.base] 1
//
//////////////////////////////////////////////////////////////////////////
int CCPPAccess::AccessAs(CCPPClass* pcClass, CCPPSyntaxClassSpecifier* pcDeclaring, int iDeclared)
{
	SCPPBase*	psBase;
	int			iAccess;
	int			iBest;
	size		i;

	if (pcClass->mpcSpecifier == pcDeclaring)
	{
		return iDeclared;
	}
	iBest = CPPA_None;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		iAccess = AccessAs(psBase->pcClass, pcDeclaring, iDeclared);
		if (iAccess == CPPA_None)
		{
			continue;
		}
		if (iAccess >= CPPA_Private)
		{
			iAccess = CPPA_INACCESSIBLE;
		}
		else if (psBase->eAccess > iAccess)
		{
			iAccess = psBase->eAccess;
		}
		if ((iBest == CPPA_None) || (iAccess < iBest))
		{
			iBest = iAccess;
		}
	}
	return iBest;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the member may be named at pcNode as a member of pcClass: by
// what it is as a member of the class, or as a member of a base of the
// class that the use may reach (a public base; any base for the class's
// own members and friends; a protected one for those of classes derived
// from it).  [class.access.base] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::IsAccessible(CCPPClass* pcClass, CCPPSyntaxClassSpecifier* pcDeclaring, int iDeclared, CCPPSyntax* pcNode)
{
	SCPPBase*	psBase;
	int			iAccess;
	bool		bReached;
	size		i;

	iAccess = AccessAs(pcClass, pcDeclaring, iDeclared);
	if (iAccess == CPPA_None)
	{
		return false;
	}
	if ((iAccess != CPPA_INACCESSIBLE) && IsAllowed(iAccess, pcClass->mpcSpecifier, pcNode))
	{
		return true;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		bReached = (psBase->eAccess == CPPA_Public) || IsInClass(pcClass->mpcSpecifier, pcNode) || ((psBase->eAccess == CPPA_Protected) && IsInDerivedFrom(pcClass->mpcSpecifier));
		if (bReached && IsAccessible(psBase->pcClass, pcDeclaring, iDeclared, pcNode))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a member that is iAccess as a member of pcClass may be named at
// pcNode.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::IsAllowed(int iAccess, CCPPSyntaxClassSpecifier* pcClass, CCPPSyntax* pcNode)
{
	switch (iAccess)
	{
	case CPPA_Public:
		return true;
	case CPPA_Protected:
		return IsInClass(pcClass, pcNode) || IsInDerivedFrom(pcClass);
	case CPPA_Private:
		return IsInClass(pcClass, pcNode);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcNode is in a member or a friend of the class.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::IsInClass(CCPPSyntaxClassSpecifier* pcClass, CCPPSyntax* pcNode)
{
	return !pcClass || IsInMemberOf(pcClass->mpcSymbol, pcNode) || IsInFriendOf(pcClass, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcNode is in a member of the class whose symbol is pcClass: it
// is written in the class's definition (a member's body or initialiser,
// or a class nested in it, whose members are the class's too), or the
// function being written is a member function of the class or of a class
// in it, defined outside.  [class.access.nest]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::IsInMemberOf(CCPPSymbol* pcClass, CCPPSyntax* pcNode)
{
	CCPPSyntax*					pcParent;
	CCPPFunction*				pcFunction;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcQualifier;

	if (!pcClass)
	{
		return false;
	}
	for (pcParent = pcNode; pcParent; pcParent = pcParent->mpcParent)
	{
		if ((pcParent->GetKind() == CPPS_ClassSpecifier) && (((CCPPSyntaxClassSpecifier*)pcParent)->mpcSymbol == pcClass))
		{
			return true;
		}
		if (pcParent->GetKind() == CPPS_InitDeclarator)
		{
			//int C::s = Seed(); a static member's definition is in its class's scope, its initialiser too.  [class.access] 7
			pcName = ((CCPPSyntaxInitDeclarator*)pcParent)->mpcDeclarator ? ((CCPPSyntaxInitDeclarator*)pcParent)->mpcDeclarator->GetName() : NULL;
			pcQualifier = (pcName && pcName->IsQualifiedByClass()) ? pcName->GetQualifier() : NULL;
			if (pcQualifier && pcQualifier->mpcSymbol && (pcQualifier->mpcSymbol == pcClass))
			{
				return true;
			}
		}
	}
	pcFunction = mpcUnit->GetClosures()->OuterFunction();
	for (pcParent = (pcFunction && pcFunction->mpcClass) ? pcFunction->mpcClass->mpcSpecifier : NULL; pcParent; pcParent = pcParent->mpcParent)
	{
		if ((pcParent->GetKind() == CPPS_ClassSpecifier) && (((CCPPSyntaxClassSpecifier*)pcParent)->mpcSymbol == pcClass))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcNode is in a friend of the class: the function being written
// is one a friend declaration among the class's members names, or pcNode
// is in a member of a class one names.  A friend function is known by its
// name alone, so every function of the name is the class's friend: more
// than the language allows.  [class.friend]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::IsInFriendOf(CCPPSyntaxClassSpecifier* pcClass, CCPPSyntax* pcNode)
{
	CCPPSyntax*						pcMember;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxTypeSpecifier*		pcType;
	CCPPSyntaxName*					pcName;
	size							i;
	size							j;

	for (i = 0; i < pcClass->macpMembers.NumElements(); i++)
	{
		pcMember = pcClass->macpMembers.GetPtr(i);
		while (pcMember && (pcMember->GetKind() == CPPS_TemplateDeclaration))
		{
			pcMember = ((CCPPSyntaxTemplateDeclaration*)pcMember)->mpcDeclaration;
		}
		if (pcMember && (pcMember->GetKind() == CPPS_FunctionDefinition))
		{
			pcDefinition = (CCPPSyntaxFunctionDefinition*)pcMember;
			if (pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & CPPDS_FRIEND) && IsFriendFunction(pcDefinition->mpcDeclarator))
			{
				return true;
			}
		}
		if (!pcMember || (pcMember->GetKind() != CPPS_SimpleDeclaration))
		{
			continue;
		}
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcMember;
		if (!pcDeclaration->mpcSpecifiers || !(pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_FRIEND))
		{
			continue;
		}
		for (j = 0; j < pcDeclaration->macpDeclarators.NumElements(); j++)
		{
			if ((pcDeclaration->macpDeclarators.GetPtr(j)->GetKind() == CPPS_InitDeclarator) && IsFriendFunction(((CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(j))->mpcDeclarator))
			{
				return true;
			}
		}

		//friend class X; and friend X;
		pcType = pcDeclaration->mpcSpecifiers->mpcType;
		pcName = NULL;
		if (pcType && (pcType->GetKind() == CPPS_ElaboratedTypeSpecifier))
		{
			pcName = ((CCPPSyntaxElaboratedTypeSpecifier*)pcType)->mpcName;
		}
		else if (pcType && (pcType->GetKind() == CPPS_NamedTypeSpecifier))
		{
			pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
		}
		if (pcName && IsInMemberOf(pcName->mpcSymbol, pcNode))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function being written has the name a friend declaration's
// declarator declares.  friend int Only<int>(int) is one specialisation of
// the template: the function must be the instance for those arguments.
// [temp.friend] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::IsFriendFunction(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPFunction*				pcFunction;
	CCPPSyntaxNameComponent*	pcComponent;
	CChars						szArguments;
	int64						lliName;
	bool						bSame;

	pcFunction = mpcUnit->GetClosures()->OuterFunction();
	if (!pcFunction || !pcDeclarator || !pcDeclarator->GetName())
	{
		return false;
	}
	pcComponent = pcDeclarator->GetName()->GetLast();
	lliName = mpcUnit->GetFunctionName(pcComponent);
	if ((lliName == -1) || (lliName != pcFunction->mlliName))
	{
		return false;
	}
	if (!pcComponent->mbTemplateArguments || (pcComponent->macpTemplateArguments.NumElements() == 0) || !pcFunction->mpcTemplate || !pcFunction->mpcTemplate->mpcDeclaration)
	{
		return true;
	}
	szArguments.Init();
	mpcDiagnostics->BeginQuiet();
	bSame = mpcUnit->GetInstantiator()->BindArguments(pcFunction->mpcTemplate->mpcDeclaration, &pcComponent->macpTemplateArguments, NULL, &szArguments, pcDeclarator, NULL) && szArguments.Equals(&pcFunction->mszArguments);
	mpcDiagnostics->EndQuiet();
	szArguments.Kill();
	return bSame;
}


//////////////////////////////////////////////////////////////////////////
//
// A using-declaration in a class (pcUsing) naming a member of a base
// (pcSymbol): the member isn't private there, but where the class is that
// base's friend.  An error (and false) if it is.  [namespace.udecl] 17
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::CheckUsing(CCPPSymbol* pcSymbol, CCPPSyntax* pcUsing)
{
	CCPPSyntaxClassSpecifier*	pcDeclaring;

	if (!pcSymbol || (DeclaredAccess(pcSymbol->mpcDeclaration, &pcDeclaring) != CPPA_Private) || IsInClass(pcDeclaring, pcUsing))
	{
		return true;
	}
	return mpcDiagnostics->ReportOnce(pcUsing, "The using-declaration names a base's private member, which the class can't reach");
}


//////////////////////////////////////////////////////////////////////////
//
// A member reached through an object of pcObjectClass (o.m, p->m), or
// named for a pointer to member (&C::m, C being pcObjectClass), which
// CheckSymbol allowed: if it is a base's protected member that isn't
// static, the object is of the class whose member or friend pcNode is in,
// or of one derived from it (IsThroughDerived).  An error (and false) if
// not.  [class.protected] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::CheckProtectedObject(CCPPSymbol* pcSymbol, CCPPClass* pcObjectClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxClassSpecifier*	pcDeclaring;
	CCPPSyntax*					pcUsing;

	if (!pcSymbol || !pcObjectClass || (DeclaredAccess(pcSymbol->mpcDeclaration, &pcDeclaring) != CPPA_Protected))
	{
		return true;
	}
	pcUsing = FindUsingDeclaration(pcObjectClass, pcSymbol->mpcDeclaration, pcDeclaring);
	if ((pcUsing && (DeclaredAccess(pcUsing, &pcDeclaring) == CPPA_Public)) || IsThroughDerived(pcSymbol->mpcDeclaration, pcDeclaring, pcObjectClass, pcNode))
	{
		return true;
	}
	return mpcDiagnostics->ReportOnce(pcNode, "The member is protected: a class derived from its class reaches it only through an object of its own class (or one derived from it)");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a protected member that isn't static, of pcDeclaring, may be
// reached through an object of pcNaming at pcNode: in pcDeclaring's own
// members and friends it may, and in pcNaming's (the object is of their
// class); in a member of another class derived from pcDeclaring, only
// through that class or one derived from it.  A constructor or destructor,
// a static member, a type and an enumerator may be reached through any.
// [class.protected] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::IsThroughDerived(CCPPSyntax* pcDeclaration, CCPPSyntaxClassSpecifier* pcDeclaring, CCPPClass* pcNaming, CCPPSyntax* pcNode)
{
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSyntax*					pcParent;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPFunction*				pcFunction;

	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator))
	{
		pcDeclaration = ((CCPPSyntaxInitDeclarator*)pcDeclaration)->mpcDeclarator;
	}
	if (!pcDeclaration || (pcDeclaration->GetKind() != CPPS_Declarator) || IsInClass(pcDeclaring, pcNode) || (pcNaming->mpcSpecifier && IsInClass(pcNaming->mpcSpecifier, pcNode)))
	{
		return true;
	}
	pcDeclarator = (CCPPSyntaxDeclarator*)pcDeclaration;
	pcName = pcDeclarator->GetName();
	pcComponent = pcName ? pcName->GetLast() : NULL;
	if (!pcComponent || (pcComponent->meKind == CPPNC_Destructor) || (pcDeclaring->mpcSymbol && (pcComponent->meKind == CPPNC_Identifier) && (mpcUnit->GetComponentName(pcComponent) == pcDeclaring->mpcSymbol->mlliName)))
	{
		return true;
	}
	pcParent = pcDeclarator->mpcParent;
	if (pcParent && (pcParent->GetKind() == CPPS_InitDeclarator))
	{
		pcParent = pcParent->mpcParent;
	}
	pcSpecifiers = NULL;
	if (pcParent && (pcParent->GetKind() == CPPS_SimpleDeclaration))
	{
		pcSpecifiers = ((CCPPSyntaxSimpleDeclaration*)pcParent)->mpcSpecifiers;
	}
	else if (pcParent && (pcParent->GetKind() == CPPS_FunctionDefinition))
	{
		pcSpecifiers = ((CCPPSyntaxFunctionDefinition*)pcParent)->mpcSpecifiers;
	}
	if (pcSpecifiers && (pcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_TYPEDEF | CPPDS_FRIEND)))
	{
		return true;
	}
	pcFunction = mpcUnit->GetClosures()->OuterFunction();
	if (!pcFunction || !pcFunction->mpcClass || !pcFunction->mpcClass->mpcSpecifier)
	{
		return true;
	}
	return (pcNaming == pcFunction->mpcClass) || HasBase(pcNaming, pcFunction->mpcClass->mpcSpecifier);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function being written is a member of a class derived from
// the class: its members may name the class's protected ones.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::IsInDerivedFrom(CCPPSyntaxClassSpecifier* pcClass)
{
	CCPPFunction*	pcFunction;

	pcFunction = mpcUnit->GetClosures()->OuterFunction();
	return pcFunction && pcFunction->mpcClass && HasBase(pcFunction->mpcClass, pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcBase is the definition of a base of pcClass, however far up.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAccess::HasBase(CCPPClass* pcClass, CCPPSyntaxClassSpecifier* pcBase)
{
	size	i;

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		if ((pcClass->macBases.Get(i)->pcClass->mpcSpecifier == pcBase) || HasBase(pcClass->macBases.Get(i)->pcClass, pcBase))
		{
			return true;
		}
	}
	return false;
}

