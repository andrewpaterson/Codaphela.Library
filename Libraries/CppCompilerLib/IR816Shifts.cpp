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
#include "IR816BackendInternal.h"


//The W65C816 back end's shifts: by a constant, a bit at a time in A or in memory, by whole bytes and words, or
//through the runtime.


#define IR816_SHIFT_LOOP_BELOW	5		//Variable shifts by fewer bits loop a bit at a time instead.


//////////////////////////////////////////////////////////////////////////
//
// The value is copied to the result and shifted there: by a constant (ShiftConstant), or a bit at a time by a count in
// X with a word in A (ShiftInA), or for 64 bits or a frame past the direct page in memory (ShiftInMemory).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Shift(SIRInstruction* psInstruction)
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
	if (BitShift(psInstruction) || ShiftTwoWords(psInstruction) || ShiftCall(psInstruction))
	{
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
		SignExtendInA(sType.Is(IRTK_I8) ? 7 : 0);
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
	if ((iWords <= 2) && IR816OnDirectPage(iResult + 2 * iWords))
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
// A 32 or 64 bit shift by a count known when it runs calls a runtime
// routine (IR816AppendShift), the count in A: a 32 bit value is copied to
// the scratch argument a and comes back in A and X; a 64 bit one is copied
// to the result and shifted there (its place in X).  False, with nothing
// written, for any other shift.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::ShiftCall(SIRInstruction* psInstruction)
{
	CChars	szName;
	int		iWords;

	iWords = Words(psInstruction->sType);
	if (psInstruction->asArguments[1].IsConstant() || ((iWords != 2) && (iWords != 4)) || psInstruction->sResult.IsNone())
	{
		return false;
	}
	szName.Init(IR816_RUNTIME_PREFIX);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Shl:	szName.Append("Shl");	break;
	case IRO_LShr:	szName.Append("LShr");	break;
	default:		szName.Append("AShr");	break;
	}
	szName.Append(16 * iWords);
	UseRuntime(szName.Text());
	if (iWords == 2)
	{
		CopyWords(IR816_SCRATCH_ARGUMENT_A, psInstruction->asArguments[0], 2);
		Operand("lda", psInstruction->asArguments[1], 0);
	}
	else
	{
		CopyWords(OffsetOf(psInstruction->sResult), psInstruction->asArguments[0], 4);
		Operand("lda", psInstruction->asArguments[1], 0);
		Line("ldx ", OffsetOf(psInstruction->sResult));
	}
	szName.Insert(0, "jsl ");
	Line(szName.Text());
	szName.Kill();
	if (iWords == 2)
	{
		CallerCleanUp(0, psInstruction);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A 32 bit shl or lshr by 1 to 7 from the value to the result, not copied
// to the result first: a bit through A from word to word, or more with the
// high word copied and the low word read into A, shifted there and stored.
// (The result is at the value's place or apart from it: the value's word
// still to be read isn't written first.)  False, with nothing written, for
// any other shift, or a result past the direct page.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::ShiftTwoWords(SIRInstruction* psInstruction)
{
	SIRReference	sValue;
	int32			iResult;
	int				iAmount;
	int				i;

	if ((Words(psInstruction->sType) != 2) || !psInstruction->asArguments[1].IsConstant() || psInstruction->sResult.IsNone() ||
		((psInstruction->uiOpcode != IRO_Shl) && (psInstruction->uiOpcode != IRO_LShr)))
	{
		return false;
	}
	iAmount = (int)(mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits & 63);
	iResult = OffsetOf(psInstruction->sResult);
	if ((iAmount < 1) || (iAmount > 7) || !IR816OnDirectPage(iResult + 4))
	{
		return false;
	}
	sValue = psInstruction->asArguments[0];
	if (iAmount == 1)
	{
		if (psInstruction->uiOpcode == IRO_Shl)
		{
			Operand("lda", sValue, 0);
			Line("asl A");
			Frame("sta", iResult);
			Operand("lda", sValue, 1);
			Line("rol A");
			Frame("sta", iResult + 2);
		}
		else
		{
			Operand("lda", sValue, 1);
			Line("lsr A");
			Frame("sta", iResult + 2);
			Operand("lda", sValue, 0);
			Line("ror A");
			Frame("sta", iResult);
		}
		return true;
	}
	Operand("lda", sValue, 1);
	Frame("sta", iResult + 2);
	Operand("lda", sValue, 0);
	for (i = 0; i < iAmount; i++)
	{
		if (psInstruction->uiOpcode == IRO_Shl)
		{
			Line("asl A");
			Frame("rol", iResult + 2);
		}
		else
		{
			Frame("lsr", iResult + 2);
			Line("ror A");
		}
	}
	Frame("sta", iResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 1 << n of 16, 32 or 64 bits, n known when it runs, looked up in
// R816_Bits16, R816_Bits32 or R816_Bits64 (AppendBitsTable) at n * 2, 4 or
// 8 in X instead of a shift: bitmaps' masks, and soft float's rounding
// masks ((1 << n) - 1, the subtract after).  False, with nothing written,
// for any other shift, or a result past the direct page (Frame would use
// X).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::BitShift(SIRInstruction* psInstruction)
{
	CChars	szTable;
	int32	iResult;
	int		iWords;
	int		i;

	iWords = Words(psInstruction->sType);
	if ((psInstruction->uiOpcode != IRO_Shl) || psInstruction->asArguments[1].IsConstant() || !psInstruction->asArguments[0].IsConstant() ||
		(mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits != 1) || !(psInstruction->sType.Is(IRTK_I16) || psInstruction->sType.Is(IRTK_I32) || psInstruction->sType.Is(IRTK_I64)) ||
		InX(psInstruction->sResult) || psInstruction->sResult.IsNone())
	{
		return false;
	}
	iResult = OffsetOf(psInstruction->sResult);
	if (!IR816OnDirectPage(iResult + 2 * iWords))
	{
		return false;
	}
	szTable.Init(IR816_BITS_TABLE);
	szTable.Append(16 * iWords);
	UseRuntime(szTable.Text());
	Operand("lda", psInstruction->asArguments[1], 0);
	Line("and 63");
	for (i = 1; i < 2 * iWords; i *= 2)
	{
		Line("asl A");
	}
	Line("tax");
	for (i = 0; i < iWords; i++)
	{
		mpszOut->Append("\t\tlda [")->Append(szTable);
		if (i > 0)
		{
			mpszOut->Append(" + ")->Append(2 * i);
		}
		mpszOut->Append(" + X]:24\n");
		Frame("sta", iResult + 2 * i);
	}
	szTable.Kill();
	return true;
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
void CIRW65C816Backend::ShiftInA(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd)
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
void CIRW65C816Backend::ShiftStepInA(uint16 uiOpcode, int iWords, int32 iOther)
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
void CIRW65C816Backend::ShiftWordInA(uint16 uiOpcode, int iAmount)
{
	int		iBits;

	if ((uiOpcode == IRO_AShr) && (iAmount >= 15))
	{
		SignWordInA();
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
void CIRW65C816Backend::SignExtendInA(int iSignBit)
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
// A becomes the sign word of A (0xFFFF if its top bit is set, else 0), with
// no branch (so no label: a hand counted branch hides the code from the
// flow passes).  Carry and the flags are changed.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::SignWordInA(void)
{
	Line("asl A");
	Line("lda 0");
	Line("adc 0xFFFF");
	Line("eor 0xFFFF");
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
void CIRW65C816Backend::ShiftInMemory(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd)
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
void CIRW65C816Backend::ShiftConstant(uint16 uiOpcode, int32 iResult, int iWords, int iAmount)
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
				SignWordInA();
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
						SignExtendInA(7);
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
// Whether a temporary is used, and only as the count of shifts by a count
// known when they run (all of which read just its low word: BitShift,
// ShiftInA, ShiftInMemory, ShiftCall).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::OnlyCounts(SIRReference sValue)
{
	size	ui;

	if (!sValue.IsTemporary())
	{
		return false;
	}
	ui = sValue.Index();
	return (ui < maiCountUses.NumElements()) && (ui < maiUses.NumElements()) && (*maiCountUses.Get(ui) > 0) && (*maiCountUses.Get(ui) == *maiUses.Get(ui));
}
