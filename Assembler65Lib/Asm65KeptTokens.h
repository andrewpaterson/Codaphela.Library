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
#ifndef __ASM65_KEPT_TOKENS_H__
#define __ASM65_KEPT_TOKENS_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "Asm65Token.h"


struct SAsm65KeptToken
{
	SAsm65Token	sToken;
	lsize		uiText;		//Where its text is in the kept tokens' text.
};


//Copies of tokens kept after their line is gone (the assembler reads its source a
//line at a time): pending labels, names used but not yet defined, the sections
//associations name.  A kept token's text is its own copy; strings aren't kept.
class CAsm65KeptTokens
{
protected:
	CArrayLargeTemplate<SAsm65KeptToken>	masTokens;
	CLargeChars								mszText;

public:
	void			Init(void);
	void			Kill(void);
	void			Clear(void);

	SAsm65Token*	Add(SAsm65Token* psToken);
	lsize			NumTokens(void);
	SAsm65Token*	Get(lsize uiIndex);
};


#endif // __ASM65_KEPT_TOKENS_H__
