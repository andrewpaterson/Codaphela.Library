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
#include <stdarg.h>
#include "IRParser.h"
#include "IR816BackendInternal.h"


//The W65C816 back end's runtime routines (R816_...): their text, in assembly or as IR for the back end to
//generate (GenerateRuntime), the tables they use and how calls find them.


//The runtime routines, as IR for each width: $T is the type, $N its bits, $M its bits - 1 and $NAME the routine
//(R816_$NAME).  Multiply adds and shifts; divide is restoring division a bit at a time (a remainder that shifts
//past the top is bigger than any divisor); signed divide and remainder work on the magnitudes.
//The unsigned 16 and 32 bit runtime routines in assembly (the IR ones are much slower).  Multiply adds byte products
//from a table (IR816_MULTIPLY_BYTES) and skips those of a high half that is 0.  The multiplies and the 16 and 32 bit
//unsigned divides run on the caller's D with a at D + 15 and b at D + 19 (IR816ScratchRoutine); the 64 bit divide's
//arguments are pushed (a at S + 4 and b after it, last first, after the jsl's return address) and it points D at the
//stack.  A 16 bit result is left in A, a 32 bit one in A (low) and X (high).
struct SIR816Routine
{
	const char*	szName;
	const char*	szText;
	bool		bScratch;	//Takes its operands in the caller's scratch bytes (IR816ScratchRoutine), not pushed.
	const char*	szCalls;	//The runtime routines it calls or jumps to (separated by spaces), which must be in the unit too, or NULL.
	void		(*pfAppend)(CChars* pszDest);	//Writes its text, when szText is NULL.
	bool		bIndexed;	//Works on the caller's frame at D + X (the 64 bit shifts): it may read any of it.
};


//The 32 bit divide runs on the caller's D (IR816ScratchRoutine): the dividend is at D + 15 (and becomes the quotient)
//and the divisor at D + 19, and D + 5 is the remainder.  With both under 65536 it is the 16 bit one (SHORT, with its
//reciprocal table), whose operands are already in place (their low words), jumped to; it returns the 32 bit result
//(X is 0).  A divisor under 65536 keeps the remainder in A: 16 unrolled steps (IR816_DIVIDE_STEP) on the dividend's
//low word, after 16 on its high word when that is not below the divisor (else the quotient's high word is 0 and the
//high word is the remainder to start).  A dividend under 65536 with a wider divisor is a quotient of 0.  Otherwise
//(both wide) the quotient is under 65536: its high word is 0, the dividend's high word is the remainder to start, and
//16 steps on the remainder at D + 5 and D + 7 take the dividend's low word, the quotient bit (C) rotated into it as
//its next bit comes out.
//One step of a 16 bit restoring divide, unrolled: the next dividend bit (in C) is rotated into the remainder in A,
//which is compared with the divisor at D + 19 (a remainder that carries out of A is always over it) and taken from
//when it is over; C is then the quotient bit, which rol shifts into the dividend word at D + WORD as the word's next
//bit comes out.  bcs skips the cmp and bcc (4 bytes), bcc skips the sbc and sec (3 bytes).
#define IR816_DIVIDE_STEP(WORD) \
	"\t\trol A\n\t\tbcs C + 4\n\t\tcmp [D + 19]\n\t\tbcc C + 3\n\t\tsbc [D + 19]\n\t\tsec\n\t\trol [D + " WORD "]\n"


#define IR816_DIVIDE_STEPS_4(WORD) IR816_DIVIDE_STEP(WORD) IR816_DIVIDE_STEP(WORD) IR816_DIVIDE_STEP(WORD) IR816_DIVIDE_STEP(WORD)


#define IR816_DIVIDE_STEPS_16(WORD) IR816_DIVIDE_STEPS_4(WORD) IR816_DIVIDE_STEPS_4(WORD) IR816_DIVIDE_STEPS_4(WORD) IR816_DIVIDE_STEPS_4(WORD)


#define IR816_DIVIDE_32(NAME, SHORT, RESULT_LOW, RESULT_HIGH) \
	"\t\t.width (A, X), 16\n" \
	"\t\t.relax\n" \
	"\t\tlda [D + 17]\n\t\tora [D + 21]\n\t\tbne " NAME "_Wide\n" \
	"\t\tjml " SHORT "\n" \
	NAME "_Wide:\n" \
	"\t\tstz [D + 7]\n" \
	"\t\tlda [D + 21]\n\t\tbne " NAME "_WideDivisor\n" \
	"\t\tlda [D + 17]\n\t\tcmp [D + 19]\n\t\tbcc " NAME "_LowWord\n" \
	"\t\tlda 0\n\t\tasl [D + 17]\n" \
	IR816_DIVIDE_STEPS_16("17") \
	"\t\tbra " NAME "_Low\n" \
	NAME "_LowWord:\n" \
	"\t\tstz [D + 17]\n" \
	NAME "_Low:\n" \
	"\t\tasl [D + 15]\n" \
	IR816_DIVIDE_STEPS_16("15") \
	"\t\tsta [D + 5]\n\t\tbra " NAME "_Done\n" \
	NAME "_WideDivisor:\n" \
	"\t\tlda [D + 17]\n\t\tbne " NAME "_Long\n" \
	"\t\tlda [D + 15]\n\t\tsta [D + 5]\n\t\tstz [D + 15]\n\t\tbra " NAME "_Done\n" \
	NAME "_Long:\n" \
	"\t\tsta [D + 5]\n\t\tstz [D + 17]\n\t\tldx 16\n\t\tasl [D + 15]\n" \
	NAME "_Bit:\n" \
	"\t\trol [D + 5]\n\t\trol [D + 7]\n" \
	"\t\tbcs " NAME "_Over\n" \
	"\t\tsec\n\t\tlda [D + 5]\n\t\tsbc [D + 19]\n\t\ttay\n\t\tlda [D + 7]\n\t\tsbc [D + 21]\n" \
	"\t\tbcc " NAME "_Next\n" \
	"\t\tbra " NAME "_Keep\n" \
	NAME "_Over:\n" \
	"\t\tsec\n\t\tlda [D + 5]\n\t\tsbc [D + 19]\n\t\ttay\n\t\tlda [D + 7]\n\t\tsbc [D + 21]\n\t\tsec\n" \
	NAME "_Keep:\n" \
	"\t\tsta [D + 7]\n\t\tsty [D + 5]\n" \
	NAME "_Next:\n" \
	"\t\trol [D + 15]\n\t\tdex\n\t\tbne " NAME "_Bit\n" \
	NAME "_Done:\n" \
	"\t\tlda [D + " RESULT_HIGH "]\n\t\ttax\n\t\tlda [D + " RESULT_LOW "]\n\t\trtl\n"


