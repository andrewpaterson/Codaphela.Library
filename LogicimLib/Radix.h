/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __RADIX_H__
#define __RADIX_H__
#include "BaseLib/Chars.h"
#include "MicroprocessorLib/TraceValue.h"


#define RADIX_MAX_BIT_WIDTH		64


//How a value of bits is written, as Logicim's Radix.
enum ERadix
{
	RADIX_Cannot,
	RADIX_Binary,
	RADIX_Decimal,
	RADIX_Hexadecimal,
};


int			CalculateMaxDigits(ERadix eRadix, int iBitWidth);
void		AppendTraceValues(CChars* psz, STraceValue* asValues, int iBitWidth, ERadix eRadix, int iMaxDigits);
const char*	RadixToString(ERadix eRadix);
ERadix		StringToRadix(const char* szRadix);


#endif // __RADIX_H__
