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
//
//////////////////////////////////////////////////////////////////////////
void CIRKnownBits::Init(void)
{
	mpcFunction = NULL;
	mauiBits.Init();
	mauiRawBits.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRKnownBits::Kill(void)
{
	mauiRawBits.Kill();
	mauiBits.Kill();
	mpcFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The bits a value of the type may have: its own for an integer, 64 for
// anything else (nothing is known).
//
//////////////////////////////////////////////////////////////////////////
int CIRKnownBits::FullBits(SIRType sType)
{
	int	iBits;

	iBits = sType.IntegerBits();
	return ((iBits > 0) && (iBits <= 64)) ? iBits : 64;
}


//////////////////////////////////////////////////////////////////////////
//
// Starts every defined temporary at 0 bits and raises each to what its
// definition can give until nothing changes (each only ever rises, so this
// ends).  A parameter, or a temporary nothing defines, may be anything its
// type holds.
//
//////////////////////////////////////////////////////////////////////////
void CIRKnownBits::Compute(CIRFunction* pcFunction)
{
	size			uiTemporaries;
	size			uiBlock;
	size			ui;
	size			uiArgument;
	size			uiTemporary;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	CArrayTemplate<uint8>	auiDefined;
	bool			bChanged;
	int				iBits;
	int				iFull;
	int				iArgument;
	uint8*			puiBits;

	mpcFunction = pcFunction;
	uiTemporaries = pcFunction->NumTemporaries() + 1;
	mauiBits.FakeSetUsedElements(0);
	mauiBits.GrowToAtLeastNumElements(uiTemporaries, true, 0);
	mauiRawBits.FakeSetUsedElements(0);
	mauiRawBits.GrowToAtLeastNumElements(uiTemporaries, true, 64);
	auiDefined.Init();
	auiDefined.GrowToAtLeastNumElements(uiTemporaries, true, 0);

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
			}
		}
	}
	for (uiTemporary = 0; uiTemporary < pcFunction->NumTemporaries(); uiTemporary++)
	{
		if (!*auiDefined.Get(uiTemporary))
		{
			*mauiBits.Get(uiTemporary) = (uint8)FullBits(pcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)uiTemporary)));
		}
	}
	auiDefined.Kill();

	do
	{
		bChanged = false;
		for (uiBlock = 0; uiBlock < pcFunction->NumBlocks(); uiBlock++)
		{
			pcBlock = pcFunction->GetBlock(uiBlock);
			for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
			{
				psPhi = pcBlock->masPhis.Get(ui);
				if (!psPhi->sResult.IsTemporary() || (psPhi->sResult.Index() >= uiTemporaries))
				{
					continue;
				}
				iBits = 0;
				for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
				{
					iArgument = Bits(psPhi->asArguments.Get(uiArgument)->sValue);
					iBits = (iArgument > iBits) ? iArgument : iBits;
				}
				iFull = FullBits(psPhi->sType);
				iBits = (iBits < iFull) ? iBits : iFull;
				puiBits = mauiBits.Get(psPhi->sResult.Index());
				if (iBits > *puiBits)
				{
					*puiBits = (uint8)iBits;
					bChanged = true;
				}
			}
			for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
			{
				psInstruction = pcBlock->masInstructions.Get(ui);
				if (!psInstruction->sResult.IsTemporary() || (psInstruction->sResult.Index() >= uiTemporaries))
				{
					continue;
				}
				iBits = InstructionBits(psInstruction);
				*mauiRawBits.Get(psInstruction->sResult.Index()) = (uint8)iBits;
				iFull = FullBits(mpcFunction->GetTemporaryType(psInstruction->sResult));
				iBits = (iBits < iFull) ? iBits : iFull;
				puiBits = mauiBits.Get(psInstruction->sResult.Index());
				if (iBits > *puiBits)
				{
					*puiBits = (uint8)iBits;
					bChanged = true;
				}
			}
		}
	}
	while (bChanged);
}


