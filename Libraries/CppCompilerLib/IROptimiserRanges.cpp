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
#include <string.h>
#include "IROptimiserInternal.h"


//The optimiser's range passes: what is known of a value's bounds, sign and nullness, and the narrowing and folded tests
//it allows.


//////////////////////////////////////////////////////////////////////////
//
// The narrow value sValue is sext or zext (uiOpcode) of, or a constant
// that is such an extension of a narrow constant of *psType; IRNone if
// not.  *psType is set from the first extension found.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::Unextended(SIRReference sValue, uint16 uiOpcode, SIRType* psType)
{
	SIRInstruction*	psDefinition;

	psDefinition = mcDefinitions.Instruction(sValue);
	if (psDefinition)
	{
		if ((psDefinition->uiOpcode == uiOpcode) && (psType->IsNone() || mpcFunction->TypeOf(psDefinition->asArguments[0]).Equals(*psType)))
		{
			*psType = mpcFunction->TypeOf(psDefinition->asArguments[0]);
			return psDefinition->asArguments[0];
		}
		return IRNone();
	}
	if (sValue.IsConstant() && !psType->IsNone() && Extended(sValue, psType->IntegerBits(), uiOpcode == IRO_SExt, 0))
	{
		return mpcFunction->AddConstant(*psType, IRTruncate(*psType, mpcFunction->GetConstant(sValue)->ulliBits));
	}
	return IRNone();
}


