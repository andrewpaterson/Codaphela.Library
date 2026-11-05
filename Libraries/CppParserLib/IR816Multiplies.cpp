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


//The W65C816 back end's multiplies, divides and remainders: runtime calls (pushed or scratch operands), and
//the wide, byte and table forms.


//////////////////////////////////////////////////////////////////////////
//
// Whether RuntimeCall calls a scratch routine (IR816ScratchRoutine) for
// psInstruction, of two or more words: its operands are copied a word at a
// time to the scratch bytes, the first then the second.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsScratchCall(SIRInstruction* psInstruction)
{
	CChars	szName;
	bool	bScratch;

	if (psInstruction->sType.IsFloat() || (Words(psInstruction->sType) < 2))
	{
		return false;
	}
	RuntimeName(&szName, psInstruction);
	bScratch = IR816ScratchRoutine(szName.Text());
	szName.Kill();
	return bScratch;
}


//////////////////////////////////////////////////////////////////////////
//
// mul, udiv, sdiv, urem and srem call R816_<operation><bits>, an i8 as 16 bits (sign extended for sdiv and srem).
// The scratch routines get their operands at IR816_SCRATCH_ARGUMENT_A and _B, the others pushed.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::RuntimeCall(SIRInstruction* psInstruction)
{
	CChars		szName;
	SIRType		sType;
	int32		iBytes;
	bool		bSigned;
	int			i;

	sType = psInstruction->sType;
	if (sType.IsFloat() || sType.Is(IRTK_I1))
	{
		return Error("Multiply and divide are for i8 to i64");
	}
	bSigned = (psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem);
	if (TableDivide(psInstruction, bSigned) || ByteMultiply(psInstruction) || WideMultiply(psInstruction))
	{
		return true;
	}

	RuntimeName(&szName, psInstruction);

	iBytes = 0;
	if (IR816ScratchRoutine(szName.Text()))
	{
		CopyWords(IR816_SCRATCH_ARGUMENT_A, psInstruction->asArguments[0], Words(sType));
		CopyWords(IR816_SCRATCH_ARGUMENT_B, psInstruction->asArguments[1], Words(sType));
	}
	else
	{
		iBytes = 2 * ValueBytes(sType);
		for (i = 1; i >= 0; i--)
		{
			if (bSigned && sType.Is(IRTK_I8))
			{
				Operand("lda", psInstruction->asArguments[i], 0);
				Line("eor 0x80");
				Line("sec");
				Line("sbc 0x80");
				Line("pha");
			}
			else
			{
				PushValue(psInstruction->asArguments[i], sType);
			}
		}
		if (IsHiddenResult(sType))
		{
			PushFrameAddress(OffsetOf(psInstruction->sResult));
			iBytes += 4;
		}
	}
	UseRuntime(szName.Text());
	szName.Insert(0, "jsl ");
	Line(szName.Text());
	szName.Kill();
	CallerCleanUp(iBytes, psInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction that works out the temporary sValue, or NULL (a
// parameter, a phi, or not a temporary).
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR816Backend::FindDefinition(SIRReference sValue)
{
	size			uiBlock;
	size			ui;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;

	if (!sValue.IsTemporary())
	{
		return NULL;
	}
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if (psInstruction->sResult.Equals(sValue))
			{
				return psInstruction;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the 64 bit sValue's high 32 bits are known to be 0: a constant
// under 2^32, a zext of 32 bits or fewer, an and with one, or an lshr by
// 32 or more.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsHighHalfZero(SIRReference sValue, int iDepth)
{
	SIRInstruction*	psDefinition;
	uint64			ulliAmount;

	if (sValue.IsConstant())
	{
		return (mpcFunction->GetConstant(sValue)->ulliBits >> 32) == 0;
	}
	psDefinition = FindDefinition(sValue);
	if ((psDefinition == NULL) || (iDepth > 2))
	{
		return false;
	}
	switch (psDefinition->uiOpcode)
	{
	case IRO_ZExt:
		return !psDefinition->asArguments[0].IsTemporary() || (mpcFunction->GetTemporaryType(psDefinition->asArguments[0]).Bits() <= 32);
	case IRO_And:
		return IsHighHalfZero(psDefinition->asArguments[0], iDepth + 1) || IsHighHalfZero(psDefinition->asArguments[1], iDepth + 1);
	case IRO_LShr:
		if (!psDefinition->asArguments[1].IsConstant())
		{
			return false;
		}
		ulliAmount = mpcFunction->GetConstant(psDefinition->asArguments[1])->ulliBits & 63;
		return ulliAmount >= 32;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A 64 bit multiply of two values whose high halves are 0 (IsHighHalfZero:
// softfloat's 32 bit halves of a fraction) is R816_UMulWide32 of their low
// halves, much faster than R816_Mul64.  False, with nothing written, for
// any other multiply.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::WideMultiply(SIRInstruction* psInstruction)
{
	int		i;

	if ((psInstruction->uiOpcode != IRO_Mul) || (Words(psInstruction->sType) != 4) ||
		!IsHighHalfZero(psInstruction->asArguments[0], 0) || !IsHighHalfZero(psInstruction->asArguments[1], 0))
	{
		return false;
	}
	CopyWords(IR816_SCRATCH_ARGUMENT_A, psInstruction->asArguments[0], 2);
	CopyWords(IR816_SCRATCH_ARGUMENT_B, psInstruction->asArguments[1], 2);
	UseRuntime(IR816_RUNTIME_PREFIX "UMulWide32");
	Line("jsl " IR816_RUNTIME_PREFIX "UMulWide32");
	if (!psInstruction->sResult.IsNone())
	{
		for (i = 0; i < 4; i++)
		{
			Frame("lda", IR816_SCRATCH_RESULT + 2 * i);
			Store(psInstruction->sResult, i);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A hot i8 divide or remainder by a constant (IRF_Hot, CIROptimiser::
// ConstantDivisions) looked up in R816_STable<c> or R816_UTable<c>: a word
// per byte value, q in its low byte and the remainder in its high byte.
// False, with nothing written, for any other.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::TableDivide(SIRInstruction* psInstruction, bool bSigned)
{
	CChars	szName;
	uint64	ulliDivisor;

	if (!(psInstruction->uiFlags & IRF_Hot) || !psInstruction->sType.Is(IRTK_I8) || !psInstruction->asArguments[1].IsConstant())
	{
		return false;
	}
	ulliDivisor = mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits & 0xFF;
	if ((ulliDivisor < 3) || (bSigned && (ulliDivisor >= 0x80)))
	{
		return false;
	}
	szName.Init(bSigned ? IR816_SIGNED_TABLE : IR816_UNSIGNED_TABLE);
	szName.Append((int)ulliDivisor);
	UseRuntime(szName.Text());
	Operand("lda", psInstruction->asArguments[0], 0);
	Line("asl A");
	Line("tax");
	mpszOut->Append("\t\tlda [")->Append(szName)->Append(" + X]:24\n");
	szName.Kill();
	if ((psInstruction->uiOpcode == IRO_SRem) || (psInstruction->uiOpcode == IRO_URem))
	{
		Line("xba");
	}
	Line("and 0xFF");
	if (!psInstruction->sResult.IsNone())
	{
		Store(psInstruction->sResult, 0);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An i8 multiply: the low byte of the product in MathMult (the word at
// (a * 256 + b) * 2 is a * b, as R816_Mul16 reads it), looked up in place.
// False, with nothing written, for any other multiply.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::ByteMultiply(SIRInstruction* psInstruction)
{
	int32	iHigh;
	int32	iGot;

	if ((psInstruction->uiOpcode != IRO_Mul) || !psInstruction->sType.Is(IRTK_I8))
	{
		return false;
	}
	iHigh = NewLabel();
	iGot = NewLabel();
	Operand("lda", psInstruction->asArguments[1], 0);
	Line("xba");
	Operand("ora", psInstruction->asArguments[0], 0);
	Line("asl A");
	Line("tax");
	Branch("bcs", iHigh);
	Line("lda [MathMult + X]:24");
	Branch("bra", iGot);
	Label(iHigh);
	Line("lda [MathMult + 0x10000 + X]:24");
	Label(iGot);
	Line("and 0xFF");
	if (!psInstruction->sResult.IsNone())
	{
		Store(psInstruction->sResult, 0);
	}
	return true;
}
