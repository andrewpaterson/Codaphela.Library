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
#include "IRBackend.h"
#include "IRW65C816Costs.h"


//The cycles a multiply by a constant costs as a runtime call (with its operands' stores), and roughly what a constant
//shift (by whole bytes, then bits), an add or sub and a neg cost, for 1, 2 and 4 word values (IR816ShiftAddCycles).
#define IR816_MULTIPLY_CALL_16	150
#define IR816_MULTIPLY_CALL_32	230
#define IR816_MULTIPLY_CALL_64	3000


//The cycles of a divide by a constant as its runtime call (R816_UDiv16, R816_UDiv32 and the signed ones over them, in
//Libc65's IRW65C816Runtime-Common.asm), counted from the routines: the operands' stores, the call, the routine's
//tests and the result's store; a step for each bit of the quotient it works out (the remainder a word wide; a byte
//wide under a divisor of 128; two words wide for a divisor of 65536 or more); and what a signed routine adds taking
//the signs off and putting them back.  The routines' short way out for an x below the divisor is not counted: the
//range says nothing of how often x is that small.
#define IR816_DIVIDE_CALL_16	110
#define IR816_DIVIDE_CALL_32	140
#define IR816_DIVIDE_STEP		18
#define IR816_DIVIDE_BYTE_STEP	14
#define IR816_DIVIDE_WIDE_STEP	46
#define IR816_DIVIDE_SIGNS_16	80
#define IR816_DIVIDE_SIGNS_32	100


//What a reciprocal costs besides its multiply and shifts: x made two words (a high word cleared, or its sign's word
//worked out), and the 1 added to the quotient of a negative x (the sign's word of x subtracted).
#define IR816_ZERO_EXTEND		6
#define IR816_SIGN_EXTEND		16
#define IR816_SIGN_FIX_16		14
#define IR816_SIGN_FIX_32		40


//////////////////////////////////////////////////////////////////////////
//
// Roughly the cycles of shifts and adds in a value of iWords words: a shift
// of iShift bits, then iAdds adds (or subs).
//
//////////////////////////////////////////////////////////////////////////
static int IR816ShiftCycles(int iWords, int iShift, int iAdds)
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
// A multiply as its runtime call.
//
//////////////////////////////////////////////////////////////////////////
int IR816MultiplyCallCycles(int iWords)
{
	return (iWords == 1) ? IR816_MULTIPLY_CALL_16 : ((iWords == 2) ? IR816_MULTIPLY_CALL_32 : IR816_MULTIPLY_CALL_64);
}


