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
#include "CPPScope.h"


#define MAX_LOOKUP_DEPTH	32


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPScope::Init(ECPPScopeKind eKind, CCPPScope* pcParent, CCPPSymbol* pcOwner, CMallocator* pcMalloc)
{
	meKind = eKind;
	mpcParent = pcParent;
	mpcOwner = pcOwner;
	if (pcMalloc)
	{
		mcSymbols.Init(pcMalloc, true);
		mapcBases.Init(pcMalloc);
		mapcUsings.Init(pcMalloc);
		masUsedFunctions.Init(pcMalloc);
	}
	else
	{
		mcSymbols.Init(true);
		mapcBases.Init();
		mapcUsings.Init();
		masUsedFunctions.Init();
	}
	mpcQualifier = NULL;
	mbDependent = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPScope::Kill(void)
{
	mapcUsings.Kill();
	masUsedFunctions.Kill();
	mapcBases.Kill();
	mcSymbols.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPScope::Add(CCPPSymbol* pcSymbol)
{
	CCPPSymbol**			ppcExisting;
	SCPPScopeUsedFunction*	psUsed;

	//Later declarations of a name chain after the first, so lookup finds the first.  A header's lazy declarations of
	//the name are parsed first, so they stay before this one.
	ppcExisting = mcSymbols.Get(pcSymbol->mlliName);
	if (ppcExisting && *ppcExisting && !pcSymbol->mpcLazy)
	{
		LoadLazy(pcSymbol->mlliName);
		ppcExisting = mcSymbols.Get(pcSymbol->mlliName);
	}
	if (ppcExisting && *ppcExisting && ((*ppcExisting)->mpcScope != this) && (*ppcExisting)->IsFunction() && pcSymbol->IsFunction() && ((*ppcExisting)->mpcScope->meKind == CPPSC_Namespace))
	{
		//A function declared after using N::F; of its name.  The symbol found so far is N's, whose chain isn't this scope's.
		psUsed = masUsedFunctions.Add();
		psUsed->lliName = pcSymbol->mlliName;
		psUsed->pcNamespace = (*ppcExisting)->mpcScope;
		mcSymbols.Put(pcSymbol->mlliName, &pcSymbol);
	}
	else if (ppcExisting && *ppcExisting)
	{
		pcSymbol->mpcNext = (*ppcExisting)->mpcNext;
		(*ppcExisting)->mpcNext = pcSymbol;
	}
	else
	{
		mcSymbols.Put(pcSymbol->mlliName, &pcSymbol);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPScope::AddAlias(int64 lliName, CCPPSymbol* pcSymbol, int iBefore)
{
	//The name finds a symbol declared somewhere else, as a using-declaration does (its first token is iBefore, -1 for
	//none).  The symbol is not changed.  A header's lazy declarations of the name are parsed first.
	CCPPSymbol**			ppcExisting;
	SCPPScopeUsedFunction*	psUsed;

	LoadLazy(lliName);
	ppcExisting = mcSymbols.Get(lliName);
	if (!ppcExisting || !*ppcExisting)
	{
		mcSymbols.Put(lliName, &pcSymbol);
	}
	if (pcSymbol->IsFunction() && pcSymbol->mpcScope && (pcSymbol->mpcScope->meKind == CPPSC_Namespace) && (pcSymbol->mpcScope != this))
	{
		//The namespace's functions declared so far are overloads of those the name has here.  [namespace.udecl] 11
		psUsed = masUsedFunctions.Add();
		psUsed->lliName = lliName;
		psUsed->pcNamespace = pcSymbol->mpcScope;
		psUsed->iBefore = iBefore;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Parses the header declarations of the name that haven't been parsed yet
// (lazy symbols, CCPPParser::LoadLazy).
//
//////////////////////////////////////////////////////////////////////////
void CCPPScope::LoadLazy(int64 lliName)
{
	CCPPSymbol**	ppcSymbol;
	CCPPSymbol*		pcSymbol;

	ppcSymbol = mcSymbols.Get(lliName);
	if (!ppcSymbol)
	{
		return;
	}
	for (pcSymbol = *ppcSymbol; pcSymbol; pcSymbol = pcSymbol->mpcNext)
	{
		if (pcSymbol->mpcLazy)
		{
			pcSymbol->mpcLoader->LoadLazy(this, lliName);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPScope::FindHere(int64 lliName)
{
	CCPPSymbol**	ppcSymbol;

	LoadLazy(lliName);
	ppcSymbol = mcSymbols.Get(lliName);
	if (ppcSymbol)
	{
		return *ppcSymbol;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPScope::FindMember(int64 lliName)
{
	return FindMember(lliName, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPScope::FindMember(int64 lliName, int iDepth)
{
	CCPPSymbol*	pcSymbol;
	size		i;

	//The scope, then the base classes of a class or the namespaces nominated by using directives.  [class.member.lookup] [namespace.udir]
	if (iDepth > MAX_LOOKUP_DEPTH)
	{
		return NULL;
	}

	pcSymbol = FindHere(lliName);
	if (pcSymbol)
	{
		return pcSymbol;
	}

	for (i = 0; i < mapcBases.NumElements(); i++)
	{
		pcSymbol = mapcBases.GetPtr(i)->FindMember(lliName, iDepth + 1);
		if (pcSymbol)
		{
			return pcSymbol;
		}
	}

	for (i = 0; i < mapcUsings.NumElements(); i++)
	{
		pcSymbol = mapcUsings.GetPtr(i)->FindMember(lliName, iDepth + 1);
		if (pcSymbol)
		{
			return pcSymbol;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Unqualified name lookup: this scope and then the enclosing ones, each's
// own names (FindDeclared), and at each namespace the names of what
// using-directives nominate there (FindNominated): a using-directive's
// names count as declared in the nearest namespace that encloses both it
// and the namespace it nominates, so in B::g() { using namespace A; ... }
// B's names hide A's.  [basic.lookup.unqual] [namespace.udir] 2
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPScope::Find(int64 lliName)
{
	CCPPScope*	pcScope;
	CCPPSymbol*	pcSymbol;

	for (pcScope = this; pcScope; pcScope = pcScope->mpcParent)
	{
		pcSymbol = pcScope->FindDeclared(lliName, 0);
		if (!pcSymbol && (pcScope->meKind == CPPSC_Namespace))
		{
			pcSymbol = FindNominated(lliName, pcScope, NULL);
		}
		if (pcSymbol)
		{
			return pcSymbol;
		}
		if (pcScope->mpcQualifier)
		{
			pcSymbol = pcScope->mpcQualifier->Find(lliName);
			if (pcSymbol)
			{
				return pcSymbol;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The scope's own names: declared here, in its base classes, or in the
// anonymous unions in it (whose members are its own).  Not what
// using-directives nominate.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPScope::FindDeclared(int64 lliName, int iDepth)
{
	CCPPSymbol*	pcSymbol;
	CCPPScope*	pcUsing;
	size		i;

	if (iDepth > MAX_LOOKUP_DEPTH)
	{
		return NULL;
	}
	pcSymbol = FindHere(lliName);
	for (i = 0; !pcSymbol && (i < mapcBases.NumElements()); i++)
	{
		pcSymbol = mapcBases.GetPtr(i)->FindMember(lliName, iDepth + 1);
	}
	for (i = 0; !pcSymbol && (i < mapcUsings.NumElements()); i++)
	{
		pcUsing = mapcUsings.GetPtr(i);
		if (pcUsing->meKind != CPPSC_Namespace)
		{
			pcSymbol = pcUsing->FindDeclared(lliName, iDepth + 1);
		}
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// The name in a namespace nominated where pcLevel is the nearest namespace
// enclosing the using-directive and what it nominates: by a directive (or
// as an unnamed or inline namespace) of this scope or of one it is in, up
// to pcLevel.  The first found, or with papcFound all of them, gathered
// there.  [namespace.udir] 2
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPScope::FindNominated(int64 lliName, CCPPScope* pcLevel, CArrayCPPSymbols* papcFound)
{
	CCPPScope*	pcScope;
	CCPPScope*	pcUsing;
	CCPPSymbol*	pcSymbol;
	size		i;

	for (pcScope = this; pcScope; pcScope = pcScope->mpcParent)
	{
		for (i = 0; i < pcScope->mapcUsings.NumElements(); i++)
		{
			pcUsing = pcScope->mapcUsings.GetPtr(i);
			if ((pcUsing->meKind == CPPSC_Namespace) && (pcScope->NominationLevel(pcUsing) == pcLevel))
			{
				pcSymbol = pcUsing->FindMember(lliName);
				if (pcSymbol && !papcFound)
				{
					return pcSymbol;
				}
				if (pcSymbol)
				{
					papcFound->Add(pcSymbol);
				}
			}
		}
		if (pcScope == pcLevel)
		{
			break;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether unqualified lookup of the name from here, at the scope it finds
// it in, finds two things that aren't the same: two using-directives' x, or
// an unnamed namespace's x beside the x of the namespace it is in.  A
// function's overloads are one name.  [namespace.udir] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::IsAmbiguous(int64 lliName)
{
	CArrayCPPSymbols	apcFound;
	CCPPScope*			pcScope;
	CCPPSymbol*			pcDeclared;
	bool				bAmbiguous;

	apcFound.Init();
	bAmbiguous = false;
	for (pcScope = this; pcScope; pcScope = pcScope->mpcParent)
	{
		pcDeclared = pcScope->FindDeclared(lliName, 0);
		if (pcDeclared)
		{
			apcFound.Add(pcDeclared);
		}
		if (pcScope->meKind == CPPSC_Namespace)
		{
			FindNominated(lliName, pcScope, &apcFound);
		}
		if ((apcFound.NumElements() > 0) || pcScope->mpcQualifier)
		{
			break;
		}
	}
	bAmbiguous = AreDifferent(&apcFound);
	apcFound.Kill();
	return bAmbiguous;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether qualified lookup of the name in this namespace finds two things
// that aren't the same: none of its own, and different ones in what its
// using-directives nominate.  [namespace.qual] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::IsAmbiguousMember(int64 lliName)
{
	CArrayCPPSymbols	apcFound;
	CCPPSymbol*			pcSymbol;
	size				i;
	bool				bAmbiguous;

	if ((meKind != CPPSC_Namespace) || FindHere(lliName))
	{
		return false;
	}
	apcFound.Init();
	for (i = 0; i < mapcUsings.NumElements(); i++)
	{
		pcSymbol = mapcUsings.GetPtr(i)->FindMember(lliName);
		if (pcSymbol)
		{
			apcFound.Add(pcSymbol);
		}
	}
	bAmbiguous = AreDifferent(&apcFound);
	apcFound.Kill();
	return bAmbiguous;
}


//////////////////////////////////////////////////////////////////////////
//
// True if what a lookup found is more than one thing: different symbols,
// not all functions.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::AreDifferent(CArrayCPPSymbols* papcFound)
{
	CCPPSymbol*	pcFirst;
	CCPPSymbol*	pcOther;
	size		i;

	if (papcFound->NumElements() < 2)
	{
		return false;
	}
	pcFirst = papcFound->GetPtr(0);
	for (i = 1; i < papcFound->NumElements(); i++)
	{
		pcOther = papcFound->GetPtr(i);
		if ((pcOther != pcFirst) && !(pcOther->IsFunction() && pcFirst->IsFunction()))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The nearest namespace that encloses both this scope and pcNominated.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPScope::NominationLevel(CCPPScope* pcNominated)
{
	CCPPScope*	pcScope;

	for (pcScope = this; pcScope; pcScope = pcScope->mpcParent)
	{
		if ((pcScope->meKind == CPPSC_Namespace) && pcScope->Encloses(pcNominated))
		{
			return pcScope;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPScope::GetDeclarationScope(void)
{
	CCPPScope*	pcScope;

	//Names declared by a template declaration go in the scope enclosing the template parameters.
	for (pcScope = this; pcScope && (pcScope->meKind == CPPSC_Template); pcScope = pcScope->mpcParent)
	{
	}
	return pcScope;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPScope::GetEnclosingNamespace(void)
{
	CCPPScope*	pcScope;

	for (pcScope = this; pcScope && (pcScope->meKind != CPPSC_Namespace); pcScope = pcScope->mpcParent)
	{
	}
	return pcScope;
}


//////////////////////////////////////////////////////////////////////////
//
// A namespace without a name.  The global scope has no owner either, but
// has no parent.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::IsUnnamedNamespace(void)
{
	return (meKind == CPPSC_Namespace) && mpcParent && !mpcOwner;
}


//////////////////////////////////////////////////////////////////////////
//
// An inline namespace: one with a name that its parent's usings have, as
// its members are visible there.  [namespace.def] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::IsInlineNamespace(void)
{
	return (meKind == CPPSC_Namespace) && mpcOwner && mpcParent && (mpcParent->mapcUsings.Find(this) != ARRAY_ELEMENT_NOT_FOUND);
}


//////////////////////////////////////////////////////////////////////////
//
// The namespace without a name directly in this one, or NULL.  It is always
// among the usings, as its members are visible here.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPScope::FindUnnamedNamespace(void)
{
	CCPPScope*	pcUsing;
	size		i;

	for (i = 0; i < mapcUsings.NumElements(); i++)
	{
		pcUsing = mapcUsings.GetPtr(i);
		if (pcUsing->IsUnnamedNamespace() && (pcUsing->mpcParent == this))
		{
			return pcUsing;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what is declared here is the unit's own, as it is in any scope
// inside a namespace without a name.  [basic.link] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::IsInUnnamedNamespace(void)
{
	CCPPScope*	pcScope;

	for (pcScope = this; pcScope; pcScope = pcScope->mpcParent)
	{
		if (pcScope->IsUnnamedNamespace())
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcScope is this scope or inside it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::Encloses(CCPPScope* pcScope)
{
	for (; pcScope; pcScope = pcScope->mpcParent)
	{
		if (pcScope == this)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// If the name is a function declared in this scope, or named here by a using
// declaration, adds the namespace its overloads are in: the scope's own, and
// those of using-declarations limited to what they named (AddAlias).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::AddOverloadScope(int64 lliName, CCPPOverloadScopes* pcNamespaces)
{
	CCPPSymbol*				pcSymbol;
	SCPPScopeUsedFunction*	psUsed;
	size					ui;

	for (pcSymbol = FindHere(lliName); pcSymbol; pcSymbol = pcSymbol->mpcNext)
	{
		if (pcSymbol->IsFunction())
		{
			if ((pcSymbol->mpcScope == this) || (pcSymbol->mpcScope->meKind != CPPSC_Namespace))
			{
				pcNamespaces->Add(pcSymbol->mpcScope->GetEnclosingNamespace());
			}
			for (ui = 0; ui < masUsedFunctions.NumElements(); ui++)
			{
				psUsed = masUsedFunctions.Get(ui);
				if (psUsed->lliName == lliName)
				{
					pcNamespaces->AddBefore(psUsed->pcNamespace, psUsed->iBefore);
				}
			}
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the namespaces this scope's using directives nominate, and those
// they nominate.
//
//////////////////////////////////////////////////////////////////////////
void CCPPScope::AddNominated(CArrayScopes* papcNominated)
{
	CCPPScope*	pcUsing;
	size		i;

	for (i = 0; i < mapcUsings.NumElements(); i++)
	{
		pcUsing = mapcUsings.GetPtr(i);
		if (papcNominated->Find(pcUsing) == ARRAY_ELEMENT_NOT_FOUND)
		{
			papcNominated->Add(pcUsing);
			pcUsing->AddNominated(papcNominated);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Unqualified lookup of a function: outwards, stopping at the first scope
// that has the name, which hides the others.  What a using directive nominates
// counts as declared in the nearest namespace enclosing both, so it can be
// hidden by a name in the namespace that has the directive.  [namespace.udir] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::GatherUnqualified(int64 lliName, CArrayScopes* papcNominated, CCPPOverloadScopes* pcNamespaces)
{
	CCPPScope*	pcScope;
	CCPPScope*	pcNominated;
	size		i;
	bool		bFound;

	for (pcScope = this; pcScope; pcScope = pcScope->mpcParent)
	{
		pcScope->AddNominated(papcNominated);
		bFound = pcScope->AddOverloadScope(lliName, pcNamespaces);
		if (pcScope->meKind == CPPSC_Namespace)
		{
			for (i = 0; i < papcNominated->NumElements(); i++)
			{
				pcNominated = papcNominated->GetPtr(i);
				if (pcScope->Encloses(pcNominated))
				{
					bFound |= pcNominated->AddOverloadScope(lliName, pcNamespaces);
				}
			}
		}
		if (bFound)
		{
			return true;
		}
		if (pcScope->mpcQualifier && pcScope->mpcQualifier->GatherUnqualified(lliName, papcNominated, pcNamespaces))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Qualified lookup of a function in a namespace: its own, with those of the
// inline and unnamed namespaces directly in it, and only if it has none those
// of what its using directives nominate.  [namespace.qual] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::GatherQualified(int64 lliName, CCPPOverloadScopes* pcNamespaces, int iDepth)
{
	CCPPScope*	pcUsing;
	size		i;
	bool		bFound;

	if (iDepth > MAX_LOOKUP_DEPTH)
	{
		return false;
	}
	bFound = AddOverloadScope(lliName, pcNamespaces);
	for (i = 0; i < mapcUsings.NumElements(); i++)
	{
		pcUsing = mapcUsings.GetPtr(i);
		if (pcUsing->mpcParent == this)
		{
			bFound |= pcUsing->GatherQualified(lliName, pcNamespaces, iDepth + 1);
		}
	}
	if (bFound)
	{
		return true;
	}
	for (i = 0; i < mapcUsings.NumElements(); i++)
	{
		bFound |= mapcUsings.GetPtr(i)->GatherQualified(lliName, pcNamespaces, iDepth + 1);
	}
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
// The namespaces whose functions of the name a call looked up from this
// scope can choose between.
//
//////////////////////////////////////////////////////////////////////////
void CCPPScope::GatherOverloadScopes(int64 lliName, bool bQualified, CCPPOverloadScopes* pcNamespaces)
{
	CArrayScopes	apcNominated;

	if (bQualified)
	{
		GatherQualified(lliName, pcNamespaces, 0);
		return;
	}
	apcNominated.Init();
	GatherUnqualified(lliName, &apcNominated, pcNamespaces);
	apcNominated.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The namespaces around this scope, and what using directives nominate on
// the way out.
//
//////////////////////////////////////////////////////////////////////////
void CCPPScope::GatherEnclosingNamespaces(CCPPOverloadScopes* pcNamespaces)
{
	CArrayScopes	apcNominated;
	CCPPScope*		pcScope;
	size			i;

	apcNominated.Init();
	for (pcScope = this; pcScope; pcScope = pcScope->mpcParent)
	{
		if (pcScope->meKind == CPPSC_Namespace)
		{
			pcNamespaces->Add(pcScope);
		}
		pcScope->AddNominated(&apcNominated);
	}
	for (i = 0; i < apcNominated.NumElements(); i++)
	{
		pcNamespaces->Add(apcNominated.GetPtr(i));
	}
	apcNominated.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPScope::IsDependent(void)
{
	CCPPScope*	pcScope;

	for (pcScope = this; pcScope; pcScope = pcScope->mpcParent)
	{
		if (pcScope->mbDependent || (pcScope->meKind == CPPSC_Template))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The namespace the scope is in (or is), and NULL for the global one.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPScope::GetNamespace(void)
{
	CCPPScope*	pcNamespace;

	pcNamespace = GetEnclosingNamespace();
	return (pcNamespace && pcNamespace->mpcParent) ? pcNamespace : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloadScopes::Init(void)
{
	mapcScopes.Init();
	maiBefore.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloadScopes::Kill(void)
{
	maiBefore.Kill();
	mapcScopes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// All of the scope's functions of the name.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloadScopes::Add(CCPPScope* pcScope)
{
	size	i;

	for (i = 0; i < mapcScopes.NumElements(); i++)
	{
		if (mapcScopes.GetPtr(i) == pcScope)
		{
			maiBefore.SetValue(i, -1);
			return;
		}
	}
	mapcScopes.Add(pcScope);
	maiBefore.Add(-1);
}


//////////////////////////////////////////////////////////////////////////
//
// The scope's functions of the name declared before the token iBefore
// (all of them for -1), unless more of them are already.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloadScopes::AddBefore(CCPPScope* pcScope, int iBefore)
{
	size	i;
	int		iHas;

	if (iBefore == -1)
	{
		Add(pcScope);
		return;
	}
	for (i = 0; i < mapcScopes.NumElements(); i++)
	{
		if (mapcScopes.GetPtr(i) == pcScope)
		{
			iHas = maiBefore.GetValue(i);
			if ((iHas != -1) && (iHas < iBefore))
			{
				maiBefore.SetValue(i, iBefore);
			}
			return;
		}
	}
	mapcScopes.Add(pcScope);
	maiBefore.Add(iBefore);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPOverloadScopes::NumElements(void)
{
	return mapcScopes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPOverloadScopes::GetPtr(size i)
{
	return mapcScopes.GetPtr(i);
}


//////////////////////////////////////////////////////////////////////////
//
// The scope as another (its namespace, as functions have them), keeping
// its limit.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloadScopes::SetPtr(size i, CCPPScope* pcScope)
{
	mapcScopes.SetPtr(i, pcScope);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloadScopes::Has(CCPPScope* pcScope)
{
	return mapcScopes.Find(pcScope) != ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function of the name in pcNamespace, declared at the token
// iToken (-1 if that isn't known), can be chosen.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloadScopes::Takes(CCPPScope* pcNamespace, int iToken)
{
	size	i;
	int		iBefore;

	for (i = 0; i < mapcScopes.NumElements(); i++)
	{
		if (mapcScopes.GetPtr(i) == pcNamespace)
		{
			iBefore = maiBefore.GetValue(i);
			if ((iBefore == -1) || (iToken == -1) || (iToken < iBefore))
			{
				return true;
			}
		}
	}
	return false;
}
