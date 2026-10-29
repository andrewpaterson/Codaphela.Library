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
#include "IRLoops.h"
#include "IR816Backend.h"


#define IR816_KEEP_IN_A	" ;A"		//Ends a store Peephole may leave the value in A for (Store).


//The runtime routines, as IR for each width: $T is the type, $N its bits, $M its bits - 1 and $NAME the routine
//(R816_$NAME).  Multiply adds and shifts; divide is restoring division a bit at a time (a remainder that shifts
//past the top is bigger than any divisor); signed divide and remainder work on the magnitudes.
//The unsigned 16 and 32 bit runtime routines in assembly (the IR ones are much slower).  Multiply adds byte products
//from a table (IR816_MULTIPLY_BYTES) and skips those of a high half that is 0.  The arguments are a at S + 4
//and b after it (pushed last first, after the jsl's return address); they are worked on where they are.  Space for
//the result is pushed, then D is pointed at the stack: D + 1 is the caller's D, then the result, the return address
//and the arguments.  A 16 bit result is left in A, a 32 bit one in A (low) and X (high).
struct SIR816Routine
{
	const char*	szName;
	const char*	szText;
};


//A 32 bit divide with both operands under 65536 runs 16 steps with the remainder in A (a step that carries out of A
//always subtracts: the remainder was over the divisor); a dividend below the divisor (the quotient is 0) or a divisor
//of 1 is the answer at once, and a dividend under 256 starts a byte up (xba) and takes 8 steps.  A divisor under 65536 keeps the remainder in A too: 16 steps
//from the dividend's high word as the remainder when that is below the divisor (the quotient's high word is 0),
//else the high word's 16 steps then the low word's, the remainder carried in A between them.  A dividend under 65536
//with a wider divisor is a quotient of 0.  Otherwise 32 steps on the remainder at
//D + 3 and D + 5.
#define IR816_DIVIDE_32(NAME, RESULT_LOW, RESULT_HIGH) \
	"\t\t.width (A, X), 16\n" \
	"\t\tpea 0\n\t\tpea 0\n\t\tphd\n\t\ttsc\n\t\ttcd\n" \
	"\t\tlda [D + 12]\n\t\tora [D + 16]\n\t\tbne " NAME "_Wide\n" \
	"\t\tlda [D + 10]\n\t\tcmp [D + 14]\n\t\tbcs " NAME "_Divides\n" \
	"\t\tsta [D + 3]\n\t\tstz [D + 10]\n\t\tbrl " NAME "_Done\n" \
	NAME "_Divides:\n" \
	"\t\tldx [D + 14]\n\t\tdex\n\t\tbne " NAME "_NotOne\n\t\tbrl " NAME "_Done\n" \
	NAME "_NotOne:\n" \
	"\t\tcmp 256\n\t\tbcs " NAME "_Full16\n" \
	"\t\txba\n\t\tsta [D + 10]\n\t\tldx 8\n\t\tlda 0\n\t\tbra " NAME "_Bit16\n" \
	NAME "_Full16:\n" \
	"\t\tldx 16\n\t\tlda 0\n" \
	NAME "_Bit16:\n" \
	"\t\tasl [D + 10]\n\t\trol A\n\t\tbcs " NAME "_Over16\n" \
	"\t\tcmp [D + 14]\n\t\tbcc " NAME "_Next16\n" \
	NAME "_Over16:\n" \
	"\t\tsbc [D + 14]\n\t\tinc [D + 10]\n" \
	NAME "_Next16:\n" \
	"\t\tdex\n\t\tbne " NAME "_Bit16\n" \
	"\t\tsta [D + 3]\n\t\tbra " NAME "_Done\n" \
	NAME "_Wide:\n" \
	"\t\tlda [D + 16]\n\t\tbne " NAME "_WideDivisor\n" \
	"\t\tlda [D + 12]\n\t\tcmp [D + 14]\n\t\tbcs " NAME "_Mid\n" \
	"\t\tstz [D + 12]\n\t\tldx 16\n\t\tbra " NAME "_Bit16\n" \
	NAME "_Mid:\n" \
	"\t\tldx 16\n\t\tlda 0\n" \
	NAME "_High:\n" \
	"\t\tasl [D + 12]\n\t\trol A\n\t\tbcs " NAME "_OverHigh\n" \
	"\t\tcmp [D + 14]\n\t\tbcc " NAME "_NextHigh\n" \
	NAME "_OverHigh:\n" \
	"\t\tsbc [D + 14]\n\t\tinc [D + 12]\n" \
	NAME "_NextHigh:\n" \
	"\t\tdex\n\t\tbne " NAME "_High\n" \
	"\t\tldx 16\n\t\tbrl " NAME "_Bit16\n" \
	NAME "_WideDivisor:\n" \
	"\t\tlda [D + 12]\n\t\tbne " NAME "_Long\n" \
	"\t\tlda [D + 10]\n\t\tsta [D + 3]\n\t\tstz [D + 10]\n\t\tbra " NAME "_Done\n" \
	NAME "_Long:\n" \
	"\t\tldx 32\n" \
	NAME "_Bit:\n" \
	"\t\tasl [D + 10]\n\t\trol [D + 12]\n\t\trol [D + 3]\n\t\trol [D + 5]\n" \
	"\t\tbcs " NAME "_Over\n" \
	"\t\tsec\n\t\tlda [D + 3]\n\t\tsbc [D + 14]\n\t\ttay\n\t\tlda [D + 5]\n\t\tsbc [D + 16]\n" \
	"\t\tbcc " NAME "_Next\n" \
	"\t\tbra " NAME "_Keep\n" \
	NAME "_Over:\n" \
	"\t\tsec\n\t\tlda [D + 3]\n\t\tsbc [D + 14]\n\t\ttay\n\t\tlda [D + 5]\n\t\tsbc [D + 16]\n" \
	NAME "_Keep:\n" \
	"\t\tsta [D + 5]\n\t\tsty [D + 3]\n\t\tinc [D + 10]\n" \
	NAME "_Next:\n" \
	"\t\tdex\n\t\tbne " NAME "_Bit\n" \
	NAME "_Done:\n" \
	"\t\tlda [D + " RESULT_HIGH "]\n\t\ttax\n\t\tlda [D + " RESULT_LOW "]\n\t\ttay\n" \
	"\t\tpld\n\t\tpla\n\t\tpla\n\t\ttya\n\t\trtl\n"

//16 steps with the remainder in A, as the 32 bit divide's fast path.
#define IR816_DIVIDE_16(NAME, RESULT) \
	"\t\t.width (A, X), 16\n" \
	"\t\tpea 0\n\t\tphd\n\t\ttsc\n\t\ttcd\n" \
	"\t\tldx 16\n\t\tlda 0\n" \
	NAME "_Bit:\n" \
	"\t\tasl [D + 8]\n\t\trol A\n\t\tbcs " NAME "_Over\n" \
	"\t\tcmp [D + 10]\n\t\tbcc " NAME "_Next\n" \
	NAME "_Over:\n" \
	"\t\tsbc [D + 10]\n\t\tinc [D + 8]\n" \
	NAME "_Next:\n" \
	"\t\tdex\n\t\tbne " NAME "_Bit\n" \
	"\t\tsta [D + 3]\n" \
	"\t\tlda [D + " RESULT "]\n\t\ttay\n" \
	"\t\tpld\n\t\tpla\n\t\ttya\n\t\trtl\n"

//The 64 bit divide: D + 3 to D + 10 is the remainder, then the caller's return address, the hidden result pointer at
//D + 14, the dividend (shifted into the quotient) at D + 18 and the divisor at D + 26.  Zero words at the top of the
//dividend are skipped 16 steps at a time (the remainder stays 0 through them); the result is written through the
//pointer.
#define IR816_DIVIDE_64(NAME, RESULT) \
	"\t\t.width (A, X), 16\n" \
	"\t\tpea 0\n\t\tpea 0\n\t\tpea 0\n\t\tpea 0\n\t\tphd\n\t\ttsc\n\t\ttcd\n" \
	"\t\tldx 64\n" \
	NAME "_Skip:\n" \
	"\t\tlda [D + 24]\n\t\tbne " NAME "_Bit\n\t\tcpx 16\n\t\tbeq " NAME "_Bit\n" \
	"\t\tlda [D + 22]\n\t\tsta [D + 24]\n\t\tlda [D + 20]\n\t\tsta [D + 22]\n\t\tlda [D + 18]\n\t\tsta [D + 20]\n\t\tstz [D + 18]\n" \
	"\t\ttxa\n\t\tsec\n\t\tsbc 16\n\t\ttax\n\t\tbra " NAME "_Skip\n" \
	NAME "_Bit:\n" \
	"\t\tasl [D + 18]\n\t\trol [D + 20]\n\t\trol [D + 22]\n\t\trol [D + 24]\n" \
	"\t\trol [D + 3]\n\t\trol [D + 5]\n\t\trol [D + 7]\n\t\trol [D + 9]\n" \
	"\t\tbcs " NAME "_Subtract\n" \
	"\t\tlda [D + 9]\n\t\tcmp [D + 32]\n\t\tbcc " NAME "_Next\n\t\tbne " NAME "_Subtract\n" \
	"\t\tlda [D + 7]\n\t\tcmp [D + 30]\n\t\tbcc " NAME "_Next\n\t\tbne " NAME "_Subtract\n" \
	"\t\tlda [D + 5]\n\t\tcmp [D + 28]\n\t\tbcc " NAME "_Next\n\t\tbne " NAME "_Subtract\n" \
	"\t\tlda [D + 3]\n\t\tcmp [D + 26]\n\t\tbcc " NAME "_Next\n" \
	NAME "_Subtract:\n" \
	"\t\tsec\n" \
	"\t\tlda [D + 3]\n\t\tsbc [D + 26]\n\t\tsta [D + 3]\n\t\tlda [D + 5]\n\t\tsbc [D + 28]\n\t\tsta [D + 5]\n" \
	"\t\tlda [D + 7]\n\t\tsbc [D + 30]\n\t\tsta [D + 7]\n\t\tlda [D + 9]\n\t\tsbc [D + 32]\n\t\tsta [D + 9]\n" \
	"\t\tinc [D + 18]\n" \
	NAME "_Next:\n" \
	"\t\tdex\n\t\tbne " NAME "_Bit\n" \
	"\t\tldy 0\n\t\tlda [D + " RESULT "]\n\t\tsta [[D + 14]:24 + Y]\n" \
	"\t\tldy 2\n\t\tlda [D + " RESULT " + 2]\n\t\tsta [[D + 14]:24 + Y]\n" \
	"\t\tldy 4\n\t\tlda [D + " RESULT " + 4]\n\t\tsta [[D + 14]:24 + Y]\n" \
	"\t\tldy 6\n\t\tlda [D + " RESULT " + 6]\n\t\tsta [[D + 14]:24 + Y]\n" \
	"\t\tpld\n\t\tpla\n\t\tpla\n\t\tpla\n\t\tpla\n\t\trtl\n"

//Multiply looks bytes up in Garth Wilson's table (Libc65/MathTbls/MathMult.asm): the word at MathMult + ((x << 8) | y)
//* 2 is x * y.  The table is two banks from a bank's start, so the index doubled is X and its top bit (bit 7 of x)
//picks the bank.  A holds (x << 8) | y and gets x * y.
#define IR816_MULTIPLY_BYTES(NAME) \
	"\t\tasl A\n\t\ttax\n\t\tbcs " NAME "_High\n" \
	"\t\tlda [MathMult + X]:24\n\t\tbra " NAME "_Got\n" \
	NAME "_High:\n" \
	"\t\tlda [MathMult + $10000 + X]:24\n" \
	NAME "_Got:\n"

//Adds the product in A a byte up into the word at WORD (only its low byte reaches it).
#define IR816_MULTIPLY_ADD_BYTE_1(WORD) \
	"\t\txba\n\t\tand $FF00\n\t\tclc\n\t\tadc " WORD "\n\t\tsta " WORD "\n"

//R816_Mul32's result is D + 3 to D + 6 (0 to start) and D + 7 is a word for the low byte of a product, then the
//return address, a (bytes D + 12 to D + 15) and b (D + 16 to D + 19).  A 32 bit product is up to ten byte products:
//x and y are the bytes' offsets and ADD puts the product (in A) in the result.  A product of a 0 byte is skipped, as
//are those of a high word that is 0.  A product a byte up adds to the result's middle word and carries into its top
//byte; two bytes up adds to its high word.
#define IR816_MULTIPLY_32(NAME, X, Y, ADD) \
	"\t\tlda [D + " Y "]\n\t\tand $FF\n\t\tbeq R816_Mul32_" NAME "_Zero\n\t\tsta [D + 7]\n" \
	"\t\tlda [D + " X " - 1]\n\t\tand $FF00\n\t\tbeq R816_Mul32_" NAME "_Zero\n\t\tora [D + 7]\n" \
	IR816_MULTIPLY_BYTES("R816_Mul32_" NAME) \
	ADD \
	"R816_Mul32_" NAME "_Zero:\n"

#define IR816_MULTIPLY_ADD_BYTES_1 \
	"\t\tclc\n\t\tadc [D + 4]\n\t\tsta [D + 4]\n\t\tbcc C + 2\n\t\tinc [D + 6]\n"

#define IR816_MULTIPLY_ADD_BYTES_2 \
	"\t\tclc\n\t\tadc [D + 5]\n\t\tsta [D + 5]\n"

static const SIR816Routine gasIR816Routines[] =
{
	{ "R816_UDiv64", IR816_DIVIDE_64("R816_UDiv64", "18") },
	{ "R816_URem64", IR816_DIVIDE_64("R816_URem64", "3") },
	{ "R816_UDiv32", IR816_DIVIDE_32("R816_UDiv32", "10", "12") },
	{ "R816_URem32", IR816_DIVIDE_32("R816_URem32", "3", "5") },
	{ "R816_UDiv16", IR816_DIVIDE_16("R816_UDiv16", "8") },
	{ "R816_URem16", IR816_DIVIDE_16("R816_URem16", "3") },
	{ "R816_Mul16",
		"\t\t.width (A, X), 16\n"
		"\t\tpea 0\n\t\tpea 0\n\t\tphd\n\t\ttsc\n\t\ttcd\n"
		"\t\tlda [D + 10]\n\t\tand $FF\n\t\tsta [D + 5]\n"
		"\t\tlda [D + 12]\n\t\txba\n\t\tand $FF00\n\t\tora [D + 5]\n"
		IR816_MULTIPLY_BYTES("R816_Mul16_LL")
		"\t\tsta [D + 3]\n"
		"\t\tlda [D + 12]\n\t\tand $FF00\n\t\tbeq R816_Mul16_NoHighB\n\t\tora [D + 5]\n"
		IR816_MULTIPLY_BYTES("R816_Mul16_LH")
		IR816_MULTIPLY_ADD_BYTE_1("[D + 3]")
		"R816_Mul16_NoHighB:\n"
		"\t\tlda [D + 10]\n\t\tand $FF00\n\t\tbeq R816_Mul16_Done\n\t\tsta [D + 5]\n"
		"\t\tlda [D + 12]\n\t\tand $FF\n\t\tora [D + 5]\n"
		IR816_MULTIPLY_BYTES("R816_Mul16_HL")
		IR816_MULTIPLY_ADD_BYTE_1("[D + 3]")
		"R816_Mul16_Done:\n"
		"\t\tlda [D + 3]\n\t\ttay\n\t\tpld\n\t\tpla\n\t\tpla\n\t\ttya\n\t\trtl\n" },
	{ "R816_Mul32",
		"\t\t.width (A, X), 16\n"
		"\t\tpea 0\n\t\tpea 0\n\t\tpea 0\n\t\tphd\n\t\ttsc\n\t\ttcd\n"
		IR816_MULTIPLY_32("LL", "12", "16", "\t\tsta [D + 3]\n")
		IR816_MULTIPLY_32("HH", "13", "17", "\t\tsta [D + 5]\n")
		IR816_MULTIPLY_32("LH", "12", "17", IR816_MULTIPLY_ADD_BYTES_1)
		IR816_MULTIPLY_32("HL", "13", "16", IR816_MULTIPLY_ADD_BYTES_1)
		"\t\tlda [D + 18]\n\t\tbeq R816_Mul32_NoHighB\n"
		IR816_MULTIPLY_32("B2", "12", "18", IR816_MULTIPLY_ADD_BYTES_2)
		IR816_MULTIPLY_32("B3", "12", "19", IR816_MULTIPLY_ADD_BYTE_1("[D + 5]"))
		IR816_MULTIPLY_32("B2A1", "13", "18", IR816_MULTIPLY_ADD_BYTE_1("[D + 5]"))
		"R816_Mul32_NoHighB:\n"
		"\t\tlda [D + 14]\n\t\tbeq R816_Mul32_Done\n"
		IR816_MULTIPLY_32("A2", "14", "16", IR816_MULTIPLY_ADD_BYTES_2)
		IR816_MULTIPLY_32("A3", "15", "16", IR816_MULTIPLY_ADD_BYTE_1("[D + 5]"))
		IR816_MULTIPLY_32("A2B1", "14", "17", IR816_MULTIPLY_ADD_BYTE_1("[D + 5]"))
		"R816_Mul32_Done:\n"
		"\t\tlda [D + 5]\n\t\ttax\n\t\tlda [D + 3]\n\t\ttay\n\t\tpld\n\t\tpla\n\t\tpla\n\t\tpla\n\t\ttya\n\t\trtl\n" },
	{ NULL, NULL }
};


static const char* gszIR816Mul =
	"function $T @R816_$NAME($T %a, $T %b)\n{\n"
	"entry:\n\tjmp loop\n"
	"loop:\n\t%r = phi $T [entry, 0], [next, %r2]\n\t%x = phi $T [entry, %a], [next, %x2]\n\t%y = phi $T [entry, %b], [next, %y2]\n"
	"\t%z = eq $T %y, 0\n\tbrif %z, done, body\n"
	"body:\n\t%bit = and $T %y, 1\n\t%odd = ne $T %bit, 0\n\tbrif %odd, add, keep\n"
	"add:\n\t%ra = add $T %r, %x\n\tjmp next\n"
	"keep:\n\tjmp next\n"
	"next:\n\t%r2 = phi $T [add, %ra], [keep, %r]\n\t%x2 = shl $T %x, 1\n\t%y2 = lshr $T %y, 1\n\tjmp loop\n"
	"done:\n\tret $T %r\n}\n\n";

static const char* gszIR816Divide =
	"function $T @R816_$NAME($T %n, $T %d)\n{\n"
	"entry:\n\tjmp loop\n"
	"loop:\n\t%i = phi i16 [entry, $N], [next, %i2]\n\t%q = phi $T [entry, 0], [next, %q2]\n\t%r = phi $T [entry, 0], [next, %r2]\n\t%m = phi $T [entry, %n], [next, %m1]\n"
	"\t%done = eq i16 %i, 0\n\tbrif %done, exit, body\n"
	"body:\n\t%top = lshr $T %m, $M\n\t%rs = shl $T %r, 1\n\t%rb = or $T %rs, %top\n\t%m1 = shl $T %m, 1\n\t%qs = shl $T %q, 1\n"
	"\t%rtop = lshr $T %r, $M\n\t%over = ne $T %rtop, 0\n\tbrif %over, sub, test\n"
	"test:\n\t%ge = uge $T %rb, %d\n\tbrif %ge, sub, keep\n"
	"sub:\n\t%rd = sub $T %rb, %d\n\t%qo = or $T %qs, 1\n\tjmp next\n"
	"keep:\n\tjmp next\n"
	"next:\n\t%r2 = phi $T [sub, %rd], [keep, %rb]\n\t%q2 = phi $T [sub, %qo], [keep, %qs]\n\t%i2 = sub i16 %i, 1\n\tjmp loop\n"
	"exit:\n\tret $T $RESULT\n}\n\n";

