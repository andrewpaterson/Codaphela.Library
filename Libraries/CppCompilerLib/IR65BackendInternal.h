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
#ifndef __I_R_65_BACKEND_INTERNAL_H__
#define __I_R_65_BACKEND_INTERNAL_H__
#include "IR65Backend.h"


//What the 65C64 back end's files (IR65Backend.cpp and the others IR65*.cpp of CIR65Backend) share.


//What the lowering of an opcode does (IR65OpcodeFlags).
#define IR65F_TAKES_A		0x01	//Takes its first operand from A when the instruction before left it there.
#define IR65F_TAKES_B		0x02	//Takes its second operand from A (copying or swapping it) the same way.
#define IR65F_COMMUTES		0x04
#define IR65F_CALLS			0x08	//Changes X, Y and [$00 + W] to [$7F + W].
#define IR65F_CHANGES_XY	0x10	//Calls the runtime: changes X and Y.
#define IR65F_CHANGES_Y		0x20	//mul and div: change Y.
#define IR65F_NO_DEFER		0x40	//Loads into registers the deferred load's address may use.


//IR65Instructions.cpp: what the lowering of an opcode needs and changes.
uint8	IR65OpcodeFlags(uint16 uiOpcode);
const char*	IR65IntegerMnemonic(uint16 uiOpcode);

//IR65BackendPeephole.cpp: [n + W] written [n] where the instruction can't address memory with W.
void	IR65PseudoWithoutW(CChars* pszCode, size uiStart);


#endif // __I_R_65_BACKEND_INTERNAL_H__
