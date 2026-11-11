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
#include "Config.h"
#include "BaseLib/ConstructorCall.h"
#include "Assembler65Lib/Asm65816Assembler.h"
#include "CPPErrors.h"
#include "CPPCompileOptions.h"
#include "IRInliner.h"
#include "IROptimiser.h"
#include "IR816Backend.h"
#include "CPPTargetW65C816.h"


//A call costs the W65C816 far more than the 65C64 (about 70 cycles with its frame), so more is inlined (measured
//2026-10-31, TestBaseLib65 and TestStandardLib65: 16/32/24 to 32/64/32 -3.3% and -5.0%, then to these -2.0% and
//-3.5%; 96 calls ran one out of bank 0's stack).
#define CPP816_INLINE_INSTRUCTIONS		64		//The most instructions a function inlined may have...
#define CPP816_INLINE_LOOP_INSTRUCTIONS	128		//...or where the call is in a loop...
#define CPP816_INLINE_HOT_INSTRUCTIONS	192		//...or, with a profile, where the call is hot: more than in a loop, or a profile would add nothing there...
#define CPP816_INLINE_CALLS				64		//...and the most calls inlined into one function at a time.
#define CPP816_INLINE_SIZE_INSTRUCTIONS	4		//Preferring size: the most instructions a function inlined may have, wherever its call is.