//////////////////////////////////////////////////////////////////////////
//
// ExpandMultiplies' shifts and adds for iDigits signed digits: the shifts
// add up to the top digit's place, each costs its setting up, and a top
// digit of -1 is a neg.
//
//////////////////////////////////////////////////////////////////////////
int IR816ShiftAddCycles(int iWords, int* aiPlaces, int* aiSigns, int iDigits)
{
	return IR816ShiftCycles(iWords, aiPlaces[iDigits - 1], iDigits - 1) + 5 * iWords * (iDigits - 1) + ((aiSigns[iDigits - 1] < 0) ? 12 * iWords : 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A multiply by ulliMultiplier as the back end will write it
// (ExpandMultiplies): the cheaper of its shifts and adds and its call; a
// power of 2 is a shift, and 1 is nothing.
//
//////////////////////////////////////////////////////////////////////////
int IR816MultiplyCycles(int iWords, uint64 ulliMultiplier)
{
	int		aiPlaces[IR_MAX_MULTIPLY_DIGITS];
	int		aiSigns[IR_MAX_MULTIPLY_DIGITS];
	int		iDigits;
	int		iCall;
	int		iShifts;

	iCall = IR816MultiplyCallCycles(iWords);
	if (iWords < 4)
	{
		ulliMultiplier &= (((uint64)1) << (iWords * 16)) - 1;
	}
	iDigits = CIRBackend::SignedDigits(ulliMultiplier, iWords * 16, aiPlaces, aiSigns);
	if (iDigits < 0)
	{
		return iCall;
	}
	if (iDigits < 2)
	{
		return (iDigits == 1) ? IR816ShiftCycles(iWords, aiPlaces[0], 0) : 0;
	}
	iShifts = IR816ShiftAddCycles(iWords, aiPlaces, aiSigns, iDigits);
	return (iShifts < iCall) ? iShifts : iCall;
}


//////////////////////////////////////////////////////////////////////////
//
// Roughly the cycles of a shift right by a constant of a value of 1 or 2
// words, as a reciprocal's product is shifted: 16 bits are the high word
// read for the low and 8 a swap of the bytes; a bit left is a shift of the
// word in A (2 cycles) or, for two words, of a word in memory and the one
// in A (9).
//
//////////////////////////////////////////////////////////////////////////
int IR816RightShiftCycles(int iWords, int iShift)
{
	if (iWords == 1)
	{
		return (iShift >= 8) ? (6 + 2 * (iShift - 8)) : (2 * iShift);
	}
	if (iShift >= 16)
	{
		return 8 + IR816RightShiftCycles(1, iShift - 16);
	}
	return (iShift >= 8) ? (20 + 9 * (iShift - 8)) : (10 + 9 * iShift);
}


//////////////////////////////////////////////////////////////////////////
//
// The words x * ulliScale is worked out in, x from lliMin to lliMax: one
// when it fits 16 bits (the optimiser narrows a multiply that does, and the
// back end leaves out a word it knows to be 0), else iMostWords.
//
//////////////////////////////////////////////////////////////////////////
int CIRW65C816DivisionCosts::ValueWords(int64 lliMin, int64 lliMax, uint64 ulliScale, int iMostWords)
{
	uint64	ulliMost;
	uint64	ulliLeast;

	ulliMost = (uint64)((lliMax < 0) ? -lliMax : lliMax);
	ulliLeast = (uint64)((lliMin < 0) ? -lliMin : lliMin);
	ulliMost = ((ulliMost > ulliLeast) ? ulliMost : ulliLeast) * ulliScale;
	if (ulliMost <= (uint64)((lliMin < 0) ? 0x7FFF : 0xFFFF))
	{
		return 1;
	}
	return iMostWords;
}


//////////////////////////////////////////////////////////////////////////
//
// x / c or x % c as the call of its divide routine: the 16 bit one for an
// i8 or an i16, and for an i32 when x and c are both under 65536
// (R816_UDiv32 goes to it), which works out a byte of quotient when x's
// high byte is below c, else a word; the 32 bit one works out 16 bits for
// x's low word after 8 or 16 for its high word when that isn't below c.  A
// c of 65536 or more gives a quotient of a byte or a word in steps two
// words wide.
//
//////////////////////////////////////////////////////////////////////////
int CIRW65C816DivisionCosts::DivideCost(SIRType sType, SIRConstantDivision* psDivision, int64 lliMin, int64 lliMax)
{
	uint64	ulliMost;
	uint64	ulliLeast;
	uint64	ulliDivisor;
	int		iWords;
	int		iSteps;
	int		iStep;
	int		iCost;

	iWords = (sType.IntegerBits() > 16) ? 2 : 1;
	ulliDivisor = (uint64)((psDivision->lliDivisor < 0) ? -psDivision->lliDivisor : psDivision->lliDivisor);
	ulliMost = (uint64)((lliMax < 0) ? -lliMax : lliMax);
	ulliLeast = (uint64)((lliMin < 0) ? -lliMin : lliMin);
	ulliMost = (ulliMost > ulliLeast) ? ulliMost : ulliLeast;

	iStep = (ulliDivisor < 0x80) ? IR816_DIVIDE_BYTE_STEP : IR816_DIVIDE_STEP;
	if ((iWords == 1) || ((ulliMost < 0x10000) && (ulliDivisor < 0x10000)))
	{
		iSteps = ((ulliMost >> 8) < ulliDivisor) ? 8 : 16;
		if (ulliDivisor >= 0x8000)
		{
			//The quotient is 0 or 1: a compare.
			iSteps = 0;
		}
	}
	else if (ulliDivisor < 0x10000)
	{
		iSteps = 16;
		if ((ulliMost >> 16) >= ulliDivisor)
		{
			iSteps += ((ulliMost >> 24) < ulliDivisor) ? 8 : 16;
		}
	}
	else
	{
		iSteps = ((ulliMost >> 8) < ulliDivisor) ? 8 : 16;
		iStep = IR816_DIVIDE_WIDE_STEP;
	}

	iCost = ((iWords == 1) ? IR816_DIVIDE_CALL_16 : IR816_DIVIDE_CALL_32) + iSteps * iStep;
	if (psDivision->bSigned)
	{
		iCost += (iWords == 1) ? IR816_DIVIDE_SIGNS_16 : IR816_DIVIDE_SIGNS_32;
	}
	return iCost;
}


//////////////////////////////////////////////////////////////////////////
//
// x / c or x % c as ConstantDivisions writes its reciprocal: x made as wide
// as the product when it is narrower, shifted right first for an even c,
// multiplied (IR816MultiplyCycles) and shifted right, 1 added to the
// quotient of a negative x; and for a remainder the quotient times c taken
// from x.  Each is counted in the words its values need (ValueWords), not
// the words of the type it is written in.
//
//////////////////////////////////////////////////////////////////////////
int CIRW65C816DivisionCosts::ReciprocalCost(SIRType sType, SIRConstantDivision* psDivision, int64 lliMin, int64 lliMax, SIRReciprocal* psReciprocal)
{
	int		iValueWords;
	int		iProductWords;
	int		iCost;

	iValueWords = ValueWords(lliMin, lliMax, 1, (sType.IntegerBits() > 16) ? 2 : 1);
	iProductWords = ValueWords(lliMin >> psReciprocal->iPreShift, lliMax >> psReciprocal->iPreShift, psReciprocal->ulliMultiplier, (psReciprocal->iBits > 16) ? 2 : 1);

	iCost = 0;
	if (iProductWords > iValueWords)
	{
		iCost += (lliMin < 0) ? IR816_SIGN_EXTEND : IR816_ZERO_EXTEND;
	}
	if (psReciprocal->iPreShift > 0)
	{
		iCost += IR816RightShiftCycles(iValueWords, psReciprocal->iPreShift);
	}
	iCost += IR816MultiplyCycles(iProductWords, psReciprocal->ulliMultiplier);
	iCost += IR816RightShiftCycles(iProductWords, psReciprocal->iShift);
	if (psDivision->bSigned && (lliMin < 0))
	{
		iCost += (iProductWords == 2) ? IR816_SIGN_FIX_32 : IR816_SIGN_FIX_16;
	}
	if (psDivision->bRemainder)
	{
		iCost += IR816MultiplyCycles(iValueWords, (uint64)psDivision->lliDivisor) + IR816ShiftCycles(iValueWords, 0, 1);
	}
	return iCost;
}

