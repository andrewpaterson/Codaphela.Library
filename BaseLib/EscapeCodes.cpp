/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "StringHelper.h"
#include "EscapeCodes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char GetEscapeCode(char cCurrent)
{
	if (cCurrent == 'n')
	{
		return '\n';
	}
	else if (cCurrent == '\\')
	{
		return '\\';
	}
	else if (cCurrent == '\"')
	{
		return '\"';
	}
	else if (cCurrent == '\'')
	{
		return '\'';
	}
	else if (cCurrent == '?')
	{
		return '\?';
	}
	else if (cCurrent == '0')
	{
		return '\0';
	}
	else if (cCurrent == 'a')
	{
		return '\a';
	}				
	else if (cCurrent == 'b')
	{
		return '\b';
	}				
	else if (cCurrent == 'f')
	{
		return '\f';
	}				
	else if (cCurrent == 'r')
	{
		return '\r';
	}
	else if (cCurrent == 't')
	{
		return '\t';
	}
	else if (cCurrent == 'v')
	{
		return '\v';
	}

	//If @ is returned there was an error.
	return '@';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void StrEscapeUnicode(char16 c, char* szDest)
{
	char c1;
	char c2;
	char c3;
	char c4;

	c1 = (c & 0xf);
	c2 = (c & 0xf0) >> 4;
	c3 = (c & 0xf00) >> 8;
	c4 = (c & 0xf000) >> 12;

	szDest[0] = '\\';
	szDest[1] = 'u';
	szDest[2] = GetHexChar(c4);
	szDest[3] = GetHexChar(c3);
	szDest[4] = GetHexChar(c2);
	szDest[5] = GetHexChar(c1);
	szDest[6] = '\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void StrEscapeHex(char c, char* szDest)
{
	char cRight;
	char cLeft;

	cRight = (c & 0xf);
	cLeft  = (c & 0xf0) >> 4;

	szDest[0] = '\\';
	szDest[1] = 'x';
	szDest[2] = GetHexChar(cLeft);
	szDest[3] = GetHexChar(cRight);
	szDest[4] = '\0';

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void StrEscapeChar(char c, char* szDest)
{
	szDest[0] = '\\';
	szDest[1] = c;
	szDest[2] = '\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* GetEscapeString(char cCurrent, char* szDest)
{
	return GetEscapeString((unsigned char)cCurrent, szDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* GetEscapeString(unsigned char cCurrent, char* szDest)
{
	if (cCurrent == '\\')
	{
		StrEscapeChar('\\', szDest);
	}
	else if (cCurrent == '\n')
	{
		StrEscapeChar('n', szDest);
	}
	else if (cCurrent == '\"')
	{
		StrEscapeChar('\"', szDest);
	}
	else if (cCurrent == '\'')
	{
		StrEscapeChar('\'', szDest);
	}
	else if (cCurrent == '\?')
	{
		StrEscapeChar('?', szDest);
	}
	else if (cCurrent == '\0')
	{
		StrEscapeChar('0', szDest);
	}
	else if (cCurrent == '\a')
	{
		StrEscapeChar('a', szDest);
	}
	else if (cCurrent == '\b')
	{
		StrEscapeChar('b', szDest);
	}
	else if (cCurrent == '\f')
	{
		StrEscapeChar('f', szDest);
	}
	else if (cCurrent == '\r')
	{
		StrEscapeChar('r', szDest);
	}
	else if (cCurrent == '\t')
	{
		StrEscapeChar('t', szDest);
	}
	else if (cCurrent == '\v')
	{
		StrEscapeChar('v', szDest);
	}
	else if (((cCurrent >= 32) && (cCurrent <= 126)) || (cCurrent == 128) || ((cCurrent >= 130) && (cCurrent <= 254)))
	{
		szDest[0] = cCurrent;
		szDest[1] = '\0';
	}
	else 
	{
		StrEscapeHex(cCurrent, szDest);
	}

	return szDest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* GetEscapeString(char16 cCurrent, char* szDest)
{
	StrEscapeUnicode(cCurrent, szDest);
	return szDest;
}

