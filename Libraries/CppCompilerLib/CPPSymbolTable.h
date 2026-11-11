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
#ifndef __C_P_P_SYMBOL_TABLE_H__
#define __C_P_P_SYMBOL_TABLE_H__
#include "BaseLib/MemoryStackExtended.h"
#include "CPPScope.h"


//Owns the scopes and symbols of a translation unit.
class CCPPSymbolTable
{
protected:
	CMemoryStackExtended	mcMemory;
	CMallocator*			mpcArena;		//The scopes, symbols and scopes' maps are allocated by it, or in mcMemory if NULL.
	CArrayScopes			mapcScopes;
	CCPPScope*				mpcGlobal;

public:
	void			Init(CMallocator* pcArena = NULL);
	void			Kill(void);

	CCPPScope*		CreateScope(ECPPScopeKind eKind, CCPPScope* pcParent, CCPPSymbol* pcOwner);
	CCPPSymbol*		CreateSymbol(int64 lliName, ECPPSymbolKind eKind, CCPPSyntax* pcDeclaration, CCPPScope* pcScope);
	CCPPScope*		GetGlobalScope(void);
	size			NumScopes(void);
	void			RollbackScopes(size uiScopes);
};


#endif // __C_P_P_SYMBOL_TABLE_H__

