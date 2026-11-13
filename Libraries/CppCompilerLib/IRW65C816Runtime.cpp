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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IRW65C816BackendInternal.h"


//The W65C816 back end's runtime routines (R816_...): they are all in assembly, this says how calls find them and
//take their operands; the small tables the compiled code reads are written here (GenerateRuntime).


//The runtime routines in assembly are Libc65's IRW65C816Runtime-Common.asm (divides, shifts, the 64 bit multiply and
//BCD) and IRW65C816Runtime-MathTables.asm or IRW65C816Runtime-MathSafe.asm (the 16 and 32 bit multiplies, with Garth
//Wilson's product table MathMult or without it), linked as libraries by CCPP65Builder::AddRuntime.  The
//multiplies and the 16 and 32 bit unsigned divides run on the caller's D with a at D + 15 and b at D + 19
//(IR816ScratchRoutine), using the caller's D + 5 to D + 22; the 64 bit ones' arguments are pushed (last first, after
//the jsl's return address) and they point D at the stack.  A 16 bit result is left in A, a 32 bit one in A (low) and
//X (high).  The files say how each works.
struct SIR816Routine
{
	const char*	szName;
	bool		bScratch;	//Takes its operands in the caller's scratch bytes (IR816ScratchRoutine), not pushed.
	bool		bIndexed;	//Works on the caller's frame at D + X (the 64 bit shifts): it may read any of it.
};


static const SIR816Routine gasIR816Routines[] =
{
	{ "R816_UDiv64", false, false },
	{ "R816_URem64", false, false },
	{ "R816_SDiv64", false, false },
	{ "R816_SRem64", false, false },
	{ "R816_UDiv32", true, false },
	{ "R816_URem32", true, false },
	{ "R816_UDiv16", true, false },
	{ "R816_URem16", true, false },
	{ "R816_SDiv32", true, false },
	{ "R816_SRem32", true, false },
	{ "R816_SDiv16", true, false },
	{ "R816_SRem16", true, false },
	{ "R816_Mul16", true, false },
	{ "R816_Mul32", true, false },
	{ "R816_Mul64", false, false },
	{ "R816_UMulWide32", true, false },
	{ "R816_Shl64", false, true },
	{ "R816_LShr64", false, true },
	{ "R816_AShr64", false, true },
	{ "R816_Shl32", true, false },
	{ "R816_LShr32", true, false },
	{ "R816_AShr32", true, false },
	{ "R816_BcdAdd", false, false },
	{ "R816_BcdSubtract", false, false },
	{ "R816_Clz8", false, false },			//A table, which the 64 bit counts read too (CCPP65IRFrontEnd::CountTable).
	{ "R816_Ctz8", false, false },
	{ "R816_Clz16", false, false },
	{ "R816_Clz32", false, false },
	{ "R816_Ctz16", false, false },
	{ "R816_Ctz32", false, false },
	{ NULL, false, false }
};


