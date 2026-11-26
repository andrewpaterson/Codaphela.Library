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
#include "BaseLib/IntegerHelper.h"
#include "IROptimiserInternal.h"


//The optimiser's division passes: divisions by powers of two, bounded divisions, divisions by constants (reciprocals
//and byte tables), divisibility tests and shared divisions.


//////////////////////////////////////////////////////////////////////////
//
// A signed x / 2^n or x % 2^n is shifts and adds, not the target's divide:
// a negative x has 2^n - 1 added first, so the shift rounds towards 0.
//   t = x + ((x >> (bits - 1)) >>> (bits - n))
//   x / 2^n = t >> n
//   x % 2^n = x - (t & -2^n)
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::SignedPowerDivisions(void)
{
	SIRConstantDivision	sDivision;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sValue;
	SIRReference	sBias;
	SIRType			sType;
	int64			lliDivisor;
	int				iBits;
	int				iShift;
	size			ui;
	size			uiInstruction;
	bool			bChanged;

	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (!sDivision.Init(mpcFunction, psInstruction) || !sDivision.bSigned || psInstruction->sType.Is(IRTK_I1))
			{
				continue;
			}
			sType = psInstruction->sType;
			iBits = sType.IntegerBits();
			lliDivisor = sDivision.lliDivisor;
			if ((lliDivisor < 2) || ((lliDivisor & (lliDivisor - 1)) != 0))
			{
				continue;
			}
			iShift = (int)FindFirstSetBit((uint64)lliDivisor);
			sValue = psInstruction->asArguments[0];
			sBias = InsertInstruction(pcBlock, uiInstruction, IRO_AShr, sType, sValue, mpcFunction->AddInteger(sType, iBits - 1));
			sBias = InsertInstruction(pcBlock, uiInstruction + 1, IRO_LShr, sType, sBias, mpcFunction->AddInteger(sType, iBits - iShift));
			sBias = InsertInstruction(pcBlock, uiInstruction + 2, IRO_Add, sType, sValue, sBias);
			uiInstruction += 3;
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->uiOpcode == IRO_SDiv)
			{
				psInstruction->uiOpcode = IRO_AShr;
				psInstruction->asArguments[0] = sBias;
				psInstruction->asArguments[1] = mpcFunction->AddInteger(sType, iShift);
			}
			else
			{
				sBias = InsertInstruction(pcBlock, uiInstruction, IRO_And, sType, sBias, mpcFunction->AddInteger(sType, -lliDivisor));
				uiInstruction++;
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				psInstruction->uiOpcode = IRO_Sub;
				psInstruction->asArguments[1] = sBias;
			}
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// An unsigned x / c or x % c (c a constant, not a power of 2) with x known
// to be below c (UpperBound) is 0 or x; below 2c it is x >= c, or
// x - (c & -(x >= c)): a compare and a subtract, not the target's divide.
// So is a signed one of an x never negative.  x's bound is UpperBound's
// or its signed range's narrowed by the branches to the divide (RangeAt).
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::BoundDivisions(void)
{
	CArrayTemplate<SIRBoundDivision>	asFound;
	SIRBoundDivision*					psFound;
	SIRConstantDivision					sDivision;
	CIRBlock*							pcBlock;
	SIRInstruction*						psInstruction;
	SIRReference						sValue;
	SIRReference						sDivisor;
	SIRReference						sTest;
	SIRType								sType;
	uint64								ulliDivisor;
	uint64								ulliBound;
	SIRValueRange						sRange;
	size								ui;
	size								uiInstruction;
	bool								bChanged;

	//Found first: the inserts move the definitions UpperBound reads.
	NeedDefinitions();
	mpcFunction->ComputePredecessors();
	asFound.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (!sDivision.Init(mpcFunction, psInstruction) || !psInstruction->sResult.IsTemporary())
			{
				continue;
			}
			//As the type's unsigned value: a negative divisor is above half of them, and left.
			ulliDivisor = IRTruncate(psInstruction->sType, (uint64)sDivision.lliDivisor);
			if ((ulliDivisor < 2) || (ulliDivisor > (IRTruncate(psInstruction->sType, (uint64)-1) >> 1)))
			{
				continue;
			}
			ulliBound = !sDivision.bSigned ? mpcContext->mcRanges.UpperBound(psInstruction->asArguments[0], IR_BOUND_DEPTH) : (uint64)-1;
			if ((ulliBound >= ulliDivisor * 2) && mpcContext->mcRanges.RangeAt(pcBlock, psInstruction->asArguments[0], &sRange, IR_RANGE_BRANCHES) && (sRange.lliMin >= 0))
			{
				ulliBound = (uint64)sRange.lliMax;
			}
			if (ulliBound < ulliDivisor * 2)
			{
				psFound = asFound.Add();
				psFound->pcBlock = pcBlock;
				psFound->uiInstruction = uiInstruction;
				psFound->bOnce = ulliBound >= ulliDivisor;
			}
		}
	}

	//Last first, so each instruction's index is still where it was found.
	ClearReplacements();
	bChanged = asFound.NumElements() > 0;
	for (ui = asFound.NumElements(); ui > 0; ui--)
	{
		psFound = asFound.Get(ui - 1);
		pcBlock = psFound->pcBlock;
		uiInstruction = psFound->uiInstruction;
		psInstruction = pcBlock->masInstructions.Get(uiInstruction);
		sType = psInstruction->sType;
		sValue = psInstruction->asArguments[0];
		sDivisor = psInstruction->asArguments[1];
		if (!psFound->bOnce)
		{
			Replace(psInstruction->sResult, IsIRRemainder(psInstruction->uiOpcode) ? sValue : mpcFunction->AddInteger(sType, 0));
			continue;
		}
		sTest = InsertInstruction(pcBlock, uiInstruction, IRO_UGe, sType, sValue, sDivisor);
		uiInstruction++;
		if (IsIRQuotient(pcBlock->masInstructions.Get(uiInstruction)->uiOpcode))
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			psInstruction->uiOpcode = IRO_ZExt;
			psInstruction->asArguments[0] = sTest;
			psInstruction->asArguments[1] = IRNone();
		}
		else
		{
			sTest = InsertDivisorMask(pcBlock, uiInstruction, sType, sTest, sDivisor);
			psInstruction = pcBlock->masInstructions.Get(uiInstruction + 3);
			psInstruction->uiOpcode = IRO_Sub;
			psInstruction->asArguments[1] = sTest;
		}
	}
	asFound.Kill();
	if (bChanged)
	{
		ApplyReplacements();
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether x / divisor and x % divisor for x from lliMin to lliMax can be
// looked up in a table of a byte's (an i8 divide marked IRF_Hot): x fits a
// signed byte (or an unsigned one when it is never negative).
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::IsByteTableDivision(int64 lliMin, int64 lliMax, int64 lliDivisor)
{
	if (lliMin < 0)
	{
		return (lliMin >= -0x80) && (lliMax < 0x80) && (lliDivisor < 0x80);
	}
	return (lliMax < 0x100) && (lliDivisor < 0x100);
}


//////////////////////////////////////////////////////////////////////////
//
// The smallest shift and multiplier with x / divisor (rounding towards 0)
// the same as (x * multiplier) >> shift (plus 1 when x and so the product
// is negative, bSigned) for every x from lliMin to lliMax, with x *
// multiplier fitting iBits (signed when bSigned).  Every x is checked
// (ranges of at most IR_DIVISION_RANGE values).  False if none fits.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::DivisionReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, uint64* pulliMultiplier, int* piShift)
{
	int64	lliMost;
	int64	lliLeast;
	int64	lliMultiplier;
	int64	lliProduct;
	int64	lliQuotient;
	int64	x;
	int		iShift;
	bool	bExact;

	if ((lliMax - lliMin >= IR_DIVISION_RANGE) || (lliDivisor < 3) || (iBits > 32))
	{
		return false;
	}
	lliMost = bSigned ? IRSignedMax(iBits) : IRUnsignedMax(iBits);
	lliLeast = bSigned ? IRSignedMin(iBits) : 0;
	for (iShift = 1; iShift < iBits; iShift++)
	{
		lliMultiplier = (((int64)1 << iShift) + lliDivisor - 1) / lliDivisor;
		if ((lliMultiplier * lliMax > lliMost) || (lliMultiplier * lliMin < lliLeast))
		{
			break;
		}
		bExact = true;
		for (x = lliMin; bExact && (x <= lliMax); x++)
		{
			lliProduct = x * lliMultiplier;
			lliQuotient = (lliProduct >= 0) ? (lliProduct >> iShift) : -(((-lliProduct) + ((int64)1 << iShift) - 1) >> iShift) + 1;
			bExact = lliQuotient == x / lliDivisor;
		}
		if (bExact)
		{
			*pulliMultiplier = (uint64)lliMultiplier;
			*piShift = iShift;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The reciprocal (DivisionReciprocal) of lliDivisor for x from lliMin to
// lliMax in iBits, else in 32 bits (psReciprocal's iBits says which).
// Failing that, for an x that is never negative and an even divisor
// 2^k * m, m's for x >> k (iPreShift is k, else 0): its smaller products
// fit where x's don't.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::FindReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, SIRReciprocal* psReciprocal)
{
	int		iPreShift;
	int		iWork;

	for (iPreShift = 0; ; )
	{
		for (iWork = iBits; iWork <= 32; iWork = (iWork < 32) ? 32 : 33)
		{
			if (DivisionReciprocal(lliMin >> iPreShift, lliMax >> iPreShift, lliDivisor >> iPreShift, iWork, bSigned, &psReciprocal->ulliMultiplier, &psReciprocal->iShift))
			{
				psReciprocal->iBits = iWork;
				psReciprocal->iPreShift = iPreShift;
				return true;
			}
		}
		if ((iPreShift > 0) || (lliMin < 0) || ((lliDivisor & 1) != 0))
		{
			return false;
		}
		while (((lliDivisor >> iPreShift) & 1) == 0)
		{
			iPreShift++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the reciprocal found for psInstruction (x / c or x % c in
// pcBlock, x in psRange) costs the target less than its divide does (the
// target's CIRDivisionCosts).  x / c and x % c of one x in the block are
// priced together, so that both get the one answer: as a reciprocal they
// cost what the remainder does (x less the quotient times c: the quotient
// is in it), and as a divide they are one (the routine leaves the
// remainder, LeavesRemainder, for ShareDivisions) or two.  True for a
// target that hasn't said what they cost.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::IsReciprocalCheaper(CIRBlock* pcBlock, SIRInstruction* psInstruction, SIRConstantDivision* psDivision, SIRValueRange* psRange, SIRReciprocal* psReciprocal)
{
	CIRDivisionCosts*	pcCosts;
	SIRConstantDivision	sPriced;
	SIRInstruction		sRemainder;
	int					iDivide;

	pcCosts = mpcContext->mpcDivisionCosts;
	if (!pcCosts)
	{
		return true;
	}
	sPriced = *psDivision;
	iDivide = pcCosts->DivideCost(psInstruction->sType, psDivision, psRange->lliMin, psRange->lliMax);
	if (HasDivision(pcBlock, IRDivisionPartner(psInstruction->uiOpcode), psInstruction->asArguments[0], psInstruction->asArguments[1]))
	{
		sPriced.bRemainder = true;
		sRemainder = *psInstruction;
		sRemainder.uiOpcode = IRDivisionOpcode(psDivision->bSigned, true);
		if (!LeavesRemainder(&sRemainder))
		{
			iDivide *= 2;
		}
	}
	return pcCosts->ReciprocalCost(psInstruction->sType, &sPriced, psRange->lliMin, psRange->lliMax, psReciprocal) < iDivide;
}


//////////////////////////////////////////////////////////////////////////
//
// x % c == 0 and x % c != 0 (c a constant above 2, not a power of 2, x an
// i16 or i32) without a divide: c = 2^k * d (d odd), so with i the
// inverse of d modulo 2^n, x % c == 0 exactly when (x * i) rotated right
// k places is at most (2^n - 1) / c (Hacker's Delight 10-17).  A signed
// x's remainder is 0 when its size's is, so a signed x that may be
// negative is made its size first.  For a target whose divides are
// runtime calls.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::DivisibilityTests(void)
{
	SIRConstantDivision	sDivision;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRInstruction*	psRemainder;
	SIRReference	sValue;
	SIRReference	sSign;
	SIRReference	sTest;
	SIRType			sType;
	uint64			ulliDivisor;
	uint64			ulliOdd;
	uint64			ulliInverse;
	uint64			ulliMask;
	SIRValueRange	sRange;
	size			ui;
	size			uj;
	int				iBits;
	int				iShift;
	int				i;
	bool			bChanged;
	bool			bSigned;

	NeedDefinitions();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if (((psInstruction->uiOpcode != IRO_Eq) && (psInstruction->uiOpcode != IRO_Ne)) || !psInstruction->sResult.IsTemporary() ||
				!psInstruction->asArguments[0].IsTemporary() || !psInstruction->asArguments[1].IsConstant() ||
				(mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits != 0))
			{
				continue;
			}
			psRemainder = mpcContext->mcDefinitions.Instruction(psInstruction->asArguments[0]);
			if (!psRemainder || !sDivision.Init(mpcFunction, psRemainder) || !sDivision.bRemainder ||
				!(psRemainder->sType.Is(IRTK_I16) || psRemainder->sType.Is(IRTK_I32)))
			{
				continue;
			}
			sType = psRemainder->sType;
			iBits = sType.IntegerBits();
			ulliMask = IRTruncate(sType, (uint64)-1);
			bSigned = sDivision.bSigned;
			ulliDivisor = (uint64)sDivision.lliDivisor;
			if (bSigned && ((int64)ulliDivisor < 0))
			{
				ulliDivisor = (uint64)(-(int64)ulliDivisor);
			}
			if ((ulliDivisor < 3) || (ulliDivisor > ulliMask) || ((ulliDivisor & (ulliDivisor - 1)) == 0))
			{
				continue;
			}
			for (iShift = 0, ulliOdd = ulliDivisor; (ulliOdd & 1) == 0; iShift++)
			{
				ulliOdd >>= 1;
			}

			//Newton's iteration: each step doubles the low bits of the inverse that are right (d * d = 1 mod 8).
			ulliInverse = ulliOdd;
			for (i = 0; i < 5; i++)
			{
				ulliInverse = (ulliInverse * (2 - ulliOdd * ulliInverse)) & ulliMask;
			}

			sValue = psRemainder->asArguments[0];
			if (bSigned && (!mpcContext->mcRanges.SignedRange(sValue, &sRange, IR_RANGE_AS_DEFINED) || (sRange.lliMin < 0)))
			{
				//|x|: (x ^ s) - s with s the sign (-2^(n-1) stays itself, which is its size unsigned).
				sSign = InsertInstruction(pcBlock, uj++, IRO_AShr, sType, sValue, mpcFunction->AddInteger(sType, iBits - 1));
				sValue = InsertInstruction(pcBlock, uj++, IRO_Xor, sType, sValue, sSign);
				sValue = InsertInstruction(pcBlock, uj++, IRO_Sub, sType, sValue, sSign);
			}
			sTest = InsertInstruction(pcBlock, uj++, IRO_Mul, sType, sValue, mpcFunction->AddConstant(sType, ulliInverse));
			if (iShift > 0)
			{
				sSign = InsertInstruction(pcBlock, uj++, IRO_Shl, sType, sTest, mpcFunction->AddInteger(sType, iBits - iShift));
				sTest = InsertInstruction(pcBlock, uj++, IRO_LShr, sType, sTest, mpcFunction->AddInteger(sType, iShift));
				sTest = InsertInstruction(pcBlock, uj++, IRO_Or, sType, sTest, sSign);
			}
			psInstruction = pcBlock->masInstructions.Get(uj);
			psInstruction->uiOpcode = (psInstruction->uiOpcode == IRO_Eq) ? IRO_ULe : IRO_UGt;
			psInstruction->asArguments[0] = sTest;
			psInstruction->asArguments[1] = mpcFunction->AddConstant(sType, ulliMask / ulliDivisor);
			bChanged = true;
			FindDefinitions();
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// x / c and x % c, c a constant above 2 and not a power of 2, x in a known
// range (SignedRange): q is (x * m) >> s (DivisionReciprocal), plus 1 when
// x is negative (signed); x % c is x - q * c.  Worked out in the type when
// x * m fits it, else in 32 bits (from 16).  For a target whose divides are
// runtime calls (the multiply and shifts are a few instructions; the
// multiply by a constant shifts and adds), and only where it says they
// cost less than the divide (IsReciprocalCheaper: a multiplier of many
// digits in 32 bits is a call itself).  A divide and a remainder of one
// x get the same q, which common subexpressions make one.  An i8 one the
// profile finds hot (IsHot) is marked IRF_Hot and left for the target to
// look up in a table.  x's range is narrowed by the branches to the divide
// (IR_RANGE_BRANCHES).  A 32 bit divide by a wide c = 2^k * m (m under 65536)
// that isn't made a multiply is split (SplitDivisor).
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::ConstantDivisions(void)
{
	CArrayTemplate<SIRFoundDivision>	asFound;
	SIRFoundDivision*					psFound;
	SIRFoundDivision					sFound;
	SIRConstantDivision					sDivision;
	SIRReciprocal*						psReciprocal;
	CIRBlock*							pcBlock;
	SIRInstruction*						psInstruction;
	SIRReference						sValue;
	SIRReference						sWide;
	SIRReference						sQuotient;
	SIRReference						sSign;
	SIRReference						sDivisor;
	SIRReference						sConstant;
	SIRType								sType;
	SIRType								sWork;
	int64								lliDivisor;
	SIRValueRange						sRange;
	int									iShift;
	size								ui;
	size								uiInstruction;
	size								uiAt;
	bool								bSigned;
	uint16								uiOpcode;

	NeedDefinitions();
	mpcFunction->ComputePredecessors();
	asFound.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (!sDivision.Init(mpcFunction, psInstruction) || !psInstruction->sResult.IsTemporary() ||
				(psInstruction->sType.IntegerBits() < 8) || (psInstruction->sType.IntegerBits() > 32))
			{
				continue;
			}
			bSigned = sDivision.bSigned;
			lliDivisor = sDivision.lliDivisor;
			if ((lliDivisor < 3) || ((lliDivisor & (lliDivisor - 1)) == 0))
			{
				continue;
			}
			if (psInstruction->sType.Is(IRTK_I8) && ((psInstruction->uiFlags & IRF_Hot) || IsHot(psInstruction, IR_DIVISION_TABLE_SAVING)))
			{
				//Hot: the target looks q and r up in a table of every byte's.
				psInstruction->uiFlags |= IRF_Hot;
				continue;
			}
			if (!mpcContext->mcRanges.RangeAt(pcBlock, psInstruction->asArguments[0], &sRange, IR_RANGE_BRANCHES) || (!bSigned && (sRange.lliMin < 0)))
			{
				sRange.lliMin = 1;
				sRange.lliMax = 0;
			}
			//The table for a hot one of a byte, else the multiply, else the split.
			sFound.uiKind = IRCD_None;
			if ((sRange.lliMin <= sRange.lliMax) && IsByteTableDivision(sRange.lliMin, sRange.lliMax, lliDivisor) && IsHot(psInstruction, IR_DIVISION_TABLE_SAVING))
			{
				sFound.uiKind = IRCD_Table;
			}
			else if ((sRange.lliMin <= sRange.lliMax) && FindReciprocal(sRange.lliMin, sRange.lliMax, lliDivisor, psInstruction->sType.IntegerBits(), bSigned, &sFound.sReciprocal) &&
					 IsReciprocalCheaper(pcBlock, psInstruction, &sDivision, &sRange, &sFound.sReciprocal))
			{
				sFound.uiKind = IRCD_Reciprocal;
			}
			else if (SplitDivisor(psInstruction->sType, lliDivisor, sRange.lliMin, sRange.lliMax))
			{
				sFound.uiKind = IRCD_Split;
			}
			if (sFound.uiKind != IRCD_None)
			{
				sFound.pcBlock = pcBlock;
				sFound.uiInstruction = uiInstruction;
				sFound.sRange = sRange;
				asFound.Add(&sFound);
			}
		}
	}

	//Last first, so each instruction's index is still where it was found.
	for (ui = asFound.NumElements(); ui > 0; ui--)
	{
		psFound = asFound.Get(ui - 1);
		pcBlock = psFound->pcBlock;
		uiInstruction = psFound->uiInstruction;
		psInstruction = pcBlock->masInstructions.Get(uiInstruction);
		uiOpcode = psInstruction->uiOpcode;
		sType = psInstruction->sType;
		//Kept by value, as the split's inserts move the block's instructions.
		sValue = psInstruction->asArguments[0];
		sConstant = psInstruction->asArguments[1];
		sDivision.Init(mpcFunction, psInstruction);
		bSigned = sDivision.bSigned;
		lliDivisor = sDivision.lliDivisor;
		sRange = psFound->sRange;
		if (psFound->uiKind == IRCD_Split)
		{
			for (iShift = 0; ((lliDivisor >> iShift) & 1) == 0; iShift++)
			{
			}
			uiAt = uiInstruction;
			sWide = InsertInstruction(pcBlock, uiAt++, IRDivisionOpcode(bSigned, false), sType, sValue, mpcFunction->AddInteger(sType, (int64)1 << iShift));
			sDivisor = mpcFunction->AddInteger(sType, lliDivisor >> iShift);
			if (IsIRQuotient(uiOpcode))
			{
				//(x / 2^k) / m.
				psInstruction = pcBlock->masInstructions.Get(uiAt);
				psInstruction->asArguments[0] = sWide;
				psInstruction->asArguments[1] = sDivisor;
			}
			else if (HasDivision(pcBlock, IRDivisionPartner(uiOpcode), sValue, sConstant))
			{
				//x - ((x / 2^k) / m) * c: the same q as the divide's.
				sQuotient = InsertInstruction(pcBlock, uiAt++, IRDivisionOpcode(bSigned, false), sType, sWide, sDivisor);
				sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Mul, sType, sQuotient, mpcFunction->AddInteger(sType, lliDivisor));
				psInstruction = pcBlock->masInstructions.Get(uiAt);
				psInstruction->uiOpcode = IRO_Sub;
				psInstruction->asArguments[0] = sValue;
				psInstruction->asArguments[1] = sQuotient;
			}
			else
			{
				//((x / 2^k) % m) << k + x % 2^k (both with x's sign).
				sQuotient = InsertInstruction(pcBlock, uiAt++, IRDivisionOpcode(bSigned, true), sType, sWide, sDivisor);
				sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Shl, sType, sQuotient, mpcFunction->AddInteger(sType, iShift));
				sSign = InsertInstruction(pcBlock, uiAt++, IRDivisionOpcode(bSigned, true), sType, sValue, mpcFunction->AddInteger(sType, (int64)1 << iShift));
				psInstruction = pcBlock->masInstructions.Get(uiAt);
				psInstruction->uiOpcode = IRO_Add;
				psInstruction->asArguments[0] = sQuotient;
				psInstruction->asArguments[1] = sSign;
			}
			continue;
		}
		if (psFound->uiKind == IRCD_Table)
		{
			//The byte's i8 divide (signed only when the byte may be negative), extended back.
			sWork = IRType(IRTK_I8);
			bSigned = sRange.lliMin < 0;
			uiAt = uiInstruction;
			sWide = InsertInstruction(pcBlock, uiAt++, IRO_Trunc, sWork, sValue, IRNone());
			uiOpcode = IRDivisionOpcode(bSigned, IsIRRemainder(uiOpcode));
			sQuotient = InsertInstruction(pcBlock, uiAt++, uiOpcode, sWork, sWide, mpcFunction->AddInteger(sWork, lliDivisor));
			pcBlock->masInstructions.Get(uiAt - 1)->uiFlags |= IRF_Hot;
			psInstruction = pcBlock->masInstructions.Get(uiAt);
			psInstruction->uiOpcode = bSigned ? IRO_SExt : IRO_ZExt;
			psInstruction->uiFlags = 0;
			psInstruction->asArguments[0] = sQuotient;
			psInstruction->asArguments[1] = IRNone();
			continue;
		}
		//The multiply and shift found for it.
		psReciprocal = &psFound->sReciprocal;
		sWork = (psReciprocal->iBits == sType.IntegerBits()) ? sType : IRType(IRTK_I32);

		uiAt = uiInstruction;
		sWide = sValue;
		if (sWork.uiKind != sType.uiKind)
		{
			sWide = InsertInstruction(pcBlock, uiAt++, bSigned ? IRO_SExt : IRO_ZExt, sWork, sValue, IRNone());
		}
		if (psReciprocal->iPreShift > 0)
		{
			//x is never negative: (x >> k) / m for c = 2^k * m.
			sWide = InsertInstruction(pcBlock, uiAt++, IRO_LShr, sWork, sWide, mpcFunction->AddInteger(sWork, psReciprocal->iPreShift));
		}
		sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Mul, sWork, sWide, mpcFunction->AddInteger(sWork, (int64)psReciprocal->ulliMultiplier));
		sQuotient = InsertInstruction(pcBlock, uiAt++, bSigned ? IRO_AShr : IRO_LShr, sWork, sQuotient, mpcFunction->AddInteger(sWork, psReciprocal->iShift));
		if (bSigned && (sRange.lliMin < 0))
		{
			sSign = InsertInstruction(pcBlock, uiAt++, IRO_AShr, sWork, sWide, mpcFunction->AddInteger(sWork, sWork.IntegerBits() - 1));
			sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Sub, sWork, sQuotient, sSign);
		}
		if (sWork.uiKind != sType.uiKind)
		{
			sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Trunc, sType, sQuotient, IRNone());
		}
		psInstruction = pcBlock->masInstructions.Get(uiAt);
		if (IsIRQuotient(uiOpcode))
		{
			//q, as an add of 0: folding gives every use q itself.  (A copy is a value put in a place of its own, for a
			//back end: no pass takes one away.)
			psInstruction->uiOpcode = IRO_Add;
			psInstruction->asArguments[0] = sQuotient;
			psInstruction->asArguments[1] = mpcFunction->AddInteger(sType, 0);
		}
		else
		{
			//x - q * c.
			sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Mul, sType, sQuotient, mpcFunction->AddInteger(sType, lliDivisor));
			psInstruction = pcBlock->masInstructions.Get(uiAt);
			psInstruction->uiOpcode = IRO_Sub;
			psInstruction->asArguments[0] = sValue;
			psInstruction->asArguments[1] = sQuotient;
		}
	}
	ui = asFound.NumElements();
	asFound.Kill();
	return ui > 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a 32 bit x / c or x % c (c a constant) is better as x / 2^k
// then / m: c = 2^k * m is wide (65536 or more) and m isn't, so the
// target's divide by m is a narrow one (and x / 2^k is a shift).  Not
// when x is known to be below c (BoundDivisions' then).
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::SplitDivisor(SIRType sType, int64 lliDivisor, int64 lliMin, int64 lliMax)
{
	if (!sType.Is(IRTK_I32))
	{
		return false;
	}
	if ((lliDivisor < 0x10000) || ((lliDivisor & 1) != 0))
	{
		return false;
	}
	if ((lliMin <= lliMax) && (lliMin > -lliDivisor) && (lliMax < lliDivisor))
	{
		return false;
	}
	while ((lliDivisor & 1) == 0)
	{
		lliDivisor >>= 1;
	}
	return lliDivisor < 0x10000;
}


//////////////////////////////////////////////////////////////////////////
//
// x % d next to x / d in one block (d not a constant where ConstantDivisions
// is a pass of the pipeline, which has those; the same signedness and type)
// is x - (x / d) * d: one divide
// instead of two, for a target whose divides are runtime calls (a multiply
// is much cheaper).  When the remainder comes first a divide is put before
// it, which common subexpressions make the later one.  C's division rounds
// towards 0, so it holds signed too.  A urem or srem whose divide leaves the
// remainder (LeavesRemainder) is instead moved to right after its udiv or
// sdiv, or the divide is put right before it.  Only that is done where
// ConstantDivisions is no pass of the pipeline: it is one for a target whose
// divides are runtime calls, and is how this pass knows of such a target.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::ShareDivisions(void)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psRemainder;
	SIRInstruction*	psDivide;
	SIRReference	sValue;
	SIRReference	sDivisor;
	SIRReference	sQuotient;
	SIRReference	sProduct;
	SIRType			sType;
	size			ui;
	size			uiRemainder;
	size			uiDivide;
	size			uiAt;
	uint16			uiDivideOpcode;
	bool			bCalls;
	bool			bChanged;

	//The target's divides are calls where divisions by constants are made multiplies.
	bCalls = mpcContext->mpcPipeline->Has(IRP_ConstantDivisions);
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiRemainder = 0; uiRemainder < pcBlock->masInstructions.NumElements(); uiRemainder++)
		{
			psRemainder = pcBlock->masInstructions.Get(uiRemainder);
			if (!IsIRRemainder(psRemainder->uiOpcode) || (bCalls && psRemainder->asArguments[1].IsConstant()) || !psRemainder->sType.IsInteger())
			{
				continue;
			}
			uiDivideOpcode = IRDivisionPartner(psRemainder->uiOpcode);
			sType = psRemainder->sType;
			sValue = psRemainder->asArguments[0];
			sDivisor = psRemainder->asArguments[1];
			for (uiDivide = 0; uiDivide < pcBlock->masInstructions.NumElements(); uiDivide++)
			{
				psDivide = pcBlock->masInstructions.Get(uiDivide);
				if ((psDivide->uiOpcode == uiDivideOpcode) && psDivide->sType.Equals(sType) && psDivide->asArguments[0].Equals(sValue) && psDivide->asArguments[1].Equals(sDivisor))
				{
					break;
				}
			}
			if (uiDivide == pcBlock->masInstructions.NumElements())
			{
				continue;
			}
			if (LeavesRemainder(psRemainder))
			{
				if (uiDivide + 1 == uiRemainder)
				{
					continue;
				}
				if (uiDivide < uiRemainder)
				{
					MoveInstruction(pcBlock, uiRemainder, uiDivide + 1);
				}
				else
				{
					InsertInstruction(pcBlock, uiRemainder, uiDivideOpcode, sType, sValue, sDivisor);
					uiRemainder++;
				}
				bChanged = true;
				continue;
			}
			if (!bCalls)
			{
				continue;
			}
			uiAt = uiRemainder;
			if (uiDivide < uiRemainder)
			{
				sQuotient = pcBlock->masInstructions.Get(uiDivide)->sResult;
			}
			else
			{
				sQuotient = InsertInstruction(pcBlock, uiAt++, uiDivideOpcode, sType, sValue, sDivisor);
			}
			sProduct = InsertInstruction(pcBlock, uiAt++, IRO_Mul, sType, sQuotient, sDivisor);
			psRemainder = pcBlock->masInstructions.Get(uiAt);
			psRemainder->uiOpcode = IRO_Sub;
			psRemainder->asArguments[0] = sValue;
			psRemainder->asArguments[1] = sProduct;
			uiRemainder = uiAt;
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the divide of psRemainder's operands (a urem or srem) leaves the
// remainder for it (SetDivideRemainders).
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::LeavesRemainder(SIRInstruction* psRemainder)
{
	uint32	uiWidth;

	if ((psRemainder->uiOpcode == IRO_SRem) && !(mpcContext->muiDivideRemainders & IR_DIVIDE_REMAINDERS_SIGNED))
	{
		return false;
	}
	switch (psRemainder->sType.IntegerBits())
	{
	case 8:		uiWidth = IR_DIVIDE_REMAINDERS_8;	break;
	case 16:	uiWidth = IR_DIVIDE_REMAINDERS_16;	break;
	case 32:	uiWidth = IR_DIVIDE_REMAINDERS_32;	break;
	case 64:	uiWidth = IR_DIVIDE_REMAINDERS_64;	break;
	default:	return false;
	}
	return (mpcContext->muiDivideRemainders & uiWidth) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcBlock has a uiOpcode of sValue by sDivisor.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::HasDivision(CIRBlock* pcBlock, uint16 uiOpcode, SIRReference sValue, SIRReference sDivisor)
{
	SIRInstruction*	psInstruction;
	size			ui;

	for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		if ((psInstruction->uiOpcode == uiOpcode) && psInstruction->asArguments[0].Equals(sValue) && psInstruction->asArguments[1].Equals(sDivisor))
		{
			return true;
		}
	}
	return false;
}
