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
#include "BaseLib/Numbers.h"
#include "CPPSymbolTable.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbolTable::Init(void)
{
	mcMemory.Init(16 KB);
	mapcScopes.Init();
	mpcGlobal = CreateScope(CPPSC_Namespace, NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbolTable::Kill(void)
{
	size	i;

	for (i = 0; i < mapcScopes.NumElements(); i++)
	{
		mapcScopes.GetPtr(i)->Kill();
	}
	mapcScopes.Kill();
	mcMemory.Kill();
	mpcGlobal = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPSymbolTable::CreateScope(ECPPScopeKind eKind, CCPPScope* pcParent, CCPPSymbol* pcOwner)
{
	CCPPScope*	pcScope;

	pcScope = (CCPPScope*)mcMemory.Add(sizeof(CCPPScope));
	pcScope->Init(eKind, pcParent, pcOwner);
	mapcScopes.Add(pcScope);
	return pcScope;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPSymbolTable::CreateSymbol(int64 lliName, ECPPSymbolKind eKind, CCPPSyntax* pcDeclaration, CCPPScope* pcScope)
{
	CCPPSymbol*	pcSymbol;

	pcSymbol = (CCPPSymbol*)mcMemory.Add(sizeof(CCPPSymbol));
	pcSymbol->Init(lliName, eKind, pcDeclaration, pcScope);
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPSymbolTable::GetGlobalScope(void)
{
	return mpcGlobal;
}


