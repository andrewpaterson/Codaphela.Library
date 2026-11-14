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
#ifndef __I_R_W65_C816_RUNTIME_H__
#define __I_R_W65_C816_RUNTIME_H__
#include "IRBackend.h"
#include "BaseLib/ArrayChars.h"
#include "CPPErrors.h"


class CIRW65C816Backend;


//The routines W65C816 code calls for what it has no instruction for (multiplies, divides, shifts by a count,
//BCD): RuntimeName is the one an instruction calls, and GenerateRuntime writes those a unit used, with their
//tables.
class CIRW65C816Runtime
{
protected:
	CIRW65C816Backend*	mpcBackend;

public:
	void	Init(CIRW65C816Backend* pcBackend);
	void	Kill(void);

	static bool		GenerateRuntime(CArrayChars* paszRuntime, CChars* pszDest, CCPPErrors* pcErrors);
	static void		AppendDivideTable(CChars* pszDest, const char* szName);
	static void		AppendBitsTable(CChars* pszDest, const char* szName);
	void			RuntimeName(CChars* pszName, SIRInstruction* psInstruction);
};


#endif // __I_R_W65_C816_RUNTIME_H__
