/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __65_TRACE_VALUE_H__
#define __65_TRACE_VALUE_H__
#include "BaseLib/Chars.h"


enum E65TraceValue
{
	TV65_Low,
	TV65_High,
	TV65_Unsettled,
	TV65_Undriven,
};


struct S65TraceValue
{
	E65TraceValue	eValue;

	void	Init(void);
	void	Init(E65TraceValue eTraceValue);
	void	Init(bool bValue);
	bool	IsHigh(void);
	bool	IsLow(void);

	void	Invert(void);

	void	Print(CChars* psz);
};


#endif // __65_TRACE_VALUE_H__

