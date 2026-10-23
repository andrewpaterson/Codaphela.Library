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
#include <string.h>
#include "BaseLib/StringHelper.h"
#include "Asm65Token.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SAsm65Token::Is(EAsm65Punctuator ePunctuator)
{
	return (eType == AT65_Punctuator) && (this->ePunctuator == ePunctuator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SAsm65Token::IsIdentifier(const char* szName)
{
	if (eType != AT65_Identifier)
	{
		return false;
	}
	return (strlen(szName) == uiLength) && (memcmp(szText, szName, uiLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SAsm65Token::IsIdentifierIgnoreCase(const char* szName)
{
	if (eType != AT65_Identifier)
	{
		return false;
	}
	return (strlen(szName) == uiLength) && (MemICmp(szText, szName, uiLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SAsm65Token::IsEndOfStatement(void)
{
	return (eType == AT65_NewLine) || (eType == AT65_End);
}
