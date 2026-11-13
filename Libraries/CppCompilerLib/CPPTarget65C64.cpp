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
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ConstructorCall.h"
#include "Assembler65Lib/Asm65C64Assembler.h"
#include "Config.h"
#include "CPPCompileOptions.h"
#include "IRInliner.h"
#include "IROptimiser.h"
#include "IR65Backend.h"
#include "CPPTarget65C64.h"


#define CPP65_INLINE_INSTRUCTIONS		16		//The most instructions a function inlined may have...
#define CPP65_INLINE_LOOP_INSTRUCTIONS	32		//...or where the call is in a loop...
#define CPP65_INLINE_HOT_INSTRUCTIONS	64		//...or, with a profile, where the call is hot (CIRInliner::SetProfile)...
#define CPP65_INLINE_CALLS				24		//...and the most calls inlined into one function at a time.
#define CPP65_INLINE_SIZE_INSTRUCTIONS	4		//Preferring size: the most instructions a function inlined may have, wherever its call is.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPTarget CCPPTarget65C64::GetTarget(void)
{
	return CPPT_65C64;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPTarget65C64::GetConfigurationName(void)
{
	return "65C64";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPTarget65C64::GetIntBytes(void)
{
	return 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPTarget65C64::GetPointerBytes(void)
{
	return 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTarget65C64::WidensResults(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPPTarget65C64::GetStackTop(void)
{
	return 0x1FFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// __65C64__ is the processor (Libc65 uses it for what its 65C64 assembly does)
// and __LIBC65__ the Libc65 headers and devices, which BaseLib's Define.h and
// its _65C64.cpp files are written for.  The host's macros (_MSC_VER, _WIN32)
// are not defined, so its headers and _Windows.cpp files are left out.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTarget65C64::AddDefines(CConfig* pcConfig, bool bAssemblyUnits)
{
	pcConfig->AddDefine("__65C64__=1");
	pcConfig->AddDefine("__LIBC65__=1");
	pcConfig->mbKeepPragmaPack = true;
	pcConfig->mbHostDefines = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The start up sets the stack to 001F:FFFF (the first 4 bytes pushed are at
// 001F:FFFC), W to the pseudo registers at 0000:0000 and the registers to 32 bits,
// calls the init table (each global's R_Initialise, in link order), calls main, calls
// the fini table backwards and stops with main's result in A.  Its object must be
// linked first so it is at the start of the text, 0001:0000; the reset vector is left
// to the loader.
//
// After main it calls Libc65's __RunAtExit (the atexit functions, last first) when
// stdlib.cpp is linked: the reference is weak, so it doesn't pull stdlib.cpp in.
//
// It also defines C's _Exit(int iStatus), which stops at once with iStatus in A.
// Libc65's exit runs the atexit functions and the fini table and then calls it.
//
// It is kept, and everything it reaches, when the linker leaves out what is unused, and
// it is resident when functions are loaded lazily.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTarget65C64::AppendStartUp(CChars* pszDest)
{
	pszDest->Append("\t\t.section text, keep, resident")->AppendNewLine();
	pszDest->Append("Start:")->AppendNewLine();
	pszDest->Append("\t\tcpy S, 0x")->Append(GetStackTop(), 16)->AppendNewLine();
	pszDest->Append("\t\tcpy W, 0")->AppendNewLine();
	pszDest->Append("\t\twtth (A, B), 32")->AppendNewLine();
	pszDest->Append("\t\twtth (X, Y), 32")->AppendNewLine();
	pszDest->Append("\t\tcpy Y, __init_start")->AppendNewLine();
	pszDest->Append("StartInitialise:")->AppendNewLine();
	pszDest->Append("\t\tcmp Y, __init_end")->AppendNewLine();
	pszDest->Append("\t\tbras c, StartMain")->AppendNewLine();
	pszDest->Append("\t\tpsh Y")->AppendNewLine();
	pszDest->Append("\t\tjsr [Y]")->AppendNewLine();
	pszDest->Append("\t\tpul Y")->AppendNewLine();
	pszDest->Append("\t\tadd Y, 4")->AppendNewLine();
	pszDest->Append("\t\tbra StartInitialise")->AppendNewLine();
	pszDest->Append("StartMain:")->AppendNewLine();
	pszDest->Append("\t\tjsr _main")->AppendNewLine();
	pszDest->Append("\t\tpsh A")->AppendNewLine();
	pszDest->Append("\t\t.weak ___RunAtExit")->AppendNewLine();
	pszDest->Append("\t\tcpy Y, ___RunAtExit")->AppendNewLine();
	pszDest->Append("\t\tcmp Y, 0")->AppendNewLine();
	pszDest->Append("\t\tbras z, StartAtExitDone")->AppendNewLine();
	pszDest->Append("\t\tjsr Y")->AppendNewLine();
	pszDest->Append("StartAtExitDone:")->AppendNewLine();
	pszDest->Append("\t\tcpy Y, __fini_end")->AppendNewLine();
	pszDest->Append("StartFinalise:")->AppendNewLine();
	pszDest->Append("\t\tcmp Y, __fini_start")->AppendNewLine();
	pszDest->Append("\t\tbras z, StartStop")->AppendNewLine();
	pszDest->Append("\t\tsub Y, 4")->AppendNewLine();
	pszDest->Append("\t\tpsh Y")->AppendNewLine();
	pszDest->Append("\t\tjsr [Y]")->AppendNewLine();
	pszDest->Append("\t\tpul Y")->AppendNewLine();
	pszDest->Append("\t\tbra StartFinalise")->AppendNewLine();
	pszDest->Append("StartStop:")->AppendNewLine();
	pszDest->Append("\t\tpul A")->AppendNewLine();
	pszDest->Append("\t\tstp")->AppendNewLine();
	pszDest->Append("\t\t.global __Exit")->AppendNewLine();
	pszDest->Append("__Exit:")->AppendNewLine();
	pszDest->Append("\t\tcpy V, S")->AppendNewLine();
	pszDest->Append("\t\tcpy A, [5 + V]")->AppendNewLine();
	pszDest->Append("\t\tstp")->AppendNewLine();
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime routines the 65C64's code calls that the unit writes itself:
// the signed divides.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTarget65C64::AppendRuntime(CCPPRuntimeWriter* pcWriter)
{
	if (pcWriter->GetRuntimeCalls()->Contains("R_DivideSigned") && pcWriter->BeginRuntime("R_DivideSigned"))
	{
		AppendDivideSigned(pcWriter->GetRuntimeCode(), "R_DivideSigned", 4);
	}
	if (pcWriter->GetRuntimeCalls()->Contains("R_DivideSigned64") && pcWriter->BeginRuntime("R_DivideSigned64"))
	{
		AppendDivideSigned(pcWriter->GetRuntimeCode(), "R_DivideSigned64", 8);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Signed divide: A / B.  The quotient is in A and the remainder in B.  The quotient
// is negative when the signs differ, the remainder has the dividend's sign.  The
// 64 bit routine is called with A and B 64 bits wide and leaves them so; it widens
// X and Y while it works.  Changes A, B, X and Y.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTarget65C64::AppendDivideSigned(CChars* pszCode, const char* szName, int iBytes)
{
	if (iBytes == 8)
	{
		pszCode->Append("\t\twtth (X, Y), 64")->AppendNewLine();
	}
	pszCode->Append("\t\tcpy X, A")->AppendNewLine();
	pszCode->Append("\t\txor X, B")->AppendNewLine();
	pszCode->Append("\t\tpsh X")->AppendNewLine();
	pszCode->Append("\t\tpsh A")->AppendNewLine();
	pszCode->Append("\t\tabs Y, A")->AppendNewLine();
	pszCode->Append("\t\tabs A, B")->AppendNewLine();
	pszCode->Append("\t\tcpy B, 0")->AppendNewLine();
	pszCode->Append("\t\tdiv")->AppendNewLine();
	pszCode->Append("\t\tcpy W, 0")->AppendNewLine();
	pszCode->Append("\t\tpul X")->AppendNewLine();
	pszCode->Append("\t\tcmp X, 0")->AppendNewLine();
	pszCode->Append("\t\tbrac n, ")->Append(szName)->Append('1')->AppendNewLine();
	pszCode->Append("\t\tneg B, B")->AppendNewLine();
	pszCode->Append(szName);
	pszCode->Append("1:");
	pszCode->AppendNewLine();
	pszCode->Append("\t\tpul X")->AppendNewLine();
	pszCode->Append("\t\tcmp X, 0")->AppendNewLine();
	pszCode->Append("\t\tbrac n, ")->Append(szName)->Append('2')->AppendNewLine();
	pszCode->Append("\t\tneg Y, Y")->AppendNewLine();
	pszCode->Append(szName);
	pszCode->Append("2:");
	pszCode->AppendNewLine();
	pszCode->Append("\t\tcpy A, Y")->AppendNewLine();
	if (iBytes == 8)
	{
		pszCode->Append("\t\twtth (X, Y), 32")->AppendNewLine();
	}
	pszCode->Append("\t\tret")->AppendNewLine();
	pszCode->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// this is in X.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTarget65C64::AppendAdjustThunk(CChars* pszDest, int iAdjust, const char* szFunction)
{
	pszDest->Append("\t\tadd X, ");
	pszDest->Append(iAdjust);
	pszDest->AppendNewLine();
	pszDest->Append("\t\tjmp ");
	pszDest->Append(szFunction);
	pszDest->AppendNewLine();
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// With the object in X:
//
// 		cpy A, [8 + X]
// 		jmp [12 + A]
//
//////////////////////////////////////////////////////////////////////////
void CCPPTarget65C64::AppendVirtualCallThunk(CChars* pszDest, int iVtableOffset, int iEntryOffset)
{
	pszDest->Append("\t\tcpy A, [");
	pszDest->Append(iVtableOffset);
	pszDest->Append(" + X]");
	pszDest->AppendNewLine();
	pszDest->Append("\t\tjmp [");
	pszDest->Append(iEntryOffset);
	pszDest->Append(" + A]");
	pszDest->AppendNewLine();
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Preferring size, only a function no bigger than its call is inlined: an
// accessor, or one that passes a call on.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTarget65C64::GetInlineLimits(SIRInlineLimits* psLimits, CCPPCompileOptions* pcOptions)
{
	psLimits->uiInstructions = CPP65_INLINE_INSTRUCTIONS;
	psLimits->uiLoopInstructions = CPP65_INLINE_LOOP_INSTRUCTIONS;
	psLimits->uiHotInstructions = CPP65_INLINE_HOT_INSTRUCTIONS;
	psLimits->uiCalls = CPP65_INLINE_CALLS;
	psLimits->bForwardersJump = false;
	if (pcOptions->PrefersSize())
	{
		psLimits->uiInstructions = CPP65_INLINE_SIZE_INSTRUCTIONS;
		psLimits->uiLoopInstructions = CPP65_INLINE_SIZE_INSTRUCTIONS;
		psLimits->uiHotInstructions = CPP65_INLINE_SIZE_INSTRUCTIONS;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The standard passes, without those that don't pay on the 65C64: it has a
// multiply and a divide, 32 bit registers and indexed addresses, so divisions
// by constants aren't made multiplies or kept as counters, and counters,
// multiplies and indices aren't narrowed or biased.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTarget65C64::BuildPipeline(CIRPipeline* pcPipeline)
{
	pcPipeline->SetStandard();
	pcPipeline->Remove(IRP_DivisibilityTests);
	pcPipeline->Remove(IRP_ConstantDivisions);
	pcPipeline->Remove(IRP_MergeAddressPhis);
	pcPipeline->Remove(IRP_NarrowCounters);
	pcPipeline->Remove(IRP_NarrowMultiplies);
	pcPipeline->Remove(IRP_ReduceDivisions);
	pcPipeline->Remove(IRP_BiasSignedIndices);
}


//////////////////////////////////////////////////////////////////////////
//
// The 65C64's loads and stores add a pointer and an index for nothing, and
// its div leaves an unsigned remainder in B.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTarget65C64::ConfigureOptimiser(CIROptimiser* pcOptimiser)
{
	pcOptimiser->SetIndexedAddresses(true);
	pcOptimiser->SetConstantDivisions(false);
	pcOptimiser->SetDivideRemainders(IR_DIVIDE_REMAINDERS_8 | IR_DIVIDE_REMAINDERS_16 | IR_DIVIDE_REMAINDERS_32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBackend* CCPPTarget65C64::NewBackend(void)
{
	return NewMalloc<CIR65Backend>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTarget65C64::Assemble(CAsm65Source* pcSource, CAsm65Object* pcObject, const char* szDirectory, const char* szSourceFile, CChars* pszErrors)
{
	CAsm65C64Assembler	cAssembler;
	bool				bResult;

	cAssembler.Init();
	if (szDirectory)
	{
		cAssembler.SetDirectory(szDirectory);
	}
	if (szSourceFile)
	{
		cAssembler.SetSourceFile(szSourceFile);
	}
	bResult = cAssembler.AssembleObject(pcSource, pcObject);
	pszErrors->Append(cAssembler.GetErrors());
	cAssembler.Kill();
	return bResult;
}

