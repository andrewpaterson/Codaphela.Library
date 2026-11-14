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
#include "IRKnownBits.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int IRBitLength(uint64 ulliValue)
{
	int	iBits;

	iBits = 0;
	while (ulliValue != 0)
	{
		ulliValue >>= 1;
		iBits++;
	}
	return iBits;
}


//////////////////////////////////////////////////////////////////////////
//
// The largest value of iBits bits (all of them set).
//
//////////////////////////////////////////////////////////////////////////
static uint64 IRAllOnes(int iBits)
{
	return (iBits >= 64) ? (uint64)-1 : ((((uint64)1) << iBits) - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint64 IRLeast(uint64 ulli1, uint64 ulli2)
{
	return (ulli1 < ulli2) ? ulli1 : ulli2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint64 IRGreatest(uint64 ulli1, uint64 ulli2)
{
	return (ulli1 > ulli2) ? ulli1 : ulli2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRKnownBits::Init(void)
{
	mpcFunction = NULL;
	maulliMost.Init();
	maulliRawMost.Init();
	mauiGrowth.Init();
	mapsDefinitions.Init();
	mapcBlocks.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRKnownBits::Kill(void)
{
	mapcBlocks.Kill();
	mapsDefinitions.Kill();
	mauiGrowth.Kill();
	maulliRawMost.Kill();
	maulliMost.Kill();
	mpcFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The largest value of the type: its own for an integer, all 64 bits for
// anything else (nothing is known).
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRKnownBits::FullMost(SIRType sType)
{
	int	iBits;

	iBits = sType.IntegerBits();
	return IRAllOnes(((iBits > 0) && (iBits <= 64)) ? iBits : 64);
}


//////////////////////////////////////////////////////////////////////////
//
// Starts every defined temporary at 0 and raises each to what its
// definition can give until nothing changes, a phi that keeps rising
// widened to its type's largest; then narrows.  A parameter, or a
// temporary nothing defines, may be anything its type holds.
//
//////////////////////////////////////////////////////////////////////////
void CIRKnownBits::Compute(CIRFunction* pcFunction)
{
	size			uiTemporaries;
	size			uiBlock;
	size			ui;
	size			uiTemporary;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	CArrayTemplate<uint8>	auiDefined;
	int				iRound;

	mpcFunction = pcFunction;
	pcFunction->ComputePredecessors();
	uiTemporaries = pcFunction->NumTemporaries();
	maulliMost.FakeSetUsedElements(0);
	maulliMost.GrowToAtLeastNumElements(uiTemporaries, true, 0);
	maulliRawMost.FakeSetUsedElements(0);
	maulliRawMost.GrowToAtLeastNumElements(uiTemporaries, true, 0xFF);
	mauiGrowth.FakeSetUsedElements(0);
	mauiGrowth.GrowToAtLeastNumElements(uiTemporaries, true, 0);
	mapsDefinitions.FakeSetUsedElements(0);
	mapsDefinitions.GrowToAtLeastNumElements(uiTemporaries, true, 0);
	mapcBlocks.FakeSetUsedElements(0);
	mapcBlocks.GrowToAtLeastNumElements(uiTemporaries, true, 0);
	auiDefined.Init();
	auiDefined.GrowToAtLeastNumElements(uiTemporaries, true, 0);
	for (uiTemporary = 0; uiTemporary < uiTemporaries; uiTemporary++)
	{
		*maulliRawMost.Get(uiTemporary) = (uint64)-1;
	}

	for (uiBlock = 0; uiBlock < pcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = pcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			psPhi = pcBlock->masPhis.Get(ui);
			if (psPhi->sResult.IsTemporary() && (psPhi->sResult.Index() < uiTemporaries))
			{
				*auiDefined.Get(psPhi->sResult.Index()) = 1;
			}
		}
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if (psInstruction->sResult.IsTemporary() && (psInstruction->sResult.Index() < uiTemporaries))
			{
				*auiDefined.Get(psInstruction->sResult.Index()) = 1;
				*mapsDefinitions.Get(psInstruction->sResult.Index()) = psInstruction;
				*mapcBlocks.Get(psInstruction->sResult.Index()) = pcBlock;
			}
		}
	}
	for (uiTemporary = 0; uiTemporary < uiTemporaries; uiTemporary++)
	{
		if (!*auiDefined.Get(uiTemporary))
		{
			*maulliMost.Get(uiTemporary) = FullMost(pcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)uiTemporary)));
		}
	}
	auiDefined.Kill();

	while (Round(false))
	{
	}
	for (iRound = 0; iRound < IR_KNOWN_BITS_NARROWING; iRound++)
	{
		if (!Round(true))
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Works out each phi and instruction once.  Rising (bNarrowing false):
// each only rises, and a phi that has risen IR_KNOWN_BITS_GROWTH times goes
// to its type's largest.  Narrowing: each takes what it works out, which
// is no more than it had.  True if any changed.
//
//////////////////////////////////////////////////////////////////////////
bool CIRKnownBits::Round(bool bNarrowing)
{
	size			uiBlock;
	size			ui;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	uint64			ulliMost;
	uint64			ulliFull;
	uint64*			pulliMost;
	uint8*			puiGrowth;
	bool			bChanged;

	bChanged = false;
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			psPhi = pcBlock->masPhis.Get(ui);
			if (!psPhi->sResult.IsTemporary() || (psPhi->sResult.Index() >= maulliMost.NumElements()))
			{
				continue;
			}
			ulliMost = 0;
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				psArgument = psPhi->asArguments.Get(uiArgument);
				ulliMost = IRGreatest(ulliMost, MostOnEdge(psArgument->sValue, psArgument->pcBlock, pcBlock));
			}
			ulliFull = FullMost(psPhi->sType);
			ulliMost = IRLeast(ulliMost, ulliFull);
			pulliMost = maulliMost.Get(psPhi->sResult.Index());
			if (bNarrowing ? (ulliMost < *pulliMost) : (ulliMost > *pulliMost))
			{
				if (!bNarrowing)
				{
					puiGrowth = mauiGrowth.Get(psPhi->sResult.Index());
					(*puiGrowth)++;
					if (*puiGrowth > IR_KNOWN_BITS_GROWTH)
					{
						ulliMost = ulliFull;
					}
				}
				*pulliMost = ulliMost;
				bChanged = true;
			}
		}
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if (!psInstruction->sResult.IsTemporary() || (psInstruction->sResult.Index() >= maulliMost.NumElements()))
			{
				continue;
			}
			ulliMost = InstructionMost(psInstruction, pcBlock);
			*maulliRawMost.Get(psInstruction->sResult.Index()) = ulliMost;
			ulliMost = IRLeast(ulliMost, FullMost(mpcFunction->GetTemporaryType(psInstruction->sResult)));
			pulliMost = maulliMost.Get(psInstruction->sResult.Index());
			if (bNarrowing ? (ulliMost < *pulliMost) : (ulliMost > *pulliMost))
			{
				*pulliMost = ulliMost;
				bChanged = true;
			}
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// The largest a value can be: a constant's own as it is held (narrow and 32
// bit constants are written as their low 32 bits), a temporary's as Compute
// found it, anything else (a symbol's address) all 64 bits.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRKnownBits::Most(SIRReference sValue)
{
	SIRConstant*	psConstant;

	if (sValue.IsConstant())
	{
		psConstant = mpcFunction->GetConstant(sValue);
		if (!psConstant->sType.IsInteger())
		{
			return (uint64)-1;
		}
		if (psConstant->sType.IntegerBits() <= 32)
		{
			return (uint32)psConstant->ulliBits;
		}
		return psConstant->ulliBits;
	}
	if (sValue.IsTemporary() && (sValue.Index() < maulliMost.NumElements()))
	{
		return *maulliMost.Get(sValue.Index());
	}
	return (uint64)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// How many low bits of a value may be set.
//
//////////////////////////////////////////////////////////////////////////
int CIRKnownBits::Bits(SIRReference sValue)
{
	return IRBitLength(Most(sValue));
}


//////////////////////////////////////////////////////////////////////////
//
// The largest the instruction defining sTemporary works out before it is
// truncated to its type.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRKnownBits::RawMost(SIRReference sTemporary)
{
	if (sTemporary.IsTemporary() && (sTemporary.Index() < maulliRawMost.NumElements()))
	{
		return *maulliRawMost.Get(sTemporary.Index());
	}
	return (uint64)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the instruction defining an integer temporary works out a value
// its type holds already, so truncating it changes nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CIRKnownBits::FitsType(SIRReference sTemporary)
{
	SIRType	sType;

	if (!sTemporary.IsTemporary() || (mpcFunction == NULL))
	{
		return false;
	}
	sType = mpcFunction->GetTemporaryType(sTemporary);
	if (!sType.IsInteger())
	{
		return false;
	}
	return RawMost(sTemporary) <= FullMost(sType);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sOperand is sValue, or sValue zero extended or copied (a bound
// on it is a bound on sValue).
//
//////////////////////////////////////////////////////////////////////////
bool CIRKnownBits::Names(SIRReference sOperand, SIRReference sValue)
{
	SIRInstruction*	psDefinition;

	if (sOperand.Equals(sValue))
	{
		return true;
	}
	if (!sOperand.IsTemporary() || (sOperand.Index() >= mapsDefinitions.NumElements()))
	{
		return false;
	}
	psDefinition = *mapsDefinitions.Get(sOperand.Index());
	return psDefinition && ((psDefinition->uiOpcode == IRO_ZExt) || (psDefinition->uiOpcode == IRO_Copy)) && psDefinition->asArguments[0].Equals(sValue);
}


//////////////////////////////////////////////////////////////////////////
//
// ulliMost lowered by the branch from pcFrom to pcTo (one side of it only):
// its condition, a comparison taken one way on this edge, may say sValue is
// below or at most the other operand.  Signed orderings count only for
// operands whose sign bits are 0.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRKnownBits::EdgeBound(SIRReference sValue, CIRBlock* pcFrom, CIRBlock* pcTo, uint64 ulliMost)
{
	SIRJump*		psJump;
	SIRInstruction*	psCompare;
	SIRReference	sLess;
	SIRReference	sMore;
	SIRReference	sTemp;
	uint16			uiOpcode;
	bool			bTrue;
	bool			bStrict;
	uint64			ulliSign;
	uint64			ulliOther;

	psJump = &pcFrom->msJump;
	if ((psJump->uiKind != IRJ_Branch) || (psJump->pcThen == psJump->pcElse) || !psJump->sArgument.IsTemporary() || (psJump->sArgument.Index() >= mapsDefinitions.NumElements()))
	{
		return ulliMost;
	}
	if (psJump->pcThen == pcTo)
	{
		bTrue = true;
	}
	else if (psJump->pcElse == pcTo)
	{
		bTrue = false;
	}
	else
	{
		return ulliMost;
	}
	psCompare = *mapsDefinitions.Get(psJump->sArgument.Index());
	if ((psCompare == NULL) || !IsIRIntegerComparison(psCompare->uiOpcode))
	{
		return ulliMost;
	}

	uiOpcode = bTrue ? psCompare->uiOpcode : IRInvertedComparison(psCompare->uiOpcode);
	if (IsIRSignedComparison(uiOpcode))
	{
		ulliSign = ((uint64)1) << (psCompare->sType.IntegerBits() - 1);
		if ((Most(psCompare->asArguments[0]) >= ulliSign) || (Most(psCompare->asArguments[1]) >= ulliSign))
		{
			return ulliMost;
		}
	}

	//As sLess < sMore (bStrict) or sLess <= sMore.
	switch (uiOpcode)
	{
	case IRO_ULt:
	case IRO_SLt:
		sLess = psCompare->asArguments[0];
		sMore = psCompare->asArguments[1];
		bStrict = true;
		break;
	case IRO_ULe:
	case IRO_SLe:
		sLess = psCompare->asArguments[0];
		sMore = psCompare->asArguments[1];
		bStrict = false;
		break;
	case IRO_UGt:
	case IRO_SGt:
		sLess = psCompare->asArguments[1];
		sMore = psCompare->asArguments[0];
		bStrict = true;
		break;
	case IRO_UGe:
	case IRO_SGe:
		sLess = psCompare->asArguments[1];
		sMore = psCompare->asArguments[0];
		bStrict = false;
		break;
	case IRO_Eq:
		if (Names(psCompare->asArguments[1], sValue))
		{
			sTemp = psCompare->asArguments[0];
		}
		else if (Names(psCompare->asArguments[0], sValue))
		{
			sTemp = psCompare->asArguments[1];
		}
		else
		{
			return ulliMost;
		}
		return IRLeast(ulliMost, Most(sTemp));
	default:
		return ulliMost;
	}
	if (!Names(sLess, sValue))
	{
		return ulliMost;
	}
	ulliOther = Most(sMore);
	if (bStrict)
	{
		if (ulliOther == 0)
		{
			return ulliMost;
		}
		ulliOther--;
	}
	return IRLeast(ulliMost, ulliOther);
}


//////////////////////////////////////////////////////////////////////////
//
// The largest sValue can be where it is used in pcBlock: lowered by the
// branches above it, through blocks with only one predecessor each (so the
// branch's edge is the only way in).
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRKnownBits::MostAt(SIRReference sValue, CIRBlock* pcBlock)
{
	uint64		ulliMost;
	CIRBlock*	pcFrom;
	int			iDepth;

	ulliMost = Most(sValue);
	if (!sValue.IsTemporary())
	{
		return ulliMost;
	}
	for (iDepth = 0; iDepth < IR_KNOWN_BITS_DEPTH; iDepth++)
	{
		if (pcBlock->mapcPredecessors.NumElements() != 1)
		{
			break;
		}
		pcFrom = pcBlock->mapcPredecessors.GetPtr(0);
		if (pcFrom == pcBlock)
		{
			break;
		}
		ulliMost = EdgeBound(sValue, pcFrom, pcBlock, ulliMost);
		pcBlock = pcFrom;
	}
	return ulliMost;
}


//////////////////////////////////////////////////////////////////////////
//
// The largest sValue can be when pcFrom jumps to pcTo (a phi's argument):
// lowered by the jump's own branch, and by those above pcFrom.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRKnownBits::MostOnEdge(SIRReference sValue, CIRBlock* pcFrom, CIRBlock* pcTo)
{
	if (!sValue.IsTemporary() || (pcFrom == NULL))
	{
		return Most(sValue);
	}
	return EdgeBound(sValue, pcFrom, pcTo, MostAt(sValue, pcFrom));
}


//////////////////////////////////////////////////////////////////////////
//
// The largest an instruction in pcBlock works out from its operands'
// largest there, in a register of 32 bits (64 for an i64 result), before
// truncating it to its type.  Operations that may borrow, sign extend,
// overflow the register or bring in high bits from elsewhere give the whole
// register.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRKnownBits::InstructionMost(SIRInstruction* psInstruction, CIRBlock* pcBlock)
{
	uint64			ulliRegister;
	uint64			ulliLeft;
	uint64			ulliRight;
	uint64			ulliMost;
	uint64			ulliAmount;
	int				iRegister;
	SIRReference	sRight;

	if (IsIRIntegerComparison(psInstruction->uiOpcode))
	{
		return 1;
	}
	if (!psInstruction->sType.IsInteger())
	{
		switch (psInstruction->uiOpcode)
		{
		case IRO_FEq:
		case IRO_FNe:
		case IRO_FLt:
		case IRO_FLe:
		case IRO_FGt:
		case IRO_FGe:
		case IRO_FOrdered:
		case IRO_FUnordered:
			return 1;
		default:
			return (uint64)-1;
		}
	}
	iRegister = (psInstruction->sType.IntegerBits() > 32) ? 64 : 32;
	ulliRegister = IRAllOnes(iRegister);

	ulliLeft = MostAt(psInstruction->asArguments[0], pcBlock);
	sRight = psInstruction->asArguments[1];
	ulliRight = sRight.IsNone() ? 0 : MostAt(sRight, pcBlock);
	switch (psInstruction->uiOpcode)
	{
	case IRO_And:
		ulliMost = IRLeast(ulliLeft, ulliRight);
		break;
	case IRO_Or:
	case IRO_Xor:
		ulliMost = IRAllOnes(IRBitLength(IRGreatest(ulliLeft, ulliRight)));
		break;
	case IRO_Add:
		ulliMost = ulliLeft + ulliRight;
		if ((ulliMost < ulliLeft) || (ulliMost > ulliRegister))
		{
			ulliMost = ulliRegister;
		}
		break;
	case IRO_Mul:
		if ((IRBitLength(ulliLeft) + IRBitLength(ulliRight)) > iRegister)
		{
			ulliMost = ulliRegister;
		}
		else
		{
			ulliMost = ulliLeft * ulliRight;
		}
		break;
	case IRO_UDiv:
		ulliMost = ulliLeft;
		break;
	case IRO_URem:
		ulliMost = (ulliRight == 0) ? ulliLeft : IRLeast(ulliLeft, ulliRight - 1);
		break;
	case IRO_LShr:
		ulliMost = ulliLeft;
		if (sRight.IsConstant())
		{
			ulliAmount = mpcFunction->GetConstant(sRight)->ulliBits;
			ulliMost = (ulliAmount >= 64) ? 0 : (ulliLeft >> ulliAmount);
		}
		break;
	case IRO_Shl:
		ulliMost = ulliRegister;
		if (sRight.IsConstant())
		{
			ulliAmount = mpcFunction->GetConstant(sRight)->ulliBits;
			if ((ulliAmount < (uint64)iRegister) && ((IRBitLength(ulliLeft) + (int)ulliAmount) <= iRegister))
			{
				ulliMost = ulliLeft << ulliAmount;
			}
		}
		break;
	case IRO_Copy:
	case IRO_ZExt:
	case IRO_Trunc:
		ulliMost = ulliLeft;
		break;
	case IRO_SExt:
		//A value whose sign bit is known 0 extends as it is.
		ulliMost = (ulliLeft <= (FullMost(mpcFunction->TypeOf(psInstruction->asArguments[0])) >> 1)) ? ulliLeft : ulliRegister;
		break;
	case IRO_Load:
		ulliMost = FullMost(psInstruction->sType);
		break;
	default:
		ulliMost = ulliRegister;
		break;
	}
	return IRLeast(ulliMost, ulliRegister);
}

