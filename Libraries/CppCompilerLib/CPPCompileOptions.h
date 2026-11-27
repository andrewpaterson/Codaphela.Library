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
#define CPP_CONSTANT_STEPS		1048576		//Full-expressions evaluated to work out one constant, by default.  [implimits]
#define CPP_CONSTANT_DEPTH		512			//constexpr calls inside calls, by default.  [implimits]


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
	bool		mbWarnMissingOverride;	//A function that overrides a base class's virtual function and isn't marked override (or final) is warned about.
	bool		mbWarnStringToPointer;	//A string literal converted to a char*, which C++14 doesn't have but is still done, is warned about.
	bool		mbAssemblyUnits;		//The target's assembly beside a unit (XW65C816.asm) is built in place of the C++ it replaces.
	bool		mbSoftFloatInFrame;		//The W65C816's soft float routines take their operands in the caller's frame: the assembly ones do.
	bool		mbRegisterArguments;	//A call's first arguments are passed in registers (the W65C816's A, X and Y, the 65C64's A, B and Y).
	bool		mbHomeV;				//The 65C64's V is a third register for values in a function that needs no frame (CIR65Backend::CanUseV).
	bool		mbHomeW;				//The 65C64's W is a fourth register for values between calls, multiplies and divides (IR65R_W).
	bool		mbHomeB;				//The 65C64's B is a fifth register, for values whose code doesn't work in B while they are live (IR65R_B).
	bool		mbMathTables;			//The W65C816's code may read Garth Wilson's product table (MathMult) in place: it is in memory when the code runs.
	size		muiConstantSteps;		//Full-expressions evaluated to work out one constant expression before it is given up on (CCPPConstants).
	size		muiConstantDepth;		//How deep constexpr calls may go inside one another for a constant.
	bool		mbExceptions;			//throw, try and catch are compiled (ExceptionsPlan.txt); without it they are errors and nothing pays for them.
	bool		mbKeepUnusedObjects;	//A global whose making or destroying runs code (R_Initialise, R_Finalise) is linked though
									//nothing refers to it ([basic.stc.static] 2); off, it goes with the rest of what is unused.
	bool		mbMultiWordCasts;		//unsigned int(3), a cast in the form T(x) whose type is more than one word, is accepted as the
									//Microsoft compiler accepts it; off, it is the error C++14 has it be ([expr.type.conv] 1).
	bool		mbCConversions;			//The sources are C: a void* becomes any object's pointer and a number an enum's value with no
									//cast, an enum is incremented and a pointer compared with 0 by < and > (C11 6.3.2.3, 6.5.16.1).
	bool		mbDebuggable;			//The code is for a debugger (DebuggerPlan.txt A2): each named variable, parameters too, has a
									//frame slot of its own that always holds its value, nothing is inlined and every frame keeps its
									//shape.  With mbOptimise the rest is optimised (a Debug build); without, nothing is.
	bool		mbDebugMarkers;			//With a line table, a variable promoted out of its slot leaves debug value markers in the IR
									//(DebuggerFPlan.txt F1); off, its value isn't described.  Off only to check they change no code.

	void		Init(void);
	void		SetAssemblyUnits(bool bAssemblyUnits);
	void		SetOwn(CCPPCompileOptions* pcSource);
	bool		PrefersSize(void) const;
	bool		Inlines(void) const;
};


CCPPCompileOptions*	GetDefaultCPPCompileOptions(void);


//What a program built without exceptions (mbExceptions) is told where it uses them.
#define CPP_EXCEPTIONS_OFF_THROW	"throw can't be used: the program is built without exceptions (the Exceptions option)"
#define CPP_EXCEPTIONS_OFF_TRY		"try can't be used: the program is built without exceptions (the Exceptions option)"


#endif // __C_P_P_COMPILE_OPTIONS_H__

