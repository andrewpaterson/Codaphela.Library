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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "DebugOutput.h"
#include "DataTypes.h"
#include "PointerRemapper.h"
#include "IntegerHelper.h"
#include "ArrayChars.h"
#include "StringHelper.h"
#include "Newline.h"
#include "FileWriter.h"
#include "FileReader.h"
#include "FileUtil.h"
#include "NullAllocator.h"
#include "FloatPrinter.h"
#include "Chars.h"
#include "PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareChars(const void* arg1, const void* arg2)
{
	CChars*	szString1;
	CChars*	szString2;

	szString1 = (CChars*)arg1;
	szString2 = (CChars*)arg2;

	return szString1->Compare(szString2);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareCharsIgnoreCase(const void* arg1, const void* arg2)
{
	CChars*	szString1;
	CChars*	szString2;

	szString1 = (CChars*)arg1;
	szString2 = (CChars*)arg2;

	return szString1->CompareIgnoreCase(szString2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char GetNumberAs1DigitHex(size nybble)
{
	if (nybble <= 15)
	{
		return gszDigits[nybble];
	}
	return '?';
}


#include "ArraySizeNatural.inl"
#include "CharsCode.inl"
#include "ArraySizeUndef.inl"

#include "ArraySizeLarge.inl"
#include "CharsCode.inl"
#include "ArraySizeUndef.inl"
