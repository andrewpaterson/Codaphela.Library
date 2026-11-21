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
#include "IRMost.h"
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
SIRReference CIRRangePasses::Unextended(SIRReference sValue, uint16 uiOpcode, SIRType* psType)
{
	SIRInstruction*	psDefinition;

	psDefinition = mpcContext->mcDefinitions.Instruction(sValue);
	if (psDefinition)
	{
		if ((psDefinition->uiOpcode == uiOpcode) && (psType->IsNone() || mpcFunction->TypeOf(psDefinition->asArguments[0]).Equals(*psType)))
		{
			*psType = mpcFunction->TypeOf(psDefinition->asArguments[0]);
			return psDefinition->asArguments[0];
		}
		return IRNone();
	}
	if (sValue.IsConstant() && !psType->IsNone() && mpcContext->mpcFoldingPasses->Extended(sValue, psType->IntegerBits(), uiOpcode == IRO_SExt, 0))
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
bool CIRRangePasses::NarrowComparison(SIRInstruction* psInstruction)
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
		psInstruction->uiOpcode = IRUnsignedOpcode(psInstruction->uiOpcode);
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
void CIRRangePasses::ComparisonRange(SIRInstruction* psInstruction)
{
	SIRType		sNarrow;
	int			iConstant;
	uint64		ulliConstant;
	bool		bTrue;
	CChars		szLine;

	if (!mpcContext->mpszWarnings)
	{
		return;
	}
	for (iConstant = 0; iConstant < 2; iConstant++)
	{
		sNarrow = IRType(IRTK_None);
		if (psInstruction->asArguments[iConstant].IsConstant() && !Unextended(psInstruction->asArguments[1 - iConstant], IRO_ZExt, &sNarrow).IsNone() &&
			!mpcContext->mpcFoldingPasses->Extended(psInstruction->asArguments[iConstant], sNarrow.IntegerBits(), false, 0))
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
	if (!strstr(mpcContext->mpszWarnings->Text(), szLine.Text()))
	{
		mpcContext->mpszWarnings->Append(szLine);
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
	if (psInstruction->uiOpcode == IRO_Shl)
	{
		return psInstruction->asArguments[1].IsConstant() && (pcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits < (uint64)iBits);
	}
	return IsIRLowBits(psInstruction->uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
// sValue's low bits as a value of the narrower sType with no instruction: a
// constant truncated, or the value an extension from sType extended; IRNone
// when it takes one.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRRangePasses::FreeNarrow(SIRReference sValue, SIRType sType)
{
	SIRInstruction*	psDefinition;

	if (sValue.IsConstant())
	{
		return mpcFunction->AddConstant(sType, IRTruncate(sType, mpcFunction->GetConstant(sValue)->ulliBits));
	}
	psDefinition = mpcContext->mcDefinitions.Instruction(sValue);
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
SIRReference CIRRangePasses::Narrowed(SIRReference sValue, SIRType sType, CArrayIRInstructions* pasNew, int iDepth)
{
	SIRInstruction*	psDefinition;
	SIRInstruction	sNew;
	SIRReference	sNarrow;

	sNarrow = FreeNarrow(sValue, sType);
	if (!sNarrow.IsNone())
	{
		return sNarrow;
	}
	psDefinition = mpcContext->mcDefinitions.Instruction(sValue);
	memset(&sNew, 0, sizeof(SIRInstruction));
	if (psDefinition && (iDepth < IR_NARROW_DEPTH) && (mpcContext->mcDefinitions.NumUses(sValue) == 1) && IROptimiserNarrows(mpcFunction, psDefinition, sType.IntegerBits()))
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


//UpperBound's operands, for the rules of the largest a value can be: each is as large as UpperBound says of it,
//looking one definition fewer back.
class CIRBoundOperands : public CIRMostRules
{
public:
	CIRRangePasses*	mpcPasses;
	CIRDefinitions*	mpcDefinitions;
	int				miDepth;

protected:
	uint64			Most(SIRReference sValue) override;
	SIRInstruction*	Definition(SIRReference sValue) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRBoundOperands::Most(SIRReference sValue)
{
	return mpcPasses->UpperBound(sValue, miDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIRBoundOperands::Definition(SIRReference sValue)
{
	return sValue.IsTemporary() ? mpcDefinitions->Instruction(sValue) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The most sValue can be as an unsigned integer of its type: a constant;
// what its instruction works out from the most its operands can be, when
// its type holds that (the rules are CIRMostRules', the back end's too:
// an and, an add that can't wrap, a multiply, a shift, x / c ...), up to
// iDepth definitions back; the most of a phi's arguments, each as the
// branch to the phi bounds it.  Else all ones of its type.
// mpcContext->mcDefinitions must be found.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRRangePasses::UpperBound(SIRReference sValue, int iDepth)
{
	CIRBoundOperands	cOperands;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	CIRBlock*		pcBlock;
	SIRType			sType;
	uint64			ulliMost;
	uint64			ulliFirst;
	uint64			ulliSecond;
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

	cOperands.mpcPasses = this;
	cOperands.mpcDefinitions = &mpcContext->mcDefinitions;
	cOperands.miDepth = iDepth - 1;
	psInstruction = mpcContext->mcDefinitions.Instruction(sValue);
	if (psInstruction)
	{
		//What the instruction works out, which its type holds or cuts short.
		ulliFirst = cOperands.InstructionMost(mpcFunction, psInstruction, NULL);
		return (ulliFirst < ulliMost) ? ulliFirst : ulliMost;
	}

	psPhi = mpcContext->mcDefinitions.Phi(sValue);
	if (psPhi)
	{
		pcBlock = mpcContext->mcDefinitions.Block(sValue);
		ulliFirst = 0;
		for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
		{
			psArgument = psPhi->asArguments.Get(ui);
			ulliSecond = cOperands.EdgeMost(psArgument->sValue, psArgument->pcBlock, pcBlock, UpperBound(psArgument->sValue, iDepth - 1));
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
SIRInstruction* CIRRangePasses::EdgeComparison(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue, uint16* puiOpcode, SIRReference* psOther)
{
	SIREdgeCondition	sCondition;

	if (!EdgeCondition(pcFrom, pcTo, &sCondition) || !sCondition.PutLeft(sValue))
	{
		return NULL;
	}
	*puiOpcode = sCondition.uiOpcode;
	*psOther = sCondition.sRight;
	return sCondition.psCompare;
}


//////////////////////////////////////////////////////////////////////////
//
// The signed range of sValue as an integer of its type: a constant's; a
// sext's of the narrower value's; x + c's, x - c's and c - x's moved by c
// (when it can't wrap); a divide's or remainder's by a constant
// (DivisionRange); x + y's, x - y's, x * c's and shifts' by a constant
// (ArithmeticRange); a phi's of all its values (up to iDepth definitions
// back); else from 0 to UpperBound when that is below the type's sign bit
// (a zext, an and).  With IR_RANGE_BRANCHES in uiNarrowBy an operand's range is narrowed by
// the branches to the instruction's block (RangeAt) and a phi's values by
// the branches to the phi on their way.  False if nothing narrower than the
// type is known.  mpcContext->mcDefinitions must be found.
//
//////////////////////////////////////////////////////////////////////////
bool CIRRangePasses::SignedRange(SIRReference sValue, SIRValueRange* psRange, uint8 uiNarrowBy, int iDepth)
{
	SIRInstruction*	psInstruction;
	SIRType			sType;
	SIRType			sNarrow;
	int				iBits;
	uint64			ulliBound;
	int64			lliConstant;
	SIRValueRange		sArgument;
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
		psRange->lliMin = IRSignExtend(sType, mpcFunction->GetConstant(sValue)->ulliBits);
		psRange->lliMax = psRange->lliMin;
		return true;
	}
	psInstruction = sValue.IsTemporary() ? mpcContext->mcDefinitions.Instruction(sValue) : NULL;
	if (psInstruction && (psInstruction->uiOpcode == IRO_Add) && psInstruction->asArguments[1].IsConstant())
	{
		//x + c: x's range moved, when it can't wrap.  ArithmeticRange's would be the same, but one that may wrap isn't
		//then the UpperBound below (x - c and c - x are ArithmeticRange's: UpperBound knows nothing of a sub).
		lliConstant = IRSignExtend(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
		if ((iDepth > 0) && RangeAt(mpcContext->mcDefinitions.Block(sValue), psInstruction->asArguments[0], psRange, uiNarrowBy, iDepth - 1))
		{
			psRange->lliMin += lliConstant;
			psRange->lliMax += lliConstant;
			return (psRange->lliMin >= IRSignedMin(iBits)) && (psRange->lliMax <= IRSignedMax(iBits));
		}
	}
	if (psInstruction && (iDepth > 0) && DivisionRange(psInstruction, psRange, uiNarrowBy, iDepth - 1))
	{
		return true;
	}
	if (psInstruction && (iDepth > 0) && ArithmeticRange(psInstruction, psRange, uiNarrowBy, iDepth - 1))
	{
		return true;
	}
	psPhi = sValue.IsTemporary() ? mpcContext->mcDefinitions.Phi(sValue) : NULL;
	if (psPhi && (iDepth > 0))
	{
		//A phi: the range of all its values (each narrowed by the branches on its way).
		for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
		{
			if (!RangeAt(psPhi->asArguments.Get(ui)->pcBlock, psPhi->asArguments.Get(ui)->sValue, &sArgument, uiNarrowBy, iDepth - 1, mpcContext->mcDefinitions.Block(sValue)))
			{
				return false;
			}
			psRange->lliMin = ((ui == 0) || (sArgument.lliMin < psRange->lliMin)) ? sArgument.lliMin : psRange->lliMin;
			psRange->lliMax = ((ui == 0) || (sArgument.lliMax > psRange->lliMax)) ? sArgument.lliMax : psRange->lliMax;
		}
		return psPhi->asArguments.NumElements() > 0;
	}
	if (psInstruction && (psInstruction->uiOpcode == IRO_SExt))
	{
		sNarrow = mpcFunction->TypeOf(psInstruction->asArguments[0]);
		if (sNarrow.IsInteger() && (sNarrow.IntegerBits() < iBits))
		{
			if ((iDepth == 0) || !SignedRange(psInstruction->asArguments[0], psRange, uiNarrowBy, iDepth - 1))
			{
				psRange->lliMin = IRSignedMin(sNarrow.IntegerBits());
				psRange->lliMax = IRSignedMax(sNarrow.IntegerBits());
			}
			return true;
		}
	}
	ulliBound = UpperBound(sValue, IR_BOUND_DEPTH);
	if (ulliBound <= (uint64)IRSignedMax(iBits))
	{
		psRange->lliMin = 0;
		psRange->lliMax = (int64)ulliBound;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The signed range of sValue where pcBlock runs (on its way to pcTo when
// pcTo isn't NULL): SignedRange's, or the type's when that isn't known,
// narrowed by the branches there (BranchRange, DominatingRange) when
// uiNarrowBy has IR_RANGE_BRANCHES.  False if nothing narrower than the type is known.
//
//////////////////////////////////////////////////////////////////////////
bool CIRRangePasses::RangeAt(CIRBlock* pcBlock, SIRReference sValue, SIRValueRange* psRange, uint8 uiNarrowBy, int iDepth, CIRBlock* pcTo)
{
	SIRType	sType;
	int64	lliLeast;
	int64	lliMost;
	bool	bKnown;

	bKnown = SignedRange(sValue, psRange, uiNarrowBy, iDepth);
	if (!(uiNarrowBy & IR_RANGE_BRANCHES) || !pcBlock)
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
		psRange->lliMin = lliLeast;
		psRange->lliMax = lliMost;
	}
	if (pcTo)
	{
		BranchRange(pcBlock, pcTo, sValue, psRange);
	}
	DominatingRange(pcBlock, sValue, psRange, uiNarrowBy);
	if (psRange->lliMin > psRange->lliMax)
	{
		//No value gets here (the branches contradict): any range will do.
		psRange->lliMax = psRange->lliMin;
	}
	return bKnown || (psRange->lliMin > lliLeast) || (psRange->lliMax < lliMost);
}


//////////////////////////////////////////////////////////////////////////
//
// The signed range of x + y, x - y (both ranges known; x - y is 0 or more
// when neither is negative and a branch on the way says x >= y,
// DominatingNotLess), x * c, x << c,
// x >> c (arithmetic, or logical of an x never negative; of any x when its
// range isn't known) and a trunc of x that fits the narrower type, when it
// can't wrap.  False for anything else.
//
//////////////////////////////////////////////////////////////////////////
bool CIRRangePasses::ArithmeticRange(SIRInstruction* psInstruction, SIRValueRange* psRange, uint8 uiNarrowBy, int iDepth)
{
	CIRBlock*	pcBlock;
	SIRType		sType;
	uint16		uiOpcode;
	SIRValueRange	sFirst;
	SIRValueRange	sSecond;
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
	pcBlock = mpcContext->mcDefinitions.Block(psInstruction->sResult);
	if (!RangeAt(pcBlock, psInstruction->asArguments[0], &sFirst, uiNarrowBy, iDepth))
	{
		//A shift right narrows any value: x is any of its type's (as signed for ashr, unsigned for lshr).
		if (((uiOpcode != IRO_AShr) && (uiOpcode != IRO_LShr)) || !psInstruction->asArguments[1].IsConstant())
		{
			return false;
		}
		sFirst.lliMin = (uiOpcode == IRO_AShr) ? IRSignedMin(iBits) : 0;
		sFirst.lliMax = (uiOpcode == IRO_AShr) ? IRSignedMax(iBits) : IRUnsignedMax(iBits);
	}
	if (uiOpcode == IRO_Trunc)
	{
		psRange->lliMin = sFirst.lliMin;
		psRange->lliMax = sFirst.lliMax;
	}
	else if ((uiOpcode == IRO_Add) || (uiOpcode == IRO_Sub))
	{
		if (!RangeAt(pcBlock, psInstruction->asArguments[1], &sSecond, uiNarrowBy, iDepth))
		{
			return false;
		}
		psRange->lliMin = (uiOpcode == IRO_Add) ? sFirst.lliMin + sSecond.lliMin : sFirst.lliMin - sSecond.lliMax;
		psRange->lliMax = (uiOpcode == IRO_Add) ? sFirst.lliMax + sSecond.lliMax : sFirst.lliMax - sSecond.lliMin;
		if ((uiOpcode == IRO_Sub) && (psRange->lliMin < 0) && (sFirst.lliMin >= 0) && (sSecond.lliMin >= 0) && (uiNarrowBy & IR_RANGE_BRANCHES) &&
			DominatingNotLess(pcBlock, psInstruction->asArguments[0], psInstruction->asArguments[1], uiNarrowBy))
		{
			//x - y where a branch on the way says x >= y (index - first, after index >= first): never negative.
			psRange->lliMin = 0;
		}
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
			psRange->lliMin = (lliConstant >= 0) ? sFirst.lliMin * lliConstant : sFirst.lliMax * lliConstant;
			psRange->lliMax = (lliConstant >= 0) ? sFirst.lliMax * lliConstant : sFirst.lliMin * lliConstant;
		}
		else if ((uiOpcode == IRO_AShr) || (sFirst.lliMin >= 0))
		{
			psRange->lliMin = sFirst.lliMin >> lliConstant;
			psRange->lliMax = sFirst.lliMax >> lliConstant;
		}
		else
		{
			return false;
		}
	}
	return (psRange->lliMin >= IRSignedMin(iBits)) && (psRange->lliMax <= IRSignedMax(iBits));
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
bool CIRRangePasses::DivisionRange(SIRInstruction* psInstruction, SIRValueRange* psRange, uint8 uiNarrowBy, int iDepth)
{
	SIRConstantDivision	sDivision;
	SIRType	sType;
	int64	lliDivisor;
	SIRValueRange	sDividend;
	int		iBits;
	bool	bSigned;

	sType = psInstruction->sType;
	if (!sDivision.Init(mpcFunction, psInstruction) || (sType.IntegerBits() > 32))
	{
		return false;
	}
	iBits = sType.IntegerBits();
	bSigned = sDivision.bSigned;
	lliDivisor = sDivision.lliDivisor;
	if (lliDivisor < 2)
	{
		return false;
	}
	if (!RangeAt(mpcContext->mcDefinitions.Block(psInstruction->sResult), psInstruction->asArguments[0], &sDividend, uiNarrowBy, iDepth))
	{
		sDividend.lliMin = IRSignedMin(iBits);
		sDividend.lliMax = IRSignedMax(iBits);
	}
	if (!bSigned && (sDividend.lliMin < 0))
	{
		sDividend.lliMin = 0;
		sDividend.lliMax = IRUnsignedMax(iBits);
	}
	if (!sDivision.bRemainder)
	{
		//Rounding towards 0 keeps the order.
		psRange->lliMin = sDividend.lliMin / lliDivisor;
		psRange->lliMax = sDividend.lliMax / lliDivisor;
	}
	else
	{
		psRange->lliMin = (sDividend.lliMin < 0) ? ((sDividend.lliMin > -lliDivisor) ? sDividend.lliMin : -(lliDivisor - 1)) : 0;
		psRange->lliMax = (sDividend.lliMax > 0) ? ((sDividend.lliMax < lliDivisor) ? sDividend.lliMax : lliDivisor - 1) : 0;
		if ((sDividend.lliMin >= 0) && (sDividend.lliMax < lliDivisor))
		{
			psRange->lliMin = sDividend.lliMin;
		}
		if ((sDividend.lliMax <= 0) && (sDividend.lliMin > -lliDivisor))
		{
			psRange->lliMax = sDividend.lliMax;
		}
	}
	return psRange->lliMax <= IRSignedMax(iBits);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFrom's branch to pcTo is taken only when sValue isn't null
// (a brif on sValue != 0 to its then, or on sValue == 0 to its else).
//
//////////////////////////////////////////////////////////////////////////
bool CIRRangePasses::BranchNonNull(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue)
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
// every value isn't null on its way in.  mpcContext->mcDefinitions and the
// predecessors must be found.
//
//////////////////////////////////////////////////////////////////////////
bool CIRRangePasses::NonNullAt(CIRBlock* pcBlock, SIRReference sValue, int iDepth)
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

	pcDefinition = mpcContext->mcDefinitions.Block(sValue);
	psInstruction = mpcContext->mcDefinitions.Instruction(sValue);
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
	psPhi = mpcContext->mcDefinitions.Phi(sValue);
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
bool CIRRangePasses::FoldNullTests(void)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sPointer;
	size			ui;
	size			uj;
	bool			bChanged;

	NeedDefinitions();
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
// The block above pcTo on the way every run takes to it: its one
// predecessor (*pbBranch true: that block's branch to pcTo holds in pcTo and
// everything pcTo dominates), or with IR_RANGE_DOMINATORS in uiNarrowBy and
// several predecessors (an if and else joined, a loop's head) its immediate
// dominator (*pbBranch false: nothing is known of how it got from there).
// NULL when there is none.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRRangePasses::RangeStep(CIRBlock* pcTo, uint8 uiNarrowBy, bool* pbBranch)
{
	*pbBranch = pcTo->mapcPredecessors.NumElements() == 1;
	if (*pbBranch)
	{
		return pcTo->mapcPredecessors.GetPtr(0);
	}
	if (!(uiNarrowBy & IR_RANGE_DOMINATORS) || (pcTo->mapcPredecessors.NumElements() == 0))
	{
		return NULL;
	}
	return mpcContext->mcDominators.GetImmediate(pcTo);
}


//////////////////////////////////////////////////////////////////////////
//
// Narrows the signed range [psRange->lliMin, psRange->lliMax] of sValue in pcBlock by the
// branches pcBlock is reached through: while a block has one predecessor,
// that predecessor's branch on a compare of sValue with a constant holds
// (the way to the block), up to IR_RANGE_DEPTH blocks up (RangeStep: with
// IR_RANGE_DOMINATORS on up past blocks with several).
//
//////////////////////////////////////////////////////////////////////////
void CIRRangePasses::DominatingRange(CIRBlock* pcBlock, SIRReference sValue, SIRValueRange* psRange, uint8 uiNarrowBy)
{
	CIRBlock*		pcTo;
	CIRBlock*		pcFrom;
	int				iDepth;
	bool			bBranch;

	pcTo = pcBlock;
	for (iDepth = 0; iDepth < IR_RANGE_DEPTH; iDepth++)
	{
		pcFrom = RangeStep(pcTo, uiNarrowBy, &bBranch);
		if (!pcFrom)
		{
			break;
		}
		if (bBranch)
		{
			BranchRange(pcFrom, pcTo, sValue, psRange);
		}
		pcTo = pcFrom;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Narrows the signed range [psRange->lliMin, psRange->lliMax] of sValue by pcFrom's
// branch to pcTo, when it is on a compare of sValue with a constant
// (EdgeComparison), or below (or not above) a value that is never more than
// a bound positive as signed (UpperBound: a zext, an and; index <= last
// with last a byte): then sValue is no more than the bound either.
//
//////////////////////////////////////////////////////////////////////////
void CIRRangePasses::BranchRange(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue, SIRValueRange* psRange)
{
	SIRReference	sConstant;
	SIRType			sType;
	uint16			uiOpcode;
	int64			lliConstant;
	uint64			ulliBound;

	if (!EdgeComparison(pcFrom, pcTo, sValue, &uiOpcode, &sConstant))
	{
		return;
	}
	if (!sConstant.IsConstant())
	{
		sType = mpcFunction->TypeOf(sValue);
		if (!sType.IsInteger() || (sType.IntegerBits() > 32) || ((uiOpcode != IRO_ULt) && (uiOpcode != IRO_ULe) && (uiOpcode != IRO_SLt) && (uiOpcode != IRO_SLe)))
		{
			return;
		}
		ulliBound = UpperBound(sConstant, IR_BOUND_DEPTH);
		if (ulliBound > (uint64)IRSignedMax(sType.IntegerBits()))
		{
			return;
		}
		//Below it: one less than the most it can be (it is at least 1 then).
		lliConstant = (int64)ulliBound - ((((uiOpcode == IRO_ULt) || (uiOpcode == IRO_SLt)) && (ulliBound > 0)) ? 1 : 0);
		if ((uiOpcode == IRO_ULt) || (uiOpcode == IRO_ULe))
		{
			psRange->lliMin = (psRange->lliMin < 0) ? 0 : psRange->lliMin;
		}
		psRange->lliMax = (lliConstant < psRange->lliMax) ? lliConstant : psRange->lliMax;
		return;
	}
	lliConstant = mpcFunction->GetSigned(sConstant);
	switch (uiOpcode)
	{
	case IRO_SLt:	psRange->lliMax = (lliConstant - 1 < psRange->lliMax) ? lliConstant - 1 : psRange->lliMax;	break;
	case IRO_SLe:	psRange->lliMax = (lliConstant < psRange->lliMax) ? lliConstant : psRange->lliMax;			break;
	case IRO_SGt:	psRange->lliMin = (lliConstant + 1 > psRange->lliMin) ? lliConstant + 1 : psRange->lliMin;	break;
	case IRO_SGe:	psRange->lliMin = (lliConstant > psRange->lliMin) ? lliConstant : psRange->lliMin;			break;
	case IRO_ULt:
	case IRO_ULe:
		//Below a constant that is positive as signed: 0 up to it.
		if (lliConstant > 0)
		{
			psRange->lliMin = (psRange->lliMin < 0) ? 0 : psRange->lliMin;
			lliConstant -= (uiOpcode == IRO_ULt) ? 1 : 0;
			psRange->lliMax = (lliConstant < psRange->lliMax) ? lliConstant : psRange->lliMax;
		}
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a branch pcBlock is reached through (as DominatingRange finds
// them) says sLeft >= sRight or sLeft > sRight, signed or
// unsigned.
//
//////////////////////////////////////////////////////////////////////////
bool CIRRangePasses::DominatingNotLess(CIRBlock* pcBlock, SIRReference sLeft, SIRReference sRight, uint8 uiNarrowBy)
{
	CIRBlock*		pcTo;
	CIRBlock*		pcFrom;
	SIRReference	sOther;
	uint16			uiOpcode;
	int				iDepth;
	bool			bBranch;

	if (!pcBlock || !sLeft.IsTemporary())
	{
		return false;
	}
	pcTo = pcBlock;
	for (iDepth = 0; iDepth < IR_RANGE_DEPTH; iDepth++)
	{
		pcFrom = RangeStep(pcTo, uiNarrowBy, &bBranch);
		if (!pcFrom)
		{
			break;
		}
		if (bBranch && EdgeComparison(pcFrom, pcTo, sLeft, &uiOpcode, &sOther) && sOther.Equals(sRight) &&
			((uiOpcode == IRO_UGe) || (uiOpcode == IRO_UGt) || (uiOpcode == IRO_SGe) || (uiOpcode == IRO_SGt)))
		{
			return true;
		}
		pcTo = pcFrom;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the branches pcBlock is reached through keep the integer sValue 0
// or more (DominatingRange).
//
//////////////////////////////////////////////////////////////////////////
bool CIRRangePasses::NotNegative(CIRBlock* pcBlock, SIRReference sValue)
{
	int		iBits;
	SIRValueRange	sRange;

	iBits = mpcFunction->TypeOf(sValue).IntegerBits();
	if ((iBits < 2) || (iBits > 32) || sValue.IsConstant())
	{
		return false;
	}
	sRange.lliMin = IRSignedMin(iBits);
	sRange.lliMax = IRSignedMax(iBits);
	DominatingRange(pcBlock, sValue, &sRange, IR_RANGE_AS_DEFINED);
	return sRange.lliMin >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
// sValue as a multiple of an extended i8 or i16 value: the sext or zext
// itself (*pllMultiple 1), or a mul or shl of such a multiple by a constant,
// or an add or sub of two multiples of the same extension ((x << 2) - x is
// 3 x: a multiply by a constant a back end made shifts and adds is still
// one), all of sValue's type.  Returns the extension, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIRRangePasses::ExtendedMultiple(SIRReference sValue, int64* plliMultiple, int iDepth)
{
	SIRInstruction*	psInstruction;
	SIRInstruction*	psExtension;
	SIRInstruction*	psOther;
	SIRType			sNarrow;
	int64			lliConstant;
	int64			lliOther;

	psInstruction = sValue.IsTemporary() ? mpcContext->mcDefinitions.Instruction(sValue) : NULL;
	if (!psInstruction || (iDepth == 0))
	{
		return NULL;
	}
	switch (psInstruction->uiOpcode)
	{
	case IRO_ZExt:
	case IRO_SExt:
		sNarrow = mpcFunction->TypeOf(psInstruction->asArguments[0]);
		if (!sNarrow.IsInteger() || ((sNarrow.IntegerBits() != 8) && (sNarrow.IntegerBits() != 16)))
		{
			return NULL;
		}
		*plliMultiple = 1;
		return psInstruction;
	case IRO_Mul:
	case IRO_Shl:
		if (!psInstruction->asArguments[1].IsConstant())
		{
			return NULL;
		}
		lliConstant = (int64)IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
		if (psInstruction->uiOpcode == IRO_Shl)
		{
			if (lliConstant > 15)
			{
				return NULL;
			}
			lliConstant = (int64)1 << lliConstant;
		}
		psExtension = ExtendedMultiple(psInstruction->asArguments[0], plliMultiple, iDepth - 1);
		if (!psExtension || (lliConstant > 0xFFFF))
		{
			return NULL;
		}
		*plliMultiple *= lliConstant;
		break;
	case IRO_Add:
	case IRO_Sub:
		psExtension = ExtendedMultiple(psInstruction->asArguments[0], plliMultiple, iDepth - 1);
		psOther = psExtension ? ExtendedMultiple(psInstruction->asArguments[1], &lliOther, iDepth - 1) : NULL;
		if (!psExtension || (psOther != psExtension))
		{
			return NULL;
		}
		*plliMultiple += (psInstruction->uiOpcode == IRO_Add) ? lliOther : -lliOther;
		break;
	default:
		return NULL;
	}
	//Small enough that nothing above overflows.
	return ((*plliMultiple >= -0x100000) && (*plliMultiple <= 0x100000)) ? psExtension : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A multiple of an extended i8 or i16 value (ExtendedMultiple: a mul or shl
// by a constant, or the shifts, adds and subs a multiply was made) that
// fits 16 bits unsigned (the value 0 or more, its range as it is worked out
// and from the branches the block is reached through, RangeAt, or a zext's
// bits) is a 16 bit mul, zero extended: (long)y * 320 with y from 0 to 199
// is a 16 bit y * 320, and (index - first) * 12 after first <= index <=
// last, both bytes, is a 16 bit one.
//
//////////////////////////////////////////////////////////////////////////
bool CIRRangePasses::NarrowMultiplies(void)
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
	SIRValueRange							sRange;
	int64								lliMultiple;
	int									iBits;
	size								ui;
	size								uiInstruction;
	bool								bChanged;

	//Found first: the inserts move the definitions RangeAt reads.
	Renumber();
	NeedDefinitions();
	mpcFunction->ComputePredecessors();
	ComputeDominators();
	sShort = IRType(IRTK_I16);
	asFound.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sType = psInstruction->sType;
			if (((psInstruction->uiOpcode != IRO_Mul) && (psInstruction->uiOpcode != IRO_Shl) && (psInstruction->uiOpcode != IRO_Add) && (psInstruction->uiOpcode != IRO_Sub)) ||
				!psInstruction->sResult.IsTemporary() || !sType.IsInteger() || (sType.IntegerBits() <= 16) || (sType.IntegerBits() > 32))
			{
				continue;
			}
			psExtension = ExtendedMultiple(psInstruction->sResult, &lliMultiple, IR_MULTIPLE_DEPTH);
			if (!psExtension || (lliMultiple < 2) || (lliMultiple > 0xFFFF))
			{
				continue;
			}
			ulliConstant = (uint64)lliMultiple;
			sNarrow = psExtension->asArguments[0];
			iBits = mpcFunction->TypeOf(sNarrow).IntegerBits();
			if ((iBits != 8) && (iBits != 16))
			{
				continue;
			}
			//The compares are of the narrow value as it is (a signed compare sees it signed); a zext of one that may be
			//negative is up to all its bits.
			if (!RangeAt(pcBlock, sNarrow, &sRange, IR_RANGE_BRANCHES | IR_RANGE_DOMINATORS))
			{
				sRange.lliMin = IRSignedMin(iBits);
				sRange.lliMax = IRSignedMax(iBits);
			}
			if ((sRange.lliMin < 0) && (psExtension->uiOpcode == IRO_ZExt))
			{
				sRange.lliMin = 0;
				sRange.lliMax = IRUnsignedMax(iBits);
			}
			if ((sRange.lliMin < 0) || ((uint64)sRange.lliMax * ulliConstant > 0xFFFF))
			{
				continue;
			}
			psFound = asFound.Add();
			psFound->pcBlock = pcBlock;
			psFound->uiInstruction = uiInstruction;
			psFound->sNarrow = sNarrow;
			psFound->uiMultiple = (uint16)ulliConstant;
		}
	}


	//Last first, so each instruction's index is still where it was found.  (The inserts leave the blocks, and so their
	//dominators, as they are.)
	bChanged = asFound.NumElements() > 0;
	for (ui = asFound.NumElements(); ui > 0; ui--)
	{
		psFound = asFound.Get(ui - 1);
		pcBlock = psFound->pcBlock;
		uiInstruction = psFound->uiInstruction;
		ulliConstant = psFound->uiMultiple;
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
	if (bChanged)
	{
		FindDefinitions();
	}
	return BoundIndices() || bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A ptradd of a zero extended i16 offset that is never more than
// IR_IN_ARRAY_MOST (its range as it is worked out and from the branches
// the block is reached through, RangeAt) is IRF_InArray: the offset and
// the bytes of anything read or written through it stay under 64KB, so a
// back end may add to the offset in 16 bits (the W65C816's 2 word loads
// and stores through Y).  A constant its pointer was just given, in the
// same block and for nothing else ((p + 12) + x, a member array's element),
// then goes into the offset while that still fits: p + (x + 12), one 16 bit
// add in place of a pointer's.  The definitions and the dominators must be
// found.
//
//////////////////////////////////////////////////////////////////////////
bool CIRRangePasses::BoundIndices(void)
{
	SIRInstruction*	psInstruction;
	SIRInstruction*	psExtension;
	SIRInstruction*	psBase;
	CIRBlock*		pcBlock;
	SIRReference	sOffset;
	SIRReference	sPointer;
	SIRReference	sNarrow;
	SIRType			sWide;
	SIRType			sShort;
	SIRValueRange		sRange;
	int64			lliConstant;
	uint16			uiBaseFlags;
	size			ui;
	size			uiInstruction;
	bool			bChanged;

	bChanged = false;
	sShort = IRType(IRTK_I16);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if ((psInstruction->uiOpcode != IRO_PtrAdd) || !psInstruction->asArguments[1].IsTemporary())
			{
				continue;
			}
			psExtension = mpcContext->mcDefinitions.DefinedBy(psInstruction->asArguments[1], IRO_ZExt);
			if (!psExtension || !mpcFunction->TypeOf(psExtension->asArguments[0]).Is(IRTK_I16))
			{
				continue;
			}
			sNarrow = psExtension->asArguments[0];
			if (!RangeAt(pcBlock, sNarrow, &sRange, IR_RANGE_BRANCHES | IR_RANGE_DOMINATORS) || (sRange.lliMin < 0) || (sRange.lliMax > IR_IN_ARRAY_MOST))
			{
				continue;
			}
			if (!(psInstruction->uiFlags & IRF_InArray))
			{
				psInstruction->uiFlags |= IRF_InArray;
				bChanged = true;
			}

			//(p + c) + x, the p + c worked out here for this alone.
			sPointer = psInstruction->asArguments[0];
			psBase = sPointer.IsTemporary() ? mpcContext->mcDefinitions.Instruction(sPointer) : NULL;
			if (!psBase || (psBase->uiOpcode != IRO_PtrAdd) || !psBase->asArguments[1].IsConstant() || (mpcContext->mcDefinitions.Block(sPointer) != pcBlock) ||
				(mpcContext->mcDefinitions.NumUses(sPointer) != 1))
			{
				continue;
			}
			lliConstant = mpcFunction->GetSigned(psBase->asArguments[1]);
			if ((lliConstant <= 0) || (sRange.lliMax + lliConstant > IR_IN_ARRAY_MOST))
			{
				continue;
			}
			sPointer = psBase->asArguments[0];
			uiBaseFlags = psBase->uiFlags;
			sWide = mpcFunction->TypeOf(psInstruction->asArguments[1]);
			sOffset = InsertInstruction(pcBlock, uiInstruction, IRO_Add, sShort, sNarrow, mpcFunction->AddInteger(sShort, lliConstant));
			uiInstruction++;
			sOffset = InsertInstruction(pcBlock, uiInstruction, IRO_ZExt, sWide, sOffset, IRNone());
			uiInstruction++;
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			psInstruction->asArguments[0] = sPointer;
			psInstruction->asArguments[1] = sOffset;
			if (!(uiBaseFlags & IRF_InBank))
			{
				psInstruction->uiFlags &= ~IRF_InBank;
			}
			bChanged = true;

			//The inserts moved the block's instructions.
			FindDefinitions();
		}
	}
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
bool CIRRangePasses::NarrowTruncations(void)
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

	NeedDefinitions();
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
			psWide = (sInstruction.uiOpcode == IRO_Trunc) && sInstruction.sResult.IsTemporary() && sType.IsInteger() && !sType.Is(IRTK_I1) ? mpcContext->mcDefinitions.Instruction(sInstruction.asArguments[0]) : NULL;
			if (psWide && (mpcContext->mcDefinitions.NumUses(sInstruction.asArguments[0]) == 1) && IROptimiserNarrows(mpcFunction, psWide, sType.IntegerBits()) &&
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
