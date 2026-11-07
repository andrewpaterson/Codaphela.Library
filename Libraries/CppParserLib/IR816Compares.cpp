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
#include "IR816BackendInternal.h"


//The W65C816 back end's compares: the flags of a comparison, and its value or a branch fused with it.


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
// Whether the flags a block is entered with already answer its fused eq or
// ne (FusedBranch skips CompareFlags): the block has no phis (nothing is
// copied on the way in) and nothing but the comparison, and its only
// predecessor branched on a fused comparison of the same operands, in
// either order, that leaves Z set when they are equal (LeavesEqualityInZ).
// The branches between change no flags.  As the 65C64's FlagsKnown.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::FlagsKnown(CIRBlock* pcBlock)
{
	SIRInstruction*	psCompare;
	SIRInstruction*	psBefore;
	CIRBlock*		pcPredecessor;

	if ((pcBlock->masPhis.NumElements() != 0) || (pcBlock->masInstructions.NumElements() != 1) || (pcBlock->mapcPredecessors.NumElements() != 1) ||
		!IsFusedCompare(pcBlock, 0))
	{
		return false;
	}
	psCompare = pcBlock->masInstructions.Get(0);
	if ((psCompare->uiOpcode != IRO_Eq) && (psCompare->uiOpcode != IRO_Ne))
	{
		return false;
	}
	pcPredecessor = pcBlock->mapcPredecessors.GetPtr(0);
	if ((pcPredecessor == pcBlock) || (pcPredecessor->masInstructions.NumElements() == 0) ||
		!IsFusedCompare(pcPredecessor, pcPredecessor->masInstructions.NumElements() - 1))
	{
		return false;
	}
	psBefore = pcPredecessor->masInstructions.Tail();
	return psBefore->sType.Equals(psCompare->sType) && LeavesEqualityInZ(psBefore) && SameOperands(psBefore, psCompare);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the code FusedBranch writes for a comparison leaves Z set exactly
// when its operands are equal: CompareFlags does (high words first, each
// cmp only reached while the ones before were equal; a signed one flips
// both sign bits), as does an lda of a value against 0.  Not a 16 bit
// signed comparison of two values (SignedLessFlags): its eor 0x8000 after
// an overflow can leave 0.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::LeavesEqualityInZ(SIRInstruction* psInstruction)
{
	return !(IsIRSignedComparison(psInstruction->uiOpcode) && psInstruction->sType.Is(IRTK_I16) && !psInstruction->asArguments[1].IsConstant());
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

	psInstruction = BranchComparison(pcBlock);
	if ((psInstruction == NULL) || (uiIndex + 1 != pcBlock->masInstructions.NumElements()))
	{
		return false;
	}
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
	return (psInstruction->sResult.Index() < maiUses.NumElements()) && (*maiUses.Get(psInstruction->sResult.Index()) == 1);
}


//////////////////////////////////////////////////////////////////////////
//
// A branch to iLabel when a comparison is bWhen: the flags, then the
// branches on them (CompareFlags leaves C and Z as an unsigned compare
// leaves them).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FusedBranch(SIRInstruction* psInstruction, bool bWhen, int32 iLabel, bool bFlagsKnown)
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
	if (!bFlagsKnown)
	{
		CompareFlags(psInstruction->asArguments[0], psInstruction->asArguments[1], psInstruction->sType, IsIRSignedComparison(psInstruction->uiOpcode));
	}
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
	sz.Init(CIR816Backend::GetScratch());
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