//The 16 bit divide n / d with Garth Wilson's reciprocals (Libc65/MathTbls/MathInvert.asm, MathInvert): d is shifted
//up s places to d' (its top bit set) and R = round(2^32 / d') is 1:Rlo (17 bits; d' = $8000, a power of two, is
//a shift instead).  The quotient is (n * R) >> (32 - s): T = n + nH * rH + ((nH * rL + nL * rH) >> 8) is
//(n * R) >> 16 less nL * rL and the carries below it, then q = T >> (16 - s).  Checked for every n and d: q is never
//over and at most 1 under, so the remainder n - q * d (two byte products: q or d is under 256) is taken from once
//more when it is d or over.  n < d is q = 0, d >= $8000 is q = 1 and d = 0 is q = $FFFF, r = n (as the loop gave).
//A product's index is built a byte at a time with A 8 bits wide (IR816_BYTES_INDEX); the shift right jumps into a
//run of lsr (_Shifts by 2s).  It runs on the caller's D (IR816ScratchRoutine): n is at D + 15 and d at D + 19 (the
//scratch arguments), and D + 5 is q, D + 7 the remainder, D + 9 2s and D + 11 Rlo (0 for a power of two).  X is 0 at
//the end, so the 32 bit divide can return through it (IR816_DIVIDE_32).
#define IR816_INVERT_BYTES(NAME) \
	"\t\tasl A\n\t\ttax\n\t\tbcs " NAME "_High\n" \
	"\t\tlda [MathInvert + X]:24\n\t\tbra " NAME "_Got\n" \
	NAME "_High:\n" \
	"\t\tlda [MathInvert + 0x10000 + X]:24\n" \
	NAME "_Got:\n"


//A = (the byte at D + HIGH << 8) | the byte at D + LOW.
#define IR816_BYTES_INDEX(HIGH, LOW) \
	"\t\tsep m\n\t\tlda [D + " HIGH "]\n\t\txba\n\t\tlda [D + " LOW "]\n\t\trep m\n"


#define IR816_DIVIDE_16(NAME, RESULT) \
	"\t\t.width (A, X), 16\n" \
	"\t\t.relax\n" \
	"\t\tstz [D + 5]\n" \
	"\t\tlda [D + 15]\n\t\tcmp [D + 19]\n\t\tbcs " NAME "_Divides\n" \
	"\t\tsta [D + 7]\n\t\tbra " NAME "_Done\n" \
	NAME "_Divides:\n" \
	"\t\tldx [D + 19]\n\t\tbpl " NAME "_Small\n" \
	"\t\tsbc [D + 19]\n\t\tsta [D + 7]\n\t\tinc [D + 5]\n\t\tbra " NAME "_Done\n" \
	NAME "_Small:\n" \
	"\t\ttxa\n\t\tbeq " NAME "_Zero\n" \
	"\t\tldx 0\n\t\tcmp 256\n\t\tbcs " NAME "_Shift\n" \
	"\t\txba\n\t\tldx 8\n" \
	NAME "_Shift:\n" \
	"\t\tinx\n\t\tasl A\n\t\tbcc " NAME "_Shift\n" \
	"\t\tror A\n\t\tdex\n\t\tstx [D + 9]\n\t\tasl [D + 9]\n" \
	"\t\tcmp 0x8000\n\t\tbeq " NAME "_Power\n" \
	"\t\tasl A\n" \
	IR816_INVERT_BYTES(NAME "_Inv") \
	"\t\tsta [D + 11]\n" \
	"\t\tsep m\n\t\tlda [D + 16]\n\t\tbeq " NAME "_SmallN\n\t\txba\n\t\tlda [D + 11]\n\t\trep m\n" \
	IR816_MULTIPLY_BYTES(NAME "_HL") \
	"\t\tsta [D + 5]\n" \
	IR816_BYTES_INDEX("12", "15") \
	IR816_MULTIPLY_BYTES(NAME "_LH") \
	"\t\tclc\n\t\tadc [D + 5]\n\t\txba\n\t\tand 0xFF\n\t\tbcc C + 3\n\t\tora 0x100\n\t\tsta [D + 5]\n" \
	IR816_BYTES_INDEX("16", "12") \
	IR816_MULTIPLY_BYTES(NAME "_HH") \
	"\t\tclc\n\t\tadc [D + 5]\n\t\tbra " NAME "_AddN\n" \
	NAME "_SmallN:\n" \
	"\t\t.width A, 8\n" \
	"\t\tlda [D + 12]\n\t\txba\n\t\tlda [D + 15]\n\t\trep m\n" \
	IR816_MULTIPLY_BYTES(NAME "_LHn") \
	"\t\txba\n\t\tand 0xFF\n" \
	NAME "_AddN:\n" \
	"\t\tclc\n\t\tadc [D + 15]\n\t\tror A\n" \
	"\t\tldx [D + 9]\n\t\tjmp [" NAME "_Shifts + X]\n" \
	NAME "_Chain:\n" \
	"\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n\t\tlsr A\n" \
	"\t\tsta [D + 5]\n" \
	"\t\tldy [D + 11]\n\t\tbeq " NAME "_Done\n" \
	IR816_BYTES_INDEX("5", "19") \
	IR816_MULTIPLY_BYTES(NAME "_QD") \
	"\t\tsta [D + 7]\n" \
	"\t\tsep m\n\t\tlda [D + 20]\n\t\tbeq " NAME "_NoHighD\n\t\txba\n\t\tlda [D + 5]\n\t\trep m\n" \
	IR816_MULTIPLY_BYTES(NAME "_QDH") \
	IR816_MULTIPLY_ADD_BYTE_1("[D + 7]") \
	NAME "_NoHighD:\n" \
	"\t\trep m\n" \
	"\t\tsep m\n\t\tlda [D + 6]\n\t\tbeq " NAME "_NoHighQ\n\t\txba\n\t\tlda [D + 19]\n\t\trep m\n" \
	IR816_MULTIPLY_BYTES(NAME "_QHD") \
	IR816_MULTIPLY_ADD_BYTE_1("[D + 7]") \
	NAME "_NoHighQ:\n" \
	"\t\trep m\n" \
	"\t\tlda [D + 15]\n\t\tsec\n\t\tsbc [D + 7]\n\t\tcmp [D + 19]\n\t\tbcc " NAME "_Remainder\n" \
	"\t\tsbc [D + 19]\n\t\tinc [D + 5]\n" \
	NAME "_Remainder:\n" \
	"\t\tsta [D + 7]\n\t\tbra " NAME "_Done\n" \
	NAME "_Power:\n" \
	"\t\tstz [D + 11]\n\t\tlda [D + 19]\n\t\tdec A\n\t\tand [D + 15]\n\t\tsta [D + 7]\n" \
	"\t\tlda [D + 15]\n\t\tldx [D + 9]\n\t\tjmp [" NAME "_Shifts + X]\n" \
	NAME "_Zero:\n" \
	"\t\tlda 0xFFFF\n\t\tsta [D + 5]\n\t\tlda [D + 15]\n\t\tsta [D + 7]\n" \
	NAME "_Done:\n" \
	"\t\tlda [D + " RESULT "]\n\t\tldx 0\n\t\trtl\n" \
	NAME "_Shifts:\n" \
	"\t\t.word (" NAME "_Chain + 0) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 1) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 2) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 3) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 4) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 5) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 6) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 7) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 8) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 9) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 10) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 11) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 12) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 13) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 14) & 0xFFFF\n" \
	"\t\t.word (" NAME "_Chain + 15) & 0xFFFF\n"


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
	"\t\tlda [MathMult + 0x10000 + X]:24\n" \
	NAME "_Got:\n"


