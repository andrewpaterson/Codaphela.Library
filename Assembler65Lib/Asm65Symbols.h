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
#include "BaseLib/ArrayChars.h"


//A label or a constant.  uiPass is the pass that last defined it.  In an object
//iSection is the label's section (iValue is its offset), ASM65_SECTION_ABSOLUTE for a
//plain value or ASM65_SECTION_UNDEFINED for an external.  uiTerm is the index of the
//label or external a constant's value is relative to (a label's is its own).  An
//external was named at uiNamedLine and uiNamedColumn.  uiIndex is the symbol's own
//index, in the order symbols were added.
struct SAsm65Symbol
{
	int64	iValue;
	uint32	uiPass;
	bool	bResolved;
	int32	iSection;
	size	uiTerm;
	size	uiIndex;
	uint32	uiNamedLine;
	uint32	uiNamedColumn;
	bool	bExternal;
	bool	bGlobal;
	bool	bWeak;
	bool	bLabel;
};


//Symbols by name (case sensitive).  They keep their values between passes so later
//passes can use forward references.  Names are kept in the order they were added.
class CAsm65Symbols
{
protected:
	CMapStringTemplate<SAsm65Symbol>	mcSymbols;
	CArrayChars							maszNames;

public:
	void			Init(void);
	void			Kill(void);

	SAsm65Symbol*	Get(const char* szName, size uiLength);
	SAsm65Symbol*	Put(const char* szName, size uiLength);
	size			NumSymbols(void);
	char*			GetName(size uiIndex);
};


#endif // __ASM65_SYMBOLS_H__
