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


//Where a syntax tree is allocated (CCPPSyntaxTree::Init): a builder compiling unit after unit on a thread gives it
//allocators that keep their memory for the next unit (CCPP65UnitWorkspace).
struct SCPPSyntaxTreeMallocators
{
	CMallocator*	pcSyntaxes;			//The list of nodes.
	CMallocator*	pcTokenSymbols;		//What each token was found to be.
	CMallocator*	pcNodeArrays;		//The nodes' own arrays (their children and the like).
	CMallocator*	pcArena;			//The nodes, scopes and symbols: an arena (they are all freed with the tree).
};


class CCPPSyntaxTranslationUnit;
class CCPPSyntaxTree
{
protected:
	CCPPTokeniser*				mpcTokeniser;
	CCPPSyntaxMemory			mcMemory;
	CCPPSymbolTable				mcSymbols;
	CCPPSyntaxTranslationUnit*	mpcRoot;
	CCPPErrors					mcErrors;
	CArrayTokenSymbols			masTokenSymbols;
	CMallocator*				mpcNodeArrays;

public:
	void						Init(CCPPTokeniser* pcTokeniser, SCPPSyntaxTreeMallocators* psMallocators = NULL);
	void						Kill(void);

	CCPPTokeniser*				GetTokeniser(void);
	CCPPSyntaxMemory*			GetMemory(void);
	CMallocator*				GetNodeArrayMallocator(void);
	CCPPSymbolTable*			GetSymbols(void);
	CCPPSyntaxTranslationUnit*	GetRoot(void);
	void						SetRoot(CCPPSyntaxTranslationUnit* pcRoot);

	CCPPErrors*					GetErrors(void);
	size						NumErrors(void);
	char*						GetError(size iIndex);
	int							GetErrorToken(size iIndex);

	void						SetTokenSymbol(int iToken, CCPPSymbol* pcSymbol, ECPPTokenRole eRole);
	SCPPTokenSymbol*			GetTokenSymbol(int iToken);

	void						PrettyPrint(CChars* pszDest);
	void						TypePrint(CChars* pszDest);
};


#endif // __C_P_P_SYNTAX_TREE_H__

