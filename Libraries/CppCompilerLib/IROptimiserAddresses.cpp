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
#include "IROptimiserInternal.h"


//The optimiser's address passes: address phis merged into one base and index, signed indices biased, and a loop's
//addresses split into a base and an offset.


//////////////////////////////////////////////////////////////////////////
//
// Per temporary: its uses as the address of a load or store of at most 2
// bytes (not volatile, and not the value a store writes).  mpcContext->mauiUses must be
// counted; pauiAddressUses must be initialised.
//
//////////////////////////////////////////////////////////////////////////
void CIRAddressPasses::CountAddressUses(CArrayTemplate<uint32>* pauiAddressUses)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sPointer;
	size			ui;
	size			uj;
	uint32			uiZero;

	uiZero = 0;
	for (ui = 0; ui < mpcContext->mauiUses.NumElements(); ui++)
	{
		pauiAddressUses->Add(&uiZero);
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if ((psInstruction->uiOpcode != IRO_Load) && (psInstruction->uiOpcode != IRO_Store))
			{
				continue;
			}
			sPointer = psInstruction->asArguments[(psInstruction->uiOpcode == IRO_Load) ? 0 : 1];
			if (sPointer.IsTemporary() && (sPointer.Index() < pauiAddressUses->NumElements()) && !(psInstruction->uiFlags & IRF_Volatile) &&
				psInstruction->sType.IsInteger() && (psInstruction->sType.IntegerBits() <= 16) &&
				((psInstruction->uiOpcode == IRO_Load) || !psInstruction->asArguments[0].Equals(sPointer)))
			{
				(*pauiAddressUses->Get(sPointer.Index()))++;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The i16 index psValue (a phi's value) adds to sBase: x when it is
// ptradd sBase, sext(x) with x an i16, or c when it is ptradd sBase, c
// with c fitting 16 bits.  IRNone if neither.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRAddressPasses::AddressPhiIndex(SIRReference sValue, SIRReference sBase)
{
	SIRInstruction*	psAdd;
	SIRInstruction*	psExtend;
	int64			lliOffset;

	psAdd = sValue.IsTemporary() ? mpcContext->mcDefinitions.Instruction(sValue) : NULL;
	if ((psAdd == NULL) || (psAdd->uiOpcode != IRO_PtrAdd) || !psAdd->asArguments[0].Equals(sBase))
	{
		return IRNone();
	}
	if (psAdd->asArguments[1].IsConstant())
	{
		lliOffset = mpcFunction->GetSigned(psAdd->asArguments[1]);
		return ((lliOffset >= -32768) && (lliOffset <= 32767)) ? mpcFunction->AddInteger(IRType(IRTK_I16), lliOffset) : IRNone();
	}
	psExtend = psAdd->asArguments[1].IsTemporary() ? mpcContext->mcDefinitions.Instruction(psAdd->asArguments[1]) : NULL;
	if ((psExtend == NULL) || (psExtend->uiOpcode != IRO_SExt) || !mpcFunction->TypeOf(psExtend->asArguments[0]).Is(IRTK_I16))
	{
		return IRNone();
	}
	return psExtend->asArguments[0];
}


//////////////////////////////////////////////////////////////////////////
//
// A phi of pointers that are one base plus i16 indices (AddressPhiIndex),
// used only as the addresses of loads and stores of at most 2 bytes, is the
// base plus a phi of the indices: p = phi(b + sext(x), b + sext(y)) is
// b + sext(phi(x, y)), so BiasSignedIndices can put the index in Y.  A
// null value (the pointer can't be used where it is null) gives the index
// phi 0.  The base must be worked out before the phi's block.
//
//////////////////////////////////////////////////////////////////////////
bool CIRAddressPasses::MergeAddressPhis(void)
{
	CArrayTemplate<uint32>	auiAddressUses;
	CArrayIRReferences		asIndices;
	CIRBlock*				pcBlock;
	CIRBlock*				pcBaseBlock;
	SIRPhi*					psPhi;
	SIRPhi*					psIndexPhi;
	SIRInstruction*			psAdd;
	SIRReference			sBase;
	SIRReference			sValue;
	SIRReference			sIndex;
	SIRReference			sPhi;
	SIRReference			sOffset;
	SIRReference			sAddress;
	SIRReference			sResult;
	SIRType					sOffsetType;
	size					ui;
	size					uj;
	size					uk;
	bool					bChanged;
	bool					bAll;

	mpcFunction->Renumber();
	mpcFunction->ComputePredecessors();
	mpcModule->Count(CPPCC_Dominators);
	mpcContext->mcDominators.Compute(mpcFunction);
	mpcFunction->CountUses(&mpcContext->mauiUses);
	FindDefinitions();
	auiAddressUses.Init();
	CountAddressUses(&auiAddressUses);
	asIndices.Init();
	ClearReplacements();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masPhis.NumElements(); uj++)
		{
			psPhi = pcBlock->masPhis.Get(uj);
			sResult = psPhi->sResult;
			if (!psPhi->sType.Is(IRTK_Pointer) || (psPhi->asArguments.NumElements() < 2) || (sResult.Index() >= auiAddressUses.NumElements()) ||
				(*mpcContext->mauiUses.Get(sResult.Index()) == 0) || (*mpcContext->mauiUses.Get(sResult.Index()) != *auiAddressUses.Get(sResult.Index())))
			{
				continue;
			}

			//The base: the first value's that isn't null.
			sBase = IRNone();
			sOffsetType = IRType(IRTK_None);
			for (uk = 0; uk < psPhi->asArguments.NumElements(); uk++)
			{
				sValue = psPhi->asArguments.Get(uk)->sValue;
				psAdd = sValue.IsTemporary() ? mpcContext->mcDefinitions.Instruction(sValue) : NULL;
				if (psAdd && (psAdd->uiOpcode == IRO_PtrAdd))
				{
					sBase = psAdd->asArguments[0];
					sOffsetType = mpcFunction->TypeOf(psAdd->asArguments[1]);
					break;
				}
			}
			pcBaseBlock = sBase.IsTemporary() ? mpcContext->mcDefinitions.Block(sBase) : NULL;
			if (sBase.IsNone() || (pcBaseBlock && ((pcBaseBlock == pcBlock) || !mpcContext->mcDominators.Dominates(pcBaseBlock, pcBlock))) || !sOffsetType.IsInteger())
			{
				continue;
			}

			bAll = true;
			asIndices.ReInit();
			for (uk = 0; bAll && (uk < psPhi->asArguments.NumElements()); uk++)
			{
				sValue = psPhi->asArguments.Get(uk)->sValue;
				if (sValue.IsConstant() && (mpcFunction->GetConstant(sValue)->ulliBits == 0))
				{
					sIndex = mpcFunction->AddInteger(IRType(IRTK_I16), 0);
				}
				else
				{
					sIndex = AddressPhiIndex(sValue, sBase);
					psAdd = sValue.IsTemporary() ? mpcContext->mcDefinitions.Instruction(sValue) : NULL;
					bAll = !sIndex.IsNone() && psAdd && mpcFunction->TypeOf(psAdd->asArguments[1]).Equals(sOffsetType);
				}
				asIndices.Add(&sIndex);
			}
			if (!bAll)
			{
				continue;
			}

			//The index phi (AddPhi may move the phis), then b + sext(it) at the top of the block.
			sPhi = mpcFunction->AddTemporary(IRType(IRTK_I16));
			psIndexPhi = pcBlock->AddPhi(IRType(IRTK_I16), sPhi);
			psPhi = pcBlock->masPhis.Get(uj);
			for (uk = 0; uk < psPhi->asArguments.NumElements(); uk++)
			{
				pcBlock->AddPhiArgument(psIndexPhi, psPhi->asArguments.Get(uk)->pcBlock, *asIndices.Get(uk));
				psPhi = pcBlock->masPhis.Get(uj);
				psIndexPhi = pcBlock->masPhis.Get(pcBlock->masPhis.NumElements() - 1);
			}
			sOffset = InsertInstruction(pcBlock, 0, IRO_SExt, sOffsetType, sPhi, IRNone());
			sAddress = InsertInstruction(pcBlock, 1, IRO_PtrAdd, IRType(IRTK_Pointer), sBase, sOffset);
			Replace(sResult, sAddress);
			bChanged = true;
		}
	}
	asIndices.Kill();
	auiAddressUses.Kill();
	if (bChanged)
	{
		ApplyReplacements();
		FindDefinitions();
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// p + sext(x), x an i16, used only as the address of loads and stores of
// at most 2 bytes, is (p - 32768) + zext(x ^ 32768): the target adds the
// zero extended index in a register as it loads (CIR816Backend::
// FoldIndices), and p - 32768 is worked out once (outside the loop, when p
// doesn't change in it).  A scaled index p + sext(x) * s (a shl or mul by
// a constant) with x from lo to hi, (hi - lo) * s under 65536, is
// (p + lo * s) + zext((x - lo) * s), worked out in 16 bits; a constant
// added after it (p + sext(x) * s + c, the sum used only by that add) goes
// in the base.
//
//////////////////////////////////////////////////////////////////////////
bool CIRAddressPasses::BiasSignedIndices(void)
{
	CArrayTemplate<uint32>	auiAddressUses;		//Per temporary: its uses as a 2 byte load's or store's address.
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRInstruction*			psInner;
	SIRInstruction*			psExtend;
	SIRReference			sIndex;
	SIRReference			sBase;
	SIRReference			sOffset;
	SIRReference			sBiased;
	SIRType					sOffsetType;
	SIRType					sShort;
	int64					lliScale;
	int64					lliScaleBy;
	int64					lliExtra;
	int64					lliBias;
	int64					lliMin;
	int64					lliMax;
	size					ui;
	size					uj;
	uint16					uiScaleOpcode;
	bool					bChanged;

	mpcFunction->CountUses(&mpcContext->mauiUses);
	FindDefinitions();
	mpcFunction->ComputePredecessors();
	auiAddressUses.Init();
	CountAddressUses(&auiAddressUses);
	sShort = IRType(IRTK_I16);

	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if ((psInstruction->uiOpcode != IRO_PtrAdd) || !psInstruction->sResult.IsTemporary() ||
				(psInstruction->sResult.Index() >= auiAddressUses.NumElements()) || (*mpcContext->mauiUses.Get(psInstruction->sResult.Index()) == 0) ||
				(*mpcContext->mauiUses.Get(psInstruction->sResult.Index()) != *auiAddressUses.Get(psInstruction->sResult.Index())))
			{
				continue;
			}
			sBase = psInstruction->asArguments[0];
			sOffset = psInstruction->asArguments[1];
			lliExtra = 0;
			if (sOffset.IsConstant() && sBase.IsTemporary() && (sBase.Index() < mpcContext->mauiUses.NumElements()) && (*mpcContext->mauiUses.Get(sBase.Index()) == 1))
			{
				//(p + sext(x) * s) + c.
				psInner = mpcContext->mcDefinitions.Instruction(sBase);
				lliExtra = mpcFunction->GetSigned(sOffset);
				if (!psInner || (psInner->uiOpcode != IRO_PtrAdd) || !psInner->asArguments[1].IsTemporary() || (lliExtra < -0x8000) || (lliExtra > 0x7FFF))
				{
					continue;
				}
				sBase = psInner->asArguments[0];
				sOffset = psInner->asArguments[1];
			}
			if (!sOffset.IsTemporary())
			{
				continue;
			}
			lliScale = 1;
			lliScaleBy = 1;
			uiScaleOpcode = IRO_Mul;
			psExtend = mpcContext->mcDefinitions.Instruction(sOffset);
			if (psExtend && ((psExtend->uiOpcode == IRO_Shl) || (psExtend->uiOpcode == IRO_Mul)) && psExtend->asArguments[1].IsConstant())
			{
				//Kept by value, as the inserts below move the block's instructions.
				uiScaleOpcode = psExtend->uiOpcode;
				lliScaleBy = mpcFunction->GetSigned(psExtend->asArguments[1]);
				lliScale = lliScaleBy;
				if ((uiScaleOpcode == IRO_Shl) && (lliScale >= 0) && (lliScale < 16))
				{
					lliScale = (int64)1 << lliScale;
				}
				else if ((uiScaleOpcode == IRO_Shl) || (lliScale < 1) || (lliScale > 0x7FFF))
				{
					continue;
				}
				psExtend = psExtend->asArguments[0].IsTemporary() ? mpcContext->mcDefinitions.Instruction(psExtend->asArguments[0]) : NULL;
			}
			if ((psExtend == NULL) || (psExtend->uiOpcode != IRO_SExt) || !mpcFunction->TypeOf(psExtend->asArguments[0]).Is(IRTK_I16))
			{
				continue;
			}
			sIndex = psExtend->asArguments[0];
			sOffsetType = mpcFunction->TypeOf(sOffset);
			if ((lliScale == 1) && (lliExtra == 0))
			{
				sBiased = InsertInstruction(pcBlock, uj++, IRO_PtrAdd, IRType(IRTK_Pointer), sBase, mpcFunction->AddInteger(sOffsetType, -32768));
				sIndex = InsertInstruction(pcBlock, uj++, IRO_Xor, sShort, sIndex, mpcFunction->AddInteger(sShort, 0x8000));
			}
			else
			{
				mpcContext->mbBranchRanges = true;
				if (!mpcContext->mpcRangePasses->RangeAt(pcBlock, sIndex, &lliMin, &lliMax))
				{
					lliMin = -0x8000;
					lliMax = 0x7FFF;
				}
				mpcContext->mbBranchRanges = false;
				if ((lliMax - lliMin) * lliScale > 0xFFFF)
				{
					continue;
				}

				//A constant after the index goes in it while it fits (so p + sext(x) * s and p + sext(x) * s + 1 share
				//a base), else in the base; so does a constant p is from another pointer.
				lliBias = -lliMin * lliScale;
				if ((lliExtra >= 0) && ((lliMax - lliMin) * lliScale + lliExtra <= 0xFFFF))
				{
					lliBias += lliExtra;
					lliExtra = 0;
				}
				psInner = sBase.IsTemporary() ? mpcContext->mcDefinitions.Instruction(sBase) : NULL;
				if (psInner && (psInner->uiOpcode == IRO_PtrAdd) && psInner->asArguments[1].IsConstant())
				{
					lliExtra += mpcFunction->GetSigned(psInner->asArguments[1]);
					sBase = psInner->asArguments[0];
				}
				sBiased = sBase;
				if (lliExtra + lliMin * lliScale != 0)
				{
					sBiased = InsertInstruction(pcBlock, uj++, IRO_PtrAdd, IRType(IRTK_Pointer), sBase, mpcFunction->AddInteger(sOffsetType, lliExtra + lliMin * lliScale));
				}
				if (lliScale != 1)
				{
					sIndex = InsertInstruction(pcBlock, uj++, uiScaleOpcode, sShort, sIndex, mpcFunction->AddInteger(sShort, (uiScaleOpcode == IRO_Shl) ? lliScaleBy : lliScale));
				}
				if (lliBias != 0)
				{
					sIndex = InsertInstruction(pcBlock, uj++, IRO_Add, sShort, sIndex, mpcFunction->AddInteger(sShort, lliBias));
				}
			}
			sIndex = InsertInstruction(pcBlock, uj++, IRO_ZExt, sOffsetType, sIndex, IRNone());
			psInstruction = pcBlock->masInstructions.Get(uj);
			psInstruction->asArguments[0] = sBiased;
			psInstruction->asArguments[1] = sIndex;
			psInstruction->uiFlags &= ~(IRF_InBank | IRF_InArray);
			bChanged = true;

			//The inserts moved the block's instructions.
			FindDefinitions();
		}
	}
	auiAddressUses.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// In a loop, ptradd p, (a + b) with p and a the same each time round (and
// the add only used there) is ptradd (ptradd p, a), b: the inner ptradd is
// then hoisted out of the loop (HoistInvariants), leaving one add a time
// round instead of two.
//
//////////////////////////////////////////////////////////////////////////
bool CIRAddressPasses::SplitLoopAddresses(void)
{
	CArrayTemplate<SIRSplitAddress>	asFound;
	SIRSplitAddress*				psFound;
	CIRBlock*						pcBlock;
	SIRInstruction*					psAddress;
	SIRInstruction*					psAdd;
	SIRReference					sInner;
	int32							iLoop;
	size							ui;
	size							uiInstruction;
	int								i;
	bool							bChanged;

	//Found first: the inserts move the definitions IsInvariant reads.
	ComputeLoops();
	asFound.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		iLoop = mpcContext->mcLoops.Innermost(pcBlock);
		if (iLoop < 0)
		{
			continue;
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psAddress = pcBlock->masInstructions.Get(uiInstruction);
			if ((psAddress->uiOpcode != IRO_PtrAdd) || (psAddress->uiFlags != 0) || !psAddress->asArguments[1].IsTemporary() ||
				!mpcContext->mcLoops.IsInvariant((size)iLoop, psAddress->asArguments[0], &mpcContext->mcDefinitions) || (mpcContext->mcDefinitions.NumUses(psAddress->asArguments[1]) != 1))
			{
				continue;
			}
			psAdd = mpcContext->mcDefinitions.Instruction(psAddress->asArguments[1]);
			if (!psAdd || (psAdd->uiOpcode != IRO_Add))
			{
				continue;
			}
			for (i = 0; i < 2; i++)
			{
				if (!psAdd->asArguments[i].IsConstant() && mpcContext->mcLoops.IsInvariant((size)iLoop, psAdd->asArguments[i], &mpcContext->mcDefinitions) &&
					!mpcContext->mcLoops.IsInvariant((size)iLoop, psAdd->asArguments[1 - i], &mpcContext->mcDefinitions))
				{
					psFound = asFound.Add();
					psFound->pcBlock = pcBlock;
					psFound->uiInstruction = uiInstruction;
					psFound->sInvariant = psAdd->asArguments[i];
					psFound->sVariant = psAdd->asArguments[1 - i];
					break;
				}
			}
		}
	}

	//Last first, so each instruction's index is still where it was found.
	bChanged = asFound.NumElements() > 0;
	for (ui = asFound.NumElements(); ui > 0; ui--)
	{
		psFound = asFound.Get(ui - 1);
		psAddress = psFound->pcBlock->masInstructions.Get(psFound->uiInstruction);
		sInner = InsertInstruction(psFound->pcBlock, psFound->uiInstruction, IRO_PtrAdd, psAddress->sType, psAddress->asArguments[0], psFound->sInvariant);
		psAddress = psFound->pcBlock->masInstructions.Get(psFound->uiInstruction + 1);
		psAddress->asArguments[0] = sInner;
		psAddress->asArguments[1] = psFound->sVariant;
		psAddress->uiFlags &= ~IRF_InArray;
	}
	asFound.Kill();
	return bChanged;
}
