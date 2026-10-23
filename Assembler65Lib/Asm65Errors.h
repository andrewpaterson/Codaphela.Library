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
#ifndef __ASM65_ERRORS_H__
#define __ASM65_ERRORS_H__
#include "BaseLib/Chars.h"


//Errors as "line:column: message" lines (the linker's start with the object's name).
class CAsm65Errors
{
protected:
	CChars	mszErrors;
	size	muiErrors;

public:
	void	Init(void);
	void	Kill(void);
	void	Clear(void);

	void	Add(uint32 uiLine, uint32 uiColumn, const char* szMessage, const char* szDetail = NULL);
	void	Add(const char* szSource, uint32 uiLine, uint32 uiColumn, const char* szMessage, const char* szDetail = NULL);
	size	NumErrors(void);
	char*	Text(void);

protected:
	void	AppendMessage(const char* szMessage, const char* szDetail);
};


#endif // __ASM65_ERRORS_H__