//The W65C816's thunks, which work a word at a time.  A this adjusting thunk (AppendAdjustThunk) has its caller's
//stack: this, the first argument and so pushed last, is above the long return address.  A virtual call thunk
//(AppendVirtualCallThunk) saves D and pushes a word twice, and D then points at the stack: from D, what follows.
//The function's address - 1 is put at CPP65_816_VCALL_TARGET, over the saved D, for rtl to jump to.
#define CPP65_816_WORD_BYTES			2
#define CPP65_816_ADJUST_THIS			4	//From S: this, in the adjusting thunk.
#define CPP65_816_VCALL_VTABLE			1	//The vtable pointer (a low word and a high word).
#define CPP65_816_VCALL_TARGET			4	//The function's address - 1: its low word...
#define CPP65_816_VCALL_CALLER_D		5	//The caller's D.
#define CPP65_816_VCALL_TARGET_BANK		6	//...and its bank.
#define CPP65_816_VCALL_THIS			10	//this, past the caller's D and the long return address.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPTarget CCPPTargetW65C816::GetTarget(void)
{
	return CPPT_W65C816;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPTargetW65C816::GetConfigurationName(void)
{
	return "W65C816";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPTargetW65C816::GetIntBytes(void)
{
	return 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPTargetW65C816::GetPointerBytes(void)
{
	return 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPPTargetW65C816::GetBankSize(void)
{
	return 0x10000;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPPTargetW65C816::GetStackTop(void)
{
	return 0xEFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// __W65C816__ is the processor, __LIBC65__ the Libc65 headers and devices
// (as the 65C64's), and __W65C816_ASSEMBLY__ leaves out the C++ that a
// unit's X816.asm replaces when those are built (bAssemblyUnits).
//
//////////////////////////////////////////////////////////////////////////
void CCPPTargetW65C816::AddDefines(CConfig* pcConfig, bool bAssemblyUnits)
{
	pcConfig->AddDefine("__W65C816__=1");
	pcConfig->AddDefine("__LIBC65__=1");
	if (bAssemblyUnits)
	{
		pcConfig->AddDefine("__W65C816_ASSEMBLY__=1");
	}
	pcConfig->mbKeepPragmaPack = true;
	pcConfig->mbHostDefines = false;
}


//////////////////////////////////////////////////////////////////////////
//
// bss takes no space in the program's image and the start up clears it.
// (The 65C64's start up and lazy loading don't, so it keeps zeros in data.)
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTargetW65C816::HasZeroedBss(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A 16 bit index is scaled and added in 16 bits, not widened to 32 first.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTargetW65C816::NarrowsSubscripts(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTargetW65C816::HasDecimalMode(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime counts 16 and 32 bits with a byte table read with long,X;
// 64 bits are narrowed to a byte looked up in GetCountTable's.
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPTargetW65C816::GetCountRoutine(bool bLeading, int iBytes)
{
	if (iBytes > 4)
	{
		return NULL;
	}
	if (bLeading)
	{
		return (iBytes == 4) ? "R816_Clz32" : "R816_Clz16";
	}
	return (iBytes == 4) ? "R816_Ctz32" : "R816_Ctz16";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPTargetW65C816::GetCountTable(bool bLeading)
{
	return bLeading ? "R816_Clz8" : "R816_Ctz8";
}


//////////////////////////////////////////////////////////////////////////
//
// A call costs the W65C816 far more than the 65C64 (about 70 cycles with
// its frame), so small functions are inlined across the project's units.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTargetW65C816::InlinesAcrossUnits(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPTargetW65C816::GetAssemblyUnitSuffix(void)
{
	return "816.asm";
}


//////////////////////////////////////////////////////////////////////////
//
// Libc65's runtime (with its own assembly for string and soft float).
//
//////////////////////////////////////////////////////////////////////////
const char** CCPPTargetW65C816::GetLibc65Units(void)
{
	static const char*	aszUnits[] = { "string.cpp", "stdlib.cpp", "ctype.cpp", "time.cpp", "math.cpp", "softfloat.cpp", "new.cpp", "signal.cpp", "stdio.cpp", "host.cpp", "string816.asm", "string816_16.asm", "softfloat816.asm", NULL };

	return aszUnits;
}


//////////////////////////////////////////////////////////////////////////
//
// The back end's runtime routines (R816_Mul16 ...: multiply, divide, shifts
// and BCD): the common ones, and the multiplies that read MathMult
// (bMathTables) or the ones that don't.
//
//////////////////////////////////////////////////////////////////////////
const char** CCPPTargetW65C816::GetRuntimeUnits(bool bMathTables)
{
	static const char*	aszTables[] = { "IR816Runtime-Common.asm", "IR816Runtime-MathTables.asm", NULL };
	static const char*	aszSafe[] = { "IR816Runtime-Common.asm", "IR816Runtime-MathSafe.asm", NULL };

	return bMathTables ? aszTables : aszSafe;
}


//////////////////////////////////////////////////////////////////////////
//
// Garth Wilson's tables: MathMult for the runtime's multiply and the soft
// float's, MathInvert for the soft float's divide.
//
//////////////////////////////////////////////////////////////////////////
const char** CCPPTargetW65C816::GetMathTableUnits(void)
{
	static const char*	aszUnits[] = { "MathMult.asm", "MathInvert.asm", NULL };

	return aszUnits;
}


//////////////////////////////////////////////////////////////////////////
//
// stp.
//
//////////////////////////////////////////////////////////////////////////
int CCPPTargetW65C816::GetStubOpcode(void)
{
	return 0xDB;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPTargetW65C816::GetRuntimePrefix(void)
{
	return "R816_";
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816's start up.  Bank 0 only holds the stack, the direct page, the vectors and their stubs (the user,
// 2026-10-25), so the reset vector goes to a jml stub there and the start up is in bank 1 onwards.  It switches to
// native mode with 16 bit registers, sets S to the target's stack top and D to 0, clears bss, calls the init table's routines
// (each global's R_Initialise, in link order), calls main, Libc65's __RunAtExit when stdlib.cpp is linked (a weak
// reference), the fini table's routines last first, and stops with main's result in A.  The table walks keep a far
// pointer at 0000:0000 (the direct page while D is 0).  _Exit(int) stops with its argument in A (__Exit.a, the
// register entry the compiler's callers use, and __Exit, the stack entry).
//
//////////////////////////////////////////////////////////////////////////
void CCPPTargetW65C816::AppendStartUp(CChars* pszDest)
{
	pszDest->Append("\t\t.section rodata.ResetVector, at 0xFFFC, keep\n");
	pszDest->Append("\t\t.word ResetStub\n");
	pszDest->Append("\t\t.section text.ResetStub, at 0xFF00, keep\n");
	pszDest->Append("ResetStub:\n");
	pszDest->Append("\t\tjml Start\n");
	pszDest->Append("\t\t.section text, keep\n");
	pszDest->Append("Start:\n");
	pszDest->Append("\t\tclc\n");
	pszDest->Append("\t\txce\n");
	pszDest->Append("\t\trep (m, x)\n");
	pszDest->Append("\t\tlda 0x")->Append(GetStackTop(), 16)->AppendNewLine();
	pszDest->Append("\t\ttcs\n");
	pszDest->Append("\t\tlda 0\n");
	pszDest->Append("\t\ttcd\n");

	//bss (globals with no initialiser) isn't in the program's image.  Up to 64KB is cleared a word at a time (a long
	//address plus X crosses banks; an odd size clears the byte after it too, where nothing is yet), more a byte at a time.
	pszDest->Append("\t\tlda __bss_end & 0xFFFF\n");
	pszDest->Append("\t\tsec\n");
	pszDest->Append("\t\tsbc __bss_start & 0xFFFF\n");
	pszDest->Append("\t\ttay\n");
	pszDest->Append("\t\tlda __bss_end >> 16\n");
	pszDest->Append("\t\tsbc __bss_start >> 16\n");
	pszDest->Append("\t\tbne StartClearLarge\n");
	pszDest->Append("\t\ttya\n");
	pszDest->Append("\t\tinc A\n");
	pszDest->Append("\t\tlsr A\n");
	pszDest->Append("\t\tbeq StartCleared\n");
	pszDest->Append("\t\ttay\n");
	pszDest->Append("\t\tldx 0\n");
	pszDest->Append("\t\tlda 0\n");
	pszDest->Append("StartClearWord:\n");
	pszDest->Append("\t\tsta [__bss_start + X]:24\n");
	pszDest->Append("\t\tinx\n");
	pszDest->Append("\t\tinx\n");
	pszDest->Append("\t\tdey\n");
	pszDest->Append("\t\tbne StartClearWord\n");
	pszDest->Append("\t\tbra StartCleared\n");
	pszDest->Append("StartClearLarge:\n");
	pszDest->Append("\t\tlda __bss_start & 0xFFFF\n");
	pszDest->Append("\t\tsta [D + 0]\n");
	pszDest->Append("\t\tlda __bss_start >> 16\n");
	pszDest->Append("\t\tsta [D + 2]\n");
	pszDest->Append("StartClear:\n");
	pszDest->Append("\t\tlda [D + 0]\n");
	pszDest->Append("\t\tcmp __bss_end & 0xFFFF\n");
	pszDest->Append("\t\tbne StartClearByte\n");
	pszDest->Append("\t\tlda [D + 2]\n");
	pszDest->Append("\t\tcmp __bss_end >> 16\n");
	pszDest->Append("\t\tbeq StartCleared\n");
	pszDest->Append("StartClearByte:\n");
	pszDest->Append("\t\tsep m\n");
	pszDest->Append("\t\tlda 0\n");
	pszDest->Append("\t\tsta [[D + 0]:24]\n");
	pszDest->Append("\t\trep m\n");
	pszDest->Append("\t\tinc [D + 0]\n");
	pszDest->Append("\t\tbne StartClear\n");
	pszDest->Append("\t\tinc [D + 2]\n");
	pszDest->Append("\t\tbra StartClear\n");
	pszDest->Append("StartCleared:\n");
	pszDest->Append("\t\tlda __init_start & 0xFFFF\n");
	pszDest->Append("\t\tsta [D + 0]\n");
	pszDest->Append("\t\tlda __init_start >> 16\n");
	pszDest->Append("\t\tsta [D + 2]\n");
	pszDest->Append("StartInitialise:\n");
	pszDest->Append("\t\tlda [D + 0]\n");
	pszDest->Append("\t\tcmp __init_end & 0xFFFF\n");
	pszDest->Append("\t\tbne StartInitialiseCall\n");
	pszDest->Append("\t\tlda [D + 2]\n");
	pszDest->Append("\t\tcmp __init_end >> 16\n");
	pszDest->Append("\t\tbeq StartMain\n");
	pszDest->Append("StartInitialiseCall:\n");
	pszDest->Append("\t\tjsr StartCallEntry\n");
	pszDest->Append("\t\tclc\n");
	pszDest->Append("\t\tlda [D + 0]\n");
	pszDest->Append("\t\tadc 4\n");
	pszDest->Append("\t\tsta [D + 0]\n");
	pszDest->Append("\t\tlda [D + 2]\n");
	pszDest->Append("\t\tadc 0\n");
	pszDest->Append("\t\tsta [D + 2]\n");
	pszDest->Append("\t\tbra StartInitialise\n");
	pszDest->Append("StartMain:\n");
	pszDest->Append("\t\tjsl _main\n");
	pszDest->Append("\t\tpha\n");
	pszDest->Append("\t\t.weak ___RunAtExit\n");
	pszDest->Append("\t\tlda ___RunAtExit & 0xFFFF\n");
	pszDest->Append("\t\tora ___RunAtExit >> 16\n");
	pszDest->Append("\t\tbeq StartAtExitDone\n");
	pszDest->Append("\t\tjsl ___RunAtExit\n");
	pszDest->Append("StartAtExitDone:\n");
	pszDest->Append("\t\tlda __fini_end & 0xFFFF\n");
	pszDest->Append("\t\tsta [D + 0]\n");
	pszDest->Append("\t\tlda __fini_end >> 16\n");
	pszDest->Append("\t\tsta [D + 2]\n");
	pszDest->Append("StartFinalise:\n");
	pszDest->Append("\t\tlda [D + 0]\n");
	pszDest->Append("\t\tcmp __fini_start & 0xFFFF\n");
	pszDest->Append("\t\tbne StartFinaliseCall\n");
	pszDest->Append("\t\tlda [D + 2]\n");
	pszDest->Append("\t\tcmp __fini_start >> 16\n");
	pszDest->Append("\t\tbeq StartStop\n");
	pszDest->Append("StartFinaliseCall:\n");
	pszDest->Append("\t\tsec\n");
	pszDest->Append("\t\tlda [D + 0]\n");
	pszDest->Append("\t\tsbc 4\n");
	pszDest->Append("\t\tsta [D + 0]\n");
	pszDest->Append("\t\tlda [D + 2]\n");
	pszDest->Append("\t\tsbc 0\n");
	pszDest->Append("\t\tsta [D + 2]\n");
	pszDest->Append("\t\tjsr StartCallEntry\n");
	pszDest->Append("\t\tbra StartFinalise\n");
	pszDest->Append("StartStop:\n");
	pszDest->Append("\t\tpla\n");
	pszDest->Append("\t\tstp\n");

	//Calls the routine the table entry [D + 0] points at: the return address and the routine - 1 are pushed, and rtl
	//goes to it.
	pszDest->Append("StartCallEntry:\n");
	pszDest->Append("\t\t.width (A, X), 16\n");
	pszDest->Append("\t\tldy 0\n");
	pszDest->Append("\t\tlda [[D + 0]:24 + Y]\n");
	pszDest->Append("\t\tsta [D + 4]\n");
	pszDest->Append("\t\tldy 2\n");
	pszDest->Append("\t\tlda [[D + 0]:24 + Y]\n");
	pszDest->Append("\t\tsta [D + 6]\n");
	pszDest->Append("\t\tphk\n");
	pszDest->Append("\t\tper StartCallReturn - 1\n");
	pszDest->Append("\t\tsep m\n");
	pszDest->Append("\t\tlda [D + 6]\n");
	pszDest->Append("\t\tpha\n");
	pszDest->Append("\t\trep m\n");
	pszDest->Append("\t\tlda [D + 4]\n");
	pszDest->Append("\t\tdec A\n");
	pszDest->Append("\t\tpha\n");
	pszDest->Append("\t\trtl\n");
	pszDest->Append("StartCallReturn:\n");
	pszDest->Append("\t\trts\n");
	//_Exit's stack entry, and its register entry (iStatus in A, as the compiler's callers pass it).
	pszDest->Append("\t\t.global __Exit, __Exit.a\n");
	pszDest->Append("__Exit:\n");
	pszDest->Append("\t\t.width (A, X), 16\n");
	pszDest->Append("\t\tlda [S + 4]\n");
	pszDest->Append("__Exit.a:\n");
	pszDest->Append("\t\tstp\n");
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The tables the W65C816 back end's code reads (R816_STable7 ...), each a
// COMDAT section so only one copy is linked.  The R816_ routines it calls are
// Libc65's (CCPP65Builder::AddRuntime).
//
//////////////////////////////////////////////////////////////////////////
void CCPPTargetW65C816::AppendRuntime(CCPPRuntimeWriter* pcWriter)
{
	CArrayChars*	paszRuntime;
	CArrayChars		aszRoutines;
	CCPPErrors		cErrors;
	size			i;

	paszRuntime = pcWriter->GetRuntimeCalls();
	aszRoutines.Init();
	for (i = 0; i < paszRuntime->NumElements(); i++)
	{
		if (strncmp(paszRuntime->GetText(i), "R816_", 5) == 0)
		{
			aszRoutines.Add(paszRuntime->GetText(i));
		}
	}
	if (aszRoutines.NumElements() != 0)
	{
		cErrors.Init();
		if (!CIR816Backend::GenerateRuntime(&aszRoutines, pcWriter->GetRuntimeCode(), &cErrors))
		{
			pcWriter->RuntimeError(cErrors.Text());
		}
		cErrors.Kill();
	}
	aszRoutines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// szOpcode of a word on the stack or the direct page, as lda [S + 4].
//
//////////////////////////////////////////////////////////////////////////
static void AppendThunkWord(CChars* pszThunks, const char* szOpcode, const char* szBase, int iOffset)
{
	pszThunks->Append("\t\t")->Append(szOpcode)->Append(" [")->Append(szBase)->Append(" + ")->Append(iOffset)->Append("]\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Loads into A the word at Y from the long pointer on the direct page at
// iPointer, Y being iOffset.
//
//////////////////////////////////////////////////////////////////////////
static void AppendThunkPointerLoad(CChars* pszThunks, int iPointer, int iOffset)
{
	pszThunks->Append("\t\tldy ")->Append(iOffset)->AppendNewLine();
	pszThunks->Append("\t\tlda [[D + ")->Append(iPointer)->Append("]:24 + Y]\n");
}


//////////////////////////////////////////////////////////////////////////
//
// this is above the return address (CPP65_816_ADJUST_THIS).
//
//////////////////////////////////////////////////////////////////////////
void CCPPTargetW65C816::AppendAdjustThunk(CChars* pszDest, int iAdjust, const char* szFunction)
{
	pszDest->Append("\t\t.width (A, X), 16\n");
	pszDest->Append("\t\tclc\n");
	AppendThunkWord(pszDest, "lda", "S", CPP65_816_ADJUST_THIS);
	pszDest->Append("\t\tadc 0x")->Append((uint32)iAdjust & 0xFFFF, 16)->AppendNewLine();
	AppendThunkWord(pszDest, "sta", "S", CPP65_816_ADJUST_THIS);
	AppendThunkWord(pszDest, "lda", "S", CPP65_816_ADJUST_THIS + CPP65_816_WORD_BYTES);
	pszDest->Append("\t\tadc 0x")->Append(((uint32)iAdjust >> 16) & 0xFFFF, 16)->AppendNewLine();
	AppendThunkWord(pszDest, "sta", "S", CPP65_816_ADJUST_THIS + CPP65_816_WORD_BYTES);
	pszDest->Append("\t\tjml ");
	pszDest->Append(szFunction);
	pszDest->AppendNewLine();
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// On the frame CPP65_816_VCALL_ names.
// The function's address - 1 is put where the caller's D was, D restored,
// the bytes below it dropped and rtl jumps to the function with the stack as
// the caller left it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTargetW65C816::AppendVirtualCallThunk(CChars* pszDest, int iVtableOffset, int iEntryOffset)
{
	pszDest->Append("\t\t.width (A, X), 16\n");
	pszDest->Append("\t\tphd\n");
	pszDest->Append("\t\tpha\n");
	pszDest->Append("\t\tpha\n");
	pszDest->Append("\t\ttsc\n");
	pszDest->Append("\t\ttcd\n");
	AppendThunkPointerLoad(pszDest, CPP65_816_VCALL_THIS, iVtableOffset);
	AppendThunkWord(pszDest, "sta", "D", CPP65_816_VCALL_VTABLE);
	AppendThunkPointerLoad(pszDest, CPP65_816_VCALL_THIS, iVtableOffset + CPP65_816_WORD_BYTES);
	AppendThunkWord(pszDest, "sta", "D", CPP65_816_VCALL_VTABLE + CPP65_816_WORD_BYTES);
	AppendThunkPointerLoad(pszDest, CPP65_816_VCALL_VTABLE, iEntryOffset + CPP65_816_WORD_BYTES);
	pszDest->Append("\t\ttax\n");
	AppendThunkPointerLoad(pszDest, CPP65_816_VCALL_VTABLE, iEntryOffset);
	pszDest->Append("\t\tdec A\n");
	AppendThunkWord(pszDest, "sta", "D", CPP65_816_VCALL_VTABLE);
	AppendThunkWord(pszDest, "lda", "D", CPP65_816_VCALL_CALLER_D);
	pszDest->Append("\t\ttay\n");
	AppendThunkWord(pszDest, "lda", "D", CPP65_816_VCALL_VTABLE);
	AppendThunkWord(pszDest, "sta", "D", CPP65_816_VCALL_TARGET);
	pszDest->Append("\t\ttxa\n");
	pszDest->Append("\t\tsep m\n");
	AppendThunkWord(pszDest, "sta", "D", CPP65_816_VCALL_TARGET_BANK);
	pszDest->Append("\t\tpla\n");
	pszDest->Append("\t\trep m\n");
	pszDest->Append("\t\tpla\n");
	pszDest->Append("\t\ttya\n");
	pszDest->Append("\t\ttcd\n");
	pszDest->Append("\t\trtl\n");
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Preferring size, only a function no bigger than its call is inlined: an
// accessor, or one that passes a call on.  A function that only passes
// a call on is a jump either way.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTargetW65C816::GetInlineLimits(SIRInlineLimits* psLimits, CCPPCompileOptions* pcOptions)
{
	psLimits->uiInstructions = CPP816_INLINE_INSTRUCTIONS;
	psLimits->uiLoopInstructions = CPP816_INLINE_LOOP_INSTRUCTIONS;
	psLimits->uiHotInstructions = CPP816_INLINE_HOT_INSTRUCTIONS;
	psLimits->uiCalls = CPP816_INLINE_CALLS;
	psLimits->bForwardersJump = true;
	if (pcOptions->PrefersSize())
	{
		psLimits->uiInstructions = CPP816_INLINE_SIZE_INSTRUCTIONS;
		psLimits->uiLoopInstructions = CPP816_INLINE_SIZE_INSTRUCTIONS;
		psLimits->uiHotInstructions = CPP816_INLINE_SIZE_INSTRUCTIONS;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816 runs every pass (CCPPTarget::BuildPipeline): its multiplies and
// counters are narrowed, divisions by constants are multiplies and signed
// indices are biased.  It has no indexed addresses, so they are not made for
// it, and its divide routines leave the remainder at D + 5.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTargetW65C816::ConfigureOptimiser(CIROptimiser* pcOptimiser)
{
	pcOptimiser->SetIndexedAddresses(false);
	pcOptimiser->SetConstantDivisions(true);
	pcOptimiser->SetDivideRemainders(IR_DIVIDE_REMAINDERS_16 | IR_DIVIDE_REMAINDERS_32 | IR_DIVIDE_REMAINDERS_64 | IR_DIVIDE_REMAINDERS_SIGNED);
}


//////////////////////////////////////////////////////////////////////////
//
// The measured functions also say which frames are worth a page.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTargetW65C816::UsesProfile(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBackend* CCPPTargetW65C816::NewBackend(void)
{
	return NewMalloc<CIR816Backend>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTargetW65C816::Assemble(CAsm65Source* pcSource, CAsm65Object* pcObject, const char* szDirectory, const char* szSourceFile, CChars* pszErrors)
{
	CAsm65816Assembler	cAssembler;
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


//////////////////////////////////////////////////////////////////////////
//
// The back end's and the assembler's indexes of their mnemonics, which the
// units only look up.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTargetW65C816::PrepareForThreads(void)
{
	CIR816Backend::IndexMnemonics();
	CAsm65816Assembler::IndexMnemonics();
}

