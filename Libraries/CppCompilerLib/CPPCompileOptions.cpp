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
#include "CPPCompileOptions.h"


//What Init sets, and what code compiled with no options given is compiled with (GetDefaultCPPCompileOptions).
static CCPPCompileOptions	gcCPPDefaultCompileOptions =
{
	CPPP_Speed,
	true,						//mbOptimise
	true,						//mbInline
	false,						//mbPrintIR
	CPP_NO_FUNCTION_LIMIT,
	false,						//mbWarnNarrowing: code written for a 32 bit int and size has many that are correct.
	true,						//mbWarnMissingOverride (the user, 2026-11-16).
	false,						//mbWarnStringToPointer: the libraries pass string literals as char* throughout (their g++ builds have -Wno-write-strings).
	true,						//mbAssemblyUnits
	true,						//mbSoftFloatInFrame
	true,						//mbRegisterArguments
	true,						//mbHomeV
	true,						//mbHomeW
	true,						//mbHomeB
	true,						//mbMathTables
	CPP_CONSTANT_STEPS,			//muiConstantSteps
	CPP_CONSTANT_DEPTH,			//muiConstantDepth
	false,						//mbExceptions: a program opts in (the user, 2026-11-18).
	false,						//mbKeepUnusedObjects: what nothing uses is dropped (the user, 2026-11-18: "I suspect it's usually fine").
	false,						//mbMultiWordCasts (the user, 2026-11-20: "Refuse it by default, build switch").
	false,						//mbCConversions (the user, 2026-11-20: "Build option", not the file's name).
	false,						//mbDebuggable: a Debug build sets it (the user, 2026-11-23: "Middle + per-project -O0").
};


//////////////////////////////////////////////////////////////////////////
//
// The options a compile has when it is given none.  They are never changed.
//
//////////////////////////////////////////////////////////////////////////
CCPPCompileOptions* GetDefaultCPPCompileOptions(void)
{
	return &gcCPPDefaultCompileOptions;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileOptions::Init(void)
{
	*this = gcCPPDefaultCompileOptions;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the target's assembly beside a unit (the W65C816's XW65C816.asm) is
// built, with __W65C816_ASSEMBLY__ defined to leave out the C++ it
// replaces, or the C++ is compiled.  The W65C816's soft float is one of
// them: softfloatW65C816.asm takes its operands in the caller's frame, and
// softfloat.cpp compiled is called as a function with its arguments pushed.
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileOptions::SetAssemblyUnits(bool bAssemblyUnits)
{
	mbAssemblyUnits = bAssemblyUnits;
	mbSoftFloatInFrame = bAssemblyUnits;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPCompileOptions::PrefersSize(void) const
{
	return mePrefer == CPPP_Size;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether functions are inlined across the unit and the project: not in
// code for a debugger, whose every function is a frame of its own.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPCompileOptions::Inlines(void) const
{
	return mbInline && !mbDebuggable;
}