//////////////////////////////////////////////////////////////////////////
//
// The bits a value may have: a constant's own as it is held (narrow and 32
// bit constants are written as their low 32 bits), a temporary's as Compute
// found them, anything else (a symbol's address) all of them.
//
//////////////////////////////////////////////////////////////////////////
int CIRKnownBits::Bits(SIRReference sValue)
{
	SIRConstant*	psConstant;

	if (sValue.IsConstant())
	{
		psConstant = mpcFunction->GetConstant(sValue);
		if (!psConstant->sType.IsInteger())
		{
			return 64;
		}
		if (psConstant->sType.IntegerBits() <= 32)
		{
			return IRBitLength((uint32)psConstant->ulliBits);
		}
		return IRBitLength(psConstant->ulliBits);
	}
	if (sValue.IsTemporary() && (sValue.Index() < mauiBits.NumElements()))
	{
		return *mauiBits.Get(sValue.Index());
	}
	return 64;
}


//////////////////////////////////////////////////////////////////////////
//
// The bits the instruction defining sTemporary works out before it is
// truncated to its type.
//
//////////////////////////////////////////////////////////////////////////
int CIRKnownBits::RawBits(SIRReference sTemporary)
{
	if (sTemporary.IsTemporary() && (sTemporary.Index() < mauiRawBits.NumElements()))
	{
		return *mauiRawBits.Get(sTemporary.Index());
	}
	return 64;
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
	return RawBits(sTemporary) <= sType.IntegerBits();
}


//////////////////////////////////////////////////////////////////////////
//
// The bits an instruction works out from its operands' bits, in a register
// of 32 bits (64 for an i64 result), before truncating it to its type.
// Operations that may borrow, sign extend or bring in high bits from
// elsewhere give the whole register.
//
//////////////////////////////////////////////////////////////////////////
int CIRKnownBits::InstructionBits(SIRInstruction* psInstruction)
{
	int				iRegister;
	int				iLeft;
	int				iRight;
	int				iBits;
	int				iFrom;
	uint64			ulliAmount;
	SIRReference	sRight;

	if (!psInstruction->sType.IsInteger())
	{
		if (IsIRIntegerComparison(psInstruction->uiOpcode))
		{
			return 1;
		}
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
			return 64;
		}
	}
	iRegister = (psInstruction->sType.IntegerBits() > 32) ? 64 : 32;
	if (IsIRIntegerComparison(psInstruction->uiOpcode))
	{
		return 1;
	}

	iLeft = Bits(psInstruction->asArguments[0]);
	sRight = psInstruction->asArguments[1];
	iRight = sRight.IsNone() ? 0 : Bits(sRight);
	switch (psInstruction->uiOpcode)
	{
	case IRO_And:
		iBits = (iLeft < iRight) ? iLeft : iRight;
		break;
	case IRO_Or:
	case IRO_Xor:
		iBits = (iLeft > iRight) ? iLeft : iRight;
		break;
	case IRO_Add:
		iBits = ((iLeft > iRight) ? iLeft : iRight) + 1;
		break;
	case IRO_Mul:
		iBits = iLeft + iRight;
		break;
	case IRO_UDiv:
		iBits = iLeft;
		break;
	case IRO_URem:
		iBits = (iLeft < iRight) ? iLeft : iRight;
		break;
	case IRO_LShr:
		iBits = iLeft;
		if (sRight.IsConstant())
		{
			ulliAmount = mpcFunction->GetConstant(sRight)->ulliBits;
			iBits = (ulliAmount >= (uint64)iLeft) ? 0 : (iLeft - (int)ulliAmount);
		}
		break;
	case IRO_Shl:
		iBits = iRegister;
		if (sRight.IsConstant())
		{
			ulliAmount = mpcFunction->GetConstant(sRight)->ulliBits;
			if (ulliAmount < (uint64)iRegister)
			{
				iBits = iLeft + (int)ulliAmount;
			}
		}
		break;
	case IRO_Copy:
	case IRO_ZExt:
	case IRO_Trunc:
		iBits = iLeft;
		break;
	case IRO_SExt:
		//A value whose sign bit is known 0 extends as it is.
		iFrom = FullBits(mpcFunction->TypeOf(psInstruction->asArguments[0]));
		iBits = (iLeft < iFrom) ? iLeft : iRegister;
		break;
	case IRO_Load:
		iBits = psInstruction->sType.IntegerBits();
		break;
	default:
		iBits = iRegister;
		break;
	}
	return (iBits < iRegister) ? iBits : iRegister;
}

