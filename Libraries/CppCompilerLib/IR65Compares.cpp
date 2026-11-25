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
#include "BaseLib/StringHelper.h"
#include "IR65BackendInternal.h"


//The 65C64 back end's compares: integer comparisons as a value or a branch, and their operands.


//////////////////////////////////////////////////////////////////////////
//
// Branches to iLabel when the comparison is bJumpIf.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::Compare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
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
void CIR65Selector::CompareOperands(SIRReference sFirst, SIRReference sSecond)
{
	CChars	szLeft;
	CChars	szRight;
	bool	bSmall;

	szLeft.Init();
	szRight.Init();
	bSmall = mpcBackend->mcPlaces.IsInA(sSecond) || mpcBackend->mcPlaces.InRegister(sSecond) || SmallImmediate(sSecond, -8, 8);

	if (mpcBackend->mcPlaces.IsInA(sFirst))
	{
		szLeft.Append('A');
	}
	else if (mpcBackend->mcPlaces.InRegister(sFirst))
	{
		szLeft.Append(mpcBackend->mcPlaces.RegisterName(mpcBackend->mcPlaces.PlaceOf(sFirst)));
	}
	else if (bSmall && sFirst.IsTemporary() && (mpcBackend->mcPlaces.PlaceOf(sFirst)->uiKind == IR65P_Pseudo))
	{
		//Memory goes with a register or -8 to 8.
		AppendPlace(&szLeft, mpcBackend->mcPlaces.PlaceOf(sFirst));
	}
	else if (mpcBackend->mcPlaces.IsInA(sSecond))
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
	mpcBackend->Line(IR65M_Cmp)->Append(szLeft)->Append(", ")->Append(szRight);
	szRight.Kill();
	szLeft.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue, not in A, can be cmp's second operand as memory (its
// pseudo register, its place in the frame, or a deferred load), which a
// narrowed A reads at its own width.  pszOperand (empty) gets the operand
// if so.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::NarrowMemoryOperand(SIRReference sValue, CChars* pszOperand)
{
	size	uiMark;

	if (!sValue.IsTemporary() || mpcBackend->mcPlaces.IsInA(sValue))
	{
		return false;
	}
	uiMark = mpcBackend->CodeLines();
	if (Operand(sValue, pszOperand, true, IR65E_Zero) && (pszOperand->Length() > 0) && (pszOperand->Text()[0] == '['))
	{
		return true;
	}
	mpcBackend->CutCode(uiMark);
	pszOperand->Clear();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// After cmp X, Y: z is X = Y, c clear is X < Y unsigned and n != v is X < Y
// signed.  > and <= swap the operands (with a constant they were made >=
// and < by AdjustComparisons).  There is no branch on n = v, so it branches
// round a bra.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::IntegerCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
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
	CChars			szOperand;

	uiOpcode = psInstruction->uiOpcode;
	if (mpcBackend->mbFlagsKnown)
	{
		//The block before compared the same operands.
		mpcBackend->Branch(((uiOpcode == IRO_Eq) == bJumpIf) ? IR65M_Bras : IR65M_Brac, "z, ", iLabel);
		return true;
	}
	szOperand.Init();
	bSigned = IsIRSignedComparison(uiOpcode);
	bSwap = IsIRSwappedComparison(uiOpcode);
	bWide = mpcBackend->IsWide(psInstruction->sType);
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];

	if (bWide || (mpcBackend->IsNarrow(psInstruction->sType) && (bSigned || (mpcBackend->mcPlaces.NarrowOnly(sLeft) && mpcBackend->mcPlaces.NarrowOnly(sRight)))))
	{
		//Signed i8 and i16 are compared at their own width (the registers narrowed), which needs no extending; so
		//are unsigned ones of values only ever compared, whose extending then goes (CIR65Widths).
		iBits = bWide ? 64 : psInstruction->sType.IntegerBits();
		if (!bWide && (iBits != 1) && mpcBackend->mcPlaces.InRegister(sLeft) && mpcBackend->mcPlaces.InRegister(sRight) && (mpcBackend->mcPlaces.PlaceOf(sLeft)->iOffset != mpcBackend->mcPlaces.PlaceOf(sRight)->iOffset))
		{
			//Both kept in X and Y: those are narrowed instead.
			mpcBackend->Line(IR65M_Wtth, "(X, Y), ")->Append(iBits);
			mpcBackend->Line(IR65M_Cmp)->Append(mpcBackend->mcPlaces.RegisterName(mpcBackend->mcPlaces.PlaceOf(bSwap ? sRight : sLeft)))->Append(", ")->Append(mpcBackend->mcPlaces.RegisterName(mpcBackend->mcPlaces.PlaceOf(bSwap ? sLeft : sRight)));
			mpcBackend->Line(IR65M_Wtth, "(X, Y), 32");
		}
		else if (!bWide && (iBits != 1) && !bSwap && mpcBackend->mcPlaces.InRegister(sLeft) && Immediate(sRight, psInstruction->sType))
		{
			//Kept in X or Y and compared with a constant: the register is narrowed.
			mpcBackend->Line(IR65M_Wth)->Append(mpcBackend->mcPlaces.RegisterName(mpcBackend->mcPlaces.PlaceOf(sLeft)))->Append(", ")->Append(iBits);
			AppendImmediate(mpcBackend->Line(IR65M_Cmp)->Append(mpcBackend->mcPlaces.RegisterName(mpcBackend->mcPlaces.PlaceOf(sLeft)))->Append(", "), sRight, bSigned ? IR65E_Sign : IR65E_Zero);
			mpcBackend->Line(IR65M_Wth)->Append(mpcBackend->mcPlaces.RegisterName(mpcBackend->mcPlaces.PlaceOf(sLeft)))->Append(", 32");
		}
		else if (!bWide && (iBits != 1) && NarrowMemoryOperand(bSwap ? sLeft : sRight, &szOperand))
		{
			//The second in memory: read at the width A is narrowed to (its low bytes), without B.
			LoadValue(bSwap ? sRight : sLeft, "A", IR65E_Zero);
			mpcBackend->Line(IR65M_Wth, "A, ")->Append(iBits);
			mpcBackend->Line(IR65M_Cmp, "A, ")->Append(szOperand);
			mpcBackend->Line(IR65M_Wth, "A, 32");
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
				mpcBackend->Line(IR65M_Wtth, "(A, B), ")->Append(iBits);
			}
			if (mpcBackend->mbRightImmediate)
			{
				AppendImmediate(mpcBackend->Line(IR65M_Cmp, "A, "), sRight, bSigned ? IR65E_Sign : IR65E_Zero);
			}
			else
			{
				mpcBackend->Line(IR65M_Cmp, bSwap ? "B, A" : "A, B");
			}
			if (iBits != 32)
			{
				mpcBackend->Line(IR65M_Wtth, "(A, B), 32");
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
	szOperand.Kill();

	if ((uiOpcode == IRO_Eq) || (uiOpcode == IRO_Ne))
	{
		mpcBackend->Branch(((uiOpcode == IRO_Eq) == bJumpIf) ? IR65M_Bras : IR65M_Brac, "z, ", iLabel);
		return true;
	}

	//slt, ult and the swapped sgt, ugt are less; sge, uge and the swapped sle, ule are greater or equal.
	bLess = ((uiOpcode == IRO_SLt) || (uiOpcode == IRO_ULt) || (uiOpcode == IRO_SGt) || (uiOpcode == IRO_UGt)) == bJumpIf;
	if (!bSigned)
	{
		mpcBackend->Branch(bLess ? IR65M_Brac : IR65M_Bras, "c, ", iLabel);
	}
	else if (!bSwap && mpcBackend->IsZero(sRight))
	{
		//cmp x, 0 can't overflow, so n alone is x < 0 (x > -1 is x >= 0 by AdjustComparisons; x > 0 is x >= 1).
		mpcBackend->Branch(bLess ? IR65M_Bras : IR65M_Brac, "n, ", iLabel);
	}
	else if (bLess)
	{
		mpcBackend->Branch(IR65M_Braso, "(n, v), ", iLabel);
	}
	else
	{
		iSkip = mpcBackend->NewLabel();
		mpcBackend->Branch(IR65M_Braso, "(n, v), ", iSkip);
		mpcBackend->Branch(IR65M_Bra, NULL, iLabel);
		mpcBackend->Label(iSkip);
	}
	return true;
}