//Adds the product in A a byte up into the word at WORD (only its low byte reaches it).
#define IR816_MULTIPLY_ADD_BYTE_1(WORD) \
	"\t\txba\n\t\tand 0xFF00\n\t\tclc\n\t\tadc " WORD "\n\t\tsta " WORD "\n"


//The multiplies run on the caller's D (IR816ScratchRoutine): a is at D + 15 and b at D + 19 (the scratch arguments),
//and the result is put together at D + 5.  R816_Mul32's result is D + 5 to D + 8, a's bytes are D + 15 to D + 18 and
//b's D + 19 to D + 22.  With b under 256 (checked first) it is a's bytes times b (_X: a's high word skipped when it is
//0, its top byte when that is).  Otherwise the result starts at 0 and the product is up to ten byte products: x and y
//are the bytes' offsets, the index is built with A 8 bits wide and ADD puts the product (in A) in the result.  A
//product of a 0 byte is skipped (A is made 16 bits again at _Zero), as are those of a high word that is 0.  A product
//a byte up adds to the result's middle word and carries into its top byte (8 bits: only that byte); two bytes up adds
//to its high word.
#define IR816_MULTIPLY_32(NAME, X, Y, ADD) \
	"\t\tsep m\n\t\tlda [D + " Y "]\n\t\tbeq R816_Mul32_" NAME "_Zero\n\t\txba\n\t\tlda [D + " X "]\n\t\tbeq R816_Mul32_" NAME "_Zero\n\t\trep m\n" \
	IR816_MULTIPLY_BYTES("R816_Mul32_" NAME) \
	ADD \
	"R816_Mul32_" NAME "_Zero:\n" \
	"\t\trep m\n"


#define IR816_MULTIPLY_ADD_BYTES_1 \
	"\t\tclc\n\t\tadc [D + 6]\n\t\tsta [D + 6]\n\t\tbcc C + 6\n\t\tsep m\n\t\tinc [D + 8]\n\t\trep m\n"


#define IR816_MULTIPLY_ADD_BYTES_2 \
	"\t\tclc\n\t\tadc [D + 7]\n\t\tsta [D + 7]\n"


//uint16 __builtin_bcd_add(uint16* pauiDest, uint16* pauiSource, uint16 uiWords, uint16 uiCarry) and
//__builtin_bcd_subtract (BaseLib's BCDAdd and BCDSubtract), with the compiler's calling convention: D is pointed at
//the stack, so the destination is at D + 6, the source at D + 10, the words at D + 14 and the carry (or borrow) at
//D + 16.  The carry in goes to C (a borrow is C clear), then each word is added (or subtracted) in decimal mode,
//the least significant first, and the carry out is returned.
#define IR816_BCD(NAME, CARRY_IN, OPERATION, CARRY_OUT) \
	"\t\t.width (A, X), 16\n" \
	"\t\tphd\n\t\ttsc\n\t\ttcd\n" \
	"\t\tlda [D + 16]\n" CARRY_IN "\t\tlsr A\n" \
	"\t\tldy 0\n\t\tldx [D + 14]\n\t\tbeq " NAME "_Done\n" \
	"\t\tsed\n" \
	NAME "_Loop:\n" \
	"\t\tlda [[D + 6]:24 + Y]\n\t\t" OPERATION " [[D + 10]:24 + Y]\n\t\tsta [[D + 6]:24 + Y]\n" \
	"\t\tiny\n\t\tiny\n\t\tdex\n\t\tbne " NAME "_Loop\n" \
	"\t\tcld\n" \
	NAME "_Done:\n" \
	"\t\tlda 0\n\t\trol A\n" CARRY_OUT \
	"\t\tpld\n\t\trtl\n"


//////////////////////////////////////////////////////////////////////////
//
// The runtime routines written by code rather than as text (pfAppend):
// an instruction line, then a label line.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AsmLine(CChars* psz, const char* szFormat, ...)
{
	char	sz[160];
	va_list	vaMarker;

	va_start(vaMarker, szFormat);
	vsnprintf(sz, sizeof(sz), szFormat, vaMarker);
	va_end(vaMarker);
	psz->Append("\t\t")->Append(sz)->AppendNewLine();
}


static void IR816AsmLabel(CChars* psz, const char* szFormat, ...)
{
	char	sz[160];
	va_list	vaMarker;

	va_start(vaMarker, szFormat);
	vsnprintf(sz, sizeof(sz), szFormat, vaMarker);
	va_end(vaMarker);
	psz->Append(sz)->Append(":\n");
}


#define IR816_SHIFT_SHL		0


#define IR816_SHIFT_LSHR	1


#define IR816_SHIFT_ASHR	2


#define IR816_SHIFT_FILL	13		//AShr's sign word ($FFFF or 0), in the caller's scratch count word.


