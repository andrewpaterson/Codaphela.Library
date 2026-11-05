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
#include "BaseLib/StringHelper.h"
#include "IR65BackendInternal.h"


//The 65C64 back end's compares: integer comparisons as a value or a branch, and their operands.


//////////////////////////////////////////////////////////////////////////
//
// Branches to iLabel when the comparison is bJumpIf.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Compare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
{
	if (psInstruction->sType.IsFloat())
	{
		return FloatCompare(psInstruction, bJumpIf, iLabel);
	}
	return IntegerCompare(psInstruction, bJumpIf, iLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// cmp first, second for a comparison of 32 bits or less that needs no
// sign extending: the first operand where it is kept (X, [4 + W]) or in A
// or B, the second as an operand that can go with it.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CompareOperands(SIRReference sFirst, SIRReference sSecond)
{
	CChars	szLeft;
	CChars	szRight;
	bool	bSmall;

	szLeft.Init();
	szRight.Init();
	bSmall = IsInA(sSecond) || InRegister(sSecond) || SmallImmediate(sSecond, -8, 8);

	if (IsInA(sFirst))
	{
		szLeft.Append('A');
	}
	else if (InRegister(sFirst))
	{
		szLeft.Append(RegisterName(PlaceOf(sFirst)));
	}
	else if (bSmall && sFirst.IsTemporary() && (PlaceOf(sFirst)->uiKind == IR65P_Pseudo))
	{
		//Memory goes with a register or -8 to 8.
		AppendPlace(&szLeft, PlaceOf(sFirst));
	}
	else if (IsInA(sSecond))
	{
		LoadValue(sFirst, "B", IR65E_Zero);
		szLeft.Append('B');
	}
	else
	{
		LoadValue(sFirst, "A", IR65E_Zero);
		szLeft.Append('A');
	}

	if (!Operand(sSecond, &szRight, true, IR65E_Zero))
	{
		LoadValue(sSecond, (StringCompare(szLeft.Text(), "B") == 0) ? "A" : "B", IR65E_Zero);
		szRight.Append((StringCompare(szLeft.Text(), "B") == 0) ? 'A' : 'B');
	}
	Line("cmp ")->Append(szLeft)->Append(", ")->Append(szRight)->AppendNewLine();
	szRight.Kill();
	szLeft.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// After cmp X, Y: z is X = Y, c clear is X < Y unsigned and n != v is X < Y
// signed.  > and <= swap the operands (with a constant they were made >=
// and < by AdjustComparisons).  There is no branch on n = v, so it branches
// round a bra.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IntegerCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
{
	uint16			uiOpcode;
	bool			bSigned;
	bool			bSwap;
	bool			bLess;
	bool			bWide;
	int32			iSkip;
	SIRReference	sLeft;
	SIRReference	sRight;
	int				iBits;

	uiOpcode = psInstruction->uiOpcode;
	if (mbFlagsKnown)
	{
		//The block before compared the same operands.
		Branch(((uiOpcode == IRO_Eq) == bJumpIf) ? "bras z, " : "brac z, ", iLabel);
		return true;
	}
	bSigned = IsIRSignedComparison(uiOpcode);
	bSwap = IsIRSwappedComparison(uiOpcode);
	bWide = IsWide(psInstruction->sType);
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];

	if (bWide || (IsNarrow(psInstruction->sType) && (bSigned || (NarrowOnly(sLeft) && NarrowOnly(sRight)))))
	{
		//Signed i8 and i16 are compared at their own width (the registers narrowed), which needs no extending; so
		//are unsigned ones of values only ever compared, whose extending then goes (CIR65Widths).
		iBits = bWide ? 64 : psInstruction->sType.IntegerBits();
		if (!bWide && (iBits != 1) && InRegister(sLeft) && InRegister(sRight) && (PlaceOf(sLeft)->iOffset != PlaceOf(sRight)->iOffset))
		{
			//Both kept in X and Y: those are narrowed instead.
			Line("wtth (X, Y), ")->Append(iBits)->AppendNewLine();
			Line("cmp ")->Append(RegisterName(PlaceOf(bSwap ? sRight : sLeft)))->Append(", ")->Append(RegisterName(PlaceOf(bSwap ? sLeft : sRight)))->AppendNewLine();
			Line("wtth (X, Y), 32")->AppendNewLine();
		}
		else if (!bWide && (iBits != 1) && !bSwap && InRegister(sLeft) && Immediate(sRight, psInstruction->sType))
		{
			//Kept in X or Y and compared with a constant: the register is narrowed.
			Line("wth ")->Append(RegisterName(PlaceOf(sLeft)))->Append(", ")->Append(iBits)->AppendNewLine();
			Line("cmp ")->Append(RegisterName(PlaceOf(sLeft)))->Append(", ");
			AppendImmediate(mpszOut, sRight, bSigned ? IR65E_Sign : IR65E_Zero);
			mpszOut->AppendNewLine();
			Line("wth ")->Append(RegisterName(PlaceOf(sLeft)))->Append(", 32")->AppendNewLine();
		}
		else
		{
			if (iBits == 1)
			{
				LoadOperands(psInstruction, IR65E_Sign, !bSwap);
				iBits = 32;
			}
			else
			{
				LoadOperands(psInstruction, IR65E_Zero, !bSwap);
			}
			if (iBits != 32)
			{
				Line("wtth (A, B), ")->Append(iBits)->AppendNewLine();
			}
			if (mbRightImmediate)
			{
				Line("cmp A, ");
				AppendImmediate(mpszOut, sRight, bSigned ? IR65E_Sign : IR65E_Zero);
				mpszOut->AppendNewLine();
			}
			else
			{
				Line(bSwap ? "cmp B, A" : "cmp A, B")->AppendNewLine();
			}
			if (iBits != 32)
			{
				Line("wtth (A, B), 32")->AppendNewLine();
			}
		}
	}
	else if (bSwap)
	{
		CompareOperands(sRight, sLeft);
	}
	else
	{
		CompareOperands(sLeft, sRight);
	}

	if ((uiOpcode == IRO_Eq) || (uiOpcode == IRO_Ne))
	{
		Branch(((uiOpcode == IRO_Eq) == bJumpIf) ? "bras z, " : "brac z, ", iLabel);
		return true;
	}

	//slt, ult and the swapped sgt, ugt are less; sge, uge and the swapped sle, ule are greater or equal.
	bLess = ((uiOpcode == IRO_SLt) || (uiOpcode == IRO_ULt) || (uiOpcode == IRO_SGt) || (uiOpcode == IRO_UGt)) == bJumpIf;
	if (!bSigned)
	{
		Branch(bLess ? "brac c, " : "bras c, ", iLabel);
	}
	else if (!bSwap && IsZero(sRight))
	{
		//cmp x, 0 can't overflow, so n alone is x < 0 (x > -1 is x >= 0 by AdjustComparisons; x > 0 is x >= 1).
		Branch(bLess ? "bras n, " : "brac n, ", iLabel);
	}
	else if (bLess)
	{
		Branch("braso (n, v), ", iLabel);
	}
	else
	{
		iSkip = NewLabel();
		Branch("braso (n, v), ", iSkip);
		Branch("bra ", iLabel);
		Label(iSkip);
	}
	return true;
}