//////////////////////////////////////////////////////////////////////////
//
// A comparison of two values sign extended (or zero extended) from one
// narrow type, or of one and a constant that type holds, compares the
// narrow values (signed comparisons of zero extended values become
// unsigned).  The extensions are then often dead.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowComparison(SIRInstruction* psInstruction)
{
	uint16			uiExtension;
	SIRType			sNarrow;
	SIRReference	sLeft;
	SIRReference	sRight;

	if (!IsIRIntegerComparison(psInstruction->uiOpcode))
	{
		return false;
	}
	for (uiExtension = IRO_SExt; ; uiExtension = IRO_ZExt)
	{
		//The type comes from whichever side is an extension (a constant can't give it).
		sNarrow = IRType(IRTK_None);
		sLeft = Unextended(psInstruction->asArguments[0], uiExtension, &sNarrow);
		sRight = Unextended(psInstruction->asArguments[1], uiExtension, &sNarrow);
		if (sLeft.IsNone() && !sRight.IsNone())
		{
			sLeft = Unextended(psInstruction->asArguments[0], uiExtension, &sNarrow);
		}
		if (!sLeft.IsNone() && !sRight.IsNone())
		{
			break;
		}
		if (uiExtension == IRO_ZExt)
		{
			ComparisonRange(psInstruction);
			return false;
		}
	}

	psInstruction->asArguments[0] = sLeft;
	psInstruction->asArguments[1] = sRight;
	psInstruction->sType = sNarrow;
	if ((uiExtension == IRO_ZExt) && IsIRSignedComparison(psInstruction->uiOpcode))
	{
		psInstruction->uiOpcode += IRO_ULt - IRO_SLt;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Describes (SetWarnings) a comparison of a value zero extended from a narrow
// type with a constant that type can't hold: its result never changes.  A
// 16 bit "not found" (MAX_SIZE) widened and compared with the 32 bit one
// (MAX_LSIZE) is never equal.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::ComparisonRange(SIRInstruction* psInstruction)
{
	SIRType		sNarrow;
	int			iConstant;
	uint64		ulliConstant;
	bool		bTrue;
	CChars		szLine;

	if (!mpszWarnings)
	{
		return;
	}
	for (iConstant = 0; iConstant < 2; iConstant++)
	{
		sNarrow = IRType(IRTK_None);
		if (psInstruction->asArguments[iConstant].IsConstant() && !Unextended(psInstruction->asArguments[1 - iConstant], IRO_ZExt, &sNarrow).IsNone() &&
			!Extended(psInstruction->asArguments[iConstant], sNarrow.IntegerBits(), false, 0))
		{
			break;
		}
	}
	if (iConstant == 2)
	{
		return;
	}

	//Out of the narrow type's range: bigger than all its values (a negative constant is too, as unsigned).
	ulliConstant = mpcFunction->GetConstant(psInstruction->asArguments[iConstant])->ulliBits;
	switch (psInstruction->uiOpcode)
	{
	case IRO_Eq:	bTrue = false;	break;
	case IRO_Ne:	bTrue = true;	break;
	case IRO_ULt:
	case IRO_ULe:	bTrue = (iConstant == 1);	break;
	case IRO_UGt:
	case IRO_UGe:	bTrue = (iConstant == 0);	break;
	default:		return;
	}

	szLine.Init("Comparison is always ");
	szLine.Append(bTrue ? "true" : "false")->Append(": a ")->Append(sNarrow.IntegerBits())->Append(" bit value, zero extended, is compared with ");
	szLine.Append((uint64)IRTruncate(psInstruction->sType, ulliConstant))->Append(".\n");
	if (!strstr(mpszWarnings->Text(), szLine.Text()))
	{
		mpszWarnings->Append(szLine);
	}
	szLine.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// True if the low bits of uiOpcode's result depend only on the low bits of
// its operands (a shift left by a constant less than iBits too).
//
//////////////////////////////////////////////////////////////////////////
static bool IROptimiserNarrows(CIRFunction* pcFunction, SIRInstruction* psInstruction, int iBits)
{
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_Sub:
	case IRO_Mul:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
		return true;
	case IRO_Shl:
		return psInstruction->asArguments[1].IsConstant() && (pcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits < (uint64)iBits);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// sValue's low bits as a value of the narrower sType with no instruction: a
// constant truncated, or the value an extension from sType extended; IRNone
// when it takes one.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::FreeNarrow(SIRReference sValue, SIRType sType)
{
	SIRInstruction*	psDefinition;

	if (sValue.IsConstant())
	{
		return mpcFunction->AddConstant(sType, IRTruncate(sType, mpcFunction->GetConstant(sValue)->ulliBits));
	}
	psDefinition = mcDefinitions.Instruction(sValue);
	if (psDefinition && ((psDefinition->uiOpcode == IRO_SExt) || (psDefinition->uiOpcode == IRO_ZExt)) && mpcFunction->TypeOf(psDefinition->asArguments[0]).Equals(sType))
	{
		return psDefinition->asArguments[0];
	}
	return IRNone();
}


//////////////////////////////////////////////////////////////////////////
//
// sValue narrowed to sType, adding the instructions that takes to pasNew:
// free (FreeNarrow), the operation done narrow when only the trunc being
// narrowed uses it, or else a trunc.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::Narrowed(SIRReference sValue, SIRType sType, CArrayIRInstructions* pasNew, int iDepth)
{
	SIRInstruction*	psDefinition;
	SIRInstruction	sNew;
	SIRReference	sNarrow;

	sNarrow = FreeNarrow(sValue, sType);
	if (!sNarrow.IsNone())
	{
		return sNarrow;
	}
	psDefinition = mcDefinitions.Instruction(sValue);
	memset(&sNew, 0, sizeof(SIRInstruction));
	if (psDefinition && (iDepth < IR_NARROW_DEPTH) && (mcDefinitions.NumUses(sValue) == 1) && IROptimiserNarrows(mpcFunction, psDefinition, sType.IntegerBits()))
	{
		sNew = *psDefinition;
		sNew.asArguments[0] = Narrowed(sNew.asArguments[0], sType, pasNew, iDepth + 1);
		sNew.asArguments[1] = Narrowed(sNew.asArguments[1], sType, pasNew, iDepth + 1);
	}
	else
	{
		sNew.uiOpcode = IRO_Trunc;
		sNew.asArguments[0] = sValue;
		sNew.asArguments[1] = IRNone();
	}
	sNew.sType = sType;
	sNew.sResult = mpcFunction->AddTemporary(sType);
	pasNew->Add(&sNew);
	return sNew.sResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The most sValue can be, unsigned: a constant; the narrow type's most for a
// zext; the smaller of an and's; x >> c, x % c and x / c of x's; the sum of
// an add's that can't wrap; the most of a phi's arguments, each as the
// branch to the phi bounds it (EdgeBound) or as it is.  Else all ones of its
// type.  mcDefinitions must be found.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIROptimiser::UpperBound(SIRReference sValue, int iDepth)
{
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	CIRBlock*		pcBlock;
	SIRType			sType;
	uint64			ulliMost;
	uint64			ulliFirst;
	uint64			ulliSecond;
	uint64			ulliConstant;
	size			ui;

	sType = mpcFunction->TypeOf(sValue);
	if (!sType.IsInteger())
	{
		return (uint64)-1;
	}
	ulliMost = IRTruncate(sType, (uint64)-1);
	if (sValue.IsConstant())
	{
		return IRTruncate(sType, mpcFunction->GetConstant(sValue)->ulliBits);
	}
	if (!sValue.IsTemporary() || (iDepth == 0))
	{
		return ulliMost;
	}

	psInstruction = mcDefinitions.Instruction(sValue);
	if (psInstruction)
	{
		ulliConstant = psInstruction->asArguments[1].IsConstant() ? IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) : 0;
		switch (psInstruction->uiOpcode)
		{
		case IRO_ZExt:
			return IRTruncate(mpcFunction->TypeOf(psInstruction->asArguments[0]), UpperBound(psInstruction->asArguments[0], iDepth - 1));
		case IRO_And:
			ulliFirst = UpperBound(psInstruction->asArguments[0], iDepth - 1);
			ulliSecond = UpperBound(psInstruction->asArguments[1], iDepth - 1);
			return (ulliFirst < ulliSecond) ? ulliFirst : ulliSecond;
		case IRO_LShr:
			if (psInstruction->asArguments[1].IsConstant() && (ulliConstant < 64))
			{
				return UpperBound(psInstruction->asArguments[0], iDepth - 1) >> ulliConstant;
			}
			return ulliMost;
		case IRO_URem:
			if (psInstruction->asArguments[1].IsConstant() && (ulliConstant != 0))
			{
				ulliFirst = UpperBound(psInstruction->asArguments[0], iDepth - 1);
				return (ulliFirst < ulliConstant - 1) ? ulliFirst : ulliConstant - 1;
			}
			return ulliMost;
		case IRO_UDiv:
			if (psInstruction->asArguments[1].IsConstant() && (ulliConstant != 0))
			{
				return UpperBound(psInstruction->asArguments[0], iDepth - 1) / ulliConstant;
			}
			return ulliMost;
		case IRO_Add:
			ulliFirst = UpperBound(psInstruction->asArguments[0], iDepth - 1);
			ulliSecond = UpperBound(psInstruction->asArguments[1], iDepth - 1);
			if ((ulliFirst <= ulliMost - ulliSecond) && (ulliSecond <= ulliMost))
			{
				return ulliFirst + ulliSecond;
			}
			return ulliMost;
		default:
			return ulliMost;
		}
	}

	psPhi = mcDefinitions.Phi(sValue);
	if (psPhi)
	{
		pcBlock = mcDefinitions.Block(sValue);
		ulliFirst = 0;
		for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
		{
			psArgument = psPhi->asArguments.Get(ui);
			ulliSecond = EdgeBound(psArgument->pcBlock, pcBlock, psArgument->sValue);
			if (ulliSecond == (uint64)-1)
			{
				ulliSecond = UpperBound(psArgument->sValue, iDepth - 1);
			}
			if (ulliSecond > ulliFirst)
			{
				ulliFirst = ulliSecond;
			}
		}
		return (ulliFirst < ulliMost) ? ulliFirst : ulliMost;
	}
	return ulliMost;
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction pcFrom's branch to pcTo is on, when it reads sValue, and
// what it says of sValue that way: *puiOpcode the compare with sValue on
// the left (n > v is v < n; inverted on the way the branch takes when it is
// false) and *psOther the other operand.  NULL when pcFrom doesn't branch
// two ways to pcTo on an integer comparison reading sValue (a branch on
// anything else, such as xor i1 %x, 1, says nothing this way).
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIROptimiser::EdgeComparison(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue, uint16* puiOpcode, SIRReference* psOther)
{
	SIRInstruction*	psCompare;

	if ((pcFrom->msJump.uiKind != IRJ_Branch) || (pcFrom->msJump.pcThen == pcFrom->msJump.pcElse))
	{
		return NULL;
	}
	psCompare = mcDefinitions.Instruction(pcFrom->msJump.sArgument);
	if (!psCompare || !IsIRIntegerComparison(psCompare->uiOpcode))
	{
		return NULL;
	}
	*puiOpcode = psCompare->uiOpcode;
	if (psCompare->asArguments[0].Equals(sValue))
	{
		*psOther = psCompare->asArguments[1];
	}
	else if (psCompare->asArguments[1].Equals(sValue))
	{
		*psOther = psCompare->asArguments[0];
		*puiOpcode = IRSwappedComparison(*puiOpcode);
	}
	else
	{
		return NULL;
	}
	if (pcFrom->msJump.pcElse == pcTo)
	{
		*puiOpcode = IRInvertedComparison(*puiOpcode);
	}
	else if (pcFrom->msJump.pcThen != pcTo)
	{
		return NULL;
	}
	return psCompare;
}


//////////////////////////////////////////////////////////////////////////
//
// The most sValue can be going from pcFrom to pcTo when pcFrom branches
// there only if an unsigned comparison of sValue with a constant says so
// (v < n, v <= n, or not v >= n and the like), else all ones.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIROptimiser::EdgeBound(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue)
{
	SIRInstruction*	psCompare;
	SIRReference	sConstant;
	uint16			uiOpcode;
	uint64			ulliConstant;

	psCompare = EdgeComparison(pcFrom, pcTo, sValue, &uiOpcode, &sConstant);
	if (!psCompare || !sConstant.IsConstant())
	{
		return (uint64)-1;
	}
	ulliConstant = IRTruncate(psCompare->sType, mpcFunction->GetConstant(sConstant)->ulliBits);
	if ((uiOpcode == IRO_ULt) && (ulliConstant != 0))
	{
		return ulliConstant - 1;
	}
	if (uiOpcode == IRO_ULe)
	{
		return ulliConstant;
	}
	return (uint64)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// The signed range of sValue as an integer of its type: a constant's; a
// sext's of the narrower value's; x + c's, x - c's and c - x's moved by c
// (when it can't wrap); a divide's or remainder's by a constant
// (DivisionRange); x + y's, x - y's, x * c's and shifts' by a constant
// (ArithmeticRange); a phi's of all its values (up to iDepth definitions
// back); else from 0 to UpperBound when that is below the type's sign bit
// (a zext, an and).  With mbBranchRanges an operand's range is narrowed by
// the branches to the instruction's block (RangeAt) and a phi's values by
// the branches to the phi on their way.  False if nothing narrower than the
// type is known.  mcDefinitions must be found.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SignedRange(SIRReference sValue, int64* plliMin, int64* plliMax, int iDepth)
{
	SIRInstruction*	psInstruction;
	SIRType			sType;
	SIRType			sNarrow;
	int				iBits;
	uint64			ulliBound;
	int64			lliConstant;
	int64			lliArgumentMin;
	int64			lliArgumentMax;
	SIRPhi*			psPhi;
	size			ui;

	sType = mpcFunction->TypeOf(sValue);
	if (!sType.IsInteger() || (sType.IntegerBits() > 32))
	{
		return false;
	}
	iBits = sType.IntegerBits();
	if (sValue.IsConstant())
	{
		*plliMin = IRSignExtend(sType, mpcFunction->GetConstant(sValue)->ulliBits);
		*plliMax = *plliMin;
		return true;
	}
	psInstruction = sValue.IsTemporary() ? mcDefinitions.Instruction(sValue) : NULL;
	if (psInstruction && (psInstruction->uiOpcode == IRO_Add) && psInstruction->asArguments[1].IsConstant())
	{
		//x + c: x's range moved, when it can't wrap.  ArithmeticRange's would be the same, but one that may wrap isn't
		//then the UpperBound below (x - c and c - x are ArithmeticRange's: UpperBound knows nothing of a sub).
		lliConstant = IRSignExtend(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
		if ((iDepth > 0) && RangeAt(mcDefinitions.Block(sValue), psInstruction->asArguments[0], plliMin, plliMax, iDepth - 1))
		{
			*plliMin += lliConstant;
			*plliMax += lliConstant;
			return (*plliMin >= IRSignedMin(iBits)) && (*plliMax <= IRSignedMax(iBits));
		}
	}
	if (psInstruction && (iDepth > 0) && DivisionRange(psInstruction, plliMin, plliMax, iDepth - 1))
	{
		return true;
	}
	if (psInstruction && (iDepth > 0) && ArithmeticRange(psInstruction, plliMin, plliMax, iDepth - 1))
	{
		return true;
	}
	psPhi = sValue.IsTemporary() ? mcDefinitions.Phi(sValue) : NULL;
	if (psPhi && (iDepth > 0))
	{
		//A phi: the range of all its values (each narrowed by the branches on its way).
		for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
		{
			if (!RangeAt(psPhi->asArguments.Get(ui)->pcBlock, psPhi->asArguments.Get(ui)->sValue, &lliArgumentMin, &lliArgumentMax, iDepth - 1, mcDefinitions.Block(sValue)))
			{
				return false;
			}
			*plliMin = ((ui == 0) || (lliArgumentMin < *plliMin)) ? lliArgumentMin : *plliMin;
			*plliMax = ((ui == 0) || (lliArgumentMax > *plliMax)) ? lliArgumentMax : *plliMax;
		}
		return psPhi->asArguments.NumElements() > 0;
	}
	if (psInstruction && (psInstruction->uiOpcode == IRO_SExt))
	{
		sNarrow = mpcFunction->TypeOf(psInstruction->asArguments[0]);
		if (sNarrow.IsInteger() && (sNarrow.IntegerBits() < iBits))
		{
			if ((iDepth == 0) || !SignedRange(psInstruction->asArguments[0], plliMin, plliMax, iDepth - 1))
			{
				*plliMin = IRSignedMin(sNarrow.IntegerBits());
				*plliMax = IRSignedMax(sNarrow.IntegerBits());
			}
			return true;
		}
	}
	ulliBound = UpperBound(sValue, IR_BOUND_DEPTH);
	if (ulliBound <= (uint64)IRSignedMax(iBits))
	{
		*plliMin = 0;
		*plliMax = (int64)ulliBound;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The signed range of sValue where pcBlock runs (on its way to pcTo when
// pcTo isn't NULL): SignedRange's, or the type's when that isn't known,
// narrowed by the branches there (BranchRange, DominatingRange) when
// mbBranchRanges.  False if nothing narrower than the type is known.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RangeAt(CIRBlock* pcBlock, SIRReference sValue, int64* plliMin, int64* plliMax, int iDepth, CIRBlock* pcTo)
{
	SIRType	sType;
	int64	lliLeast;
	int64	lliMost;
	bool	bKnown;

	bKnown = SignedRange(sValue, plliMin, plliMax, iDepth);
	if (!mbBranchRanges || !pcBlock)
	{
		return bKnown;
	}
	sType = mpcFunction->TypeOf(sValue);
	if (!sType.IsInteger() || (sType.IntegerBits() > 32))
	{
		return bKnown;
	}
	lliLeast = IRSignedMin(sType.IntegerBits());
	lliMost = IRSignedMax(sType.IntegerBits());
	if (!bKnown)
	{
		*plliMin = lliLeast;
		*plliMax = lliMost;
	}
	if (pcTo)
	{
		BranchRange(pcBlock, pcTo, sValue, plliMin, plliMax);
	}
	DominatingRange(pcBlock, sValue, plliMin, plliMax);
	if (*plliMin > *plliMax)
	{
		//No value gets here (the branches contradict): any range will do.
		*plliMax = *plliMin;
	}
	return bKnown || (*plliMin > lliLeast) || (*plliMax < lliMost);
}


//////////////////////////////////////////////////////////////////////////
//
// The signed range of x + y, x - y (both ranges known), x * c, x << c,
// x >> c (arithmetic, or logical of an x never negative; of any x when its
// range isn't known) and a trunc of x that fits the narrower type, when it
// can't wrap.  False for anything else.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ArithmeticRange(SIRInstruction* psInstruction, int64* plliMin, int64* plliMax, int iDepth)
{
	CIRBlock*	pcBlock;
	SIRType		sType;
	uint16		uiOpcode;
	int64		lliMin;
	int64		lliMax;
	int64		lliOtherMin;
	int64		lliOtherMax;
	int64		lliConstant;
	int			iBits;

	uiOpcode = psInstruction->uiOpcode;
	sType = psInstruction->sType;
	if (((uiOpcode != IRO_Add) && (uiOpcode != IRO_Sub) && (uiOpcode != IRO_Mul) && (uiOpcode != IRO_Shl) && (uiOpcode != IRO_AShr) &&
		 (uiOpcode != IRO_LShr) && (uiOpcode != IRO_Trunc)) || !sType.IsInteger() || (sType.IntegerBits() > 32) || !psInstruction->sResult.IsTemporary())
	{
		return false;
	}
	iBits = sType.IntegerBits();
	pcBlock = mcDefinitions.Block(psInstruction->sResult);
	if (!RangeAt(pcBlock, psInstruction->asArguments[0], &lliMin, &lliMax, iDepth))
	{
		//A shift right narrows any value: x is any of its type's (as signed for ashr, unsigned for lshr).
		if (((uiOpcode != IRO_AShr) && (uiOpcode != IRO_LShr)) || !psInstruction->asArguments[1].IsConstant())
		{
			return false;
		}
		lliMin = (uiOpcode == IRO_AShr) ? IRSignedMin(iBits) : 0;
		lliMax = (uiOpcode == IRO_AShr) ? IRSignedMax(iBits) : IRUnsignedMax(iBits);
	}
	if (uiOpcode == IRO_Trunc)
	{
		*plliMin = lliMin;
		*plliMax = lliMax;
	}
	else if ((uiOpcode == IRO_Add) || (uiOpcode == IRO_Sub))
	{
		if (!RangeAt(pcBlock, psInstruction->asArguments[1], &lliOtherMin, &lliOtherMax, iDepth))
		{
			return false;
		}
		*plliMin = (uiOpcode == IRO_Add) ? lliMin + lliOtherMin : lliMin - lliOtherMax;
		*plliMax = (uiOpcode == IRO_Add) ? lliMax + lliOtherMax : lliMax - lliOtherMin;
	}
	else
	{
		if (!psInstruction->asArguments[1].IsConstant())
		{
			return false;
		}
		lliConstant = mpcFunction->GetSigned(psInstruction->asArguments[1]);
		if ((uiOpcode != IRO_Mul) && ((lliConstant < 0) || (lliConstant >= iBits)))
		{
			return false;
		}
		if (uiOpcode == IRO_Shl)
		{
			lliConstant = (int64)1 << lliConstant;
		}
		if ((uiOpcode == IRO_Mul) || (uiOpcode == IRO_Shl))
		{
			if ((lliConstant > 0xFFFFFFFFLL) || (lliConstant < -0xFFFFFFFFLL))
			{
				return false;
			}
			*plliMin = (lliConstant >= 0) ? lliMin * lliConstant : lliMax * lliConstant;
			*plliMax = (lliConstant >= 0) ? lliMax * lliConstant : lliMin * lliConstant;
		}
		else if ((uiOpcode == IRO_AShr) || (lliMin >= 0))
		{
			*plliMin = lliMin >> lliConstant;
			*plliMax = lliMax >> lliConstant;
		}
		else
		{
			return false;
		}
	}
	return (*plliMin >= IRSignedMin(iBits)) && (*plliMax <= IRSignedMax(iBits));
}


//////////////////////////////////////////////////////////////////////////
//
// The signed range of a divide or remainder by a constant c (at least 1):
// x / c's is x's range divided (the type's when x's isn't known); x % c's
// is within c - 1 of 0 on x's side of it, and x's own when that is nearer.
// An unsigned x that may be negative as signed is any of the type's
// unsigned values.  False for anything else.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::DivisionRange(SIRInstruction* psInstruction, int64* plliMin, int64* plliMax, int iDepth)
{
	SIRType	sType;
	uint16	uiOpcode;
	int64	lliDivisor;
	int64	lliMin;
	int64	lliMax;
	int		iBits;
	bool	bSigned;

	uiOpcode = psInstruction->uiOpcode;
	sType = psInstruction->sType;
	if (((uiOpcode != IRO_SDiv) && (uiOpcode != IRO_SRem) && (uiOpcode != IRO_UDiv) && (uiOpcode != IRO_URem)) ||
		!psInstruction->asArguments[1].IsConstant() || !sType.IsInteger() || (sType.IntegerBits() > 32))
	{
		return false;
	}
	iBits = sType.IntegerBits();
	bSigned = (uiOpcode == IRO_SDiv) || (uiOpcode == IRO_SRem);
	lliDivisor = bSigned ? IRSignExtend(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) :
						   (int64)IRTruncate(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
	if (lliDivisor < 2)
	{
		return false;
	}
	if (!RangeAt(mcDefinitions.Block(psInstruction->sResult), psInstruction->asArguments[0], &lliMin, &lliMax, iDepth))
	{
		lliMin = IRSignedMin(iBits);
		lliMax = IRSignedMax(iBits);
	}
	if (!bSigned && (lliMin < 0))
	{
		lliMin = 0;
		lliMax = IRUnsignedMax(iBits);
	}
	if ((uiOpcode == IRO_SDiv) || (uiOpcode == IRO_UDiv))
	{
		//Rounding towards 0 keeps the order.
		*plliMin = lliMin / lliDivisor;
		*plliMax = lliMax / lliDivisor;
	}
	else
	{
		*plliMin = (lliMin < 0) ? ((lliMin > -lliDivisor) ? lliMin : -(lliDivisor - 1)) : 0;
		*plliMax = (lliMax > 0) ? ((lliMax < lliDivisor) ? lliMax : lliDivisor - 1) : 0;
		if ((lliMin >= 0) && (lliMax < lliDivisor))
		{
			*plliMin = lliMin;
		}
		if ((lliMax <= 0) && (lliMin > -lliDivisor))
		{
			*plliMax = lliMax;
		}
	}
	return *plliMax <= IRSignedMax(iBits);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFrom's branch to pcTo is taken only when sValue isn't null
// (a brif on sValue != 0 to its then, or on sValue == 0 to its else).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::BranchNonNull(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue)
{
	SIRReference	sOther;
	uint16			uiOpcode;

	//Only an eq or ne is ne that way (IRInvertedComparison makes ne only of eq).
	return EdgeComparison(pcFrom, pcTo, sValue, &uiOpcode, &sOther) && (uiOpcode == IRO_Ne) && sOther.IsConstant() && (mpcFunction->GetConstant(sOther)->ulliBits == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the pointer sValue is never null where pcBlock runs: a global's
// address, a slot, a constant other than 0; tested not null on the way to
// pcBlock (while blocks have one predecessor); a pointer added to one that
// isn't null (C++'s pointer arithmetic stays in an object); a phi whose
// every value isn't null on its way in.  mcDefinitions and the
// predecessors must be found.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NonNullAt(CIRBlock* pcBlock, SIRReference sValue, int iDepth)
{
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	CIRBlock*		pcTo;
	CIRBlock*		pcDefinition;
	size			ui;
	int				i;

	if (sValue.IsSymbol())
	{
		return true;
	}
	if (sValue.IsConstant())
	{
		return mpcFunction->GetConstant(sValue)->ulliBits != 0;
	}
	if (!sValue.IsTemporary() || (iDepth <= 0))
	{
		return false;
	}
	pcTo = pcBlock;
	for (i = 0; pcTo && (i < IR_RANGE_DEPTH) && (pcTo->mapcPredecessors.NumElements() == 1); i++)
	{
		if (BranchNonNull(pcTo->mapcPredecessors.GetPtr(0), pcTo, sValue))
		{
			return true;
		}
		pcTo = pcTo->mapcPredecessors.GetPtr(0);
	}

	pcDefinition = mcDefinitions.Block(sValue);
	psInstruction = mcDefinitions.Instruction(sValue);
	if (psInstruction)
	{
		if (psInstruction->uiOpcode == IRO_Slot)
		{
			return true;
		}
		if ((psInstruction->uiOpcode == IRO_PtrAdd) || (psInstruction->uiOpcode == IRO_Copy))
		{
			return NonNullAt(pcDefinition, psInstruction->asArguments[0], iDepth - 1);
		}
		return false;
	}
	psPhi = mcDefinitions.Phi(sValue);
	if (!psPhi || (psPhi->asArguments.NumElements() == 0))
	{
		return false;
	}
	for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
	{
		psArgument = psPhi->asArguments.Get(ui);
		if (!BranchNonNull(psArgument->pcBlock, pcDefinition, psArgument->sValue) && !NonNullAt(psArgument->pcBlock, psArgument->sValue, iDepth - 1))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// p == 0 and p != 0 of a pointer that isn't null there (NonNullAt) are
// false and true.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::FoldNullTests(void)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sPointer;
	size			ui;
	size			uj;
	bool			bChanged;

	FindDefinitions();
	mpcFunction->ComputePredecessors();
	ClearReplacements();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if (((psInstruction->uiOpcode != IRO_Eq) && (psInstruction->uiOpcode != IRO_Ne)) || !psInstruction->sResult.IsTemporary())
			{
				continue;
			}
			if (psInstruction->asArguments[1].IsConstant() && (mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits == 0))
			{
				sPointer = psInstruction->asArguments[0];
			}
			else if (psInstruction->asArguments[0].IsConstant() && (mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits == 0))
			{
				sPointer = psInstruction->asArguments[1];
			}
			else
			{
				continue;
			}
			if (!sPointer.IsTemporary() || !mpcFunction->TypeOf(sPointer).IsPointer() || !NonNullAt(pcBlock, sPointer, IR_NON_NULL_DEPTH))
			{
				continue;
			}
			Replace(psInstruction->sResult, mpcFunction->AddConstant(mpcFunction->GetTemporaryType(psInstruction->sResult), (psInstruction->uiOpcode == IRO_Ne) ? 1 : 0));
			bChanged = true;
		}
	}
	if (bChanged)
	{
		ApplyReplacements();
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Narrows the signed range [*plliMin, *plliMax] of sValue in pcBlock by the
// branches pcBlock is reached through: while a block has one predecessor,
// that predecessor's branch on a compare of sValue with a constant holds
// (the way to the block), up to IR_RANGE_DEPTH blocks up.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::DominatingRange(CIRBlock* pcBlock, SIRReference sValue, int64* plliMin, int64* plliMax)
{
	CIRBlock*		pcTo;
	CIRBlock*		pcFrom;
	int				iDepth;

	pcTo = pcBlock;
	for (iDepth = 0; (iDepth < IR_RANGE_DEPTH) && (pcTo->mapcPredecessors.NumElements() == 1); iDepth++)
	{
		pcFrom = pcTo->mapcPredecessors.GetPtr(0);
		BranchRange(pcFrom, pcTo, sValue, plliMin, plliMax);
		pcTo = pcFrom;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Narrows the signed range [*plliMin, *plliMax] of sValue by pcFrom's
// branch to pcTo, when it is on a compare of sValue with a constant
// (EdgeComparison).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::BranchRange(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue, int64* plliMin, int64* plliMax)
{
	SIRReference	sConstant;
	uint16			uiOpcode;
	int64			lliConstant;

	if (!EdgeComparison(pcFrom, pcTo, sValue, &uiOpcode, &sConstant) || !sConstant.IsConstant())
	{
		return;
	}
	lliConstant = mpcFunction->GetSigned(sConstant);
	switch (uiOpcode)
	{
	case IRO_SLt:	*plliMax = (lliConstant - 1 < *plliMax) ? lliConstant - 1 : *plliMax;	break;
	case IRO_SLe:	*plliMax = (lliConstant < *plliMax) ? lliConstant : *plliMax;			break;
	case IRO_SGt:	*plliMin = (lliConstant + 1 > *plliMin) ? lliConstant + 1 : *plliMin;	break;
	case IRO_SGe:	*plliMin = (lliConstant > *plliMin) ? lliConstant : *plliMin;			break;
	case IRO_ULt:
	case IRO_ULe:
		//Below a constant that is positive as signed: 0 up to it.
		if (lliConstant > 0)
		{
			*plliMin = (*plliMin < 0) ? 0 : *plliMin;
			lliConstant -= (uiOpcode == IRO_ULt) ? 1 : 0;
			*plliMax = (lliConstant < *plliMax) ? lliConstant : *plliMax;
		}
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the branches pcBlock is reached through keep the integer sValue 0
// or more (DominatingRange).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NotNegative(CIRBlock* pcBlock, SIRReference sValue)
{
	int		iBits;
	int64	lliMin;
	int64	lliMax;

	iBits = mpcFunction->TypeOf(sValue).IntegerBits();
	if ((iBits < 2) || (iBits > 32) || sValue.IsConstant())
	{
		return false;
	}
	lliMin = IRSignedMin(iBits);
	lliMax = IRSignedMax(iBits);
	DominatingRange(pcBlock, sValue, &lliMin, &lliMax);
	return lliMin >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A mul of an extended i8 or i16 value by a constant whose product fits 16
// bits unsigned (the value 0 or more, its range from the branches the block
// is reached through, DominatingRange, or a zext's bits) is a 16 bit mul,
// zero extended: (long)y * 320 with y from 0 to 199 is a 16 bit y * 320.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowMultiplies(void)
{
	CArrayTemplate<SIRNarrowMultiply>	asFound;
	SIRNarrowMultiply*					psFound;
	SIRInstruction*						psInstruction;
	SIRInstruction*						psExtension;
	CIRBlock*							pcBlock;
	SIRReference						sNarrow;
	SIRType								sType;
	SIRType								sShort;
	uint64								ulliConstant;
	int64								lliMin;
	int64								lliMax;
	int									iBits;
	size								ui;
	size								uiInstruction;
	bool								bChanged;

	//Found first: the inserts move the definitions DominatingRange reads.
	FindDefinitions();
	mpcFunction->ComputePredecessors();
	sShort = IRType(IRTK_I16);
	asFound.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sType = psInstruction->sType;
			if ((psInstruction->uiOpcode != IRO_Mul) || !psInstruction->sResult.IsTemporary() || !sType.IsInteger() || (sType.IntegerBits() <= 16) ||
				!psInstruction->asArguments[1].IsConstant())
			{
				continue;
			}
			ulliConstant = IRTruncate(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
			psExtension = mcDefinitions.Instruction(psInstruction->asArguments[0]);
			if ((ulliConstant == 0) || (ulliConstant > 0xFFFF) || !psExtension || ((psExtension->uiOpcode != IRO_SExt) && (psExtension->uiOpcode != IRO_ZExt)))
			{
				continue;
			}
			sNarrow = psExtension->asArguments[0];
			iBits = mpcFunction->TypeOf(sNarrow).IntegerBits();
			if ((iBits != 8) && (iBits != 16))
			{
				continue;
			}
			//The compares are of the narrow value as it is (a signed compare sees it signed); a zext of one that may be
			//negative is up to all its bits.
			lliMin = IRSignedMin(iBits);
			lliMax = IRSignedMax(iBits);
			DominatingRange(pcBlock, sNarrow, &lliMin, &lliMax);
			if ((lliMin < 0) && (psExtension->uiOpcode == IRO_ZExt))
			{
				lliMin = 0;
				lliMax = IRUnsignedMax(iBits);
			}
			if ((lliMin < 0) || ((uint64)lliMax * ulliConstant > 0xFFFF))
			{
				continue;
			}
			psFound = asFound.Add();
			psFound->pcBlock = pcBlock;
			psFound->uiInstruction = uiInstruction;
			psFound->sNarrow = sNarrow;
		}
	}

	//Last first, so each instruction's index is still where it was found.
	bChanged = asFound.NumElements() > 0;
	for (ui = asFound.NumElements(); ui > 0; ui--)
	{
		psFound = asFound.Get(ui - 1);
		pcBlock = psFound->pcBlock;
		uiInstruction = psFound->uiInstruction;
		ulliConstant = mpcFunction->GetConstant(pcBlock->masInstructions.Get(uiInstruction)->asArguments[1])->ulliBits;
		sNarrow = psFound->sNarrow;
		if (mpcFunction->TypeOf(sNarrow).IntegerBits() == 8)
		{
			sNarrow = InsertInstruction(pcBlock, uiInstruction, IRO_ZExt, sShort, sNarrow, IRNone());
			uiInstruction++;
		}
		sNarrow = InsertInstruction(pcBlock, uiInstruction, IRO_Mul, sShort, sNarrow, mpcFunction->AddInteger(sShort, (int64)(ulliConstant & 0xFFFF)));
		psInstruction = pcBlock->masInstructions.Get(uiInstruction + 1);
		psInstruction->uiOpcode = IRO_ZExt;
		psInstruction->asArguments[0] = sNarrow;
		psInstruction->asArguments[1] = IRNone();
	}
	asFound.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A trunc of an add, sub, mul, and, or, xor or shift left by a constant
// that only it uses is the operation done narrow, when an operand of it is
// narrow for nothing (an extension or a constant: C's promotions of char
// and short arithmetic).  Operands are narrowed the same way, or truncated.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowTruncations(void)
{
	CArrayIRInstructions	asNew;
	SIRInstruction			sInstruction;
	SIRInstruction*			psWide;
	CIRBlock*				pcBlock;
	SIRType					sType;
	size					ui;
	size					uiInstruction;
	bool					bChanged;
	bool					bBlockChanged;

	FindDefinitions();
	asNew.Init();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		asNew.FakeSetUsedElements(0);
		bBlockChanged = false;
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			sInstruction = *pcBlock->masInstructions.Get(uiInstruction);
			sType = sInstruction.sType;
			psWide = (sInstruction.uiOpcode == IRO_Trunc) && sInstruction.sResult.IsTemporary() && sType.IsInteger() && !sType.Is(IRTK_I1) ? mcDefinitions.Instruction(sInstruction.asArguments[0]) : NULL;
			if (psWide && (mcDefinitions.NumUses(sInstruction.asArguments[0]) == 1) && IROptimiserNarrows(mpcFunction, psWide, sType.IntegerBits()) &&
				(!FreeNarrow(psWide->asArguments[0], sType).IsNone() || !FreeNarrow(psWide->asArguments[1], sType).IsNone()))
			{
				sInstruction.uiOpcode = psWide->uiOpcode;
				sInstruction.uiFlags = psWide->uiFlags;
				sInstruction.asArguments[0] = psWide->asArguments[0];
				sInstruction.asArguments[1] = psWide->asArguments[1];
				sInstruction.asArguments[0] = Narrowed(sInstruction.asArguments[0], sType, &asNew, 1);
				sInstruction.asArguments[1] = Narrowed(sInstruction.asArguments[1], sType, &asNew, 1);
				bBlockChanged = true;
			}
			asNew.Add(&sInstruction);
		}
		if (bBlockChanged)
		{
			pcBlock->masInstructions.FakeSetUsedElements(0);
			for (uiInstruction = 0; uiInstruction < asNew.NumElements(); uiInstruction++)
			{
				pcBlock->masInstructions.Add(asNew.Get(uiInstruction));
			}
			bChanged = true;
			FindDefinitions();
		}
	}
	asNew.Kill();
	return bChanged;
}
