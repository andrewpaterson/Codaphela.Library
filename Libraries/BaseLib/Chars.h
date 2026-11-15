/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __CHARS_H__
#define __CHARS_H__
#include "StringHelper.h"
#include "ExternalString.h"
#include "ArrayChar.h"
#include "CharHelper.h"
#include "ArrayTemplate.h"
#include "ArrayInt.h"
#include "CharsImmutable.h"


class CArrayChars;
#include "CharsClass.inl"


//The names the strings are known by.  A large string also appends a natural one.
class CChars : public CCharsBase<CChars, CArrayChar>
{
CONSTRUCTABLE(CChars)
};


class CLargeChars : public CCharsBase<CLargeChars, CArrayLargeChar>
{
CONSTRUCTABLE(CLargeChars)
public:
	using CCharsBase<CLargeChars, CArrayLargeChar>::Append;

	CLargeChars*	Append(CChars szString);
	CLargeChars*	Append(CChars* psz);
};


//Chars.cpp instantiates them; no other unit compiles their functions.
extern template class CCharsBase<CChars, CArrayChar>;
extern template class CCharsBase<CLargeChars, CArrayLargeChar>;


int CompareChars(const void* arg1, const void* arg2);
int CompareCharsIgnoreCase(const void* arg1, const void* arg2);


#endif // __CHARS_H__

