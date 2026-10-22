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
#include <string.h>
#include "Punctuators.h"


static const char* gaszPunctuators[] =
{
	"{", "}", "[", "]", "#", "##", "(", ")", "<:", ":>", "<%", "%>", "%:", "%:%:", ";", ":", "...",
	"?", "::", ".", ".*", "+", "-", "*", "/", "%", "^", "&", "|", "~", "!", "=", "<", ">",
	"+=", "-=", "*=", "/=", "%=", "^=", "&=", "|=", "<<", ">>", ">>=", "<<=", "==", "!=", "<=", ">=",
	"&&", "||", "++", "--", ",", "->*", "->",
	NULL
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool IsPunctuator(const char* sz, size uiLength)
{
	size	i;

	for (i = 0; gaszPunctuators[i] != NULL; i++)
	{
		if ((strlen(gaszPunctuators[i]) == uiLength) && (memcmp(gaszPunctuators[i], sz, uiLength) == 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool IsPunctuatorPrefix(const char* sz, size uiLength)
{
	size	i;

	for (i = 0; gaszPunctuators[i] != NULL; i++)
	{
		if ((strlen(gaszPunctuators[i]) >= uiLength) && (memcmp(gaszPunctuators[i], sz, uiLength) == 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size PunctuatorLength(const char* sz, size uiAvailable)
{
	size	i;
	size	uiLength;
	size	uiBest;

	uiBest = 0;
	for (i = 0; gaszPunctuators[i] != NULL; i++)
	{
		uiLength = strlen(gaszPunctuators[i]);
		if ((uiLength > uiBest) && (uiLength <= uiAvailable) && (memcmp(gaszPunctuators[i], sz, uiLength) == 0))
		{
			uiBest = uiLength;
		}
	}

	//C++11 [lex.pptoken]: <:: is < followed by :: unless the next character is : or >.
	if ((uiBest == 2) && (sz[0] == '<') && (sz[1] == ':') && (uiAvailable >= 3) && (sz[2] == ':'))
	{
		if ((uiAvailable == 3) || ((sz[3] != ':') && (sz[3] != '>')))
		{
			uiBest = 1;
		}
	}
	return uiBest;
}

