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
#ifndef __CHARS_HELPER_H__
#define __CHARS_HELPER_H__
#include "Chars.h"
#include "ArrayChars.h"
//This file contains any useful function that are CChars related but don't really fit anywhere.


class CCharsHelper
{
public:
	static void GenerateRandomNumbersCharList(int iNum);
	static void GenerateBitStream(uint8* ucBytes, int iByteCount);
	static void Intersect(CArrayChars* paszDest, CArrayChars* paszLeft, CArrayChars* paszRight);
	static void InverseIntersect(CArrayChars* paszDest, CArrayChars* paszLeft, CArrayChars* paszRight);
};


#endif // __CHARS_HELPER_H__

