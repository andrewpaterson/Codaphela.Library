/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ASM65_TOKEN_H__
#define __ASM65_TOKEN_H__
#include "BaseLib/PrimitiveTypes.h"


enum EAsm65TokenType
{
	AT65_End,
	AT65_NewLine,
	AT65_Identifier,
	AT65_Number,
	AT65_String,
	AT65_Punctuator,
};


enum EAsm65Punctuator
{
	AP65_None,
	AP65_Comma,
	AP65_Colon,
	AP65_LeftSquare,
	AP65_RightSquare,
	AP65_LeftRound,
	AP65_RightRound,
	AP65_Plus,
	AP65_Minus,
	AP65_Multiply,
	AP65_Divide,
	AP65_Modulus,
	AP65_And,
	AP65_Or,
	AP65_Xor,
	AP65_Not,
	AP65_LogicalNot,
	AP65_ShiftLeft,
	AP65_ShiftRight,
	AP65_Equals,
};


//A token points into the source text.  Numbers and characters have their value; strings
//have their unescaped text in the tokeniser's string pool at iValue.
struct SAsm65Token
{
	EAsm65TokenType		eType;
	EAsm65Punctuator	ePunctuator;
	uint32				uiLine;
	uint32				uiColumn;
	const char*			szText;
	uint32				uiLength;
	int64				iValue;
	uint32				uiStringLength;

	bool	Is(EAsm65Punctuator ePunctuator);
	bool	IsIdentifier(const char* szName);
	bool	IsIdentifierIgnoreCase(const char* szName);
	bool	IsEndOfStatement(void);
};


#endif // __ASM65_TOKEN_H__
