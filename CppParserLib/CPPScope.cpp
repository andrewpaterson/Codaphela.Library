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
#include "CPPScope.h"


#define MAX_LOOKUP_DEPTH	32


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPScope::Init(ECPPScopeKind eKind, CCPPScope* pcParent, CCPPSymbol* pcOwner)
{
	meKind = eKind;
	mpcParent = pcParent;
	mpcOwner = pcOwner;
	mcSymbols.Init(true);
	mapcBases.Init();
	mapcUsings.Init();
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
	mapcBases.Kill();
	mcSymbols.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPScope::Add(CCPPSymbol* pcSymbol)
{
	CCPPSymbol**	ppcExisting;

	//Later declarations of a name chain after the first, so lookup finds the first.
	ppcExisting = mcSymbols.Get(pcSymbol->mlliName);
	if (ppcExisting)
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
void CCPPScope::AddAlias(int64 lliName, CCPPSymbol* pcSymbol)
{
	//The name finds a symbol declared somewhere else, as a using-declaration does.  The symbol is not changed.
	if (!mcSymbols.Get(lliName))
	{
		mcSymbols.Put(lliName, &pcSymbol);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPScope::FindHere(int64 lliName)
{
	CCPPSymbol**	ppcSymbol;

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
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPScope::Find(int64 lliName)
{
	CCPPScope*	pcScope;
	CCPPSymbol*	pcSymbol;

	//Unqualified name lookup: this scope and then the enclosing ones.  [basic.lookup.unqual]
	for (pcScope = this; pcScope; pcScope = pcScope->mpcParent)
	{
		pcSymbol = pcScope->FindMember(lliName);
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