static const char* gszIR816Signed =
	"function $T @R816_$NAME($T %a, $T %b)\n{\n"
	"entry:\n\t%an = slt $T %a, 0\n\tbrif %an, negA, posA\n"
	"negA:\n\t%na = sub $T 0, %a\n\tjmp gotA\n"
	"posA:\n\tjmp gotA\n"
	"gotA:\n\t%ua = phi $T [negA, %na], [posA, %a]\n\t%bn = slt $T %b, 0\n\tbrif %bn, negB, posB\n"
	"negB:\n\t%nb = sub $T 0, %b\n\tjmp gotB\n"
	"posB:\n\tjmp gotB\n"
	"gotB:\n\t%ub = phi $T [negB, %nb], [posB, %b]\n\t%u = call $T @R816_U$UNAME($T %ua, $T %ub)\n"
	"\t%s = $SIGN\n\tbrif %s, neg, pos\n"
	"neg:\n\t%nu = sub $T 0, %u\n\tret $T %nu\n"
	"pos:\n\tret $T %u\n}\n\n";


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Init(CIRModule* pcModule, int* piLabels)
{
	mpcModule = pcModule;
	mpiLabels = piLabels;
	maszRuntime.Init();
	mpcFunction = NULL;
	mpszOut = NULL;
	mpcErrors = NULL;
	mbResult = true;
	maiOffsets.Init();
	maiIncoming.Init();
	maiSlots.Init();
	maiBlockLabels.Init();
	maiParameters.Init();
	maiPendingArguments.Init();
	masFoldBase.Init();
	maiFoldOffset.Init();
	maiUses.Init();
	maiUseBlock.Init();
	mabInX.Init();
	miFused = 0;
	miFrameBytes = 0;
	miHiddenResult = -1;
	mszSectionOptions = "";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Kill(void)
{
	maiPendingArguments.Kill();
	maiParameters.Kill();
	maiBlockLabels.Kill();
	maiSlots.Kill();
	maiIncoming.Kill();
	mabInX.Kill();
	maiUseBlock.Kill();
	maiUses.Kill();
	maiFoldOffset.Kill();
	masFoldBase.Kill();
	maiOffsets.Kill();
	maszRuntime.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each function and global a section of its own.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::GenerateModule(CChars* pszDest, CCPPErrors* pcErrors)
{
	size			ui;
	CIRFunction*	pcFunction;
	char*			szName;
	bool			bResult;

	bResult = true;
	for (ui = 0; ui < mpcModule->NumFunctions(); ui++)
	{
		pcFunction = mpcModule->GetFunction(ui);
		szName = mpcModule->GetSymbolName(pcFunction->muiSymbol);
		pszDest->Append("\t\t.section text.");
		pszDest->Append(szName);
		pszDest->Append(mszSectionOptions);
		pszDest->AppendNewLine();
		Binding(szName, pcFunction->muiAttributes, pszDest);
		bResult &= GenerateFunction(pcFunction, szName, pszDest, pcErrors);
	}
	for (ui = 0; ui < mpcModule->NumGlobals(); ui++)
	{
		GenerateGlobal(mpcModule->GetGlobal(ui), pszDest);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime routines GenerateModule's functions called.
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CIR816Backend::GetRuntime(void)
{
	return &maszRuntime;
}


//////////////////////////////////////////////////////////////////////////
//
// .global or .weak, or nothing for the unit's own label.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Binding(const char* szName, uint32 uiAttributes, CChars* pszDest)
{
	if (uiAttributes & IRA_Local)
	{
		return;
	}
	pszDest->Append((uiAttributes & IRA_Weak) ? "\t\t.weak " : "\t\t.global ");
	pszDest->Append(szName);
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::GenerateGlobal(CIRGlobal* pcGlobal, CChars* pszDest)
{
	size			ui;
	size			uiByte;
	SIRDataItem*	psItem;
	char*			szName;
	int				iBytes;

	szName = mpcModule->GetSymbolName(pcGlobal->muiSymbol);
	pszDest->Append("\t\t.section data.");
	pszDest->Append(szName);
	pszDest->AppendNewLine();
	if (pcGlobal->muiAlign > 1)
	{
		pszDest->Append("\t\t.align ");
		pszDest->Append(pcGlobal->muiAlign);
		pszDest->AppendNewLine();
	}
	Binding(szName, pcGlobal->muiAttributes, pszDest);
	pszDest->Append(szName);
	pszDest->Append(':');
	pszDest->AppendNewLine();

	for (ui = 0; ui < pcGlobal->masItems.NumElements(); ui++)
	{
		psItem = pcGlobal->masItems.Get(ui);
		switch (psItem->uiKind)
		{
		case IRDK_Value:
		case IRDK_Symbol:
			iBytes = mpcModule->GetSize(psItem->sType);
			pszDest->Append((iBytes == 1) ? "\t\t.byte " : (iBytes == 2) ? "\t\t.word " : (iBytes == 4) ? "\t\t.long " : "\t\t.quad ");
			if (psItem->uiKind == IRDK_Symbol)
			{
				pszDest->Append(mpcModule->GetSymbolName((uint32)psItem->ulliValue));
				if (psItem->lliOffset != 0)
				{
					pszDest->Append(psItem->lliOffset > 0 ? " + " : " - ");
					pszDest->Append(psItem->lliOffset > 0 ? psItem->lliOffset : -psItem->lliOffset);
				}
			}
			else
			{
				pszDest->Append(IRSignExtend(psItem->sType, psItem->ulliValue));
			}
			pszDest->AppendNewLine();
			break;
		case IRDK_Bytes:
			for (uiByte = 0; uiByte < psItem->uiLength; uiByte++)
			{
				pszDest->Append(((uiByte % 16) == 0) ? "\t\t.byte " : ", ");
				pszDest->Append((int32)*pcGlobal->macBytes.Get(psItem->uiStart + uiByte));
				if (((uiByte % 16) == 15) || (uiByte + 1 == psItem->uiLength))
				{
					pszDest->AppendNewLine();
				}
			}
			break;
		case IRDK_Zero:
			if (psItem->ulliValue > 0)
			{
				pszDest->Append("\t\t.space ");
				pszDest->Append(psItem->ulliValue);
				pszDest->AppendNewLine();
			}
			break;
		}
	}
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::Error(const char* szMessage, const char* szDetail)
{
	CChars	szError;

	szError.Init("@");
	szError.Append(mpcModule->GetSymbolName(mpcFunction->muiSymbol));
	szError.Append(": ");
	szError.Append(szMessage);
	if (mpcErrors)
	{
		mpcErrors->Add(NULL, -1, -1, -1, szError.Text(), szDetail);
	}
	szError.Kill();
	mbResult = false;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Line(const char* szText)
{
	mpszOut->Append("\t\t");
	mpszOut->Append(szText);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// szText followed by a number.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Line(const char* szText, int64 lliValue)
{
	mpszOut->Append("\t\t");
	mpszOut->Append(szText);
	mpszOut->Append(lliValue);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::NewLabel(void)
{
	(*mpiLabels)++;
	return *mpiLabels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Label(int32 iLabel)
{
	mpszOut->Append('L');
	mpszOut->Append(iLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::UseRuntime(const char* szName)
{
	size	i;

	for (i = 0; i < maszRuntime.NumElements(); i++)
	{
		if (strcmp(maszRuntime.GetText(i), szName) == 0)
		{
			return;
		}
	}
	maszRuntime.Add(szName);
}


//////////////////////////////////////////////////////////////////////////
//
// A value's place in the frame: a word for i1 to i16 (zero extended), two for 32 bits, four for 64.
//
//////////////////////////////////////////////////////////////////////////
int CIR816Backend::ValueBytes(SIRType sType)
{
	switch (sType.uiKind)
	{
	case IRTK_I1:
	case IRTK_I8:
	case IRTK_I16:
		return 2;
	case IRTK_I32:
	case IRTK_F32:
		return 4;
	case IRTK_I64:
	case IRTK_F64:
		return 8;
	case IRTK_Pointer:
		if (mpcModule->GetPointerSize(sType.uiSpace) != 4)
		{
			Error("Only 4 byte pointers are done yet");
		}
		return 4;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIR816Backend::Words(SIRType sType)
{
	return ValueBytes(sType) / 2;
}


//////////////////////////////////////////////////////////////////////////
//
// A 64 bit result is written through a hidden pointer.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsHiddenResult(SIRType sType)
{
	return ValueBytes(sType) == 8;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::Allocate(int32* piNext, int32 iBytes)
{
	int32	iOffset;

	iOffset = *piNext;
	*piNext += (iBytes + 1) & ~1;
	return iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// The temporaries get places after the scratch bytes, shared by those that are never live at once; each phi's way
// in and slot gets its own place after them.  The arguments are above the frame, the saved D and the return address:
// the hidden result pointer first, then the parameters in order.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Layout(void)
{
	size				ui;
	size				uiBlock;
	size				uiInstruction;
	int32				iNext;
	int32				iNone;
	int32				iArgument;
	CIRBlock*			pcBlock;
	SIRPhi*				psPhi;
	SIRInstruction*		psInstruction;
	SIRReference		sParameter;

	iNone = -1;
	maiOffsets.FakeSetUsedElements(0);
	maiIncoming.FakeSetUsedElements(0);
	maiSlots.FakeSetUsedElements(0);
	maiParameters.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		maiOffsets.Add(&iNone);
		maiIncoming.Add(&iNone);
		maiSlots.Add(&iNone);
	}

	//The temporaries share places when they aren't live at once (PackTemporaries); then the phis' ways in and the
	//slots, each a place of its own.
	FoldPointers();
	iNext = PackTemporaries();
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			psPhi = pcBlock->masPhis.Get(ui);
			if (IsCoalescedPhiBlock(pcBlock))
			{
				//The jumps in put the values straight in the phis' places.
				*maiIncoming.Get(psPhi->sResult.Index()) = OffsetOf(psPhi->sResult);
			}
			else
			{
				*maiIncoming.Get(psPhi->sResult.Index()) = Allocate(&iNext, ValueBytes(psPhi->sType));
			}
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->uiOpcode == IRO_Slot)
			{
				*maiSlots.Get(psInstruction->sResult.Index()) = Allocate(&iNext, (int32)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits);
			}
		}
	}
	miFrameBytes = iNext - 1;

	iArgument = miFrameBytes + 1 + IR816_LINK_BYTES;
	miHiddenResult = -1;
	if (IsHiddenResult(mpcFunction->msReturnType))
	{
		miHiddenResult = iArgument;
		iArgument += 4;
	}
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		sParameter = *mpcFunction->masParameters.Get(ui);
		maiParameters.Add(&iArgument);
		*maiOffsets.Get(sParameter.Index()) = iArgument;
		iArgument += ValueBytes(mpcFunction->GetTemporaryType(sParameter));
	}

	//Frame reaches past the direct page through X, so nothing is kept in X then.
	if (iArgument > 255)
	{
		for (ui = 0; ui < mabInX.NumElements(); ui++)
		{
			*mabInX.Get(ui) = false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer plus a constant that is only used as loads' and stores'
// addresses isn't worked out: they use the pointer it adds to, with the
// constant in Y ([[D + n]:24 + Y]) or added to a symbol.  Its base pointer
// (msFoldBase) and constant (miFoldOffset) are kept per temporary.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FoldPointers(void)
{
	CArrayTemplate<int32>	aiAddressUses;
	CArrayTemplate<int32>	aiUses;
	size					uiTemporaries;
	size					ui;
	size					uj;
	size					uk;
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRPhi*					psPhi;
	SIRReference			sNone;
	int32					iZero;
	int64					lliOffset;

	uiTemporaries = mpcFunction->NumTemporaries();
	sNone = IRNone();
	iZero = 0;
	masFoldBase.FakeSetUsedElements(0);
	maiFoldOffset.FakeSetUsedElements(0);
	aiAddressUses.Init();
	aiUses.Init();
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		masFoldBase.Add(&sNone);
		maiFoldOffset.Add(&iZero);
		aiAddressUses.Add(&iZero);
		aiUses.Add(&iZero);
	}

	//How many uses each temporary has, and how many of them are loads' or stores' addresses.
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masPhis.NumElements(); uj++)
		{
			psPhi = pcBlock->masPhis.Get(uj);
			for (uk = 0; uk < psPhi->asArguments.NumElements(); uk++)
			{
				if (psPhi->asArguments.Get(uk)->sValue.IsTemporary())
				{
					(*aiUses.Get(psPhi->asArguments.Get(uk)->sValue.Index()))++;
				}
			}
		}
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			for (uk = 0; uk < 2; uk++)
			{
				if (psInstruction->asArguments[uk].IsTemporary())
				{
					(*aiUses.Get(psInstruction->asArguments[uk].Index()))++;
					if (((psInstruction->uiOpcode == IRO_Load) && (uk == 0)) || ((psInstruction->uiOpcode == IRO_Store) && (uk == 1)))
					{
						(*aiAddressUses.Get(psInstruction->asArguments[uk].Index()))++;
					}
				}
			}
		}
		if (pcBlock->msJump.sArgument.IsTemporary())
		{
			(*aiUses.Get(pcBlock->msJump.sArgument.Index()))++;
		}
	}

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if ((psInstruction->uiOpcode == IRO_PtrAdd) && psInstruction->sResult.IsTemporary() && psInstruction->asArguments[1].IsConstant() &&
				(psInstruction->asArguments[0].IsTemporary() || psInstruction->asArguments[0].IsSymbol()) &&
				(*aiUses.Get(psInstruction->sResult.Index()) > 0) && (*aiUses.Get(psInstruction->sResult.Index()) == *aiAddressUses.Get(psInstruction->sResult.Index())))
			{
				lliOffset = (int64)mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
				if ((lliOffset >= 0) && (lliOffset <= 0xFF00))
				{
					*masFoldBase.Get(psInstruction->sResult.Index()) = psInstruction->asArguments[0];
					*maiFoldOffset.Get(psInstruction->sResult.Index()) = (int32)lliOffset;
				}
			}
		}
	}

	//A folded pointer's base that is itself folded is followed to the end.
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		while (!masFoldBase.Get(ui)->IsNone() && masFoldBase.Get(ui)->IsTemporary() && !masFoldBase.Get(masFoldBase.Get(ui)->Index())->IsNone())
		{
			uk = masFoldBase.Get(ui)->Index();
			*maiFoldOffset.Get(ui) += *maiFoldOffset.Get(uk);
			*masFoldBase.Get(ui) = *masFoldBase.Get(uk);
		}
		if (*maiFoldOffset.Get(ui) > 0xFF00)
		{
			*masFoldBase.Get(ui) = sNone;
			*maiFoldOffset.Get(ui) = 0;
		}
	}
	maiUses.FakeSetUsedElements(0);
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		maiUses.Add(aiUses.Get(ui));
	}
	aiUses.Kill();
	aiAddressUses.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A value, or the base pointer a folded pointer adds to.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIR816Backend::FoldedBase(SIRReference sValue)
{
	if (sValue.IsTemporary() && (sValue.Index() < masFoldBase.NumElements()) && !masFoldBase.Get(sValue.Index())->IsNone())
	{
		return *masFoldBase.Get(sValue.Index());
	}
	return sValue;
}


#define IR816_EVENT_USE		0
#define IR816_EVENT_DEFINE	1
#define IR816_EVENT_WRITE	2


struct SIR816Event
{
	int32	iTemporary;
	int32	iPosition;
	int32	iKind;		//IR816_EVENT_USE, _DEFINE or _WRITE.
};


//////////////////////////////////////////////////////////////////////////
//
// The positions each temporary is live over, as ranges in position order
// (with holes where it isn't live), and the pairs of temporaries that may
// meet at one position when they have the same place: an instruction's
// result and an argument it reads a word at a time ahead of writing that
// word, and a phi and its value on a jump that copies the value to it.
//
//////////////////////////////////////////////////////////////////////////
class CIR816LiveRanges
{
public:
	CArrayTemplate<int32>	maiFirst;		//Per temporary: its first range (or -1), a list through maiNext.
	CArrayTemplate<int32>	maiLast;		//Per temporary: its last range.
	CArrayTemplate<int32>	maiStart;		//Per range.
	CArrayTemplate<int32>	maiEnd;
	CArrayTemplate<int32>	maiNext;
	CArrayTemplate<int32>	maiPairFirst;	//Per temporary: its first pair (or -1), a list through maiPairNext.
	CArrayTemplate<int32>	maiPairNext;
	CArrayTemplate<int32>	maiPairOther;	//Per pair: the other temporary...
	CArrayTemplate<int32>	maiPairPosition;	//...and where they meet.
	CArrayTemplate<int32>	maiWordFirst;		//Per word of the frame: its first occupant (or -1), a list through maiOccupantNext.
	CArrayTemplate<int32>	maiOccupantNext;
	CArrayTemplate<int32>	maiOccupant;		//Per occupant: the temporary placed on the word.

	void	Init(size uiTemporaries);
	void	Kill(void);
	void	Add(int32 iTemporary, int32 iStart, int32 iEnd);
	void	AddPair(SIRReference sOne, SIRReference sOther, int32 iPosition);
	bool	IsPair(int32 iOne, int32 iOther, int32 iPosition);
	bool	Interferes(int32 iOne, int32 iOther, bool bSamePlace);
	int32	Start(int32 iTemporary);
	bool	IsFree(int32 iTemporary, int32 iOffset, int32 iBytes, CArrayTemplate<int32>* paiOffsets);
	bool	LiveAcross(int32 iTemporary, int32 iPosition);
	void	Take(int32 iTemporary, int32 iOffset, int32 iBytes);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816LiveRanges::Init(size uiTemporaries)
{
	size	ui;
	int32	iNone;

	iNone = -1;
	maiFirst.Init();
	maiLast.Init();
	maiPairFirst.Init();
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		maiFirst.Add(&iNone);
		maiLast.Add(&iNone);
		maiPairFirst.Add(&iNone);
	}
	maiStart.Init();
	maiEnd.Init();
	maiNext.Init();
	maiPairNext.Init();
	maiPairOther.Init();
	maiPairPosition.Init();
	maiWordFirst.Init();
	maiOccupantNext.Init();
	maiOccupant.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816LiveRanges::Kill(void)
{
	maiOccupant.Kill();
	maiOccupantNext.Kill();
	maiWordFirst.Kill();
	maiPairPosition.Kill();
	maiPairOther.Kill();
	maiPairNext.Kill();
	maiNext.Kill();
	maiEnd.Kill();
	maiStart.Kill();
	maiPairFirst.Kill();
	maiLast.Kill();
	maiFirst.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Ranges are added in position order; one that overlaps the last is merged into it.
//
//////////////////////////////////////////////////////////////////////////
void CIR816LiveRanges::Add(int32 iTemporary, int32 iStart, int32 iEnd)
{
	int32	iLast;
	int32	iRange;
	int32	iNone;

	iLast = *maiLast.Get(iTemporary);
	if ((iLast != -1) && (*maiEnd.Get(iLast) >= iStart))
	{
		if (iEnd > *maiEnd.Get(iLast))
		{
			*maiEnd.Get(iLast) = iEnd;
		}
		return;
	}
	iNone = -1;
	iRange = (int32)maiStart.NumElements();
	maiStart.Add(&iStart);
	maiEnd.Add(&iEnd);
	maiNext.Add(&iNone);
	if (iLast == -1)
	{
		*maiFirst.Get(iTemporary) = iRange;
	}
	else
	{
		*maiNext.Get(iLast) = iRange;
	}
	*maiLast.Get(iTemporary) = iRange;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816LiveRanges::AddPair(SIRReference sOne, SIRReference sOther, int32 iPosition)
{
	int32	iPair;
	int32	iOne;
	int32	iOther;

	if (!sOne.IsTemporary() || !sOther.IsTemporary() || sOne.Equals(sOther))
	{
		return;
	}
	iOne = (int32)sOne.Index();
	iOther = (int32)sOther.Index();

	iPair = (int32)maiPairOther.NumElements();
	maiPairOther.Add(&iOther);
	maiPairPosition.Add(&iPosition);
	maiPairNext.Add(maiPairFirst.Get(iOne));
	*maiPairFirst.Get(iOne) = iPair;

	iPair = (int32)maiPairOther.NumElements();
	maiPairOther.Add(&iOne);
	maiPairPosition.Add(&iPosition);
	maiPairNext.Add(maiPairFirst.Get(iOther));
	*maiPairFirst.Get(iOther) = iPair;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816LiveRanges::IsPair(int32 iOne, int32 iOther, int32 iPosition)
{
	int32	iPair;

	for (iPair = *maiPairFirst.Get(iOne); iPair != -1; iPair = *maiPairNext.Get(iPair))
	{
		if ((*maiPairOther.Get(iPair) == iOther) && (*maiPairPosition.Get(iPair) == iPosition))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two temporaries are live at once, but where a pair of them in the
// same place (bSamePlace) meets at a single position: one's range ends there
// and the other's starts.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816LiveRanges::Interferes(int32 iOne, int32 iOther, bool bSamePlace)
{
	int32	iA;
	int32	iB;
	int32	iLow;
	int32	iHigh;

	iA = *maiFirst.Get(iOne);
	iB = *maiFirst.Get(iOther);
	while ((iA != -1) && (iB != -1))
	{
		iLow = (*maiStart.Get(iA) > *maiStart.Get(iB)) ? *maiStart.Get(iA) : *maiStart.Get(iB);
		iHigh = (*maiEnd.Get(iA) < *maiEnd.Get(iB)) ? *maiEnd.Get(iA) : *maiEnd.Get(iB);
		if (iLow <= iHigh)
		{
			//Only one ending where the other starts (a result that is never used, at the place of an argument still
			//live after it, is live with it).
			if (!bSamePlace || (iLow != iHigh) || !IsPair(iOne, iOther, iLow) ||
				!(((*maiEnd.Get(iA) == iLow) && (*maiStart.Get(iB) == iLow)) || ((*maiEnd.Get(iB) == iLow) && (*maiStart.Get(iA) == iLow))))
			{
				return true;
			}
		}
		if (*maiEnd.Get(iA) < *maiEnd.Get(iB))
		{
			iA = *maiNext.Get(iA);
		}
		else
		{
			iB = *maiNext.Get(iB);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The first position a temporary is live at, or -1 when it never is.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816LiveRanges::Start(int32 iTemporary)
{
	int32	iRange;

	iRange = *maiFirst.Get(iTemporary);
	return (iRange == -1) ? -1 : *maiStart.Get(iRange);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a temporary is live before and after iPosition (not only read or
// written there).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816LiveRanges::LiveAcross(int32 iTemporary, int32 iPosition)
{
	int32	iRange;

	for (iRange = *maiFirst.Get(iTemporary); iRange != -1; iRange = *maiNext.Get(iRange))
	{
		if ((*maiStart.Get(iRange) < iPosition) && (*maiEnd.Get(iRange) > iPosition))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether iBytes from iOffset are free for a temporary: no temporary placed
// on them is live with it (but a pair meeting in the same place).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816LiveRanges::IsFree(int32 iTemporary, int32 iOffset, int32 iBytes, CArrayTemplate<int32>* paiOffsets)
{
	int32	iWord;
	int32	iOccupant;
	int32	iOther;

	for (iWord = (iOffset - IR816_FIRST_TEMPORARY) / 2; iWord < (iOffset + iBytes - IR816_FIRST_TEMPORARY) / 2; iWord++)
	{
		if (iWord >= (int32)maiWordFirst.NumElements())
		{
			break;
		}
		for (iOccupant = *maiWordFirst.Get(iWord); iOccupant != -1; iOccupant = *maiOccupantNext.Get(iOccupant))
		{
			iOther = *maiOccupant.Get(iOccupant);
			if (Interferes(iTemporary, iOther, *paiOffsets->Get(iOther) == iOffset))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816LiveRanges::Take(int32 iTemporary, int32 iOffset, int32 iBytes)
{
	int32	iWord;
	int32	iOccupant;
	int32	iNone;

	iNone = -1;
	for (iWord = (iOffset - IR816_FIRST_TEMPORARY) / 2; iWord < (iOffset + iBytes - IR816_FIRST_TEMPORARY) / 2; iWord++)
	{
		while (iWord >= (int32)maiWordFirst.NumElements())
		{
			maiWordFirst.Add(&iNone);
		}
		iOccupant = (int32)maiOccupant.NumElements();
		maiOccupant.Add(&iTemporary);
		maiOccupantNext.Add(maiWordFirst.Get(iWord));
		*maiWordFirst.Get(iWord) = iOccupant;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A temporary's use, definition or write (a phi's value put in the phi's
// place on the way out of a block) at a position.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AddEvent(CArrayTemplate<SIR816Event>* pasEvents, SIRReference sValue, int32 iPosition, int32 iKind)
{
	SIR816Event*	psEvent;

	if (!sValue.IsTemporary())
	{
		return;
	}
	psEvent = pasEvents->Add();
	psEvent->iTemporary = (int32)sValue.Index();
	psEvent->iPosition = iPosition;
	psEvent->iKind = iKind;
}


//////////////////////////////////////////////////////////////////////////
//
// A temporary used in a block (before the block defines it) is live in.
//
//////////////////////////////////////////////////////////////////////////
static void IR816Use(uint32* puiUse, uint32* puiDef, SIRReference sValue)
{
	uint32	ui;

	if (!sValue.IsTemporary())
	{
		return;
	}
	ui = sValue.Index();
	if (!(puiDef[ui / 32] & (1u << (ui % 32))))
	{
		puiUse[ui / 32] |= 1u << (ui % 32);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction's result can have the place of its argument
// iArgument when that argument dies there: true when each word of the
// argument is read before that word of the result is written and never
// after.  A shift copies its value to the result before reading the count; a
// load writes the result through the pointer.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CanShareArgument(uint16 uiOpcode, int iArgument)
{
	switch (uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
	case IRO_Sub:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
	case IRO_Neg:
	case IRO_Copy:
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
	case IRO_Eq:
	case IRO_Ne:
	case IRO_SLt:
	case IRO_SLe:
	case IRO_SGt:
	case IRO_SGe:
	case IRO_ULt:
	case IRO_ULe:
	case IRO_UGt:
	case IRO_UGe:
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
	case IRO_SpaceCast:
	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
	case IRO_FNeg:
	case IRO_FEq:
	case IRO_FNe:
	case IRO_FLt:
	case IRO_FLe:
	case IRO_FGt:
	case IRO_FGe:
	case IRO_FOrdered:
	case IRO_FUnordered:
	case IRO_SIToF:
	case IRO_UIToF:
	case IRO_FToSI:
	case IRO_FToUI:
	case IRO_FExt:
	case IRO_FTrunc:
	case IRO_Call:
		return true;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		return iArgument == 0;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gives the temporaries (but the parameters, which are the caller's
// arguments) their places from IR816_FIRST_TEMPORARY on, sharing places
// between temporaries that are never live at once.  Positions number the
// phis, instructions and jump of each block in the order the blocks are
// written.  A temporary is live from its definition, or the start of a block
// it is live into, to its last use, or the end of a block it is live out of
// (CIR816LiveRanges).  An argument is used where its call pushes it; a
// phi's arguments at the end of their blocks and its result at the start of
// its own, or also at the ends of its predecessors when they put its values
// straight in its place (IsCoalescedPhiBlock).  Uses of an instruction's
// operands and its result's definition are at the same position, so they
// only share a place when they are a pair (IR816CanShareArgument, or a call's
// argument) at exactly the same place.  Each temporary, in the order they
// start, is placed at a pair's place when it can be, else at the first place
// free.  Returns the first byte after them.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::PackTemporaries(void)
{
	CIR816LiveRanges			cRanges;
	CArrayTemplate<SIR816Event>	asEvents;
	CArrayTemplate<int32>		aiBlockEvents;	//Per block: its first event, and one past the last block's last.
	CArrayTemplate<int32>		aiBlockStart;
	CArrayTemplate<int32>		aiBlockEnd;
	CArrayTemplate<uint32>		auiUse;
	CArrayTemplate<uint32>		auiDef;
	CArrayTemplate<uint32>		auiIn;
	CArrayTemplate<uint32>		auiOut;
	CArrayTemplate<int32>		aiOpen;			//Per temporary: where its range in the block being scanned starts, or -1.
	CArrayTemplate<int32>		aiLastUse;		//Per temporary: its last use or definition in that block.
	CArrayTemplate<int32>		aiOpened;		//The temporaries with a range open in that block.
	CArrayTemplate<int32>		aiOrder;
	CArrayTemplate<bool>		abParameter;
	CIRBlock*					pcBlock;
	CIRBlock*					pcSuccessor;
	SIRInstruction*				psInstruction;
	SIRPhi*						psPhi;
	SIRPhiArgument*				psArgument;
	SIR816Event*				psEvent;
	size						uiTemporaries;
	size						uiWords;
	size						uiBlocks;
	size						ui;
	size						uj;
	size						uk;
	int32						iPosition;
	int32						iCallPosition;
	int32						iNone;
	int32						iBytes;
	int32						iOffset;
	int32						iNext;
	int32						iTemporary;
	int32						iEvent;
	int32						iEnd;
	int32						iPair;
	int32						iArgument;
	uint32*						puiUse;
	uint32*						puiDef;
	uint32*						puiIn;
	uint32*						puiOut;
	uint32*						puiSuccessorIn;
	uint32						uiNew;
	bool						bChanged;
	bool						bFree;

	uiTemporaries = mpcFunction->NumTemporaries();
	uiWords = (uiTemporaries + 31) / 32;
	uiBlocks = mpcFunction->NumBlocks();
	iNone = -1;
	cRanges.Init(uiTemporaries);
	asEvents.Init();
	aiBlockEvents.Init();
	aiBlockStart.Init();
	aiBlockEnd.Init();

	//The uses, definitions and writes, and the pairs.
	iPosition = 0;
	for (ui = 0; ui < uiBlocks; ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		iEvent = (int32)asEvents.NumElements();
		aiBlockEvents.Add(&iEvent);
		aiBlockStart.Add(&iPosition);
		for (uj = 0; uj < pcBlock->masPhis.NumElements(); uj++)
		{
			IR816AddEvent(&asEvents, pcBlock->masPhis.Get(uj)->sResult, iPosition, IR816_EVENT_DEFINE);
		}
		iPosition++;
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if (psInstruction->uiOpcode == IRO_Arg)
			{
				//Used where the call pushes it.
				iCallPosition = iPosition;
				for (uk = uj + 1; uk < pcBlock->masInstructions.NumElements(); uk++)
				{
					if (pcBlock->masInstructions.Get(uk)->uiOpcode == IRO_Call)
					{
						iCallPosition = iPosition + (int32)(uk - uj);
						cRanges.AddPair(pcBlock->masInstructions.Get(uk)->sResult, FoldedBase(psInstruction->asArguments[0]), iCallPosition);
						break;
					}
				}
				IR816AddEvent(&asEvents, FoldedBase(psInstruction->asArguments[0]), iCallPosition, IR816_EVENT_USE);
			}
			else
			{
				IR816AddEvent(&asEvents, FoldedBase(psInstruction->asArguments[0]), iPosition, IR816_EVENT_USE);
				IR816AddEvent(&asEvents, FoldedBase(psInstruction->asArguments[1]), iPosition, IR816_EVENT_USE);
			}
			if (psInstruction->sResult.IsTemporary() && (FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult)))
			{
				IR816AddEvent(&asEvents, psInstruction->sResult, iPosition, IR816_EVENT_DEFINE);
				for (iArgument = 0; iArgument < 2; iArgument++)
				{
					if (IR816CanShareArgument(psInstruction->uiOpcode, iArgument) && FoldedBase(psInstruction->asArguments[iArgument]).Equals(psInstruction->asArguments[iArgument]))
					{
						cRanges.AddPair(psInstruction->sResult, psInstruction->asArguments[iArgument], iPosition);
					}
				}
			}
			iPosition++;
		}

		//The jump's argument and the phi arguments of the successors (copied on the way out).
		IR816AddEvent(&asEvents, pcBlock->msJump.sArgument, iPosition, IR816_EVENT_USE);
		for (uj = 0; uj < pcBlock->NumSuccessors(); uj++)
		{
			pcSuccessor = pcBlock->GetSuccessor(uj);
			for (uk = 0; uk < pcSuccessor->masPhis.NumElements(); uk++)
			{
				psPhi = pcSuccessor->masPhis.Get(uk);
				psArgument = pcSuccessor->FindPhiArgument(psPhi, pcBlock);
				if (psArgument)
				{
					IR816AddEvent(&asEvents, psArgument->sValue, iPosition, IR816_EVENT_USE);
				}
				if (IsCoalescedPhiBlock(pcSuccessor))
				{
					//Written here, on the way out.
					IR816AddEvent(&asEvents, psPhi->sResult, iPosition, IR816_EVENT_WRITE);
					if (psArgument)
					{
						cRanges.AddPair(psPhi->sResult, psArgument->sValue, iPosition);
					}
				}
			}
		}
		aiBlockEnd.Add(&iPosition);
		iPosition++;
	}
	iEvent = (int32)asEvents.NumElements();
	aiBlockEvents.Add(&iEvent);

	//Each block's use and def sets.
	auiUse.Init();
	auiDef.Init();
	auiIn.Init();
	auiOut.Init();
	auiUse.GrowToAtLeastNumElements(uiBlocks * uiWords + 1, true, 0);
	auiDef.GrowToAtLeastNumElements(uiBlocks * uiWords + 1, true, 0);
	auiIn.GrowToAtLeastNumElements(uiBlocks * uiWords + 1, true, 0);
	auiOut.GrowToAtLeastNumElements(uiBlocks * uiWords + 1, true, 0);
	for (ui = 0; ui < uiBlocks; ui++)
	{
		puiUse = auiUse.Get(ui * uiWords);
		puiDef = auiDef.Get(ui * uiWords);
		for (iEvent = *aiBlockEvents.Get(ui); iEvent < *aiBlockEvents.Get(ui + 1); iEvent++)
		{
			psEvent = asEvents.Get(iEvent);
			if (psEvent->iKind == IR816_EVENT_USE)
			{
				IR816Use(puiUse, puiDef, IRReference(IRRK_Temporary, (uint32)psEvent->iTemporary));
			}
			else if (psEvent->iKind == IR816_EVENT_DEFINE)
			{
				puiDef[psEvent->iTemporary / 32] |= 1u << (psEvent->iTemporary % 32);
			}
		}
	}

	//Liveness: in = use | (out & ~def), out = the successors' ins (less their phis' results), until nothing changes.
	do
	{
		bChanged = false;
		for (ui = uiBlocks; ui > 0; ui--)
		{
			pcBlock = mpcFunction->GetBlock(ui - 1);
			puiUse = auiUse.Get((ui - 1) * uiWords);
			puiDef = auiDef.Get((ui - 1) * uiWords);
			puiIn = auiIn.Get((ui - 1) * uiWords);
			puiOut = auiOut.Get((ui - 1) * uiWords);
			for (uj = 0; uj < pcBlock->NumSuccessors(); uj++)
			{
				pcSuccessor = pcBlock->GetSuccessor(uj);
				puiSuccessorIn = auiIn.Get(pcSuccessor->muiIndex * uiWords);
				for (uk = 0; uk < uiWords; uk++)
				{
					uiNew = puiOut[uk] | puiSuccessorIn[uk];
					if (uiNew != puiOut[uk])
					{
						puiOut[uk] = uiNew;
						bChanged = true;
					}
				}
			}
			for (uk = 0; uk < uiWords; uk++)
			{
				uiNew = puiUse[uk] | (puiOut[uk] & ~puiDef[uk]);
				if (uiNew != puiIn[uk])
				{
					puiIn[uk] = uiNew;
					bChanged = true;
				}
			}
		}
	}
	while (bChanged);

	//The ranges: in each block from the start (live in) or the definition, to the end (live out) or the last use.
	aiOpen.Init();
	aiLastUse.Init();
	aiOpened.Init();
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		aiOpen.Add(&iNone);
		aiLastUse.Add(&iNone);
	}
	for (ui = 0; ui < uiBlocks; ui++)
	{
		puiIn = auiIn.Get(ui * uiWords);
		puiOut = auiOut.Get(ui * uiWords);
		for (uj = 0; uj < uiTemporaries; uj++)
		{
			if (puiIn[uj / 32] & (1u << (uj % 32)))
			{
				iTemporary = (int32)uj;
				*aiOpen.Get(uj) = *aiBlockStart.Get(ui);
				*aiLastUse.Get(uj) = *aiBlockStart.Get(ui);
				aiOpened.Add(&iTemporary);
			}
		}
		for (iEvent = *aiBlockEvents.Get(ui); iEvent < *aiBlockEvents.Get(ui + 1); iEvent++)
		{
			psEvent = asEvents.Get(iEvent);
			if (psEvent->iKind != IR816_EVENT_WRITE)
			{
				if (*aiOpen.Get(psEvent->iTemporary) == -1)
				{
					*aiOpen.Get(psEvent->iTemporary) = psEvent->iPosition;
					aiOpened.Add(&psEvent->iTemporary);
				}
				if (psEvent->iPosition > *aiLastUse.Get(psEvent->iTemporary))
				{
					*aiLastUse.Get(psEvent->iTemporary) = psEvent->iPosition;
				}
			}
		}
		for (uj = 0; uj < aiOpened.NumElements(); uj++)
		{
			iTemporary = *aiOpened.Get(uj);
			iEnd = (puiOut[iTemporary / 32] & (1u << (iTemporary % 32))) ? *aiBlockEnd.Get(ui) : *aiLastUse.Get(iTemporary);
			cRanges.Add(iTemporary, *aiOpen.Get(iTemporary), iEnd);
			*aiOpen.Get(iTemporary) = -1;
			*aiLastUse.Get(iTemporary) = -1;
		}
		aiOpened.FakeSetUsedElements(0);
		for (iEvent = *aiBlockEvents.Get(ui); iEvent < *aiBlockEvents.Get(ui + 1); iEvent++)
		{
			psEvent = asEvents.Get(iEvent);
			if (psEvent->iKind == IR816_EVENT_WRITE)
			{
				cRanges.Add(psEvent->iTemporary, psEvent->iPosition, psEvent->iPosition);
			}
		}
	}

	//Placed in the order they start (but the parameters).
	abParameter.Init();
	abParameter.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		*abParameter.Get(mpcFunction->masParameters.Get(ui)->Index()) = true;
	}
	aiOrder.Init();
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		iTemporary = (int32)ui;
		if ((cRanges.Start(iTemporary) >= 0) && (ValueBytes(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)ui))) != 0) && !*abParameter.Get(ui))
		{
			aiOrder.Add(&iTemporary);
		}
	}
	for (ui = 1; ui < aiOrder.NumElements(); ui++)
	{
		iTemporary = *aiOrder.Get(ui);
		for (uj = ui; (uj > 0) && (cRanges.Start(*aiOrder.Get(uj - 1)) > cRanges.Start(iTemporary)); uj--)
		{
			*aiOrder.Get(uj) = *aiOrder.Get(uj - 1);
		}
		*aiOrder.Get(uj) = iTemporary;
	}

	iNext = IR816_FIRST_TEMPORARY;
	for (ui = 0; ui < aiOrder.NumElements(); ui++)
	{
		iTemporary = *aiOrder.Get(ui);
		iBytes = (ValueBytes(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)iTemporary))) + 1) & ~1;

		//A pair's place, then the first place free.
		bFree = false;
		for (iPair = *cRanges.maiPairFirst.Get(iTemporary); (iPair != -1) && !bFree; iPair = *cRanges.maiPairNext.Get(iPair))
		{
			iOffset = *maiOffsets.Get(*cRanges.maiPairOther.Get(iPair));
			if (iOffset >= IR816_FIRST_TEMPORARY)
			{
				bFree = cRanges.IsFree(iTemporary, iOffset, iBytes, &maiOffsets);
			}
		}
		if (!bFree)
		{
			iOffset = IR816_FIRST_TEMPORARY;
			while (!cRanges.IsFree(iTemporary, iOffset, iBytes, &maiOffsets))
			{
				iOffset += 2;
			}
		}
		cRanges.Take(iTemporary, iOffset, iBytes);
		*maiOffsets.Get(iTemporary) = iOffset;
		if (iOffset + iBytes > iNext)
		{
			iNext = iOffset + iBytes;
		}
	}

	//A temporary with no definition or use still has a place (it is never live).
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		if ((cRanges.Start((int32)ui) == -1) && (ValueBytes(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)ui))) != 0))
		{
			*maiOffsets.Get(ui) = IR816_FIRST_TEMPORARY;
		}
	}

	AllocateX(&cRanges, &abParameter);
	abParameter.Kill();
	aiOrder.Kill();
	aiOpened.Kill();
	aiLastUse.Kill();
	aiOpen.Kill();
	auiOut.Kill();
	auiIn.Kill();
	auiDef.Kill();
	auiUse.Kill();
	aiBlockEnd.Kill();
	aiBlockStart.Kill();
	aiBlockEvents.Kill();
	asEvents.Kill();
	cRanges.Kill();
	return iNext;
}


#define IR816_X_BAD		-100000		//A use X can't serve: the temporary isn't kept in X.


//////////////////////////////////////////////////////////////////////////
//
// What keeping argument iArgument of an instruction in X saves (in about
// cycles) where it is read: txa for an lda, phx for a push, the operands
// of a commutative instruction swapped; a second operand read through a
// scratch word costs.  IR816_X_BAD for a use that can't read X.
//
//////////////////////////////////////////////////////////////////////////
static int32 IR816XUse(uint16 uiOpcode, int iArgument)
{
	switch (uiOpcode)
	{
	case IRO_Arg:
	case IRO_Copy:
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
	case IRO_Add:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
		return 3;
	case IRO_Sub:
	case IRO_Eq:
	case IRO_Ne:
	case IRO_ULt:
	case IRO_ULe:
	case IRO_UGt:
	case IRO_UGe:
		return (iArgument == 0) ? 3 : -5;
	case IRO_SLt:
	case IRO_SLe:
	case IRO_SGt:
	case IRO_SGe:
		return 3;
	case IRO_Neg:
		return -5;
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
		return 0;
	case IRO_Store:
		return (iArgument == 0) ? 3 : IR816_X_BAD;
	default:
		return IR816_X_BAD;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction's result goes to its place through Store (which
// can put it in X), or can be worked out in A and put in X (a copy, an
// extension or trunc, a 16 bit shift by a constant).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816XResult(SIRInstruction* psInstruction)
{
	switch (psInstruction->uiOpcode)
	{
	case IRO_Copy:
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
		return true;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		return psInstruction->asArguments[1].IsConstant() && psInstruction->sType.Is(IRTK_I16);
	case IRO_Add:
	case IRO_Sub:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
	case IRO_Neg:
	case IRO_Load:
	case IRO_Call:
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether writing an instruction changes X: calls (and the runtime and soft
// float routines), and shifts by a count only known at run time.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ClobbersX(SIRInstruction* psInstruction)
{
	switch (psInstruction->uiOpcode)
	{
	case IRO_Call:
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
	case IRO_FEq:
	case IRO_FNe:
	case IRO_FLt:
	case IRO_FLe:
	case IRO_FGt:
	case IRO_FGe:
	case IRO_FOrdered:
	case IRO_FUnordered:
	case IRO_SIToF:
	case IRO_UIToF:
	case IRO_FToSI:
	case IRO_FToUI:
	case IRO_FExt:
	case IRO_FTrunc:
		return true;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		return !psInstruction->asArguments[1].IsConstant();
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Register allocation of X: one word temporaries (16 bits or fewer) whose
// results Store writes, or phis, are kept in X instead of their places when
// nothing that changes X is done while they are live (a call, a shift by a
// count in X) and they don't interfere with the others in X (pairs in one
// place may meet, as PackTemporaries' ranges allow).  Each is scored by what
// X saves where it is used and defined (tax for sta, txa for lda, phx, inx,
// cpx, phi copies), 8 times more for each loop it is in, and the best are
// taken first.  Layout takes them all out again for a frame past the direct
// page (Frame reaches it through X).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::AllocateX(CIR816LiveRanges* pcRanges, CArrayTemplate<bool>* pabParameter)
{
	CIRDominators			cDominators;
	CIRLoops				cLoops;
	CArrayTemplate<int32>	aiScore;		//Per temporary.
	CArrayTemplate<uint8>	abCandidate;	//Per temporary.
	CArrayTemplate<int32>	aiClobbers;		//Positions that change X.
	CArrayTemplate<int32>	aiOrder;
	CArrayTemplate<int32>	aiInX;
	CIRBlock*				pcBlock;
	CIRBlock*				pcSuccessor;
	SIRInstruction*			psInstruction;
	SIRPhi*					psPhi;
	SIRPhiArgument*			psArgument;
	SIRReference			sValue;
	size					uiTemporaries;
	size					ui;
	size					uj;
	size					uk;
	int32					iPosition;
	int32					iWeight;
	int32					iDepth;
	int32					iTemporary;
	int32					iScore;
	int						iArgument;
	bool					bFree;

	uiTemporaries = mpcFunction->NumTemporaries();
	mabInX.FakeSetUsedElements(0);
	mabInX.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	aiScore.Init();
	aiScore.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	abCandidate.Init();
	abCandidate.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	aiClobbers.Init();

	mpcFunction->ComputePredecessors();
	cDominators.Init();
	cDominators.Compute(mpcFunction);
	cLoops.Init();
	cLoops.Compute(mpcFunction, &cDominators);

	//The scores and the positions that change X, numbered as PackTemporaries numbers them.
	iPosition = 0;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		iDepth = cLoops.Depth(pcBlock);
		iWeight = 1 << ((iDepth > 4) ? 12 : 3 * iDepth);
		for (uj = 0; uj < pcBlock->masPhis.NumElements(); uj++)
		{
			*abCandidate.Get(pcBlock->masPhis.Get(uj)->sResult.Index()) = true;
		}
		iPosition++;
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if (IR816ClobbersX(psInstruction))
			{
				aiClobbers.Add(&iPosition);
			}
			for (iArgument = 0; iArgument < 2; iArgument++)
			{
				sValue = psInstruction->asArguments[iArgument];
				if (sValue.IsTemporary())
				{
					iScore = IR816XUse(psInstruction->uiOpcode, iArgument);
					if (!FoldedBase(sValue).Equals(sValue) || (psInstruction->uiOpcode == IRO_Call) || (psInstruction->uiOpcode == IRO_Load))
					{
						iScore = IR816_X_BAD;
					}
					*aiScore.Get(sValue.Index()) += (iScore == IR816_X_BAD) ? IR816_X_BAD : iScore * iWeight;
				}
			}
			sValue = psInstruction->sResult;
			if (sValue.IsTemporary() && FoldedBase(sValue).Equals(sValue) && IR816XResult(psInstruction))
			{
				*abCandidate.Get(sValue.Index()) = true;
				*aiScore.Get(sValue.Index()) += 3 * iWeight;
				if (((psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_Sub)) && psInstruction->asArguments[0].IsTemporary() &&
					psInstruction->asArguments[1].IsConstant() && (mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits <= 2))
				{
					//inx or dex when both are in X.
					*aiScore.Get(sValue.Index()) += 6 * iWeight;
					*aiScore.Get(psInstruction->asArguments[0].Index()) += 6 * iWeight;
				}
			}
			iPosition++;
		}
		if (pcBlock->msJump.sArgument.IsTemporary())
		{
			*aiScore.Get(pcBlock->msJump.sArgument.Index()) += 3 * iWeight;
		}
		for (uj = 0; uj < pcBlock->NumSuccessors(); uj++)
		{
			pcSuccessor = pcBlock->GetSuccessor(uj);
			for (uk = 0; uk < pcSuccessor->masPhis.NumElements(); uk++)
			{
				psPhi = pcSuccessor->masPhis.Get(uk);
				psArgument = pcSuccessor->FindPhiArgument(psPhi, pcBlock);
				*aiScore.Get(psPhi->sResult.Index()) += 5 * iWeight;
				if (psArgument && psArgument->sValue.IsTemporary())
				{
					*aiScore.Get(psArgument->sValue.Index()) += 3 * iWeight;
				}
			}
		}
		iPosition++;
	}

	//The candidates, best first.
	aiOrder.Init();
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		iTemporary = (int32)ui;
		if (*abCandidate.Get(ui) && !*pabParameter->Get(ui) && (*aiScore.Get(ui) > 0) && (pcRanges->Start(iTemporary) >= 0) &&
			(Words(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)ui))) == 1))
		{
			aiOrder.Add(&iTemporary);
		}
	}
	for (ui = 1; ui < aiOrder.NumElements(); ui++)
	{
		iTemporary = *aiOrder.Get(ui);
		for (uj = ui; (uj > 0) && (*aiScore.Get(*aiOrder.Get(uj - 1)) < *aiScore.Get(iTemporary)); uj--)
		{
			*aiOrder.Get(uj) = *aiOrder.Get(uj - 1);
		}
		*aiOrder.Get(uj) = iTemporary;
	}

	aiInX.Init();
	for (ui = 0; ui < aiOrder.NumElements(); ui++)
	{
		iTemporary = *aiOrder.Get(ui);
		bFree = true;
		for (uj = 0; bFree && (uj < aiClobbers.NumElements()); uj++)
		{
			bFree = !pcRanges->LiveAcross(iTemporary, *aiClobbers.Get(uj));
		}
		for (uj = 0; bFree && (uj < aiInX.NumElements()); uj++)
		{
			bFree = !pcRanges->Interferes(iTemporary, *aiInX.Get(uj), true);
		}
		if (bFree)
		{
			aiInX.Add(&iTemporary);
			*mabInX.Get(iTemporary) = true;
		}
	}

	aiInX.Kill();
	aiOrder.Kill();
	cLoops.Kill();
	cDominators.Kill();
	aiClobbers.Kill();
	abCandidate.Kill();
	aiScore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::InX(SIRReference sValue)
{
	return sValue.IsTemporary() && (sValue.Index() < mabInX.NumElements()) && *mabInX.Get(sValue.Index());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::OffsetOf(SIRReference sTemporary)
{
	return *maiOffsets.Get(sTemporary.Index());
}


//////////////////////////////////////////////////////////////////////////
//
// szMnemonic on the frame's word at iOffset: [D + offset], or through X past the direct page's 256 bytes.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Frame(const char* szMnemonic, int32 iOffset)
{
	mpszOut->Append("\t\t");
	if (iOffset <= 255)
	{
		mpszOut->Append(szMnemonic);
		mpszOut->Append(" [D + ");
		mpszOut->Append(iOffset);
		mpszOut->Append("]");
	}
	else
	{
		mpszOut->Append("ldx ");
		mpszOut->Append(iOffset);
		mpszOut->AppendNewLine();
		mpszOut->Append("\t\t");
		mpszOut->Append(szMnemonic);
		mpszOut->Append(" [D + X]");
	}
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// szMnemonic with word iWord of a temporary, constant or symbol.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Operand(const char* szMnemonic, SIRReference sValue, int iWord)
{
	uint16	uiWord;
	char*	szName;

	if ((miFused > 0) && (strcmp(szMnemonic, "lda") == 0) && Fused(sValue))
	{
		WordInA(Fused(sValue), iWord);
		return;
	}
	if (sValue.IsTemporary())
	{
		if ((iWord == 0) && InX(sValue))
		{
			//Kept in X (AllocateX): read through A or a push, or else the scratch word.
			if (strcmp(szMnemonic, "lda") == 0)
			{
				Line("txa");
			}
			else if (strcmp(szMnemonic, "pei") == 0)
			{
				Line("phx");
			}
			else if (strcmp(szMnemonic, "ldx") != 0)
			{
				Frame("stx", IR816_SCRATCH_COUNT);
				Frame(szMnemonic, IR816_SCRATCH_COUNT);
			}
			return;
		}
		if (iWord < Words(mpcFunction->GetTemporaryType(sValue)))
		{
			Frame(szMnemonic, OffsetOf(sValue) + 2 * iWord);
			return;
		}
		uiWord = 0;
	}
	else if (sValue.IsConstant())
	{
		uiWord = (iWord < 4) ? (uint16)(mpcFunction->GetConstant(sValue)->ulliBits >> (16 * iWord)) : 0;
	}
	else if (sValue.IsSymbol())
	{
		szName = mpcModule->GetSymbolName(sValue.Index());
		mpszOut->Append("\t\t");
		mpszOut->Append(szMnemonic);
		if (iWord == 0)
		{
			mpszOut->Append(" ");
			mpszOut->Append(szName);
			mpszOut->Append(" & $FFFF");
		}
		else if (iWord == 1)
		{
			mpszOut->Append(" ");
			mpszOut->Append(szName);
			mpszOut->Append(" >> 16");
		}
		else
		{
			mpszOut->Append(" 0");
		}
		mpszOut->AppendNewLine();
		return;
	}
	else
	{
		uiWord = 0;
	}
	mpszOut->Append("\t\t");
	mpszOut->Append(szMnemonic);
	mpszOut->Append(" ");
	mpszOut->Append((int32)uiWord);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Store(SIRReference sResult, int iWord)
{
	if (InX(sResult))
	{
		Line("tax");
		return;
	}
	if ((miBlock >= 0) && (Words(mpcFunction->GetTemporaryType(sResult)) == 1) && (OffsetOf(sResult) <= 255) &&
		(sResult.Index() < maiUseBlock.NumElements()) && (*maiUseBlock.Get(sResult.Index()) == miBlock))
	{
		//Its one use is later in the block: the store is marked for Peephole, which leaves the value in A when that
		//use is the next lda of it with A not changed before it.
		mpszOut->Append("\t\tsta [D + ");
		mpszOut->Append(OffsetOf(sResult));
		mpszOut->Append("]" IR816_KEEP_IN_A);
		mpszOut->AppendNewLine();
		return;
	}
	Frame("sta", OffsetOf(sResult) + 2 * iWord);
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps an i8 or i1 in A zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Mask(SIRType sType)
{
	if (sType.Is(IRTK_I8))
	{
		Line("and $FF");
	}
	else if (sType.Is(IRTK_I1))
	{
		Line("and 1");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CopyWords(int32 iDestination, SIRReference sValue, int iWords)
{
	int		i;
	uint16	uiWord;

	for (i = 0; i < iWords; i++)
	{
		if (ConstantWord(sValue, i, &uiWord) && (uiWord == 0))
		{
			Frame("stz", iDestination + 2 * i);
			continue;
		}
		Operand("lda", sValue, i);
		Frame("sta", iDestination + 2 * i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether word iWord of sValue is a constant known now (*puiWord): a
// constant's word, or 0 past the end of a temporary or a constant.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::ConstantWord(SIRReference sValue, int iWord, uint16* puiWord)
{
	if (sValue.IsConstant())
	{
		*puiWord = (iWord < 4) ? (uint16)(mpcFunction->GetConstant(sValue)->ulliBits >> (16 * iWord)) : 0;
		return true;
	}
	if (sValue.IsTemporary() && (iWord >= Words(mpcFunction->GetTemporaryType(sValue))))
	{
		*puiWord = 0;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ZeroWords(int32 iDestination, int iFirst, int iWords)
{
	int		i;

	for (i = 0; i < iWords; i++)
	{
		Frame("stz", iDestination + 2 * (iFirst + i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Fills iWords words from iFirst with the sign of the word before them.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FillSign(int32 iDestination, int iFirst, int iWords)
{
	int		i;

	if (iWords <= 0)
	{
		return;
	}
	Frame("lda", iDestination + 2 * (iFirst - 1));
	Line("asl A");
	Line("lda 0");
	Line("adc $FFFF");
	Line("eor $FFFF");
	for (i = 0; i < iWords; i++)
	{
		Frame("sta", iDestination + 2 * (iFirst + i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes a value's words, the high word first so it is little endian on the stack.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PushValue(SIRReference sValue, SIRType sType)
{
	int		i;

	for (i = Words(sType) - 1; i >= 0; i--)
	{
		if (sValue.IsTemporary() && (i < Words(mpcFunction->GetTemporaryType(sValue))) && ((OffsetOf(sValue) + 2 * i <= 255) || InX(sValue)))
		{
			//pei pushes a direct page word without A (Peephole makes it pha when A holds it).
			Operand("pei", sValue, i);
		}
		else if (sValue.IsTemporary())
		{
			Operand("lda", sValue, i);
			Line("pha");
		}
		else
		{
			Operand("pea", sValue, i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the 24 bit address (in bank 0) of the frame's byte at iOffset.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PushFrameAddress(int32 iOffset)
{
	Line("pea 0");
	Line("tdc");
	Line("clc");
	Line("adc ", iOffset);
	Line("pha");
}


//////////////////////////////////////////////////////////////////////////
//
// Where a load or store's pointer is: a symbol (long addresses) or a place in the direct page ([d],y needs it within
// the first 256 bytes, else it is copied to the scratch pointer), and the constant a folded pointer adds to it.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PointerPlace(SIRReference sPointer, int32* piPlace, char** pszSymbol, int32* piOffset)
{
	*piOffset = 0;
	if (!FoldedBase(sPointer).Equals(sPointer))
	{
		*piOffset = *maiFoldOffset.Get(sPointer.Index());
		sPointer = FoldedBase(sPointer);
	}
	*pszSymbol = NULL;
	*piPlace = IR816_SCRATCH_POINTER;
	if (sPointer.IsSymbol())
	{
		*pszSymbol = mpcModule->GetSymbolName(sPointer.Index());
		return;
	}
	if (sPointer.IsTemporary() && (OffsetOf(sPointer) <= 253))
	{
		*piPlace = OffsetOf(sPointer);
		return;
	}
	CopyWords(IR816_SCRATCH_POINTER, sPointer, 2);
}


//////////////////////////////////////////////////////////////////////////
//
// The function's code from uiStart without its frame when it never reads
// or writes the frame's bytes (D + 1 to D + frame size; its temporaries may
// all have been kept in A or fused) or D but to move S back: S isn't moved
// down and back, so D is S and each D + n is n - frame size.  Without any
// D + n, D isn't saved and set either.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::DropFrame(size uiStart)
{
	CChars		sz;
	CChars		szPrologue;
	CChars		szEpilogue;
	const char*	szCode;
	const char*	szAt;
	int32		iOffset;
	int			iEpilogues;
	int			iTdcs;
	bool		bUsesD;

	szCode = mpszOut->Text() + uiStart;
	if ((miFrameBytes == 0) || (strstr(szCode, "[D + X]") != NULL))
	{
		return;
	}
	for (szAt = strstr(szCode, "D + "); szAt; szAt = strstr(szAt + 4, "D + "))
	{
		if (atoi(szAt + 4) <= miFrameBytes)
		{
			return;
		}
	}
	szPrologue.Init("\t\tphd\n\t\ttsc\n\t\tsec\n\t\tsbc ");
	szPrologue.Append(miFrameBytes);
	szPrologue.Append("\n\t\ttcs\n\t\ttcd\n");
	szEpilogue.Init("\t\ttay\n\t\ttdc\n\t\tclc\n\t\tadc ");
	szEpilogue.Append(miFrameBytes);
	szEpilogue.Append("\n\t\ttcs\n\t\ttya\n\t\tpld\n");
	iEpilogues = 0;
	for (szAt = strstr(szCode, szEpilogue.Text()); szAt; szAt = strstr(szAt + 1, szEpilogue.Text()))
	{
		iEpilogues++;
	}
	iTdcs = 0;
	for (szAt = strstr(szCode, "\t\ttdc\n"); szAt; szAt = strstr(szAt + 1, "\t\ttdc\n"))
	{
		iTdcs++;
	}
	if ((iTdcs != iEpilogues) || (strstr(szCode, szPrologue.Text()) == NULL))
	{
		szPrologue.Kill();
		szEpilogue.Kill();
		return;
	}

	//Without D + anything it needn't set D either.
	bUsesD = strstr(szCode, "D + ") != NULL;
	sz.Init();
	while (*szCode)
	{
		if (strncmp(szCode, szPrologue.Text(), szPrologue.Length()) == 0)
		{
			sz.Append(bUsesD ? "\t\tphd\n\t\ttsc\n\t\ttcd\n" : "");
			szCode += szPrologue.Length();
		}
		else if (strncmp(szCode, szEpilogue.Text(), szEpilogue.Length()) == 0)
		{
			sz.Append(bUsesD ? "\t\tpld\n" : "");
			szCode += szEpilogue.Length();
		}
		else if (strncmp(szCode, "D + ", 4) == 0)
		{
			iOffset = atoi(szCode + 4);
			sz.Append("D + ");
			sz.Append(iOffset - miFrameBytes);
			for (szCode += 4; (*szCode >= '0') && (*szCode <= '9'); szCode++)
			{
			}
		}
		else
		{
			sz.Append(*szCode);
			szCode++;
		}
	}
	mpszOut->RemoveEnd(uiStart);
	mpszOut->Append(sz);
	sz.Kill();
	szPrologue.Kill();
	szEpilogue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors)
{
	size		uiStart;

	mpcFunction = pcFunction;
	mpszOut = pszDest;
	mpcErrors = pcErrors;
	mbResult = true;
	uiStart = pszDest->Length();

	pcFunction->Renumber();
	Layout();
	if (miFrameBytes > IR816_MAX_FRAME_BYTES)
	{
		CChars	szBytes;

		szBytes.Init();
		szBytes.Append(miFrameBytes);
		Error("The frame is too big for the W65C816's stack (bytes)", szBytes.Text());
		szBytes.Kill();
		return false;
	}
	GenerateBody(szLabel, uiStart);
	DropFrame(uiStart);
	pszDest->AppendNewLine();
	return mbResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's code after Layout: the prologue, each block's
// instructions (those CanFuse finds read by the next as it goes) and jump,
// then Peephole from uiStart.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::GenerateBody(const char* szLabel, size uiStart)
{
	size		ui;
	size		uiInstruction;
	CIRBlock*	pcBlock;
	CIRBlock*	pcNext;
	int32		iLabel;

	maiBlockLabels.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		iLabel = NewLabel();
		maiBlockLabels.Add(&iLabel);
	}

	FindSingleUses();
	Prologue(szLabel);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		pcNext = (ui + 1 < mpcFunction->NumBlocks()) ? mpcFunction->GetBlock(ui + 1) : NULL;
		miBlock = (int32)ui;
		Label(*maiBlockLabels.Get(ui));
		PhiCopies(pcBlock);
		maiPendingArguments.FakeSetUsedElements(0);
		miFused = 0;
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			if (CanFuse(pcBlock, uiInstruction))
			{
				mapsFused[miFused] = pcBlock->masInstructions.Get(uiInstruction);
				miFused++;
				continue;
			}
			Instruction(pcBlock, uiInstruction);
			miFused = 0;
		}
		Jump(pcBlock, pcNext);
		miFused = 0;
	}
	miBlock = -1;
	Peephole(mpszOut, uiStart);
}


//////////////////////////////////////////////////////////////////////////
//
// The temporaries used once, by an instruction or a jump (not a phi): the
// block of that use (maiUseBlock).
//
//////////////////////////////////////////////////////////////////////////
static void IR816UseIn(CArrayTemplate<int32>* paiUseBlock, CArrayTemplate<int32>* paiCount, SIRReference sValue, int32 iBlock)
{
	if (!sValue.IsTemporary())
	{
		return;
	}
	(*paiCount->Get(sValue.Index()))++;
	*paiUseBlock->Get(sValue.Index()) = iBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FindSingleUses(void)
{
	CArrayTemplate<int32>	aiCount;
	CIRBlock*				pcBlock;
	SIRPhi*					psPhi;
	SIRInstruction*			psInstruction;
	size					ui;
	size					uiIndex;
	size					uiArgument;

	aiCount.Init();
	aiCount.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	maiUseBlock.FakeSetUsedElements(0);
	maiUseBlock.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0xFF);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				//Twice: a phi's use isn't in a block's straight line code.
				IR816UseIn(&maiUseBlock, &aiCount, psPhi->asArguments.Get(uiArgument)->sValue, -1);
				IR816UseIn(&maiUseBlock, &aiCount, psPhi->asArguments.Get(uiArgument)->sValue, -1);
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			IR816UseIn(&maiUseBlock, &aiCount, psInstruction->asArguments[0], (int32)ui);
			IR816UseIn(&maiUseBlock, &aiCount, psInstruction->asArguments[1], (int32)ui);
		}
		IR816UseIn(&maiUseBlock, &aiCount, pcBlock->msJump.sArgument, (int32)ui);
	}
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		if (*aiCount.Get(ui) != 1)
		{
			*maiUseBlock.Get(ui) = -1;
		}
	}
	aiCount.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the line after szEnd (the end of a line) branches on Z or N.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816BranchesOnZN(const char* szEnd)
{
	if (*szEnd != '\n')
	{
		return false;
	}
	szEnd++;
	return (strncmp(szEnd, "\t\tbeq", 5) == 0) || (strncmp(szEnd, "\t\tbne", 5) == 0) || (strncmp(szEnd, "\t\tbpl", 5) == 0) || (strncmp(szEnd, "\t\tbmi", 5) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the line at szLine is a branch on the carry.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816BranchesOnCarry(const char* szLine)
{
	return (strncmp(szLine, "\t\tbcc", 5) == 0) || (strncmp(szLine, "\t\tbcs", 5) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The first instruction at or after szLine, past labels and directives
// (.width follows every label); NULL at the end.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816NextInstruction(const char* szLine)
{
	const char*	szEnd;

	while (szLine && *szLine)
	{
		if ((szLine[0] == '\t') && (szLine[1] == '\t') && (szLine[2] != '.') && (szLine[2] != '\0') && (szLine[2] != '\n'))
		{
			return szLine;
		}
		szEnd = strchr(szLine, '\n');
		szLine = szEnd ? szEnd + 1 : NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the run of conditional branches from the instruction at or after
// szLine (past labels and directives) has a branch on the carry: a 32 bit
// unsigned compare branches on Z then C ("L: beq L2 / bcs L3").
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CarryReadAt(const char* szLine)
{
	const char*	szEnd;

	for (szLine = IR816NextInstruction(szLine); szLine; szLine = IR816NextInstruction(szEnd + 1))
	{
		if (IR816BranchesOnCarry(szLine))
		{
			return true;
		}
		if ((strncmp(szLine, "\t\tbeq", 5) != 0) && (strncmp(szLine, "\t\tbne", 5) != 0) && (strncmp(szLine, "\t\tbmi", 5) != 0) &&
			(strncmp(szLine, "\t\tbpl", 5) != 0) && (strncmp(szLine, "\t\tbvs", 5) != 0) && (strncmp(szLine, "\t\tbvc", 5) != 0))
		{
			return false;
		}
		szEnd = strchr(szLine, '\n');
		if (szEnd == NULL)
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the carry may be read after the branch on the line after szEnd:
// after it, or where it branches to (IR816CarryReadAt; branching on the
// carry is the only way the code reads a carry it didn't set).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CarryReadAfterBranch(const char* szText, const char* szEnd)
{
	const char*	szBranch;
	const char*	szAfter;
	const char*	szLabel;
	const char*	szFound;
	char		szTarget[40];
	size		uiLabel;

	szBranch = szEnd + 1;
	szAfter = strchr(szBranch, '\n');
	if ((szAfter == NULL) || IR816CarryReadAt(szAfter + 1))
	{
		return true;
	}

	//"\t\tbne L63": the label's line is "L63:".
	szLabel = szBranch + 6;
	uiLabel = (size)(szAfter - szLabel);
	if ((uiLabel == 0) || (uiLabel + 3 > sizeof(szTarget)))
	{
		return true;
	}
	szTarget[0] = '\n';
	memcpy(&szTarget[1], szLabel, uiLabel);
	szTarget[1 + uiLabel] = ':';
	szTarget[2 + uiLabel] = '\n';
	szTarget[3 + uiLabel] = '\0';
	szFound = strstr(szText, szTarget);
	if (szFound == NULL)
	{
		return true;
	}
	return IR816CarryReadAt(szFound + uiLabel + 3);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the mnemonic sets N and Z from all of A (as wide as it is).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SetsZNFromA(const char* szMnemonic, const char* szOperand, size uiOperand)
{
	static const char*	aszMnemonics[] = { "lda", "and", "ora", "eor", "adc", "sbc", "txa", "tya", "tdc", "pla", NULL };
	int					i;

	for (i = 0; aszMnemonics[i]; i++)
	{
		if (strcmp(szMnemonic, aszMnemonics[i]) == 0)
		{
			return true;
		}
	}
	if ((uiOperand == 1) && (szOperand[0] == 'A'))
	{
		return (strcmp(szMnemonic, "asl") == 0) || (strcmp(szMnemonic, "lsr") == 0) || (strcmp(szMnemonic, "rol") == 0) ||
			   (strcmp(szMnemonic, "ror") == 0) || (strcmp(szMnemonic, "inc") == 0) || (strcmp(szMnemonic, "dec") == 0);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The mnemonic of the instruction at szLine (a line starting with two tabs)
// into szMnemonic, and its operand (to the end of the line, less a mark).
//
//////////////////////////////////////////////////////////////////////////
static void IR816SplitLine(const char* szLine, char* szMnemonic, const char** pszOperand, size* puiOperand)
{
	const char*	szEnd;
	size		uiLength;
	int			i;

	szEnd = strchr(szLine, '\n');
	uiLength = szEnd ? (size)(szEnd - szLine) : strlen(szLine);
	if ((uiLength >= strlen(IR816_KEEP_IN_A)) && (strncmp(szLine + uiLength - strlen(IR816_KEEP_IN_A), IR816_KEEP_IN_A, strlen(IR816_KEEP_IN_A)) == 0))
	{
		uiLength -= strlen(IR816_KEEP_IN_A);
	}
	for (i = 0; (i < 7) && (2 + (size)i < uiLength) && (szLine[2 + i] != ' '); i++)
	{
		szMnemonic[i] = szLine[2 + i];
	}
	szMnemonic[i] = '\0';
	*pszOperand = szLine + 2 + i + ((2 + (size)i < uiLength) ? 1 : 0);
	*puiOperand = (size)(szLine + uiLength - *pszOperand);
}


//////////////////////////////////////////////////////////////////////////
//
// The line after the one at szLine; NULL at the end.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816NextLine(const char* szLine)
{
	const char*	szEnd;

	szEnd = strchr(szLine, '\n');
	return (szEnd && szEnd[1]) ? szEnd + 1 : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether N or Z may be read after the line at szLine before an
// instruction sets them again (stores and pushes don't; a label or the
// end might lead to a read).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ZNReadAfter(const char* szLine)
{
	char		szMnemonic[8];
	const char*	szOperand;
	size		uiOperand;

	for (szLine = IR816NextLine(szLine); szLine; szLine = IR816NextLine(szLine))
	{
		if ((szLine[0] != '\t') || (szLine[1] != '\t') || (szLine[2] == '.'))
		{
			return true;
		}
		IR816SplitLine(szLine, szMnemonic, &szOperand, &uiOperand);
		if ((strcmp(szMnemonic, "beq") == 0) || (strcmp(szMnemonic, "bne") == 0) || (strcmp(szMnemonic, "bmi") == 0) || (strcmp(szMnemonic, "bpl") == 0))
		{
			return true;
		}
		if ((strcmp(szMnemonic, "sta") != 0) && (strcmp(szMnemonic, "stx") != 0) && (strcmp(szMnemonic, "sty") != 0) &&
			(strcmp(szMnemonic, "stz") != 0) && (strcmp(szMnemonic, "pha") != 0) && (strcmp(szMnemonic, "phx") != 0) &&
			(strcmp(szMnemonic, "phy") != 0) && (strcmp(szMnemonic, "clc") != 0) && (strcmp(szMnemonic, "sec") != 0))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value stored to the word szWord (as "[D + 12]") by the line
// before szLine can stay in A instead: the next line that mentions the word
// loads it (and nothing after the load reads the N and Z it sets) or pushes
// it (pei, which becomes pha), and nothing before that changes A or goes
// anywhere.  Its one use (Store) is that load or push.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816KeepsInA(const char* szLine, const char* szWord, size uiWord)
{
	char		szMnemonic[8];
	const char*	szOperand;
	size		uiOperand;
	size		ui;
	bool		bMentions;

	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if ((szLine[0] != '\t') || (szLine[1] != '\t') || (szLine[2] == '.'))
		{
			return false;
		}
		IR816SplitLine(szLine, szMnemonic, &szOperand, &uiOperand);
		bMentions = false;
		for (ui = 0; !bMentions && (ui + uiWord <= uiOperand); ui++)
		{
			bMentions = strncmp(szOperand + ui, szWord, uiWord) == 0;
		}
		if (bMentions)
		{
			return (uiOperand == uiWord) && (((strcmp(szMnemonic, "lda") == 0) && !IR816ZNReadAfter(szLine)) || (strcmp(szMnemonic, "pei") == 0));
		}
		if ((strcmp(szMnemonic, "clc") != 0) && (strcmp(szMnemonic, "sec") != 0) && (strcmp(szMnemonic, "ldx") != 0) &&
			(strcmp(szMnemonic, "ldy") != 0) && (strcmp(szMnemonic, "tax") != 0) && (strcmp(szMnemonic, "tay") != 0) &&
			(strcmp(szMnemonic, "pha") != 0) && (strcmp(szMnemonic, "pea") != 0) && (strcmp(szMnemonic, "phk") != 0) &&
			(strcmp(szMnemonic, "pei") != 0) &&
			(strcmp(szMnemonic, "cmp") != 0) && (strcmp(szMnemonic, "sta") != 0) && (strcmp(szMnemonic, "stx") != 0) &&
			(strcmp(szMnemonic, "sty") != 0) && (strcmp(szMnemonic, "stz") != 0))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operand may read or write the direct page bytes near
// iOffset: it names [D + n] with n within 2 of it (a pointer is 3 bytes;
// words may start at either byte), or [D + X].
//
//////////////////////////////////////////////////////////////////////////
static bool IR816MentionsNear(const char* szOperand, size uiOperand, int32 iOffset)
{
	size	ui;
	int32	iValue;
	size	uiDigit;

	for (ui = 0; ui + 5 <= uiOperand; ui++)
	{
		if (strncmp(szOperand + ui, "D + ", 4) != 0)
		{
			continue;
		}
		if (szOperand[ui + 4] == 'X')
		{
			return true;
		}
		iValue = 0;
		for (uiDigit = ui + 4; (uiDigit < uiOperand) && (szOperand[uiDigit] >= '0') && (szOperand[uiDigit] <= '9'); uiDigit++)
		{
			iValue = iValue * 10 + (szOperand[uiDigit] - '0');
		}
		if ((iValue >= iOffset - 2) && (iValue <= iOffset + 2))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a store to the direct page word szWord (as "[D + 12]") by the line
// before szLine is never read: in straight line code (no label, jump, call
// or width change) the next instruction that names a byte near it stores a
// word to it.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816DeadStore(const char* szLine, const char* szWord, size uiWord)
{
	char		szMnemonic[8];
	const char*	szOperand;
	size		uiOperand;
	int32		iOffset;

	iOffset = atoi(szWord + 5);
	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if ((szLine[0] != '\t') || (szLine[1] != '\t') || (szLine[2] == '.'))
		{
			return false;
		}
		IR816SplitLine(szLine, szMnemonic, &szOperand, &uiOperand);
		if ((szMnemonic[0] == 'b') || (szMnemonic[0] == 'j') || (strncmp(szMnemonic, "rt", 2) == 0) || (strcmp(szMnemonic, "sep") == 0) ||
			(strcmp(szMnemonic, "rep") == 0) || (strcmp(szMnemonic, "mvn") == 0) || (strcmp(szMnemonic, "mvp") == 0) || (strcmp(szMnemonic, "pld") == 0) ||
			(strcmp(szMnemonic, "tcd") == 0) || (strcmp(szMnemonic, "brk") == 0) || (strcmp(szMnemonic, "cop") == 0))
		{
			return false;
		}
		if (IR816MentionsNear(szOperand, uiOperand, iOffset))
		{
			return ((strcmp(szMnemonic, "sta") == 0) || (strcmp(szMnemonic, "stz") == 0) || (strcmp(szMnemonic, "stx") == 0) || (strcmp(szMnemonic, "sty") == 0)) &&
				   (uiOperand == uiWord) && (strncmp(szOperand, szWord, uiWord) == 0);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an lda before the line at szLine is never read: in straight line
// code, before anything reads A or the N and Z the lda set, an instruction
// sets A again (only instructions that read neither come between).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816DeadLoad(const char* szLine)
{
	char		szMnemonic[8];
	const char*	szOperand;
	size		uiOperand;
	bool		bMemory;

	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if ((szLine[0] != '\t') || (szLine[1] != '\t') || (szLine[2] == '.'))
		{
			return false;
		}
		IR816SplitLine(szLine, szMnemonic, &szOperand, &uiOperand);
		if ((strcmp(szMnemonic, "lda") == 0) || (strcmp(szMnemonic, "pla") == 0) || (strcmp(szMnemonic, "txa") == 0) ||
			(strcmp(szMnemonic, "tya") == 0) || (strcmp(szMnemonic, "tdc") == 0) || (strcmp(szMnemonic, "tsc") == 0))
		{
			return true;
		}
		bMemory = (uiOperand > 0) && (szOperand[0] == '[');
		if ((strcmp(szMnemonic, "clc") != 0) && (strcmp(szMnemonic, "sec") != 0) && (strcmp(szMnemonic, "ldx") != 0) &&
			(strcmp(szMnemonic, "ldy") != 0) && (strcmp(szMnemonic, "stz") != 0) && (strcmp(szMnemonic, "stx") != 0) &&
			(strcmp(szMnemonic, "sty") != 0) && (strcmp(szMnemonic, "pea") != 0) && (strcmp(szMnemonic, "phk") != 0) &&
			(strcmp(szMnemonic, "pei") != 0) &&
			(strcmp(szMnemonic, "phx") != 0) && (strcmp(szMnemonic, "phy") != 0) && (strcmp(szMnemonic, "cpx") != 0) &&
			(strcmp(szMnemonic, "cpy") != 0) && (strcmp(szMnemonic, "inx") != 0) && (strcmp(szMnemonic, "iny") != 0) &&
			(strcmp(szMnemonic, "dex") != 0) && (strcmp(szMnemonic, "dey") != 0) &&
			!(bMemory && ((strcmp(szMnemonic, "inc") == 0) || (strcmp(szMnemonic, "dec") == 0) || (strcmp(szMnemonic, "asl") == 0) ||
						  (strcmp(szMnemonic, "lsr") == 0) || (strcmp(szMnemonic, "rol") == 0) || (strcmp(szMnemonic, "ror") == 0))))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value stored to the word szWord by the line before szLine can
// stay in A as the first operand of a commutative instruction instead (adc,
// and, ora or eor of the word straight after an lda of something else, with
// only clc or sec before it): *pszLoad is that lda's line and *pszUse the
// instruction's, which becomes the instruction on the lda's operand.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SwapsInA(const char* szLine, const char* szWord, size uiWord, const char** pszLoad, const char** pszUse)
{
	const char*	szLoad;
	char		szMnemonic[8];
	const char*	szOperand;
	size		uiOperand;
	size		ui;

	while (szLine && ((strncmp(szLine, "\t\tclc\n", 6) == 0) || (strncmp(szLine, "\t\tsec\n", 6) == 0)))
	{
		szLine = IR816NextLine(szLine);
	}
	if (!szLine || (strncmp(szLine, "\t\tlda ", 6) != 0))
	{
		return false;
	}
	szLoad = szLine;
	IR816SplitLine(szLine, szMnemonic, &szOperand, &uiOperand);
	for (ui = 0; ui + uiWord <= uiOperand; ui++)
	{
		if (strncmp(szOperand + ui, szWord, uiWord) == 0)
		{
			return false;
		}
	}
	if ((uiOperand == 0) || (uiOperand > 40) || (szOperand[uiOperand - 1] == 'A') || (strchr(szOperand, ';') && (strchr(szOperand, ';') < szOperand + uiOperand)))
	{
		return false;
	}
	szLine = IR816NextLine(szLine);
	if (!szLine || ((szLine[0] != '\t') || (szLine[1] != '\t')))
	{
		return false;
	}
	IR816SplitLine(szLine, szMnemonic, &szOperand, &uiOperand);
	if (((strcmp(szMnemonic, "adc") != 0) && (strcmp(szMnemonic, "and") != 0) && (strcmp(szMnemonic, "ora") != 0) && (strcmp(szMnemonic, "eor") != 0)) ||
		(uiOperand != uiWord) || (strncmp(szOperand, szWord, uiWord) != 0))
	{
		return false;
	}
	*pszLoad = szLoad;
	*pszUse = szLine;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes instructions that change nothing from the code written from
// uiStart on, in straight line code (a label, a call or a width change
// forgets what is known): a value's store and its one load when A keeps it
// between them (IR816KeepsInA, for a store Store marked), an lda of the
// direct page word A already has (but
// not before a branch on its Z or N), an sta of it back to that word, an lda
// of a direct page word or a constant that the next lda replaces, an ldy of
// the constant Y already has, adc 0 after clc and an lda (the carry stays
// clear), and cmp 0 after an instruction that set N and Z from A when only a
// branch on Z or N follows (not a branch on the carry it sets, after the
// branch or where it goes).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Peephole(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szText;
	const char*	szLine;
	const char*	szEnd;
	const char*	szOperand;
	char		szA[32];		//The direct page operand A holds ("[D + 12]"), or empty.
	char		szKept[32];		//The word whose store was left out, its value kept in A for its next lda, or empty.
	const char*	szSwapLoad;		//An lda left out: the value kept in A is the first operand instead (IR816SwapsInA)...
	const char*	szSwapUse;		//...of this instruction, which takes the lda's operand.
	char		szSwapped[64];
	char		szY[32];		//The constant Y holds, or empty.
	char		szMnemonic[8];
	size		uiLength;
	size		uiOperand;
	size		uiDeadLoad;		//Where in szOut the last line starts if it is an lda of a direct page word or a constant, else -1.
	bool		bClearCarry;
	bool		bZNFromA;		//The last instruction kept set N and Z from A.
	bool		bKeep;
	bool		bDirect;
	bool		bMarked;

	szText = pszDest->Text() + uiStart;
	szOut.Init();
	szA[0] = '\0';
	szY[0] = '\0';
	szKept[0] = '\0';
	szSwapLoad = NULL;
	szSwapUse = NULL;
	bClearCarry = false;
	bZNFromA = false;
	uiDeadLoad = (size)-1;
	for (szLine = szText; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = strchr(szLine, '\n');
		if (!szEnd)
		{
			szEnd = szLine + strlen(szLine);
		}
		uiLength = (size)(szEnd - szLine);
		bKeep = true;

		//A label, or a directive: nothing is known after it.
		if ((uiLength < 3) || (szLine[0] != '\t') || (szLine[1] != '\t') || (szLine[2] == '.'))
		{
			szA[0] = '\0';
			szY[0] = '\0';
			bClearCarry = false;
			bZNFromA = false;
			uiDeadLoad = (size)-1;
			szOut.Append(szLine, uiLength);
			szOut.AppendNewLine();
			continue;
		}

		//The mnemonic and the operand.
		bMarked = (uiLength >= strlen(IR816_KEEP_IN_A)) && (strncmp(szEnd - strlen(IR816_KEEP_IN_A), IR816_KEEP_IN_A, strlen(IR816_KEEP_IN_A)) == 0);
		if (bMarked)
		{
			uiLength -= strlen(IR816_KEEP_IN_A);
		}
		IR816SplitLine(szLine, szMnemonic, &szOperand, &uiOperand);
		bDirect = (uiOperand < 30) && (strncmp(szOperand, "[D + ", 5) == 0) && (szOperand[5] >= '0') && (szOperand[5] <= '9') && (szOperand[uiOperand - 1] == ']');

		//A value kept in A instead of its store and load, or as the first operand of a commutative instruction.
		if (bMarked && bDirect && (strcmp(szMnemonic, "sta") == 0) && (*szEnd == '\n') && IR816KeepsInA(szEnd + 1, szOperand, uiOperand))
		{
			memcpy(szKept, szOperand, uiOperand);
			szKept[uiOperand] = '\0';
			continue;
		}
		if (bMarked && bDirect && (szKept[0] == '\0') && (strcmp(szMnemonic, "sta") == 0) && (*szEnd == '\n') && IR816SwapsInA(szEnd + 1, szOperand, uiOperand, &szSwapLoad, &szSwapUse))
		{
			continue;
		}
		if (szLine == szSwapLoad)
		{
			//The lda's operand goes to the instruction it was loaded for.
			snprintf(szSwapped, sizeof(szSwapped), "%.*s", (int)uiOperand, szOperand);
			szSwapLoad = NULL;
			continue;
		}
		if (szLine == szSwapUse)
		{
			szOut.Append("\t\t");
			szOut.Append(szMnemonic);
			szOut.Append(" ");
			szOut.Append(szSwapped);
			szOut.AppendNewLine();
			szSwapUse = NULL;
			szA[0] = '\0';
			bClearCarry = false;
			bZNFromA = true;
			uiDeadLoad = (size)-1;
			continue;
		}
		if (((strcmp(szMnemonic, "lda") == 0) || (strcmp(szMnemonic, "pei") == 0)) && (strlen(szKept) == uiOperand) && (strncmp(szKept, szOperand, uiOperand) == 0))
		{
			szKept[0] = '\0';
			if (strcmp(szMnemonic, "pei") == 0)
			{
				szOut.Append("\t\tpha\n");
				uiDeadLoad = (size)-1;
			}
			continue;
		}
		if ((strcmp(szMnemonic, "pei") == 0) && (strlen(szA) == uiOperand) && (strncmp(szA, szOperand, uiOperand) == 0))
		{
			//A holds the word: pha pushes it for less.
			szOut.Append("\t\tpha\n");
			uiDeadLoad = (size)-1;
			continue;
		}

		if ((strcmp(szMnemonic, "lda") == 0) && bDirect && (strlen(szA) == uiOperand) && (strncmp(szA, szOperand, uiOperand) == 0) && !IR816BranchesOnZN(szEnd))
		{
			bKeep = false;
		}
		else if ((strcmp(szMnemonic, "ldy") == 0) && (uiOperand < 30) && (strlen(szY) == uiOperand) && (strncmp(szY, szOperand, uiOperand) == 0))
		{
			bKeep = false;
		}
		else if ((strcmp(szMnemonic, "adc") == 0) && (uiOperand == 1) && (szOperand[0] == '0') && bClearCarry)
		{
			bKeep = false;
		}
		else if ((strcmp(szMnemonic, "sta") == 0) && bDirect && (strlen(szA) == uiOperand) && (strncmp(szA, szOperand, uiOperand) == 0))
		{
			//The word already holds A.
			bKeep = false;
		}
		else if ((strcmp(szMnemonic, "cmp") == 0) && (uiOperand == 1) && (szOperand[0] == '0') && bZNFromA && IR816BranchesOnZN(szEnd) &&
				 !IR816CarryReadAfterBranch(szText, szEnd))
		{
			bKeep = false;
		}
		if (!bKeep)
		{
			continue;
		}
		if ((strcmp(szMnemonic, "lda") == 0) && (uiDeadLoad != (size)-1))
		{
			//The lda before it loaded A for nothing.
			szOut.SetLength(uiDeadLoad);
		}
		uiDeadLoad = ((strcmp(szMnemonic, "lda") == 0) && (bDirect || ((szOperand[0] >= '0') && (szOperand[0] <= '9')))) ? szOut.Length() : (size)-1;
		szOut.Append(szLine, uiLength);
		szOut.AppendNewLine();
		if ((strcmp(szMnemonic, "sta") != 0) && (strcmp(szMnemonic, "stx") != 0) && (strcmp(szMnemonic, "sty") != 0) &&
			(strcmp(szMnemonic, "stz") != 0) && (strcmp(szMnemonic, "pha") != 0))
		{
			//Stores change neither A nor the flags.
			bZNFromA = IR816SetsZNFromA(szMnemonic, szOperand, uiOperand);
		}

		//What A and Y hold after it.
		if (((strcmp(szMnemonic, "lda") == 0) || (strcmp(szMnemonic, "sta") == 0)) && bDirect)
		{
			memcpy(szA, szOperand, uiOperand);
			szA[uiOperand] = '\0';
		}
		else if ((strcmp(szMnemonic, "ldx") == 0) || (strcmp(szMnemonic, "clc") == 0) ||
				 (strcmp(szMnemonic, "sec") == 0) || (strcmp(szMnemonic, "cmp") == 0) || (strcmp(szMnemonic, "tax") == 0) ||
				 (strcmp(szMnemonic, "tay") == 0) || (strcmp(szMnemonic, "pha") == 0) || (strcmp(szMnemonic, "pea") == 0) ||
				 (strcmp(szMnemonic, "per") == 0) || (strcmp(szMnemonic, "phk") == 0) || (strcmp(szMnemonic, "ldy") == 0) ||
				 (strcmp(szMnemonic, "beq") == 0) || (strcmp(szMnemonic, "bne") == 0) || (strcmp(szMnemonic, "bcc") == 0) ||
				 (strcmp(szMnemonic, "bcs") == 0) || (strcmp(szMnemonic, "bpl") == 0) || (strcmp(szMnemonic, "bmi") == 0))
		{
			//A and the word it holds are kept.
		}
		else if (((strcmp(szMnemonic, "sta") == 0) || (strcmp(szMnemonic, "stx") == 0) || (strcmp(szMnemonic, "sty") == 0) || (strcmp(szMnemonic, "stz") == 0) ||
				  (strcmp(szMnemonic, "inc") == 0) || (strcmp(szMnemonic, "dec") == 0) || (strcmp(szMnemonic, "asl") == 0) ||
				  (strcmp(szMnemonic, "lsr") == 0) || (strcmp(szMnemonic, "rol") == 0) || (strcmp(szMnemonic, "ror") == 0)) &&
				 !((uiOperand == 1) && (szOperand[0] == 'A')))
		{
			//Memory changes: A no longer matches it if it is the word A holds (a store through a pointer might be, to an address taken local).
			if (!bDirect || ((strlen(szA) == uiOperand) && (strncmp(szA, szOperand, uiOperand) == 0)))
			{
				szA[0] = '\0';
			}
		}
		else
		{
			szA[0] = '\0';
		}

		if ((strcmp(szMnemonic, "ldy") == 0) && (uiOperand < 30) && (szOperand[0] >= '0') && (szOperand[0] <= '9'))
		{
			memcpy(szY, szOperand, uiOperand);
			szY[uiOperand] = '\0';
		}
		else if ((strcmp(szMnemonic, "ldy") == 0) || (strcmp(szMnemonic, "tay") == 0) || (strcmp(szMnemonic, "iny") == 0) ||
				 (strcmp(szMnemonic, "dey") == 0) || (strcmp(szMnemonic, "ply") == 0) || (strcmp(szMnemonic, "jsl") == 0) ||
				 (strcmp(szMnemonic, "jsr") == 0) || (strcmp(szMnemonic, "rtl") == 0) || (strcmp(szMnemonic, "sep") == 0) ||
				 (strcmp(szMnemonic, "rep") == 0) || (strcmp(szMnemonic, "txy") == 0) || (strcmp(szMnemonic, "brl") == 0) ||
				 (strcmp(szMnemonic, "bra") == 0) || (strcmp(szMnemonic, "jml") == 0) ||
				 (strcmp(szMnemonic, "mvn") == 0) || (strcmp(szMnemonic, "mvp") == 0))
		{
			szY[0] = '\0';
		}
		if ((strcmp(szMnemonic, "sep") == 0) || (strcmp(szMnemonic, "rep") == 0) || (strcmp(szMnemonic, "jsl") == 0) ||
			(strcmp(szMnemonic, "jsr") == 0) || (strcmp(szMnemonic, "pld") == 0) || (strcmp(szMnemonic, "tcd") == 0))
		{
			szA[0] = '\0';
		}
		bClearCarry = (strcmp(szMnemonic, "clc") == 0) || (bClearCarry && (strcmp(szMnemonic, "lda") == 0));
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut.Text(), szOut.Length());
	szOut.Kill();
	RemoveDeadStores(pszDest, uiStart);
}


//////////////////////////////////////////////////////////////////////////
//
// After Peephole: stores to a direct page word that the next store to it
// overwrites unread (IR816DeadStore) go, and loads of a direct page word or
// a constant that A is loaded again over before it is read (IR816DeadLoad).
// Places are shared, so a value worked out in steps is often stored after
// each.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::RemoveDeadStores(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	const char*	szOperand;
	char		szMnemonic[8];
	size		uiOperand;
	bool		bDirect;

	szOut.Init();
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = strchr(szLine, '\n');
		if (!szEnd)
		{
			szEnd = szLine + strlen(szLine);
		}
		if ((szEnd - szLine >= 3) && (szLine[0] == '\t') && (szLine[1] == '\t') && (szLine[2] != '.') && (*szEnd == '\n'))
		{
			IR816SplitLine(szLine, szMnemonic, &szOperand, &uiOperand);
			bDirect = (uiOperand < 30) && (strncmp(szOperand, "[D + ", 5) == 0) && (szOperand[5] >= '0') && (szOperand[5] <= '9') && (szOperand[uiOperand - 1] == ']');
			if (bDirect && ((strcmp(szMnemonic, "sta") == 0) || (strcmp(szMnemonic, "stz") == 0) || (strcmp(szMnemonic, "stx") == 0) || (strcmp(szMnemonic, "sty") == 0)) &&
				IR816DeadStore(szEnd + 1, szOperand, uiOperand))
			{
				continue;
			}
			if ((strcmp(szMnemonic, "lda") == 0) && (bDirect || ((uiOperand > 0) && (szOperand[0] != '['))) && IR816DeadLoad(szEnd + 1))
			{
				continue;
			}
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut.Text(), szOut.Length());
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// D is pointed at the frame: the frame is D + 1 to D + frame size.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Prologue(const char* szLabel)
{
	mpszOut->Append(szLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
	Line(".width (A, X), 16");
	Line(".relax");
	Line("phd");
	Line("tsc");
	Line("sec");
	Line("sbc ", miFrameBytes);
	Line("tcs");
	Line("tcd");
}


//////////////////////////////////////////////////////////////////////////
//
// The result is in A (and X); Y keeps A while S is moved back.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Epilogue(void)
{
	Line("tay");
	Line("tdc");
	Line("clc");
	Line("adc ", miFrameBytes);
	Line("tcs");
	Line("tya");
	Line("pld");
	Line("rtl");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Return(CIRBlock* pcBlock)
{
	SIRJump*	psJump;
	int			i;
	int			iWords;

	psJump = &pcBlock->msJump;
	if (!psJump->sType.IsNone())
	{
		iWords = Words(psJump->sType);
		if (miHiddenResult >= 0)
		{
			Frame("lda", miHiddenResult);
			Frame("sta", IR816_SCRATCH_POINTER);
			Frame("lda", miHiddenResult + 2);
			Frame("sta", IR816_SCRATCH_POINTER + 2);
			for (i = 0; i < iWords; i++)
			{
				Line("ldy ", 2 * i);
				Operand("lda", psJump->sArgument, i);
				Line("sta [[D + 1]:24 + Y]");
			}
		}
		else if (Fused(psJump->sArgument))
		{
			//The high word first (CanFuseReturn).
			Operand("lda", psJump->sArgument, 1);
			Line("tax");
			Operand("lda", psJump->sArgument, 0);
		}
		else if (!psJump->sArgument.IsTemporary() || (OffsetOf(psJump->sArgument) + 2 * iWords <= 255))
		{
			//Straight into A and X.
			if (iWords > 1)
			{
				Operand("ldx", psJump->sArgument, 1);
			}
			Operand("lda", psJump->sArgument, 0);
		}
		else
		{
			CopyWords(IR816_SCRATCH_RESULT, psJump->sArgument, iWords);
			Frame("lda", IR816_SCRATCH_RESULT);
			if (iWords > 1)
			{
				Frame("ldx", IR816_SCRATCH_RESULT + 2);
			}
		}
	}
	Epilogue();
}


//////////////////////////////////////////////////////////////////////////
//
// A jump into a block with phis puts each phi's value in its way in; the block copies them to the phis as it starts,
// so phis that take each other's values (a swap) are all read before any is written.  A block whose phis don't
// (IsCoalescedPhiBlock) has each phi's place as its way in, and nothing to copy.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::EdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	size			ui;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRPhi*			psInX;

	//A phi kept in X is given its value last, so the copies before it can still read a value in X.
	psInX = NULL;
	for (ui = 0; ui < pcTo->masPhis.NumElements(); ui++)
	{
		psPhi = pcTo->masPhis.Get(ui);
		psArgument = pcTo->FindPhiArgument(psPhi, pcFrom);
		if (psArgument && InX(psPhi->sResult) && IsCoalescedPhiBlock(pcTo))
		{
			psInX = psPhi;
		}
		else if (psArgument)
		{
			CopyWords(*maiIncoming.Get(psPhi->sResult.Index()), psArgument->sValue, Words(psPhi->sType));
		}
	}
	if (psInX)
	{
		psArgument = pcTo->FindPhiArgument(psInX, pcFrom);
		Operand("ldx", psArgument->sValue, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a block's phis can be given their values in their own places on
// the way in: none of them takes another one's value (so writing one never
// loses a value another still needs).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsCoalescedPhiBlock(CIRBlock* pcBlock)
{
	size			ui;
	size			uj;
	size			uk;
	SIRPhi*			psPhi;
	SIRReference	sValue;

	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		psPhi = pcBlock->masPhis.Get(ui);
		for (uj = 0; uj < psPhi->asArguments.NumElements(); uj++)
		{
			sValue = psPhi->asArguments.Get(uj)->sValue;
			for (uk = 0; uk < pcBlock->masPhis.NumElements(); uk++)
			{
				if ((uk != ui) && sValue.Equals(pcBlock->masPhis.Get(uk)->sResult))
				{
					return false;
				}
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PhiCopies(CIRBlock* pcBlock)
{
	size		ui;
	SIRPhi*		psPhi;
	int			i;
	int32		iIncoming;

	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		psPhi = pcBlock->masPhis.Get(ui);
		iIncoming = *maiIncoming.Get(psPhi->sResult.Index());
		if (iIncoming == OffsetOf(psPhi->sResult))
		{
			continue;
		}
		for (i = 0; i < Words(psPhi->sType); i++)
		{
			Frame("lda", iIncoming + 2 * i);
			Store(psPhi->sResult, i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::JumpTo(CIRBlock* pcTo, CIRBlock* pcNext)
{
	if (pcTo == pcNext)
	{
		return;
	}
	Branch("bra", *maiBlockLabels.Get(pcTo->muiIndex));
}


//////////////////////////////////////////////////////////////////////////
//
// szMnemonic to label iLabel; the assembler makes it reach (.relax).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Branch(const char* szMnemonic, int32 iLabel)
{
	CChars	sz;

	sz.Init(szMnemonic);
	sz.Append(" L");
	sz.Append(iLabel);
	Line(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the jump from pcFrom to pcTo has phi values to copy.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::HasEdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	size	ui;

	for (ui = 0; ui < pcTo->masPhis.NumElements(); ui++)
	{
		if (pcTo->FindPhiArgument(pcTo->masPhis.Get(ui), pcFrom))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Branches to iLabel when the block's brif condition is bWhen: the fused
// comparison's flags (FusedBranch) or the condition's word.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::BranchWhen(CIRBlock* pcBlock, bool bWhen, int32 iLabel)
{
	if ((pcBlock->masInstructions.NumElements() > 0) && IsFusedCompare(pcBlock, pcBlock->masInstructions.NumElements() - 1))
	{
		FusedBranch(pcBlock->masInstructions.Get(pcBlock->masInstructions.NumElements() - 1), bWhen, iLabel);
	}
	else
	{
		Operand("lda", pcBlock->msJump.sArgument, 0);
		Branch(bWhen ? "bne" : "beq", iLabel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A brif skips a brl to its else edge when the condition isn't 0, so neither edge is limited to a short branch.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Jump(CIRBlock* pcBlock, CIRBlock* pcNext)
{
	SIRJump*	psJump;
	int32		iThen;
	bool		bThenCopies;
	bool		bElseCopies;

	psJump = &pcBlock->msJump;
	switch (psJump->uiKind)
	{
	case IRJ_Jump:
		EdgeCopies(pcBlock, psJump->pcThen);
		JumpTo(psJump->pcThen, pcNext);
		break;
	case IRJ_Branch:
		//Straight to a block whose edge has no copies, falling through to the next block where it can.
		bThenCopies = HasEdgeCopies(pcBlock, psJump->pcThen);
		bElseCopies = HasEdgeCopies(pcBlock, psJump->pcElse);
		if (!bElseCopies && ((psJump->pcThen == pcNext) || bThenCopies || (psJump->pcElse != pcNext)))
		{
			BranchWhen(pcBlock, false, *maiBlockLabels.Get(psJump->pcElse->muiIndex));
			EdgeCopies(pcBlock, psJump->pcThen);
			JumpTo(psJump->pcThen, pcNext);
		}
		else if (!bThenCopies)
		{
			BranchWhen(pcBlock, true, *maiBlockLabels.Get(psJump->pcThen->muiIndex));
			EdgeCopies(pcBlock, psJump->pcElse);
			JumpTo(psJump->pcElse, pcNext);
		}
		else
		{
			iThen = NewLabel();
			BranchWhen(pcBlock, true, iThen);
			EdgeCopies(pcBlock, psJump->pcElse);
			JumpTo(psJump->pcElse, NULL);
			Label(iThen);
			EdgeCopies(pcBlock, psJump->pcThen);
			JumpTo(psJump->pcThen, pcNext);
		}
		break;
	case IRJ_Return:
		Return(pcBlock);
		break;
	default:
		Line("stp");
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::Instruction(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	int				i;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
		if (FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult))
		{
			Arithmetic(psInstruction);
		}
		return true;
	case IRO_Sub:
		Arithmetic(psInstruction);
		return true;
	case IRO_And:
		Logic(psInstruction);
		return true;
	case IRO_Or:
		Logic(psInstruction);
		return true;
	case IRO_Xor:
		Logic(psInstruction);
		return true;
	case IRO_Not:
		Logic(psInstruction);
		return true;
	case IRO_Neg:
		Line("sec");
		for (i = 0; i < Words(psInstruction->sType); i++)
		{
			Line("lda 0");
			Operand("sbc", psInstruction->asArguments[0], i);
			if (Words(psInstruction->sType) == 1)
			{
				Mask(psInstruction->sType);
			}
			Store(psInstruction->sResult, i);
		}
		return true;
	case IRO_Copy:
		if (InX(psInstruction->sResult))
		{
			Operand("ldx", psInstruction->asArguments[0], 0);
			return true;
		}
		CopyWords(OffsetOf(psInstruction->sResult), psInstruction->asArguments[0], Words(psInstruction->sType));
		return true;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		Shift(psInstruction);
		return true;
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
		return RuntimeCall(psInstruction);
	case IRO_Eq:
	case IRO_Ne:
	case IRO_SLt:
	case IRO_SLe:
	case IRO_SGt:
	case IRO_SGe:
	case IRO_ULt:
	case IRO_ULe:
	case IRO_UGt:
	case IRO_UGe:
		if (IsFusedCompare(pcBlock, uiIndex))
		{
			//Worked out by the block's branch (Jump).
			return true;
		}
		Compare(psInstruction);
		return true;
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
	case IRO_SpaceCast:
		Extend(psInstruction);
		return true;
	case IRO_Slot:
		Address(psInstruction, *maiSlots.Get(psInstruction->sResult.Index()));
		return true;
	case IRO_ArgAddress:
		Address(psInstruction, *maiParameters.Get((size)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits));
		return true;
	case IRO_Load:
		Load(psInstruction);
		return true;
	case IRO_Store:
		StoreMemory(psInstruction);
		return true;
	case IRO_Arg:
		maiPendingArguments.Add(&uiIndex);
		return true;
	case IRO_Call:
		Call(pcBlock, psInstruction);
		return true;
	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
		FloatArithmetic(psInstruction);
		return true;
	case IRO_FNeg:
		CopyWords(OffsetOf(psInstruction->sResult), psInstruction->asArguments[0], Words(psInstruction->sType));
		Frame("lda", OffsetOf(psInstruction->sResult) + 2 * (Words(psInstruction->sType) - 1));
		Line("eor $8000");
		Frame("sta", OffsetOf(psInstruction->sResult) + 2 * (Words(psInstruction->sType) - 1));
		return true;
	case IRO_FEq:
	case IRO_FNe:
	case IRO_FLt:
	case IRO_FLe:
	case IRO_FGt:
	case IRO_FGe:
	case IRO_FOrdered:
	case IRO_FUnordered:
		FloatCompare(psInstruction);
		return true;
	case IRO_SIToF:
	case IRO_UIToF:
	case IRO_FToSI:
	case IRO_FToUI:
	case IRO_FExt:
	case IRO_FTrunc:
		FloatConversion(psInstruction);
		return true;
	default:
		return Error("The W65C816 back end doesn't do this instruction yet", GetIROpcode(psInstruction->uiOpcode)->szName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The soft float runtime (Libc65's softfloat.cpp, compiled for the W65C816): _SoftFloat<operation> for f32 and
// _SoftDouble<operation> for f64.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FloatArithmetic(SIRInstruction* psInstruction)
{
	CChars	szName;

	szName.Init(psInstruction->sType.Is(IRTK_F64) ? "_SoftDouble" : "_SoftFloat");
	switch (psInstruction->uiOpcode)
	{
	case IRO_FAdd:	szName.Append("Add");		break;
	case IRO_FSub:	szName.Append("Subtract");	break;
	case IRO_FMul:	szName.Append("Multiply");	break;
	default:		szName.Append("Divide");	break;
	}
	PushValue(psInstruction->asArguments[1], psInstruction->sType);
	PushValue(psInstruction->asArguments[0], psInstruction->sType);
	CallFunction(szName.Text(), 2 * ValueBytes(psInstruction->sType), psInstruction);
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the hidden result pointer when the result is 64 bits, calls szName and removes the iBytes of arguments
// already pushed (and the pointer), leaving the result in its place.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CallFunction(const char* szName, int32 iBytes, SIRInstruction* psInstruction)
{
	CChars	sz;

	UseRuntime(szName);
	if (!psInstruction->sResult.IsNone() && IsHiddenResult(psInstruction->sType))
	{
		PushFrameAddress(OffsetOf(psInstruction->sResult));
		iBytes += 4;
	}
	sz.Init("jsl ");
	sz.Append(szName);
	Line(sz.Text());
	sz.Kill();
	CallerCleanUp(iBytes, psInstruction);
}


//////////////////////////////////////////////////////////////////////////
//
// _SoftFloatCompare gives -1, 0 or 1, or 2 when either is a NaN, in A: each comparison is true for some of those.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FloatCompare(SIRInstruction* psInstruction)
{
	int					aiValues[2];
	int					iValues;
	bool				bNot;
	int32				iTrue;
	int32				iFalse;
	int32				iEnd;
	int					i;
	CChars				sz;
	SIRType				sType;

	sType = psInstruction->sType;
	PushValue(psInstruction->asArguments[1], sType);
	PushValue(psInstruction->asArguments[0], sType);
	UseRuntime(sType.Is(IRTK_F64) ? "_SoftDoubleCompare" : "_SoftFloatCompare");
	Line(sType.Is(IRTK_F64) ? "jsl _SoftDoubleCompare" : "jsl _SoftFloatCompare");
	Frame("sta", IR816_SCRATCH_RESULT);
	Line("tsc");
	Line("clc");
	Line("adc ", 2 * ValueBytes(sType));
	Line("tcs");

	//The compare's results that make it true (or false, for fne and fordered).
	bNot = false;
	iValues = 1;
	switch (psInstruction->uiOpcode)
	{
	case IRO_FEq:		aiValues[0] = 0;	break;
	case IRO_FNe:		aiValues[0] = 0;	bNot = true;	break;
	case IRO_FLt:		aiValues[0] = -1;	break;
	case IRO_FGt:		aiValues[0] = 1;	break;
	case IRO_FLe:		aiValues[0] = -1;	aiValues[1] = 0;	iValues = 2;	break;
	case IRO_FGe:		aiValues[0] = 0;	aiValues[1] = 1;	iValues = 2;	break;
	case IRO_FOrdered:	aiValues[0] = 2;	bNot = true;	break;
	default:			aiValues[0] = 2;	break;
	}

	iTrue = NewLabel();
	iFalse = NewLabel();
	iEnd = NewLabel();
	sz.Init();
	Frame("lda", IR816_SCRATCH_RESULT);
	for (i = 0; i < iValues; i++)
	{
		Line("cmp ", aiValues[i] & 0xFFFF);
		sz.Clear();
		sz.Append("beq L");
		sz.Append(bNot ? iFalse : iTrue);
		Line(sz.Text());
	}
	sz.Clear();
	sz.Append("bra L");
	sz.Append(bNot ? iTrue : iFalse);
	Line(sz.Text());
	Label(iTrue);
	Line("lda 1");
	sz.Clear();
	sz.Append("bra L");
	sz.Append(iEnd);
	Line(sz.Text());
	Label(iFalse);
	Line("lda 0");
	Label(iEnd);
	Store(psInstruction->sResult, 0);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Conversions with floats: an integer narrower than 32 bits converts as an int32_t (sign or zero extended), a 64
// bit one as a long long.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FloatConversion(SIRInstruction* psInstruction)
{
	SIRType			sFrom;
	SIRType			sTo;
	SIRReference	sValue;
	bool			bSigned;
	CChars			szName;
	int32			iBytes;

	sValue = psInstruction->asArguments[0];
	sFrom = mpcFunction->TypeOf(sValue);
	sTo = psInstruction->sType;
	szName.Init();
	switch (psInstruction->uiOpcode)
	{
	case IRO_FExt:
	case IRO_FTrunc:
		szName.Append(sTo.Is(IRTK_F64) ? "_SoftFloatToDouble" : "_SoftDoubleToFloat");
		PushValue(sValue, sFrom);
		iBytes = ValueBytes(sFrom);
		break;
	case IRO_SIToF:
	case IRO_UIToF:
		bSigned = psInstruction->uiOpcode == IRO_SIToF;
		szName.Append(sTo.Is(IRTK_F64) ? "_SoftDoubleFrom" : "_SoftFloatFrom");
		if (sFrom.Is(IRTK_I64))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
			PushValue(sValue, sFrom);
			iBytes = 8;
		}
		else
		{
			szName.Append((bSigned || (sFrom.IntegerBits() < 32)) ? "Int" : "Unsigned");
			PushInt32(sValue, sFrom, bSigned);
			iBytes = 4;
		}
		break;
	default:
		bSigned = psInstruction->uiOpcode == IRO_FToSI;
		szName.Append(sFrom.Is(IRTK_F64) ? "_SoftDoubleTo" : "_SoftFloatTo");
		if (sTo.Is(IRTK_I64))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			szName.Append((bSigned || (sTo.IntegerBits() < 32)) ? "Int" : "Unsigned");
		}
		PushValue(sValue, sFrom);
		iBytes = ValueBytes(sFrom);
		break;
	}
	CallFunction(szName.Text(), iBytes, psInstruction);
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes an integer of up to 32 bits as 32 bits, sign or zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PushInt32(SIRReference sValue, SIRType sType, bool bSigned)
{
	if (Words(sType) == 2)
	{
		PushValue(sValue, sType);
		return;
	}
	Operand("lda", sValue, 0);
	if (bSigned && sType.Is(IRTK_I8))
	{
		Line("eor $80");
		Line("sec");
		Line("sbc $80");
	}
	Frame("sta", IR816_SCRATCH_VALUE);
	if (bSigned && !sType.Is(IRTK_I1))
	{
		Line("asl A");
		Line("lda 0");
		Line("adc $FFFF");
		Line("eor $FFFF");
	}
	else
	{
		Line("lda 0");
	}
	Line("pha");
	Frame("lda", IR816_SCRATCH_VALUE);
	Line("pha");
}


//////////////////////////////////////////////////////////////////////////
//
// add, sub and ptradd: a word at a time with the carry between them
// (WordInA), or an i16 kept in X stepped by 1 or 2.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Arithmetic(SIRInstruction* psInstruction)
{
	int		i;
	int		iWords;
	uint16	uiStep;

	iWords = Words(psInstruction->sType);
	if ((iWords == 1) && psInstruction->sType.Is(IRTK_I16) && InX(psInstruction->sResult) && InX(psInstruction->asArguments[0]) &&
		ConstantWord(psInstruction->asArguments[1], 0, &uiStep) && (uiStep >= 1) && (uiStep <= 2))
	{
		for (i = 0; i < uiStep; i++)
		{
			Line((psInstruction->uiOpcode == IRO_Sub) ? "dex" : "inx");
		}
		return;
	}
	if (StepInPlace(psInstruction))
	{
		return;
	}
	for (i = 0; i < iWords; i++)
	{
		WordInA(psInstruction, i);
		if (iWords == 1)
		{
			Mask(psInstruction->sType);
		}
		Store(psInstruction->sResult, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A two word value stepped by 1 or -1 into its own place (the result shares
// the operand's) with inc and dec on the direct page: inc low, and high when
// low wrapped to 0; or dec high when low is 0, then dec low.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::StepInPlace(SIRInstruction* psInstruction)
{
	SIRReference	sValue;
	uint32			uiStep;
	int32			iOffset;
	int32			iSkip;

	sValue = psInstruction->asArguments[0];
	if ((Words(psInstruction->sType) != 2) || !psInstruction->sResult.IsTemporary() || !sValue.IsTemporary() || Fused(sValue) ||
		!psInstruction->asArguments[1].IsConstant() || (OffsetOf(psInstruction->sResult) != OffsetOf(sValue)) || (OffsetOf(sValue) > 253))
	{
		return false;
	}
	uiStep = (uint32)mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
	if (psInstruction->uiOpcode == IRO_Sub)
	{
		uiStep = 0 - uiStep;
	}
	if ((uiStep != 1) && (uiStep != 0xFFFFFFFF))
	{
		return false;
	}
	iOffset = OffsetOf(sValue);
	iSkip = NewLabel();
	if (uiStep == 1)
	{
		Frame("inc", iOffset);
		Branch("bne", iSkip);
		Frame("inc", iOffset + 2);
		Label(iSkip);
	}
	else
	{
		Frame("lda", iOffset);
		Branch("bne", iSkip);
		Frame("dec", iOffset + 2);
		Label(iSkip);
		Frame("dec", iOffset);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// and, or, xor and not a word at a time (WordInA); an and with a constant
// word 0 is 0 without reading the other.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Logic(SIRInstruction* psInstruction)
{
	int		i;
	uint16	uiWord;

	for (i = 0; i < Words(psInstruction->sType); i++)
	{
		if ((psInstruction->uiOpcode == IRO_And) && (ConstantWord(psInstruction->asArguments[1], i, &uiWord) || ConstantWord(psInstruction->asArguments[0], i, &uiWord)) && (uiWord == 0))
		{
			if (InX(psInstruction->sResult))
			{
				Line("ldx 0");
			}
			else
			{
				Frame("stz", OffsetOf(psInstruction->sResult) + 2 * i);
			}
			continue;
		}
		WordInA(psInstruction, i);
		if (psInstruction->uiOpcode == IRO_Not)
		{
			Mask(psInstruction->sType);
		}
		Store(psInstruction->sResult, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The value is copied to the result and shifted there: by a constant (ShiftConstant), or a bit at a time by a count in
// X with a word in A (ShiftInA), or for 64 bits or a frame past the direct page in memory (ShiftInMemory).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Shift(SIRInstruction* psInstruction)
{
	int32			iResult;
	int				iWords;
	int32			iLoop;
	int32			iEnd;
	SIRReference	sAmount;
	SIRType			sType;
	int64			lliAmount;

	sType = psInstruction->sType;
	iResult = OffsetOf(psInstruction->sResult);
	iWords = Words(sType);
	sAmount = psInstruction->asArguments[1];
	if (InX(psInstruction->sResult))
	{
		//A 16 bit shift by a constant (AllocateX), in A.
		Operand("lda", psInstruction->asArguments[0], 0);
		for (lliAmount = (int64)(mpcFunction->GetConstant(sAmount)->ulliBits & 63); lliAmount > 0; lliAmount--)
		{
			if (psInstruction->uiOpcode == IRO_Shl)
			{
				Line("asl A");
			}
			else if (psInstruction->uiOpcode == IRO_LShr)
			{
				Line("lsr A");
			}
			else
			{
				Line("cmp $8000");
				Line("ror A");
			}
		}
		Line("tax");
		return;
	}
	CopyWords(iResult, psInstruction->asArguments[0], iWords);
	if (sAmount.IsConstant())
	{
		lliAmount = (int64)(mpcFunction->GetConstant(sAmount)->ulliBits & 63);
		if (lliAmount == 0)
		{
			return;
		}
		if (!sType.Is(IRTK_I8) && !sType.Is(IRTK_I1))
		{
			ShiftConstant(psInstruction->uiOpcode, iResult, iWords, (int)lliAmount);
			return;
		}
	}

	if ((psInstruction->uiOpcode == IRO_AShr) && (sType.Is(IRTK_I8) || sType.Is(IRTK_I1)))
	{
		Frame("lda", iResult);
		Line(sType.Is(IRTK_I8) ? "eor $80" : "eor 1");
		Line("sec");
		Line(sType.Is(IRTK_I8) ? "sbc $80" : "sbc 1");
		Frame("sta", iResult);
	}

	iLoop = NewLabel();
	iEnd = NewLabel();
	if ((iWords <= 2) && (iResult + 2 * iWords <= 255))
	{
		ShiftInA(psInstruction->uiOpcode, iResult, iWords, sAmount, iLoop, iEnd);
	}
	else
	{
		ShiftInMemory(psInstruction->uiOpcode, iResult, iWords, sAmount, iLoop, iEnd);
	}
	if (sType.Is(IRTK_I8) || sType.Is(IRTK_I1))
	{
		Frame("lda", iResult);
		Mask(sType);
		Frame("sta", iResult);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A shift by a count in X a bit at a time with a word in A: the value's
// word, or for 32 bits the word the bits leave from, the other rolled in
// memory (after a whole word is moved when the count is 16 or more).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftInA(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd)
{
	int32	iBits;
	int32	iInA;
	int32	iOther;

	Operand("lda", sAmount, 0);
	Line("and 63");
	Line("tax");
	Branch("beq", iEnd);
	if (iWords == 2)
	{
		iBits = NewLabel();
		Line("cpx 16");
		Branch("bcc", iBits);
		ShiftConstant(uiOpcode, iResult, 2, 16);
		Line("txa");
		Line("sec");
		Line("sbc 16");
		Line("tax");
		Branch("beq", iEnd);
		Label(iBits);
	}

	//Left shifts carry from the low word up, right shifts from the high word down.
	iInA = (uiOpcode == IRO_Shl) ? iResult : iResult + 2 * (iWords - 1);
	iOther = (uiOpcode == IRO_Shl) ? iResult + 2 : iResult;
	Frame("lda", iInA);
	Label(iLoop);
	if (uiOpcode == IRO_Shl)
	{
		Line("asl A");
	}
	else if (uiOpcode == IRO_LShr)
	{
		Line("lsr A");
	}
	else
	{
		Line("cmp $8000");
		Line("ror A");
	}
	if (iWords == 2)
	{
		Frame((uiOpcode == IRO_Shl) ? "rol" : "ror", iOther);
	}
	Line("dex");
	Branch("bne", iLoop);
	Frame("sta", iInA);
	Label(iEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// A shift by a count in the scratch word a bit at a time in memory.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftInMemory(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd)
{
	CChars	sz;
	int		i;

	Operand("lda", sAmount, 0);
	Line("and 63");
	Frame("sta", IR816_SCRATCH_COUNT);
	sz.Init("beq L");
	sz.Append(iEnd);
	Line(sz.Text());
	sz.Kill();

	Label(iLoop);
	if (uiOpcode == IRO_Shl)
	{
		Frame("asl", iResult);
		for (i = 1; i < iWords; i++)
		{
			Frame("rol", iResult + 2 * i);
		}
	}
	else
	{
		if (uiOpcode == IRO_LShr)
		{
			Frame("lsr", iResult + 2 * (iWords - 1));
		}
		else
		{
			Frame("lda", iResult + 2 * (iWords - 1));
			Line("cmp $8000");
			Line("ror A");
			Frame("sta", iResult + 2 * (iWords - 1));
		}
		for (i = iWords - 2; i >= 0; i--)
		{
			Frame("ror", iResult + 2 * i);
		}
	}
	Frame("dec", IR816_SCRATCH_COUNT);
	sz.Init("bne L");
	sz.Append(iLoop);
	Line(sz.Text());
	sz.Kill();
	Label(iEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// The value at iResult (iWords words) shifted iAmount bits where it is: whole
// words are moved first (the words shifted in are 0, or the sign for ashr),
// then the bits left are shifted a bit at a time without a count.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftConstant(uint16 uiOpcode, int32 iResult, int iWords, int iAmount)
{
	int		iWordShift;
	int		iBits;
	int		i;
	int		j;

	iWordShift = iAmount / 16;
	iBits = iAmount % 16;
	if (iWordShift >= iWords)
	{
		iWordShift = iWords;
		iBits = 0;
	}

	if (iWordShift > 0)
	{
		if (uiOpcode == IRO_Shl)
		{
			for (i = iWords - 1; i >= iWordShift; i--)
			{
				Frame("lda", iResult + 2 * (i - iWordShift));
				Frame("sta", iResult + 2 * i);
			}
			for (i = iWordShift - 1; i >= 0; i--)
			{
				Frame("stz", iResult + 2 * i);
			}
		}
		else
		{
			if (uiOpcode == IRO_AShr)
			{
				//The sign word: $FFFF when the top bit is set.
				Frame("lda", iResult + 2 * (iWords - 1));
				Line("cmp $8000");
				Line("lda 0");
				Line("bcc C + 1");
				Line("dec A");
				Frame("sta", IR816_SCRATCH_COUNT);
			}
			for (i = 0; i < iWords - iWordShift; i++)
			{
				Frame("lda", iResult + 2 * (i + iWordShift));
				Frame("sta", iResult + 2 * i);
			}
			for (i = iWords - iWordShift; i < iWords; i++)
			{
				if (uiOpcode == IRO_AShr)
				{
					Frame("lda", IR816_SCRATCH_COUNT);
					Frame("sta", iResult + 2 * i);
				}
				else
				{
					Frame("stz", iResult + 2 * i);
				}
			}
		}
	}

	if ((iWords - iWordShift == 1) && (iBits >= 2))
	{
		//The one word left, shifted in A.
		i = (uiOpcode == IRO_Shl) ? iWords - 1 : 0;
		Frame("lda", iResult + 2 * i);
		for (j = 0; j < iBits; j++)
		{
			if (uiOpcode == IRO_Shl)
			{
				Line("asl A");
			}
			else if (uiOpcode == IRO_LShr)
			{
				Line("lsr A");
			}
			else
			{
				Line("cmp $8000");
				Line("ror A");
			}
		}
		Frame("sta", iResult + 2 * i);
		return;
	}
	for (j = 0; j < iBits; j++)
	{
		if (uiOpcode == IRO_Shl)
		{
			Frame("asl", iResult + 2 * iWordShift);
			for (i = iWordShift + 1; i < iWords; i++)
			{
				Frame("rol", iResult + 2 * i);
			}
		}
		else
		{
			if (uiOpcode == IRO_LShr)
			{
				Frame("lsr", iResult + 2 * (iWords - 1 - iWordShift));
			}
			else
			{
				Frame("lda", iResult + 2 * (iWords - 1));
				Line("cmp $8000");
				Line("ror A");
				Frame("sta", iResult + 2 * (iWords - 1));
			}
			for (i = ((uiOpcode == IRO_LShr) ? iWords - 2 - iWordShift : iWords - 2); i >= 0; i--)
			{
				Frame("ror", iResult + 2 * i);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// mul, udiv, sdiv, urem and srem call R816_<operation><bits>, an i8 as 16 bits (sign extended for sdiv and srem).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::RuntimeCall(SIRInstruction* psInstruction)
{
	CChars		szName;
	SIRType		sType;
	int			iBits;
	int32		iBytes;
	bool		bSigned;
	int			i;

	sType = psInstruction->sType;
	if (sType.IsFloat() || sType.Is(IRTK_I1))
	{
		return Error("Multiply and divide are for i8 to i64");
	}
	iBits = sType.Is(IRTK_I8) ? 16 : sType.Bits();
	bSigned = (psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem);

	szName.Init("R816_");
	switch (psInstruction->uiOpcode)
	{
	case IRO_Mul:	szName.Append("Mul");	break;
	case IRO_UDiv:	szName.Append("UDiv");	break;
	case IRO_SDiv:	szName.Append("SDiv");	break;
	case IRO_URem:	szName.Append("URem");	break;
	default:		szName.Append("SRem");	break;
	}
	szName.Append(iBits);
	UseRuntime(szName.Text());

	iBytes = 2 * ValueBytes(sType);
	for (i = 1; i >= 0; i--)
	{
		if (bSigned && sType.Is(IRTK_I8))
		{
			Operand("lda", psInstruction->asArguments[i], 0);
			Line("eor $80");
			Line("sec");
			Line("sbc $80");
			Line("pha");
		}
		else
		{
			PushValue(psInstruction->asArguments[i], sType);
		}
	}
	if (IsHiddenResult(sType))
	{
		PushFrameAddress(OffsetOf(psInstruction->sResult));
		iBytes += 4;
	}
	szName.Insert(0, "jsl ");
	Line(szName.Text());
	szName.Kill();
	CallerCleanUp(iBytes, psInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// After a call: the arguments are removed (pulled into Y, or S moved with A kept in Y; X is kept), then the result (A,
// and X for 32 bits) goes to its place.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CallerCleanUp(int32 iBytes, SIRInstruction* psInstruction)
{
	bool	bResult;
	int		iWords;
	int		i;

	bResult = !psInstruction->sResult.IsNone() && !IsHiddenResult(psInstruction->sType);
	iWords = bResult ? Words(psInstruction->sType) : 0;
	if ((iBytes == 2) || (iBytes == 4))
	{
		//Pulled into Y, which keeps A and X.
		for (i = 0; i < iBytes; i += 2)
		{
			Line("ply");
		}
	}
	else if (iBytes != 0)
	{
		if (bResult)
		{
			Line("tay");
		}
		Line("tsc");
		Line("clc");
		Line("adc ", iBytes);
		Line("tcs");
		if (bResult)
		{
			Line("tya");
		}
	}
	if (bResult)
	{
		if (iWords == 1)
		{
			Mask(psInstruction->sType);
			Store(psInstruction->sResult, 0);
		}
		else if (OffsetOf(psInstruction->sResult) + 2 <= 255)
		{
			Store(psInstruction->sResult, 0);
			Frame("stx", OffsetOf(psInstruction->sResult) + 2);
		}
		else
		{
			//Past the direct page Frame goes through X: the high word waits in the scratch word.
			Frame("stx", IR816_SCRATCH_RESULT + 2);
			Store(psInstruction->sResult, 0);
			Frame("lda", IR816_SCRATCH_RESULT + 2);
			Store(psInstruction->sResult, 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves C and Z as an unsigned compare of the left with the right leaves them, high words first.  A signed compare
// flips the sign bits so it can compare unsigned (a constant's as it is written).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CompareFlags(SIRReference sLeft, SIRReference sRight, SIRType sType, bool bSigned)
{
	int		iWords;
	int		i;
	int32	iDone;
	const char*	szFlip;
	uint64	uiFlip;
	CChars	sz;

	iWords = Words(sType);
	iDone = NewLabel();
	szFlip = sType.Is(IRTK_I1) ? "eor 1" : (sType.Is(IRTK_I8) ? "eor $80" : "eor $8000");
	uiFlip = sType.Is(IRTK_I1) ? 1 : (sType.Is(IRTK_I8) ? 0x80 : 0x8000);
	for (i = iWords - 1; i >= 0; i--)
	{
		if (bSigned && (i == iWords - 1) && sRight.IsConstant())
		{
			//The constant's sign bit flipped as it is written.
			Operand("lda", sLeft, i);
			Line(szFlip);
			Line("cmp ", (int64)((((i < 4) ? (mpcFunction->GetConstant(sRight)->ulliBits >> (16 * i)) : 0) ^ uiFlip) & 0xFFFF));
		}
		else if (bSigned && (i == iWords - 1))
		{
			Operand("lda", sRight, i);
			Line(szFlip);
			Frame("sta", IR816_SCRATCH_VALUE);
			Operand("lda", sLeft, i);
			Line(szFlip);
			Frame("cmp", IR816_SCRATCH_VALUE);
		}
		else if ((iWords == 1) && InX(sLeft))
		{
			Operand("cpx", sRight, i);
		}
		else
		{
			Operand("lda", sLeft, i);
			Operand("cmp", sRight, i);
		}
		if (i > 0)
		{
			sz.Init("bne L");
			sz.Append(iDone);
			Line(sz.Text());
			sz.Kill();
		}
	}
	Label(iDone);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction at uiIndex is an integer comparison the block's
// branch works out itself: the block's last instruction, whose result only
// the branch uses.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsFusedCompare(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;

	if ((pcBlock->msJump.uiKind != IRJ_Branch) || (uiIndex + 1 != pcBlock->masInstructions.NumElements()))
	{
		return false;
	}
	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Eq:
	case IRO_Ne:
	case IRO_SLt:
	case IRO_SLe:
	case IRO_SGt:
	case IRO_SGe:
	case IRO_ULt:
	case IRO_ULe:
	case IRO_UGt:
	case IRO_UGe:
		break;
	default:
		return false;
	}
	return psInstruction->sResult.IsTemporary() && pcBlock->msJump.sArgument.Equals(psInstruction->sResult) &&
		   (psInstruction->sResult.Index() < maiUses.NumElements()) && (*maiUses.Get(psInstruction->sResult.Index()) == 1);
}


//////////////////////////////////////////////////////////////////////////
//
// A branch to iLabel when a comparison is bWhen: the flags, then the
// branches on them (CompareFlags leaves C and Z as an unsigned compare
// leaves them).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FusedBranch(SIRInstruction* psInstruction, bool bWhen, int32 iLabel)
{
	int32	iSkip;
	uint16	uiOpcode;
	uint16	uiZero;

	uiOpcode = bWhen ? psInstruction->uiOpcode : IRInvertedComparison(psInstruction->uiOpcode);
	if (IsIRSignedComparison(uiOpcode) && psInstruction->sType.Is(IRTK_I16) && ConstantWord(psInstruction->asArguments[1], 0, &uiZero) && (uiZero == 0))
	{
		//Against 0 the sign and zero flags of the value itself answer (lda or txa sets them).
		Operand("lda", psInstruction->asArguments[0], 0);
		switch (uiOpcode)
		{
		case IRO_SLt:
			Branch("bmi", iLabel);
			break;
		case IRO_SGe:
			Branch("bpl", iLabel);
			break;
		case IRO_SGt:
			iSkip = NewLabel();
			Branch("beq", iSkip);
			Branch("bpl", iLabel);
			Label(iSkip);
			break;
		default:
			Branch("bmi", iLabel);
			Branch("beq", iLabel);
			break;
		}
		return;
	}
	CompareFlags(psInstruction->asArguments[0], psInstruction->asArguments[1], psInstruction->sType, IsIRSignedComparison(psInstruction->uiOpcode));
	switch (uiOpcode)
	{
	case IRO_Eq:
		Branch("beq", iLabel);
		break;
	case IRO_Ne:
		Branch("bne", iLabel);
		break;
	case IRO_SLt:
	case IRO_ULt:
		Branch("bcc", iLabel);
		break;
	case IRO_SGe:
	case IRO_UGe:
		Branch("bcs", iLabel);
		break;
	case IRO_SGt:
	case IRO_UGt:
		//gt: C set and Z clear.
		iSkip = NewLabel();
		Branch("beq", iSkip);
		Branch("bcs", iLabel);
		Label(iSkip);
		break;
	default:
		//le: C clear or Z set.
		Branch("bcc", iLabel);
		Branch("beq", iLabel);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The comparison's i1 (0 or 1).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Compare(SIRInstruction* psInstruction)
{
	int32	iTrue;
	int32	iFalse;
	int32	iEnd;
	uint16	uiOpcode;
	CChars	sz;

	uiOpcode = psInstruction->uiOpcode;
	CompareFlags(psInstruction->asArguments[0], psInstruction->asArguments[1], psInstruction->sType, IsIRSignedComparison(uiOpcode));
	iTrue = NewLabel();
	iFalse = NewLabel();
	iEnd = NewLabel();
	sz.Init();
	switch (uiOpcode)
	{
	case IRO_Eq:
		sz.Append("bne L");
		break;
	case IRO_Ne:
		sz.Append("beq L");
		break;
	case IRO_SLt:
	case IRO_ULt:
		sz.Append("bcs L");
		break;
	case IRO_SGe:
	case IRO_UGe:
		sz.Append("bcc L");
		break;
	case IRO_SGt:
	case IRO_UGt:
		sz.Append("bcc L");
		sz.Append(iFalse);
		Line(sz.Text());
		sz.Clear();
		sz.Append("beq L");
		break;
	default:
		//le: true when C is clear or Z set.
		sz.Append("bcc L");
		sz.Append(iTrue);
		Line(sz.Text());
		sz.Clear();
		sz.Append("bne L");
		break;
	}
	sz.Append(iFalse);
	Line(sz.Text());
	Label(iTrue);
	Line("lda 1");
	sz.Clear();
	sz.Append("bra L");
	sz.Append(iEnd);
	Line(sz.Text());
	Label(iFalse);
	Line("lda 0");
	Label(iEnd);
	Store(psInstruction->sResult, 0);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An extension or trunc to one word kept in X: the value's low word in A,
// masked to a narrow result or sign extended from i8 or i1, then tax (a
// zero extension or trunc to 16 bits is ldx of the word).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ExtendInX(SIRInstruction* psInstruction)
{
	SIRReference	sValue;
	SIRType			sSource;
	SIRType			sType;

	sValue = psInstruction->asArguments[0];
	sSource = mpcFunction->TypeOf(sValue);
	sType = psInstruction->sType;
	if ((psInstruction->uiOpcode == IRO_SExt) && sSource.Is(IRTK_I1))
	{
		Line("lda 0");
		Line("sec");
		Operand("sbc", sValue, 0);
		Mask(sType);
	}
	else if ((psInstruction->uiOpcode == IRO_SExt) && sSource.Is(IRTK_I8))
	{
		Operand("lda", sValue, 0);
		Line("eor $80");
		Line("sec");
		Line("sbc $80");
		Mask(sType);
	}
	else if (sType.Is(IRTK_I8) || sType.Is(IRTK_I1))
	{
		Operand("lda", sValue, 0);
		Mask(sType);
	}
	else
	{
		Operand("ldx", sValue, 0);
		return;
	}
	Line("tax");
}


//////////////////////////////////////////////////////////////////////////
//
// zext, sext, trunc and the pointer conversions.  Narrow values are already zero extended in their word.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Extend(SIRInstruction* psInstruction)
{
	SIRType			sSource;
	SIRType			sType;
	SIRReference	sValue;
	int32			iResult;
	int				iSourceWords;
	int				iWords;
	int				iCopied;

	sValue = psInstruction->asArguments[0];
	sSource = mpcFunction->TypeOf(sValue);
	sType = psInstruction->sType;
	if (InX(psInstruction->sResult))
	{
		ExtendInX(psInstruction);
		return;
	}
	iResult = OffsetOf(psInstruction->sResult);
	iSourceWords = Words(sSource);
	iWords = Words(sType);
	iCopied = (iSourceWords < iWords) ? iSourceWords : iWords;

	if (psInstruction->uiOpcode == IRO_SExt)
	{
		if (sSource.Is(IRTK_I1))
		{
			Line("lda 0");
			Line("sec");
			Operand("sbc", sValue, 0);
			Mask(sType);
			Frame("sta", iResult);
			iCopied = 1;
		}
		else if (sSource.Is(IRTK_I8))
		{
			Operand("lda", sValue, 0);
			Line("eor $80");
			Line("sec");
			Line("sbc $80");
			Frame("sta", iResult);
			iCopied = 1;
		}
		else
		{
			CopyWords(iResult, sValue, iCopied);
		}
		FillSign(iResult, iCopied, iWords - iCopied);
		return;
	}

	CopyWords(iResult, sValue, iCopied);
	ZeroWords(iResult, iCopied, iWords - iCopied);
	if ((psInstruction->uiOpcode == IRO_Trunc) && (sType.Is(IRTK_I8) || sType.Is(IRTK_I1)))
	{
		Frame("lda", iResult);
		Mask(sType);
		Frame("sta", iResult);
	}
	else if (psInstruction->uiOpcode == IRO_IToPtr)
	{
		//A pointer's top byte is always 0.
		Frame("lda", iResult + 2);
		Line("and $FF");
		Frame("sta", iResult + 2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A slot's or argument's address: D + offset, in bank 0.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Address(SIRInstruction* psInstruction, int32 iOffset)
{
	Line("tdc");
	Line("clc");
	Line("adc ", iOffset);
	Store(psInstruction->sResult, 0);
	Frame("stz", OffsetOf(psInstruction->sResult) + 2);
}


//////////////////////////////////////////////////////////////////////////
//
// A word at a time (WordInA).  An i8 is read as a word and masked (the byte after it is only memory), or with m 8 bits
// when volatile (it may be a device's register).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Load(SIRInstruction* psInstruction)
{
	int		i;
	bool	bByte;

	bByte = psInstruction->sType.Is(IRTK_I8) && (psInstruction->uiFlags & IRF_Volatile);
	if (bByte)
	{
		Line("sep m");
	}
	for (i = 0; i < Words(psInstruction->sType); i++)
	{
		WordInA(psInstruction, i);
		if (bByte)
		{
			Line("rep m");
		}
		Mask(psInstruction->sType);
		Store(psInstruction->sResult, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sResult's place overlaps sInput's (a temporary's) so that writing
// its first word changes a later word of sInput's: any overlap when
// bAnyOverlap (a pointer read for each word), else one not at the same place.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::FusedOverlaps(SIRReference sResult, SIRReference sInput, bool bAnyOverlap)
{
	int32	iResult;
	int32	iInput;

	if (!sInput.IsTemporary() || sResult.IsNone())
	{
		return false;
	}
	iResult = OffsetOf(sResult);
	iInput = OffsetOf(sInput);
	if ((iResult >= iInput + ValueBytes(mpcFunction->GetTemporaryType(sInput))) || (iInput >= iResult + ValueBytes(mpcFunction->GetTemporaryType(sResult))))
	{
		return false;
	}
	return bAnyOverlap || (iResult != iInput);
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction waiting to be read into A (CanFuse) that works out
// sValue, else NULL.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR816Backend::Fused(SIRReference sValue)
{
	int		i;

	for (i = 0; i < miFused; i++)
	{
		if (mapsFused[i]->sResult.Equals(sValue))
		{
			return mapsFused[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what psInstruction (to be fused) reads is still there for each
// word after sResult's earlier words are written: a load's pointer anywhere
// else (it is read for each word) and in the direct page, other values not
// partly overlapped.  The fused values it reads aren't written.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::FusedInputsKept(SIRInstruction* psInstruction, SIRReference sResult)
{
	SIRReference	sPointer;
	int				i;

	if (psInstruction->uiOpcode == IRO_Load)
	{
		sPointer = FoldedBase(psInstruction->asArguments[0]);
		if (sPointer.IsTemporary())
		{
			return (OffsetOf(sPointer) <= 253) && !FusedOverlaps(sResult, sPointer, true);
		}
		return sPointer.IsSymbol();
	}
	for (i = 0; i < 2; i++)
	{
		if (!Fused(psInstruction->asArguments[i]) && FusedOverlaps(sResult, psInstruction->asArguments[i], false))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psInstruction is an and with a constant.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816AndsConstant(SIRInstruction* psInstruction)
{
	return (psInstruction->uiOpcode == IRO_And) && (psInstruction->asArguments[0].IsConstant() || psInstruction->asArguments[1].IsConstant());
}


//////////////////////////////////////////////////////////////////////////
//
// Whether instruction uiIndex of pcBlock, a load, and, or, xor or not of
// two or more words, or an add or sub, needn't be written: its result's one
// use is the next instruction, an and, or, xor, not, add or sub, or the
// value of a store, which then reads each word into A as it gets to it
// (WordInA): no store and load of each.  The instructions before it may
// be fused too (a chain read into A a word at a time), with one add or sub
// at most in the chain and the instruction that reads it (the carry), and a
// load in it only when the store at the end is to the same place (else the
// store could change a word still to be loaded).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::CanFuse(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIRInstruction*	psNext;
	SIRInstruction*	psChained;
	SIRReference	sResult;
	SIRReference	sNextResult;
	int				iCarries;		//The next instruction's.
	int				iChainCarries;	//The chain's, this one's too.
	int				i;
	bool			bStore;

	if (miFused >= IR816_MAX_FUSED)
	{
		return false;
	}
	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	if (uiIndex + 1 >= pcBlock->masInstructions.NumElements())
	{
		return CanFuseReturn(pcBlock, psInstruction);
	}
	psNext = pcBlock->masInstructions.Get(uiIndex + 1);
	sResult = psInstruction->sResult;
	if (!sResult.IsTemporary() || (Words(psInstruction->sType) < 2) || psInstruction->sType.IsFloat() ||
		(*maiUseBlock.Get(sResult.Index()) != miBlock))
	{
		return false;
	}

	iCarries = 0;
	iChainCarries = 0;
	switch (psInstruction->uiOpcode)
	{
	case IRO_Load:
		if (psInstruction->uiFlags & IRF_Volatile)
		{
			return false;
		}
		break;
	case IRO_Add:
	case IRO_PtrAdd:
		if (!FoldedBase(sResult).Equals(sResult))
		{
			return false;
		}
		break;
	case IRO_Sub:
		break;
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
		break;
	default:
		return false;
	}

	bStore = false;
	sNextResult = IRNone();
	switch (psNext->uiOpcode)
	{
	case IRO_Store:
		if (!psNext->asArguments[0].Equals(sResult) || psNext->asArguments[1].Equals(sResult))
		{
			return false;
		}
		bStore = true;
		break;
	case IRO_Add:
	case IRO_PtrAdd:
	case IRO_Sub:
		if ((psNext->uiOpcode != IRO_Sub) && !FoldedBase(psNext->sResult).Equals(psNext->sResult))
		{
			return false;
		}
		iCarries++;
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
		if ((Words(psNext->sType) != Words(psInstruction->sType)) || psNext->asArguments[0].Equals(psNext->asArguments[1]))
		{
			return false;
		}
		if (!psNext->asArguments[0].Equals(sResult) && !(psNext->asArguments[1].Equals(sResult) && (psNext->uiOpcode != IRO_Sub) && (psNext->uiOpcode != IRO_Not)))
		{
			return false;
		}
		sNextResult = psNext->sResult;
		break;
	default:
		return false;
	}
	if (sNextResult.IsTemporary() && (InX(sNextResult) || (OffsetOf(sNextResult) > 255)))
	{
		return false;
	}

	//The whole chain: the carry, loads before a store, and what each reads.
	for (i = 0; i <= miFused; i++)
	{
		psChained = (i < miFused) ? mapsFused[i] : psInstruction;
		if ((psChained->uiOpcode == IRO_Add) || (psChained->uiOpcode == IRO_PtrAdd) || (psChained->uiOpcode == IRO_Sub))
		{
			iChainCarries++;
		}
		if ((psChained->uiOpcode == IRO_Load) && bStore && !psChained->asArguments[0].Equals(psNext->asArguments[1]))
		{
			return false;
		}
		if (!FusedInputsKept(psChained, sNextResult))
		{
			return false;
		}
	}
	if (iChainCarries && (IR816AndsConstant(psInstruction) || IR816AndsConstant(psNext)))
	{
		//An and of a constant word 0 doesn't read the word: an add's carry wouldn't get to the next.
		return false;
	}
	return iChainCarries + iCarries <= 1;
}


//////////////////////////////////////////////////////////////////////////
//
// A word of all ones of sType: an i8's or i1's only its bits.
//
//////////////////////////////////////////////////////////////////////////
static uint16 IR816AllOnes(SIRType sType)
{
	return sType.Is(IRTK_I8) ? 0xFF : (sType.Is(IRTK_I1) ? 1 : 0xFFFF);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psInstruction, pcBlock's last, is fused into the block's ret of
// its two words (Return reads the high word into A then X first, so the
// chain can't have an add or sub's carry).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::CanFuseReturn(CIRBlock* pcBlock, SIRInstruction* psInstruction)
{
	SIRInstruction*	psChained;
	int				i;

	if ((pcBlock->msJump.uiKind != IRJ_Return) || !pcBlock->msJump.sArgument.Equals(psInstruction->sResult) || (miHiddenResult >= 0) ||
		!psInstruction->sResult.IsTemporary() || (Words(psInstruction->sType) != 2) || psInstruction->sType.IsFloat() ||
		(*maiUseBlock.Get(psInstruction->sResult.Index()) != miBlock))
	{
		return false;
	}
	for (i = 0; i <= miFused; i++)
	{
		psChained = (i < miFused) ? mapsFused[i] : psInstruction;
		switch (psChained->uiOpcode)
		{
		case IRO_Load:
			if (psChained->uiFlags & IRF_Volatile)
			{
				return false;
			}
		case IRO_And:
		case IRO_Or:
		case IRO_Xor:
		case IRO_Not:
			break;
		default:
			return false;
		}
		if (!FusedInputsKept(psChained, IRNone()))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Word iWord of a load's, add's, sub's, ptradd's, and's, or's, xor's or
// not's value into A: through the pointer ([[D + p]:24 + Y]) or a symbol's
// long address, or the first operand's word and the second's (an add's or
// sub's carry first with word 0).  A commutative one reads the operand kept
// in X, or fused (CanFuse), first.  Fused operands are worked out as they
// are read.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::WordInA(SIRInstruction* psInstruction, int iWord)
{
	int32			iPlace;
	int32			iOffset;
	char*			szSymbol;
	uint16			uiWord;
	const char*		szMnemonic;
	SIRReference	sFirst;
	SIRReference	sSecond;
	bool			bAnd;
	CChars			sz;

	sFirst = psInstruction->asArguments[0];
	sSecond = psInstruction->asArguments[1];
	if ((psInstruction->uiOpcode != IRO_Sub) && (psInstruction->uiOpcode != IRO_Not) && (psInstruction->uiOpcode != IRO_Load) &&
		(Fused(sSecond) || ((Words(psInstruction->sType) == 1) && InX(sSecond) && !InX(sFirst))))
	{
		sFirst = psInstruction->asArguments[1];
		sSecond = psInstruction->asArguments[0];
	}
	switch (psInstruction->uiOpcode)
	{
	case IRO_Load:
		PointerPlace(sFirst, &iPlace, &szSymbol, &iOffset);
		sz.Init();
		if (szSymbol)
		{
			sz.Append("lda [");
			sz.Append(szSymbol);
			sz.Append(" + ");
			sz.Append(iOffset + 2 * iWord);
			sz.Append("]:24");
		}
		else
		{
			Line("ldy ", iOffset + 2 * iWord);
			sz.Append("lda [[D + ");
			sz.Append(iPlace);
			sz.Append("]:24 + Y]");
		}
		Line(sz.Text());
		sz.Kill();
		return;
	case IRO_Add:
	case IRO_PtrAdd:
	case IRO_Sub:
		if (iWord == 0)
		{
			Line((psInstruction->uiOpcode == IRO_Sub) ? "sec" : "clc");
		}
		Operand("lda", sFirst, iWord);
		Operand((psInstruction->uiOpcode == IRO_Sub) ? "sbc" : "adc", sSecond, iWord);
		return;
	case IRO_Not:
		Operand("lda", sFirst, iWord);
		Line("eor $FFFF");
		return;
	default:
		//A constant word that leaves the other's (and all ones, or and eor 0) is a copy; and 0 is 0.
		bAnd = psInstruction->uiOpcode == IRO_And;
		szMnemonic = bAnd ? "and" : ((psInstruction->uiOpcode == IRO_Or) ? "ora" : "eor");
		if (ConstantWord(sSecond, iWord, &uiWord))
		{
			if (bAnd && (uiWord == 0))
			{
				Line("lda 0");
				return;
			}
			if ((bAnd && (uiWord == IR816AllOnes(psInstruction->sType))) || (!bAnd && (uiWord == 0)))
			{
				Operand("lda", sFirst, iWord);
				return;
			}
		}
		Operand("lda", sFirst, iWord);
		Operand(szMnemonic, sSecond, iWord);
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::StoreMemory(SIRInstruction* psInstruction)
{
	int32	iPlace;
	int32	iOffset;
	char*	szSymbol;
	int		i;
	CChars	sz;

	PointerPlace(psInstruction->asArguments[1], &iPlace, &szSymbol, &iOffset);
	sz.Init();
	if (psInstruction->sType.Is(IRTK_I8))
	{
		Line("sep m");
	}
	for (i = 0; i < Words(psInstruction->sType); i++)
	{
		sz.Clear();
		if (szSymbol)
		{
			sz.Append("sta [");
			sz.Append(szSymbol);
			sz.Append(" + ");
			sz.Append(iOffset + 2 * i);
			sz.Append("]:24");
		}
		else
		{
			sz.Append("sta [[D + ");
			sz.Append(iPlace);
			sz.Append("]:24 + Y]");
		}

		//The value first: a fused load sets Y.
		Operand("lda", psInstruction->asArguments[0], i);
		if (!szSymbol)
		{
			Line("ldy ", iOffset + 2 * i);
		}
		Line(sz.Text());
	}
	if (psInstruction->sType.Is(IRTK_I8))
	{
		Line("rep m");
	}
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the arguments last first (and the hidden result pointer), calls and removes them.  A call through a pointer
// pushes the return address and the target - 1, and rtl's to it (rtl adds 1 to the low 16 bits only).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Call(CIRBlock* pcBlock, SIRInstruction* psInstruction)
{
	int32			iBytes;
	size			ui;
	SIRInstruction*	psArgument;
	SIRReference	sCallee;
	int32			iReturn;
	int32			iCallee;
	CChars			sz;

	iBytes = 0;
	for (ui = maiPendingArguments.NumElements(); ui > 0; ui--)
	{
		psArgument = pcBlock->masInstructions.Get(*maiPendingArguments.Get(ui - 1));
		PushValue(psArgument->asArguments[0], psArgument->sType);
		iBytes += ValueBytes(psArgument->sType);
	}
	maiPendingArguments.FakeSetUsedElements(0);
	if (!psInstruction->sResult.IsNone() && IsHiddenResult(psInstruction->sType))
	{
		PushFrameAddress(OffsetOf(psInstruction->sResult));
		iBytes += 4;
	}

	sz.Init();
	sCallee = psInstruction->asArguments[0];
	if (sCallee.IsSymbol())
	{
		sz.Append("jsl ");
		sz.Append(mpcModule->GetSymbolName(sCallee.Index()));
		Line(sz.Text());
	}
	else
	{
		iReturn = NewLabel();
		iCallee = OffsetOf(sCallee);
		Line("phk");
		sz.Append("per L");
		sz.Append(iReturn);
		sz.Append(" - 1");
		Line(sz.Text());
		Line("sep m");
		Frame("lda", iCallee + 2);
		Line("pha");
		Line("rep m");
		Frame("lda", iCallee);
		Line("dec A");
		Line("pha");
		Line("rtl");
		Label(iReturn);
	}
	sz.Kill();
	CallerCleanUp(iBytes, psInstruction);
}


//////////////////////////////////////////////////////////////////////////
//
// Generates the runtime routines named in paszRuntime (and those they call) from their IR.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::GenerateRuntime(CArrayChars* paszRuntime, int* piLabels, CChars* pszDest, CCPPErrors* pcErrors)
{
	CArrayChars		aszNames;
	CChars			szIR;
	CChars			szText;
	CChars			szType;
	CChars			szBits;
	CChars			szTop;
	CIRModule		cModule;
	CIRParser		cParser;
	CIR816Backend	cBackend;
	size			i;
	size			j;
	char*			szName;
	int				iBits;
	bool			bResult;
	bool			bFound;

	//Only the R816_ routines are made here (the soft float functions are Libc65's).
	aszNames.Init();
	for (i = 0; i < paszRuntime->NumElements(); i++)
	{
		if (strncmp(paszRuntime->GetText(i), "R816_", 5) == 0)
		{
			aszNames.Add(paszRuntime->GetText(i));
		}
	}
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		//Signed divide and remainder call the unsigned ones.
		szName = aszNames.GetText(i);
		if ((strncmp(szName, "R816_SDiv", 9) == 0) || (strncmp(szName, "R816_SRem", 9) == 0))
		{
			szText.Init("R816_U");
			szText.Append(&szName[6]);
			bFound = false;
			for (j = 0; j < aszNames.NumElements(); j++)
			{
				bFound |= (strcmp(aszNames.GetText(j), szText.Text()) == 0);
			}
			if (!bFound)
			{
				aszNames.Add(szText.Text());
			}
			szText.Kill();
		}
	}

	szIR.Init();
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		szName = aszNames.GetText(i);

		//The routines written in assembly.
		for (j = 0; gasIR816Routines[j].szName; j++)
		{
			if (strcmp(gasIR816Routines[j].szName, szName) == 0)
			{
				pszDest->Append("\t\t.section text.")->Append(szName)->Append(", comdat\n");
				pszDest->Append("\t\t.global ")->Append(szName)->AppendNewLine();
				pszDest->Append(szName)->Append(":\n");
				pszDest->Append(gasIR816Routines[j].szText);
				pszDest->AppendNewLine();
				break;
			}
		}
		if (gasIR816Routines[j].szName)
		{
			continue;
		}

		iBits = atoi(&szName[strlen(szName) - 2]);
		szType.Init("i");
		szType.Append(iBits);
		szBits.Init();
		szBits.Append(iBits);
		szTop.Init();
		szTop.Append(iBits - 1);
		if (strncmp(szName, "R816_Mul", 8) == 0)
		{
			szText.Init(gszIR816Mul);
		}
		else if (szName[5] == 'U')
		{
			szText.Init(gszIR816Divide);
			szText.Replace("$RESULT", (szName[6] == 'D') ? "%q" : "%r");
		}
		else
		{
			szText.Init(gszIR816Signed);
			szText.Replace("$UNAME", &szName[6]);
			szText.Replace("$SIGN", (szName[6] == 'D') ? "ne i1 %an, %bn" : "copy i1 %an");
		}
		szText.Replace("$NAME", &szName[5]);
		szText.Replace("$T", szType.Text());
		szText.Replace("$N", szBits.Text());
		szText.Replace("$M", szTop.Text());
		szIR.Append(szText);
		szText.Kill();
		szTop.Kill();
		szBits.Kill();
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

