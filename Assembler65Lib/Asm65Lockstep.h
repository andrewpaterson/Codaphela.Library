/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ASM65_LOCKSTEP_H__
#define __ASM65_LOCKSTEP_H__
#include "BaseLib/Chars.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "MicroprocessorLib/Fast65C64.h"
#include "Asm65Memory.h"


//Runs a program on CFast65C64 and on the C65C64 simulator side by side, each with its own memory, and finds where
//they first differ: the registers after every step, memory every so often and at the end.
class CAsm65Lockstep
{
protected:
	CAsm65Memory*	mpcFastMemory;
	CFast65C64*		mpcFast;
	CAsm65Memory*	mpcSimMemory;
	C65C64Meta		mcMeta;
	size			muiSteps;
	bool			mbStopped;
	CChars			mszDifferences;

public:
	void	Init(CAsm65Memory* pcFastMemory, CFast65C64* pcFast);
	void	Kill(void);

	size	Run(size uiMaxSteps, size uiMemoryInterval);
	bool	Step(void);
	bool	CompareMemory(void);

	char*	GetDifferences(void);
	bool	IsStopped(void);
};


#endif // __ASM65_LOCKSTEP_H__

