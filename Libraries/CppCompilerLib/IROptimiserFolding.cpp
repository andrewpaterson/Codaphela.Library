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


//The optimiser's folding: constants folded, identities and operations that cancel removed, comparisons decided by the
//branches before them, and common subexpressions removed.


//////////////////////////////////////////////////////////////////////////
//
// The bits of an instruction whose operands are integer (or pointer)
// constants, wrapping as the IR does.  False if it can't be worked out (a
// divide by zero, floats, an instruction with an effect).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::Fold(SIRInstruction* psInstruction, uint64* pulliResult)
{
	SIROpcode*	psOpcode;
	SIRType		sType;
	SIRType		sFrom;
	uint64		a;
	uint64		b;
	int64		sa;
	int64		sb;
	int			iBits;

	psOpcode = GetIROpcode(psInstruction->uiOpcode);
	if (!psOpcode || (psOpcode->uiShape == IRSH_Call) || !psInstruction->sResult.IsTemporary())
	{
		return false;
	}
	if (!psInstruction->asArguments[0].IsConstant() || ((psOpcode->NumOperands() == 2) && !psInstruction->asArguments[1].IsConstant()))
	{
		return false;
	}
	sType = psInstruction->sType;
	sFrom = mpcFunction->TypeOf(psInstruction->asArguments[0]);
	if (sType.IsFloat() || sFrom.IsFloat())
	{
		return false;
	}
	a = mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits;
	sa = IRSignExtend(sFrom, a);
	b = 0;
	sb = 0;
	if (psOpcode->NumOperands() == 2)
	{
		b = mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
		sb = IRSignExtend(sType, b);
	}
	iBits = sType.IsPointer() ? 8 * mpcModule->GetPointerSize(sType.uiSpace) : sType.IntegerBits();

	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:		*pulliResult = a + b;	break;
	case IRO_Sub:		*pulliResult = a - b;	break;
	case IRO_Mul:		*pulliResult = a * b;	break;
	case IRO_And:		*pulliResult = a & b;	break;
	case IRO_Or:		*pulliResult = a | b;	break;
	case IRO_Xor:		*pulliResult = a ^ b;	break;
	case IRO_Neg:		*pulliResult = (uint64)-(int64)a;	break;
	case IRO_Not:		*pulliResult = ~a;	break;
	case IRO_Copy:		*pulliResult = a;	break;
	case IRO_SDiv:
	case IRO_SRem:
		if ((sb == 0) || ((sb == -1) && (sa == (int64)(((uint64)1) << (iBits - 1)) * ((iBits == 64) ? 1 : -1))))
		{
			return false;
		}
		*pulliResult = (psInstruction->uiOpcode == IRO_SDiv) ? (uint64)(sa / sb) : (uint64)(sa % sb);
		break;
	case IRO_UDiv:
	case IRO_URem:
		if (b == 0)
		{
			return false;
		}
		*pulliResult = (psInstruction->uiOpcode == IRO_UDiv) ? a / b : a % b;
		break;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		if (b >= (uint64)iBits)
		{
			return false;
		}
		if (psInstruction->uiOpcode == IRO_Shl)
		{
			*pulliResult = a << b;
		}
		else if (psInstruction->uiOpcode == IRO_LShr)
		{
			*pulliResult = a >> b;
		}
		else
		{
			*pulliResult = (uint64)(sa >> b);
		}
		break;
	case IRO_Eq:
	case IRO_Ne:
	case IRO_ULt:
	case IRO_ULe:
	case IRO_UGt:
	case IRO_UGe:
	case IRO_SLt:
	case IRO_SLe:
	case IRO_SGt:
	case IRO_SGe:
		*pulliResult = IRCompareIntegers(psInstruction->uiOpcode, sType, a, b);
		break;
	case IRO_SExt:	*pulliResult = (uint64)sa;	break;
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
		*pulliResult = a;
		break;
	default:
		return false;
	}
	if (sType.IsPointer() && (iBits < 64))
	{
		//A pointer is as wide as its space (IRTruncate doesn't know the module's sizes).
		*pulliResult &= (((uint64)1) << iBits) - 1;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Instructions of constants are their constant results, and so are
// comparisons a branch on the way in decides (ImpliedComparison); x * 2^n,
// unsigned x / 2^n and x % 2^n are shifts and ands; comparisons of
// extended values are narrowed (NarrowComparison), extensions that are
// undone go (UndoesExtension, ConvertPhi) and so do x + 0 and the like
// (IsIdentity) and sums that cancel ((x + y) - y, Cancels); a pointer's
// constant offsets are added together (PointerOffsets).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::FoldConstants(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	uint64			ulliResult;
	SIRType			sResultType;
	SIRReference	sValue;
	bool			bChanged;
	int				iShift;

	ClearReplacements();
	FindDefinitions();
	mpcFunction->ComputePredecessors();
	mpcFunction->CountUses(&mauiUses);
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			psInstruction->asArguments[0] = Replacement(psInstruction->asArguments[0]);
			psInstruction->asArguments[1] = Replacement(psInstruction->asArguments[1]);
			if (IsIRCommutative(psInstruction->uiOpcode) && psInstruction->asArguments[0].IsConstant() && !psInstruction->asArguments[1].IsConstant())
			{
				//A constant goes on the right (320 * y is y * 320), where the rules below and the back ends look for it.
				sValue = psInstruction->asArguments[0];
				psInstruction->asArguments[0] = psInstruction->asArguments[1];
				psInstruction->asArguments[1] = sValue;
				bChanged = true;
			}
			if (NarrowComparison(psInstruction))
			{
				bChanged = true;
			}
			if (Fold(psInstruction, &ulliResult) || ImpliedComparison(pcBlock, psInstruction, &ulliResult, &bChanged))
			{
				sResultType = mpcFunction->GetTemporaryType(psInstruction->sResult);
				Replace(psInstruction->sResult, mpcFunction->AddConstant(sResultType, ulliResult));
				bChanged = true;
			}
			else if (UndoesExtension(psInstruction, &sValue) || ConvertPhi(psInstruction, &sValue) || IsIdentity(psInstruction, &sValue) || Cancels(psInstruction, &sValue))
			{
				//The value's uses go up by the result's (ConvertPhi mustn't change a phi something else now uses).
				Replace(psInstruction->sResult, sValue);
				if (sValue.IsTemporary() && psInstruction->sResult.IsTemporary())
				{
					*mauiUses.Get(sValue.Index()) += *mauiUses.Get(psInstruction->sResult.Index());
				}
				bChanged = true;
			}
			else if (PointerOffset(psInstruction) || PointerOffsets(psInstruction))
			{
				bChanged = true;
			}
			else if ((psInstruction->uiOpcode == IRO_SExt) && (mpcFunction->TypeOf(psInstruction->asArguments[0]).IntegerBits() > 1) &&
					 (Extended(psInstruction->asArguments[0], mpcFunction->TypeOf(psInstruction->asArguments[0]).IntegerBits() - 1, false, 0) ||
					  NotNegative(pcBlock, psInstruction->asArguments[0])))
			{
				//A sext of a value whose top bit is 0 (or that the branches to here keep 0 or more) is a zext (cheaper
				//where a sign must be worked out).
				psInstruction->uiOpcode = IRO_ZExt;
				bChanged = true;
			}
			else if (((psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem)) && psInstruction->asArguments[1].IsConstant() &&
					 (IRSignExtend(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) > 0) &&
					 NotNegative(pcBlock, psInstruction->asArguments[0]))
			{
				//A signed x / c or x % c is unsigned when x is never negative (and c is more than 0).
				psInstruction->uiOpcode = (psInstruction->uiOpcode == IRO_SDiv) ? IRO_UDiv : IRO_URem;
				bChanged = true;
			}
			else if (((psInstruction->uiOpcode == IRO_Mul) || (psInstruction->uiOpcode == IRO_UDiv) || (psInstruction->uiOpcode == IRO_URem)) && psInstruction->asArguments[1].IsConstant())
			{
				//x * 2^n is x << n; unsigned x / 2^n is x >> n and x % 2^n is x & (2^n - 1); x * -1 is -x.
				ulliResult = IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
				if ((psInstruction->uiOpcode == IRO_Mul) && (ulliResult == IRTruncate(psInstruction->sType, (uint64)-1)) && (ulliResult > 1))
				{
					psInstruction->uiOpcode = IRO_Neg;
					psInstruction->asArguments[1] = IRNone();
					bChanged = true;
				}
				else if ((ulliResult > 1) && ((ulliResult & (ulliResult - 1)) == 0))
				{
					iShift = (int)FindFirstSetBit(ulliResult);
					if (psInstruction->uiOpcode == IRO_URem)
					{
						psInstruction->uiOpcode = IRO_And;
						psInstruction->asArguments[1] = mpcFunction->AddInteger(psInstruction->sType, (int64)(ulliResult - 1));
					}
					else
					{
						psInstruction->uiOpcode = (psInstruction->uiOpcode == IRO_Mul) ? IRO_Shl : IRO_LShr;
						psInstruction->asArguments[1] = mpcFunction->AddInteger(psInstruction->sType, iShift);
					}
					bChanged = true;
				}
			}
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
// An itoptr of the sum of a ptrtoi and an integer, p turned into an integer
// and back with an offset added, becomes ptradd p, offset (a pointer add
// needs no conversions: the W65C816 masks an itoptr's top byte).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::PointerOffset(SIRInstruction* psInstruction)
{
	SIRInstruction*	psAdd;
	SIRInstruction*	psToInteger;
	SIRReference	sPointer;
	SIRType			sType;
	int				i;

	if ((psInstruction->uiOpcode != IRO_IToPtr) || !psInstruction->sResult.IsTemporary())
	{
		return false;
	}
	sType = mpcFunction->GetTemporaryType(psInstruction->sResult);
	psAdd = mcDefinitions.Instruction(Replacement(psInstruction->asArguments[0]));
	if (!psAdd || (psAdd->uiOpcode != IRO_Add) || !psAdd->sType.Equals(IRIntegerType(mpcModule->GetPointerSize(sType.uiSpace))))
	{
		return false;
	}
	for (i = 0; i < 2; i++)
	{
		psToInteger = mcDefinitions.Instruction(Replacement(psAdd->asArguments[i]));
		if (psToInteger && (psToInteger->uiOpcode == IRO_PtrToI))
		{
			sPointer = Replacement(psToInteger->asArguments[0]);
			if (mpcFunction->TypeOf(sPointer).Equals(sType))
			{
				psInstruction->uiOpcode = IRO_PtrAdd;
				psInstruction->uiFlags = 0;
				psInstruction->sType = sType;
				psInstruction->asArguments[0] = sPointer;
				psInstruction->asArguments[1] = Replacement(psAdd->asArguments[1 - i]);
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A ptradd of a constant to a ptradd of a constant (a member's member: an
// object's array's block) becomes one ptradd of their sum to the inner's
// pointer.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::PointerOffsets(SIRInstruction* psInstruction)
{
	SIRInstruction*	psInner;
	SIRType			sOffsetType;

	if ((psInstruction->uiOpcode != IRO_PtrAdd) || !psInstruction->asArguments[1].IsConstant())
	{
		return false;
	}
	psInner = mcDefinitions.Instruction(psInstruction->asArguments[0]);
	if (!psInner || (psInner->uiOpcode != IRO_PtrAdd) || !psInner->asArguments[1].IsConstant())
	{
		return false;
	}
	sOffsetType = mpcFunction->TypeOf(psInstruction->asArguments[1]);
	psInstruction->asArguments[1] = mpcFunction->AddInteger(sOffsetType, mpcFunction->GetSigned(psInner->asArguments[1]) + mpcFunction->GetSigned(psInstruction->asArguments[1]));
	psInstruction->asArguments[0] = Replacement(psInner->asArguments[0]);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if psInstruction gives back a value it was made from: a trunc of an
// extension of a value of the trunc's type, or a sext (zext) of a trunc of
// a value already sign (zero) extended from the trunc's width (Extended).
// *psValue is that value.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::UndoesExtension(SIRInstruction* psInstruction, SIRReference* psValue)
{
	SIRInstruction*	psDefinition;
	SIRType			sResultType;
	SIRReference	sInner;

	if (!IsIRIntegerConversion(psInstruction->uiOpcode) || !psInstruction->sResult.IsTemporary())
	{
		return false;
	}
	psDefinition = mcDefinitions.Instruction(psInstruction->asArguments[0]);
	if (psDefinition == NULL)
	{
		return false;
	}
	sResultType = mpcFunction->GetTemporaryType(psInstruction->sResult);
	sInner = psDefinition->asArguments[0];
	if (!sResultType.IsInteger() || !mpcFunction->TypeOf(sInner).Equals(sResultType))
	{
		return false;
	}
	if (psInstruction->uiOpcode == IRO_Trunc)
	{
		if ((psDefinition->uiOpcode != IRO_SExt) && (psDefinition->uiOpcode != IRO_ZExt))
		{
			return false;
		}
	}
	else if ((psDefinition->uiOpcode != IRO_Trunc) || !Extended(sInner, mpcFunction->TypeOf(psInstruction->asArguments[0]).IntegerBits(), psInstruction->uiOpcode == IRO_SExt, 0))
	{
		return false;
	}
	*psValue = sInner;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if psInstruction gives back an operand (*psValue): x + 0, x - 0,
// x | 0, x ^ 0, a shift by 0, x * 1, x / 1, x & all ones, ptradd x, 0 (and 0 + x
// and the like for a commutative op), or the constant: x * 0, x & 0.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::IsIdentity(SIRInstruction* psInstruction, SIRReference* psValue)
{
	uint64			ulliBits;
	SIRReference	sValue;
	SIRReference	sConstant;

	sValue = psInstruction->asArguments[0];
	sConstant = psInstruction->asArguments[1];
	if (sValue.IsConstant() && IsIRCommutative(psInstruction->uiOpcode))
	{
		//0 + x is x + 0.
		sValue = psInstruction->asArguments[1];
		sConstant = psInstruction->asArguments[0];
	}
	if (!psInstruction->sResult.IsTemporary() || !sConstant.IsConstant() ||
		!mpcFunction->TypeOf(sValue).Equals(mpcFunction->GetTemporaryType(psInstruction->sResult)))
	{
		return false;
	}
	ulliBits = IRTruncate(mpcFunction->TypeOf(sConstant), mpcFunction->GetConstant(sConstant)->ulliBits);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_Sub:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
	case IRO_PtrAdd:
		if (ulliBits != 0)
		{
			return false;
		}
		break;
	case IRO_Mul:
	case IRO_And:
		if (ulliBits == 0)
		{
			//x * 0 and x & 0 are the 0.
			*psValue = sConstant;
			return true;
		}
		if (ulliBits != ((psInstruction->uiOpcode == IRO_Mul) ? 1 : IRTruncate(psInstruction->sType, (uint64)-1)))
		{
			return false;
		}
		break;
	case IRO_UDiv:
	case IRO_SDiv:
		if (ulliBits != 1)
		{
			return false;
		}
		break;
	default:
		return false;
	}
	*psValue = sValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if psInstruction adds and takes away the same value: (x + y) - y,
// (y + x) - y, x - (x - y), x + (y - x), (y - x) + x, and p + (q - p) (an offset is as
// wide as a pointer), or turns a pointer into an integer and back (itoptr of ptrtoi p).
// The arithmetic wraps, so it is exact.  *psValue is what is left.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::Cancels(SIRInstruction* psInstruction, SIRReference* psValue)
{
	SIRInstruction*	psDefinition;
	SIRInstruction*	psTo;
	SIRInstruction*	psFrom;
	SIRReference	sValue;
	SIRType			sType;
	int				i;

	if (!psInstruction->sResult.IsTemporary())
	{
		return false;
	}
	sType = mpcFunction->GetTemporaryType(psInstruction->sResult);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Sub:
		//x - (x - y).
		psDefinition = mcDefinitions.Instruction(Replacement(psInstruction->asArguments[1]));
		if (psDefinition && (psDefinition->uiOpcode == IRO_Sub) && psDefinition->sType.Equals(psInstruction->sType) &&
			Replacement(psDefinition->asArguments[0]).Equals(psInstruction->asArguments[0]))
		{
			sValue = Replacement(psDefinition->asArguments[1]);
			if (mpcFunction->TypeOf(sValue).Equals(sType))
			{
				*psValue = sValue;
				return true;
			}
		}

		//(x + y) - y or (y + x) - y.
		psDefinition = mcDefinitions.Instruction(Replacement(psInstruction->asArguments[0]));
		if (psDefinition && (psDefinition->uiOpcode == IRO_Add) && psDefinition->sType.Equals(psInstruction->sType))
		{
			for (i = 0; i < 2; i++)
			{
				if (Replacement(psDefinition->asArguments[1 - i]).Equals(psInstruction->asArguments[1]))
				{
					sValue = Replacement(psDefinition->asArguments[i]);
					if (mpcFunction->TypeOf(sValue).Equals(sType))
					{
						*psValue = sValue;
						return true;
					}
				}
			}
		}
		return false;

	case IRO_Add:
		//x + (y - x) or (y - x) + x.
		for (i = 0; i < 2; i++)
		{
			psDefinition = mcDefinitions.Instruction(Replacement(psInstruction->asArguments[i]));
			if (psDefinition && (psDefinition->uiOpcode == IRO_Sub) && psDefinition->sType.Equals(psInstruction->sType) &&
				Replacement(psDefinition->asArguments[1]).Equals(psInstruction->asArguments[1 - i]))
			{
				sValue = Replacement(psDefinition->asArguments[0]);
				if (mpcFunction->TypeOf(sValue).Equals(sType))
				{
					*psValue = sValue;
					return true;
				}
			}
		}
		return false;

	case IRO_IToPtr:
		//A pointer turned into an integer and back.
		psDefinition = mcDefinitions.Instruction(Replacement(psInstruction->asArguments[0]));
		if (!psDefinition || (psDefinition->uiOpcode != IRO_PtrToI))
		{
			return false;
		}
		sValue = Replacement(psDefinition->asArguments[0]);
		if (!mpcFunction->TypeOf(sValue).Equals(sType))
		{
			return false;
		}
		*psValue = sValue;
		return true;

	case IRO_PtrAdd:
		//p + (q - p).
		psDefinition = mcDefinitions.Instruction(Replacement(psInstruction->asArguments[1]));
		if (!psDefinition || (psDefinition->uiOpcode != IRO_Sub))
		{
			return false;
		}
		psTo = mcDefinitions.Instruction(Replacement(psDefinition->asArguments[0]));
		psFrom = mcDefinitions.Instruction(Replacement(psDefinition->asArguments[1]));
		if (!psTo || !psFrom || (psTo->uiOpcode != IRO_PtrToI) || (psFrom->uiOpcode != IRO_PtrToI) ||
			!Replacement(psFrom->asArguments[0]).Equals(psInstruction->asArguments[0]))
		{
			return false;
		}
		sValue = Replacement(psTo->asArguments[0]);
		if (!mpcFunction->TypeOf(sValue).Equals(sType))
		{
			return false;
		}
		*psValue = sValue;
		return true;

	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A sext, zext or trunc of a phi of constants that nothing else uses
// changes the phi into one of the converted constants (so a ret of it can
// go to each predecessor: DuplicateReturns).  *psValue is the phi.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ConvertPhi(SIRInstruction* psInstruction, SIRReference* psValue)
{
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRConstant*	psConstant;
	SIRType			sResultType;
	uint64			ulliBits;
	size			ui;

	if (!IsIRIntegerConversion(psInstruction->uiOpcode) || !psInstruction->sResult.IsTemporary() || !psInstruction->asArguments[0].IsTemporary())
	{
		return false;
	}
	psPhi = mcDefinitions.Phi(psInstruction->asArguments[0]);
	sResultType = mpcFunction->GetTemporaryType(psInstruction->sResult);
	if ((psPhi == NULL) || !psPhi->sType.IsInteger() || !sResultType.IsInteger() || (*mauiUses.Get(psPhi->sResult.Index()) != 1))
	{
		return false;
	}
	for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
	{
		if (!psPhi->asArguments.Get(ui)->sValue.IsConstant())
		{
			return false;
		}
	}

	for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
	{
		psArgument = psPhi->asArguments.Get(ui);
		psConstant = mpcFunction->GetConstant(psArgument->sValue);
		ulliBits = (psInstruction->uiOpcode == IRO_SExt) ? (uint64)IRSignExtend(psConstant->sType, psConstant->ulliBits) : IRTruncate(psConstant->sType, psConstant->ulliBits);
		psArgument->sValue = mpcFunction->AddConstant(sResultType, IRTruncate(sResultType, ulliBits));
	}
	psPhi->sType = sResultType;
	mpcFunction->SetTemporaryType(psPhi->sResult, sResultType);
	*mauiUses.Get(psPhi->sResult.Index()) = 0;
	*psValue = psPhi->sResult;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is the sign (bSigned) or zero extension of its low iBits:
// a constant that fits, an extension from no more bits (a zext from fewer
// for signed), an and with a mask that fits, or a phi of such.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::Extended(SIRReference sValue, int iBits, bool bSigned, int iDepth)
{
	SIRConstant*	psConstant;
	SIRInstruction*	psDefinition;
	SIRPhi*			psPhi;
	int64			lliValue;
	int				iFrom;
	size			ui;

	if ((iBits <= 0) || (iBits >= 64))
	{
		return false;
	}
	if (sValue.IsConstant())
	{
		psConstant = mpcFunction->GetConstant(sValue);
		if (bSigned)
		{
			lliValue = IRSignExtend(psConstant->sType, psConstant->ulliBits);
			return (lliValue >= IRSignedMin(iBits)) && (lliValue <= IRSignedMax(iBits));
		}
		return IRTruncate(psConstant->sType, psConstant->ulliBits) <= (uint64)IRUnsignedMax(iBits);
	}
	if (!sValue.IsTemporary() || (iDepth > IR_EXTENDED_DEPTH))
	{
		return false;
	}
	psDefinition = mcDefinitions.Instruction(sValue);
	if (psDefinition)
	{
		iFrom = mpcFunction->TypeOf(psDefinition->asArguments[0]).IntegerBits();
		switch (psDefinition->uiOpcode)
		{
		case IRO_SExt:
			return bSigned && (iFrom > 0) && (iFrom <= iBits);
		case IRO_ZExt:
			return (iFrom > 0) && (bSigned ? (iFrom < iBits) : (iFrom <= iBits));
		case IRO_And:
			return psDefinition->asArguments[1].IsConstant() && Extended(psDefinition->asArguments[1], bSigned ? iBits - 1 : iBits, false, iDepth + 1);
		default:
			return false;
		}
	}
	psPhi = mcDefinitions.Phi(sValue);
	if (psPhi == NULL)
	{
		return false;
	}
	for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
	{
		if (!Extended(psPhi->asArguments.Get(ui)->sValue, iBits, bSigned, iDepth + 1))
		{
			return false;
		}
	}
	return true;
}


//What each integer comparison (from IRO_Eq) says of its left operand against its right: IR_REL_LESS, _EQUAL and
//_GREATER; and whether it is signed (1), unsigned (2) or either (0).
#define IR_REL_LESS		1
#define IR_REL_EQUAL	2
#define IR_REL_GREATER	4
#define IR_REL_ALL		(IR_REL_LESS | IR_REL_EQUAL | IR_REL_GREATER)
static int	gaiIRRelations[] =		{ IR_REL_EQUAL, IR_REL_LESS | IR_REL_GREATER,
									  IR_REL_LESS, IR_REL_LESS | IR_REL_EQUAL, IR_REL_GREATER, IR_REL_GREATER | IR_REL_EQUAL,
									  IR_REL_LESS, IR_REL_LESS | IR_REL_EQUAL, IR_REL_GREATER, IR_REL_GREATER | IR_REL_EQUAL };
static int	gaiIRSignedness[] =		{ 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 };


//////////////////////////////////////////////////////////////////////////
//
// The relations of a comparison with its operands swapped.
//
//////////////////////////////////////////////////////////////////////////
static int IROptimiserSwapRelations(int iRelations)
{
	return (iRelations & IR_REL_EQUAL) | ((iRelations & IR_REL_LESS) ? IR_REL_GREATER : 0) | ((iRelations & IR_REL_GREATER) ? IR_REL_LESS : 0);
}


//////////////////////////////////////////////////////////////////////////
//
// What pcPredecessor's branch into pcBlock says of psInstruction's operands
// (IR_REL_ bits), if it branches on a comparison of them of the same type
// and signedness; -1 if it doesn't.
//
//////////////////////////////////////////////////////////////////////////
int CIROptimiser::EntryRelations(CIRBlock* pcPredecessor, CIRBlock* pcBlock, SIRInstruction* psInstruction)
{
	SIRJump*			psJump;
	SIRInstruction*		psKnown;
	int					iKnown;
	int					iSigned;
	int					iKnownSigned;

	psJump = &pcPredecessor->msJump;
	psKnown = ((psJump->uiKind == IRJ_Branch) && (psJump->pcThen != psJump->pcElse)) ? mcDefinitions.Instruction(psJump->sArgument) : NULL;
	if (!psKnown || !IsIRIntegerComparison(psKnown->uiOpcode) || !psKnown->sType.Equals(psInstruction->sType))
	{
		return -1;
	}
	iSigned = gaiIRSignedness[psInstruction->uiOpcode - IRO_Eq];
	iKnownSigned = gaiIRSignedness[psKnown->uiOpcode - IRO_Eq];
	if ((iSigned != 0) && (iKnownSigned != 0) && (iSigned != iKnownSigned))
	{
		return -1;
	}

	if (psKnown->asArguments[0].Equals(psInstruction->asArguments[0]) && psKnown->asArguments[1].Equals(psInstruction->asArguments[1]))
	{
		iKnown = gaiIRRelations[psKnown->uiOpcode - IRO_Eq];
	}
	else if (psKnown->asArguments[0].Equals(psInstruction->asArguments[1]) && psKnown->asArguments[1].Equals(psInstruction->asArguments[0]))
	{
		iKnown = IROptimiserSwapRelations(gaiIRRelations[psKnown->uiOpcode - IRO_Eq]);
	}
	else
	{
		return -1;
	}
	return (psJump->pcElse == pcBlock) ? (IR_REL_ALL & ~iKnown) : iKnown;
}


//////////////////////////////////////////////////////////////////////////
//
// An integer comparison in pcBlock that a branch on the way in (along
// single predecessors) already decides is a constant (returns true with
// *pulliResult); one it narrows to eq or ne is changed into that (*pbChanged
// set): after !(a < b), a > b is a != b.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ImpliedComparison(CIRBlock* pcBlock, SIRInstruction* psInstruction, uint64* pulliResult, bool* pbChanged)
{
	size				ui;
	CIRBlock*			pcPredecessor;
	int					iKnown;
	int					iWanted;
	uint16				uiOpcode;

	if (!IsIRIntegerComparison(psInstruction->uiOpcode))
	{
		return false;
	}
	iWanted = gaiIRRelations[psInstruction->uiOpcode - IRO_Eq];
	for (ui = 0; ui < IR_KNOWN_DEPTH; ui++)
	{
		if ((pcBlock == mpcFunction->GetEntry()) || (pcBlock->mapcPredecessors.NumElements() != 1))
		{
			return false;
		}
		pcPredecessor = pcBlock->mapcPredecessors.GetPtr(0);
		iKnown = EntryRelations(pcPredecessor, pcBlock, psInstruction);
		if (iKnown < 0)
		{
			pcBlock = pcPredecessor;
			continue;
		}

		if ((iKnown & ~iWanted) == 0)
		{
			*pulliResult = 1;
			return true;
		}
		if ((iKnown & iWanted) == 0)
		{
			*pulliResult = 0;
			return true;
		}
		uiOpcode = psInstruction->uiOpcode;
		if ((iKnown & iWanted) == (iKnown & IR_REL_EQUAL))
		{
			uiOpcode = IRO_Eq;
		}
		else if ((iKnown & iWanted) == (iKnown & (IR_REL_LESS | IR_REL_GREATER)))
		{
			uiOpcode = IRO_Ne;
		}
		if (uiOpcode != psInstruction->uiOpcode)
		{
			psInstruction->uiOpcode = uiOpcode;
			*pbChanged = true;
		}
		return false;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if two instructions work out the same value (either way round for
// a commutative opcode).
//
//////////////////////////////////////////////////////////////////////////
bool IROptimiserSameInstruction(CIRFunction* pcFunction, SIRInstruction* psEarlier, SIRInstruction* psInstruction)
{
	bool	bSame;
	bool	bSwapped;

	if ((psEarlier->uiOpcode != psInstruction->uiOpcode) || !psEarlier->sType.Equals(psInstruction->sType) || (psEarlier->uiFlags != psInstruction->uiFlags) ||
		!psEarlier->sResult.IsTemporary() || !pcFunction->GetTemporaryType(psEarlier->sResult).Equals(pcFunction->GetTemporaryType(psInstruction->sResult)))
	{
		return false;
	}
	bSame = psEarlier->asArguments[0].Equals(psInstruction->asArguments[0]) && psEarlier->asArguments[1].Equals(psInstruction->asArguments[1]);
	bSwapped = IsIRCommutative(psInstruction->uiOpcode) && psEarlier->asArguments[0].Equals(psInstruction->asArguments[1]) && psEarlier->asArguments[1].Equals(psInstruction->asArguments[0]);
	return bSame || bSwapped;
}


//////////////////////////////////////////////////////////////////////////
//
// An instruction without effects (and not a load) the same as one earlier in
// its block or in a block that dominates it is the earlier one's result.
// The search is cut short in very large functions.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RemoveCommonSubexpressions(void)
{
	size			ui;
	size			uiInstruction;
	size			uiEarlier;
	size			uiEnd;
	size			uiSearched;
	CIRBlock*		pcBlock;
	CIRBlock*		pcDominator;
	SIRInstruction*	psInstruction;
	SIRInstruction*	psEarlier;
	bool			bChanged;
	bool			bFound;

	mpcFunction->Renumber();
	mpcFunction->ComputePredecessors();
	mcDominators.Compute(mpcFunction);
	ClearReplacements();
	bChanged = false;
	for (ui = 0; ui < mcDominators.NumReachable(); ui++)
	{
		//Reverse post order: dominators' instructions are already replaced.
		pcBlock = mcDominators.GetReversePostOrder(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			psInstruction->asArguments[0] = Replacement(psInstruction->asArguments[0]);
			psInstruction->asArguments[1] = Replacement(psInstruction->asArguments[1]);
			if (!IsIRPure(psInstruction->uiOpcode) || (psInstruction->uiOpcode == IRO_Slot) || !psInstruction->sResult.IsTemporary())
			{
				continue;
			}
			bFound = false;
			uiSearched = 0;
			uiEnd = uiInstruction;
			for (pcDominator = pcBlock; pcDominator && !bFound && (uiSearched < IR_CSE_SEARCH_LIMIT); pcDominator = mcDominators.GetImmediate(pcDominator))
			{
				for (uiEarlier = 0; (uiEarlier < uiEnd) && (uiSearched < IR_CSE_SEARCH_LIMIT); uiEarlier++)
				{
					uiSearched++;
					psEarlier = pcDominator->masInstructions.Get(uiEarlier);
					if (IROptimiserSameInstruction(mpcFunction, psEarlier, psInstruction))
					{
						Replace(psInstruction->sResult, psEarlier->sResult);
						bChanged = true;
						bFound = true;
						break;
					}
				}
				if (mcDominators.GetImmediate(pcDominator))
				{
					uiEnd = mcDominators.GetImmediate(pcDominator)->masInstructions.NumElements();
				}
			}
		}
	}
	if (bChanged)
	{
		ApplyReplacements();
	}
	return bChanged;
}
