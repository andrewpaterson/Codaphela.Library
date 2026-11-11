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
			if (((psInstruction->uiOpcode != IRO_SDiv) && (psInstruction->uiOpcode != IRO_SRem)) || !psInstruction->asArguments[1].IsConstant() ||
				!psInstruction->sType.IsInteger() || psInstruction->sType.Is(IRTK_I1))
			{
				continue;
			}
			sType = psInstruction->sType;
			iBits = sType.IntegerBits();
			lliDivisor = IRSignExtend(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
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
	CIRBlock*							pcBlock;
	SIRInstruction*						psInstruction;
	SIRReference						sValue;
	SIRReference						sDivisor;
	SIRReference						sTest;
	SIRType								sType;
	uint64								ulliDivisor;
	uint64								ulliBound;
	int64								lliMin;
	int64								lliMax;
	uint16								uiOpcode;
	size								ui;
	size								uiInstruction;
	bool								bChanged;

	//Found first: the inserts move the definitions UpperBound reads.
	FindDefinitions();
	mpcFunction->ComputePredecessors();
	mpcContext->mbBranchRanges = true;
	asFound.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			uiOpcode = psInstruction->uiOpcode;
			if (((uiOpcode != IRO_UDiv) && (uiOpcode != IRO_URem) && (uiOpcode != IRO_SDiv) && (uiOpcode != IRO_SRem)) || !psInstruction->asArguments[1].IsConstant() ||
				!psInstruction->sResult.IsTemporary() || !psInstruction->sType.IsInteger())
			{
				continue;
			}
			ulliDivisor = IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
			if ((ulliDivisor < 2) || (ulliDivisor > (IRTruncate(psInstruction->sType, (uint64)-1) >> 1)))
			{
				continue;
			}
			ulliBound = ((uiOpcode == IRO_UDiv) || (uiOpcode == IRO_URem)) ? mpcContext->mpcRangePasses->UpperBound(psInstruction->asArguments[0], IR_BOUND_DEPTH) : (uint64)-1;
			if ((ulliBound >= ulliDivisor * 2) && mpcContext->mpcRangePasses->RangeAt(pcBlock, psInstruction->asArguments[0], &lliMin, &lliMax) && (lliMin >= 0))
			{
				ulliBound = (uint64)lliMax;
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
	mpcContext->mbBranchRanges = false;

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
			Replace(psInstruction->sResult, ((psInstruction->uiOpcode == IRO_URem) || (psInstruction->uiOpcode == IRO_SRem)) ? sValue : mpcFunction->AddInteger(sType, 0));
			continue;
		}
		sTest = InsertInstruction(pcBlock, uiInstruction, IRO_UGe, sType, sValue, sDivisor);
		uiInstruction++;
		if ((pcBlock->masInstructions.Get(uiInstruction)->uiOpcode == IRO_UDiv) || (pcBlock->masInstructions.Get(uiInstruction)->uiOpcode == IRO_SDiv))
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			psInstruction->uiOpcode = IRO_ZExt;
			psInstruction->asArguments[0] = sTest;
			psInstruction->asArguments[1] = IRNone();
		}
		else
		{
			sTest = InsertInstruction(pcBlock, uiInstruction, IRO_ZExt, sType, sTest, IRNone());
			sTest = InsertInstruction(pcBlock, uiInstruction + 1, IRO_Neg, sType, sTest, IRNone());
			sTest = InsertInstruction(pcBlock, uiInstruction + 2, IRO_And, sType, sTest, sDivisor);
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
// lliMax in iBits, else in 32 bits (*piBits says which).  Failing that, for
// an x that is never negative and an even divisor 2^k * m, m's for x >> k
// (*piPreShift is k, else 0): its smaller products fit where x's don't.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::FindReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, uint64* pulliMultiplier, int* piShift, int* piBits, int* piPreShift)
{
	int		iPreShift;
	int		iWork;

	for (iPreShift = 0; ; )
	{
		for (iWork = iBits; iWork <= 32; iWork = (iWork < 32) ? 32 : 33)
		{
			if (DivisionReciprocal(lliMin >> iPreShift, lliMax >> iPreShift, lliDivisor >> iPreShift, iWork, bSigned, pulliMultiplier, piShift))
			{
				*piBits = iWork;
				*piPreShift = iPreShift;
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
	int64			lliMin;
	int64			lliMax;
	size			ui;
	size			uj;
	int				iBits;
	int				iShift;
	int				i;
	bool			bChanged;
	bool			bSigned;

	FindDefinitions();
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
			if (!psRemainder || ((psRemainder->uiOpcode != IRO_SRem) && (psRemainder->uiOpcode != IRO_URem)) || !psRemainder->asArguments[1].IsConstant() ||
				!(psRemainder->sType.Is(IRTK_I16) || psRemainder->sType.Is(IRTK_I32)))
			{
				continue;
			}
			sType = psRemainder->sType;
			iBits = sType.IntegerBits();
			ulliMask = IRTruncate(sType, (uint64)-1);
			bSigned = psRemainder->uiOpcode == IRO_SRem;
			ulliDivisor = bSigned ? (uint64)IRSignExtend(sType, mpcFunction->GetConstant(psRemainder->asArguments[1])->ulliBits) :
									IRTruncate(sType, mpcFunction->GetConstant(psRemainder->asArguments[1])->ulliBits);
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
			if (bSigned && (!mpcContext->mpcRangePasses->SignedRange(sValue, &lliMin, &lliMax) || (lliMin < 0)))
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
// multiply by a constant shifts and adds).  A divide and a remainder of one
// x get the same q, which common subexpressions make one.  An i8 one the
// profile finds hot (IsHot) is marked IRF_Hot and left for the target to
// look up in a table.  x's range is narrowed by the branches to the divide
// (mpcContext->mbBranchRanges).  A 32 bit divide by a wide c = 2^k * m (m under 65536)
// that isn't made a multiply is split (SplitDivisor).
//
//////////////////////////////////////////////////////////////////////////
bool CIRDivisionPasses::ConstantDivisions(void)
{
	CArrayTemplate<SIRBoundDivision>	asFound;
	SIRBoundDivision*					psFound;
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
	int64								lliMin;
	int64								lliMax;
	uint64								ulliMultiplier;
	int									iShift;
	int									iBits;
	int									iPreShift;
	size								ui;
	size								uiInstruction;
	size								uiAt;
	bool								bSigned;
	uint16								uiOpcode;

	FindDefinitions();
	mpcFunction->ComputePredecessors();
	mpcContext->mbBranchRanges = true;
	asFound.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			uiOpcode = psInstruction->uiOpcode;
			if (((uiOpcode != IRO_SDiv) && (uiOpcode != IRO_SRem) && (uiOpcode != IRO_UDiv) && (uiOpcode != IRO_URem)) ||
				!psInstruction->asArguments[1].IsConstant() || !psInstruction->sResult.IsTemporary() || !psInstruction->sType.IsInteger() ||
				(psInstruction->sType.IntegerBits() < 8) || (psInstruction->sType.IntegerBits() > 32))
			{
				continue;
			}
			bSigned = (uiOpcode == IRO_SDiv) || (uiOpcode == IRO_SRem);
			lliDivisor = bSigned ? IRSignExtend(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) :
								   (int64)IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
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
			if (!mpcContext->mpcRangePasses->RangeAt(pcBlock, psInstruction->asArguments[0], &lliMin, &lliMax) || (!bSigned && (lliMin < 0)))
			{
				lliMin = 1;
				lliMax = 0;
			}
			if ((lliMin <= lliMax) && IsByteTableDivision(lliMin, lliMax, lliDivisor) && IsHot(psInstruction, IR_DIVISION_TABLE_SAVING))
			{
				psFound = asFound.Add();
				psFound->pcBlock = pcBlock;
				psFound->uiInstruction = uiInstruction;
				psFound->bOnce = false;
				psFound->bTable = true;
				psFound->bSplit = false;
				psFound->lliMin = lliMin;
				psFound->lliMax = lliMax;
				continue;
			}
			if ((lliMin <= lliMax) && FindReciprocal(lliMin, lliMax, lliDivisor, psInstruction->sType.IntegerBits(), bSigned, &ulliMultiplier, &iShift, &iBits, &iPreShift))
			{
				psFound = asFound.Add();
				psFound->pcBlock = pcBlock;
				psFound->uiInstruction = uiInstruction;
				psFound->bOnce = false;
				psFound->bTable = false;
				psFound->bSplit = false;
				psFound->lliMin = lliMin;
				psFound->lliMax = lliMax;
			}
			else if (SplitDivisor(psInstruction, lliMin, lliMax))
			{
				psFound = asFound.Add();
				psFound->pcBlock = pcBlock;
				psFound->uiInstruction = uiInstruction;
				psFound->bOnce = false;
				psFound->bTable = false;
				psFound->bSplit = true;
				psFound->lliMin = lliMin;
				psFound->lliMax = lliMax;
			}
		}
	}
	mpcContext->mbBranchRanges = false;

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
		bSigned = (uiOpcode == IRO_SDiv) || (uiOpcode == IRO_SRem);
		lliDivisor = bSigned ? IRSignExtend(sType, mpcFunction->GetConstant(sConstant)->ulliBits) : (int64)IRTruncate(sType, mpcFunction->GetConstant(sConstant)->ulliBits);
		lliMin = psFound->lliMin;
		lliMax = psFound->lliMax;
		if (psFound->bSplit)
		{
			for (iShift = 0; ((lliDivisor >> iShift) & 1) == 0; iShift++)
			{
			}
			uiAt = uiInstruction;
			sWide = InsertInstruction(pcBlock, uiAt++, bSigned ? IRO_SDiv : IRO_UDiv, sType, sValue, mpcFunction->AddInteger(sType, (int64)1 << iShift));
			sDivisor = mpcFunction->AddInteger(sType, lliDivisor >> iShift);
			if ((uiOpcode == IRO_SDiv) || (uiOpcode == IRO_UDiv))
			{
				//(x / 2^k) / m.
				psInstruction = pcBlock->masInstructions.Get(uiAt);
				psInstruction->asArguments[0] = sWide;
				psInstruction->asArguments[1] = sDivisor;
			}
			else if (HasDivision(pcBlock, uiOpcode == IRO_SRem ? IRO_SDiv : IRO_UDiv, sValue, sConstant))
			{
				//x - ((x / 2^k) / m) * c: the same q as the divide's.
				sQuotient = InsertInstruction(pcBlock, uiAt++, bSigned ? IRO_SDiv : IRO_UDiv, sType, sWide, sDivisor);
				sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Mul, sType, sQuotient, mpcFunction->AddInteger(sType, lliDivisor));
				psInstruction = pcBlock->masInstructions.Get(uiAt);
				psInstruction->uiOpcode = IRO_Sub;
				psInstruction->asArguments[0] = sValue;
				psInstruction->asArguments[1] = sQuotient;
			}
			else
			{
				//((x / 2^k) % m) << k + x % 2^k (both with x's sign).
				sQuotient = InsertInstruction(pcBlock, uiAt++, bSigned ? IRO_SRem : IRO_URem, sType, sWide, sDivisor);
				sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Shl, sType, sQuotient, mpcFunction->AddInteger(sType, iShift));
				sSign = InsertInstruction(pcBlock, uiAt++, bSigned ? IRO_SRem : IRO_URem, sType, sValue, mpcFunction->AddInteger(sType, (int64)1 << iShift));
				psInstruction = pcBlock->masInstructions.Get(uiAt);
				psInstruction->uiOpcode = IRO_Add;
				psInstruction->asArguments[0] = sQuotient;
				psInstruction->asArguments[1] = sSign;
			}
			continue;
		}
		if (psFound->bTable)
		{
			//The byte's i8 divide (signed only when the byte may be negative), extended back.
			sWork = IRType(IRTK_I8);
			bSigned = lliMin < 0;
			uiAt = uiInstruction;
			sWide = InsertInstruction(pcBlock, uiAt++, IRO_Trunc, sWork, sValue, IRNone());
			if ((uiOpcode == IRO_SDiv) || (uiOpcode == IRO_UDiv))
			{
				uiOpcode = bSigned ? IRO_SDiv : IRO_UDiv;
			}
			else
			{
				uiOpcode = bSigned ? IRO_SRem : IRO_URem;
			}
			sQuotient = InsertInstruction(pcBlock, uiAt++, uiOpcode, sWork, sWide, mpcFunction->AddInteger(sWork, lliDivisor));
			pcBlock->masInstructions.Get(uiAt - 1)->uiFlags |= IRF_Hot;
			psInstruction = pcBlock->masInstructions.Get(uiAt);
			psInstruction->uiOpcode = bSigned ? IRO_SExt : IRO_ZExt;
			psInstruction->uiFlags = 0;
			psInstruction->asArguments[0] = sQuotient;
			psInstruction->asArguments[1] = IRNone();
			continue;
		}
		FindReciprocal(lliMin, lliMax, lliDivisor, sType.IntegerBits(), bSigned, &ulliMultiplier, &iShift, &iBits, &iPreShift);
		sWork = (iBits == sType.IntegerBits()) ? sType : IRType(IRTK_I32);

		uiAt = uiInstruction;
		sWide = sValue;
		if (sWork.uiKind != sType.uiKind)
		{
			sWide = InsertInstruction(pcBlock, uiAt++, bSigned ? IRO_SExt : IRO_ZExt, sWork, sValue, IRNone());
		}
		if (iPreShift > 0)
		{
			//x is never negative: (x >> k) / m for c = 2^k * m.
			sWide = InsertInstruction(pcBlock, uiAt++, IRO_LShr, sWork, sWide, mpcFunction->AddInteger(sWork, iPreShift));
		}
		sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Mul, sWork, sWide, mpcFunction->AddInteger(sWork, (int64)ulliMultiplier));
		sQuotient = InsertInstruction(pcBlock, uiAt++, bSigned ? IRO_AShr : IRO_LShr, sWork, sQuotient, mpcFunction->AddInteger(sWork, iShift));
		if (bSigned && (lliMin < 0))
		{
			sSign = InsertInstruction(pcBlock, uiAt++, IRO_AShr, sWork, sWide, mpcFunction->AddInteger(sWork, sWork.IntegerBits() - 1));
			sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Sub, sWork, sQuotient, sSign);
		}
		if (sWork.uiKind != sType.uiKind)
		{
			sQuotient = InsertInstruction(pcBlock, uiAt++, IRO_Trunc, sType, sQuotient, IRNone());
		}
		psInstruction = pcBlock->masInstructions.Get(uiAt);
		if ((uiOpcode == IRO_SDiv) || (uiOpcode == IRO_UDiv))
		{
			//q.
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
bool CIRDivisionPasses::SplitDivisor(SIRInstruction* psInstruction, int64 lliMin, int64 lliMax)
{
	int64	lliDivisor;
	bool	bSigned;

	if (!psInstruction->sType.Is(IRTK_I32))
	{
		return false;
	}
	bSigned = (psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem);
	lliDivisor = bSigned ? IRSignExtend(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) :
						   (int64)IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
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
// x % d next to x / d in one block (d not a constant with
// SetConstantDivisions: ConstantDivisions has those; the same signedness and
// type) is x - (x / d) * d: one divide
// instead of two, for a target whose divides are runtime calls (a multiply
// is much cheaper).  When the remainder comes first a divide is put before
// it, which common subexpressions make the later one.  C's division rounds
// towards 0, so it holds signed too.  A urem or srem whose divide leaves the
// remainder (LeavesRemainder) is instead moved to right after its udiv or
// sdiv, or the divide is put right before it.  Without SetConstantDivisions
// only that is done.
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
	bool			bChanged;

	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiRemainder = 0; uiRemainder < pcBlock->masInstructions.NumElements(); uiRemainder++)
		{
			psRemainder = pcBlock->masInstructions.Get(uiRemainder);
			if (((psRemainder->uiOpcode != IRO_URem) && (psRemainder->uiOpcode != IRO_SRem)) || (mpcContext->mbConstantDivisions && psRemainder->asArguments[1].IsConstant()) || !psRemainder->sType.IsInteger())
			{
				continue;
			}
			uiDivideOpcode = (psRemainder->uiOpcode == IRO_SRem) ? IRO_SDiv : IRO_UDiv;
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
			if (!mpcContext->mbConstantDivisions)
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