//////////////////////////////////////////////////////////////////////////
//
// Whether szName starts with szPrefix.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816StartsWith(const char* szName, const char* szPrefix)
{
	return strncmp(szName, szPrefix, StrLen(szPrefix)) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The assembly runtime routine named szName (gasIR816Routines), or NULL.
//
//////////////////////////////////////////////////////////////////////////
static const SIR816Routine* IR816FindRoutine(const char* szName)
{
	size	ui;

	for (ui = 0; gasIR816Routines[ui].szName; ui++)
	{
		if (StringCompare(gasIR816Routines[ui].szName, szName) == 0)
		{
			return &gasIR816Routines[ui];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the routine a jsl's operand (uiOperand characters) names works
// on the caller's frame at D + X (SIR816Routine::bIndexed): it may read
// and write any of it.
//
//////////////////////////////////////////////////////////////////////////
bool IR816IndexedRoutine(const char* szOperand, size uiOperand)
{
	size	ui;

	for (ui = 0; gasIR816Routines[ui].szName; ui++)
	{
		if (gasIR816Routines[ui].bIndexed && (strlen(gasIR816Routines[ui].szName) == uiOperand) && (strncmp(gasIR816Routines[ui].szName, szOperand, uiOperand) == 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A runtime routine's or table's start: its own COMDAT section (each unit
// that uses it writes it), in szSection ("text." or "rodata."), and its
// global label.
//
//////////////////////////////////////////////////////////////////////////
static void IR816RuntimeHeader(CChars* pszDest, const char* szSection, const char* szName)
{
	pszDest->Append("\t\t.section ")->Append(szSection)->Append(szName)->Append(", comdat\n");
	pszDest->Append("\t\t.global ")->Append(szName)->AppendNewLine();
	pszDest->Append(szName)->Append(":\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the runtime routine szName takes its operands in the caller's
// scratch bytes (the assembly ones of 16 and 32 bits) rather than pushed.
//
//////////////////////////////////////////////////////////////////////////
bool IR816ScratchRoutine(const char* szName)
{
	const SIR816Routine*	psRoutine;

	psRoutine = IR816FindRoutine(szName);
	return psRoutine && psRoutine->bScratch;
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime routine RuntimeCall calls for psInstruction (mul, udiv, sdiv,
// urem or srem), into pszName (Init'd here).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::RuntimeName(CChars* pszName, SIRInstruction* psInstruction)
{
	pszName->Init(IR816_RUNTIME_PREFIX);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Mul:	pszName->Append("Mul");	break;
	case IRO_UDiv:	pszName->Append("UDiv");	break;
	case IRO_SDiv:	pszName->Append("SDiv");	break;
	case IRO_URem:	pszName->Append("URem");	break;
	default:		pszName->Append("SRem");	break;
	}
	pszName->Append(psInstruction->sType.Is(IRTK_I8) ? 16 : psInstruction->sType.Bits());
}


//////////////////////////////////////////////////////////////////////////
//
// The table a shift by a variable count reads (szName is R816_Bits<n>, for
// n bits): 1 << k in n / 16 words for each k from 0 to 63, 0 past the width.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::AppendBitsTable(CChars* pszDest, const char* szName)
{
	int		iBits;
	int		i;
	int		iWord;
	uint64	ulliBit;

	iBits = atoi(&szName[IR816_BITS_TABLE_LENGTH]);
	IR816RuntimeHeader(pszDest, "rodata.", szName);
	for (i = 0; i < 64; i++)
	{
		ulliBit = (i < iBits) ? ((uint64)1 << i) : 0;
		pszDest->Append("\t\t.word ");
		for (iWord = 0; iWord < iBits / 16; iWord++)
		{
			if (iWord > 0)
			{
				pszDest->Append(", ");
			}
			pszDest->Append((int)((ulliBit >> (16 * iWord)) & 0xFFFF));
		}
		pszDest->AppendNewLine();
	}
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The table TableDivide reads (szName is R816_STable<c> or R816_UTable<c>):
// for each byte b, b / c and b % c (b signed for STable) as a word, q in
// its low byte.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::AppendDivideTable(CChars* pszDest, const char* szName)
{
	int		iDivisor;
	int		iValue;
	int		iQuotient;
	int		iRemainder;
	int		i;
	bool	bSigned;

	bSigned = IR816StartsWith(szName, IR816_SIGNED_TABLE);
	iDivisor = atoi(&szName[IR816_TABLE_LENGTH]);
	IR816RuntimeHeader(pszDest, "rodata.", szName);
	for (i = 0; i < 256; i++)
	{
		iValue = (bSigned && (i >= 0x80)) ? i - 0x100 : i;
		iQuotient = iValue / iDivisor;
		iRemainder = iValue % iDivisor;
		if ((i % 8) == 0)
		{
			pszDest->Append("\t\t.word ");
		}
		pszDest->Append(((iRemainder & 0xFF) << 8) | (iQuotient & 0xFF));
		pszDest->Append(((i % 8) == 7) ? "\n" : ", ");
	}
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the tables named in paszRuntime that TableDivide and the shifts
// read (R816_STable<c>, R816_UTable<c>, R816_Bits<n>).  Every other R816_
// routine is in assembly, linked from Libc65 (CCPP65Builder::AddRuntime;
// IR816FindRoutine); any other name is an error.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::GenerateRuntime(CArrayChars* paszRuntime, CChars* pszDest, CCPPErrors* pcErrors)
{
	size	i;
	char*	szName;
	bool	bResult;

	//Only the R816_ names are the back end's (the soft float functions are Libc65's).
	bResult = true;
	for (i = 0; i < paszRuntime->NumElements(); i++)
	{
		szName = paszRuntime->GetText(i);
		if (!IR816StartsWith(szName, IR816_RUNTIME_PREFIX) || IR816FindRoutine(szName))
		{
			continue;
		}
		if (IR816StartsWith(szName, IR816_SIGNED_TABLE) || IR816StartsWith(szName, IR816_UNSIGNED_TABLE))
		{
			AppendDivideTable(pszDest, szName);
		}
		else if (IR816StartsWith(szName, IR816_BITS_TABLE))
		{
			AppendBitsTable(pszDest, szName);
		}
		else
		{
			pcErrors->Add("runtime", -1, -1, -1, "No such W65C816 runtime routine", szName);
			bResult = false;
		}
	}
	return bResult;
}