//////////////////////////////////////////////////////////////////////////
//
// A shift routine's operand for the value's byte iByte: [D + iByte + X]
// for the 64 bit ones (the value in the caller's frame at D + X), the
// caller's scratch argument for the 32 bit ones.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816ShiftOperand(char* szDest, int iByte, bool bIndexed)
{
	if (!bIndexed)
	{
		snprintf(szDest, 32, "[D + %d]", IR816_SCRATCH_ARGUMENT_A + iByte);
	}
	else if (iByte == 0)
	{
		snprintf(szDest, 32, "[D + X]");
	}
	else
	{
		snprintf(szDest, 32, "[D + %d + X]", iByte);
	}
	return szDest;
}


//////////////////////////////////////////////////////////////////////////
//
// The way out of a shift routine: rtl for the 64 bit ones (the value is
// shifted where it is), to _Done for the 32 bit ones (it returns the value
// in A and X).
//
//////////////////////////////////////////////////////////////////////////
static void IR816ShiftExit(CChars* psz, const char* szName, bool bIndexed)
{
	if (bIndexed)
	{
		IR816AsmLine(psz, "rtl");
	}
	else
	{
		IR816AsmLine(psz, "brl %s_Done", szName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// R816_Shl64, R816_LShr64 and R816_AShr64 shift the 64 bit value in the
// caller's frame at D + X where it is; R816_Shl32, R816_LShr32 and
// R816_AShr32 the 32 bit value at the caller's scratch argument a (D + 15)
// and return it in A (low) and X (high).  The count is in A (its low 6
// bits; a 32 bit value shifted 32 or more is filled).  The whole words go
// first (q = count / 16, so only the words left, m of them, are shifted
// further: a shift by 52 shifts one word 4 bits), then a byte (a word read
// a byte along), then the 1 to 7 bits left: one word in A and the others
// rolled in memory, counted down in Y (64 bits: X is the value's place), or
// jumped into an unrolled run of 7 steps (32 bits).  AShr's sign word is
// worked out first, in the caller's scratch count word.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AppendShift(CChars* psz, const char* szName, int iKind, int iWords)
{
	char	szA[32];
	char	szB[32];
	bool	bIndexed;
	bool	bLeft;
	int		q;
	int		m;
	int		k;
	int		i;
	int		iLive;	//The word kept in A for the bits: the lowest left for shl, the highest for shr.

	bIndexed = iWords == 4;
	bLeft = iKind == IR816_SHIFT_SHL;

	IR816AsmLine(psz, ".width (A, X), 16");
	IR816AsmLine(psz, ".relax");
	IR816AsmLine(psz, "and 63");
	IR816AsmLine(psz, "bne %s_Some", szName);
	IR816ShiftExit(psz, szName, bIndexed);
	IR816AsmLabel(psz, "%s_Some", szName);
	if (iKind == IR816_SHIFT_ASHR)
	{
		IR816AsmLine(psz, "tay");
		IR816AsmLine(psz, "lda %s", IR816ShiftOperand(szA, 2 * iWords - 2, bIndexed));
		IR816AsmLine(psz, "cmp 0x8000");
		IR816AsmLine(psz, "lda 0");
		IR816AsmLine(psz, "bcc %s_Sign", szName);
		IR816AsmLine(psz, "dec A");
		IR816AsmLabel(psz, "%s_Sign", szName);
		IR816AsmLine(psz, "sta [D + %d]", IR816_SHIFT_FILL);
		IR816AsmLine(psz, "tya");
	}
	IR816AsmLine(psz, "cmp 32");
	IR816AsmLine(psz, "bcs %s_Q%s", szName, (iWords == 4) ? "23" : "2");
	IR816AsmLine(psz, "cmp 16");
	IR816AsmLine(psz, "bcs %s_Q1", szName);

	for (q = 0; q <= iWords; q++)
	{
		if ((iWords == 4) && (q == 2))
		{
			IR816AsmLabel(psz, "%s_Q23", szName);
			IR816AsmLine(psz, "cmp 48");
			IR816AsmLine(psz, "bcs %s_Q3", szName);
		}
		if (q > 0)
		{
			IR816AsmLabel(psz, "%s_Q%d", szName, q);
		}
		if (q == iWords)
		{
			//32 bits shifted 32 or more: all filled.
			if (iKind == IR816_SHIFT_ASHR)
			{
				IR816AsmLine(psz, "lda [D + %d]", IR816_SHIFT_FILL);
			}
			for (i = 0; i < iWords; i++)
			{
				IR816AsmLine(psz, "%s %s", (iKind == IR816_SHIFT_ASHR) ? "sta" : "stz", IR816ShiftOperand(szA, 2 * i, bIndexed));
			}
			IR816ShiftExit(psz, szName, bIndexed);
			continue;
		}

		//The whole words: q of them shifted in (0, or the sign).
		if (q > 0)
		{
			IR816AsmLine(psz, "tay");
			if (bLeft)
			{
				for (i = iWords - 1; i >= q; i--)
				{
					IR816AsmLine(psz, "lda %s", IR816ShiftOperand(szA, 2 * (i - q), bIndexed));
					IR816AsmLine(psz, "sta %s", IR816ShiftOperand(szA, 2 * i, bIndexed));
				}
				for (i = q - 1; i >= 0; i--)
				{
					IR816AsmLine(psz, "stz %s", IR816ShiftOperand(szA, 2 * i, bIndexed));
				}
			}
			else
			{
				for (i = 0; i < iWords - q; i++)
				{
					IR816AsmLine(psz, "lda %s", IR816ShiftOperand(szA, 2 * (i + q), bIndexed));
					IR816AsmLine(psz, "sta %s", IR816ShiftOperand(szA, 2 * i, bIndexed));
				}
				if (iKind == IR816_SHIFT_ASHR)
				{
					IR816AsmLine(psz, "lda [D + %d]", IR816_SHIFT_FILL);
				}
				for (i = iWords - q; i < iWords; i++)
				{
					IR816AsmLine(psz, "%s %s", (iKind == IR816_SHIFT_ASHR) ? "sta" : "stz", IR816ShiftOperand(szA, 2 * i, bIndexed));
				}
			}
			IR816AsmLine(psz, "tya");
			IR816AsmLine(psz, "and 15");
			IR816AsmLine(psz, "bne %s_B%d", szName, q);
			IR816ShiftExit(psz, szName, bIndexed);
			IR816AsmLabel(psz, "%s_B%d", szName, q);
		}

		//A byte: the m words left read a byte along.
		m = iWords - q;
		iLive = bLeft ? q : iWords - 1 - q;
		IR816AsmLine(psz, "cmp 8");
		IR816AsmLine(psz, "bcc %s_R%d", szName, q);
		IR816AsmLine(psz, "tay");
		if (bLeft)
		{
			for (k = iWords - 1; k > q; k--)
			{
				IR816AsmLine(psz, "lda %s", IR816ShiftOperand(szA, 2 * k - 1, bIndexed));
				IR816AsmLine(psz, "sta %s", IR816ShiftOperand(szA, 2 * k, bIndexed));
			}
			IR816AsmLine(psz, "lda %s", IR816ShiftOperand(szA, 2 * q, bIndexed));
			IR816AsmLine(psz, "xba");
			IR816AsmLine(psz, "and 0xFF00");
			IR816AsmLine(psz, "sta %s", IR816ShiftOperand(szA, 2 * q, bIndexed));
		}
		else
		{
			for (k = 0; k < iLive; k++)
			{
				IR816AsmLine(psz, "lda %s", IR816ShiftOperand(szA, 2 * k + 1, bIndexed));
				IR816AsmLine(psz, "sta %s", IR816ShiftOperand(szA, 2 * k, bIndexed));
			}
			IR816AsmLine(psz, "lda %s", IR816ShiftOperand(szA, 2 * iLive, bIndexed));
			IR816AsmLine(psz, "xba");
			IR816AsmLine(psz, "and 0xFF");
			if (iKind == IR816_SHIFT_ASHR)
			{
				IR816AsmLine(psz, "eor 0x80");
				IR816AsmLine(psz, "sec");
				IR816AsmLine(psz, "sbc 0x80");
			}
			IR816AsmLine(psz, "sta %s", IR816ShiftOperand(szA, 2 * iLive, bIndexed));
		}
		IR816AsmLine(psz, "tya");
		IR816AsmLine(psz, "and 7");
		IR816AsmLine(psz, "bne %s_R%d", szName, q);
		IR816ShiftExit(psz, szName, bIndexed);

		//The 1 to 7 bits left, with the live word in A.
		IR816AsmLabel(psz, "%s_R%d", szName, q);
		if (bIndexed)
		{
			IR816AsmLine(psz, "tay");
			IR816AsmLine(psz, "lda %s", IR816ShiftOperand(szA, 2 * iLive, bIndexed));
			IR816AsmLabel(psz, "%s_L%d", szName, q);
		}
		else
		{
			IR816AsmLine(psz, "asl A");
			IR816AsmLine(psz, "tax");
			IR816AsmLine(psz, "lda %s", IR816ShiftOperand(szA, 2 * iLive, bIndexed));
			IR816AsmLine(psz, "jmp [%s_T%d + X]", szName, q);
			IR816AsmLabel(psz, "%s_T%d", szName, q);
			for (i = 0; i <= IR816_SHIFT_STEPS; i++)
			{
				if (i == 0)
				{
					IR816AsmLine(psz, ".word %s_E%d & 0xFFFF", szName, q);
				}
				else
				{
					IR816AsmLine(psz, ".word %s_S%d_%d & 0xFFFF", szName, q, IR816_SHIFT_STEPS - i);
				}
			}
		}
		for (i = 0; i < (bIndexed ? 1 : IR816_SHIFT_STEPS); i++)
		{
			if (!bIndexed)
			{
				IR816AsmLabel(psz, "%s_S%d_%d", szName, q, i);
			}
			if (bLeft)
			{
				IR816AsmLine(psz, "asl A");
				for (k = q + 1; k < iWords; k++)
				{
					IR816AsmLine(psz, "rol %s", IR816ShiftOperand(szA, 2 * k, bIndexed));
				}
			}
			else
			{
				if (iKind == IR816_SHIFT_ASHR)
				{
					IR816AsmLine(psz, "cmp 0x8000");
					IR816AsmLine(psz, "ror A");
				}
				else
				{
					IR816AsmLine(psz, "lsr A");
				}
				for (k = iLive - 1; k >= 0; k--)
				{
					IR816AsmLine(psz, "ror %s", IR816ShiftOperand(szA, 2 * k, bIndexed));
				}
			}
		}
		if (bIndexed)
		{
			IR816AsmLine(psz, "dey");
			IR816AsmLine(psz, "bne %s_L%d", szName, q);
		}
		else
		{
			IR816AsmLabel(psz, "%s_E%d", szName, q);
		}
		IR816AsmLine(psz, "sta %s", IR816ShiftOperand(szB, 2 * iLive, bIndexed));
		IR816ShiftExit(psz, szName, bIndexed);
	}

	if (!bIndexed)
	{
		IR816AsmLabel(psz, "%s_Done", szName);
		IR816AsmLine(psz, "ldx [D + %d]", IR816_SCRATCH_ARGUMENT_A + 2);
		IR816AsmLine(psz, "lda [D + %d]", IR816_SCRATCH_ARGUMENT_A);
		IR816AsmLine(psz, "rtl");
	}
}


static void IR816AppendShl64(CChars* psz)	{ IR816AppendShift(psz, "R816_Shl64", IR816_SHIFT_SHL, 4); }
static void IR816AppendLShr64(CChars* psz)	{ IR816AppendShift(psz, "R816_LShr64", IR816_SHIFT_LSHR, 4); }
static void IR816AppendAShr64(CChars* psz)	{ IR816AppendShift(psz, "R816_AShr64", IR816_SHIFT_ASHR, 4); }
static void IR816AppendShl32(CChars* psz)	{ IR816AppendShift(psz, "R816_Shl32", IR816_SHIFT_SHL, 2); }
static void IR816AppendLShr32(CChars* psz)	{ IR816AppendShift(psz, "R816_LShr32", IR816_SHIFT_LSHR, 2); }
static void IR816AppendAShr32(CChars* psz)	{ IR816AppendShift(psz, "R816_AShr32", IR816_SHIFT_ASHR, 2); }


//////////////////////////////////////////////////////////////////////////
//
// A carry out of an add into the 64 bit product at D + 5 goes into its
// byte iByte (a word inc, and the next word's when that wraps); none past
// the product (the sum never is bigger than the whole product).
//
//////////////////////////////////////////////////////////////////////////
static void IR816AppendProductCarry(CChars* psz, const char* szName, int iRow, int iCarry, int iByte)
{
	if (iByte > 7)
	{
		return;
	}
	IR816AsmLine(psz, "bcc %s_C%d_%d", szName, iRow, iCarry);
	for (; iByte <= 7; iByte += 2)
	{
		IR816AsmLine(psz, "inc [D + %d]", IR816_SCRATCH_RESULT + iByte);
		if (iByte + 2 <= 7)
		{
			IR816AsmLine(psz, "bne %s_C%d_%d", szName, iRow, iCarry);
		}
	}
	IR816AsmLabel(psz, "%s_C%d_%d", szName, iRow, iCarry);
}


//////////////////////////////////////////////////////////////////////////
//
// Row i of R816_UMulWide32 (Y is a's byte i * 512, its bank in szTable):
// the byte products of b's bytes (each doubled, on the stack at S + 1, 3,
// 5 and 7) and a's byte i, read from Garth Wilson's table (the word at
// MathMult + ((x << 8) | y) * 2 is x * y) and added to the product a byte
// apart: the even ones (b0 and b2) as a 32 bit word pair at byte i, the
// odd ones (b1 and b3) at byte i + 1.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AppendProductRow(CChars* psz, const char* szName, const char* szTable, int iRow, int iBank)
{
	static const int aiOrder[4] = { 0, 2, 1, 3 };
	int		i;
	int		j;

	for (i = 0; i < 4; i++)
	{
		j = aiOrder[i];
		IR816AsmLine(psz, "tya");
		IR816AsmLine(psz, "ora [S + %d]", 1 + 2 * j);
		IR816AsmLine(psz, "tax");
		IR816AsmLine(psz, "lda [%s + X]:24", szTable);
		if (j < 2)
		{
			IR816AsmLine(psz, "clc");
		}
		IR816AsmLine(psz, "adc [D + %d]", IR816_SCRATCH_RESULT + iRow + j);
		IR816AsmLine(psz, "sta [D + %d]", IR816_SCRATCH_RESULT + iRow + j);
		if (j >= 2)
		{
			IR816AppendProductCarry(psz, szName, 2 * iRow + iBank, j - 2, iRow + j + 2);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// R816_UMulWide32: the 64 bit product of the 32 bit a (D + 15) and b
// (D + 19), at D + 5 (a scratch routine: the caller's scratch bytes; A, X
// and Y changed).  b's bytes doubled are pushed; each row (a byte of a that
// isn't 0) has a's byte * 512 in Y and the table's bank by its top bit, so
// a byte product is tya, ora, tax and a long load.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AppendUMulWide32(CChars* psz)
{
	const char*	szName = "R816_UMulWide32";
	int			i;

	IR816AsmLine(psz, ".width (A, X), 16");
	IR816AsmLine(psz, ".relax");
	for (i = 3; i >= 0; i--)
	{
		IR816AsmLine(psz, "lda [D + %d]", IR816_SCRATCH_ARGUMENT_B + i);
		IR816AsmLine(psz, "and 0xFF");
		IR816AsmLine(psz, "asl A");
		IR816AsmLine(psz, "pha");
	}
	for (i = 0; i < 4; i++)
	{
		IR816AsmLine(psz, "stz [D + %d]", IR816_SCRATCH_RESULT + 2 * i);
	}
	for (i = 0; i < 4; i++)
	{
		IR816AsmLine(psz, "lda [D + %d]", IR816_SCRATCH_ARGUMENT_A + i);
		IR816AsmLine(psz, "and 0xFF");
		IR816AsmLine(psz, "beq %s_Next%d", szName, i);
		IR816AsmLine(psz, "xba");
		IR816AsmLine(psz, "asl A");
		IR816AsmLine(psz, "tay");
		IR816AsmLine(psz, "bcs %s_High%d", szName, i);
		IR816AppendProductRow(psz, szName, "MathMult", i, 0);
		IR816AsmLine(psz, "brl %s_Next%d", szName, i);
		IR816AsmLabel(psz, "%s_High%d", szName, i);
		IR816AppendProductRow(psz, szName, "MathMult + 0x10000", i, 1);
		IR816AsmLabel(psz, "%s_Next%d", szName, i);
	}
	IR816AsmLine(psz, "tsc");
	IR816AsmLine(psz, "clc");
	IR816AsmLine(psz, "adc 8");
	IR816AsmLine(psz, "tcs");
	IR816AsmLine(psz, "rtl");
}


//////////////////////////////////////////////////////////////////////////
//
// R816_Mul64: the low 64 bits of a * b, with the compiler's convention (the
// result's pointer pushed last, then a, then b; D pointed at its own frame
// of 26 bytes, scratch bytes and the cross products' sum at D + 23).  aL *
// bL (R816_UMulWide32) plus (aL * bH + aH * bL) (R816_Mul32, each skipped
// when the high word is 0) 32 bits up.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AppendMul64(CChars* psz)
{
	const char*	szName = "R816_Mul64";
	int			iA;
	int			iB;
	int			iPointer;
	int			i;

	iPointer = 26 + IR816_LINK_BYTES + 1;
	iA = iPointer + 4;
	iB = iA + 8;
	IR816AsmLine(psz, ".width (A, X), 16");
	IR816AsmLine(psz, ".relax");
	IR816AsmLine(psz, "phd");
	IR816AsmLine(psz, "tsc");
	IR816AsmLine(psz, "sec");
	IR816AsmLine(psz, "sbc 26");
	IR816AsmLine(psz, "tcs");
	IR816AsmLine(psz, "tcd");
	IR816AsmLine(psz, "stz [D + 23]");
	IR816AsmLine(psz, "stz [D + 25]");

	//aL * bH.
	IR816AsmLine(psz, "lda [D + %d]", iB + 4);
	IR816AsmLine(psz, "ora [D + %d]", iB + 6);
	IR816AsmLine(psz, "beq %s_NoHighB", szName);
	for (i = 0; i < 2; i++)
	{
		IR816AsmLine(psz, "lda [D + %d]", iA + 2 * i);
		IR816AsmLine(psz, "sta [D + %d]", IR816_SCRATCH_ARGUMENT_A + 2 * i);
		IR816AsmLine(psz, "lda [D + %d]", iB + 4 + 2 * i);
		IR816AsmLine(psz, "sta [D + %d]", IR816_SCRATCH_ARGUMENT_B + 2 * i);
	}
	IR816AsmLine(psz, "jsl R816_Mul32");
	IR816AsmLine(psz, "sta [D + 23]");
	IR816AsmLine(psz, "stx [D + 25]");
	IR816AsmLabel(psz, "%s_NoHighB", szName);

	//aH * bL.
	IR816AsmLine(psz, "lda [D + %d]", iA + 4);
	IR816AsmLine(psz, "ora [D + %d]", iA + 6);
	IR816AsmLine(psz, "beq %s_NoHighA", szName);
	for (i = 0; i < 2; i++)
	{
		IR816AsmLine(psz, "lda [D + %d]", iA + 4 + 2 * i);
		IR816AsmLine(psz, "sta [D + %d]", IR816_SCRATCH_ARGUMENT_A + 2 * i);
		IR816AsmLine(psz, "lda [D + %d]", iB + 2 * i);
		IR816AsmLine(psz, "sta [D + %d]", IR816_SCRATCH_ARGUMENT_B + 2 * i);
	}
	IR816AsmLine(psz, "jsl R816_Mul32");
	IR816AsmLine(psz, "clc");
	IR816AsmLine(psz, "adc [D + 23]");
	IR816AsmLine(psz, "sta [D + 23]");
	IR816AsmLine(psz, "txa");
	IR816AsmLine(psz, "adc [D + 25]");
	IR816AsmLine(psz, "sta [D + 25]");
	IR816AsmLabel(psz, "%s_NoHighA", szName);

	//aL * bL, and the cross products 32 bits up.
	for (i = 0; i < 2; i++)
	{
		IR816AsmLine(psz, "lda [D + %d]", iA + 2 * i);
		IR816AsmLine(psz, "sta [D + %d]", IR816_SCRATCH_ARGUMENT_A + 2 * i);
		IR816AsmLine(psz, "lda [D + %d]", iB + 2 * i);
		IR816AsmLine(psz, "sta [D + %d]", IR816_SCRATCH_ARGUMENT_B + 2 * i);
	}
	IR816AsmLine(psz, "jsl R816_UMulWide32");
	IR816AsmLine(psz, "lda [D + %d]", IR816_SCRATCH_RESULT + 4);
	IR816AsmLine(psz, "clc");
	IR816AsmLine(psz, "adc [D + 23]");
	IR816AsmLine(psz, "sta [D + %d]", IR816_SCRATCH_RESULT + 4);
	IR816AsmLine(psz, "lda [D + %d]", IR816_SCRATCH_RESULT + 6);
	IR816AsmLine(psz, "adc [D + 25]");
	IR816AsmLine(psz, "sta [D + %d]", IR816_SCRATCH_RESULT + 6);
	for (i = 0; i < 4; i++)
	{
		IR816AsmLine(psz, "ldy %d", 2 * i);
		IR816AsmLine(psz, "lda [D + %d]", IR816_SCRATCH_RESULT + 2 * i);
		IR816AsmLine(psz, "sta [[D + %d]:24 + Y]", iPointer);
	}
	IR816AsmLine(psz, "tsc");
	IR816AsmLine(psz, "clc");
	IR816AsmLine(psz, "adc 26");
	IR816AsmLine(psz, "tcs");
	IR816AsmLine(psz, "pld");
	IR816AsmLine(psz, "rtl");
}


static const SIR816Routine gasIR816Routines[] =
{
	{ "R816_UDiv64", IR816_DIVIDE_64("R816_UDiv64", "18"), false, NULL },
	{ "R816_URem64", IR816_DIVIDE_64("R816_URem64", "3"), false, NULL },
	{ "R816_UDiv32", IR816_DIVIDE_32("R816_UDiv32", "R816_UDiv16", "15", "17"), true, "R816_UDiv16" },
	{ "R816_URem32", IR816_DIVIDE_32("R816_URem32", "R816_URem16", "5", "7"), true, "R816_URem16" },
	{ "R816_UDiv16", IR816_DIVIDE_16("R816_UDiv16", "5"), true, NULL },
	{ "R816_URem16", IR816_DIVIDE_16("R816_URem16", "7"), true, NULL },
	{ "R816_Mul16",
		"\t\t.width (A, X), 16\n"
		IR816_BYTES_INDEX("19", "15")
		IR816_MULTIPLY_BYTES("R816_Mul16_LL")
		"\t\tsta [D + 5]\n"
		"\t\tsep m\n\t\tlda [D + 20]\n\t\tbeq R816_Mul16_NoHighB\n\t\txba\n\t\tlda [D + 15]\n\t\trep m\n"
		IR816_MULTIPLY_BYTES("R816_Mul16_LH")
		IR816_MULTIPLY_ADD_BYTE_1("[D + 5]")
		"R816_Mul16_NoHighB:\n"
		"\t\trep m\n"
		"\t\tsep m\n\t\tlda [D + 16]\n\t\tbeq R816_Mul16_Done\n\t\txba\n\t\tlda [D + 19]\n\t\trep m\n"
		IR816_MULTIPLY_BYTES("R816_Mul16_HL")
		IR816_MULTIPLY_ADD_BYTE_1("[D + 5]")
		"R816_Mul16_Done:\n"
		"\t\trep m\n"
		"\t\tlda [D + 5]\n\t\trtl\n", true, NULL },
	{ "R816_Mul32",
		"\t\t.width (A, X), 16\n"
		"\t\t.relax\n"
		"\t\tlda [D + 21]\n\t\tbne R816_Mul32_Wide\n\t\tlda [D + 19]\n\t\tcmp 256\n\t\tbcc R816_Mul32_X\n"
		"R816_Mul32_Wide:\n"
		"\t\tstz [D + 5]\n\t\tstz [D + 7]\n"
		IR816_MULTIPLY_32("LL", "15", "19", "\t\tsta [D + 5]\n")
		IR816_MULTIPLY_32("HH", "16", "20", "\t\tsta [D + 7]\n")
		IR816_MULTIPLY_32("LH", "15", "20", IR816_MULTIPLY_ADD_BYTES_1)
		IR816_MULTIPLY_32("HL", "16", "19", IR816_MULTIPLY_ADD_BYTES_1)
		"\t\tlda [D + 21]\n\t\tbeq R816_Mul32_NoHighB\n"
		IR816_MULTIPLY_32("B2", "15", "21", IR816_MULTIPLY_ADD_BYTES_2)
		IR816_MULTIPLY_32("B3", "15", "22", IR816_MULTIPLY_ADD_BYTE_1("[D + 7]"))
		IR816_MULTIPLY_32("B2A1", "16", "21", IR816_MULTIPLY_ADD_BYTE_1("[D + 7]"))
		"R816_Mul32_NoHighB:\n"
		"\t\tlda [D + 17]\n\t\tbeq R816_Mul32_Done\n"
		IR816_MULTIPLY_32("A2", "17", "19", IR816_MULTIPLY_ADD_BYTES_2)
		IR816_MULTIPLY_32("A3", "18", "19", IR816_MULTIPLY_ADD_BYTE_1("[D + 7]"))
		IR816_MULTIPLY_32("A2B1", "17", "20", IR816_MULTIPLY_ADD_BYTE_1("[D + 7]"))
		"R816_Mul32_Done:\n"
		"\t\tlda [D + 7]\n\t\ttax\n\t\tlda [D + 5]\n\t\trtl\n"
		"R816_Mul32_X:\n"
		IR816_BYTES_INDEX("19", "15")
		IR816_MULTIPLY_BYTES("R816_Mul32_X0")
		"\t\tsta [D + 5]\n"
		"\t\tlda [D + 17]\n\t\tbeq R816_Mul32_XShort\n"
		IR816_BYTES_INDEX("19", "17")
		IR816_MULTIPLY_BYTES("R816_Mul32_X2")
		"\t\tsta [D + 7]\n"
		"\t\tsep m\n\t\tlda [D + 19]\n\t\txba\n\t\tlda [D + 18]\n\t\tbeq R816_Mul32_XNo3\n\t\trep m\n"
		IR816_MULTIPLY_BYTES("R816_Mul32_X3")
		IR816_MULTIPLY_ADD_BYTE_1("[D + 7]")
		"R816_Mul32_XNo3:\n"
		"\t\trep m\n\t\tbra R816_Mul32_XByte1\n"
		"R816_Mul32_XShort:\n"
		"\t\tstz [D + 7]\n"
		"R816_Mul32_XByte1:\n"
		"\t\tsep m\n\t\tlda [D + 19]\n\t\txba\n\t\tlda [D + 16]\n\t\tbeq R816_Mul32_XDone\n\t\trep m\n"
		IR816_MULTIPLY_BYTES("R816_Mul32_X1")
		IR816_MULTIPLY_ADD_BYTES_1
		"R816_Mul32_XDone:\n"
		"\t\trep m\n"
		"\t\tlda [D + 7]\n\t\ttax\n\t\tlda [D + 5]\n\t\trtl\n", true, NULL },
	{ "R816_Mul64", NULL, false, "R816_Mul32 R816_UMulWide32", IR816AppendMul64, false },
	{ "R816_UMulWide32", NULL, true, NULL, IR816AppendUMulWide32, false },
	{ "R816_Shl64", NULL, false, NULL, IR816AppendShl64, true },
	{ "R816_LShr64", NULL, false, NULL, IR816AppendLShr64, true },
	{ "R816_AShr64", NULL, false, NULL, IR816AppendAShr64, true },
	{ "R816_Shl32", NULL, true, NULL, IR816AppendShl32, false },
	{ "R816_LShr32", NULL, true, NULL, IR816AppendLShr32, false },
	{ "R816_AShr32", NULL, true, NULL, IR816AppendAShr32, false },
	{ "R816_BcdAdd", IR816_BCD("R816_BcdAdd", "", "adc", ""), false, NULL },
	{ "R816_BcdSubtract", IR816_BCD("R816_BcdSubtract", "\t\teor 1\n", "sbc", "\t\teor 1\n"), false, NULL },
	{ NULL, NULL, false, NULL }
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
// Adds the runtime routines in szCalls (names separated by spaces) to
// paszNames, each once.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AddCalls(CArrayChars* paszNames, const char* szCalls)
{
	CChars		szName;
	const char*	szEnd;

	while (*szCalls)
	{
		szEnd = strchr(szCalls, ' ');
		if (szEnd == NULL)
		{
			szEnd = szCalls + strlen(szCalls);
		}
		szName.Init(szCalls, 0, (int)(szEnd - szCalls));
		paszNames->AddIfUnique(szName.Text());
		szName.Kill();
		szCalls = (*szEnd == ' ') ? szEnd + 1 : szEnd;
	}
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
	CIR816Backend			cBackend;
	size					i;
	char*					szName;
	const char*				szOperation;
	const SIR816Routine*	psRoutine;
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
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		//Signed divide and remainder call the unsigned ones (R816_S... to R816_U...), and the 32 bit unsigned ones
		//jump to the 16 bit ones (szCalls).
		szName = aszNames.GetText(i);
		psRoutine = IR816FindRoutine(szName);
		if (psRoutine && psRoutine->szCalls)
		{
			IR816AddCalls(&aszNames, psRoutine->szCalls);
		}
		else if (IR816StartsWith(szName, IR816_RUNTIME_PREFIX "SDiv") || IR816StartsWith(szName, IR816_RUNTIME_PREFIX "SRem"))
		{
			szText.Init(IR816_RUNTIME_PREFIX "U");
			szText.Append(&szName[IR816_RUNTIME_PREFIX_LENGTH + 1]);
			aszNames.AddIfUnique(szText.Text());
			if (atoi(&szName[StrLen(szName) - 2]) == 64)
			{
				//gszIR816Signed64's 32 bit one too.
				szText.Replace("64", "32");
				aszNames.AddIfUnique(szText.Text());
			}
			szText.Kill();
		}
	}

	szIR.Init();
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		szName = aszNames.GetText(i);

		//The routines written in assembly, and the divide tables.
		psRoutine = IR816FindRoutine(szName);
		if (psRoutine)
		{
			IR816RuntimeHeader(pszDest, "text.", szName);
			if (psRoutine->szText)
			{
				pszDest->Append(psRoutine->szText);
			}
			else
			{
				psRoutine->pfAppend(pszDest);
			}
			pszDest->AppendNewLine();
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

		//The others from IR: szOperation is Mul, UDiv, URem, SDiv or SRem, then the bits.
		szOperation = &szName[IR816_RUNTIME_PREFIX_LENGTH];
		bDivide = szOperation[1] == 'D';
		iBits = atoi(&szName[StrLen(szName) - 2]);
		szType.Init("i");
		szType.Append(iBits);
		szBits.Init();
		szBits.Append(iBits);
		szTop.Init();
		szTop.Append(iBits - 1);
		if (IR816StartsWith(szOperation, "Mul"))
		{
			szText.Init(gszIR816Mul);
		}
		else if (szOperation[0] == 'U')
		{
			szText.Init(gszIR816Divide);
			szText.Replace("$RESULT", bDivide ? "%q" : "%r");
		}
		else
		{
			szText.Init((iBits == 64) ? gszIR816Signed64 : gszIR816Signed);
			szText.Replace("$UOP", bDivide ? "udiv" : "urem");
			szText.Replace("$SIGN", bDivide ? "ne i1 %an, %bn" : "copy i1 %an");
		}
		szText.Replace("$NAME", szOperation);
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

