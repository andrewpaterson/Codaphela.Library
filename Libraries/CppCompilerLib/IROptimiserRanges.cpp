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
			if (!sPointer.IsTemporary() || !mpcFunction->TypeOf(sPointer).IsPointer() || !mpcContext->mcRanges.NonNullAt(pcBlock, sPointer, IR_NON_NULL_DEPTH))
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
	SIRValueRange						sRange;
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
			if (!mpcContext->mcRanges.RangeAt(pcBlock, sNarrow, &sRange, IR_RANGE_BRANCHES | IR_RANGE_DOMINATORS))
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
	SIRValueRange	sRange;
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
			if (!mpcContext->mcRanges.RangeAt(pcBlock, sNarrow, &sRange, IR_RANGE_BRANCHES | IR_RANGE_DOMINATORS) || (sRange.lliMin < 0) || (sRange.lliMax > IR_IN_ARRAY_MOST))
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
	CArrayTemplate<uint32>	auiOld;		//Each old instruction's index in asNew (where what it became starts), then the jump's.
	SIRInstruction			sInstruction;
	SIRInstruction*			psWide;
	CIRBlock*				pcBlock;
	SIRType					sType;
	size					ui;
	size					uiInstruction;
	uint32					uiNew;
	bool					bChanged;
	bool					bBlockChanged;

	NeedDefinitions();
	asNew.Init();
	auiOld.Init();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		asNew.FakeSetUsedElements(0);
		auiOld.FakeSetUsedElements(0);
		bBlockChanged = false;
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			uiNew = (uint32)asNew.NumElements();
			auiOld.Add(&uiNew);
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
		uiNew = (uint32)asNew.NumElements();
		auiOld.Add(&uiNew);
		if (bBlockChanged)
		{
			pcBlock->ReplaceInstructions(&asNew, &auiOld);
			bChanged = true;
			FindDefinitions();
		}
	}
	auiOld.Kill();
	asNew.Kill();
	return bChanged;
}
