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
#ifndef __ASM65_TOKENISER_H__
#define __ASM65_TOKENISER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "Asm65Token.h"
#include "Asm65Errors.h"


typedef CArrayTemplate<SAsm65Token>	CArrayAsm65Tokens;


//Splits source text into identifiers, numbers, strings, punctuators and new lines.
//Comments start with ';' and run to the end of the line.  The source text must outlive
//the tokens.  The last token is always AT65_End.
class CAsm65Tokeniser
{
protected:
	CArrayAsm65Tokens	mcTokens;
	CChars				mszStrings;
	CAsm65Errors*		mpcErrors;

	const char*			mszPos;
	uint32				muiLine;
	const char*			mszLineStart;

public:
	void			Init(CAsm65Errors* pcErrors);
	void			Kill(void);

	bool			Tokenise(const char* szSource);

	size			NumTokens(void);
	SAsm65Token*	Get(size uiIndex);
	char*			GetString(SAsm65Token* psToken);
	void			Print(CChars* psz);

protected:
	SAsm65Token*	AddToken(EAsm65TokenType eType, const char* szStart);
	bool			Number(void);
	bool			Character(void);
	bool			String(void);
	bool			Escape(char* pc);
	bool			Punctuator(void);
	void			Error(const char* szStart, const char* szMessage);
};


#endif // __ASM65_TOKENISER_H__
