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
#ifndef __ASM65_SYMBOLS_H__
#define __ASM65_SYMBOLS_H__
#include "BaseLib/MapStringTemplate.h"


//A label or a constant.  uiPass is the pass that last defined it.
struct SAsm65Symbol
{
	int64	iValue;
	uint32	uiPass;
	bool	bResolved;
};


//Symbols by name (case sensitive).  They keep their values between passes so later
//passes can use forward references.
class CAsm65Symbols
{
protected:
	CMapStringTemplate<SAsm65Symbol>	mcSymbols;

public:
	void			Init(void);
	void			Kill(void);

	SAsm65Symbol*	Get(const char* szName, size uiLength);
	SAsm65Symbol*	Put(const char* szName, size uiLength);
	size			NumSymbols(void);
};


#endif // __ASM65_SYMBOLS_H__
