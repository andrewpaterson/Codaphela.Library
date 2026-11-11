/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "Punctuators.h"
#include "CPPPunctuatorList.h"


//Each punctuator with its length, so a lookup compares the first character before anything else (the
//preprocessor looks up every run of punctuation characters).
struct SPunctuator
{
	const char*	sz;
	size		uiLength;
};


#define PUNCTUATOR(sz)	{ sz, sizeof(sz) - 1 }


#define CPP_PUNCTUATOR_ENTRY(sz, eToken)	PUNCTUATOR(sz),
static const SPunctuator gasPunctuators[] =
{
	CPP_PUNCTUATORS(CPP_PUNCTUATOR_ENTRY)
	CPP_ALTERNATIVE_PUNCTUATORS(CPP_PUNCTUATOR_ENTRY)
};
#undef CPP_PUNCTUATOR_ENTRY


#define NUM_PUNCTUATORS	(sizeof(gasPunctuators) / sizeof(SPunctuator))


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool IsPunctuatorPrefix(const char* sz, size uiLength)
{
	size				i;
	const SPunctuator*	psPunctuator;

	if (uiLength == 0)
	{
		return true;
	}
	for (i = 0; i < NUM_PUNCTUATORS; i++)
	{
		psPunctuator = &gasPunctuators[i];
		if ((psPunctuator->sz[0] == sz[0]) && (psPunctuator->uiLength >= uiLength) && (memcmp(psPunctuator->sz, sz, uiLength) == 0))
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
	size				i;
	size				uiBest;
	const SPunctuator*	psPunctuator;

	uiBest = 0;
	if (uiAvailable == 0)
	{
		return 0;
	}
	for (i = 0; i < NUM_PUNCTUATORS; i++)
	{
		psPunctuator = &gasPunctuators[i];
		if ((psPunctuator->sz[0] == sz[0]) && (psPunctuator->uiLength > uiBest) && (psPunctuator->uiLength <= uiAvailable) && (memcmp(psPunctuator->sz, sz, psPunctuator->uiLength) == 0))
		{
			uiBest = psPunctuator->uiLength;
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

