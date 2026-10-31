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
#define IR816_ROUND		" ;R"		//Ends a bra round a loop (JumpTo), for PeepholeBranches.
#define IR816_SHIFT_STEPS	7			//The one bit steps a variable shift jumps into, after its byte and word moves.
#define IR816_SHIFT_LOOP_BELOW	5		//Variable shifts by fewer bits loop a bit at a time instead.


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
	const char*	szCalls;	//The runtime routine it jumps to, which must be in the unit too, or NULL.
};


//Every runtime routine's name starts with IR816_RUNTIME_PREFIX, IR816_RUNTIME_PREFIX_LENGTH characters; then the
//operation and the bits (R816_UDiv32), or the name of an assembly routine (R816_BcdAdd).  TableDivide's tables are
//IR816_SIGNED_TABLE or IR816_UNSIGNED_TABLE and the divisor (R816_UTable10), the divisor IR816_TABLE_LENGTH in.
#define IR816_RUNTIME_PREFIX		"R816_"
#define IR816_RUNTIME_PREFIX_LENGTH	5
#define IR816_SIGNED_TABLE			"R816_STable"
#define IR816_UNSIGNED_TABLE		"R816_UTable"
#define IR816_TABLE_LENGTH			11


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
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Init(CIRModule* pcModule, int* piLabels)
{
	CIRBackend::Init(pcModule, piLabels);
	maszRuntime.Init();
	maiOffsets.Init();
	maiIncoming.Init();
	maiSlots.Init();
	maiParameters.Init();
	mauiPendingArguments.Init();
	masFoldBase.Init();
	maiFoldOffset.Init();
	masFoldIndex.Init();
	mcDominators.Init();
	mcLoops.Init();
	mpcDominators = NULL;
	maiUses.Init();
	maiUseBlock.Init();
	mabInX.Init();
	miFused = 0;
	miHiddenResult = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Kill(void)
{
	mauiPendingArguments.Kill();
	maiParameters.Kill();
	maiSlots.Kill();
	maiIncoming.Kill();
	mabInX.Kill();
	maiUseBlock.Kill();
	maiUses.Kill();
	masFoldIndex.Kill();
	maiFoldOffset.Kill();
	masFoldBase.Kill();
	maiOffsets.Kill();
	mcLoops.Kill();
	mcDominators.Kill();
	maszRuntime.Kill();
	CIRBackend::Kill();
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
// Notes that the function calls the runtime routine szName (once each),
// for GetRuntime and GenerateRuntime.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::UseRuntime(const char* szName)
{
	maszRuntime.AddIfUnique((char*)szName);
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
	int32				iRest;
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

	//A frame (with its arguments) past the page keeps S, as it was when aligned (AlignFrame), in two bytes in the page.
	iRest = IR816_LINK_BYTES + (IsHiddenResult(mpcFunction->msReturnType) ? 4 : 0);
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		iRest += ValueBytes(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(ui)));
	}
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			iRest += (ValueBytes(pcBlock->masPhis.Get(ui)->sType) + 1) & ~1;
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->uiOpcode == IRO_Slot)
			{
				iRest += ((int32)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits + 1) & ~1;
			}
		}
	}
	miSavedS = ((iNext + iRest > 256) && (iNext + 2 <= 256)) ? Allocate(&iNext, 2) : -1;
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
	miArgumentBytes = iArgument - (miFrameBytes + 1 + IR816_LINK_BYTES);

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
// (msFoldBase) and constant (miFoldOffset) are kept per temporary.  Then
// pointers plus zero extended 16 bit values are folded too (FoldIndices).
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
	masFoldIndex.FakeSetUsedElements(0);
	aiAddressUses.Init();
	aiUses.Init();
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		masFoldBase.Add(&sNone);
		maiFoldOffset.Add(&iZero);
		masFoldIndex.Add(&sNone);
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
	FoldIndices(&aiUses);
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
// A pointer plus a zero extended i16 x (ptradd p, zext x) that is only used
// as the addresses of 1 word loads and stores isn't worked out: they put x
// in Y and use p ([[D + p]:24 + Y]; the W65C816 adds Y to all 24 bits).
// p must be a temporary that isn't folded itself, and nothing folded may
// add to the ptradd (Y would be x plus a constant, which can wrap).  The
// zext isn't worked out either when the folded ptradds are all its uses:
// it is folded to x (FoldedBase), so x lives to the loads and stores.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FoldIndices(CArrayTemplate<int32>* paiUses)
{
	CArrayTemplate<SIRInstruction*>	apsDefinitions;
	CArrayTemplate<int32>			aiWordUses;		//Per temporary: its uses as a 1 word load's or store's address.
	CArrayTemplate<int32>			aiFoldedUses;	//Per zext: its uses by ptradds folded here.
	CArrayTemplate<uint8>			abBase;			//Per temporary: something folded adds to it.
	SIRInstruction*					psInstruction;
	SIRInstruction*					psExtend;
	CIRBlock*						pcBlock;
	SIRInstruction*					psNull;
	SIRReference					sPointer;
	size							uiTemporaries;
	size							ui;
	size							uj;
	int32							iZero;
	uint8							uiFalse;

	uiTemporaries = mpcFunction->NumTemporaries();
	psNull = NULL;
	iZero = 0;
	uiFalse = 0;
	apsDefinitions.Init();
	aiWordUses.Init();
	aiFoldedUses.Init();
	abBase.Init();
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		apsDefinitions.Add(&psNull);
		aiWordUses.Add(&iZero);
		aiFoldedUses.Add(&iZero);
		abBase.Add(&uiFalse);
	}
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		if (!masFoldBase.Get(ui)->IsNone() && masFoldBase.Get(ui)->IsTemporary())
		{
			*abBase.Get(masFoldBase.Get(ui)->Index()) = true;
		}
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if (psInstruction->sResult.IsTemporary())
			{
				*apsDefinitions.Get(psInstruction->sResult.Index()) = psInstruction;
			}
			if (psInstruction->uiOpcode == IRO_Load)
			{
				sPointer = psInstruction->asArguments[0];
			}
			else if (psInstruction->uiOpcode == IRO_Store)
			{
				sPointer = psInstruction->asArguments[1];
			}
			else
			{
				continue;
			}
			if (sPointer.IsTemporary() && (Words(psInstruction->sType) == 1) && !(psInstruction->uiFlags & IRF_Volatile))
			{
				(*aiWordUses.Get(sPointer.Index()))++;
			}
		}
	}

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if ((psInstruction->uiOpcode != IRO_PtrAdd) || !psInstruction->sResult.IsTemporary() || !psInstruction->asArguments[0].IsTemporary() ||
				!psInstruction->asArguments[1].IsTemporary() || !masFoldBase.Get(psInstruction->sResult.Index())->IsNone() ||
				!masFoldBase.Get(psInstruction->asArguments[0].Index())->IsNone() || *abBase.Get(psInstruction->sResult.Index()) ||
				(*paiUses->Get(psInstruction->sResult.Index()) == 0) || (*paiUses->Get(psInstruction->sResult.Index()) != *aiWordUses.Get(psInstruction->sResult.Index())))
			{
				continue;
			}
			psExtend = *apsDefinitions.Get(psInstruction->asArguments[1].Index());
			if ((psExtend == NULL) || (psExtend->uiOpcode != IRO_ZExt) || !psExtend->asArguments[0].IsTemporary() ||
				!mpcFunction->GetTemporaryType(psExtend->asArguments[0]).Is(IRTK_I16))
			{
				continue;
			}
			*masFoldBase.Get(psInstruction->sResult.Index()) = psInstruction->asArguments[0];
			*maiFoldOffset.Get(psInstruction->sResult.Index()) = 0;
			*masFoldIndex.Get(psInstruction->sResult.Index()) = psExtend->asArguments[0];
			(*aiFoldedUses.Get(psInstruction->asArguments[1].Index()))++;
		}
	}

	//A zext only the folded ptradds use isn't worked out.
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		psExtend = *apsDefinitions.Get(ui);
		if (psExtend && (psExtend->uiOpcode == IRO_ZExt) && (*aiFoldedUses.Get(ui) > 0) && (*aiFoldedUses.Get(ui) == *paiUses->Get(ui)))
		{
			*masFoldBase.Get(ui) = psExtend->asArguments[0];
			*maiFoldOffset.Get(ui) = 0;
		}
	}
	apsDefinitions.Kill();
	aiWordUses.Kill();
	aiFoldedUses.Kill();
	abBase.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The i16 a folded pointer adds in Y (FoldIndices), else none.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIR816Backend::FoldedIndex(SIRReference sValue)
{
	if (sValue.IsTemporary() && (sValue.Index() < masFoldIndex.NumElements()))
	{
		return *masFoldIndex.Get(sValue.Index());
	}
	return IRNone();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction isn't worked out: a ptradd or add folded into its
// loads and stores (FoldPointers), or a zext folded into its ptradds
// (FoldIndices).  It writes no code.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsFolded(SIRInstruction* psInstruction)
{
	return ((psInstruction->uiOpcode == IRO_PtrAdd) || (psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_ZExt)) &&
		psInstruction->sResult.IsTemporary() && !FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult);
}


