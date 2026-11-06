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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IRParser.h"
#include "IR816BackendInternal.h"


//The W65C816 back end's runtime routines (R816_...): which are in assembly and how calls find them, and those
//generated from IR (GenerateRuntime) with the tables they use.


//The runtime routines in assembly are Libc65's IR816Runtime-Common.asm (divides, shifts, the 64 bit multiply and
//BCD) and IR816Runtime-MathTables.asm or IR816Runtime-MathSafe.asm (the 16 and 32 bit multiplies, with Garth
//Wilson's product table MathMult or without it), linked as libraries by CCPP65Builder::AddRuntime816.  The
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
	{ "R816_UDiv32", true, false },
	{ "R816_URem32", true, false },
	{ "R816_UDiv16", true, false },
	{ "R816_URem16", true, false },
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
	{ NULL, false, false }
};


//The signed divide and remainder, as IR for each width ($T the type, $NAME the routine, R816_$NAME): they work on
//the magnitudes and call the unsigned ones in assembly ($UOP).
static const char* gszIR816Signed =
	"function $T @R816_$NAME($T %a, $T %b)\n{\n"
	"entry:\n\t%an = slt $T %a, 0\n\tbrif %an, negA, posA\n"
	"negA:\n\t%na = sub $T 0, %a\n\tjmp gotA\n"
	"posA:\n\tjmp gotA\n"
	"gotA:\n\t%ua = phi $T [negA, %na], [posA, %a]\n\t%bn = slt $T %b, 0\n\tbrif %bn, negB, posB\n"
	"negB:\n\t%nb = sub $T 0, %b\n\tjmp gotB\n"
	"posB:\n\tjmp gotB\n"
	"gotB:\n\t%ub = phi $T [negB, %nb], [posB, %b]\n\t%u = $UOP $T %ua, %ub\n"
	"\t%s = $SIGN\n\tbrif %s, neg, pos\n"
	"neg:\n\t%nu = sub $T 0, %u\n\tret $T %nu\n"
	"pos:\n\tret $T %u\n}\n\n";


//The 64 bit signed divide and remainder: as gszIR816Signed, but when both magnitudes fit in 32 bits (file positions
//and sizes nearly always do) the 32 bit unsigned routine works it out, much faster than the 64 bit loop.
static const char* gszIR816Signed64 =
	"function i64 @R816_$NAME(i64 %a, i64 %b)\n{\n"
	"entry:\n\t%an = slt i64 %a, 0\n\tbrif %an, negA, posA\n"
	"negA:\n\t%na = sub i64 0, %a\n\tjmp gotA\n"
	"posA:\n\tjmp gotA\n"
	"gotA:\n\t%ua = phi i64 [negA, %na], [posA, %a]\n\t%bn = slt i64 %b, 0\n\tbrif %bn, negB, posB\n"
	"negB:\n\t%nb = sub i64 0, %b\n\tjmp gotB\n"
	"posB:\n\tjmp gotB\n"
	"gotB:\n\t%ub = phi i64 [negB, %nb], [posB, %b]\n\t%w = or i64 %ua, %ub\n\t%hi = lshr i64 %w, 32\n\t%narrow = eq i64 %hi, 0\n\tbrif %narrow, short, long\n"
	"short:\n\t%sa = trunc i64 %ua to i32\n\t%sb = trunc i64 %ub to i32\n\t%su = $UOP i32 %sa, %sb\n\t%sw = zext i32 %su to i64\n\tjmp got\n"
	"long:\n\t%lu = $UOP i64 %ua, %ub\n\tjmp got\n"
	"got:\n\t%u = phi i64 [short, %sw], [long, %lu]\n"
	"\t%s = $SIGN\n\tbrif %s, neg, pos\n"
	"neg:\n\t%nu = sub i64 0, %u\n\tret i64 %nu\n"
	"pos:\n\tret i64 %u\n}\n\n";


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
void CIR816Backend::RuntimeName(CChars* pszName, SIRInstruction* psInstruction)
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
// The table TableDivide reads (szName is R816_STable<c> or R816_UTable<c>):
// for each byte b, b / c and b % c (b signed for STable) as a word, q in
// its low byte.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::AppendBitsTable(CChars* pszDest, const char* szName)
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
void CIR816Backend::AppendDivideTable(CChars* pszDest, const char* szName)
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
// Generates the runtime routines named in paszRuntime that aren't in assembly (IR816FindRoutine): the signed
// divides from their IR, and the tables TableDivide and the shifts read.  Any other name is an error.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::GenerateRuntime(CArrayChars* paszRuntime, int* piLabels, CChars* pszDest, CCPPErrors* pcErrors)
{
	CArrayChars		aszNames;
	CChars			szIR;
	CChars			szText;
	CChars			szType;
	CIRModule		cModule;
	CIRParser		cParser;
	CIR816Backend			cBackend;
	size					i;
	char*					szName;
	const char*				szOperation;
	int						iBits;
	bool					bResult;
	bool					bDivide;

	//Only the R816_ routines are made here (the soft float functions are Libc65's).
	aszNames.Init();
	for (i = 0; i < paszRuntime->NumElements(); i++)
	{
		if (IR816StartsWith(paszRuntime->GetText(i), IR816_RUNTIME_PREFIX))
		{
			aszNames.Add(paszRuntime->GetText(i));
		}
	}
	szIR.Init();
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		szName = aszNames.GetText(i);

		//The routines in assembly are linked from Libc65 (CCPP65Builder::AddRuntime816); the divide and bit tables are
		//written here.
		if (IR816FindRoutine(szName))
		{
			continue;
		}
		if (IR816StartsWith(szName, IR816_SIGNED_TABLE) || IR816StartsWith(szName, IR816_UNSIGNED_TABLE))
		{
			AppendDivideTable(pszDest, szName);
			continue;
		}
		if (IR816StartsWith(szName, IR816_BITS_TABLE))
		{
			AppendBitsTable(pszDest, szName);
			continue;
		}

		//The signed divides from IR: szOperation is SDiv or SRem, then the bits.  Every other routine is in assembly.
		szOperation = &szName[IR816_RUNTIME_PREFIX_LENGTH];
		if (!IR816StartsWith(szOperation, "SDiv") && !IR816StartsWith(szOperation, "SRem"))
		{
			pcErrors->Add("runtime", -1, -1, -1, "No such W65C816 runtime routine", szName);
			continue;
		}
		bDivide = szOperation[1] == 'D';
		iBits = atoi(&szName[StrLen(szName) - 2]);
		szType.Init("i");
		szType.Append(iBits);
		szText.Init((iBits == 64) ? gszIR816Signed64 : gszIR816Signed);
		szText.Replace("$UOP", bDivide ? "udiv" : "urem");
		szText.Replace("$SIGN", bDivide ? "ne i1 %an, %bn" : "copy i1 %an");
		szText.Replace("$NAME", szOperation);
		szText.Replace("$T", szType.Text());
		szIR.Append(szText);
		szText.Kill();
		szType.Kill();
	}
	aszNames.Kill();

	cModule.Init();
	cModule.AddSpace("default", 4);
	cParser.Init(&cModule, pcErrors, "runtime");
	bResult = cParser.Parse(szIR.Text());
	cParser.Kill();
	szIR.Kill();
	if (bResult)
	{
		//Each unit that uses a routine writes it, so it is COMDAT.
		cBackend.Init(&cModule, piLabels);
		cBackend.mszSectionOptions = ", comdat";
		bResult = cBackend.GenerateModule(pszDest, pcErrors);
		cBackend.Kill();
	}
	cModule.Kill();
	return bResult;
}

