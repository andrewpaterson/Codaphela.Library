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
#include "CPPTarget.h"


//The 65C64's numbers are the generator's (CPP65Generator.txt Memory map); the W65C816's are W65C816.txt's.
static SCPPTarget	gasCPPTargets[CPPT_NumTargets] =
{
	{ CPPT_65C64,	"65C64",	"__65C64__=1",		4, 4, true,  true,  0,			0x1FFFFF },
	{ CPPT_W65C816,	"W65C816",	"__W65C816__=1",	2, 4, false, false, 0x10000,	0xEFFF },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPPTarget* GetCPPTarget(ECPPTarget eTarget)
{
	return &gasCPPTargets[eTarget];
}

