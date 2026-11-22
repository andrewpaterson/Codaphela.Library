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
#include "IRMost.h"


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
// The largest value of the type: its own for an integer, all 64 bits for
// anything else (nothing is known).
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRMostRules::FullMost(SIRType sType)
{
	int	iBits;

	iBits = sType.IntegerBits();
	return IRAllOnes(((iBits > 0) && (iBits <= 64)) ? iBits : 64);
}


//////////////////////////////////////////////////////////////////////////
//
// The largest sValue can be where it is used in pcBlock (NULL: anywhere).
// Unless what asks knows the branches above the block, as large as it can
// be anywhere.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRMostRules::MostAt(SIRReference sValue, CIRBlock* pcBlock)
{
	return Most(sValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The largest an instruction's operand can be in the instruction's block;
// 0 for one it doesn't have.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRMostRules::OperandMost(SIRReference sOperand, CIRBlock* pcBlock)
{
	return sOperand.IsNone() ? 0 : MostAt(sOperand, pcBlock);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sOperand is sValue, or sValue zero extended or copied (a bound
// on it is a bound on sValue).
//
//////////////////////////////////////////////////////////////////////////
bool CIRMostRules::Names(SIRReference sOperand, SIRReference sValue)
{
	SIRInstruction*	psDefinition;

	if (sOperand.Equals(sValue))
	{
		return true;
	}
	psDefinition = Definition(sOperand);
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
uint64 CIRMostRules::EdgeMost(SIRReference sValue, CIRBlock* pcFrom, CIRBlock* pcTo, uint64 ulliMost)
{
	SIRJump*			psJump;
	SIRInstruction*		psCompare;
	SIRReference		sLess;
	SIRReference		sMore;
	SIRReference		sTemp;
	uint16				uiOpcode;
	uint8				uiHolds;
	SIREdgeCondition	sCondition;
	bool				bLeftLess;
	bool				bStrict;
	uint64				ulliSign;
	uint64				ulliOther;

	psJump = &pcFrom->msJump;
	psCompare = Definition(psJump->sArgument);
	if (!sCondition.Init(psJump, pcTo, psCompare))
	{
		return ulliMost;
	}

	uiOpcode = sCondition.uiOpcode;
	uiHolds = IRComparisonHolds(uiOpcode);
	if (uiHolds == IRCH_Equal)
	{
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
	}
	if (uiHolds == (IRCH_Less | IRCH_Greater))
	{
		return ulliMost;
	}

	//As sLess < sMore (bStrict) or sLess <= sMore: the left operand is the lesser if the comparison holds for it less.
	bLeftLess = (uiHolds & IRCH_Less) != 0;
	sLess = psCompare->asArguments[bLeftLess ? 0 : 1];
	sMore = psCompare->asArguments[bLeftLess ? 1 : 0];
	bStrict = (uiHolds & IRCH_Equal) == 0;
	if (!Names(sLess, sValue))
	{
		return ulliMost;
	}
	if (IsIRSignedComparison(uiOpcode))
	{
		//Asked only now: what asks may have far to look for how large an operand is.
		ulliSign = ((uint64)1) << (psCompare->sType.IntegerBits() - 1);
		if ((Most(psCompare->asArguments[0]) >= ulliSign) || (Most(psCompare->asArguments[1]) >= ulliSign))
		{
			return ulliMost;
		}
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
// The largest an instruction in pcBlock works out from its operands'
// largest there, in a register of 32 bits (64 for an i64 result), before
// truncating it to its type.  Operations that may borrow, sign extend,
// overflow the register or bring in high bits from elsewhere give the whole
// register.  An operand is only asked about where its opcode reads it.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRMostRules::InstructionMost(CIRFunction* pcFunction, SIRInstruction* psInstruction, CIRBlock* pcBlock)
{
	uint64			ulliRegister;
	uint64			ulliLeft;
	uint64			ulliRight;
	uint64			ulliMost;
	uint64			ulliAmount;
	int				iRegister;
	SIRReference	sLeft;
	SIRReference	sRight;

	if (IsIRIntegerComparison(psInstruction->uiOpcode))
	{
		return 1;
	}
	if (!psInstruction->sType.IsInteger())
	{
		//A float comparison's result is an i1.
		return IRResultIsI1(psInstruction->uiOpcode) ? 1 : (uint64)-1;
	}
	iRegister = (psInstruction->sType.IntegerBits() > 32) ? 64 : 32;
	ulliRegister = IRAllOnes(iRegister);

	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];
	switch (psInstruction->uiOpcode)
	{
	case IRO_And:
		ulliMost = IRLeast(OperandMost(sLeft, pcBlock), OperandMost(sRight, pcBlock));
		break;
	case IRO_Or:
	case IRO_Xor:
		ulliMost = IRAllOnes(IRBitLength(IRGreatest(OperandMost(sLeft, pcBlock), OperandMost(sRight, pcBlock))));
		break;
	case IRO_Add:
		ulliLeft = OperandMost(sLeft, pcBlock);
		ulliMost = ulliLeft + OperandMost(sRight, pcBlock);
		if ((ulliMost < ulliLeft) || (ulliMost > ulliRegister))
		{
			ulliMost = ulliRegister;
		}
		break;
	case IRO_Mul:
		ulliLeft = OperandMost(sLeft, pcBlock);
		ulliRight = OperandMost(sRight, pcBlock);
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
		//By a constant: that many times smaller.
		ulliMost = OperandMost(sLeft, pcBlock);
		if (sRight.IsConstant())
		{
			ulliAmount = IRTruncate(psInstruction->sType, pcFunction->GetConstant(sRight)->ulliBits);
			if (ulliAmount != 0)
			{
				ulliMost /= ulliAmount;
			}
		}
		break;
	case IRO_URem:
		ulliLeft = OperandMost(sLeft, pcBlock);
		ulliRight = OperandMost(sRight, pcBlock);
		ulliMost = (ulliRight == 0) ? ulliLeft : IRLeast(ulliLeft, ulliRight - 1);
		break;
	case IRO_LShr:
		ulliMost = OperandMost(sLeft, pcBlock);
		if (sRight.IsConstant())
		{
			ulliAmount = pcFunction->GetConstant(sRight)->ulliBits;
			ulliMost = (ulliAmount >= 64) ? 0 : (ulliMost >> ulliAmount);
		}
		break;
	case IRO_Shl:
		ulliMost = ulliRegister;
		if (sRight.IsConstant())
		{
			ulliLeft = OperandMost(sLeft, pcBlock);
			ulliAmount = pcFunction->GetConstant(sRight)->ulliBits;
			if ((ulliAmount < (uint64)iRegister) && ((IRBitLength(ulliLeft) + (int)ulliAmount) <= iRegister))
			{
				ulliMost = ulliLeft << ulliAmount;
			}
		}
		break;
	case IRO_Copy:
	case IRO_ZExt:
	case IRO_Trunc:
		ulliMost = OperandMost(sLeft, pcBlock);
		break;
	case IRO_SExt:
		//A value whose sign bit is known 0 extends as it is.
		ulliLeft = OperandMost(sLeft, pcBlock);
		ulliMost = (ulliLeft <= (FullMost(pcFunction->TypeOf(sLeft)) >> 1)) ? ulliLeft : ulliRegister;
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRMost::Init(void)
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
void CIRMost::Kill(void)
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
// Starts every defined temporary at 0 and raises each to what its
// definition can give until nothing changes, a phi that keeps rising
// widened to its type's largest; then narrows.  A parameter, or a
// temporary nothing defines, may be anything its type holds.
//
//////////////////////////////////////////////////////////////////////////
void CIRMost::Compute(CIRFunction* pcFunction)
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
	for (iRound = 0; iRound < IR_MOST_NARROWING; iRound++)
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
// each only rises, and a phi that has risen IR_MOST_GROWTH times goes
// to its type's largest.  Narrowing: each takes what it works out, which
// is no more than it had.  True if any changed.
//
//////////////////////////////////////////////////////////////////////////
bool CIRMost::Round(bool bNarrowing)
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
					if (*puiGrowth > IR_MOST_GROWTH)
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
			ulliMost = InstructionMost(mpcFunction, psInstruction, pcBlock);
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
uint64 CIRMost::Most(SIRReference sValue)
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
// The instruction that defines a temporary, NULL if none does (a phi, a
// parameter) or it isn't one.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIRMost::Definition(SIRReference sValue)
{
	if (!sValue.IsTemporary() || (sValue.Index() >= mapsDefinitions.NumElements()))
	{
		return NULL;
	}
	return *mapsDefinitions.Get(sValue.Index());
}


//////////////////////////////////////////////////////////////////////////
//
// The largest the instruction defining sTemporary works out before it is
// truncated to its type.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRMost::RawMost(SIRReference sTemporary)
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
bool CIRMost::FitsType(SIRReference sTemporary)
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
// The largest sValue can be where it is used in pcBlock: lowered by the
// branches above it, through blocks with only one predecessor each (so the
// branch's edge is the only way in).
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRMost::MostAt(SIRReference sValue, CIRBlock* pcBlock)
{
	uint64		ulliMost;
	CIRBlock*	pcFrom;
	int			iDepth;

	ulliMost = Most(sValue);
	if (!sValue.IsTemporary())
	{
		return ulliMost;
	}
	for (iDepth = 0; iDepth < IR_MOST_DEPTH; iDepth++)
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
		ulliMost = EdgeMost(sValue, pcFrom, pcBlock, ulliMost);
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
uint64 CIRMost::MostOnEdge(SIRReference sValue, CIRBlock* pcFrom, CIRBlock* pcTo)
{
	if (!sValue.IsTemporary() || (pcFrom == NULL))
	{
		return Most(sValue);
	}
	return EdgeMost(sValue, pcFrom, pcTo, MostAt(sValue, pcFrom));
}
