/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __C_P_P_TOKENISER_H__
#define __C_P_P_TOKENISER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/TextParser.h"
#include "ASCIITree.h"
#include "CppTokens.h"
#include "CppToken.h"
#include "PreprocessorTokenParser.h"


typedef CArrayTemplate<CCppToken*>	CCTArrayTokens;


class CTokeniser
{
public:
	CASCIITree					mcReservedWords;
	CASCIITree					mcIdentifiers;
	CCTArrayTokens				mcTokens;
	CMemoryStackExtended		mcStack;
	
	CPreprocessorTokenParser	mcParser;

	void 	Init(void);
	void 	Kill(void);
	BOOL 	Tokenise(CPPTokenHolder* pcSource);
	BOOL	Line(void);
	BOOL	Literal(void);
	BOOL	ReservedWord(void);
	BOOL	Identifier(void);
	BOOL	DoubleLiteral(void);
	BOOL	BooleanLiteral(void);
	BOOL	SingleQuotedLiteral(BOOL bShort);
	BOOL	StringLiteral(void);
	BOOL	WideStringLiteral(void);
	BOOL	IntegerLiteral(void);
	BOOL	ShortLiteral(void);
	void	IntegerType(unsigned long long int ulliValue);
	void	DoubleType(long double ldValue);
	BOOL	DecimalInteger(void);
	void	AddToken(CCppToken* pcToken);
	void	Error(char* szError);
	void	PrintError(CPPLine* pcLine);
	void	PrintTokens(void);
};


#endif // !__C_P_P_TOKENISER_H__

