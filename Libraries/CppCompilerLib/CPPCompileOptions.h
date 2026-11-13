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
#ifndef __C_P_P_COMPILE_OPTIONS_H__
#define __C_P_P_COMPILE_OPTIONS_H__
#include "BaseLib/PrimitiveTypes.h"


//What the compiler prefers where faster code is bigger code: how much is inlined, what is copied in place.
enum ECPPPrefer
{
	CPPP_Speed,		//What the compiler has always done.
	CPPP_Size,
};


#define CPP_NO_FUNCTION_LIMIT	((size)-1)


//How a build's units are compiled, where there is a choice.  A builder has one (CCPP65Builder::GetOptions), set before
//anything is added to it, and gives it to each unit's generator, which hands it down to the IR front end and the back
//end; a generator or a back end given none has the defaults (GetDefaultCPPCompileOptions).  Nothing changes it while
//units are compiled, so every unit and thread reads the same one.
class CCPPCompileOptions
{
public:
	ECPPPrefer	mePrefer;				//Speed or size: how big a function is inlined (CCPPTarget::GetInlineLimits) and how many bytes memcpy copies in place.
	bool		mbOptimise;				//The passes over a function's IR run (CIROptimiser).
	bool		mbInline;				//Small functions are inlined across the unit and the project (CIRInliner).
	bool		mbPrintIR;				//Each function's IR is printed to stderr, for debugging.
	size		muiFunctionLimit;		//Only the first so many of a unit's function bodies are written (to find a wrong one), or CPP_NO_FUNCTION_LIMIT.
	bool		mbWarnNarrowing;		//An implicit conversion of a non constant integer to a narrower type is warned about.
	bool		mbAssemblyUnits;		//The target's assembly beside a unit (XW65C816.asm) is built in place of the C++ it replaces.
	bool		mbSoftFloatInFrame;		//The W65C816's soft float routines take their operands in the caller's frame: the assembly ones do.
	bool		mbRegisterArguments;	//A call's first arguments are passed in registers (the W65C816's A, X and Y, the 65C64's A, B and Y).
	bool		mbMathTables;			//The W65C816's code may read Garth Wilson's product table (MathMult) in place: it is in memory when the code runs.

	void		Init(void);
	void		SetAssemblyUnits(bool bAssemblyUnits);
	bool		PrefersSize(void);
};


CCPPCompileOptions*	GetDefaultCPPCompileOptions(void);


#endif // __C_P_P_COMPILE_OPTIONS_H__

