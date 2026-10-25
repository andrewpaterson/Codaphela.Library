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
#ifndef __ASM816_LOCKSTEP_H__
#define __ASM816_LOCKSTEP_H__
#include "BaseLib/Chars.h"
#include "MicroprocessorLib/W65C816Meta.h"
#include "MicroprocessorLib/Fast65C816.h"
#include "Asm65Memory.h"


//Runs a program on CFast65C816 and on the CW65C816 simulator side by side, each with its own memory, and finds where
//they first differ: the registers and cycles after every step, memory every so often and at the end.  As
//CAsm65Lockstep is for the 65C64.
class CAsm816Lockstep
{
protected:
	CAsm65Memory*	mpcFastMemory;
	CFast65C816*	mpcFast;
	CAsm65Memory*	mpcSimMemory;
	CW65C816Meta	mcMeta;
	size			muiSteps;
	bool			mbStopped;
	CChars			mszDifferences;

public:
	void	Init(CAsm65Memory* pcFastMemory, CFast65C816* pcFast);
	void	Kill(void);

	size	Run(size uiMaxSteps, size uiMemoryInterval);
	bool	Step(void);
	bool	CompareMemory(void);

	char*	GetDifferences(void);
	bool	IsStopped(void);
};


#endif // __ASM816_LOCKSTEP_H__

