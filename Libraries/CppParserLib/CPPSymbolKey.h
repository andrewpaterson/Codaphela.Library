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
#ifndef __C_P_P_SYMBOL_KEY_H__
#define __C_P_P_SYMBOL_KEY_H__
#include "BaseLib/Chars.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"


class CCPPSyntaxDeclarator;
class CCPPSyntaxFunctionSuffix;
class CCPPSyntaxParameter;


//What names a symbol the same in every translation unit, so the units' own symbols can be matched: its qualified name
//(CChars::mcText), a function's with its parameter types (CChars::Append(char const*)), and a local's the place it is
//declared (@File.cpp:12:5).
class CCPPSymbolKey
{
protected:
	CCPPTokeniser*	mpcTokeniser;
	CCPPSyntaxTree*	mpcTree;

public:
	void					Init(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree);
	void					Kill(void);

	bool					GetKey(CCPPSymbol* pcSymbol, CChars* pszDest);
	bool					GetFunctionKey(CCPPSymbol* pcSymbol, CCPPSyntaxDeclarator* pcDeclarator, CChars* pszDest);
	void					GetQualifiedName(CCPPSymbol* pcSymbol, CChars* pszDest);
	void					GetSignature(CCPPSyntaxFunctionSuffix* pcSuffix, CChars* pszDest);
	CCPPSyntaxDeclarator*	GetDeclarator(CCPPSymbol* pcSymbol);
	bool					IsLocal(CCPPSymbol* pcSymbol);
	bool					IsFunction(CCPPSymbol* pcSymbol);
	bool					GetParameterCounts(CCPPSymbol* pcSymbol, int* piMinimum, int* piMaximum);

protected:
	void					AppendName(CCPPSymbol* pcSymbol, CChars* pszDest);
	void					AppendParameter(CCPPSyntaxParameter* pcParameter, CChars* pszDest);
	void					AppendToken(int iToken, CChars* pszDest);
	void					AppendSpaced(int iToken, CChars* pszDest);
	CCPPSyntaxDeclarator*	FindDeclarator(CCPPSyntax* pcNode, CCPPSymbol* pcSymbol);
};


bool IsCPPNameCharacter(char c);
bool IsDeclaratorToken(CCPPToken* pcToken);


#endif // __C_P_P_SYMBOL_KEY_H__