//////////////////////////////////////////////////////////////////////////
//
// Y for word iOffset / 2 of a load or store through sPointer: the folded
// index (a 1 word access, so iOffset is 0), else the constant.  Word 0 with
// no index needs no Y ([[D + p]:24]): returns whether Y was loaded.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::LoadY(SIRReference sPointer, int32 iOffset)
{
	SIRReference	sIndex;

	sIndex = FoldedIndex(sPointer);
	if (sIndex.IsNone())
	{
		if (iOffset == 0)
		{
			return false;
		}
		Line("ldy ", iOffset);
		return true;
	}
	Operand("ldy", sIndex, 0);
	return true;
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
	CArrayTemplate<int32>	maiPairPosition;	//...where they meet...
	CArrayTemplate<int32>	maiPairWeight;		//...and how many loops the copy sharing a place saves is in.
	CArrayTemplate<int32>	maiWordFirst;		//Per word of the frame: its first occupant (or -1), a list through maiOccupantNext.
	CArrayTemplate<int32>	maiOccupantNext;
	CArrayTemplate<int32>	maiOccupant;		//Per occupant: the temporary placed on the word.

	void	Init(size uiTemporaries);
	void	Kill(void);
	void	Add(int32 iTemporary, int32 iStart, int32 iEnd);
	void	AddPair(SIRReference sOne, SIRReference sOther, int32 iPosition, int32 iWeight);
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
	maiPairWeight.Init();
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
	maiPairWeight.Kill();
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
void CIR816LiveRanges::AddPair(SIRReference sOne, SIRReference sOther, int32 iPosition, int32 iWeight)
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
	maiPairWeight.Add(&iWeight);
	maiPairNext.Add(maiPairFirst.Get(iOne));
	*maiPairFirst.Get(iOne) = iPair;

	iPair = (int32)maiPairOther.NumElements();
	maiPairOther.Add(&iOne);
	maiPairPosition.Add(&iPosition);
	maiPairWeight.Add(&iWeight);
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
	CArrayTemplate<int32>		aiDepths;		//Per block: how many loops it is in.
	int32						iDepth;
	int32						iBestDepth;
	int32						iBestOffset;

	uiTemporaries = mpcFunction->NumTemporaries();
	uiWords = (uiTemporaries + 31) / 32;
	uiBlocks = mpcFunction->NumBlocks();
	iNone = -1;
	cRanges.Init(uiTemporaries);
	asEvents.Init();
	aiBlockEvents.Init();
	aiBlockStart.Init();
	aiBlockEnd.Init();

	//Each block's loop depth, for choosing between pairs.
	aiDepths.Init();
	for (ui = 0; ui < uiBlocks; ui++)
	{
		iDepth = mcLoops.Depth(mpcFunction->GetBlock(ui));
		aiDepths.Add(&iDepth);
	}

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
						cRanges.AddPair(pcBlock->masInstructions.Get(uk)->sResult, FoldedBase(psInstruction->asArguments[0]), iCallPosition, *aiDepths.Get(ui));
						break;
					}
				}
				IR816AddEvent(&asEvents, FoldedBase(psInstruction->asArguments[0]), iCallPosition, IR816_EVENT_USE);
			}
			else
			{
				IR816AddEvent(&asEvents, FoldedBase(psInstruction->asArguments[0]), iPosition, IR816_EVENT_USE);
				IR816AddEvent(&asEvents, FoldedBase(psInstruction->asArguments[1]), iPosition, IR816_EVENT_USE);
				IR816AddEvent(&asEvents, FoldedIndex(psInstruction->asArguments[0]), iPosition, IR816_EVENT_USE);
				IR816AddEvent(&asEvents, FoldedIndex(psInstruction->asArguments[1]), iPosition, IR816_EVENT_USE);
			}
			if (psInstruction->sResult.IsTemporary() && (FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult)))
			{
				IR816AddEvent(&asEvents, psInstruction->sResult, iPosition, IR816_EVENT_DEFINE);
				for (iArgument = 0; iArgument < 2; iArgument++)
				{
					if (IR816CanShareArgument(psInstruction->uiOpcode, iArgument) && FoldedBase(psInstruction->asArguments[iArgument]).Equals(psInstruction->asArguments[iArgument]))
					{
						cRanges.AddPair(psInstruction->sResult, psInstruction->asArguments[iArgument], iPosition, *aiDepths.Get(ui));
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
						//The copy is made going that way: as often as the shallower block of the two.
						iDepth = *aiDepths.Get(ui);
						if (*aiDepths.Get(pcSuccessor->muiIndex) < iDepth)
						{
							iDepth = *aiDepths.Get(pcSuccessor->muiIndex);
						}
						cRanges.AddPair(psPhi->sResult, psArgument->sValue, iPosition, iDepth);
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

		//A pair's place (the one where they meet in the deepest loop: the copy saved is done most), then the first
		//place free.
		bFree = false;
		iBestDepth = -1;
		iBestOffset = -1;
		for (iPair = *cRanges.maiPairFirst.Get(iTemporary); iPair != -1; iPair = *cRanges.maiPairNext.Get(iPair))
		{
			iOffset = *maiOffsets.Get(*cRanges.maiPairOther.Get(iPair));
			if ((iOffset >= IR816_FIRST_TEMPORARY) && cRanges.IsFree(iTemporary, iOffset, iBytes, &maiOffsets))
			{
				iDepth = *cRanges.maiPairWeight.Get(iPair);
				if (iDepth > iBestDepth)
				{
					iBestDepth = iDepth;
					iBestOffset = iOffset;
				}
			}
		}
		if (iBestOffset >= 0)
		{
			iOffset = iBestOffset;
			bFree = true;
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
	aiDepths.Kill();
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

	//The scores and the positions that change X, numbered as PackTemporaries numbers them.
	iPosition = 0;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		iDepth = mcLoops.Depth(pcBlock);
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
			else if (strcmp(szMnemonic, "ldy") == 0)
			{
				Line("txy");
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
			mpszOut->Append(" & 0xFFFF");
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
		Line("and 0xFF");
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
	Line("adc 0xFFFF");
	Line("eor 0xFFFF");
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
// The frame's set up as Prologue writes it (after the label and the
// directives): D saved, S moved down iFrameBytes and D pointed at it.
// DropFrame and AlignFrame find and replace it by this text.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AppendFrameEntry(CChars* psz, int32 iFrameBytes)
{
	psz->Append("\t\tphd\n\t\ttsc\n\t\tsec\n\t\tsbc ");
	psz->Append(iFrameBytes);
	psz->Append("\n\t\ttcs\n\t\ttcd\n");
}


//////////////////////////////////////////////////////////////////////////
//
// The frame's removal as Epilogue writes it (before the rtl): A kept in Y
// while S moves back up iFrameBytes, then D restored.  DropFrame and
// AlignFrame find and replace it by this text.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AppendFrameExit(CChars* psz, int32 iFrameBytes)
{
	psz->Append("\t\ttay\n\t\ttdc\n\t\tclc\n\t\tadc ");
	psz->Append(iFrameBytes);
	psz->Append("\n\t\ttcs\n\t\ttya\n\t\tpld\n");
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
	szPrologue.Init();
	IR816AppendFrameEntry(&szPrologue, miFrameBytes);
	szEpilogue.Init();
	IR816AppendFrameExit(&szEpilogue, miFrameBytes);
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
// Gives the function in pszCode (as GenerateFunction wrote it, psFrame from
// GetFrame) D on a page, where its direct page accesses cost a cycle less,
// when it's worth it: estimated, or measured by pcProfile (IR816Backend.h).
// Never when it may recurse (bRecursive: in a cycle of calls the unit can
// see; or calling itself), nor when variadic (its arguments are walked past
// the named ones).  D (and S) go down to the page below the frame, the
// arguments and the link bytes; the arguments are copied up to the frame
// (from X, S as it was) where they were, and S as it was is kept where the
// caller's D was (at D + frame size + 1) for the epilogue.  A frame past
// the page (its far bytes are reached through X) has its arguments pushed
// into place and S kept in psFrame->iSavedS (in the page).  Returns true if
// it was aligned.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::AlignFrame(CChars* pszCode, const char* szLabel, SIR816Frame* psFrame, bool bRecursive, CAsm65816Profile* pcProfile)
{
	CChars					sz;
	CChars					szPrologue;
	CChars					szEpilogue;
	CChars					szAligned;
	CChars					szAlignedEpilogue;
	CChars					szCall;
	const char*				szCode;
	int32					iTop;
	int32					iWords;
	int32					iSavedS;
	int32					i;
	uint64					ulliCost;
	SAsm65816FunctionProfile*	psProfile;
	bool					bBig;
	bool					bWorth;

	iTop = psFrame->iFrameBytes + IR816_LINK_BYTES + psFrame->iArgumentBytes;
	iWords = (psFrame->iArgumentBytes + 1) / 2;
	szCode = pszCode->Text();
	bBig = iTop > 255;
	if ((psFrame->iFrameBytes == 0) || (bBig && (psFrame->iSavedS < 0)) || psFrame->bVariadic || bRecursive)
	{
		return false;
	}

	//A call of itself is recursion.
	szCall.Init("jsl ");
	szCall.Append(szLabel);
	szCall.AppendNewLine();
	bRecursive = strstr(szCode, szCall.Text()) != NULL;
	szCall.Kill();
	if (bRecursive)
	{
		return false;
	}

	ulliCost = (uint64)(IR816_ALIGN_CYCLES + IR816_ALIGN_WORD_CYCLES * iWords + (bBig ? IR816_ALIGN_BIG_CYCLES : 0));
	if (pcProfile)
	{
		//Measured: never seen recursing, and the accesses well past the cost of its calls.
		psProfile = pcProfile->Get(szLabel);
		bWorth = psProfile && !psProfile->bRecursive && (psProfile->ulliCalls > 0) &&
				 (psProfile->ulliDirectPage >= IR816_ALIGN_GAIN * ulliCost * psProfile->ulliCalls) &&
				 (psProfile->ulliDirectPage - ulliCost * psProfile->ulliCalls >= pcProfile->NumCycles() / IR816_ALIGN_SHARE);
	}
	else
	{
		//Estimated: loops, whose accesses are well past the cost of a call.
		bWorth = psFrame->bLoops && ((uint64)psFrame->iDirectPageWeight >= IR816_ALIGN_GAIN * ulliCost);
	}
	if (!bWorth)
	{
		return false;
	}

	iSavedS = bBig ? psFrame->iSavedS : psFrame->iFrameBytes + 1;
	szPrologue.Init();
	IR816AppendFrameEntry(&szPrologue, psFrame->iFrameBytes);
	szEpilogue.Init();
	IR816AppendFrameExit(&szEpilogue, psFrame->iFrameBytes);
	if (strstr(szCode, szPrologue.Text()) == NULL)
	{
		szPrologue.Kill();
		szEpilogue.Kill();
		return false;
	}

	//S goes down before anything is written below where it was (an interrupt pushes there).
	szAligned.Init("\t\tphd\n\t\ttsc\n\t\ttax\n\t\tsec\n\t\tsbc ");
	szAligned.Append(iTop);
	szAligned.Append("\n\t\tand 0xFF00\n");
	if (!bBig)
	{
		szAligned.Append("\t\ttcs\n\t\ttcd\n");
		for (i = 0; i < iWords; i++)
		{
			szAligned.Append("\t\tlda [");
			szAligned.Append(1 + IR816_LINK_BYTES + 2 * i);
			szAligned.Append(" + X]:24\n\t\tsta [D + ");
			szAligned.Append(psFrame->iFrameBytes + 1 + IR816_LINK_BYTES + 2 * i);
			szAligned.Append("]\n");
		}
	}
	else if (iWords == 0)
	{
		szAligned.Append("\t\ttcs\n\t\ttcd\n");
	}
	else
	{
		//Past the page the arguments are pushed into place, last first, from S at their top; then S is D.
		szAligned.Append("\t\ttcd\n\t\tclc\n\t\tadc ");
		szAligned.Append(iTop);
		szAligned.Append("\n\t\ttcs\n");
		for (i = iWords - 1; i >= 0; i--)
		{
			szAligned.Append("\t\tlda [");
			szAligned.Append(1 + IR816_LINK_BYTES + 2 * i);
			szAligned.Append(" + X]:24\n\t\tpha\n");
		}
		szAligned.Append("\t\ttdc\n\t\ttcs\n");
	}
	szAligned.Append("\t\tstx [D + ");
	szAligned.Append(iSavedS);
	szAligned.Append("]\n");
	szAlignedEpilogue.Init("\t\ttay\n\t\tlda [D + ");
	szAlignedEpilogue.Append(iSavedS);
	szAlignedEpilogue.Append("]\n\t\ttcs\n\t\ttya\n\t\tpld\n");

	sz.Init();
	while (*szCode)
	{
		if (strncmp(szCode, szPrologue.Text(), szPrologue.Length()) == 0)
		{
			sz.Append(szAligned);
			szCode += szPrologue.Length();
		}
		else if (strncmp(szCode, szEpilogue.Text(), szEpilogue.Length()) == 0)
		{
			sz.Append(szAlignedEpilogue);
			szCode += szEpilogue.Length();
		}
		else
		{
			sz.Append(*szCode);
			szCode++;
		}
	}
	pszCode->Kill();
	pszCode->Init(sz);
	sz.Kill();
	szAligned.Kill();
	szAlignedEpilogue.Kill();
	szPrologue.Kill();
	szEpilogue.Kill();
	return true;
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
	muiSource = 0;

	ExpandMultiplies();
	pcFunction->Renumber();

	//The blocks' dominators and loops, for Layout (the loop depths PackTemporaries and AllocateX weigh by) and
	//GenerateBody (the jumps round loops, AlignFrame's estimate).  Nothing after changes the blocks or their jumps.
	pcFunction->ComputePredecessors();
	mcDominators.Compute(pcFunction);
	mcLoops.Compute(pcFunction, &mcDominators);
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
	if (mbSourceMarkers)
	{
		mcSourceMarkers.Put(pszDest, uiStart);
	}
	pszDest->AppendNewLine();
	return mbResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What AlignFrame needs of the function GenerateFunction just wrote.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::GetFrame(SIR816Frame* psFrame)
{
	psFrame->iFrameBytes = miFrameBytes;
	psFrame->iArgumentBytes = miArgumentBytes;
	psFrame->iSavedS = miSavedS;
	psFrame->iDirectPageWeight = miDirectPageWeight;
	psFrame->bLoops = mbLoops;
	psFrame->bVariadic = (mpcFunction->muiAttributes & IRA_Variadic) != 0;
}


//The cycles a multiply by a constant costs as a runtime call (with its operands' stores), and roughly what a constant
//shift (by whole bytes, then bits), an add or sub and a neg cost, for 1, 2 and 4 word values (IR816MultiplyCost).
#define IR816_MULTIPLY_CALL_16	150
#define IR816_MULTIPLY_CALL_32	230
#define IR816_MULTIPLY_CALL_64	3000
#define IR816_MAX_MULTIPLY_DIGITS	8


//////////////////////////////////////////////////////////////////////////
//
// Roughly the cycles of shifts and adds in a value of iWords words: a shift
// of iShift bits, then iAdds adds (or subs).
//
//////////////////////////////////////////////////////////////////////////
static int IR816ShiftAddCost(int iWords, int iShift, int iAdds)
{
	int	iCost;

	iCost = 0;
	if (iShift > 0)
	{
		iCost = 5 * iWords + 12 * iWords * (iShift / 8) + 7 * iWords * (iShift % 8);
	}
	return iCost + 12 * iWords * iAdds;
}


//////////////////////////////////////////////////////////////////////////
//
// A mul by a constant (either side) costs a runtime call: when shifts and
// adds cost less, it is written as them, x times the constant's signed
// digits (its non-adjacent form, top first) Horner's way: acc = x (or -x),
// then for each digit acc = (acc << the gap) + x (or - x), and the last
// acc << the lowest digit's place.  Multiplies by powers of 2, 0 and 1 were
// already made shifts by the optimiser (FoldConstants).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ExpandMultiplies(void)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sValue;
	SIRReference	sAcc;
	SIRType			sType;
	uint64			ulliConstant;
	int				aiPlaces[IR816_MAX_MULTIPLY_DIGITS];
	int				aiSigns[IR816_MAX_MULTIPLY_DIGITS];
	int				iDigits;
	int				iBits;
	int				iWords;
	int				iCost;
	int				iCall;
	int				iPlace;
	int				i;
	size			ui;
	size			uiIndex;
	bool			bTooMany;
	bool			bDone;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			sType = psInstruction->sType;
			if ((psInstruction->uiOpcode != IRO_Mul) || !psInstruction->sResult.IsTemporary() || !sType.IsInteger() || sType.Is(IRTK_I1) ||
				(psInstruction->asArguments[0].IsConstant() == psInstruction->asArguments[1].IsConstant()))
			{
				continue;
			}
			i = psInstruction->asArguments[0].IsConstant() ? 0 : 1;
			ulliConstant = IRTruncate(sType, mpcFunction->GetConstant(psInstruction->asArguments[i])->ulliBits);
			sValue = psInstruction->asArguments[1 - i];
			iBits = sType.IntegerBits();
			iWords = Words(sType);

			//The non-adjacent form: digits of +1 and -1, never two next to each other.  Digits past the top are 0.
			iDigits = 0;
			bTooMany = false;
			for (iPlace = 0; (ulliConstant != 0) && (iPlace < iBits); iPlace++)
			{
				if (ulliConstant & 1)
				{
					if (iDigits == IR816_MAX_MULTIPLY_DIGITS)
					{
						bTooMany = true;
						break;
					}
					aiPlaces[iDigits] = iPlace;
					aiSigns[iDigits] = ((ulliConstant & 3) == 3) ? -1 : 1;
					ulliConstant = (aiSigns[iDigits] < 0) ? ulliConstant + 1 : ulliConstant - 1;
					iDigits++;
				}
				ulliConstant >>= 1;
			}
			if (bTooMany || (iDigits < 2))
			{
				continue;
			}

			//Horner's way: the shifts add up to the top digit's place.
			iCost = IR816ShiftAddCost(iWords, aiPlaces[iDigits - 1], iDigits - 1) + 5 * iWords * (iDigits - 1) + ((aiSigns[iDigits - 1] < 0) ? 12 * iWords : 0);
			iCall = (iWords == 1) ? IR816_MULTIPLY_CALL_16 : ((iWords == 2) ? IR816_MULTIPLY_CALL_32 : IR816_MULTIPLY_CALL_64);
			if (iCost >= iCall)
			{
				continue;
			}

			//The new instructions go before the mul, which becomes the last of them (keeping its result).
			sAcc = sValue;
			if (aiSigns[iDigits - 1] < 0)
			{
				sAcc = InsertBefore(pcBlock, &uiIndex, IRO_Neg, sType, sAcc, IRNone());
			}
			bDone = false;
			for (i = iDigits - 1; i > 0; i--)
			{
				sAcc = InsertBefore(pcBlock, &uiIndex, IRO_Shl, sType, sAcc, mpcFunction->AddInteger(sType, aiPlaces[i] - aiPlaces[i - 1]));
				if ((i == 1) && (aiPlaces[0] == 0))
				{
					//The last digit is at bit 0: its add is the mul's place.
					psInstruction = pcBlock->masInstructions.Get(uiIndex);
					psInstruction->uiOpcode = (aiSigns[0] > 0) ? IRO_Add : IRO_Sub;
					psInstruction->asArguments[0] = sAcc;
					psInstruction->asArguments[1] = sValue;
					bDone = true;
					break;
				}
				sAcc = InsertBefore(pcBlock, &uiIndex, (aiSigns[i - 1] > 0) ? IRO_Add : IRO_Sub, sType, sAcc, sValue);
			}
			if (!bDone)
			{
				//acc << the lowest digit's place is the mul's place.
				psInstruction = pcBlock->masInstructions.Get(uiIndex);
				psInstruction->uiOpcode = IRO_Shl;
				psInstruction->asArguments[0] = sAcc;
				psInstruction->asArguments[1] = mpcFunction->AddInteger(sType, aiPlaces[0]);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A new instruction before pcBlock's instruction *puiIndex, which moves up
// one (*puiIndex follows it).  Returns its result.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIR816Backend::InsertBefore(CIRBlock* pcBlock, size* puiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond)
{
	SIRInstruction	sMoved;
	SIRReference	sResult;
	size			uiLast;

	sResult = mpcFunction->Add(pcBlock, uiOpcode, sType, sFirst, sSecond);
	uiLast = pcBlock->masInstructions.NumElements() - 1;
	sMoved = *pcBlock->masInstructions.Get(uiLast);
	pcBlock->masInstructions.RemoveAt(uiLast, true);
	pcBlock->masInstructions.InsertAt(&sMoved, *puiIndex);
	(*puiIndex)++;
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The direct page operands ([D + n]) in szCode.
//
//////////////////////////////////////////////////////////////////////////
static int32 IR816CountDirectPage(const char* szCode)
{
	int32	iCount;

	iCount = 0;
	for (szCode = strstr(szCode, "D + "); szCode; szCode = strstr(szCode + 4, "D + "))
	{
		iCount++;
	}
	return iCount;
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
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	CIRBlock*		pcNext;
	int32			iLabel;
	size			uiBlockStart;
	int32			iWeight;
	int32			iDepth;

	maiBlockLabels.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		iLabel = NewLabel();
		maiBlockLabels.Add(&iLabel);
	}

	FindSingleUses();
	mpcDominators = &mcDominators;
	miDirectPageWeight = 0;
	mbLoops = false;
	Prologue(szLabel);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		pcNext = (ui + 1 < mpcFunction->NumBlocks()) ? mpcFunction->GetBlock(ui + 1) : NULL;
		miBlock = (int32)ui;
		uiBlockStart = mpszOut->Length();
		Label(*maiBlockLabels.Get(ui));
		PhiCopies(pcBlock);
		mauiPendingArguments.FakeSetUsedElements(0);
		miFused = 0;
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			//Folded instructions write nothing: what is fused goes on past them.
			if (IsFolded(pcBlock->masInstructions.Get(uiInstruction)))
			{
				continue;
			}
			if (CanFuse(pcBlock, uiInstruction))
			{
				mapsFused[miFused] = pcBlock->masInstructions.Get(uiInstruction);
				miFused++;
				continue;
			}
			Source(pcBlock->masInstructions.Get(uiInstruction)->uiSource);
			Instruction(pcBlock, uiInstruction);
			miFused = 0;
		}
		Source(pcBlock->msJump.uiSource);
		Jump(pcBlock, pcNext);
		miFused = 0;

		//The block's direct page accesses, weighted by its loops (AlignFrame's estimate).
		iWeight = 1;
		for (iDepth = mcLoops.Depth(pcBlock); iDepth > 0; iDepth--)
		{
			mbLoops = true;
			iWeight *= (iDepth <= IR816_ALIGN_MAX_DEPTH) ? IR816_ALIGN_LOOP_WEIGHT : 1;
		}
		miDirectPageWeight += iWeight * IR816CountDirectPage(mpszOut->Text() + uiBlockStart);
	}
	miBlock = -1;
	mpcDominators = NULL;
	if (mbSourceMarkers)
	{
		mcSourceMarkers.Take(mpszOut, uiStart);
	}
	Peephole(mpszOut, uiStart);
	PeepholeMasks(mpszOut, uiStart);
	PeepholeStores(mpszOut, uiStart);
	PeepholeTransfers(mpszOut, uiStart);
	PeepholeBranches(mpszOut, uiStart);
	PeepholeRegisters(mpszOut, uiStart);
}


//////////////////////////////////////////////////////////////////////////
//
// The temporaries used once, by an instruction or a jump (not a phi): the
// block of that use (maiUseBlock).  A folded load's or store's index counts
// as a use.
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

			//A folded index (FoldIndices) is read by the load or store too (ldy), not only by its zext.
			if (psInstruction->uiOpcode == IRO_Load)
			{
				IR816UseIn(&maiUseBlock, &aiCount, FoldedIndex(psInstruction->asArguments[0]), (int32)ui);
			}
			else if (psInstruction->uiOpcode == IRO_Store)
			{
				IR816UseIn(&maiUseBlock, &aiCount, FoldedIndex(psInstruction->asArguments[1]), (int32)ui);
			}
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


//The W65C816's mnemonics, as the text passes (Peephole ...) decode them (IR816Decode); gasIR816Mnemonics is in this
//order, alphabetical.
enum EIR816Mnemonic
{
	IR816M_Adc,
	IR816M_And,
	IR816M_Asl,
	IR816M_Bcc,
	IR816M_Bcs,
	IR816M_Beq,
	IR816M_Bit,
	IR816M_Bmi,
	IR816M_Bne,
	IR816M_Bpl,
	IR816M_Bra,
	IR816M_Brk,
	IR816M_Brl,
	IR816M_Bvc,
	IR816M_Bvs,
	IR816M_Clc,
	IR816M_Cld,
	IR816M_Cli,
	IR816M_Clv,
	IR816M_Cmp,
	IR816M_Cop,
	IR816M_Cpx,
	IR816M_Cpy,
	IR816M_Dec,
	IR816M_Dex,
	IR816M_Dey,
	IR816M_Eor,
	IR816M_Inc,
	IR816M_Inx,
	IR816M_Iny,
	IR816M_Jml,
	IR816M_Jmp,
	IR816M_Jsl,
	IR816M_Jsr,
	IR816M_Lda,
	IR816M_Ldx,
	IR816M_Ldy,
	IR816M_Lsr,
	IR816M_Mvn,
	IR816M_Mvp,
	IR816M_Nop,
	IR816M_Ora,
	IR816M_Pea,
	IR816M_Pei,
	IR816M_Per,
	IR816M_Pha,
	IR816M_Phb,
	IR816M_Phd,
	IR816M_Phk,
	IR816M_Php,
	IR816M_Phx,
	IR816M_Phy,
	IR816M_Pla,
	IR816M_Plb,
	IR816M_Pld,
	IR816M_Plp,
	IR816M_Plx,
	IR816M_Ply,
	IR816M_Rep,
	IR816M_Rol,
	IR816M_Ror,
	IR816M_Rti,
	IR816M_Rtl,
	IR816M_Rts,
	IR816M_Sbc,
	IR816M_Sec,
	IR816M_Sed,
	IR816M_Sei,
	IR816M_Sep,
	IR816M_Sta,
	IR816M_Stp,
	IR816M_Stx,
	IR816M_Sty,
	IR816M_Stz,
	IR816M_Tax,
	IR816M_Tay,
	IR816M_Tcd,
	IR816M_Tcs,
	IR816M_Tdc,
	IR816M_Trb,
	IR816M_Tsb,
	IR816M_Tsc,
	IR816M_Tsx,
	IR816M_Txa,
	IR816M_Txs,
	IR816M_Txy,
	IR816M_Tya,
	IR816M_Tyx,
	IR816M_Wai,
	IR816M_Wdm,
	IR816M_Xba,
	IR816M_Xce,
	IR816M_Unknown,		//Anything else (IR816UnknownFlags).
};


//What the text passes know of a mnemonic, a bit each.  The groups are the ones the passes' rules are written in; a
//rule only relies on the instructions it names, so a group holds those its rules were taught (the passes see only
//code the back end writes), not always every instruction with the property.
#define IR816MF_CARRY			0x00000001	//clc, sec: change C only.
#define IR816MF_LOAD_INDEX		0x00000002	//ldx, ldy.
#define IR816MF_STEP_INDEX		0x00000004	//inx, iny, dex, dey.
#define IR816MF_STORE			0x00000008	//sta, stx, sty, stz: write their operand, change no register or flag.
#define IR816MF_LOGIC			0x00000010	//and, ora, eor: a bit of A depends only on that bit of A and the operand.
#define IR816MF_COMPARE			0x00000020	//cmp, cpx, cpy.
#define IR816MF_SHIFT_STEP		0x00000040	//asl, lsr, rol, ror, inc, dec: A or memory, as the operand ("A") says.
#define IR816MF_LOADS_A			0x00000080	//lda, pla, txa, tya, tdc, tsc: set all of A (and N and Z) without reading it.
#define IR816MF_ZN_FROM_A		0x00000100	//N and Z are all of A after it: lda, pla, txa, tya, tdc, and, ora, eor, adc, sbc (not tsc).
#define IR816MF_READS_A			0x00000200	//Reads A (as a value: a shift or step of A is the operand's).
#define IR816MF_A_ALU			0x00000400	//lda, sta, and, ora, eor, adc, sbc, cmp, bit: A and an operand.
#define IR816MF_Y_ALU			0x00000800	//ldy, sty, tay, tya, iny, dey, cpy: Y (and A) and an operand.
#define IR816MF_COPIES_A		0x00001000	//tax, tay, pha: copy A, changing neither it nor C.
#define IR816MF_BRANCH			0x00002000	//bra, brl and the conditional branches: to a label (IR816BranchLine).
#define IR816MF_BRANCH_ZN		0x00004000	//beq, bne, bmi, bpl: read Z or N.
#define IR816MF_BRANCH_C		0x00008000	//bcc, bcs: read C.
#define IR816MF_ENDS_FLOW		0x00010000	//bra, brl, jml, jmp, rtl, rts, stp: the next line isn't run after it (IR816Unreachable).
#define IR816MF_LEAVES			0x00020000	//Every b, j and rt mnemonic (bit and brk too, conservatively): may leave straight line code.
#define IR816MF_WIDTH			0x00040000	//sep, rep.
#define IR816MF_SETS_D			0x00080000	//pld, tcd: the direct page moves.
#define IR816MF_BLOCK_MOVE		0x00100000	//mvn, mvp: A, X, Y and memory.
#define IR816MF_CALL			0x00200000	//jsl, jsr.
#define IR816MF_INTERRUPT		0x00400000	//brk, cop.
#define IR816MF_READS_X			0x00800000	//txa, txy, stx, inx, dex, cpx, phx, txs, mvn, mvp (an X indexed operand is the operand's).
#define IR816MF_WRITES_X		0x01000000	//tax, tyx, ldx, plx: set all of X (tsx too, but only IR816UsesXOrY takes it).
#define IR816MF_USES_Y			0x02000000	//Reads or writes Y; a call (jsl, jsr) may change it.
#define IR816MF_WRITES_Y		0x04000000	//tay, txy, ldy, ply, iny, dey, mvn, mvp.
#define IR816MF_PUSH_REGISTER	0x08000000	//pha, phx, phy.
#define IR816MF_PUSH_WORD		0x10000000	//pea, pei, phk: a constant, a direct page word or K.
#define IR816MF_SETS_CARRY		0x20000000	//clc, sec, cmp, cpx, cpy, asl, lsr, rol, ror, adc, sbc, xce (not plp, sep and rep).
#define IR816MF_READS_CARRY		0x40000000	//adc, sbc, rol, ror, xce, php, bcc, bcs.


//A mnemonic and its IR816MF_ bits.
struct SIR816Mnemonic
{
	const char*	szName;
	uint32		uiFlags;
};


//Per EIR816Mnemonic.
static const SIR816Mnemonic gasIR816Mnemonics[] =
{
	{ "adc", IR816MF_ZN_FROM_A | IR816MF_READS_A | IR816MF_A_ALU | IR816MF_SETS_CARRY | IR816MF_READS_CARRY },
	{ "and", IR816MF_LOGIC | IR816MF_ZN_FROM_A | IR816MF_READS_A | IR816MF_A_ALU },
	{ "asl", IR816MF_SHIFT_STEP | IR816MF_SETS_CARRY },
	{ "bcc", IR816MF_BRANCH | IR816MF_BRANCH_C | IR816MF_LEAVES | IR816MF_READS_CARRY },
	{ "bcs", IR816MF_BRANCH | IR816MF_BRANCH_C | IR816MF_LEAVES | IR816MF_READS_CARRY },
	{ "beq", IR816MF_BRANCH | IR816MF_BRANCH_ZN | IR816MF_LEAVES },
	{ "bit", IR816MF_READS_A | IR816MF_A_ALU | IR816MF_LEAVES },
	{ "bmi", IR816MF_BRANCH | IR816MF_BRANCH_ZN | IR816MF_LEAVES },
	{ "bne", IR816MF_BRANCH | IR816MF_BRANCH_ZN | IR816MF_LEAVES },
	{ "bpl", IR816MF_BRANCH | IR816MF_BRANCH_ZN | IR816MF_LEAVES },
	{ "bra", IR816MF_BRANCH | IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "brk", IR816MF_LEAVES | IR816MF_INTERRUPT },
	{ "brl", IR816MF_BRANCH | IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "bvc", IR816MF_BRANCH | IR816MF_LEAVES },
	{ "bvs", IR816MF_BRANCH | IR816MF_LEAVES },
	{ "clc", IR816MF_CARRY | IR816MF_SETS_CARRY },
	{ "cld", 0 },
	{ "cli", 0 },
	{ "clv", 0 },
	{ "cmp", IR816MF_COMPARE | IR816MF_READS_A | IR816MF_A_ALU | IR816MF_SETS_CARRY },
	{ "cop", IR816MF_INTERRUPT },
	{ "cpx", IR816MF_COMPARE | IR816MF_READS_X | IR816MF_SETS_CARRY },
	{ "cpy", IR816MF_COMPARE | IR816MF_Y_ALU | IR816MF_USES_Y | IR816MF_SETS_CARRY },
	{ "dec", IR816MF_SHIFT_STEP },
	{ "dex", IR816MF_STEP_INDEX | IR816MF_READS_X },
	{ "dey", IR816MF_STEP_INDEX | IR816MF_Y_ALU | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "eor", IR816MF_LOGIC | IR816MF_ZN_FROM_A | IR816MF_READS_A | IR816MF_A_ALU },
	{ "inc", IR816MF_SHIFT_STEP },
	{ "inx", IR816MF_STEP_INDEX | IR816MF_READS_X },
	{ "iny", IR816MF_STEP_INDEX | IR816MF_Y_ALU | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "jml", IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "jmp", IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "jsl", IR816MF_LEAVES | IR816MF_CALL | IR816MF_USES_Y },
	{ "jsr", IR816MF_LEAVES | IR816MF_CALL | IR816MF_USES_Y },
	{ "lda", IR816MF_LOADS_A | IR816MF_ZN_FROM_A | IR816MF_A_ALU },
	{ "ldx", IR816MF_LOAD_INDEX | IR816MF_WRITES_X },
	{ "ldy", IR816MF_LOAD_INDEX | IR816MF_Y_ALU | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "lsr", IR816MF_SHIFT_STEP | IR816MF_SETS_CARRY },
	{ "mvn", IR816MF_READS_A | IR816MF_BLOCK_MOVE | IR816MF_READS_X | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "mvp", IR816MF_READS_A | IR816MF_BLOCK_MOVE | IR816MF_READS_X | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "nop", 0 },
	{ "ora", IR816MF_LOGIC | IR816MF_ZN_FROM_A | IR816MF_READS_A | IR816MF_A_ALU },
	{ "pea", IR816MF_PUSH_WORD },
	{ "pei", IR816MF_PUSH_WORD },
	{ "per", 0 },
	{ "pha", IR816MF_READS_A | IR816MF_COPIES_A | IR816MF_PUSH_REGISTER },
	{ "phb", 0 },
	{ "phd", 0 },
	{ "phk", IR816MF_PUSH_WORD },
	{ "php", IR816MF_READS_CARRY },
	{ "phx", IR816MF_READS_X | IR816MF_PUSH_REGISTER },
	{ "phy", IR816MF_USES_Y | IR816MF_PUSH_REGISTER },
	{ "pla", IR816MF_LOADS_A | IR816MF_ZN_FROM_A },
	{ "plb", 0 },
	{ "pld", IR816MF_SETS_D },
	{ "plp", 0 },
	{ "plx", IR816MF_WRITES_X },
	{ "ply", IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "rep", IR816MF_WIDTH },
	{ "rol", IR816MF_SHIFT_STEP | IR816MF_SETS_CARRY | IR816MF_READS_CARRY },
	{ "ror", IR816MF_SHIFT_STEP | IR816MF_SETS_CARRY | IR816MF_READS_CARRY },
	{ "rti", IR816MF_LEAVES },
	{ "rtl", IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "rts", IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "sbc", IR816MF_ZN_FROM_A | IR816MF_READS_A | IR816MF_A_ALU | IR816MF_SETS_CARRY | IR816MF_READS_CARRY },
	{ "sec", IR816MF_CARRY | IR816MF_SETS_CARRY },
	{ "sed", 0 },
	{ "sei", 0 },
	{ "sep", IR816MF_WIDTH },
	{ "sta", IR816MF_STORE | IR816MF_READS_A | IR816MF_A_ALU },
	{ "stp", IR816MF_ENDS_FLOW },
	{ "stx", IR816MF_STORE | IR816MF_READS_X },
	{ "sty", IR816MF_STORE | IR816MF_Y_ALU | IR816MF_USES_Y },
	{ "stz", IR816MF_STORE },
	{ "tax", IR816MF_READS_A | IR816MF_COPIES_A | IR816MF_WRITES_X },
	{ "tay", IR816MF_READS_A | IR816MF_Y_ALU | IR816MF_COPIES_A | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "tcd", IR816MF_READS_A | IR816MF_SETS_D },
	{ "tcs", IR816MF_READS_A },
	{ "tdc", IR816MF_LOADS_A | IR816MF_ZN_FROM_A },
	{ "trb", 0 },
	{ "tsb", 0 },
	{ "tsc", IR816MF_LOADS_A },
	{ "tsx", 0 },
	{ "txa", IR816MF_LOADS_A | IR816MF_ZN_FROM_A | IR816MF_READS_X },
	{ "txs", IR816MF_READS_X },
	{ "txy", IR816MF_READS_X | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "tya", IR816MF_LOADS_A | IR816MF_ZN_FROM_A | IR816MF_Y_ALU | IR816MF_USES_Y },
	{ "tyx", IR816MF_WRITES_X | IR816MF_USES_Y },
	{ "wai", 0 },
	{ "wdm", 0 },
	{ "xba", IR816MF_READS_A },
	{ "xce", IR816MF_SETS_CARRY | IR816MF_READS_CARRY },
};


//The conditional branches in pairs: each goes when the other doesn't (IR816InvertedBranch).
static const uint16 gauiIR816InvertedBranches[] = { IR816M_Bcc, IR816M_Bcs, IR816M_Beq, IR816M_Bne, IR816M_Bmi, IR816M_Bpl, IR816M_Bvc, IR816M_Bvs };


//An instruction line ("\t\tmnemonic operand"), decoded by IR816Decode.
struct SIR816Line
{
	char		szMnemonic[8];
	uint16		uiMnemonic;		//EIR816Mnemonic.
	uint32		uiFlags;		//Its IR816MF_ bits.
	const char*	szOperand;
	size		uiOperand;		//To the end of the line, less a IR816_KEEP_IN_A mark.
	bool		bMarked;		//The line ends with IR816_KEEP_IN_A (Store).
	bool		bDirect;		//The operand is a direct page word, [D + iOffset] (IR816IsDirect).
	int32		iOffset;
};


//////////////////////////////////////////////////////////////////////////
//
// The IR816MF_ bits of a mnemonic the table doesn't have, as the rules
// took any b, j or rt mnemonic to leave straight line code, and any other
// three letter b one (but brk and bit) to branch to a label.
//
//////////////////////////////////////////////////////////////////////////
static uint32 IR816UnknownFlags(const char* szMnemonic)
{
	uint32	uiFlags;

	uiFlags = 0;
	if ((szMnemonic[0] == 'b') || (szMnemonic[0] == 'j') || ((szMnemonic[0] == 'r') && (szMnemonic[1] == 't')))
	{
		uiFlags |= IR816MF_LEAVES;
	}
	if ((szMnemonic[0] == 'b') && (StrLen(szMnemonic) == 3))
	{
		uiFlags |= IR816MF_BRANCH;
	}
	return uiFlags;
}


//////////////////////////////////////////////////////////////////////////
//
// The EIR816Mnemonic of szMnemonic (a binary search of gasIR816Mnemonics),
// IR816M_Unknown for one it doesn't have.
//
//////////////////////////////////////////////////////////////////////////
static uint16 IR816FindMnemonic(const char* szMnemonic)
{
	int		iLow;
	int		iHigh;
	int		iMiddle;
	int		iCompare;

	iLow = 0;
	iHigh = IR816M_Unknown - 1;
	while (iLow <= iHigh)
	{
		iMiddle = (iLow + iHigh) / 2;
		iCompare = StringCompare(szMnemonic, gasIR816Mnemonics[iMiddle].szName);
		if (iCompare == 0)
		{
			return (uint16)iMiddle;
		}
		if (iCompare < 0)
		{
			iHigh = iMiddle - 1;
		}
		else
		{
			iLow = iMiddle + 1;
		}
	}
	return IR816M_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operand is a direct page word, [D + n] (n decimal): n into
// *piOffset.  [D + X], a pointer's [[D + n]:24 ...] and a symbol aren't.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsDirect(const char* szOperand, size uiOperand, int32* piOffset)
{
	if ((uiOperand >= 30) || (strncmp(szOperand, "[D + ", 5) != 0) || (szOperand[5] < '0') || (szOperand[5] > '9') || (szOperand[uiOperand - 1] != ']'))
	{
		return false;
	}
	*piOffset = atoi(&szOperand[5]);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is an instruction: it starts with two tabs and isn't a
// directive (".width").  Labels start in the first column.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsInstruction(const char* szLine)
{
	return (szLine[0] == '\t') && (szLine[1] == '\t') && (szLine[2] != '.');
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction at szLine (a line starting with two tabs, ending at a new
// line or the end) decoded: its mnemonic, what is known of it and its
// operand (less a IR816_KEEP_IN_A mark).
//
//////////////////////////////////////////////////////////////////////////
static void IR816Decode(const char* szLine, SIR816Line* psLine)
{
	const char*	szEnd;
	size		uiLength;
	size		uiMark;
	int			i;

	szEnd = strchr(szLine, '\n');
	uiLength = szEnd ? (size)(szEnd - szLine) : StrLen(szLine);
	uiMark = StrLen(IR816_KEEP_IN_A);
	psLine->bMarked = (uiLength >= uiMark) && (strncmp(szLine + uiLength - uiMark, IR816_KEEP_IN_A, uiMark) == 0);
	if (psLine->bMarked)
	{
		uiLength -= uiMark;
	}
	for (i = 0; (i < 7) && (2 + (size)i < uiLength) && (szLine[2 + i] != ' '); i++)
	{
		psLine->szMnemonic[i] = szLine[2 + i];
	}
	psLine->szMnemonic[i] = '\0';
	psLine->szOperand = szLine + 2 + i + ((2 + (size)i < uiLength) ? 1 : 0);
	psLine->uiOperand = (size)(szLine + uiLength - psLine->szOperand);
	psLine->uiMnemonic = IR816FindMnemonic(psLine->szMnemonic);
	psLine->uiFlags = (psLine->uiMnemonic != IR816M_Unknown) ? gasIR816Mnemonics[psLine->uiMnemonic].uiFlags : IR816UnknownFlags(psLine->szMnemonic);
	psLine->iOffset = 0;
	psLine->bDirect = IR816IsDirect(psLine->szOperand, psLine->uiOperand, &psLine->iOffset);
}


//////////////////////////////////////////////////////////////////////////
//
// A line of a split function (PeepholeBranches) decoded, when it may be an
// instruction (it starts with a tab and has a mnemonic's room).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816DecodeLine(CChars* psz, SIR816Line* psLine)
{
	if ((psz->Length() < 3) || (psz->Text()[0] != '\t'))
	{
		return false;
	}
	IR816Decode(psz->Text(), psLine);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line of a split function is an instruction with any of the
// IR816MF_ bits in uiFlags.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LineHas(CChars* psz, uint32 uiFlags)
{
	SIR816Line	sLine;

	return IR816DecodeLine(psz, &sLine) && ((sLine.uiFlags & uiFlags) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the operand is the accumulator ("asl A").
//
//////////////////////////////////////////////////////////////////////////
static bool IR816OperandIsA(SIR816Line* psLine)
{
	return (psLine->uiOperand == 1) && (psLine->szOperand[0] == 'A');
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction's operand is exactly szOperand (uiOperand long).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816OperandIs(SIR816Line* psLine, const char* szOperand, size uiOperand)
{
	return (psLine->uiOperand == uiOperand) && (strncmp(psLine->szOperand, szOperand, uiOperand) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The end of the last label line after the line ending at szEnd (labels
// change nothing on the way through), or szEnd when none follows.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816SkipLabels(const char* szEnd)
{
	const char*	szNext;

	while ((*szEnd == '\n') && (szEnd[1] != '\0') && (szEnd[1] != '\t'))
	{
		szNext = strchr(szEnd + 1, '\n');
		if (szNext == NULL)
		{
			break;
		}
		szEnd = szNext;
	}
	return szEnd;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the next instruction after szEnd (the end of a line, past any
// labels) branches on Z or N.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816BranchesOnZN(const char* szEnd)
{
	SIR816Line	sLine;

	szEnd = IR816SkipLabels(szEnd);
	if ((*szEnd != '\n') || !IR816IsInstruction(szEnd + 1))
	{
		return false;
	}
	IR816Decode(szEnd + 1, &sLine);
	return (sLine.uiFlags & IR816MF_BRANCH_ZN) != 0;
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
	SIR816Line	sLine;

	for (szLine = IR816NextInstruction(szLine); szLine; szLine = IR816NextInstruction(szEnd + 1))
	{
		IR816Decode(szLine, &sLine);
		if (sLine.uiFlags & IR816MF_BRANCH_C)
		{
			return true;
		}
		if (!(sLine.uiFlags & IR816MF_BRANCH) || (sLine.uiFlags & IR816MF_ENDS_FLOW) || (sLine.uiMnemonic == IR816M_Unknown))
		{
			//Not a conditional branch.
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

	szBranch = IR816SkipLabels(szEnd) + 1;
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
// Whether the instruction sets N and Z from all of A (as wide as it is).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SetsZNFromA(SIR816Line* psLine)
{
	if (psLine->uiFlags & IR816MF_ZN_FROM_A)
	{
		return true;
	}
	return (psLine->uiFlags & IR816MF_SHIFT_STEP) && IR816OperandIsA(psLine);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction reads neither A nor the N and Z an lda before it
// set, and leaves A as it is: clc, sec, ldx, ldy, inx, iny, dex, dey, stx,
// sty, stz, pea, pei, phk, phx, phy, cpx, cpy, and shifts and steps of
// memory.  An lda followed only by these and then a load of A is dead.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LeavesA(SIR816Line* psLine)
{
	bool	bMemory;

	if (psLine->uiFlags & (IR816MF_CARRY | IR816MF_LOAD_INDEX | IR816MF_STEP_INDEX | IR816MF_PUSH_WORD))
	{
		return true;
	}
	if ((psLine->uiFlags & (IR816MF_STORE | IR816MF_PUSH_REGISTER | IR816MF_COMPARE)) && !(psLine->uiFlags & IR816MF_READS_A))
	{
		return true;
	}
	bMemory = (psLine->uiOperand > 0) && (psLine->szOperand[0] == '[');
	return bMemory && (psLine->uiFlags & IR816MF_SHIFT_STEP);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is a label ("L12:" or a function's).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsLabel(CChars* psz)
{
	return (psz->Length() > 1) && (psz->Text()[0] != '\t') && (psz->Text()[psz->Length() - 1] == ':');
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is a branch (bra or a conditional one) to a label: the
// line decoded and its label (without the colon).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816BranchLine(CChars* psz, SIR816Line* psLine, char* szLabel)
{
	size	ui;

	if (!IR816DecodeLine(psz, psLine))
	{
		return false;
	}
	if (!(psLine->uiFlags & IR816MF_BRANCH) || (psLine->uiOperand < 2) || (psLine->uiOperand > 30) || (psLine->szOperand[0] != 'L'))
	{
		return false;
	}
	for (ui = 0; (ui < psLine->uiOperand) && (psLine->szOperand[ui] != ' '); ui++)
	{
		szLabel[ui] = psLine->szOperand[ui];
	}
	szLabel[ui] = '\0';
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The branch that goes when uiMnemonic's doesn't, or NULL.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816InvertedBranch(uint16 uiMnemonic)
{
	int	i;

	for (i = 0; i < 8; i++)
	{
		if (gauiIR816InvertedBranches[i] == uiMnemonic)
		{
			return gasIR816Mnemonics[gauiIR816InvertedBranches[i ^ 1]].szName;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the lines from uiFrom up to the first instruction are labels
// with szLabel among them.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LabelFollows(CArrayChars* paszLines, size uiFrom, const char* szLabel)
{
	size	ui;
	size	uiLength;
	CChars*	psz;

	uiLength = StrLen(szLabel);
	for (ui = uiFrom; (ui < paszLines->NumElements()) && IR816IsLabel(paszLines->Get(ui)); ui++)
	{
		psz = paszLines->Get(ui);
		if ((psz->Length() == uiLength + 1) && (strncmp(psz->Text(), szLabel, uiLength) == 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the first instruction after label szLabel, else -1.
//
//////////////////////////////////////////////////////////////////////////
static size IR816LabelledInstruction(CArrayChars* paszLines, const char* szLabel)
{
	size	ui;
	size	uiLength;
	CChars*	psz;

	uiLength = StrLen(szLabel);
	for (ui = 0; ui < paszLines->NumElements(); ui++)
	{
		psz = paszLines->Get(ui);
		if (IR816IsLabel(psz) && (psz->Length() == uiLength + 1) && (strncmp(psz->Text(), szLabel, uiLength) == 0))
		{
			for (ui++; (ui < paszLines->NumElements()) && IR816IsLabel(paszLines->Get(ui)); ui++)
			{
			}
			return (ui < paszLines->NumElements()) ? ui : (size)-1;
		}
	}
	return (size)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether c may be part of a label's name.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsNameCharacter(char c)
{
	return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '_');
}


//////////////////////////////////////////////////////////////////////////
//
// A decimal constant operand.  The 0x and 0b constants the backend writes
// for masks and sign flips are not taken as plain constants.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsDecimal(const char* szOperand)
{
	if ((szOperand[0] < '0') || (szOperand[0] > '9'))
	{
		return false;
	}
	return !((szOperand[0] == '0') && ((szOperand[1] == 'x') || (szOperand[1] == 'b')));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a local label ("L12:") is named by an instruction's operand.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LabelUsed(CArrayChars* paszLines, CChars* pszLabel)
{
	size		ui;
	size		uiLength;
	const char*	szAt;
	const char*	szText;

	uiLength = pszLabel->Length() - 1;
	for (ui = 0; ui < paszLines->NumElements(); ui++)
	{
		szText = paszLines->GetText(ui);
		if (szText[0] != '\t')
		{
			continue;
		}
		for (szAt = szText; (szAt = strchr(szAt, 'L')) != NULL; szAt++)
		{
			if ((strncmp(szAt, pszLabel->Text(), uiLength) == 0) && !IR816IsNameCharacter(szAt[-1]) && !IR816IsNameCharacter(szAt[uiLength]))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the instructions after a bra, brl, jml, jmp, rtl, rts or stp
// (IR816MF_ENDS_FLOW) up to a label other than an unused local one
// (IR816LabelUsed), which go too.  True if any did.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816Unreachable(CArrayChars* paszLines)
{
	size	ui;
	CChars*	psz;
	bool	bChanged;

	bChanged = false;
	for (ui = 0; ui + 1 < paszLines->NumElements(); ui++)
	{
		if (!IR816LineHas(paszLines->Get(ui), IR816MF_ENDS_FLOW))
		{
			continue;
		}
		while (ui + 1 < paszLines->NumElements())
		{
			psz = paszLines->Get(ui + 1);
			if (psz->Empty() || ((psz->Text()[0] != '\t') && ((psz->Text()[0] != 'L') || !IR816IsLabel(psz) || IR816LabelUsed(paszLines, psz))))
			{
				break;
			}
			if ((psz->Text()[0] == '\t') && (psz->Text()[2] == '.'))
			{
				break;
			}
			paszLines->Remove(ui + 1);
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what runs from line uiFrom on (past labels, following bras, past
// instructions that leave A and don't read it or N and Z: IR816LeavesA)
// starts with an instruction that sets all of A (IR816MF_LOADS_A) and N and
// Z, so a value in A and its flags aren't read.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SetsAFirst(CArrayChars* paszLines, size uiFrom)
{
	size		ui;
	size		uiTarget;
	int			iSteps;
	SIR816Line	sLine;
	char		szLabel[32];

	ui = uiFrom;
	for (iSteps = 0; (iSteps < 32) && (ui < paszLines->NumElements()); iSteps++)
	{
		if (IR816IsLabel(paszLines->Get(ui)))
		{
			ui++;
			continue;
		}
		if (IR816DecodeLine(paszLines->Get(ui), &sLine) && IR816LeavesA(&sLine))
		{
			ui++;
			continue;
		}
		if (IR816BranchLine(paszLines->Get(ui), &sLine, szLabel) && (sLine.uiMnemonic == IR816M_Bra))
		{
			uiTarget = IR816LabelledInstruction(paszLines, szLabel);
			if (uiTarget == (size)-1)
			{
				return false;
			}
			ui = uiTarget;
			continue;
		}
		return IR816LineHas(paszLines->Get(ui), IR816MF_LOADS_A);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the straight line code from line uiFrom on writes X (tax, ldx,
// plx) before anything that might read it: only instructions that don't
// (on A, Y, the flags or the direct page, not "[D + X]") come between, and
// no label, branch, call or return.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SetsXFirst(CArrayChars* paszLines, size uiFrom)
{
	size		ui;
	CChars*		psz;
	SIR816Line	sLine;

	for (ui = uiFrom; ui < paszLines->NumElements(); ui++)
	{
		psz = paszLines->Get(ui);
		if (!IR816DecodeLine(psz, &sLine) || (strstr(psz->Text(), "X]") != NULL) || (strstr(psz->Text(), ", X") != NULL))
		{
			return false;
		}
		if ((sLine.uiFlags & IR816MF_WRITES_X) && (sLine.uiMnemonic != IR816M_Tyx))
		{
			return true;
		}
		if (!(sLine.uiFlags & (IR816MF_A_ALU | IR816MF_Y_ALU | IR816MF_CARRY | IR816MF_WIDTH | IR816MF_SHIFT_STEP)) && (sLine.uiMnemonic != IR816M_Stz))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a branch line, "\t\tszMnemonic szLabel", over psz.
//
//////////////////////////////////////////////////////////////////////////
static void IR816SetBranch(CChars* psz, const char* szMnemonic, const char* szLabel)
{
	psz->Clear();
	psz->Append("\t\t");
	psz->Append(szMnemonic);
	psz->Append(" ");
	psz->Append(szLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// Branches tidied in the code written from uiStart on, until none changes:
// a bra to the label that follows goes; a branch to a label whose first
// instruction is a bra goes where that does; Bcc X, bra Y, X: is B!cc Y,
// X: (and bcc X, beq X, bra Y, X: is beq X, bcs Y, X:); Bcc X, bra Y
// round a loop (marked IR816_ROUND by JumpTo) is B!cc Y, bra X; an lda of a direct
// page word or a constant goes when what runs next (falling through or
// after a bra) sets A and N and Z before reading A or the flags; a tax goes
// when X is written before it is read (IR816SetsXFirst); txa after tax (or tax after txa),
// only clc or sec between, goes; a txy goes after a txy and an instruction that changes neither X
// nor Y; and code after a bra, rtl or stp up to a label that is used goes
// (IR816Unreachable).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeBranches(CChars* pszDest, size uiStart)
{
	CArrayChars		aszLines;
	CChars			szOut;
	CChars*			psz;
	SIR816Line		sLine;
	SIR816Line		sLine2;
	SIR816Line		sLine3;
	char			szLabel[32];
	char			szLabel2[32];
	char			szLabel3[32];
	size			ui;
	size			uiTarget;
	int				iPass;
	bool			bChanged;

	aszLines.Init();
	aszLines.Split(pszDest->Text() + uiStart, '\n');
	if ((aszLines.NumElements() > 0) && aszLines.Tail()->Empty())
	{
		aszLines.RemoveTail();
	}
	for (iPass = 0; iPass < 8; iPass++)
	{
		bChanged = false;
		for (ui = 0; ui < aszLines.NumElements(); ui++)
		{
			psz = aszLines.Get(ui);
			if (IR816BranchLine(psz, &sLine, szLabel))
			{
				//To a bra: straight to where it goes.
				uiTarget = IR816LabelledInstruction(&aszLines, szLabel);
				if ((uiTarget != (size)-1) && IR816BranchLine(aszLines.Get(uiTarget), &sLine2, szLabel2) && (sLine2.uiMnemonic == IR816M_Bra) &&
					(StringCompare(szLabel2, szLabel) != 0))
				{
					IR816SetBranch(psz, sLine.szMnemonic, szLabel2);
					bChanged = true;
					continue;
				}

				//A bra to the next label.
				if ((sLine.uiMnemonic == IR816M_Bra) && IR816LabelFollows(&aszLines, ui + 1, szLabel))
				{
					aszLines.Remove(ui);
					ui--;
					bChanged = true;
					continue;
				}

				//Bcc X, bra Y, X:
				if ((ui + 1 < aszLines.NumElements()) && IR816InvertedBranch(sLine.uiMnemonic) && IR816BranchLine(aszLines.Get(ui + 1), &sLine2, szLabel2) &&
					(sLine2.uiMnemonic == IR816M_Bra) && IR816LabelFollows(&aszLines, ui + 2, szLabel))
				{
					IR816SetBranch(psz, IR816InvertedBranch(sLine.uiMnemonic), szLabel2);
					aszLines.Remove(ui + 1);
					bChanged = true;
					continue;
				}

				//Bcc X, bra Y round a loop (JumpTo marked it) is B!cc Y, bra X: going round, the usual way, is one branch taken.
				if ((ui + 1 < aszLines.NumElements()) && IR816InvertedBranch(sLine.uiMnemonic) && IR816BranchLine(aszLines.Get(ui + 1), &sLine2, szLabel2) &&
					(sLine2.uiMnemonic == IR816M_Bra) && aszLines.Get(ui + 1)->EndsWith(IR816_ROUND) && (StringCompare(szLabel, szLabel2) != 0))
				{
					IR816SetBranch(psz, IR816InvertedBranch(sLine.uiMnemonic), szLabel2);
					IR816SetBranch(aszLines.Get(ui + 1), "bra", szLabel);
					bChanged = true;
					continue;
				}

				//bcc X, beq X, bra Y, X: (either order of the first two) is beq X, bcs Y, X:.
				if ((ui + 2 < aszLines.NumElements()) && IR816BranchLine(aszLines.Get(ui + 1), &sLine2, szLabel2) &&
					IR816BranchLine(aszLines.Get(ui + 2), &sLine3, szLabel3) && (sLine3.uiMnemonic == IR816M_Bra) && (StringCompare(szLabel, szLabel2) == 0) &&
					(((sLine.uiMnemonic == IR816M_Bcc) && (sLine2.uiMnemonic == IR816M_Beq)) || ((sLine.uiMnemonic == IR816M_Beq) && (sLine2.uiMnemonic == IR816M_Bcc))) &&
					IR816LabelFollows(&aszLines, ui + 3, szLabel))
				{
					IR816SetBranch(psz, "beq", szLabel);
					IR816SetBranch(aszLines.Get(ui + 1), "bcs", szLabel3);
					aszLines.Remove(ui + 2);
					bChanged = true;
					continue;
				}
				continue;
			}

			//An lda (of a direct page word or a constant) for nothing: what runs next sets A before reading A or N and Z.
			if ((psz->Length() > 3) && IR816DecodeLine(psz, &sLine))
			{
				if ((sLine.uiMnemonic == IR816M_Lda) && ((strncmp(sLine.szOperand, "[D + ", 5) == 0) || IR816IsDecimal(sLine.szOperand)) &&
					IR816SetsAFirst(&aszLines, ui + 1))
				{
					aszLines.Remove(ui);
					ui--;
					bChanged = true;
					continue;
				}
			}

			//A tax for nothing: X is written again before anything reads it.
			if (psz->Equals("\t\ttax") && IR816SetsXFirst(&aszLines, ui + 1))
			{
				aszLines.Remove(ui);
				ui--;
				bChanged = true;
				continue;
			}

			//tax then txa, or txa then tax, with only clc or sec between: A and X hold the same already (and N and Z
			//are as the first set them).
			if (psz->Equals("\t\ttax") || psz->Equals("\t\ttxa"))
			{
				for (uiTarget = ui + 1; (uiTarget < aszLines.NumElements()) && (aszLines.Get(uiTarget)->Equals("\t\tclc") || aszLines.Get(uiTarget)->Equals("\t\tsec")); uiTarget++)
				{
				}
				if ((uiTarget < aszLines.NumElements()) && aszLines.Get(uiTarget)->Equals((psz->Text()[3] == 'a') ? "\t\ttxa" : "\t\ttax"))
				{
					aszLines.Remove(uiTarget);
					bChanged = true;
					continue;
				}
			}

			//txy, an instruction that keeps X and Y (lda, sta, and, ora, eor, adc, sbc, cmp, sep, rep, clc or sec), txy.
			if ((ui + 2 < aszLines.NumElements()) && psz->Equals("\t\ttxy") && IR816DecodeLine(aszLines.Get(ui + 1), &sLine) &&
				(((sLine.uiFlags & IR816MF_A_ALU) && (sLine.uiMnemonic != IR816M_Bit)) || (sLine.uiFlags & (IR816MF_WIDTH | IR816MF_CARRY))) &&
				aszLines.Get(ui + 2)->Equals("\t\ttxy"))
			{
				aszLines.Remove(ui + 2);
				bChanged = true;
				continue;
			}
		}
		bChanged |= IR816Unreachable(&aszLines);
		if (!bChanged)
		{
			break;
		}
	}

	szOut.Init();
	for (ui = 0; ui < aszLines.NumElements(); ui++)
	{
		psz = aszLines.Get(ui);
		if (psz->EndsWith(IR816_ROUND))
		{
			psz->RemoveFromEnd(StrLen(IR816_ROUND));
		}
		szOut.Append(psz);
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut);
	szOut.Kill();
	aszLines.Kill();
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
// The end of the line at szLine: its new line, or the end of the text.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816LineEnd(const char* szLine)
{
	const char*	szEnd;

	szEnd = strchr(szLine, '\n');
	return szEnd ? szEnd : szLine + StrLen(szLine);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether N or Z may be read after the line at szLine before an
// instruction sets them again (stores, register pushes, clc and sec don't;
// a label or the end might lead to a read).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ZNReadAfter(const char* szLine)
{
	SIR816Line	sLine;

	for (szLine = IR816NextLine(szLine); szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return true;
		}
		IR816Decode(szLine, &sLine);
		if (sLine.uiFlags & IR816MF_BRANCH_ZN)
		{
			return true;
		}
		if (!(sLine.uiFlags & (IR816MF_STORE | IR816MF_PUSH_REGISTER | IR816MF_CARRY)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the carry may be read after the line at szLine before an
// instruction sets it again without reading it (a label, a directive, a
// branch, a jump, a return, a width change or the end might lead to a
// read).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CarryReadLater(const char* szLine)
{
	SIR816Line	sLine;

	for (szLine = IR816NextLine(szLine); szLine && *szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return true;
		}
		IR816Decode(szLine, &sLine);
		if ((sLine.uiFlags & IR816MF_SETS_CARRY) && !(sLine.uiFlags & IR816MF_READS_CARRY))
		{
			return false;
		}
		if (sLine.uiFlags & (IR816MF_LEAVES | IR816MF_READS_CARRY | IR816MF_WIDTH))
		{
			return true;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the "and 0xFF" before szLine is done for nothing: A's low byte is
// all that is used before the next "and 0xFF" masks it again, as only clc,
// sec, adc, sbc, ora, eor, and and asl A come between (their low bytes
// don't depend on the high bytes), and the carry those leave isn't read
// after it (IR816CarryReadLater).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816MaskRedone(const char* szLine)
{
	SIR816Line	sLine;
	bool		bCarry;

	bCarry = false;
	for (; szLine && *szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
		if ((sLine.uiMnemonic == IR816M_And) && IR816OperandIs(&sLine, "0xFF", 4))
		{
			return !bCarry || !IR816CarryReadLater(szLine);
		}
		if ((sLine.uiMnemonic == IR816M_Adc) || (sLine.uiMnemonic == IR816M_Sbc) || ((sLine.uiMnemonic == IR816M_Asl) && IR816OperandIsA(&sLine)))
		{
			bCarry = true;
		}
		else if (!(sLine.uiFlags & (IR816MF_CARRY | IR816MF_LOGIC)))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the line at szLine is "uiMnemonic szOperand" (szOperand as long
// as uiOperand).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LineIs(const char* szLine, uint16 uiMnemonic, const char* szOperand, size uiOperand)
{
	SIR816Line	sLine;

	if (!szLine || !IR816IsInstruction(szLine))
	{
		return false;
	}
	IR816Decode(szLine, &sLine);
	return (sLine.uiMnemonic == uiMnemonic) && IR816OperandIs(&sLine, szOperand, uiOperand);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the operand is indexed by c ('X' or 'Y': "+ X" or ", X").
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IndexedBy(const char* szOperand, size uiOperand, char c)
{
	size	ui;

	for (ui = 0; ui + 2 < uiOperand; ui++)
	{
		if (((szOperand[ui] == '+') || (szOperand[ui] == ',')) && (szOperand[ui + 1] == ' ') && (szOperand[ui + 2] == c))
		{
			return true;
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
// or width change) the next instruction that names a byte near it, other
// than a store to another direct page word, stores a word to it.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816DeadStore(const char* szLine, const char* szWord, size uiWord)
{
	SIR816Line	sLine;
	int32		iOffset;
	bool		bStore;

	iOffset = atoi(szWord + 5);
	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
		if (sLine.uiFlags & (IR816MF_LEAVES | IR816MF_WIDTH | IR816MF_BLOCK_MOVE | IR816MF_SETS_D | IR816MF_INTERRUPT))
		{
			return false;
		}
		if (IR816MentionsNear(sLine.szOperand, sLine.uiOperand, iOffset))
		{
			bStore = (sLine.uiFlags & IR816MF_STORE) != 0;
			if (bStore && IR816OperandIs(&sLine, szWord, uiWord))
			{
				return true;
			}
			if (bStore && (strncmp(sLine.szOperand, "[D + ", 5) == 0) && (sLine.szOperand[sLine.uiOperand - 1] == ']') && !memchr(sLine.szOperand, ':', sLine.uiOperand))
			{
				//A store to another direct page word reads nothing.
				continue;
			}
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The line "ldy w" after szLine (not the next line), when the lines from
// szLine to it are straight line code that neither uses Y nor names a byte
// near w; else NULL.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816LoadYLater(const char* szLine, const char* szWord, size uiWord)
{
	SIR816Line	sLine;
	const char*	szFirst;

	szFirst = szLine;
	for (; szLine && *szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine) || !strchr(szLine, '\n'))
		{
			return NULL;
		}
		IR816Decode(szLine, &sLine);
		if ((sLine.uiMnemonic == IR816M_Ldy) && IR816OperandIs(&sLine, szWord, uiWord))
		{
			return (szLine == szFirst) ? NULL : szLine;
		}
		if ((sLine.uiFlags & (IR816MF_LEAVES | IR816MF_INTERRUPT | IR816MF_SETS_D | IR816MF_USES_Y)) || IR816IndexedBy(sLine.szOperand, sLine.uiOperand, 'Y') ||
			IR816MentionsNear(sLine.szOperand, sLine.uiOperand, atoi(szWord + 5)))
		{
			return NULL;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Stores of A to a direct page word w that the next lines read back,
// after Peephole:
//   sta w / dec w / lda w  ->  dec A / sta w  (inc the same)
//   sta w / ldy w          ->  sta w / tay    (ldx: tax)
// and the store is left out too when w is written again before it is read
// (IR816DeadStore).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeStores(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	const char*	szNext;
	const char*	szAfter;
	SIR816Line	sLine;
	bool		bDirect;

	szOut.Init();
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		bDirect = false;
		if (IR816IsInstruction(szLine))
		{
			IR816Decode(szLine, &sLine);
			bDirect = (sLine.uiMnemonic == IR816M_Sta) && sLine.bDirect && (*szEnd == '\n');
		}
		szNext = bDirect ? IR816NextLine(szLine) : NULL;
		if (szNext && (IR816LineIs(szNext, IR816M_Dec, sLine.szOperand, sLine.uiOperand) || IR816LineIs(szNext, IR816M_Inc, sLine.szOperand, sLine.uiOperand)) &&
			IR816LineIs(IR816NextLine(szNext), IR816M_Lda, sLine.szOperand, sLine.uiOperand))
		{
			szAfter = IR816NextLine(IR816NextLine(szNext));
			szOut.Append("\t\t")->Append((szNext[2] == 'd') ? "dec" : "inc")->Append(" A\n");
			if (!szAfter || !IR816DeadStore(szAfter, sLine.szOperand, sLine.uiOperand))
			{
				szOut.Append(szLine, (size)(szEnd - szLine));
				szOut.AppendNewLine();
			}
			szEnd = strchr(IR816NextLine(szNext), '\n');
			if (!szEnd)
			{
				szEnd = szLine + StrLen(szLine);
			}
			continue;
		}
		szAfter = szNext ? IR816LoadYLater(szNext, sLine.szOperand, sLine.uiOperand) : NULL;
		if (szAfter && !IR816ZNReadAfter(szLine) && !IR816ZNReadAfter(szAfter))
		{
			//sta w ... ldy w (Y and w untouched between): tay at once.
			if (!IR816NextLine(szAfter) || !IR816DeadStore(IR816NextLine(szAfter), sLine.szOperand, sLine.uiOperand))
			{
				szOut.Append(szLine, (size)(szEnd - szLine));
				szOut.AppendNewLine();
			}
			szOut.Append("\t\ttay\n");
			for (szNext = IR816NextLine(szLine); szNext != szAfter; szNext = IR816NextLine(szNext))
			{
				szOut.Append(szNext, (size)(strchr(szNext, '\n') - szNext));
				szOut.AppendNewLine();
			}
			szEnd = IR816LineEnd(szAfter);
			continue;
		}
		if (szNext && (IR816LineIs(szNext, IR816M_Ldy, sLine.szOperand, sLine.uiOperand) || IR816LineIs(szNext, IR816M_Ldx, sLine.szOperand, sLine.uiOperand)))
		{
			szAfter = IR816NextLine(szNext);
			if (!szAfter || !IR816DeadStore(szAfter, sLine.szOperand, sLine.uiOperand))
			{
				szOut.Append(szLine, (size)(szEnd - szLine));
				szOut.AppendNewLine();
			}
			szOut.Append((szNext[4] == 'y') ? "\t\ttay\n" : "\t\ttax\n");
			szEnd = IR816LineEnd(szNext);
			continue;
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut);
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction reads X: a transfer, store, step, compare or push
// of it (IR816MF_READS_X), or an X indexed operand.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ReadsX(SIR816Line* psLine)
{
	return (psLine->uiFlags & IR816MF_READS_X) || IR816IndexedBy(psLine->szOperand, psLine->uiOperand, 'X');
}


//////////////////////////////////////////////////////////////////////////
//
// Whether X is written before it is read after the line at szLine (tax,
// ldx, plx or tyx first, in straight line code).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816XDeadAfter(const char* szLine)
{
	SIR816Line	sLine;

	for (szLine = IR816NextLine(szLine); szLine && *szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
		if (IR816ReadsX(&sLine))
		{
			return false;
		}
		if (sLine.uiFlags & IR816MF_WRITES_X)
		{
			return true;
		}
		if (sLine.uiFlags & (IR816MF_LEAVES | IR816MF_INTERRUPT))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// tax ... txy is tay ... when the instructions between don't use X or Y
// (and don't leave the straight line code), X is written again before it
// is read after the txy (IR816XDeadAfter) and nothing reads the N and Z
// the txy set (IR816ZNReadAfter).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeTransfers(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	const char*	szScan;
	const char*	szTransfer;
	SIR816Line	sLine;

	szOut.Init();
	szTransfer = NULL;
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		if (szLine == szTransfer)
		{
			//The txy that the tay before made.
			szTransfer = NULL;
			continue;
		}
		if (!szTransfer && ((size)(szEnd - szLine) == 5) && (strncmp(szLine, "\t\ttax", 5) == 0) && (*szEnd == '\n'))
		{
			for (szScan = IR816NextLine(szLine); szScan && *szScan; szScan = IR816NextLine(szScan))
			{
				if (!IR816IsInstruction(szScan))
				{
					break;
				}
				IR816Decode(szScan, &sLine);
				if (sLine.uiMnemonic == IR816M_Txy)
				{
					if (IR816XDeadAfter(szScan) && !IR816ZNReadAfter(szScan))
					{
						szTransfer = szScan;
					}
					break;
				}
				if ((sLine.uiFlags & (IR816MF_LEAVES | IR816MF_READS_X | IR816MF_WRITES_X | IR816MF_USES_Y)) || (sLine.uiMnemonic == IR816M_Tsx) ||
					IR816IndexedBy(sLine.szOperand, sLine.uiOperand, 'X') || IR816IndexedBy(sLine.szOperand, sLine.uiOperand, 'Y'))
				{
					break;
				}
			}
			if (szTransfer)
			{
				szOut.Append("\t\ttay\n");
				continue;
			}
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut);
	szOut.Kill();
}


//What a register holds (PeepholeRegisters): up to IR816_HELD direct page words ([D + n], aiOffset n) or constants
//(aiOffset -1), each "" when not known.  A register stored to a word holds both the word and what it held.
#define IR816_HELD			2
#define IR816_HELD_LENGTH	32

struct SIR816Held
{
	char	aszOperand[IR816_HELD][IR816_HELD_LENGTH];
	int32	aiOffset[IR816_HELD];
};


//////////////////////////////////////////////////////////////////////////
//
// Nothing known of what registers iFirst to iLast (0 A, 1 X, 2 Y) hold.
//
//////////////////////////////////////////////////////////////////////////
static void IR816ForgetHeld(SIR816Held* asHeld, int iFirst, int iLast)
{
	int	i;
	int	j;

	for (i = iFirst; i <= iLast; i++)
	{
		for (j = 0; j < IR816_HELD; j++)
		{
			asHeld[i].aszOperand[j][0] = '\0';
			asHeld[i].aiOffset[j] = -1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What registers hold of a direct page word a 16 bit write at [D + iOffset]
// may change (any word overlapping it), or, for an iOffset of -1 (a write
// through a pointer or an index, which may reach the frame), of any word.
//
//////////////////////////////////////////////////////////////////////////
static void IR816ForgetWritten(SIR816Held* asHeld, int32 iOffset)
{
	int	i;
	int	j;

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < IR816_HELD; j++)
		{
			if ((asHeld[i].aiOffset[j] >= 0) && ((iOffset < 0) || ((asHeld[i].aiOffset[j] >= iOffset - 1) && (asHeld[i].aiOffset[j] <= iOffset + 1))))
			{
				asHeld[i].aszOperand[j][0] = '\0';
				asHeld[i].aiOffset[j] = -1;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction's operand is one a register's contents are known
// as: a direct page word or a constant.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsHeldOperand(SIR816Line* psLine)
{
	size	ui;

	if (psLine->bDirect)
	{
		return true;
	}
	if ((psLine->uiOperand == 0) || (psLine->uiOperand >= IR816_HELD_LENGTH) || (psLine->szOperand[0] < '0') || (psLine->szOperand[0] > '9'))
	{
		return false;
	}
	for (ui = 0; ui < psLine->uiOperand; ui++)
	{
		if (psLine->szOperand[ui] == ' ')
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether register iRegister holds the instruction's operand.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816Holds(SIR816Held* asHeld, int iRegister, SIR816Line* psLine)
{
	int	j;

	for (j = 0; j < IR816_HELD; j++)
	{
		if ((asHeld[iRegister].aszOperand[j][0] != '\0') && (StrLen(asHeld[iRegister].aszOperand[j]) == psLine->uiOperand) &&
			(strncmp(asHeld[iRegister].aszOperand[j], psLine->szOperand, psLine->uiOperand) == 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Register iRegister holds the instruction's operand: only it after a load
// (bAlso false), as well as what it held after a store (bAlso).  Nothing
// is known after a load of anything but a direct page word or a constant.
//
//////////////////////////////////////////////////////////////////////////
static void IR816Hold(SIR816Held* asHeld, int iRegister, SIR816Line* psLine, bool bAlso)
{
	SIR816Held*	psHeld;

	psHeld = &asHeld[iRegister];
	if (!bAlso)
	{
		IR816ForgetHeld(asHeld, iRegister, iRegister);
	}
	if (!IR816IsHeldOperand(psLine) || IR816Holds(asHeld, iRegister, psLine))
	{
		return;
	}
	//The operand first, what it held second (the oldest is forgotten).
	memcpy(psHeld->aszOperand[1], psHeld->aszOperand[0], IR816_HELD_LENGTH);
	psHeld->aiOffset[1] = psHeld->aiOffset[0];
	memcpy(psHeld->aszOperand[0], psLine->szOperand, psLine->uiOperand);
	psHeld->aszOperand[0][psLine->uiOperand] = '\0';
	psHeld->aiOffset[0] = psLine->bDirect ? psLine->iOffset : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// What A, X and Y hold after the instruction (PeepholeRegisters).
//
//////////////////////////////////////////////////////////////////////////
static void IR816UpdateHeld(SIR816Held* asHeld, SIR816Line* psLine)
{
	bool	bOfA;

	switch (psLine->uiMnemonic)
	{
	case IR816M_Lda:	IR816Hold(asHeld, 0, psLine, false);	return;
	case IR816M_Ldx:	IR816Hold(asHeld, 1, psLine, false);	return;
	case IR816M_Ldy:	IR816Hold(asHeld, 2, psLine, false);	return;
	case IR816M_Sta:
	case IR816M_Stx:
	case IR816M_Sty:
	case IR816M_Stz:
		IR816ForgetWritten(asHeld, psLine->bDirect ? psLine->iOffset : -1);
		if (psLine->bDirect && (psLine->uiMnemonic != IR816M_Stz))
		{
			IR816Hold(asHeld, (psLine->uiMnemonic == IR816M_Sta) ? 0 : (psLine->uiMnemonic == IR816M_Stx) ? 1 : 2, psLine, true);
		}
		return;
	case IR816M_Tax:	asHeld[1] = asHeld[0];	return;
	case IR816M_Tay:	asHeld[2] = asHeld[0];	return;
	case IR816M_Txa:	asHeld[0] = asHeld[1];	return;
	case IR816M_Tya:	asHeld[0] = asHeld[2];	return;
	case IR816M_Txy:	asHeld[2] = asHeld[1];	return;
	case IR816M_Tyx:	asHeld[1] = asHeld[2];	return;
	case IR816M_Inc:
	case IR816M_Dec:
	case IR816M_Asl:
	case IR816M_Lsr:
	case IR816M_Rol:
	case IR816M_Ror:
	case IR816M_Tsb:
	case IR816M_Trb:
		bOfA = (psLine->uiOperand == 0) || ((psLine->uiOperand == 1) && (psLine->szOperand[0] == 'A'));
		if (bOfA)
		{
			IR816ForgetHeld(asHeld, 0, 0);
		}
		else
		{
			IR816ForgetWritten(asHeld, psLine->bDirect ? psLine->iOffset : -1);
		}
		return;
	case IR816M_Inx:
	case IR816M_Dex:
	case IR816M_Plx:
	case IR816M_Tsx:
		IR816ForgetHeld(asHeld, 1, 1);
		return;
	case IR816M_Iny:
	case IR816M_Dey:
	case IR816M_Ply:
		IR816ForgetHeld(asHeld, 2, 2);
		return;
	case IR816M_Bcc:
	case IR816M_Bcs:
	case IR816M_Beq:
	case IR816M_Bmi:
	case IR816M_Bne:
	case IR816M_Bpl:
	case IR816M_Bvc:
	case IR816M_Bvs:
	case IR816M_Cmp:
	case IR816M_Cpx:
	case IR816M_Cpy:
	case IR816M_Bit:
	case IR816M_Clc:
	case IR816M_Sec:
	case IR816M_Clv:
	case IR816M_Nop:
	case IR816M_Pha:
	case IR816M_Phx:
	case IR816M_Phy:
	case IR816M_Php:
	case IR816M_Phb:
	case IR816M_Phd:
	case IR816M_Phk:
	case IR816M_Pea:
	case IR816M_Pei:
	case IR816M_Per:
		return;
	case IR816M_Adc:
	case IR816M_Sbc:
	case IR816M_And:
	case IR816M_Ora:
	case IR816M_Eor:
	case IR816M_Pla:
	case IR816M_Xba:
	case IR816M_Tsc:
	case IR816M_Tdc:
		IR816ForgetHeld(asHeld, 0, 0);
		return;
	default:
		//Calls, jumps, returns, width and mode changes, moves and D or the stack changing: nothing is known after.
		IR816ForgetHeld(asHeld, 0, 2);
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A load of what the register already holds (a direct page word or a
// constant, from a load, store or transfer earlier in the same straight
// line code) taken out of the code from uiStart, when nothing reads the N
// and Z it sets (IR816ZNReadAfter).  What a register holds is forgotten
// when the register may change, when the word is written (a write through
// a pointer or an index may write any of the frame's words), and at labels,
// directives, calls, jumps and width changes.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeRegisters(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	SIR816Line	sLine;
	SIR816Held	asHeld[3];
	int			iRegister;

	szOut.Init();
	IR816ForgetHeld(asHeld, 0, 2);
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		if (!IR816IsInstruction(szLine))
		{
			IR816ForgetHeld(asHeld, 0, 2);
		}
		else if (szLine[2] != ';')
		{
			IR816Decode(szLine, &sLine);
			iRegister = (sLine.uiMnemonic == IR816M_Lda) ? 0 : (sLine.uiMnemonic == IR816M_Ldx) ? 1 : (sLine.uiMnemonic == IR816M_Ldy) ? 2 : -1;
			if ((iRegister >= 0) && IR816Holds(asHeld, iRegister, &sLine) && !IR816ZNReadAfter(szLine))
			{
				continue;
			}
			IR816UpdateHeld(asHeld, &sLine);
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut);
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The "and 0xFF"s done for nothing (IR816MaskRedone) taken out of the code
// from uiStart, after Peephole has kept values in A (so only the masking
// is left between two masks).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeMasks(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;

	szOut.Init();
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		if (((size)(szEnd - szLine) == 10) && (strncmp(szLine, "\t\tand 0xFF", 10) == 0) && (*szEnd == '\n') && IR816MaskRedone(szEnd + 1))
		{
			continue;
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut);
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the operand names szWord anywhere in it.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816OperandMentions(SIR816Line* psLine, const char* szWord, size uiWord)
{
	size	ui;

	for (ui = 0; ui + uiWord <= psLine->uiOperand; ui++)
	{
		if (strncmp(psLine->szOperand + ui, szWord, uiWord) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value stored to the word szWord (as "[D + 12]") by the line
// before szLine can stay in A instead: the next line that mentions the word
// loads it (and nothing after the load reads the N and Z it sets) or pushes
// it (pei, which becomes pha), and nothing before that changes A or goes
// anywhere (only clc, sec, ldx, ldy, stores, tax, tay, pha, pea, pei, phk and
// cmp come between).  Its one use (Store) is that load or push.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816KeepsInA(const char* szLine, const char* szWord, size uiWord)
{
	SIR816Line	sLine;

	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
		if (IR816OperandMentions(&sLine, szWord, uiWord))
		{
			return (sLine.uiOperand == uiWord) && (((sLine.uiMnemonic == IR816M_Lda) && !IR816ZNReadAfter(szLine)) || (sLine.uiMnemonic == IR816M_Pei));
		}
		if (!(sLine.uiFlags & (IR816MF_CARRY | IR816MF_LOAD_INDEX | IR816MF_STORE | IR816MF_PUSH_WORD | IR816MF_COPIES_A)) && (sLine.uiMnemonic != IR816M_Cmp))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an lda before the line at szLine is never read: in straight line
// code, before anything reads A or the N and Z the lda set, an instruction
// sets A again (only IR816LeavesA instructions come between).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816DeadLoad(const char* szLine)
{
	SIR816Line	sLine;

	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
		if (sLine.uiFlags & IR816MF_LOADS_A)
		{
			return true;
		}
		if (!IR816LeavesA(&sLine))
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
	SIR816Line	sLine;

	while (szLine && ((strncmp(szLine, "\t\tclc\n", 6) == 0) || (strncmp(szLine, "\t\tsec\n", 6) == 0)))
	{
		szLine = IR816NextLine(szLine);
	}
	if (!szLine || (strncmp(szLine, "\t\tlda ", 6) != 0))
	{
		return false;
	}
	szLoad = szLine;
	IR816Decode(szLine, &sLine);
	if (IR816OperandMentions(&sLine, szWord, uiWord))
	{
		return false;
	}
	if ((sLine.uiOperand == 0) || (sLine.uiOperand > 40) || (sLine.szOperand[sLine.uiOperand - 1] == 'A') || (strchr(sLine.szOperand, ';') && (strchr(sLine.szOperand, ';') < sLine.szOperand + sLine.uiOperand)))
	{
		return false;
	}
	szLine = IR816NextLine(szLine);
	if (!szLine || ((szLine[0] != '\t') || (szLine[1] != '\t')))
	{
		return false;
	}
	IR816Decode(szLine, &sLine);
	if ((!(sLine.uiFlags & IR816MF_LOGIC) && (sLine.uiMnemonic != IR816M_Adc)) || !IR816OperandIs(&sLine, szWord, uiWord))
	{
		return false;
	}
	*pszLoad = szLoad;
	*pszUse = szLine;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether A and the direct page word it holds (Peephole) stay as they are
// after the instruction: it changes neither (ldx, ldy, clc, sec, cmp, tax,
// tay, pha, pea, per, phk and the branches on Z, N and C), or it changes
// memory other than that word (a store, or a shift or step of memory; a
// store through a pointer might be to it, an address taken local).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816KeepsAWord(SIR816Line* psLine, const char* szA)
{
	if ((psLine->uiFlags & (IR816MF_LOAD_INDEX | IR816MF_CARRY | IR816MF_COPIES_A | IR816MF_BRANCH_ZN | IR816MF_BRANCH_C)) ||
		(psLine->uiMnemonic == IR816M_Cmp) || (psLine->uiMnemonic == IR816M_Pea) || (psLine->uiMnemonic == IR816M_Per) || (psLine->uiMnemonic == IR816M_Phk))
	{
		return true;
	}
	if ((psLine->uiFlags & (IR816MF_STORE | IR816MF_SHIFT_STEP)) && !IR816OperandIsA(psLine))
	{
		return psLine->bDirect && !IR816OperandIs(psLine, szA, StrLen(szA));
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the constant Y holds (Peephole) is changed by the instruction:
// it writes Y (IR816MF_WRITES_Y), calls, changes the widths, or is a bra,
// brl, jml or rtl.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ChangesYConstant(SIR816Line* psLine)
{
	return (psLine->uiFlags & (IR816MF_WRITES_Y | IR816MF_CALL | IR816MF_WIDTH)) ||
		   (psLine->uiMnemonic == IR816M_Rtl) || (psLine->uiMnemonic == IR816M_Brl) || (psLine->uiMnemonic == IR816M_Bra) || (psLine->uiMnemonic == IR816M_Jml);
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
	SIR816Line	sLine;
	char		szA[32];		//The direct page operand A holds ("[D + 12]"), or empty.
	char		szKept[32];		//The word whose store was left out, its value kept in A for its next lda, or empty.
	const char*	szSwapLoad;		//An lda left out: the value kept in A is the first operand instead (IR816SwapsInA)...
	const char*	szSwapUse;		//...of this instruction, which takes the lda's operand.
	char		szSwapped[64];
	char		szY[32];		//The constant Y holds, or empty.
	size		uiLength;
	size		uiDeadLoad;		//Where in szOut the last line starts if it is an lda of a direct page word or a constant, else -1.
	bool		bClearCarry;
	bool		bZNFromA;		//The last instruction kept set N and Z from A.
	bool		bKeep;

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
		szEnd = IR816LineEnd(szLine);
		uiLength = (size)(szEnd - szLine);
		bKeep = true;

		//A label, or a directive: nothing is known after it.
		if ((uiLength < 3) || !IR816IsInstruction(szLine))
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
		IR816Decode(szLine, &sLine);
		if (sLine.bMarked)
		{
			uiLength -= StrLen(IR816_KEEP_IN_A);
		}

		//A value kept in A instead of its store and load, or as the first operand of a commutative instruction.
		if (sLine.bMarked && sLine.bDirect && (sLine.uiMnemonic == IR816M_Sta) && (*szEnd == '\n') && IR816KeepsInA(szEnd + 1, sLine.szOperand, sLine.uiOperand))
		{
			memcpy(szKept, sLine.szOperand, sLine.uiOperand);
			szKept[sLine.uiOperand] = '\0';
			continue;
		}
		if (sLine.bMarked && sLine.bDirect && (szKept[0] == '\0') && (sLine.uiMnemonic == IR816M_Sta) && (*szEnd == '\n') &&
			IR816SwapsInA(szEnd + 1, sLine.szOperand, sLine.uiOperand, &szSwapLoad, &szSwapUse))
		{
			continue;
		}
		if (szLine == szSwapLoad)
		{
			//The lda's operand goes to the instruction it was loaded for.
			snprintf(szSwapped, sizeof(szSwapped), "%.*s", (int)sLine.uiOperand, sLine.szOperand);
			szSwapLoad = NULL;
			continue;
		}
		if (szLine == szSwapUse)
		{
			szOut.Append("\t\t");
			szOut.Append(sLine.szMnemonic);
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
		if (((sLine.uiMnemonic == IR816M_Lda) || (sLine.uiMnemonic == IR816M_Pei)) && IR816OperandIs(&sLine, szKept, StrLen(szKept)))
		{
			szKept[0] = '\0';
			if (sLine.uiMnemonic == IR816M_Pei)
			{
				szOut.Append("\t\tpha\n");
				uiDeadLoad = (size)-1;
			}
			continue;
		}
		if ((sLine.uiMnemonic == IR816M_Pei) && IR816OperandIs(&sLine, szA, StrLen(szA)))
		{
			//A holds the word: pha pushes it for less.
			szOut.Append("\t\tpha\n");
			uiDeadLoad = (size)-1;
			continue;
		}

		if ((sLine.uiMnemonic == IR816M_Lda) && sLine.bDirect && IR816OperandIs(&sLine, szA, StrLen(szA)) && !IR816BranchesOnZN(szEnd))
		{
			bKeep = false;
		}
		else if ((sLine.uiMnemonic == IR816M_Ldy) && (sLine.uiOperand < 30) && IR816OperandIs(&sLine, szY, StrLen(szY)))
		{
			bKeep = false;
		}
		else if ((sLine.uiMnemonic == IR816M_Adc) && IR816OperandIs(&sLine, "0", 1) && bClearCarry)
		{
			bKeep = false;
		}
		else if ((sLine.uiMnemonic == IR816M_Sta) && sLine.bDirect && IR816OperandIs(&sLine, szA, StrLen(szA)))
		{
			//The word already holds A.
			bKeep = false;
		}
		else if ((sLine.uiMnemonic == IR816M_Cmp) && IR816OperandIs(&sLine, "0", 1) && bZNFromA && IR816BranchesOnZN(szEnd) &&
				 !IR816CarryReadAfterBranch(szText, szEnd))
		{
			bKeep = false;
		}
		if (!bKeep)
		{
			continue;
		}
		if ((sLine.uiMnemonic == IR816M_Lda) && (uiDeadLoad != (size)-1))
		{
			//The lda before it loaded A for nothing.
			szOut.SetLength(uiDeadLoad);
		}
		uiDeadLoad = ((sLine.uiMnemonic == IR816M_Lda) && (sLine.bDirect || IR816IsDecimal(sLine.szOperand))) ? szOut.Length() : (size)-1;
		szOut.Append(szLine, uiLength);
		szOut.AppendNewLine();
		if (!(sLine.uiFlags & IR816MF_STORE) && (sLine.uiMnemonic != IR816M_Pha))
		{
			//Stores change neither A nor the flags.
			bZNFromA = IR816SetsZNFromA(&sLine);
		}

		//What A and Y hold after it.
		if (((sLine.uiMnemonic == IR816M_Lda) || (sLine.uiMnemonic == IR816M_Sta)) && sLine.bDirect)
		{
			memcpy(szA, sLine.szOperand, sLine.uiOperand);
			szA[sLine.uiOperand] = '\0';
		}
		else if (!IR816KeepsAWord(&sLine, szA))
		{
			szA[0] = '\0';
		}

		if ((sLine.uiMnemonic == IR816M_Ldy) && (sLine.uiOperand < 30) && IR816IsDecimal(sLine.szOperand))
		{
			memcpy(szY, sLine.szOperand, sLine.uiOperand);
			szY[sLine.uiOperand] = '\0';
		}
		else if (IR816ChangesYConstant(&sLine))
		{
			szY[0] = '\0';
		}
		if (sLine.uiFlags & (IR816MF_WIDTH | IR816MF_CALL | IR816MF_SETS_D))
		{
			szA[0] = '\0';
		}
		bClearCarry = (sLine.uiMnemonic == IR816M_Clc) || (bClearCarry && (sLine.uiMnemonic == IR816M_Lda));
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
	SIR816Line	sLine;

	szOut.Init();
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		if ((szEnd - szLine >= 3) && IR816IsInstruction(szLine) && (*szEnd == '\n'))
		{
			IR816Decode(szLine, &sLine);
			if (sLine.bDirect && (sLine.uiFlags & IR816MF_STORE) && IR816DeadStore(szEnd + 1, sLine.szOperand, sLine.uiOperand))
			{
				continue;
			}
			if ((sLine.uiMnemonic == IR816M_Lda) && (sLine.bDirect || ((sLine.uiOperand > 0) && (sLine.szOperand[0] != '['))) && IR816DeadLoad(szEnd + 1))
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
	IR816AppendFrameEntry(mpszOut, miFrameBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// The result is in A (and X); Y keeps A while S is moved back.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Epilogue(void)
{
	IR816AppendFrameExit(mpszOut, miFrameBytes);
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
				mpszOut->Append("\t\tsta [[D + ")->Append(IR816_SCRATCH_POINTER)->Append("]:24 + Y]")->AppendNewLine();
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
void CIR816Backend::JumpTo(CIRBlock* pcFrom, CIRBlock* pcTo, CIRBlock* pcNext)
{
	CChars	sz;

	if (pcTo == pcNext)
	{
		return;
	}
	if (mpcDominators && mpcDominators->IsBackEdge(pcFrom, pcTo))
	{
		//Going round a loop: marked for PeepholeBranches.
		sz.Init("bra L");
		sz.Append(*maiBlockLabels.Get(pcTo->muiIndex));
		sz.Append(IR816_ROUND);
		Line(sz.Text());
		sz.Kill();
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
		JumpTo(pcBlock, psJump->pcThen, pcNext);
		break;
	case IRJ_Branch:
		//Straight to a block whose edge has no copies, falling through to the next block where it can.
		bThenCopies = HasEdgeCopies(pcBlock, psJump->pcThen);
		bElseCopies = HasEdgeCopies(pcBlock, psJump->pcElse);
		if (!bElseCopies && ((psJump->pcThen == pcNext) || bThenCopies || (psJump->pcElse != pcNext)))
		{
			BranchWhen(pcBlock, false, *maiBlockLabels.Get(psJump->pcElse->muiIndex));
			EdgeCopies(pcBlock, psJump->pcThen);
			JumpTo(pcBlock, psJump->pcThen, pcNext);
		}
		else if (!bThenCopies)
		{
			BranchWhen(pcBlock, true, *maiBlockLabels.Get(psJump->pcThen->muiIndex));
			EdgeCopies(pcBlock, psJump->pcElse);
			JumpTo(pcBlock, psJump->pcElse, pcNext);
		}
		else
		{
			iThen = NewLabel();
			BranchWhen(pcBlock, true, iThen);
			EdgeCopies(pcBlock, psJump->pcElse);
			JumpTo(pcBlock, psJump->pcElse, NULL);
			Label(iThen);
			EdgeCopies(pcBlock, psJump->pcThen);
			JumpTo(pcBlock, psJump->pcThen, pcNext);
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
		//A zext folded into its ptradds' loads and stores (FoldIndices) isn't worked out.
		if (FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult))
		{
			Extend(psInstruction);
		}
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
		mauiPendingArguments.Add(&uiIndex);
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
		Line("eor 0x8000");
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
		Line("eor 0x80");
		Line("sec");
		Line("sbc 0x80");
	}
	Frame("sta", IR816_SCRATCH_VALUE);
	if (bSigned && !sType.Is(IRTK_I1))
	{
		Line("asl A");
		Line("lda 0");
		Line("adc 0xFFFF");
		Line("eor 0xFFFF");
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
// low wrapped to 0; or dec high when low is 0, then dec low.  An i16 is one
// inc or dec.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::StepInPlace(SIRInstruction* psInstruction)
{
	SIRReference	sValue;
	uint32			uiStep;
	int32			iOffset;
	int32			iSkip;
	bool			bWord;

	sValue = psInstruction->asArguments[0];
	bWord = psInstruction->sType.Is(IRTK_I16);
	if (((Words(psInstruction->sType) != 2) && !bWord) || !psInstruction->sResult.IsTemporary() || !sValue.IsTemporary() || Fused(sValue) ||
		!psInstruction->asArguments[1].IsConstant() || InX(sValue) || InX(psInstruction->sResult) ||
		(OffsetOf(psInstruction->sResult) != OffsetOf(sValue)) || (OffsetOf(sValue) > 253))
	{
		return false;
	}
	uiStep = (uint32)mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
	if (psInstruction->uiOpcode == IRO_Sub)
	{
		uiStep = 0 - uiStep;
	}
	if (bWord)
	{
		uiStep = (uiStep & 0xFFFF) == 0xFFFF ? 0xFFFFFFFF : (uiStep & 0xFFFF);
	}
	if ((uiStep != 1) && (uiStep != 0xFFFFFFFF))
	{
		return false;
	}
	iOffset = OffsetOf(sValue);
	if (bWord)
	{
		Frame((uiStep == 1) ? "inc" : "dec", iOffset);
		return true;
	}
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

	sType = psInstruction->sType;
	iResult = OffsetOf(psInstruction->sResult);
	iWords = Words(sType);
	sAmount = psInstruction->asArguments[1];
	if (InX(psInstruction->sResult))
	{
		//A 16 bit shift by a constant (AllocateX), in A.
		Operand("lda", psInstruction->asArguments[0], 0);
		ShiftWordInA(psInstruction->uiOpcode, (int)(mpcFunction->GetConstant(sAmount)->ulliBits & 63));
		Line("tax");
		return;
	}
	CopyWords(iResult, psInstruction->asArguments[0], iWords);
	if (sAmount.IsConstant() && ((mpcFunction->GetConstant(sAmount)->ulliBits & 63) == 0))
	{
		return;
	}

	if ((psInstruction->uiOpcode == IRO_AShr) && (sType.Is(IRTK_I8) || sType.Is(IRTK_I1)))
	{
		Frame("lda", iResult);
		Line(sType.Is(IRTK_I8) ? "eor 0x80" : "eor 1");
		Line("sec");
		Line(sType.Is(IRTK_I8) ? "sbc 0x80" : "sbc 1");
		Frame("sta", iResult);
	}

	if (sAmount.IsConstant())
	{
		//Without a count in X (a value kept in X may be live: AllocateX only keeps them from variable shifts).
		ShiftConstant(psInstruction->uiOpcode, iResult, iWords, (int)(mpcFunction->GetConstant(sAmount)->ulliBits & 63));
		if (sType.Is(IRTK_I8) || sType.Is(IRTK_I1))
		{
			Frame("lda", iResult);
			Mask(sType);
			Frame("sta", iResult);
		}
		return;
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
// A shift by a count known when it runs, with a word in A: the value's
// word, or for 32 bits the word the bits leave from, the other rolled in
// memory.  Counts below IR816_SHIFT_LOOP_BELOW loop a bit at a time (the
// cheapest for so few).  Larger counts past the width fill the value
// (ShiftConstant); a whole word (32 bits) and then a byte are moved as
// ShiftConstant moves them; the 0 to 7 bits left jump (jmp [table + X])
// into an unrolled run of seven one bit steps.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftInA(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd)
{
	int32	iSmall;
	int32	iSmallLoop;
	int32	iUnder;
	int32	iBits;
	int32	iDone;
	int32	aiSteps[IR816_SHIFT_STEPS];
	int32	iInA;
	int32	iOther;
	int		i;
	CChars	sz;

	//Left shifts carry from the low word up, right shifts from the high word down.
	iInA = (uiOpcode == IRO_Shl) ? iResult : iResult + 2 * (iWords - 1);
	iOther = (uiOpcode == IRO_Shl) ? iResult + 2 : iResult;

	//The count is tested in A; it waits in X while a word or byte move uses A.
	iSmall = NewLabel();
	Operand("lda", sAmount, 0);
	Line("and 63");
	Branch("beq", iEnd);
	sz.Init("cmp ");
	sz.Append(IR816_SHIFT_LOOP_BELOW);
	Line(sz.Text());
	sz.Kill();
	Branch("bcc", iSmall);

	//The whole value shifted out.
	iUnder = NewLabel();
	Line((iWords == 2) ? "cmp 32" : "cmp 16");
	Branch("bcc", iUnder);
	ShiftConstant(uiOpcode, iResult, iWords, 16 * iWords);
	Branch("bra", iEnd);
	Label(iUnder);

	if (iWords == 2)
	{
		iBits = NewLabel();
		Line("cmp 16");
		Branch("bcc", iBits);
		Line("tax");
		ShiftConstant(uiOpcode, iResult, 2, 16);
		Line("txa");
		Line("and 15");
		Branch("beq", iEnd);
		Label(iBits);
	}

	iBits = NewLabel();
	Line("cmp 8");
	Branch("bcc", iBits);
	Line("tax");
	ShiftConstant(uiOpcode, iResult, iWords, 8);
	Line("txa");
	Line("and 7");
	Branch("beq", iEnd);
	Label(iBits);

	iDone = NewLabel();
	for (i = 0; i < IR816_SHIFT_STEPS; i++)
	{
		aiSteps[i] = NewLabel();
	}
	Line("asl A");
	Line("tax");
	Frame("lda", iInA);
	sz.Init("jmp [L");
	sz.Append(iLoop);
	sz.Append(" + X]");
	Line(sz.Text());
	sz.Kill();

	//Entry n runs the last n steps.
	Label(iLoop);
	for (i = 0; i <= IR816_SHIFT_STEPS; i++)
	{
		sz.Init(".word L");
		sz.Append((i == 0) ? iDone : aiSteps[IR816_SHIFT_STEPS - i]);
		sz.Append(" & 0xFFFF");
		Line(sz.Text());
		sz.Kill();
	}
	for (i = 0; i < IR816_SHIFT_STEPS; i++)
	{
		Label(aiSteps[i]);
		ShiftStepInA(uiOpcode, iWords, iOther);
	}
	Label(iDone);
	Frame("sta", iInA);
	Branch("bra", iEnd);

	//A few bits: one a time, counted down in X.
	Label(iSmall);
	iSmallLoop = NewLabel();
	Line("tax");
	Frame("lda", iInA);
	Label(iSmallLoop);
	ShiftStepInA(uiOpcode, iWords, iOther);
	Line("dex");
	Branch("bne", iSmallLoop);
	Frame("sta", iInA);
	Label(iEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// One bit of a shift with a word in A (the other word, for 32 bits, rolled
// in memory at iOther).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftStepInA(uint16 uiOpcode, int iWords, int32 iOther)
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
		Line("cmp 0x8000");
		Line("ror A");
	}
	if (iWords == 2)
	{
		Frame((uiOpcode == IRO_Shl) ? "rol" : "ror", iOther);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The 16 bit word in A shifted iAmount bits: 8 of them at once by
// swapping its bytes (xba, then the byte shifted in made 0), the rest a
// bit at a time.  An ashr by 15 or more is the sign (asl puts it in the
// carry; 0 + $FFFF and the carry is 0 or $FFFF, inverted); one by 3 or more
// is the lsr then the sign bit, now at bit 15 - iAmount, extended (x ^ m) -
// m (IR816SignExtend); by 1 or 2 a ror with the sign in the carry.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftWordInA(uint16 uiOpcode, int iAmount)
{
	int		iBits;

	if ((uiOpcode == IRO_AShr) && (iAmount >= 15))
	{
		Line("asl A");
		Line("lda 0");
		Line("adc 0xFFFF");
		Line("eor 0xFFFF");
		return;
	}
	if (iAmount >= 16)
	{
		Line("lda 0");
		return;
	}
	if ((uiOpcode == IRO_AShr) && (iAmount < 3))
	{
		for (iBits = 0; iBits < iAmount; iBits++)
		{
			Line("cmp 0x8000");
			Line("ror A");
		}
		return;
	}

	iBits = iAmount;
	if (iBits >= 8)
	{
		Line("xba");
		Line((uiOpcode == IRO_Shl) ? "and 0xFF00" : "and 0xFF");
		iBits -= 8;
	}
	for (; iBits > 0; iBits--)
	{
		Line((uiOpcode == IRO_Shl) ? "asl A" : "lsr A");
	}
	if ((uiOpcode == IRO_AShr) && (iAmount > 0))
	{
		SignExtendInA(15 - iAmount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The word in A (0 above bit iSignBit) with that bit copied up through bit
// 15: (x ^ m) - m with m the sign bit.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::SignExtendInA(int iSignBit)
{
	char	szLine[32];

	snprintf(szLine, sizeof(szLine), "eor 0x%X", 1u << iSignBit);
	Line(szLine);
	Line("sec");
	snprintf(szLine, sizeof(szLine), "sbc 0x%X", 1u << iSignBit);
	Line(szLine);
}




//////////////////////////////////////////////////////////////////////////
//
// A shift in memory (64 bits, or a frame past the direct page) by a count
// known when it runs, kept in the scratch value word (ShiftConstant's ashr
// uses the scratch count for its sign word, and X addresses frames past the
// direct page).  Counts past the width fill the value; then 32 and 16 bits
// are moved as whole words and 8 as a byte (ShiftConstant), so only the 0
// to 7 bits left are shifted a bit at a time.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftInMemory(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd)
{
	static const int	aiSteps[] = { 32, 16, 8 };
	CChars				sz;
	int					i;
	int32				iSkip;

	Operand("lda", sAmount, 0);
	Line("and 63");
	Branch("beq", iEnd);

	//The whole value shifted out (64 bits never is: the count is below 64).
	if (16 * iWords <= 63)
	{
		iSkip = NewLabel();
		sz.Init("cmp ");
		sz.Append(16 * iWords);
		Line(sz.Text());
		sz.Kill();
		Branch("bcc", iSkip);
		ShiftConstant(uiOpcode, iResult, iWords, 16 * iWords);
		Branch("bra", iEnd);
		Label(iSkip);
	}
	Frame("sta", IR816_SCRATCH_VALUE);

	//Each step only when the count, now less than twice it, reaches it.
	for (i = 0; i < 3; i++)
	{
		if (aiSteps[i] >= 16 * iWords)
		{
			continue;
		}
		iSkip = NewLabel();
		sz.Init("cmp ");
		sz.Append(aiSteps[i]);
		Line(sz.Text());
		Branch("bcc", iSkip);
		sz.Kill();
		ShiftConstant(uiOpcode, iResult, iWords, aiSteps[i]);
		Frame("lda", IR816_SCRATCH_VALUE);
		sz.Init("and ");
		sz.Append(aiSteps[i] - 1);
		Line(sz.Text());
		sz.Kill();
		Branch("beq", iEnd);
		Frame("sta", IR816_SCRATCH_VALUE);
		Label(iSkip);
	}

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
			Line("cmp 0x8000");
			Line("ror A");
			Frame("sta", iResult + 2 * (iWords - 1));
		}
		for (i = iWords - 2; i >= 0; i--)
		{
			Frame("ror", iResult + 2 * i);
		}
	}
	Frame("dec", IR816_SCRATCH_VALUE);
	Branch("bne", iLoop);
	Label(iEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// The value at iResult (iWords words) shifted iAmount bits where it is: whole
// words are moved first (the words shifted in are 0, or the sign for ashr),
// then a byte when 8 or more bits are left, then the bits left a bit at a
// time without a count: one word in A, two with the low one in A, more in
// memory; ashr's bits as lshr's, then the top word's sign extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftConstant(uint16 uiOpcode, int32 iResult, int iWords, int iAmount)
{
	int		iWordShift;
	int		iBits;
	int		i;
	int		j;
	int32	iLow;
	int		iRemaining;

	if (iWords == 1)
	{
		Frame("lda", iResult);
		ShiftWordInA(uiOpcode, iAmount);
		Frame("sta", iResult);
		return;
	}

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
				Line("cmp 0x8000");
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

	if (iBits >= 8)
	{
		//A byte is moved a word at a time: a word read a byte along is the bytes shifted 8 bits.  The byte past the
		//value's end (or before its start) is 0, or the sign for ashr (the words moved in are), except at the value's
		//own ends.
		if (uiOpcode == IRO_Shl)
		{
			for (i = iWords - 1; i >= iWordShift; i--)
			{
				Frame("lda", iResult + 2 * i - 1);
				if (i == 0)
				{
					Line("and 0xFF00");
				}
				Frame("sta", iResult + 2 * i);
			}
		}
		else
		{
			for (i = 0; i < iWords - iWordShift; i++)
			{
				Frame("lda", iResult + 2 * i + 1);
				if (i == iWords - 1)
				{
					Line("and 0xFF");
					if (uiOpcode == IRO_AShr)
					{
						Line("cmp 0x80");
						Line("bcc C + 3");
						Line("ora 0xFF00");
					}
				}
				Frame("sta", iResult + 2 * i);
			}
		}
		iBits -= 8;
	}

	if (iBits == 0)
	{
		return;
	}
	iLow = (uiOpcode == IRO_Shl) ? iResult + 2 * iWordShift : iResult;
	iRemaining = iWords - iWordShift;
	if (iRemaining == 1)
	{
		//The one word left, shifted in A (a bit of shl or lshr in memory).
		if ((iBits == 1) && (uiOpcode != IRO_AShr))
		{
			Frame((uiOpcode == IRO_Shl) ? "asl" : "lsr", iLow);
			return;
		}
		Frame("lda", iLow);
		ShiftWordInA(uiOpcode, iBits);
		Frame("sta", iLow);
		return;
	}
	if (iRemaining == 2)
	{
		//Two words left: the low one in A, the bit between them carried through memory's high word.
		Frame("lda", iLow);
		for (j = 0; j < iBits; j++)
		{
			if (uiOpcode == IRO_Shl)
			{
				Line("asl A");
				Frame("rol", iLow + 2);
			}
			else
			{
				Frame("lsr", iLow + 2);
				Line("ror A");
			}
		}
		Frame("sta", iLow);
	}
	else
	{
		//More words, a bit at a time in memory.
		for (j = 0; j < iBits; j++)
		{
			if (uiOpcode == IRO_Shl)
			{
				Frame("asl", iLow);
				for (i = 1; i < iRemaining; i++)
				{
					Frame("rol", iLow + 2 * i);
				}
			}
			else
			{
				Frame("lsr", iLow + 2 * (iRemaining - 1));
				for (i = iRemaining - 2; i >= 0; i--)
				{
					Frame("ror", iLow + 2 * i);
				}
			}
		}
	}

	//ashr was shifted as lshr: the top word left gets its sign bit, now at 15 - iBits, extended.
	if (uiOpcode == IRO_AShr)
	{
		Frame("lda", iLow + 2 * (iRemaining - 1));
		SignExtendInA(15 - iBits);
		Frame("sta", iLow + 2 * (iRemaining - 1));
	}
}


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
static bool IR816ScratchRoutine(const char* szName)
{
	const SIR816Routine*	psRoutine;

	psRoutine = IR816FindRoutine(szName);
	return psRoutine && psRoutine->bScratch;
}


//////////////////////////////////////////////////////////////////////////
//
// mul, udiv, sdiv, urem and srem call R816_<operation><bits>, an i8 as 16 bits (sign extended for sdiv and srem).
// The scratch routines get their operands at IR816_SCRATCH_ARGUMENT_A and _B, the others pushed.
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
	if (TableDivide(psInstruction, bSigned) || ByteMultiply(psInstruction))
	{
		return true;
	}

	szName.Init(IR816_RUNTIME_PREFIX);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Mul:	szName.Append("Mul");	break;
	case IRO_UDiv:	szName.Append("UDiv");	break;
	case IRO_SDiv:	szName.Append("SDiv");	break;
	case IRO_URem:	szName.Append("URem");	break;
	default:		szName.Append("SRem");	break;
	}
	szName.Append(iBits);

	iBytes = 0;
	if (IR816ScratchRoutine(szName.Text()))
	{
		CopyWords(IR816_SCRATCH_ARGUMENT_A, psInstruction->asArguments[0], Words(sType));
		CopyWords(IR816_SCRATCH_ARGUMENT_B, psInstruction->asArguments[1], Words(sType));
	}
	else
	{
		iBytes = 2 * ValueBytes(sType);
		for (i = 1; i >= 0; i--)
		{
			if (bSigned && sType.Is(IRTK_I8))
			{
				Operand("lda", psInstruction->asArguments[i], 0);
				Line("eor 0x80");
				Line("sec");
				Line("sbc 0x80");
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
	}
	UseRuntime(szName.Text());
	szName.Insert(0, "jsl ");
	Line(szName.Text());
	szName.Kill();
	CallerCleanUp(iBytes, psInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A hot i8 divide or remainder by a constant (IRF_Hot, CIROptimiser::
// ConstantDivisions) looked up in R816_STable<c> or R816_UTable<c>: a word
// per byte value, q in its low byte and the remainder in its high byte.
// False, with nothing written, for any other.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::TableDivide(SIRInstruction* psInstruction, bool bSigned)
{
	CChars	szName;
	uint64	ulliDivisor;

	if (!(psInstruction->uiFlags & IRF_Hot) || !psInstruction->sType.Is(IRTK_I8) || !psInstruction->asArguments[1].IsConstant())
	{
		return false;
	}
	ulliDivisor = mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits & 0xFF;
	if ((ulliDivisor < 3) || (bSigned && (ulliDivisor >= 0x80)))
	{
		return false;
	}
	szName.Init(bSigned ? IR816_SIGNED_TABLE : IR816_UNSIGNED_TABLE);
	szName.Append((int)ulliDivisor);
	UseRuntime(szName.Text());
	Operand("lda", psInstruction->asArguments[0], 0);
	Line("asl A");
	Line("tax");
	mpszOut->Append("\t\tlda [")->Append(szName)->Append(" + X]:24\n");
	szName.Kill();
	if ((psInstruction->uiOpcode == IRO_SRem) || (psInstruction->uiOpcode == IRO_URem))
	{
		Line("xba");
	}
	Line("and 0xFF");
	if (!psInstruction->sResult.IsNone())
	{
		Store(psInstruction->sResult, 0);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An i8 multiply: the low byte of the product in MathMult (the word at
// (a * 256 + b) * 2 is a * b, as R816_Mul16 reads it), looked up in place.
// False, with nothing written, for any other multiply.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::ByteMultiply(SIRInstruction* psInstruction)
{
	int32	iHigh;
	int32	iGot;

	if ((psInstruction->uiOpcode != IRO_Mul) || !psInstruction->sType.Is(IRTK_I8))
	{
		return false;
	}
	iHigh = NewLabel();
	iGot = NewLabel();
	Operand("lda", psInstruction->asArguments[1], 0);
	Line("xba");
	Operand("ora", psInstruction->asArguments[0], 0);
	Line("asl A");
	Line("tax");
	Branch("bcs", iHigh);
	Line("lda [MathMult + X]:24");
	Branch("bra", iGot);
	Label(iHigh);
	Line("lda [MathMult + 0x10000 + X]:24");
	Label(iGot);
	Line("and 0xFF");
	if (!psInstruction->sResult.IsNone())
	{
		Store(psInstruction->sResult, 0);
	}
	return true;
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
	szFlip = sType.Is(IRTK_I1) ? "eor 1" : (sType.Is(IRTK_I8) ? "eor 0x80" : "eor 0x8000");
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
// Leaves N set when the first of two 16 bit signed values is less than the
// second (the second less than the first when bSwapped): the subtraction's
// sign, flipped when it overflowed.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::SignedLessFlags(SIRReference* psArguments, bool bSwapped)
{
	int32	iNoOverflow;

	iNoOverflow = NewLabel();
	Line("sec");
	Operand("lda", psArguments[bSwapped ? 1 : 0], 0);
	Operand("sbc", psArguments[bSwapped ? 0 : 1], 0);
	Branch("bvc", iNoOverflow);
	Line("eor 0x8000");
	Label(iNoOverflow);
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
	if (IsIRSignedComparison(uiOpcode) && psInstruction->sType.Is(IRTK_I16) && !psInstruction->asArguments[1].IsConstant())
	{
		//Two values: one subtraction with its overflow folded into the sign answers lt and ge; gt and le subtract the
		//other way around (a > b is b < a).
		SignedLessFlags(psInstruction->asArguments, (uiOpcode == IRO_SGt) || (uiOpcode == IRO_SLe));
		Branch(((uiOpcode == IRO_SLt) || (uiOpcode == IRO_SGt)) ? "bmi" : "bpl", iLabel);
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
		Line("eor 0x80");
		Line("sec");
		Line("sbc 0x80");
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
			Line("eor 0x80");
			Line("sec");
			Line("sbc 0x80");
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
		Line("and 0xFF");
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
	SIRReference	sIndex;
	int				i;

	if (psInstruction->uiOpcode == IRO_Load)
	{
		sPointer = FoldedBase(psInstruction->asArguments[0]);
		sIndex = FoldedIndex(psInstruction->asArguments[0]);
		if (sIndex.IsTemporary() && !InX(sIndex) && FusedOverlaps(sResult, sIndex, true))
		{
			return false;
		}
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
	bool			bWordCopy;
	size			uiNext;

	if (miFused >= IR816_MAX_FUSED)
	{
		return false;
	}
	psInstruction = pcBlock->masInstructions.Get(uiIndex);

	//The next instruction that writes code (IsFolded ones don't).
	for (uiNext = uiIndex + 1; (uiNext < pcBlock->masInstructions.NumElements()) && IsFolded(pcBlock->masInstructions.Get(uiNext)); uiNext++)
	{
	}
	if (uiNext >= pcBlock->masInstructions.NumElements())
	{
		return CanFuseReturn(pcBlock, psInstruction);
	}
	psNext = pcBlock->masInstructions.Get(uiNext);
	sResult = psInstruction->sResult;

	//A 1 word load only stored by the next instruction is a copy through A (a byte's under the store's sep m), from
	//any address: there is no second word to read after the first is written.
	bWordCopy = (miFused == 0) && (psInstruction->uiOpcode == IRO_Load) && (Words(psInstruction->sType) == 1) && (psNext->uiOpcode == IRO_Store) &&
		psNext->sType.Equals(psInstruction->sType) && psNext->asArguments[0].Equals(sResult) && !psNext->asArguments[1].Equals(sResult);
	if (!sResult.IsTemporary() || ((Words(psInstruction->sType) < 2) && !bWordCopy) || psInstruction->sType.IsFloat() ||
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
		//Fall through.
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
		if ((psChained->uiOpcode == IRO_Load) && bStore && !bWordCopy && !psChained->asArguments[0].Equals(psNext->asArguments[1]))
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
			sz.Append("lda [[D + ");
			sz.Append(iPlace);
			sz.Append(LoadY(sFirst, iOffset + 2 * iWord) ? "]:24 + Y]" : "]:24]");
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
		Line("eor 0xFFFF");
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
		}

		//The value first: a fused load sets Y.
		Operand("lda", psInstruction->asArguments[0], i);
		if (!szSymbol)
		{
			sz.Append(LoadY(psInstruction->asArguments[1], iOffset + 2 * i) ? "]:24 + Y]" : "]:24]");
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
	for (ui = mauiPendingArguments.NumElements(); ui > 0; ui--)
	{
		psArgument = pcBlock->masInstructions.Get(*mauiPendingArguments.Get(ui - 1));
		PushValue(psArgument->asArguments[0], psArgument->sType);
		iBytes += ValueBytes(psArgument->sType);
	}
	mauiPendingArguments.FakeSetUsedElements(0);
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
			aszNames.AddIfUnique((char*)psRoutine->szCalls);
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
			pszDest->Append(psRoutine->szText);
			pszDest->AppendNewLine();
			continue;
		}
		if (IR816StartsWith(szName, IR816_SIGNED_TABLE) || IR816StartsWith(szName, IR816_UNSIGNED_TABLE))
		{
			AppendDivideTable(pszDest, szName);
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

