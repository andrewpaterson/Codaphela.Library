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
#ifndef __C_P_P_SYNTAX_TREE_H__
#define __C_P_P_SYNTAX_TREE_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayInt.h"
#include "CppTokeniser.h"
#include "CPPSyntaxMemory.h"
#include "CPPSymbolTable.h"


//What a token was found to be by the parser.
enum ECPPTokenRole
{
	CPPTR_None,
	CPPTR_Declaration,		//The name being declared.
	CPPTR_Reference,		//A use of a declared name.
	CPPTR_Unresolved,		//A name that was not found (dependent names, names in skimmed code).
};


struct SCPPTokenSymbol
{
	CCPPSymbol*		pcSymbol;
	uint8			uiRole;
};


typedef CArrayTemplate<SCPPTokenSymbol>	CArrayTokenSymbols;


class CCPPSyntaxTranslationUnit;
class CCPPSyntaxTree
{
protected:
	CCPPTokeniser*				mpcTokeniser;
	CCPPSyntaxMemory			mcMemory;
	CCPPSymbolTable				mcSymbols;
	CCPPSyntaxTranslationUnit*	mpcRoot;
	CArrayChars					maszErrors;
	CArrayInt					maiErrorTokens;
	CArrayTokenSymbols			masTokenSymbols;

public:
	void						Init(CCPPTokeniser* pcTokeniser);
	void						Kill(void);

	CCPPTokeniser*				GetTokeniser(void);
	CCPPSyntaxMemory*			GetMemory(void);
	CCPPSymbolTable*			GetSymbols(void);
	CCPPSyntaxTranslationUnit*	GetRoot(void);
	void						SetRoot(CCPPSyntaxTranslationUnit* pcRoot);

	void						AddError(int iToken, char* szError);
	size						NumErrors(void);
	char*						GetError(size iIndex);
	int							GetErrorToken(size iIndex);

	void						SetTokenSymbol(int iToken, CCPPSymbol* pcSymbol, ECPPTokenRole eRole);
	SCPPTokenSymbol*			GetTokenSymbol(int iToken);

	void						PrettyPrint(CChars* pszDest);
	void						TypePrint(CChars* pszDest);
};


#endif // __C_P_P_SYNTAX_TREE_H__

