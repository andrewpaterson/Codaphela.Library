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
#include <string.h>
#include "BaseLib/IntegerHelper.h"
#include "IRSSAUpdater.h"
#include "IRInliner.h"
#include "IROptimiser.h"


#define IR_OPTIMISE_ROUNDS		16
#define IR_ROTATE_LIMIT			8		//The most instructions a loop's head may have for RotateLoops to copy it.
#define IR_THREAD_LIMIT			8		//The most instructions a block may have for ThreadPhis to work out.
#define IR_RANGE_DEPTH			8		//How many single predecessors up DominatingRange looks.
#define IR_NON_NULL_DEPTH		6		//How many definitions back NonNullAt looks (through phis and pointer adds).
#define IR_NARROW_DEPTH			4		//How deep NarrowTruncations follows operations to narrow.
#define IR_IDIOM_MIN_BYTES		16		//A loop filling or copying a known number of bytes up to this many stays a loop.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::Init(CIRModule* pcModule)
{
	mpcModule = pcModule;
	mpcFunction = NULL;
	mbIndexedAddresses = true;
	mbNarrowMultiplies = false;
	mbConstantDivisions = false;
	mbNarrowCounters = false;
	mbBiasedIndices = false;
	mbBranchRanges = false;
	mpszWarnings = NULL;
	mbRestrict = false;
	miUnswitched = 0;
	msLibraryInt = IRType(IRTK_None);
	mpcProfile = NULL;
	mcDominators.Init();
	mcDefinitions.Init();
	mcLoops.Init();
	masReplacements.Init();
	mauiUses.Init();
	masPromotions.Init();
	maiPromotion.Init();
	maiPhiPromotion.Init();
	masValues.Init();
	maiValueLinks.Init();
	maiTops.Init();
	maiPushedPromotions.Init();
	masArgAddresses.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::Kill(void)
{
	masArgAddresses.Kill();
	maiPushedPromotions.Kill();
	maiTops.Kill();
	maiValueLinks.Kill();
	masValues.Kill();
	maiPhiPromotion.Kill();
	maiPromotion.Kill();
	masPromotions.Kill();
	mauiUses.Kill();
	masReplacements.Kill();
	mcLoops.Kill();
	mcDefinitions.Kill();
	mcDominators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The type memset's int is (16 or 32 bits): loops that fill or copy memory
// become calls to the library's memset and memmove (ReplaceLoopIdioms).
// IRTK_None (the default) leaves them loops.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetLibraryInt(SIRType sInt)
{
	msLibraryInt = sInt;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the target's loads and stores add a pointer and an index for
// nothing (the 65C64's [R1 + R2]; the W65C816's don't): if not, p + i in a
// loop is strength reduced too.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetIndexedAddresses(bool bIndexedAddresses)
{
	mbIndexedAddresses = bIndexedAddresses;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the comparisons found always true or always false because one side
// is zero extended from a type that can't hold the constant on the other are
// described (ComparisonRange), a line each.  The front end can only see the
// types: x != 0xFFFFFFFF with a 32 bit x assigned a 16 bit value is seen
// here, once the values are followed.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetWarnings(CChars* pszWarnings)
{
	mpszWarnings = pszWarnings;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether NarrowMultiplies runs (for a target whose multiplies cost more
// the wider they are: the W65C816's are shifts and adds or calls).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetNarrowMultiplies(bool bNarrowMultiplies)
{
	mbNarrowMultiplies = bNarrowMultiplies;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether ConstantDivisions runs (for a target whose divides are calls:
// the W65C816's).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetConstantDivisions(bool bConstantDivisions)
{
	mbConstantDivisions = bConstantDivisions;
}


//////////////////////////////////////////////////////////////////////////
//
// How often the profiled run ran each instruction's line, for the passes
// that spend memory on hot code (ConstantDivisions' byte tables).  NULL for
// no profile.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetProfile(CIRInlineProfile* pcProfile)
{
	mpcProfile = pcProfile;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the profile has the instruction's line run often enough that
// saving ulliSaving cycles each time is 1/IR_INLINE_HOT_SHARE of the run.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::IsHot(SIRInstruction* psInstruction, uint64 ulliSaving)
{
	uint64	ulliCount;

	if (!mpcProfile || (mpcProfile->ProfileCycles() == 0))
	{
		return false;
	}
	ulliCount = mpcProfile->CallCount(mpcFunction, psInstruction);
	return (ulliCount != IR_INLINE_UNKNOWN) && (ulliCount * ulliSaving * IR_INLINE_HOT_SHARE >= mpcProfile->ProfileCycles());
}


//////////////////////////////////////////////////////////////////////////
//
// Whether NarrowCounters runs (for a target whose 32 bit arithmetic takes
// more instructions than 16 bit: the W65C816's).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetNarrowCounters(bool bNarrowCounters)
{
	mbNarrowCounters = bNarrowCounters;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether BiasSignedIndices runs (for a target that adds an unsigned 16
// bit index to a pointer for nothing: the W65C816's [p],Y).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetBiasedIndices(bool bBiasedIndices)
{
	mbBiasedIndices = bBiasedIndices;
}


//////////////////////////////////////////////////////////////////////////
//
// Per temporary: its uses as the address of a load or store of at most 2
// bytes (not volatile, and not the value a store writes).  mauiUses must be
// counted; pauiAddressUses must be initialised.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::CountAddressUses(CArrayTemplate<uint32>* pauiAddressUses)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sPointer;
	size			ui;
	size			uj;
	uint32			uiZero;

	uiZero = 0;
	for (ui = 0; ui < mauiUses.NumElements(); ui++)
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
SIRReference CIROptimiser::AddressPhiIndex(SIRReference sValue, SIRReference sBase)
{
	SIRInstruction*	psAdd;
	SIRInstruction*	psExtend;
	int64			lliOffset;

	psAdd = sValue.IsTemporary() ? mcDefinitions.Instruction(sValue) : NULL;
	if ((psAdd == NULL) || (psAdd->uiOpcode != IRO_PtrAdd) || !psAdd->asArguments[0].Equals(sBase))
	{
		return IRNone();
	}
	if (psAdd->asArguments[1].IsConstant())
	{
		lliOffset = mpcFunction->GetSigned(psAdd->asArguments[1]);
		return ((lliOffset >= -32768) && (lliOffset <= 32767)) ? mpcFunction->AddInteger(IRType(IRTK_I16), lliOffset) : IRNone();
	}
	psExtend = psAdd->asArguments[1].IsTemporary() ? mcDefinitions.Instruction(psAdd->asArguments[1]) : NULL;
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
bool CIROptimiser::MergeAddressPhis(void)
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
	mcDominators.Compute(mpcFunction);
	mpcFunction->CountUses(&mauiUses);
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
				(*mauiUses.Get(sResult.Index()) == 0) || (*mauiUses.Get(sResult.Index()) != *auiAddressUses.Get(sResult.Index())))
			{
				continue;
			}

			//The base: the first value's that isn't null.
			sBase = IRNone();
			sOffsetType = IRType(IRTK_None);
			for (uk = 0; uk < psPhi->asArguments.NumElements(); uk++)
			{
				sValue = psPhi->asArguments.Get(uk)->sValue;
				psAdd = sValue.IsTemporary() ? mcDefinitions.Instruction(sValue) : NULL;
				if (psAdd && (psAdd->uiOpcode == IRO_PtrAdd))
				{
					sBase = psAdd->asArguments[0];
					sOffsetType = mpcFunction->TypeOf(psAdd->asArguments[1]);
					break;
				}
			}
			pcBaseBlock = sBase.IsTemporary() ? mcDefinitions.Block(sBase) : NULL;
			if (sBase.IsNone() || (pcBaseBlock && ((pcBaseBlock == pcBlock) || !mcDominators.Dominates(pcBaseBlock, pcBlock))) || !sOffsetType.IsInteger())
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
					psAdd = sValue.IsTemporary() ? mcDefinitions.Instruction(sValue) : NULL;
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
bool CIROptimiser::BiasSignedIndices(void)
{
	CArrayTemplate<uint32>	auiAddressUses;		//Per temporary: its uses as a 2 byte load's or store's address.
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRInstruction*			psInner;
	SIRInstruction*			psExtend;
	SIRInstruction*			psScale;
	SIRReference			sIndex;
	SIRReference			sBase;
	SIRReference			sOffset;
	SIRReference			sBiased;
	SIRType					sOffsetType;
	SIRType					sShort;
	int64					lliScale;
	int64					lliExtra;
	int64					lliBias;
	int64					lliMin;
	int64					lliMax;
	size					ui;
	size					uj;
	bool					bChanged;

	mpcFunction->CountUses(&mauiUses);
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
				(psInstruction->sResult.Index() >= auiAddressUses.NumElements()) || (*mauiUses.Get(psInstruction->sResult.Index()) == 0) ||
				(*mauiUses.Get(psInstruction->sResult.Index()) != *auiAddressUses.Get(psInstruction->sResult.Index())))
			{
				continue;
			}
			sBase = psInstruction->asArguments[0];
			sOffset = psInstruction->asArguments[1];
			lliExtra = 0;
			if (sOffset.IsConstant() && sBase.IsTemporary() && (sBase.Index() < mauiUses.NumElements()) && (*mauiUses.Get(sBase.Index()) == 1))
			{
				//(p + sext(x) * s) + c.
				psInner = mcDefinitions.Instruction(sBase);
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
			psExtend = mcDefinitions.Instruction(sOffset);
			psScale = NULL;
			if (psExtend && ((psExtend->uiOpcode == IRO_Shl) || (psExtend->uiOpcode == IRO_Mul)) && psExtend->asArguments[1].IsConstant())
			{
				psScale = psExtend;
				lliScale = mpcFunction->GetSigned(psScale->asArguments[1]);
				if ((psScale->uiOpcode == IRO_Shl) && (lliScale >= 0) && (lliScale < 16))
				{
					lliScale = (int64)1 << lliScale;
				}
				else if ((psScale->uiOpcode == IRO_Shl) || (lliScale < 1) || (lliScale > 0x7FFF))
				{
					continue;
				}
				psExtend = psScale->asArguments[0].IsTemporary() ? mcDefinitions.Instruction(psScale->asArguments[0]) : NULL;
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
				mbBranchRanges = true;
				if (!RangeAt(pcBlock, sIndex, &lliMin, &lliMax))
				{
					lliMin = -0x8000;
					lliMax = 0x7FFF;
				}
				mbBranchRanges = false;
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
				psInner = sBase.IsTemporary() ? mcDefinitions.Instruction(sBase) : NULL;
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
					sIndex = InsertInstruction(pcBlock, uj++, (psScale->uiOpcode == IRO_Shl) ? IRO_Shl : IRO_Mul, sShort, sIndex, mpcFunction->AddInteger(sShort, psScale->uiOpcode == IRO_Shl ? mpcFunction->GetSigned(psScale->asArguments[1]) : lliScale));
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
			psInstruction->uiFlags &= ~IRF_InBank;
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
// Runs the passes until none changes the function.  Returns true if any did.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::Optimise(CIRFunction* pcFunction)
{
	bool	bChanged;
	bool	bAny;
	int		iRound;

	mpcFunction = pcFunction;
	mbRestrict = mpcModule->HasRestrict() || (pcFunction->muiRestrictParameters != 0);
	miUnswitched = 0;
	bAny = false;
	for (iRound = 0; iRound < IR_OPTIMISE_ROUNDS; iRound++)
	{
		bChanged = SimplifyCFG();
		bChanged |= UnifySlotTypes();
		bChanged |= SplitSlots();
		bChanged |= PromoteSlots();
		bChanged |= RemoveTrivialPhis();
		bChanged |= FoldConstants();
		bChanged |= FoldNullTests();
		bChanged |= NarrowTruncations();
		bChanged |= BoundDivisions();
		bChanged |= SignedPowerDivisions();
		bChanged |= mbConstantDivisions && DivisibilityTests();
		bChanged |= mbConstantDivisions && ConstantDivisions();
		bChanged |= RemoveCommonSubexpressions();
		bChanged |= RemoveRedundantLoads();
		bChanged |= mbBiasedIndices && MergeAddressPhis();
		bChanged |= RotateLoops();
		bChanged |= LessLoopTests();
		bChanged |= ReplaceLoopCounters();
		bChanged |= mbNarrowCounters && NarrowCounters();
		bChanged |= ReduceMultiplies();
		bChanged |= ReplaceExitValues();
		bChanged |= DeleteDeadLoops();
		bChanged |= ReduceStrength();
		bChanged |= SplitLoopAddresses();
		bChanged |= HoistInvariants();
		bChanged |= SinkLoads();
		bChanged |= HoistCommon();
		bChanged |= UnswitchLoops();
		bChanged |= ReplaceLoopIdioms();
		bChanged |= RemoveDeadCode();
		bChanged |= SinkComparisons();
		if (!bChanged)
		{
			break;
		}
		bAny = true;
	}
	if (mbNarrowMultiplies && NarrowMultiplies())
	{
		FoldConstants();
		RemoveDeadCode();
		bAny = true;
	}
	if (mbBiasedIndices && BiasSignedIndices())
	{
		RemoveCommonSubexpressions();
		HoistInvariants();
		RemoveDeadCode();
		bAny = true;
	}
	mpcFunction->Renumber();
	mpcFunction->ComputePredecessors();
	return bAny;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::ClearReplacements(void)
{
	masReplacements.FakeSetUsedElements(0);
	masReplacements.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Uses of the temporary sOld are to be sNew.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::Replace(SIRReference sOld, SIRReference sNew)
{
	masReplacements.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	*masReplacements.Get(sOld.Index()) = sNew;
}


//////////////////////////////////////////////////////////////////////////
//
// What sReference is after the replacements (following replaced
// replacements).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::Replacement(SIRReference sReference)
{
	SIRReference*	psReplacement;

	while (sReference.IsTemporary())
	{
		psReplacement = masReplacements.SafeGet(sReference.Index());
		if (!psReplacement || psReplacement->IsNone() || psReplacement->Equals(sReference))
		{
			break;
		}
		sReference = *psReplacement;
	}
	return sReference;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::ApplyReplacements(void)
{
	size			ui;
	size			uiInstruction;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masPhis.NumElements(); uiInstruction++)
		{
			psPhi = pcBlock->masPhis.Get(uiInstruction);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				psArgument = psPhi->asArguments.Get(uiArgument);
				psArgument->sValue = Replacement(psArgument->sValue);
			}
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			psInstruction->asArguments[0] = Replacement(psInstruction->asArguments[0]);
			psInstruction->asArguments[1] = Replacement(psInstruction->asArguments[1]);
		}
		pcBlock->msJump.sArgument = Replacement(pcBlock->msJump.sArgument);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Stores, args, calls, a target's instructions (none are IROP_Pure) and
// volatile loads do something besides make a value.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::HasEffect(SIRInstruction* psInstruction)
{
	if (psInstruction->uiOpcode == IRO_Load)
	{
		return (psInstruction->uiFlags & IRF_Volatile) != 0;
	}
	return !IsIRPure(psInstruction->uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SimplifyCFG(void)
{
	bool	bChanged;
	bool	bAny;

	ClearReplacements();
	bAny = false;
	do
	{
		mpcFunction->Renumber();
		mpcFunction->ComputePredecessors();
		bChanged = RemoveUnreachable() || FoldBranches() || SkipEmptyBlocks() || ThreadBranches() || ThreadPhis() || DuplicateReturns() || JoinBlocks();
		bAny |= bChanged;
	} while (bChanged);
	ApplyReplacements();
	mpcFunction->ComputePredecessors();
	return bAny;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes pcPredecessor's arguments from pcBlock's phis.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::RemovePhiArguments(CIRBlock* pcBlock, CIRBlock* pcPredecessor)
{
	size			ui;
	size			uiArgument;
	SIRPhi*			psPhi;

	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		psPhi = pcBlock->masPhis.Get(ui);
		for (uiArgument = psPhi->asArguments.NumElements(); uiArgument > 0; uiArgument--)
		{
			if (psPhi->asArguments.Get(uiArgument - 1)->pcBlock == pcPredecessor)
			{
				psPhi->asArguments.RemoveAt(uiArgument - 1, true);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::RenamePhiArguments(CIRBlock* pcBlock, CIRBlock* pcOld, CIRBlock* pcNew)
{
	size			ui;
	size			uiArgument;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;

	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		psPhi = pcBlock->masPhis.Get(ui);
		for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
		{
			psArgument = psPhi->asArguments.Get(uiArgument);
			if (psArgument->pcBlock == pcOld)
			{
				psArgument->pcBlock = pcNew;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pcBlock's jump goes to pcNew where it went to pcOld.  A brif to one block
// twice becomes a jmp.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::RedirectJump(CIRBlock* pcBlock, CIRBlock* pcOld, CIRBlock* pcNew)
{
	if (pcBlock->msJump.pcThen == pcOld)
	{
		pcBlock->msJump.pcThen = pcNew;
	}
	if ((pcBlock->msJump.uiKind == IRJ_Branch) && (pcBlock->msJump.pcElse == pcOld))
	{
		pcBlock->msJump.pcElse = pcNew;
	}
	if ((pcBlock->msJump.uiKind == IRJ_Branch) && (pcBlock->msJump.pcThen == pcBlock->msJump.pcElse))
	{
		pcBlock->Jump(pcBlock->msJump.pcThen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pcPredecessor jumps straight to pcTarget instead of through pcBlock,
// passing pcTarget's phis what pcBlock did.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::Bypass(CIRBlock* pcPredecessor, CIRBlock* pcBlock, CIRBlock* pcTarget)
{
	size			uiPhi;
	SIRPhiArgument*	psArgument;

	SIRReference	sValue;
	SIRPhiArgument*	psThrough;
	size			uiBlockPhi;

	for (uiPhi = 0; uiPhi < pcTarget->masPhis.NumElements(); uiPhi++)
	{
		psArgument = pcTarget->FindPhiArgument(pcTarget->masPhis.Get(uiPhi), pcBlock);
		sValue = psArgument ? psArgument->sValue : IRNone();
		for (uiBlockPhi = 0; uiBlockPhi < pcBlock->masPhis.NumElements(); uiBlockPhi++)
		{
			if (pcBlock->masPhis.Get(uiBlockPhi)->sResult.Equals(sValue))
			{
				//One of pcBlock's phis: what pcPredecessor gives it.
				psThrough = pcBlock->FindPhiArgument(pcBlock->masPhis.Get(uiBlockPhi), pcPredecessor);
				sValue = psThrough ? psThrough->sValue : IRNone();
				break;
			}
		}
		pcTarget->AddPhiArgument(pcTarget->masPhis.Get(uiPhi), pcPredecessor, sValue);
	}
	RedirectJump(pcPredecessor, pcBlock, pcTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks the entry can't reach go; their arguments to reachable blocks' phis
// too.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RemoveUnreachable(void)
{
	size			ui;
	size			uiSuccessor;
	CIRBlock*		pcBlock;
	CArrayIRBlocks	apcUnreachable;

	mcDominators.Compute(mpcFunction);
	if (mcDominators.NumReachable() == mpcFunction->NumBlocks())
	{
		return false;
	}
	apcUnreachable.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if (!mcDominators.IsReachable(pcBlock))
		{
			apcUnreachable.Add(pcBlock);
			for (uiSuccessor = 0; uiSuccessor < pcBlock->NumSuccessors(); uiSuccessor++)
			{
				RemovePhiArguments(pcBlock->GetSuccessor(uiSuccessor), pcBlock);
			}
		}
	}
	for (ui = 0; ui < apcUnreachable.NumElements(); ui++)
	{
		mpcFunction->RemoveBlock(apcUnreachable.GetPtr(ui));
	}
	apcUnreachable.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// brif on a constant is a jmp.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::FoldBranches(void)
{
	size			ui;
	CIRBlock*		pcBlock;
	CIRBlock*		pcTaken;
	CIRBlock*		pcDropped;
	bool			bChanged;

	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if ((pcBlock->msJump.uiKind == IRJ_Branch) && pcBlock->msJump.sArgument.IsConstant())
		{
			if (mpcFunction->GetConstant(pcBlock->msJump.sArgument)->ulliBits & 1)
			{
				pcTaken = pcBlock->msJump.pcThen;
				pcDropped = pcBlock->msJump.pcElse;
			}
			else
			{
				pcTaken = pcBlock->msJump.pcElse;
				pcDropped = pcBlock->msJump.pcThen;
			}
			RemovePhiArguments(pcDropped, pcBlock);
			pcBlock->Jump(pcTaken);
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A block with nothing in it but a jmp is skipped: its predecessors jump
// where it does (unless the target's phis would need two arguments from one
// block).  Returns true after the first block that changes.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SkipEmptyBlocks(void)
{
	size			ui;
	size			uiPredecessor;
	CIRBlock*		pcBlock;
	CIRBlock*		pcTarget;
	CIRBlock*		pcPredecessor;
	bool			bChanged;

	for (ui = 1; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		pcTarget = pcBlock->msJump.pcThen;
		if ((pcBlock->msJump.uiKind != IRJ_Jump) || (pcTarget == pcBlock) || (pcTarget == mpcFunction->GetEntry()) || (pcBlock->masPhis.NumElements() != 0) || (pcBlock->masInstructions.NumElements() != 0))
		{
			continue;
		}

		bChanged = false;
		for (uiPredecessor = 0; uiPredecessor < pcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			pcPredecessor = pcBlock->mapcPredecessors.GetPtr(uiPredecessor);
			if ((pcTarget->masPhis.NumElements() != 0) && (pcTarget->HasPredecessor(pcPredecessor) || (pcPredecessor == pcTarget)))
			{
				continue;
			}
			Bypass(pcPredecessor, pcBlock, pcTarget);
			bChanged = true;
		}
		if (bChanged)
		{
			mpcFunction->ComputePredecessors();
			if (pcBlock->mapcPredecessors.NumElements() == 0)
			{
				RemovePhiArguments(pcTarget, pcBlock);
				mpcFunction->RemoveBlock(pcBlock);
			}
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The value sCondition has on the way into pcBlock from pcPredecessor, if a
// branch on it decides that: pcPredecessor branches on it, or is reached
// along single predecessors from a block that does.  Returns -1 if unknown.
//
//////////////////////////////////////////////////////////////////////////
int CIROptimiser::KnownCondition(CIRBlock* pcPredecessor, CIRBlock* pcBlock, SIRReference sCondition)
{
	size		ui;
	SIRJump*	psJump;

	for (ui = 0; ui < IR_KNOWN_DEPTH; ui++)
	{
		psJump = &pcPredecessor->msJump;
		if ((psJump->uiKind == IRJ_Branch) && psJump->sArgument.Equals(sCondition) && (psJump->pcThen != psJump->pcElse))
		{
			return psJump->pcThen == pcBlock ? 1 : 0;
		}

		//Going further back must stay where sCondition is the same value.
		if ((pcPredecessor == mpcFunction->GetEntry()) || (pcPredecessor->mapcPredecessors.NumElements() != 1) || pcPredecessor->Defines(sCondition))
		{
			return -1;
		}
		pcBlock = pcPredecessor;
		pcPredecessor = pcBlock->mapcPredecessors.GetPtr(0);
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// A block with nothing in it but a brif is skipped by a predecessor that
// knows which way it goes (KnownCondition).  Returns true after the first
// block that changes.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ThreadBranches(void)
{
	size			ui;
	size			uiPredecessor;
	int				iKnown;
	CIRBlock*		pcBlock;
	CIRBlock*		pcTarget;
	CIRBlock*		pcPredecessor;
	bool			bChanged;

	for (ui = 1; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if ((pcBlock->msJump.uiKind != IRJ_Branch) || !pcBlock->msJump.sArgument.IsTemporary() || (pcBlock->masPhis.NumElements() != 0) || (pcBlock->masInstructions.NumElements() != 0))
		{
			continue;
		}

		bChanged = false;
		for (uiPredecessor = 0; uiPredecessor < pcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			pcPredecessor = pcBlock->mapcPredecessors.GetPtr(uiPredecessor);
			if (pcPredecessor == pcBlock)
			{
				continue;
			}
			iKnown = KnownCondition(pcPredecessor, pcBlock, pcBlock->msJump.sArgument);
			if (iKnown < 0)
			{
				continue;
			}
			pcTarget = iKnown ? pcBlock->msJump.pcThen : pcBlock->msJump.pcElse;
			if ((pcTarget == pcBlock) || ((pcTarget->masPhis.NumElements() != 0) && pcTarget->HasPredecessor(pcPredecessor)))
			{
				continue;
			}
			Bypass(pcPredecessor, pcBlock, pcTarget);
			bChanged = true;
		}
		if (bChanged)
		{
			//RemoveUnreachable takes pcBlock out if nothing goes to it now.
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Counts in pauiInside the values pcSuccessor's phis take from pcBlock
// that are pcBlock's phis (not when pcSuccessor is pcBlock: those are
// counted with its phis).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::CountSuccessorPhiUses(CIRBlock* pcBlock, CIRBlock* pcSuccessor, CArrayTemplate<uint32>* pauiInside)
{
	SIRPhiArgument*	psArgument;
	size			ui;
	size			uj;

	if (!pcSuccessor || (pcSuccessor == pcBlock))
	{
		return;
	}
	for (ui = 0; ui < pcSuccessor->masPhis.NumElements(); ui++)
	{
		psArgument = pcSuccessor->FindPhiArgument(pcSuccessor->masPhis.Get(ui), pcBlock);
		if (!psArgument || !psArgument->sValue.IsTemporary())
		{
			continue;
		}
		for (uj = 0; uj < pcBlock->masPhis.NumElements(); uj++)
		{
			if (pcBlock->masPhis.Get(uj)->sResult.Equals(psArgument->sValue))
			{
				(*pauiInside->Get(psArgument->sValue.Index()))++;
				break;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcBlock's phis and results are only used in pcBlock (by its
// instructions, jump or phis; so a predecessor can go round it without
// them).  With bSuccessorPhis its phis may also be the values its
// successors' phis take from it (Bypass gives those what the predecessor
// gave).  mauiUses must be counted.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::OnlyUsedInBlock(CIRBlock* pcBlock, bool bSuccessorPhis)
{
	CArrayTemplate<uint32>	auiInside;
	SIRInstruction*			psInstruction;
	SIRReference			sValue;
	size					ui;
	size					uiArgument;
	bool					bOnly;

	auiInside.Init();
	auiInside.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		for (uiArgument = 0; uiArgument < pcBlock->masPhis.Get(ui)->asArguments.NumElements(); uiArgument++)
		{
			sValue = pcBlock->masPhis.Get(ui)->asArguments.Get(uiArgument)->sValue;
			if (sValue.IsTemporary())
			{
				(*auiInside.Get(sValue.Index()))++;
			}
		}
	}
	for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		for (uiArgument = 0; uiArgument < 2; uiArgument++)
		{
			if (psInstruction->asArguments[uiArgument].IsTemporary())
			{
				(*auiInside.Get(psInstruction->asArguments[uiArgument].Index()))++;
			}
		}
	}
	if (pcBlock->msJump.sArgument.IsTemporary())
	{
		(*auiInside.Get(pcBlock->msJump.sArgument.Index()))++;
	}
	if (bSuccessorPhis)
	{
		CountSuccessorPhiUses(pcBlock, pcBlock->msJump.pcThen, &auiInside);
		if (pcBlock->msJump.pcElse != pcBlock->msJump.pcThen)
		{
			CountSuccessorPhiUses(pcBlock, pcBlock->msJump.pcElse, &auiInside);
		}
	}

	bOnly = true;
	for (ui = 0; bOnly && (ui < pcBlock->masPhis.NumElements()); ui++)
	{
		sValue = pcBlock->masPhis.Get(ui)->sResult;
		bOnly = *mauiUses.Get(sValue.Index()) == *auiInside.Get(sValue.Index());
	}
	for (ui = 0; bOnly && (ui < pcBlock->masInstructions.NumElements()); ui++)
	{
		sValue = pcBlock->masInstructions.Get(ui)->sResult;
		bOnly = !sValue.IsTemporary() || (*mauiUses.Get(sValue.Index()) == *auiInside.Get(sValue.Index()));
	}
	auiInside.Kill();
	return bOnly;
}


//////////////////////////////////////////////////////////////////////////
//
// Where pcBlock's brif goes when it is entered from pcPredecessor, if the
// values pcPredecessor gives its phis make the condition a constant; NULL
// if they don't.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIROptimiser::KnownTarget(CIRBlock* pcPredecessor, CIRBlock* pcBlock)
{
	CArrayIRReferences	asKnown;		//Per temporary: its constant coming from pcPredecessor, IRNone if not known.
	SIRInstruction		sCopy;
	SIRPhi*				psPhi;
	SIRPhiArgument*		psArgument;
	SIRReference		sCondition;
	uint64				ulliBits;
	size				ui;
	int					iArgument;

	asKnown.Init();
	asKnown.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		psPhi = pcBlock->masPhis.Get(ui);
		psArgument = pcBlock->FindPhiArgument(psPhi, pcPredecessor);
		if (psArgument && psArgument->sValue.IsConstant())
		{
			*asKnown.Get(psPhi->sResult.Index()) = psArgument->sValue;
		}
	}
	for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
	{
		sCopy = *pcBlock->masInstructions.Get(ui);
		if (sCopy.sResult.IsTemporary() && ((sCopy.uiOpcode == IRO_Eq) || (sCopy.uiOpcode == IRO_Ne)) && sCopy.asArguments[1].IsConstant() &&
			(mpcFunction->GetConstant(sCopy.asArguments[1])->ulliBits == 0) && sCopy.asArguments[0].IsTemporary() && mpcFunction->TypeOf(sCopy.asArguments[0]).IsPointer())
		{
			//A pointer phi that isn't null coming from pcPredecessor (NonNullAt): p == 0 is false, p != 0 true.
			psPhi = mcDefinitions.Phi(sCopy.asArguments[0]);
			psArgument = (psPhi && (mcDefinitions.Block(sCopy.asArguments[0]) == pcBlock)) ? pcBlock->FindPhiArgument(psPhi, pcPredecessor) : NULL;
			if (psArgument && (BranchNonNull(pcPredecessor, pcBlock, psArgument->sValue) || NonNullAt(pcPredecessor, psArgument->sValue, IR_NON_NULL_DEPTH)))
			{
				*asKnown.Get(sCopy.sResult.Index()) = mpcFunction->AddConstant(mpcFunction->GetTemporaryType(sCopy.sResult), (sCopy.uiOpcode == IRO_Ne) ? 1 : 0);
				continue;
			}
		}
		for (iArgument = 0; iArgument < 2; iArgument++)
		{
			if (sCopy.asArguments[iArgument].IsTemporary() && !asKnown.Get(sCopy.asArguments[iArgument].Index())->IsNone())
			{
				sCopy.asArguments[iArgument] = *asKnown.Get(sCopy.asArguments[iArgument].Index());
			}
		}
		if (sCopy.sResult.IsTemporary() && Fold(&sCopy, &ulliBits))
		{
			*asKnown.Get(sCopy.sResult.Index()) = mpcFunction->AddConstant(mpcFunction->GetTemporaryType(sCopy.sResult), ulliBits);
		}
	}

	sCondition = pcBlock->msJump.sArgument;
	if (sCondition.IsTemporary())
	{
		sCondition = *asKnown.Get(sCondition.Index());
	}
	asKnown.Kill();
	if (!sCondition.IsConstant())
	{
		return NULL;
	}
	return (mpcFunction->GetConstant(sCondition)->ulliBits & 1) ? pcBlock->msJump.pcThen : pcBlock->msJump.pcElse;
}


//////////////////////////////////////////////////////////////////////////
//
// Jump threading through phis: a block of phis and a few instructions
// without effects, ending in a brif, whose values are only used in it (as
// an inlined test that returns a constant on some ways makes them).  A
// predecessor whose phi arguments make the condition a constant jumps
// straight to where the brif would go (unless it already jumps there).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ThreadPhis(void)
{
	CArrayIRBlocks	apcPredecessors;
	size			ui;
	size			uiPredecessor;
	CIRBlock*		pcBlock;
	CIRBlock*		pcPredecessor;
	CIRBlock*		pcTarget;
	bool			bChanged;
	bool			bPure;

	mpcFunction->CountUses(&mauiUses);
	FindDefinitions();
	apcPredecessors.Init();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if ((pcBlock->masPhis.NumElements() == 0) || (pcBlock->msJump.uiKind != IRJ_Branch) || (pcBlock->msJump.pcThen == pcBlock->msJump.pcElse) ||
			(pcBlock->masInstructions.NumElements() > IR_THREAD_LIMIT))
		{
			continue;
		}
		bPure = true;
		for (uiPredecessor = 0; bPure && (uiPredecessor < pcBlock->masInstructions.NumElements()); uiPredecessor++)
		{
			bPure = IsIRPure(pcBlock->masInstructions.Get(uiPredecessor)->uiOpcode);
		}
		if (!bPure || !OnlyUsedInBlock(pcBlock, true))
		{
			continue;
		}

		apcPredecessors.FakeSetUsedElements(0);
		for (uiPredecessor = 0; uiPredecessor < pcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			apcPredecessors.Add(pcBlock->mapcPredecessors.GetPtr(uiPredecessor));
		}
		for (uiPredecessor = 0; uiPredecessor < apcPredecessors.NumElements(); uiPredecessor++)
		{
			pcPredecessor = apcPredecessors.GetPtr(uiPredecessor);
			pcTarget = (pcPredecessor == pcBlock) ? NULL : KnownTarget(pcPredecessor, pcBlock);
			if ((pcTarget == NULL) || (pcTarget == pcBlock) || pcTarget->HasPredecessor(pcPredecessor))
			{
				continue;
			}
			Bypass(pcPredecessor, pcBlock, pcTarget);
			RemovePhiArguments(pcBlock, pcPredecessor);
			mpcFunction->ComputePredecessors();
			bChanged = true;
		}
	}
	apcPredecessors.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A block with nothing in it but phis and a ret is copied into each
// predecessor that just jumps to it: that one returns what it would have
// passed to the phi, so no copy is made to the phi's home on the way.
// Returns true after the first block that changes.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::DuplicateReturns(void)
{
	size			ui;
	size			uiPredecessor;
	size			uiPhi;
	CIRBlock*		pcBlock;
	CIRBlock*		pcPredecessor;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sValue;
	bool			bChanged;

	for (ui = 1; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if ((pcBlock->msJump.uiKind != IRJ_Return) || (pcBlock->masInstructions.NumElements() != 0))
		{
			continue;
		}

		//The phi the ret returns, if it returns one of the block's.
		psPhi = NULL;
		for (uiPhi = 0; uiPhi < pcBlock->masPhis.NumElements(); uiPhi++)
		{
			if (pcBlock->masPhis.Get(uiPhi)->sResult.Equals(pcBlock->msJump.sArgument))
			{
				psPhi = pcBlock->masPhis.Get(uiPhi);
			}
		}

		bChanged = false;
		for (uiPredecessor = 0; uiPredecessor < pcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			pcPredecessor = pcBlock->mapcPredecessors.GetPtr(uiPredecessor);
			if ((pcPredecessor == pcBlock) || (pcPredecessor->msJump.uiKind != IRJ_Jump))
			{
				continue;
			}
			sValue = pcBlock->msJump.sArgument;
			if (psPhi)
			{
				psArgument = pcBlock->FindPhiArgument(psPhi, pcPredecessor);
				sValue = psArgument ? psArgument->sValue : IRNone();
			}
			pcPredecessor->Return(pcBlock->msJump.sType, sValue);
			RemovePhiArguments(pcBlock, pcPredecessor);
			bChanged = true;
		}
		if (bChanged)
		{
			//RemoveUnreachable takes the block out if nothing goes to it now.
			mpcFunction->ComputePredecessors();
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A block that is its jmp target's only predecessor takes in the target.
// Returns true after the first join.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::JoinBlocks(void)
{
	size			ui;
	size			uiInstruction;
	size			uiSuccessor;
	CIRBlock*		pcBlock;
	CIRBlock*		pcNext;
	SIRPhi*			psPhi;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		pcNext = pcBlock->msJump.pcThen;
		if ((pcBlock->msJump.uiKind != IRJ_Jump) || (pcNext == pcBlock) || (pcNext == mpcFunction->GetEntry()) || (pcNext->mapcPredecessors.NumElements() != 1))
		{
			continue;
		}

		for (uiInstruction = 0; uiInstruction < pcNext->masPhis.NumElements(); uiInstruction++)
		{
			psPhi = pcNext->masPhis.Get(uiInstruction);
			Replace(psPhi->sResult, psPhi->asArguments.Get(0)->sValue);
		}
		for (uiInstruction = 0; uiInstruction < pcNext->masInstructions.NumElements(); uiInstruction++)
		{
			pcBlock->masInstructions.Add(pcNext->masInstructions.Get(uiInstruction));
		}
		pcBlock->msJump = pcNext->msJump;
		for (uiSuccessor = 0; uiSuccessor < pcNext->NumSuccessors(); uiSuccessor++)
		{
			RenamePhiArguments(pcNext->GetSuccessor(uiSuccessor), pcNext, pcBlock);
		}
		pcNext->Unreachable();
		mpcFunction->ComputePredecessors();
		mpcFunction->RemoveBlock(pcNext);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A phi whose arguments are all one value (or the phi itself) is that
// value; one taking the same values as an earlier phi of its block is that
// phi.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RemoveTrivialPhis(void)
{
	size			ui;
	size			uiPhi;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRPhi*			psPhi;
	SIRReference	sValue;
	SIRReference	sArgument;
	bool			bTrivial;
	bool			bChanged;
	bool			bAny;

	ClearReplacements();
	bAny = false;
	do
	{
		bChanged = false;
		for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uiPhi = pcBlock->masPhis.NumElements(); uiPhi > 0; uiPhi--)
			{
				psPhi = pcBlock->masPhis.Get(uiPhi - 1);
				sValue = IRNone();
				bTrivial = true;
				for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
				{
					sArgument = Replacement(psPhi->asArguments.Get(uiArgument)->sValue);
					if (sArgument.Equals(psPhi->sResult) || sArgument.Equals(sValue))
					{
						continue;
					}
					if (!sValue.IsNone())
					{
						bTrivial = false;
						break;
					}
					sValue = sArgument;
				}
				if (!(bTrivial && !sValue.IsNone()))
				{
					sValue = SamePhi(pcBlock, uiPhi - 1);
				}
				if (!sValue.IsNone())
				{
					Replace(psPhi->sResult, sValue);
					psPhi->asArguments.Kill();
					pcBlock->masPhis.RemoveAt(uiPhi - 1, true);
					bChanged = true;
				}
			}
		}
		bAny |= bChanged;
	} while (bChanged);
	ApplyReplacements();
	return bAny;
}


//////////////////////////////////////////////////////////////////////////
//
// An earlier phi of pcBlock taking the same value from each block as its
// phi uiPhi does; IRNone for none.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::SamePhi(CIRBlock* pcBlock, size uiPhi)
{
	SIRPhi*			psPhi;
	SIRPhi*			psOther;
	SIRPhiArgument*	psArgument;
	SIRPhiArgument*	psOtherArgument;
	size			uiOther;
	size			uiArgument;
	bool			bSame;

	psPhi = pcBlock->masPhis.Get(uiPhi);
	for (uiOther = 0; uiOther < uiPhi; uiOther++)
	{
		psOther = pcBlock->masPhis.Get(uiOther);
		if (!psOther->sType.Equals(psPhi->sType) || (psOther->asArguments.NumElements() != psPhi->asArguments.NumElements()))
		{
			continue;
		}
		bSame = true;
		for (uiArgument = 0; bSame && (uiArgument < psPhi->asArguments.NumElements()); uiArgument++)
		{
			psArgument = psPhi->asArguments.Get(uiArgument);
			psOtherArgument = pcBlock->FindPhiArgument(psOther, psArgument->pcBlock);
			bSame = psOtherArgument && Replacement(psOtherArgument->sValue).Equals(Replacement(psArgument->sValue));
		}
		if (bSame)
		{
			return psOther->sResult;
		}
	}
	return IRNone();
}


//////////////////////////////////////////////////////////////////////////
//
// Instructions and phis without effects whose results nothing with an
// effect needs go: what instructions with effects, and jumps, use is live,
// and so is what live values are made from (so a counter only going round a
// loop to itself goes too).
//
//////////////////////////////////////////////////////////////////////////
static void IROptimiserMarkLive(CArrayTemplate<uint8>* pabLive, CArrayIRReferences* pasWork, SIRReference sValue)
{
	if (sValue.IsTemporary() && !*pabLive->Get(sValue.Index()))
	{
		*pabLive->Get(sValue.Index()) = true;
		pasWork->Add(&sValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An integer comparison only a brif uses moves to the end of the brif's
// block, so the back ends branch on its flags rather than keep a 0 or 1
// (a loop's latch compares and then steps its pointers; an unswitched
// loop's test was worked out blocks before).  Its operands are values, so
// they don't change; it isn't moved into a loop it wasn't in.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SinkComparisons(void)
{
	CIRBlock*		pcBlock;
	CIRBlock*		pcFrom;
	SIRInstruction	sMoved;
	SIRInstruction*	psCompare;
	size			ui;
	size			uiLoop;
	bool			bChanged;
	bool			bDeeper;
	int32			iIndex;

	ComputeLoops();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if ((pcBlock->msJump.uiKind != IRJ_Branch) || !pcBlock->msJump.sArgument.IsTemporary() || (mcDefinitions.NumUses(pcBlock->msJump.sArgument) != 1))
		{
			continue;
		}
		psCompare = mcDefinitions.Instruction(pcBlock->msJump.sArgument);
		pcFrom = mcDefinitions.Block(pcBlock->msJump.sArgument);
		if (!psCompare || !pcFrom || !IsIRIntegerComparison(psCompare->uiOpcode))
		{
			continue;
		}
		iIndex = mcDefinitions.Index(pcBlock->msJump.sArgument);
		if ((iIndex < 0) || ((pcFrom == pcBlock) && ((size)iIndex == pcBlock->masInstructions.NumElements() - 1)))
		{
			continue;
		}
		bDeeper = false;
		for (uiLoop = 0; !bDeeper && (uiLoop < mcLoops.NumLoops()); uiLoop++)
		{
			bDeeper = mcLoops.Contains(uiLoop, pcBlock) && !mcLoops.Contains(uiLoop, pcFrom);
		}
		if (bDeeper)
		{
			continue;
		}
		sMoved = *psCompare;
		pcFrom->masInstructions.RemoveAt(iIndex, true);
		pcBlock->masInstructions.Add(&sMoved);
		bChanged = true;
		FindDefinitions();
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RemoveDeadCode(void)
{
	CArrayTemplate<uint8>	abLive;		//Per temporary.
	CArrayIRReferences		asWork;
	SIRReference			sValue;
	size					ui;
	size					uiInstruction;
	size					uiArgument;
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRPhi*					psPhi;
	bool					bChanged;

	FindDefinitions();
	abLive.Init();
	abLive.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	asWork.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (HasEffect(psInstruction) || !psInstruction->sResult.IsTemporary())
			{
				IROptimiserMarkLive(&abLive, &asWork, psInstruction->asArguments[0]);
				IROptimiserMarkLive(&abLive, &asWork, psInstruction->asArguments[1]);
				IROptimiserMarkLive(&abLive, &asWork, psInstruction->sResult);
			}
		}
		IROptimiserMarkLive(&abLive, &asWork, pcBlock->msJump.sArgument);
	}
	while (asWork.NumElements() > 0)
	{
		asWork.Pop(&sValue);
		psInstruction = mcDefinitions.Instruction(sValue);
		psPhi = mcDefinitions.Phi(sValue);
		if (psInstruction)
		{
			IROptimiserMarkLive(&abLive, &asWork, psInstruction->asArguments[0]);
			IROptimiserMarkLive(&abLive, &asWork, psInstruction->asArguments[1]);
		}
		else if (psPhi)
		{
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				IROptimiserMarkLive(&abLive, &asWork, psPhi->asArguments.Get(uiArgument)->sValue);
			}
		}
	}

	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = pcBlock->masInstructions.NumElements(); uiInstruction > 0; uiInstruction--)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction - 1);
			if (!HasEffect(psInstruction) && psInstruction->sResult.IsTemporary() && !*abLive.Get(psInstruction->sResult.Index()))
			{
				pcBlock->masInstructions.RemoveAt(uiInstruction - 1, true);
				bChanged = true;
			}
		}
		for (uiInstruction = pcBlock->masPhis.NumElements(); uiInstruction > 0; uiInstruction--)
		{
			psPhi = pcBlock->masPhis.Get(uiInstruction - 1);
			if (!*abLive.Get(psPhi->sResult.Index()))
			{
				psPhi->asArguments.Kill();
				pcBlock->masPhis.RemoveAt(uiInstruction - 1, true);
				bChanged = true;
			}
		}
	}
	asWork.Kill();
	abLive.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Slots and argaddrs whose address is only loaded from and stored to.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::PromoteSlots(void)
{
	mpcFunction->Renumber();
	mpcFunction->ComputePredecessors();
	FindPromotions();
	if (masPromotions.NumElements() == 0)
	{
		return false;
	}
	ClearReplacements();
	mcDominators.Compute(mpcFunction);
	mcDominators.ComputeFrontiers();
	PlacePhis();
	Rename();
	RemovePromoted();
	ApplyReplacements();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The promotion sAddress is, -1 if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
int32 CIROptimiser::PromotionOf(SIRReference sAddress)
{
	int32*	piPromotion;

	if (!sAddress.IsTemporary())
	{
		return -1;
	}
	piPromotion = maiPromotion.SafeGet(sAddress.Index());
	return piPromotion ? *piPromotion : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// True if argument iArgument of psInstruction is a use slot promotion
// allows: the address of a load or store that isn't volatile.
//
//////////////////////////////////////////////////////////////////////////
static bool IROptimiserCandidateUse(SIRInstruction* psInstruction, int iArgument)
{
	if (psInstruction->uiFlags & IRF_Volatile)
	{
		return false;
	}
	return ((psInstruction->uiOpcode == IRO_Load) && (iArgument == 0)) || ((psInstruction->uiOpcode == IRO_Store) && (iArgument == 1));
}


//////////////////////////////////////////////////////////////////////////
//
// The entry block's slots and argaddrs whose every use is the address of a
// load or store (not volatile, all at one type).  An argaddr's value before
// a store is its parameter's; a slot's is zero (C++ says it isn't known).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::FindPromotions(void)
{
	size					ui;
	size					uiInstruction;
	size					uiArgument;
	CIRBlock*				pcBlock;
	CIRBlock*				pcEntry;
	SIRInstruction*			psInstruction;
	SIRPhi*					psPhi;
	SIRPromotion*			psPromotion;
	SIRPromotion			sPromotion;
	int32					iPromotion;
	int						iArgument;
	CArrayTemplate<uint8>	abValid;
	CArrayTemplate<uint8>	abSeen;
	uint8					uiFalse;
	uint8					uiTrue;
	CArrayIRPromotions		asCandidates;
	size					uiParameter;

	uiFalse = 0;
	uiTrue = 1;
	masPromotions.FakeSetUsedElements(0);
	maiPromotion.FakeSetUsedElements(0);
	maiPromotion.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0xFF);
	asCandidates.Init();
	abValid.Init();
	abSeen.Init();

	pcEntry = mpcFunction->GetEntry();
	for (uiInstruction = 0; uiInstruction < pcEntry->masInstructions.NumElements(); uiInstruction++)
	{
		psInstruction = pcEntry->masInstructions.Get(uiInstruction);
		if (((psInstruction->uiOpcode == IRO_Slot) || (psInstruction->uiOpcode == IRO_ArgAddress)) && psInstruction->sResult.IsTemporary())
		{
			sPromotion.sAddress = psInstruction->sResult;
			sPromotion.sType = IRType(IRTK_None);
			sPromotion.sInitial = IRNone();
			if (psInstruction->uiOpcode == IRO_ArgAddress)
			{
				uiParameter = (size)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits;
				if (uiParameter >= mpcFunction->masParameters.NumElements())
				{
					continue;
				}
				sPromotion.sInitial = *mpcFunction->masParameters.Get(uiParameter);
			}
			*maiPromotion.Get(psInstruction->sResult.Index()) = (int32)asCandidates.NumElements();
			asCandidates.Add(&sPromotion);
			abValid.Add(&uiTrue);
			abSeen.Add(&uiFalse);
		}
	}

	//Every use must be a load's or store's address, at one type.
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masPhis.NumElements(); uiInstruction++)
		{
			psPhi = pcBlock->masPhis.Get(uiInstruction);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				iPromotion = PromotionOf(psPhi->asArguments.Get(uiArgument)->sValue);
				if (iPromotion >= 0)
				{
					*abValid.Get(iPromotion) = 0;
				}
			}
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			for (iArgument = 0; iArgument < 2; iArgument++)
			{
				iPromotion = PromotionOf(psInstruction->asArguments[iArgument]);
				if (iPromotion < 0)
				{
					continue;
				}
				psPromotion = asCandidates.Get(iPromotion);
				if (!IROptimiserCandidateUse(psInstruction, iArgument))
				{
					*abValid.Get(iPromotion) = 0;
				}
				else if (!*abSeen.Get(iPromotion))
				{
					psPromotion->sType = psInstruction->sType;
					*abSeen.Get(iPromotion) = 1;
				}
				else if (!psPromotion->sType.Equals(psInstruction->sType))
				{
					*abValid.Get(iPromotion) = 0;
				}
			}
		}
		iPromotion = PromotionOf(pcBlock->msJump.sArgument);
		if (iPromotion >= 0)
		{
			*abValid.Get(iPromotion) = 0;
		}
	}

	//The ones that are left, numbered again.
	maiPromotion.FakeSetUsedElements(0);
	maiPromotion.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0xFF);
	for (ui = 0; ui < asCandidates.NumElements(); ui++)
	{
		psPromotion = asCandidates.Get(ui);
		if (!*abValid.Get(ui) || !*abSeen.Get(ui))
		{
			continue;
		}
		if (psPromotion->sInitial.IsNone())
		{
			psPromotion->sInitial = mpcFunction->AddConstant(psPromotion->sType, 0);
		}
		else if (!mpcFunction->TypeOf(psPromotion->sInitial).Equals(psPromotion->sType))
		{
			continue;
		}
		*maiPromotion.Get(psPromotion->sAddress.Index()) = (int32)masPromotions.NumElements();
		masPromotions.Add(psPromotion);
	}
	abSeen.Kill();
	abValid.Kill();
	asCandidates.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A phi for each promotion in the iterated dominance frontier of the blocks
// that store to it.  Their arguments are filled in by Rename.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::PlacePhis(void)
{
	size					ui;
	size					uiInstruction;
	size					uiFrontier;
	size					uiNumBlocks;
	int32					iPromotion;
	CIRBlock*				pcBlock;
	CIRBlock*				pcFrontier;
	SIRInstruction*			psInstruction;
	SIRReference			sResult;
	SIRPromotion*			psPromotion;
	CArrayIRBlocks			apcWork;
	CArrayTemplate<int32>	aiHasPhi;		//Per block: the last promotion given a phi there.
	CArrayTemplate<int32>	aiQueued;		//Per block: the last promotion it was queued for.

	uiNumBlocks = mpcFunction->NumBlocks();
	maiPhiPromotion.FakeSetUsedElements(0);
	maiPhiPromotion.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0xFF);
	apcWork.Init();
	aiHasPhi.Init();
	aiHasPhi.GrowToAtLeastNumElements(uiNumBlocks, true, 0xFF);
	aiQueued.Init();
	aiQueued.GrowToAtLeastNumElements(uiNumBlocks, true, 0xFF);

	for (iPromotion = 0; iPromotion < (int32)masPromotions.NumElements(); iPromotion++)
	{
		psPromotion = masPromotions.Get(iPromotion);
		apcWork.FakeSetUsedElements(0);
		for (ui = 0; ui < uiNumBlocks; ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				if ((psInstruction->uiOpcode == IRO_Store) && (PromotionOf(psInstruction->asArguments[1]) == iPromotion))
				{
					if (*aiQueued.Get(ui) != iPromotion)
					{
						*aiQueued.Get(ui) = iPromotion;
						apcWork.Add(pcBlock);
					}
					break;
				}
			}
		}
		while (apcWork.NumElements() > 0)
		{
			pcBlock = apcWork.Pop();
			for (uiFrontier = 0; uiFrontier < mcDominators.NumFrontiers(pcBlock); uiFrontier++)
			{
				pcFrontier = mcDominators.GetFrontier(pcBlock, uiFrontier);
				if (*aiHasPhi.Get(pcFrontier->muiIndex) == iPromotion)
				{
					continue;
				}
				*aiHasPhi.Get(pcFrontier->muiIndex) = iPromotion;
				sResult = mpcFunction->AddTemporary(psPromotion->sType);
				pcFrontier->AddPhi(psPromotion->sType, sResult);
				maiPhiPromotion.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0xFF);
				*maiPhiPromotion.Get(sResult.Index()) = iPromotion;
				if (*aiQueued.Get(pcFrontier->muiIndex) != iPromotion)
				{
					*aiQueued.Get(pcFrontier->muiIndex) = iPromotion;
					apcWork.Add(pcFrontier);
				}
			}
		}
	}
	aiQueued.Kill();
	aiHasPhi.Kill();
	apcWork.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::PushValue(int32 iPromotion, SIRReference sValue)
{
	int32	iIndex;

	iIndex = (int32)masValues.NumElements();
	masValues.Add(&sValue);
	maiValueLinks.Add(maiTops.Get(iPromotion));
	*maiTops.Get(iPromotion) = iIndex;
	maiPushedPromotions.Add(&iPromotion);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::PopValue(void)
{
	int32	iPromotion;
	int32	iIndex;

	iPromotion = *maiPushedPromotions.Tail();
	maiPushedPromotions.Pop();
	iIndex = *maiTops.Get(iPromotion);
	*maiTops.Get(iPromotion) = *maiValueLinks.Get(iIndex);
	masValues.Pop();
	maiValueLinks.Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::CurrentValue(int32 iPromotion)
{
	int32	iTop;

	iTop = *maiTops.Get(iPromotion);
	return (iTop >= 0) ? *masValues.Get(iTop) : masPromotions.Get(iPromotion)->sInitial;
}


//////////////////////////////////////////////////////////////////////////
//
// Walks the dominator tree from the entry: loads become the value stored
// last on the way (or the phi), stores push values, and each block gives
// its successors' phis their arguments.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::Rename(void)
{
	CArrayIRRenameFrames	asStack;
	SIRRenameFrame*			psFrame;
	SIRRenameFrame*			psNew;
	CIRBlock*				pcChild;
	size					uiPushed;
	size					ui;

	masValues.FakeSetUsedElements(0);
	maiValueLinks.FakeSetUsedElements(0);
	maiPushedPromotions.FakeSetUsedElements(0);
	maiTops.FakeSetUsedElements(0);
	maiTops.GrowToAtLeastNumElements(masPromotions.NumElements(), true, 0xFF);

	asStack.Init();
	psNew = asStack.Add();
	psNew->pcBlock = mpcFunction->GetEntry();
	psNew->uiNextChild = 0;
	RenameBlock(psNew->pcBlock, &uiPushed);
	asStack.Tail()->uiPushed = uiPushed;
	while (asStack.NumElements() > 0)
	{
		psFrame = asStack.Tail();
		if (psFrame->uiNextChild < mcDominators.NumChildren(psFrame->pcBlock))
		{
			pcChild = mcDominators.GetChild(psFrame->pcBlock, psFrame->uiNextChild);
			psFrame->uiNextChild++;
			RenameBlock(pcChild, &uiPushed);
			psNew = asStack.Add();
			psNew->pcBlock = pcChild;
			psNew->uiNextChild = 0;
			psNew->uiPushed = uiPushed;
		}
		else
		{
			for (ui = 0; ui < psFrame->uiPushed; ui++)
			{
				PopValue();
			}
			asStack.Pop();
		}
	}
	asStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::RenameBlock(CIRBlock* pcBlock, size* puiPushed)
{
	size			ui;
	SIRPhi*			psPhi;
	SIRInstruction*	psInstruction;
	int32			iPromotion;
	int32*			piPromotion;

	*puiPushed = 0;
	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		psPhi = pcBlock->masPhis.Get(ui);
		piPromotion = maiPhiPromotion.SafeGet(psPhi->sResult.Index());
		if (piPromotion && (*piPromotion >= 0))
		{
			PushValue(*piPromotion, psPhi->sResult);
			(*puiPushed)++;
		}
	}
	for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		if (psInstruction->uiOpcode == IRO_Load)
		{
			iPromotion = PromotionOf(psInstruction->asArguments[0]);
			if (iPromotion >= 0)
			{
				Replace(psInstruction->sResult, CurrentValue(iPromotion));
			}
		}
		else if (psInstruction->uiOpcode == IRO_Store)
		{
			iPromotion = PromotionOf(psInstruction->asArguments[1]);
			if (iPromotion >= 0)
			{
				PushValue(iPromotion, Replacement(psInstruction->asArguments[0]));
				(*puiPushed)++;
			}
		}
	}
	FillSuccessorPhis(pcBlock);
}


//////////////////////////////////////////////////////////////////////////
//
// pcBlock's successors' promotion phis get pcBlock's current values.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::FillSuccessorPhis(CIRBlock* pcBlock)
{
	size			uiSuccessor;
	size			ui;
	CIRBlock*		pcSuccessor;
	SIRPhi*			psPhi;
	int32*			piPromotion;

	for (uiSuccessor = 0; uiSuccessor < pcBlock->NumSuccessors(); uiSuccessor++)
	{
		pcSuccessor = pcBlock->GetSuccessor(uiSuccessor);
		if ((uiSuccessor == 1) && (pcSuccessor == pcBlock->msJump.pcThen))
		{
			continue;
		}
		for (ui = 0; ui < pcSuccessor->masPhis.NumElements(); ui++)
		{
			psPhi = pcSuccessor->masPhis.Get(ui);
			piPromotion = maiPhiPromotion.SafeGet(psPhi->sResult.Index());
			if (piPromotion && (*piPromotion >= 0))
			{
				pcSuccessor->AddPhiArgument(psPhi, pcBlock, Replacement(CurrentValue(*piPromotion)));
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The promoted slots, argaddrs and their loads and stores go.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::RemovePromoted(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	bool			bRemove;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = pcBlock->masInstructions.NumElements(); uiInstruction > 0; uiInstruction--)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction - 1);
			switch (psInstruction->uiOpcode)
			{
			case IRO_Load:
				bRemove = PromotionOf(psInstruction->asArguments[0]) >= 0;
				break;
			case IRO_Store:
				bRemove = PromotionOf(psInstruction->asArguments[1]) >= 0;
				break;
			case IRO_Slot:
			case IRO_ArgAddress:
				bRemove = PromotionOf(psInstruction->sResult) >= 0;
				break;
			default:
				bRemove = false;
				break;
			}
			if (bRemove)
			{
				pcBlock->masInstructions.RemoveAt(uiInstruction - 1, true);
			}
		}
	}
}


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
	case IRO_Eq:	*pulliResult = a == b;	break;
	case IRO_Ne:	*pulliResult = a != b;	break;
	case IRO_ULt:	*pulliResult = a < b;	break;
	case IRO_ULe:	*pulliResult = a <= b;	break;
	case IRO_UGt:	*pulliResult = a > b;	break;
	case IRO_UGe:	*pulliResult = a >= b;	break;
	case IRO_SLt:	*pulliResult = IRSignExtend(sType, a) < sb;	break;
	case IRO_SLe:	*pulliResult = IRSignExtend(sType, a) <= sb;	break;
	case IRO_SGt:	*pulliResult = IRSignExtend(sType, a) > sb;	break;
	case IRO_SGe:	*pulliResult = IRSignExtend(sType, a) >= sb;	break;
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Instructions of constants are their constant results, and so are
// comparisons a branch on the way in decides (ImpliedComparison); x * 2^n,
// unsigned x / 2^n and x % 2^n are shifts and ands; comparisons of
// extended values are narrowed (NarrowComparison), extensions that are
// undone go (UndoesExtension, ConvertPhi) and so do x + 0 and the like
// (IsIdentity) and sums that cancel ((x + y) - y, Cancels).
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
			else if (PointerOffset(psInstruction))
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


#define IR_EXTENDED_DEPTH	6


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
			return (lliValue >= -(((int64)1) << (iBits - 1))) && (lliValue < (((int64)1) << (iBits - 1)));
		}
		return IRTruncate(psConstant->sType, psConstant->ulliBits) < (((uint64)1) << iBits);
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


#define IR_CSE_SEARCH_LIMIT		4096


//////////////////////////////////////////////////////////////////////////
//
// True if two instructions work out the same value (either way round for
// a commutative opcode).
//
//////////////////////////////////////////////////////////////////////////
static bool IROptimiserSameInstruction(CIRFunction* pcFunction, SIRInstruction* psEarlier, SIRInstruction* psInstruction)
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


//////////////////////////////////////////////////////////////////////////
//
// Each temporary's definition (mcDefinitions) and each parameter's first
// argaddr.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::FindDefinitions(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	size			uiParameter;

	mcDefinitions.Compute(mpcFunction);
	masArgAddresses.FakeSetUsedElements(0);
	masArgAddresses.GrowToAtLeastNumElements(mpcFunction->masParameters.NumElements(), true, 0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			uiParameter = (psInstruction->uiOpcode == IRO_ArgAddress) ? (size)mpcFunction->GetSigned(psInstruction->asArguments[0]) : SIZE_MAX;
			if ((uiParameter < masArgAddresses.NumElements()) && masArgAddresses.Get(uiParameter)->IsNone())
			{
				*masArgAddresses.Get(uiParameter) = psInstruction->sResult;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The blocks numbered with their predecessors, the dominators, the
// definitions and the loops.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::ComputeLoops(void)
{
	mpcFunction->Renumber();
	mpcFunction->ComputePredecessors();
	mcDominators.Compute(mpcFunction);
	FindDefinitions();
	mcLoops.Compute(mpcFunction, &mcDominators);
}


//////////////////////////////////////////////////////////////////////////
//
// What a load of sType reads at sRoot + lliOffset when sRoot is a symbol
// whose contents never change (a constant global, or known data such as a
// vtable): a constant, or a symbol for a pointer to one (a vtable entry);
// IRNone when it isn't known.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::ConstantData(SIRReference sRoot, int64 lliOffset, SIRType sType)
{
	CIRGlobal*		pcData;
	SIRDataItem*	psItem;
	size			ui;
	int64			lliStart;
	int64			lliSize;
	int				iBytes;
	int				i;
	uint64			ulliBits;

	if (!sRoot.IsSymbol() || (lliOffset < 0) || sType.IsFloat())
	{
		return IRNone();
	}
	pcData = mpcModule->FindConstantData(sRoot.Index());
	if (pcData == NULL)
	{
		return IRNone();
	}
	iBytes = mpcModule->GetSize(sType);
	lliStart = 0;
	for (ui = 0; ui < pcData->masItems.NumElements(); ui++)
	{
		psItem = pcData->masItems.Get(ui);
		switch (psItem->uiKind)
		{
		case IRDK_Value:
		case IRDK_Symbol:
			lliSize = mpcModule->GetSize(psItem->sType);
			break;
		case IRDK_Bytes:
			lliSize = (int64)psItem->uiLength;
			break;
		default:
			lliSize = (int64)psItem->ulliValue;
			break;
		}
		if (lliOffset < lliStart + lliSize)
		{
			if (lliOffset + iBytes > lliStart + lliSize)
			{
				//Across two items.
				return IRNone();
			}
			switch (psItem->uiKind)
			{
			case IRDK_Value:
				return ((lliOffset == lliStart) && psItem->sType.Equals(sType)) ? mpcFunction->AddConstant(sType, psItem->ulliValue) : IRNone();
			case IRDK_Symbol:
				return ((lliOffset == lliStart) && sType.IsPointer() && (psItem->lliOffset == 0)) ? IRReference(IRRK_Symbol, (uint32)psItem->ulliValue) : IRNone();
			case IRDK_Bytes:
				if (!sType.IsInteger())
				{
					return IRNone();
				}
				ulliBits = 0;
				for (i = iBytes - 1; i >= 0; i--)
				{
					ulliBits = (ulliBits << 8) | *pcData->macBytes.Get(psItem->uiStart + (size)(lliOffset - lliStart) + i);
				}
				return mpcFunction->AddConstant(sType, ulliBits);
			default:
				return (sType.IsInteger() || sType.IsPointer()) ? mpcFunction->AddConstant(sType, 0) : IRNone();
			}
		}
		lliStart += lliSize;
	}
	return IRNone();
}


//////////////////////////////////////////////////////////////////////////
//
// An address as a root (a temporary or symbol that isn't a ptradd of a
// constant) and a constant offset from it.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SplitAddress(SIRReference sAddress, SIRReference* psRoot, int64* plliOffset)
{
	SIRInstruction*	psDefinition;
	int				iSteps;
	size			uiParameter;

	*psRoot = sAddress;
	*plliOffset = 0;
	for (iSteps = 0; iSteps < 16; iSteps++)
	{
		psDefinition = mcDefinitions.Instruction(*psRoot);
		if ((psDefinition == NULL) || (psDefinition->uiOpcode != IRO_PtrAdd) || !psDefinition->asArguments[1].IsConstant())
		{
			break;
		}
		*plliOffset += mpcFunction->GetSigned(psDefinition->asArguments[1]);
		*psRoot = psDefinition->asArguments[0];
	}

	//Every argaddr of a parameter is the same memory: the first is its root.
	psDefinition = mcDefinitions.Instruction(*psRoot);
	if (psDefinition && (psDefinition->uiOpcode == IRO_ArgAddress))
	{
		uiParameter = (size)mpcFunction->GetSigned(psDefinition->asArguments[0]);
		if (uiParameter < masArgAddresses.NumElements())
		{
			*psRoot = *masArgAddresses.Get(uiParameter);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if a root is a slot or an argaddr: memory no other root names.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::IsFrameRoot(SIRReference sRoot)
{
	SIRInstruction*	psDefinition;

	psDefinition = mcDefinitions.Instruction(sRoot);
	return psDefinition && ((psDefinition->uiOpcode == IRO_Slot) || (psDefinition->uiOpcode == IRO_ArgAddress));
}


//////////////////////////////////////////////////////////////////////////
//
// What a root's pointer is based on: a pointer loaded from a restrict
// symbol or a parameter, through ptradds (with any offset) and phis whose
// values are all based on the same.  Else IRPB_Unknown.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::PointerBase(SIRReference sRoot, SIRPointerBase* psBase)
{
	SIRReference	asWork[IR_BASE_VALUES];
	SIRReference	asSeen[IR_BASE_VALUES];
	size			uiWork;
	size			uiSeen;
	size			ui;
	SIRReference	sValue;
	SIRInstruction*	psDefinition;
	SIRPhi*			psPhi;
	SIRPointerBase	sBase;
	bool			bFound;
	int				iSteps;
	int				iParameter;

	psBase->eKind = IRPB_Unknown;
	if (!mbRestrict)
	{
		return;
	}

	asWork[0] = sRoot;
	uiWork = 1;
	uiSeen = 0;
	bFound = false;
	while (uiWork > 0)
	{
		sValue = asWork[--uiWork];
		for (iSteps = 0; iSteps < 16; iSteps++)
		{
			psDefinition = mcDefinitions.Instruction(sValue);
			if ((psDefinition == NULL) || (psDefinition->uiOpcode != IRO_PtrAdd))
			{
				break;
			}
			sValue = psDefinition->asArguments[0];
		}
		for (ui = 0; ui < uiSeen; ui++)
		{
			if (asSeen[ui].Equals(sValue))
			{
				break;
			}
		}
		if (ui < uiSeen)
		{
			continue;
		}
		if (uiSeen == IR_BASE_VALUES)
		{
			psBase->eKind = IRPB_Unknown;
			return;
		}
		asSeen[uiSeen++] = sValue;

		psPhi = mcDefinitions.Phi(sValue);
		if (psPhi)
		{
			for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
			{
				if (uiWork == IR_BASE_VALUES)
				{
					psBase->eKind = IRPB_Unknown;
					return;
				}
				asWork[uiWork++] = psPhi->asArguments.Get(ui)->sValue;
			}
			continue;
		}

		iParameter = mpcFunction->FindParameter(sValue);
		psDefinition = mcDefinitions.Instruction(sValue);
		if (iParameter >= 0)
		{
			sBase.eKind = IRPB_Parameter;
			sBase.uiIndex = (uint32)iParameter;
			sBase.bRestrict = mpcFunction->IsRestrictParameter((size)iParameter);
		}
		else if (psDefinition && (psDefinition->uiOpcode == IRO_Load) && psDefinition->asArguments[0].IsSymbol() && mpcModule->IsRestrict(psDefinition->asArguments[0].Index()))
		{
			sBase.eKind = IRPB_RestrictSymbol;
			sBase.uiIndex = psDefinition->asArguments[0].Index();
			sBase.bRestrict = true;
		}
		else
		{
			psBase->eKind = IRPB_Unknown;
			return;
		}

		if (!bFound)
		{
			*psBase = sBase;
			bFound = true;
		}
		else if ((psBase->eKind != sBase.eKind) || (psBase->uiIndex != sBase.uiIndex))
		{
			psBase->eKind = IRPB_Unknown;
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False only if the two accesses can't touch the same bytes: the same root
// at offsets that don't overlap, different symbols, different slots or
// argaddrs, or a slot and a symbol.  What is reached through a restrict
// pointer (a restrict symbol's value or a restrict parameter, see
// PointerBase) is no symbol's or slot's, and isn't reached through a
// pointer with another base when either base is restrict and a parameter,
// or both are restrict.  A restrict symbol and a plain parameter may alias:
// the caller may have passed the symbol's value.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::MayAlias(SIRAvailable* psAvailable, SIRReference sRoot, int64 lliOffset, int iBytes)
{
	bool			bFrame1;
	bool			bFrame2;
	SIRPointerBase	sBase1;
	SIRPointerBase	sBase2;
	bool			bRestrict1;
	bool			bRestrict2;

	if (psAvailable->sRoot.Equals(sRoot))
	{
		return (psAvailable->lliOffset < lliOffset + iBytes) && (lliOffset < psAvailable->lliOffset + mpcModule->GetSize(psAvailable->sType));
	}
	bFrame1 = IsFrameRoot(psAvailable->sRoot) || psAvailable->sRoot.IsSymbol();
	bFrame2 = IsFrameRoot(sRoot) || sRoot.IsSymbol();
	if (bFrame1 && bFrame2)
	{
		return false;
	}

	sBase1.eKind = IRPB_Unknown;
	sBase2.eKind = IRPB_Unknown;
	if (!bFrame1)
	{
		PointerBase(psAvailable->sRoot, &sBase1);
	}
	if (!bFrame2)
	{
		PointerBase(sRoot, &sBase2);
	}
	bRestrict1 = (sBase1.eKind != IRPB_Unknown) && sBase1.bRestrict;
	bRestrict2 = (sBase2.eKind != IRPB_Unknown) && sBase2.bRestrict;
	if ((bRestrict1 && bFrame2) || (bRestrict2 && bFrame1))
	{
		return false;
	}
	if ((sBase1.eKind == IRPB_Unknown) || (sBase2.eKind == IRPB_Unknown))
	{
		return true;
	}
	if ((sBase1.eKind == sBase2.eKind) && (sBase1.uiIndex == sBase2.uiIndex))
	{
		return true;
	}
	if ((bRestrict1 && bRestrict2) || (bRestrict1 && (sBase1.eKind == IRPB_Parameter)) || (bRestrict2 && (sBase2.eKind == IRPB_Parameter)))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Notes that sValue is in memory at sRoot + lliOffset, unless too much is
// noted already.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::AddAvailable(CArrayIRAvailables* pasAvailable, SIRReference sRoot, int64 lliOffset, SIRType sType, SIRReference sValue)
{
	SIRAvailable*	psAvailable;

	if (pasAvailable->NumElements() < IR_AVAILABLE_LIMIT)
	{
		psAvailable = pasAvailable->Add();
		psAvailable->sRoot = sRoot;
		psAvailable->lliOffset = lliOffset;
		psAvailable->sType = sType;
		psAvailable->sValue = sValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What a block entered from several blocks knows is in memory: what all
// of them leave known, as the same value (none if one is a loop's way back,
// not walked yet).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::MergeAvailable(CIRBlock* pcBlock, CArrayIRAvailables* pasAvailable, CArrayIRAvailables* pasEnds, CArrayTemplate<SIRBlockEnd>* pasBlockEnds)
{
	size			uiPredecessor;
	size			uiFirst;
	size			uiOther;
	SIRBlockEnd*	psFirstEnd;
	SIRBlockEnd*	psEnd;
	SIRAvailable*	psCandidate;
	SIRAvailable*	psOther;
	bool			bEverywhere;
	bool			bFound;

	if (pcBlock->mapcPredecessors.NumElements() < 2)
	{
		return;
	}
	for (uiPredecessor = 0; uiPredecessor < pcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
	{
		if (pasBlockEnds->Get(pcBlock->mapcPredecessors.GetPtr(uiPredecessor)->muiIndex)->iStart < 0)
		{
			return;
		}
	}
	psFirstEnd = pasBlockEnds->Get(pcBlock->mapcPredecessors.GetPtr(0)->muiIndex);
	for (uiFirst = 0; uiFirst < (size)psFirstEnd->iCount; uiFirst++)
	{
		psCandidate = pasEnds->Get(psFirstEnd->iStart + uiFirst);
		bEverywhere = true;
		for (uiPredecessor = 1; bEverywhere && (uiPredecessor < pcBlock->mapcPredecessors.NumElements()); uiPredecessor++)
		{
			psEnd = pasBlockEnds->Get(pcBlock->mapcPredecessors.GetPtr(uiPredecessor)->muiIndex);
			bFound = false;
			for (uiOther = 0; uiOther < (size)psEnd->iCount; uiOther++)
			{
				psOther = pasEnds->Get(psEnd->iStart + uiOther);
				if (psOther->sRoot.Equals(psCandidate->sRoot) && (psOther->lliOffset == psCandidate->lliOffset) && psOther->sType.Equals(psCandidate->sType) && Replacement(psOther->sValue).Equals(Replacement(psCandidate->sValue)))
				{
					bFound = true;
					break;
				}
			}
			bEverywhere = bFound;
		}
		if (bEverywhere)
		{
			AddAvailable(pasAvailable, psCandidate->sRoot, psCandidate->lliOffset, psCandidate->sType, Replacement(psCandidate->sValue));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A load of what is known to be in memory is that value: what an earlier
// load read or a store wrote, in its block or in the blocks before it (all
// the blocks that enter it agreeing), with no call, volatile load or store
// that may touch it between.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RemoveRedundantLoads(void)
{
	CArrayIRAvailables				asAvailable;
	CArrayIRAvailables				asEnds;			//What each block leaves known, from asBlockEnds.
	CArrayTemplate<SIRBlockEnd>		asBlockEnds;	//Per block index.
	size							ui;
	size							uiInstruction;
	size							uiAvailable;
	CIRBlock*						pcBlock;
	SIRInstruction*					psInstruction;
	SIRAvailable*					psAvailable;
	SIRBlockEnd*					psEnd;
	SIRReference					sRoot;
	SIRReference					sConstant;
	int64							lliOffset;
	bool							bChanged;
	bool							bFound;

	mpcFunction->Renumber();
	mpcFunction->ComputePredecessors();
	mcDominators.Compute(mpcFunction);
	ClearReplacements();
	FindDefinitions();

	asAvailable.Init();
	asEnds.Init();
	asBlockEnds.Init();
	asBlockEnds.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0xFF);
	bChanged = false;
	for (ui = 0; ui < mcDominators.NumReachable(); ui++)
	{
		pcBlock = mcDominators.GetReversePostOrder(ui);
		asAvailable.FakeSetUsedElements(0);
		if (pcBlock->mapcPredecessors.NumElements() == 1)
		{
			psEnd = asBlockEnds.Get(pcBlock->mapcPredecessors.GetPtr(0)->muiIndex);
			if (psEnd->iStart >= 0)
			{
				asAvailable.InsertBlockAt(asEnds.Get(psEnd->iStart), psEnd->iCount, 0);
			}
		}
		else
		{
			MergeAvailable(pcBlock, &asAvailable, &asEnds, &asBlockEnds);
		}

		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			psInstruction->asArguments[0] = Replacement(psInstruction->asArguments[0]);
			psInstruction->asArguments[1] = Replacement(psInstruction->asArguments[1]);
			if ((psInstruction->uiOpcode == IRO_Load) && !(psInstruction->uiFlags & IRF_Volatile) && psInstruction->sResult.IsTemporary())
			{
				SplitAddress(psInstruction->asArguments[0], &sRoot, &lliOffset);
				sConstant = ConstantData(sRoot, lliOffset, psInstruction->sType);
				if (!sConstant.IsNone())
				{
					Replace(psInstruction->sResult, sConstant);
					bChanged = true;
					continue;
				}
				bFound = false;
				for (uiAvailable = 0; uiAvailable < asAvailable.NumElements(); uiAvailable++)
				{
					psAvailable = asAvailable.Get(uiAvailable);
					if (psAvailable->sRoot.Equals(sRoot) && (psAvailable->lliOffset == lliOffset) && psAvailable->sType.Equals(psInstruction->sType))
					{
						Replace(psInstruction->sResult, psAvailable->sValue);
						bChanged = true;
						bFound = true;
						break;
					}
				}
				if (!bFound)
				{
					AddAvailable(&asAvailable, sRoot, lliOffset, psInstruction->sType, psInstruction->sResult);
				}
			}
			else if (psInstruction->uiOpcode == IRO_Store)
			{
				SplitAddress(psInstruction->asArguments[1], &sRoot, &lliOffset);
				for (uiAvailable = asAvailable.NumElements(); uiAvailable > 0; uiAvailable--)
				{
					if (MayAlias(asAvailable.Get(uiAvailable - 1), sRoot, lliOffset, mpcModule->GetSize(psInstruction->sType)))
					{
						asAvailable.RemoveAt(uiAvailable - 1, true);
					}
				}
				if (!(psInstruction->uiFlags & IRF_Volatile))
				{
					AddAvailable(&asAvailable, sRoot, lliOffset, psInstruction->sType, psInstruction->asArguments[0]);
				}
			}
			else if (HasEffect(psInstruction) && (psInstruction->uiOpcode != IRO_Arg))
			{
				asAvailable.FakeSetUsedElements(0);
			}
		}

		psEnd = asBlockEnds.Get(pcBlock->muiIndex);
		psEnd->iStart = (int32)asEnds.NumElements();
		psEnd->iCount = (int32)asAvailable.NumElements();
		if (asAvailable.NumElements() > 0)
		{
			asEnds.InsertBlockAt(asAvailable.Get(0), asAvailable.NumElements(), asEnds.NumElements());
		}
	}
	asBlockEnds.Kill();
	asEnds.Kill();
	asAvailable.Kill();
	if (bChanged)
	{
		ApplyReplacements();
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// The narrow value sValue is sext or zext (uiOpcode) of, or a constant
// that is such an extension of a narrow constant of *psType; IRNone if
// not.  *psType is set from the first extension found.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::Unextended(SIRReference sValue, uint16 uiOpcode, SIRType* psType)
{
	SIRInstruction*	psDefinition;

	psDefinition = mcDefinitions.Instruction(sValue);
	if (psDefinition)
	{
		if ((psDefinition->uiOpcode == uiOpcode) && (psType->IsNone() || mpcFunction->TypeOf(psDefinition->asArguments[0]).Equals(*psType)))
		{
			*psType = mpcFunction->TypeOf(psDefinition->asArguments[0]);
			return psDefinition->asArguments[0];
		}
		return IRNone();
	}
	if (sValue.IsConstant() && !psType->IsNone() && Extended(sValue, psType->IntegerBits(), uiOpcode == IRO_SExt, 0))
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
bool CIROptimiser::NarrowComparison(SIRInstruction* psInstruction)
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
	if ((uiExtension == IRO_ZExt) && (psInstruction->uiOpcode >= IRO_SLt) && (psInstruction->uiOpcode <= IRO_SGe))
	{
		psInstruction->uiOpcode += IRO_ULt - IRO_SLt;
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
void CIROptimiser::ComparisonRange(SIRInstruction* psInstruction)
{
	SIRType		sNarrow;
	int			iConstant;
	uint64		ulliConstant;
	bool		bTrue;
	CChars		szLine;

	if (!mpszWarnings)
	{
		return;
	}
	for (iConstant = 0; iConstant < 2; iConstant++)
	{
		sNarrow = IRType(IRTK_None);
		if (psInstruction->asArguments[iConstant].IsConstant() && !Unextended(psInstruction->asArguments[1 - iConstant], IRO_ZExt, &sNarrow).IsNone() &&
			!Extended(psInstruction->asArguments[iConstant], sNarrow.IntegerBits(), false, 0))
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
	if (!strstr(mpszWarnings->Text(), szLine.Text()))
	{
		mpszWarnings->Append(szLine);
	}
	szLine.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The slot and constant offset an address is (a slot, or a ptradd of one
// and a constant); false if it is neither.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SlotOffset(SIRReference sAddress, SIRReference* psSlot, int64* plliOffset)
{
	SIRInstruction*	psDefinition;

	psDefinition = mcDefinitions.Instruction(sAddress);
	if (psDefinition && (psDefinition->uiOpcode == IRO_PtrAdd) && psDefinition->asArguments[1].IsConstant())
	{
		*plliOffset = mpcFunction->GetSigned(psDefinition->asArguments[1]);
		psDefinition = mcDefinitions.Instruction(psDefinition->asArguments[0]);
		if (psDefinition && (psDefinition->uiOpcode == IRO_Slot))
		{
			*psSlot = psDefinition->sResult;
			return true;
		}
		return false;
	}
	if (psDefinition && (psDefinition->uiOpcode == IRO_Slot))
	{
		*psSlot = sAddress;
		*plliOffset = 0;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The type a slot's bytes at lliOffset are loaded and stored at when it is
// one type, or a pointer and an integer of a pointer's size (a pointer
// field copied as an integer: an object returned by value): the pointer.
// IRTK_None for anything else (or a slot with an access that isn't a load
// or store).  mcDefinitions must be found.
//
//////////////////////////////////////////////////////////////////////////
SIRType CIROptimiser::SlotOffsetType(SIRReference sSlot, int64 lliOffset)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sFoundSlot;
	SIRType			sType;
	SIRType			sInteger;
	int64			lliFound;
	size			ui;
	size			uj;
	bool			bPointer;

	bPointer = false;
	sInteger = IRType(IRTK_None);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if (((psInstruction->uiOpcode != IRO_Load) && (psInstruction->uiOpcode != IRO_Store)) ||
				!SlotOffset(psInstruction->asArguments[(psInstruction->uiOpcode == IRO_Load) ? 0 : 1], &sFoundSlot, &lliFound) ||
				!sFoundSlot.Equals(sSlot) || (lliFound != lliOffset))
			{
				continue;
			}
			sType = psInstruction->sType;
			if (sType.IsPointer())
			{
				bPointer = true;
			}
			else if (sType.IsInteger() && (mpcModule->GetSize(sType) == mpcModule->GetSize(IRType(IRTK_Pointer))) &&
					 (sInteger.Is(IRTK_None) || sInteger.Equals(sType)))
			{
				sInteger = sType;
			}
			else
			{
				return IRType(IRTK_None);
			}
		}
	}
	return (bPointer && !sInteger.Is(IRTK_None)) ? IRType(IRTK_Pointer) : IRType(IRTK_None);
}


//////////////////////////////////////////////////////////////////////////
//
// A slot's pointer that is also loaded or stored as an integer of a
// pointer's size (an object returned by value is copied a word at a time)
// is only loaded and stored as a pointer: the integer is converted
// (ptrtoi after a load, itoptr before a store), so the slot can be split
// and promoted.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::UnifySlotTypes(void)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sSlot;
	SIRReference	sValue;
	SIRType			sType;
	int64			lliOffset;
	size			ui;
	size			uj;
	bool			bChanged;

	FindDefinitions();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = pcBlock->masInstructions.NumElements(); uj > 0; uj--)
		{
			psInstruction = pcBlock->masInstructions.Get(uj - 1);
			if (((psInstruction->uiOpcode != IRO_Load) && (psInstruction->uiOpcode != IRO_Store)) || (psInstruction->uiFlags & IRF_Volatile) ||
				!psInstruction->sType.IsInteger() || ((psInstruction->uiOpcode == IRO_Load) && !psInstruction->sResult.IsTemporary()) ||
				!SlotOffset(psInstruction->asArguments[(psInstruction->uiOpcode == IRO_Load) ? 0 : 1], &sSlot, &lliOffset))
			{
				continue;
			}
			sType = SlotOffsetType(sSlot, lliOffset);
			if (!sType.IsPointer())
			{
				continue;
			}
			if (psInstruction->uiOpcode == IRO_Load)
			{
				//x = load iN p is t = load ptr p; x = ptrtoi t.
				sValue = InsertInstruction(pcBlock, uj - 1, IRO_Load, IRType(IRTK_Pointer), psInstruction->asArguments[0], IRNone());
				pcBlock->masInstructions.Get(uj - 1)->uiFlags = pcBlock->masInstructions.Get(uj)->uiFlags;
				psInstruction = pcBlock->masInstructions.Get(uj);
				psInstruction->uiOpcode = IRO_PtrToI;
				psInstruction->asArguments[0] = sValue;
				psInstruction->asArguments[1] = IRNone();
			}
			else
			{
				//store iN x, p is t = itoptr x; store ptr t, p.
				sValue = InsertInstruction(pcBlock, uj - 1, IRO_IToPtr, IRType(IRTK_Pointer), psInstruction->asArguments[0], IRNone());
				psInstruction = pcBlock->masInstructions.Get(uj);
				psInstruction->sType = IRType(IRTK_Pointer);
				psInstruction->asArguments[0] = sValue;
			}
			bChanged = true;
			FindDefinitions();
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Slots go to the start of the entry block (inlining leaves them in other
// blocks), and a slot only loaded and stored, at constant offsets that
// don't overlap and each at one type (an object's fields), becomes a slot
// per offset, which slot promotion can make temporaries.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SplitSlots(void)
{
	CArrayTemplate<SIRSlotAccess>	asAccesses;
	CArrayTemplate<uint8>			auiInvalid;		//Per temporary (a slot's): 1 if it can't be split.
	size							ui;
	size							uiInstruction;
	size							uiArgument;
	size							uiAccess;
	size							uiOther;
	CIRBlock*						pcBlock;
	CIRBlock*						pcEntry;
	SIRInstruction*					psInstruction;
	SIRInstruction					sMoved;
	SIRSlotAccess*					psAccess;
	SIRSlotAccess*					psOther;
	SIRSlotAccess					sAccess;
	SIRReference					sSlot;
	SIRReference					sValue;
	SIRReference					sNew;
	int64							lliOffset;
	bool							bAccess;
	bool							bChanged;

	//Every slot to the start of the entry block.
	bChanged = false;
	pcEntry = mpcFunction->GetEntry();
	for (ui = 1; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = pcBlock->masInstructions.NumElements(); uiInstruction > 0; uiInstruction--)
		{
			if (pcBlock->masInstructions.Get(uiInstruction - 1)->uiOpcode == IRO_Slot)
			{
				sMoved = *pcBlock->masInstructions.Get(uiInstruction - 1);
				pcBlock->masInstructions.RemoveAt(uiInstruction - 1, true);
				pcEntry->masInstructions.InsertAt(&sMoved, 0);
				bChanged = true;
			}
		}
	}

	FindDefinitions();
	asAccesses.Init();
	auiInvalid.Init();
	auiInvalid.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);

	//Each use of a slot (or of a ptradd of it and a constant) must be a load's or store's address.
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masPhis.NumElements(); uiInstruction++)
		{
			for (uiArgument = 0; uiArgument < pcBlock->masPhis.Get(uiInstruction)->asArguments.NumElements(); uiArgument++)
			{
				if (SlotOffset(pcBlock->masPhis.Get(uiInstruction)->asArguments.Get(uiArgument)->sValue, &sSlot, &lliOffset))
				{
					*auiInvalid.Get(sSlot.Index()) = 1;
				}
			}
		}
		if (SlotOffset(pcBlock->msJump.sArgument, &sSlot, &lliOffset))
		{
			*auiInvalid.Get(sSlot.Index()) = 1;
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				sValue = psInstruction->asArguments[uiArgument];
				if (!SlotOffset(sValue, &sSlot, &lliOffset))
				{
					continue;
				}
				//The ptradd that makes an offset address isn't a use of the slot's memory.
				if ((psInstruction->uiOpcode == IRO_PtrAdd) && (uiArgument == 0) && sValue.Equals(sSlot) && psInstruction->asArguments[1].IsConstant())
				{
					continue;
				}
				bAccess = !(psInstruction->uiFlags & IRF_Volatile) &&
						  (((psInstruction->uiOpcode == IRO_Load) && (uiArgument == 0)) || ((psInstruction->uiOpcode == IRO_Store) && (uiArgument == 1)));
				if (!bAccess)
				{
					*auiInvalid.Get(sSlot.Index()) = 1;
					continue;
				}
				sAccess.sSlot = sSlot;
				sAccess.lliOffset = lliOffset;
				sAccess.sType = psInstruction->sType;
				sAccess.sNew = IRNone();
				asAccesses.Add(&sAccess);
			}
		}
	}

	//One type at each offset, offsets that don't overlap, inside the slot; more than one offset, or not 0.
	for (uiAccess = 0; uiAccess < asAccesses.NumElements(); uiAccess++)
	{
		psAccess = asAccesses.Get(uiAccess);
		if (*auiInvalid.Get(psAccess->sSlot.Index()))
		{
			continue;
		}
		if ((psAccess->lliOffset < 0) || (psAccess->lliOffset + mpcModule->GetSize(psAccess->sType) > mpcFunction->GetSigned(mcDefinitions.Instruction(psAccess->sSlot)->asArguments[0])))
		{
			*auiInvalid.Get(psAccess->sSlot.Index()) = 1;
			continue;
		}
		for (uiOther = 0; uiOther < asAccesses.NumElements(); uiOther++)
		{
			psOther = asAccesses.Get(uiOther);
			if (!psOther->sSlot.Equals(psAccess->sSlot))
			{
				continue;
			}
			if ((psOther->lliOffset == psAccess->lliOffset) ? !psOther->sType.Equals(psAccess->sType) :
				((psOther->lliOffset < psAccess->lliOffset + mpcModule->GetSize(psAccess->sType)) && (psAccess->lliOffset < psOther->lliOffset + mpcModule->GetSize(psOther->sType))))
			{
				*auiInvalid.Get(psAccess->sSlot.Index()) = 1;
				break;
			}
		}
	}

	//A slot per offset; the old slot and its ptradds are replaced by them (and die).
	for (uiAccess = 0; uiAccess < asAccesses.NumElements(); uiAccess++)
	{
		psAccess = asAccesses.Get(uiAccess);
		if (*auiInvalid.Get(psAccess->sSlot.Index()) || !psAccess->sNew.IsNone())
		{
			continue;
		}
		if (psAccess->lliOffset == 0)
		{
			for (uiOther = 0; (uiOther < asAccesses.NumElements()) && (asAccesses.Get(uiOther)->lliOffset == 0 || !asAccesses.Get(uiOther)->sSlot.Equals(psAccess->sSlot)); uiOther++)
			{
			}
			if (uiOther == asAccesses.NumElements())
			{
				//Only ever at 0: already a slot slot promotion takes.
				continue;
			}
		}
		sNew = mpcFunction->AddTemporary(IRType(IRTK_Pointer));
		pcEntry->masInstructions.InsertAt(0);
		psInstruction = pcEntry->masInstructions.Get(0);
		psInstruction->uiOpcode = IRO_Slot;
		psInstruction->uiFlags = 0;
		psInstruction->uiNamed = 0;
		psInstruction->sType = IRType(IRTK_Pointer);
		psInstruction->sResult = sNew;
		psInstruction->asArguments[0] = mpcFunction->AddInteger(IRType(IRTK_I32), mpcModule->GetSize(psAccess->sType));
		psInstruction->asArguments[1] = mpcFunction->AddInteger(IRType(IRTK_I32), 4);
		for (uiOther = uiAccess; uiOther < asAccesses.NumElements(); uiOther++)
		{
			psOther = asAccesses.Get(uiOther);
			if (psOther->sSlot.Equals(psAccess->sSlot) && (psOther->lliOffset == psAccess->lliOffset))
			{
				psOther->sNew = sNew;
			}
		}
		bChanged = true;
	}

	//Loads and stores of a split slot use the new slots.
	if (bChanged)
	{
		FindDefinitions();
		for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				uiArgument = (psInstruction->uiOpcode == IRO_Load) ? 0 : 1;
				if (((psInstruction->uiOpcode != IRO_Load) && (psInstruction->uiOpcode != IRO_Store)) || !SlotOffset(psInstruction->asArguments[uiArgument], &sSlot, &lliOffset))
				{
					continue;
				}
				for (uiAccess = 0; uiAccess < asAccesses.NumElements(); uiAccess++)
				{
					psAccess = asAccesses.Get(uiAccess);
					if (psAccess->sSlot.Equals(sSlot) && (psAccess->lliOffset == lliOffset) && !psAccess->sNew.IsNone())
					{
						psInstruction->asArguments[uiArgument] = psAccess->sNew;
						break;
					}
				}
			}
		}
	}
	auiInvalid.Kill();
	asAccesses.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// True if an instruction may be moved out of a loop when its operands are
// worked out outside it: integer operations (not division), conversions,
// comparisons and loads that aren't volatile.
//
//////////////////////////////////////////////////////////////////////////
static bool IROptimiserHoistable(CIRFunction* pcFunction, SIRInstruction* psInstruction)
{
	uint64	ulliDivisor;

	switch (psInstruction->uiOpcode)
	{
	case IRO_UDiv:
	case IRO_URem:
	case IRO_SDiv:
	case IRO_SRem:
		//Only by a constant that can't trap or overflow.
		if (!psInstruction->asArguments[1].IsConstant() || !psInstruction->sResult.IsTemporary())
		{
			return false;
		}
		ulliDivisor = IRTruncate(psInstruction->sType, pcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
		return (ulliDivisor != 0) && (ulliDivisor != IRTruncate(psInstruction->sType, (uint64)-1));
	case IRO_Add:
	case IRO_Sub:
	case IRO_Mul:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
	case IRO_Neg:
	case IRO_Not:
	case IRO_PtrAdd:
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
		return psInstruction->sResult.IsTemporary();
	case IRO_Load:
		return psInstruction->sResult.IsTemporary() && !(psInstruction->uiFlags & IRF_Volatile);
	default:
		return IsIRIntegerComparison(psInstruction->uiOpcode) && psInstruction->sResult.IsTemporary();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue + iStep (1 or -1) can't wrap: a constant that isn't the
// largest (smallest) of its type, or an extension of a narrower value.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::CanStep(SIRReference sValue, int iStep)
{
	SIRInstruction*	psDefinition;
	SIRType			sType;
	int64			lliValue;
	int				iBits;

	sType = mpcFunction->TypeOf(sValue);
	iBits = sType.IntegerBits();
	if (sValue.IsConstant())
	{
		lliValue = mpcFunction->GetSigned(sValue);
		return (iBits >= 64) ? (iStep > 0 ? (lliValue != 0x7FFFFFFFFFFFFFFFLL) : (lliValue != (-0x7FFFFFFFFFFFFFFFLL - 1))) :
							   (iStep > 0 ? (lliValue < (((int64)1 << (iBits - 1)) - 1)) : (lliValue > -((int64)1 << (iBits - 1))));
	}
	psDefinition = mcDefinitions.Instruction(sValue);
	return psDefinition && ((psDefinition->uiOpcode == IRO_SExt) || (psDefinition->uiOpcode == IRO_ZExt)) &&
		   (mpcFunction->TypeOf(psDefinition->asArguments[0]).IntegerBits() < iBits);
}


//////////////////////////////////////////////////////////////////////////
//
// A loop's test that goes on round when a <= b (or b >= a), signed, is
// a < b + 1 when b + 1 can't wrap and the loop doesn't change b (or
// a - 1 < b, the other way round): the 65C64 branches on less in one
// instruction but on less or equal in two.  The new add is moved out of
// the loop by HoistInvariants.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::LessLoopTests(void)
{
	size					uiLoop;
	size					uiBlock;
	CIRBlock*				pcBlock;
	CIRBlock*				pcInside;
	CIRBlock*				pcOutside;
	SIRInstruction*			psCompare;
	SIRInstruction*			psNew;
	SIRReference			sA;
	SIRReference			sB;
	SIRReference			sStepped;
	SIRReference			sTest;
	SIRType					sType;
	uint16					uiOpcode;
	bool					bChanged;
	bool					bStepB;

	ComputeLoops();
	bChanged = false;
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
		{
			pcBlock = mpcFunction->GetBlock(uiBlock);
			if (!mcLoops.Contains(uiLoop, pcBlock) || (pcBlock->msJump.uiKind != IRJ_Branch) || !pcBlock->msJump.sArgument.IsTemporary())
			{
				continue;
			}
			psCompare = mcDefinitions.Instruction(pcBlock->msJump.sArgument);
			if ((psCompare == NULL) || !IsIRSignedComparison(psCompare->uiOpcode))
			{
				continue;
			}

			//The way round the loop and the way out.
			if (mcLoops.Contains(uiLoop, pcBlock->msJump.pcThen) && !mcLoops.Contains(uiLoop, pcBlock->msJump.pcElse))
			{
				pcInside = pcBlock->msJump.pcThen;
				pcOutside = pcBlock->msJump.pcElse;
				uiOpcode = psCompare->uiOpcode;
			}
			else if (mcLoops.Contains(uiLoop, pcBlock->msJump.pcElse) && !mcLoops.Contains(uiLoop, pcBlock->msJump.pcThen))
			{
				pcInside = pcBlock->msJump.pcElse;
				pcOutside = pcBlock->msJump.pcThen;
				uiOpcode = IRInvertedComparison(psCompare->uiOpcode);
			}
			else
			{
				continue;
			}

			//Round when a <= b.
			if (uiOpcode == IRO_SLe)
			{
				sA = psCompare->asArguments[0];
				sB = psCompare->asArguments[1];
			}
			else if (uiOpcode == IRO_SGe)
			{
				sA = psCompare->asArguments[1];
				sB = psCompare->asArguments[0];
			}
			else
			{
				continue;
			}
			if (!mcLoops.IsInvariant(uiLoop, sB, &mcDefinitions) || !CanStep(sB, 1))
			{
				if (!mcLoops.IsInvariant(uiLoop, sA, &mcDefinitions) || !CanStep(sA, -1))
				{
					continue;
				}
				bStepB = false;
			}
			else
			{
				bStepB = true;
			}

			sType = psCompare->sType;
			sStepped = mpcFunction->AddTemporary(sType);
			sTest = mpcFunction->AddTemporary(IRType(IRTK_I1));
			psNew = pcBlock->masInstructions.Add();
			psNew->uiOpcode = bStepB ? IRO_Add : IRO_Sub;
			psNew->uiFlags = 0;
			psNew->uiNamed = 0;
			psNew->sType = sType;
			psNew->sResult = sStepped;
			psNew->asArguments[0] = bStepB ? sB : sA;
			psNew->asArguments[1] = mpcFunction->AddInteger(sType, 1);
			psNew = pcBlock->masInstructions.Add();
			psNew->uiOpcode = IRO_SLt;
			psNew->uiFlags = 0;
			psNew->uiNamed = 0;
			psNew->sType = sType;
			psNew->sResult = sTest;
			psNew->asArguments[0] = bStepB ? sA : sStepped;
			psNew->asArguments[1] = bStepB ? sStepped : sB;
			pcBlock->msJump.sArgument = sTest;
			pcBlock->msJump.pcThen = pcInside;
			pcBlock->msJump.pcElse = pcOutside;
			bChanged = true;
			FindDefinitions();
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A block between a loop's entering block and its head, which the entering
// block's jump now goes to and which jumps to the head: the loop's
// preheader.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIROptimiser::AddPreheader(size uiLoop)
{
	CIRBlock*	pcPreheader;
	CIRBlock*	pcEntering;
	CIRBlock*	pcHead;

	pcEntering = mcLoops.GetLoop(uiLoop)->pcEntering;
	pcHead = mcLoops.GetLoop(uiLoop)->pcHead;
	pcPreheader = mpcFunction->AddBlock();
	pcPreheader->Jump(pcHead);
	RedirectJump(pcEntering, pcHead, pcPreheader);
	RenamePhiArguments(pcHead, pcEntering, pcPreheader);
	mpcFunction->ComputePredecessors();
	mcLoops.SetPreheader(uiLoop, pcPreheader);
	return pcPreheader;
}


//////////////////////////////////////////////////////////////////////////
//
// A preheader for a loop entered from several blocks (each loop before it
// leaving straight to its head, say): the blocks outside that jumped to the
// head jump to it, phis in it take what they passed the head's phis, and the
// head's phis take those from it.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIROptimiser::AddMergedPreheader(size uiLoop)
{
	CIRBlock*		pcPreheader;
	CIRBlock*		pcHead;
	CIRBlock*		pcPredecessor;
	SIRPhi*			psPhi;
	SIRPhi*			psMerged;
	SIRPhiArgument*	psArgument;
	SIRType			sType;
	size			ui;
	size			uiArgument;

	pcHead = mcLoops.GetLoop(uiLoop)->pcHead;
	pcPreheader = mpcFunction->AddBlock();
	for (ui = 0; ui < pcHead->masPhis.NumElements(); ui++)
	{
		psPhi = pcHead->masPhis.Get(ui);
		sType = mpcFunction->GetTemporaryType(psPhi->sResult);
		psMerged = pcPreheader->AddPhi(sType, mpcFunction->AddTemporary(sType));
		for (uiArgument = psPhi->asArguments.NumElements(); uiArgument > 0; uiArgument--)
		{
			psArgument = psPhi->asArguments.Get(uiArgument - 1);
			if (!mcLoops.Contains(uiLoop, psArgument->pcBlock))
			{
				pcPreheader->AddPhiArgument(psMerged, psArgument->pcBlock, psArgument->sValue);
				psPhi->asArguments.RemoveAt(uiArgument - 1, true);
			}
		}
		pcHead->AddPhiArgument(psPhi, pcPreheader, psMerged->sResult);
	}
	for (ui = 0; ui < pcHead->mapcPredecessors.NumElements(); ui++)
	{
		pcPredecessor = pcHead->mapcPredecessors.GetPtr(ui);
		if (!mcLoops.Contains(uiLoop, pcPredecessor))
		{
			RedirectJump(pcPredecessor, pcHead, pcPreheader);
		}
	}
	pcPreheader->Jump(pcHead);
	mpcFunction->ComputePredecessors();
	mcLoops.SetPreheader(uiLoop, pcPreheader);
	return pcPreheader;
}


//////////////////////////////////////////////////////////////////////////
//
// True if sStepped is sValue + 1.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::StepsByOne(SIRReference sStepped, SIRReference sValue)
{
	SIRInstruction*	psStep;

	psStep = mcDefinitions.Instruction(sStepped);
	return psStep && (psStep->uiOpcode == IRO_Add) && psStep->asArguments[0].Equals(sValue) && psStep->asArguments[1].IsConstant() &&
		(mpcFunction->GetSigned(psStep->asArguments[1]) == 1);
}


//////////////////////////////////////////////////////////////////////////
//
// True if loop uiLoop is only entered when sInitial uiOpcode sBound (ult or
// slt) holds: its head, or a block up IR_GUARD_DEPTH immediate dominators
// from it, is reached (but from inside the loop) only from a branch on that
// compare, the way it holds.
//
//////////////////////////////////////////////////////////////////////////
//
// LoopGuarded of a compare in the form it looks for (a > b as b < a).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowGuardedAs(size uiLoop, uint16 uiOpcode, SIRReference sA, SIRReference sB)
{
	if (IsIRSwappedComparison(uiOpcode))
	{
		return LoopGuarded(uiLoop, IRSwappedComparison(uiOpcode), sB, sA);
	}
	return LoopGuarded(uiLoop, uiOpcode, sA, sB);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the stepped counter sStepped is used only by the counter's phi,
// the compare, and phis after the loop on the way out of pcBlock (the
// block that tests it), which are added to papsExits.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SteppedExitUses(size uiLoop, CIRBlock* pcBlock, SIRPhi* psCounter, SIRInstruction* psCompare, SIRReference sStepped, CArrayIRExitArguments* papsExits)
{
	CIRBlock*		pcUser;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	size			ui;
	size			uiIndex;
	size			uiArgument;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcUser = mpcFunction->GetBlock(ui);
		for (uiIndex = 0; uiIndex < pcUser->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcUser->masInstructions.Get(uiIndex);
			if ((psInstruction != psCompare) && (psInstruction->asArguments[0].Equals(sStepped) || psInstruction->asArguments[1].Equals(sStepped)))
			{
				return false;
			}
		}
		if ((pcUser != pcBlock) && pcUser->msJump.sArgument.Equals(sStepped))
		{
			return false;
		}
		for (uiIndex = 0; uiIndex < pcUser->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcUser->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				psArgument = psPhi->asArguments.Get(uiArgument);
				if (!psArgument->sValue.Equals(sStepped) || (psPhi == psCounter))
				{
					continue;
				}
				if (mcLoops.Contains(uiLoop, pcUser) || (psArgument->pcBlock != pcBlock))
				{
					return false;
				}
				papsExits->Add(psArgument);
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 32 bit loop counters that stay in 16 bits count in 16 bits
// (NarrowCounter, for each block of each loop that leaves or goes round on
// a compare).
//
//////////////////////////////////////////////////////////////////////////
//
// Whether the loop is only entered when sInitial uiOpcode sBound
// (LoopGuarded), or the same of the narrower values they extend (the
// guard compares those once FoldConstants narrows it; a constant is the
// same constant narrow).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowLoopGuarded(size uiLoop, uint16 uiOpcode, SIRReference sInitial, SIRReference sBound)
{
	SIRInstruction*	psInitial;
	SIRInstruction*	psBound;
	SIRType			sNarrow;
	SIRReference	sNarrowInitial;
	SIRReference	sNarrowBound;

	if (NarrowGuardedAs(uiLoop, uiOpcode, sInitial, sBound))
	{
		return true;
	}
	psInitial = sInitial.IsTemporary() ? mcDefinitions.Instruction(sInitial) : NULL;
	psBound = sBound.IsTemporary() ? mcDefinitions.Instruction(sBound) : NULL;
	if (psInitial && (psInitial->uiOpcode == IRO_SExt))
	{
		sNarrow = mpcFunction->TypeOf(psInitial->asArguments[0]);
	}
	else if (psBound && (psBound->uiOpcode == IRO_SExt))
	{
		sNarrow = mpcFunction->TypeOf(psBound->asArguments[0]);
	}
	else
	{
		return false;
	}

	if (psInitial && (psInitial->uiOpcode == IRO_SExt) && mpcFunction->TypeOf(psInitial->asArguments[0]).Equals(sNarrow))
	{
		sNarrowInitial = psInitial->asArguments[0];
	}
	else if (sInitial.IsConstant())
	{
		sNarrowInitial = mpcFunction->AddInteger(sNarrow, mpcFunction->GetSigned(sInitial));
	}
	else
	{
		return false;
	}
	if (psBound && (psBound->uiOpcode == IRO_SExt) && mpcFunction->TypeOf(psBound->asArguments[0]).Equals(sNarrow))
	{
		sNarrowBound = psBound->asArguments[0];
	}
	else if (sBound.IsConstant())
	{
		sNarrowBound = mpcFunction->AddInteger(sNarrow, mpcFunction->GetSigned(sBound));
	}
	else
	{
		return false;
	}
	return NarrowGuardedAs(uiLoop, uiOpcode, sNarrowInitial, sNarrowBound);
}


//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowCounters(void)
{
	size		uiLoop;
	size		uiBlock;
	bool		bChanged;

	ComputeLoops();
	bChanged = false;
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		for (uiBlock = 0; uiBlock < mcLoops.NumBlocks(uiLoop); uiBlock++)
		{
			if (NarrowCounter(uiLoop, mcLoops.GetBlock(uiLoop, uiBlock)))
			{
				//The loops and definitions are found again for the next.
				ComputeLoops();
				bChanged = true;
				break;
			}
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// pcBlock (in the loop) branches round the loop or out of it on a signed
// compare of a counter c (a 32 bit phi of the loop's head from c0 outside
// and c + 1 or c - 1 from inside, or that c + 1 or c - 1) with a bound N
// the loop doesn't change, going round while c moves towards N (up while
// < or <=, down while > or >=).  When c0 and N are known to fit 16 bits
// (SignedRange), with one to spare past c0 in the way it goes (unless the
// loop is only entered while c0 is towards N: NarrowLoopGuarded) and past
// N for <= and >=, c can't leave 16 bits: it stops at N (one past it for
// <= and >=), or one step past c0 when it starts past N.  So it counts in
// a 16 bit phi; the compare is of 16 bits, and other uses get it sign
// extended.  The 32 bit phi and step go (RemoveDeadCode).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowCounter(size uiLoop, CIRBlock* pcBlock)
{
	CIRBlock*		pcHead;
	CIRBlock*		pcOutside;
	CIRBlock*		pcInside;
	CIRBlock*		pcStepBlock;
	SIRInstruction*	psCompare;
	SIRInstruction*	psStep;
	SIRPhi*			psCounter;
	SIRPhi*			psNarrow;
	SIRReference	sCompared;
	SIRReference	sBound;
	SIRReference	sCounter;
	SIRReference	sStepped;
	SIRReference	sInitial;
	SIRReference	sNarrowCounter;
	SIRReference	sNarrowStepped;
	SIRReference	sNarrowInitial;
	SIRReference	sNarrowBound;
	SIRReference	sWideCounter;
	SIRReference	sWideStepped;
	SIRType			sWide;
	SIRType			sNarrowType;
	uint16			uiOpcode;
	size			uiPhi;
	size			uiOutside;
	size			uiStep;
	int64			lliStep;
	int64			lliMin;
	int64			lliMax;
	int64			lliBoundMin;
	int64			lliBoundMax;
	bool			bRoundOnTrue;
	bool			bUp;
	bool			bBoundFirst;
	bool			bInclusive;
	bool			bStartSpare;
	bool			bRewritten;
	CArrayIRExitArguments	asExits;		//The phi arguments after the loop that take c +/- 1 leaving through the test.
	SIRReference	sExitValue;
	size			uiExit;
	SIRReference	sCounterBound;
	SIRInstruction*	psBound;

	if ((pcBlock->msJump.uiKind != IRJ_Branch) || !pcBlock->msJump.sArgument.IsTemporary() || (mcDefinitions.NumUses(pcBlock->msJump.sArgument) != 1))
	{
		return false;
	}
	psCompare = mcDefinitions.Instruction(pcBlock->msJump.sArgument);
	if ((psCompare == NULL) || (mcDefinitions.Block(psCompare->sResult) != pcBlock) || !psCompare->sType.Is(IRTK_I32))
	{
		return false;
	}
	uiOpcode = psCompare->uiOpcode;
	if ((uiOpcode != IRO_SLt) && (uiOpcode != IRO_SLe) && (uiOpcode != IRO_SGt) && (uiOpcode != IRO_SGe))
	{
		return false;
	}
	bRoundOnTrue = mcLoops.Contains(uiLoop, pcBlock->msJump.pcThen);
	if (bRoundOnTrue == mcLoops.Contains(uiLoop, pcBlock->msJump.pcElse))
	{
		return false;
	}

	//Which operand is the counter: the bound is the one the loop doesn't change.
	bBoundFirst = mcLoops.IsInvariant(uiLoop, psCompare->asArguments[0], &mcDefinitions);
	sCompared = psCompare->asArguments[bBoundFirst ? 1 : 0];
	sBound = psCompare->asArguments[bBoundFirst ? 0 : 1];
	if (!sCompared.IsTemporary() || !mcLoops.IsInvariant(uiLoop, sBound, &mcDefinitions) || mcLoops.IsInvariant(uiLoop, sCompared, &mcDefinitions))
	{
		return false;
	}

	//c: a phi of the head with c0 from outside and c +/- 1 from inside (sStepped); the compare is of c or c +/- 1.
	pcHead = mcLoops.GetBlock(uiLoop, 0);
	psCounter = NULL;
	psStep = NULL;
	uiOutside = 0;
	for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
	{
		psCounter = pcHead->masPhis.Get(uiPhi);
		if (!psCounter->sType.Is(IRTK_I32) || (psCounter->asArguments.NumElements() != 2))
		{
			psCounter = NULL;
			continue;
		}
		uiOutside = mcLoops.Contains(uiLoop, psCounter->asArguments.Get(0)->pcBlock) ? 1 : 0;
		sStepped = psCounter->asArguments.Get(1 - uiOutside)->sValue;
		psStep = sStepped.IsTemporary() ? mcDefinitions.Instruction(sStepped) : NULL;
		if (!mcLoops.Contains(uiLoop, psCounter->asArguments.Get(1 - uiOutside)->pcBlock) || mcLoops.Contains(uiLoop, psCounter->asArguments.Get(uiOutside)->pcBlock) ||
			(psStep == NULL) || ((psStep->uiOpcode != IRO_Add) && (psStep->uiOpcode != IRO_Sub)) || !psStep->asArguments[0].Equals(psCounter->sResult) ||
			!psStep->asArguments[1].IsConstant() || (!sCompared.Equals(psCounter->sResult) && !sCompared.Equals(sStepped)))
		{
			psCounter = NULL;
			continue;
		}
		break;
	}
	if (psCounter == NULL)
	{
		return false;
	}
	sCounter = psCounter->sResult;
	lliStep = mpcFunction->GetSigned(psStep->asArguments[1]);
	if (psStep->uiOpcode == IRO_Sub)
	{
		lliStep = -lliStep;
	}
	if ((lliStep != 1) && (lliStep != -1))
	{
		return false;
	}

	//Going round while it moves towards the bound: up while c < N or c <= N, down while c > N or c >= N.
	if (!bRoundOnTrue)
	{
		uiOpcode = (uiOpcode == IRO_SLt) ? IRO_SGe : ((uiOpcode == IRO_SLe) ? IRO_SGt : ((uiOpcode == IRO_SGt) ? IRO_SLe : IRO_SLt));
	}
	if (bBoundFirst)
	{
		uiOpcode = (uiOpcode == IRO_SLt) ? IRO_SGt : ((uiOpcode == IRO_SLe) ? IRO_SGe : ((uiOpcode == IRO_SGt) ? IRO_SLt : IRO_SLe));
	}
	bUp = (uiOpcode == IRO_SLt) || (uiOpcode == IRO_SLe);
	if (bUp != (lliStep == 1))
	{
		return false;
	}

	//A test of c +/- 1 that the phi only otherwise uses is a test of c, so the last step (past N) is never seen:
	//c + 1 <= N is c < N and c + 1 < N + 1 is c < N (down, c - 1 >= N is c > N and c - 1 > N - 1 is c > N).  Leaving
	//through the test, c +/- 1 is N +/- 1 when the loop is only entered with c0 <= N (or >=), so a phi after the loop
	//that takes it on that way out takes N +/- 1 worked out before the loop instead.
	bRewritten = false;
	asExits.Init();
	if (!sCompared.Equals(sCounter) && SteppedExitUses(uiLoop, pcBlock, psCounter, psCompare, sStepped, &asExits))
	{
		sCounterBound = IRNone();
		if ((uiOpcode == IRO_SLe) || (uiOpcode == IRO_SGe))
		{
			sCounterBound = sBound;
		}
		else
		{
			psBound = sBound.IsTemporary() ? mcDefinitions.Instruction(sBound) : NULL;
			if (psBound && psBound->asArguments[1].IsConstant() &&
				(((psBound->uiOpcode == IRO_Add) && (mpcFunction->GetSigned(psBound->asArguments[1]) == lliStep)) ||
				 ((psBound->uiOpcode == IRO_Sub) && (mpcFunction->GetSigned(psBound->asArguments[1]) == -lliStep))))
			{
				sCounterBound = psBound->asArguments[0];
			}
		}
		if (!sCounterBound.IsNone() && ((asExits.NumElements() == 0) || NarrowLoopGuarded(uiLoop, bUp ? IRO_SLe : IRO_SGe, psCounter->asArguments.Get(uiOutside)->sValue, sCounterBound)))
		{
			sBound = sCounterBound;
			sCompared = sCounter;
			uiOpcode = bUp ? IRO_SLt : IRO_SGt;
			bRewritten = true;
		}
	}
	if (!bRewritten)
	{
		asExits.Kill();
		asExits.Init();
	}

	//c0 and N fit 16 bits.  The values seen stay between them, with one to spare for a step past one: past N for <=
	//and >= (< and > stop at N), and past c0 (when it starts past N) in the way it goes, when the stepped value is seen
	//past the test (tested, or used after it) and the loop isn't only entered towards N.
	sInitial = psCounter->asArguments.Get(uiOutside)->sValue;
	if (!SignedRange(sInitial, &lliMin, &lliMax) || !SignedRange(sBound, &lliBoundMin, &lliBoundMax))
	{
		return false;
	}
	bInclusive = (uiOpcode == IRO_SLe) || (uiOpcode == IRO_SGe);
	bStartSpare = (!sCompared.Equals(sCounter) || (mcDefinitions.NumUses(sStepped) > (bRewritten ? 2u + asExits.NumElements() : 1u))) && !NarrowLoopGuarded(uiLoop, uiOpcode, sInitial, sBound);
	if ((lliMin < -32768) || (lliMax > 32767) || (lliBoundMin < -32768) || (lliBoundMax > 32767) ||
		(bUp && ((bStartSpare && (lliMax > 32766)) || (bInclusive && (lliBoundMax > 32766)))) ||
		(!bUp && ((bStartSpare && (lliMin < -32767)) || (bInclusive && (lliBoundMin < -32767)))))
	{
		asExits.Kill();
		return false;
	}

	//The 16 bit counter: c0 and N narrowed outside, c' +/- 1 after the step.
	sWide = IRType(IRTK_I32);
	sNarrowType = IRType(IRTK_I16);
	pcOutside = psCounter->asArguments.Get(uiOutside)->pcBlock;
	sNarrowInitial = sInitial.IsConstant() ? mpcFunction->AddInteger(sNarrowType, mpcFunction->GetSigned(sInitial)) : mpcFunction->Add(pcOutside, IRO_Trunc, sNarrowType, sInitial, IRNone());
	sNarrowBound = sBound.IsConstant() ? mpcFunction->AddInteger(sNarrowType, mpcFunction->GetSigned(sBound)) : mpcFunction->Add(pcOutside, IRO_Trunc, sNarrowType, sBound, IRNone());
	if (asExits.NumElements() > 0)
	{
		//N +/- 1, what c +/- 1 is leaving through the test.
		sExitValue = mpcFunction->Add(pcOutside, IRO_Add, sWide, sBound, mpcFunction->AddInteger(sWide, lliStep));
		for (uiExit = 0; uiExit < asExits.NumElements(); uiExit++)
		{
			asExits.GetPtr(uiExit)->sValue = sExitValue;
		}
	}
	asExits.Kill();
	sNarrowCounter = mpcFunction->AddTemporary(sNarrowType);
	//AddPhi may move the head's phis, psCounter too.
	pcInside = psCounter->asArguments.Get(1 - uiOutside)->pcBlock;
	psCounter = NULL;
	psNarrow = pcHead->AddPhi(sNarrowType, sNarrowCounter);
	pcStepBlock = mcDefinitions.Block(sStepped);
	for (uiStep = 0; (uiStep < pcStepBlock->masInstructions.NumElements()) && !pcStepBlock->masInstructions.Get(uiStep)->sResult.Equals(sStepped); uiStep++)
	{
	}
	sNarrowStepped = InsertInstruction(pcStepBlock, uiStep + 1, IRO_Add, sNarrowType, sNarrowCounter, mpcFunction->AddInteger(sNarrowType, lliStep));
	sWideStepped = InsertInstruction(pcStepBlock, uiStep + 2, IRO_SExt, sWide, sNarrowStepped, IRNone());
	pcHead->AddPhiArgument(psNarrow, pcOutside, sNarrowInitial);
	pcHead->AddPhiArgument(psNarrow, pcInside, sNarrowStepped);
	sWideCounter = InsertInstruction(pcHead, 0, IRO_SExt, sWide, sNarrowCounter, IRNone());

	//The compare is of the 16 bit values.
	FindDefinitions();
	psCompare = mcDefinitions.Instruction(pcBlock->msJump.sArgument);
	psCompare->sType = sNarrowType;
	if (bRewritten)
	{
		//c < N (or c > N) going round, its inverse leaving.
		psCompare->uiOpcode = bRoundOnTrue ? uiOpcode : IRInvertedComparison(uiOpcode);
		psCompare->asArguments[0] = sNarrowCounter;
		psCompare->asArguments[1] = sNarrowBound;
	}
	else
	{
		psCompare->asArguments[bBoundFirst ? 0 : 1] = sNarrowBound;
		psCompare->asArguments[bBoundFirst ? 1 : 0] = sCompared.Equals(sCounter) ? sNarrowCounter : sNarrowStepped;
	}

	//Everything else that used c or c +/- 1 has them sign extended.
	ClearReplacements();
	Replace(sCounter, sWideCounter);
	Replace(sStepped, sWideStepped);
	ApplyReplacements();
	FindDefinitions();
	return true;
}


//////////////////////////////////////////////////////////////////////////
#define IR_GUARD_DEPTH	16
bool CIROptimiser::LoopGuarded(size uiLoop, uint16 uiOpcode, SIRReference sInitial, SIRReference sBound)
{
	CIRBlock*		pcBlock;
	CIRBlock*		pcFrom;
	CIRBlock*		pcPredecessor;
	SIRInstruction*	psCompare;
	uint16			uiCompare;
	SIRReference	sA;
	SIRReference	sB;
	size			ui;
	int				iDepth;

	pcBlock = mcLoops.GetBlock(uiLoop, 0);
	for (iDepth = 0; pcBlock && (iDepth < IR_GUARD_DEPTH); iDepth++)
	{
		pcFrom = NULL;
		for (ui = 0; ui < pcBlock->mapcPredecessors.NumElements(); ui++)
		{
			pcPredecessor = pcBlock->mapcPredecessors.GetPtr(ui);
			if (!mcLoops.Contains(uiLoop, pcPredecessor))
			{
				pcFrom = (pcFrom == NULL) ? pcPredecessor : NULL;
				if (pcFrom == NULL)
				{
					break;
				}
			}
		}
		psCompare = (pcFrom && (pcFrom->msJump.uiKind == IRJ_Branch) && (pcFrom->msJump.pcThen != pcFrom->msJump.pcElse)) ? mcDefinitions.Instruction(pcFrom->msJump.sArgument) : NULL;
		if (psCompare && IsIRIntegerComparison(psCompare->uiOpcode))
		{
			uiCompare = (pcFrom->msJump.pcThen == pcBlock) ? psCompare->uiOpcode : IRInvertedComparison(psCompare->uiOpcode);
			sA = psCompare->asArguments[0];
			sB = psCompare->asArguments[1];
			if (IsIRSwappedComparison(uiCompare))
			{
				uiCompare = IRSwappedComparison(uiCompare);
				sA = psCompare->asArguments[1];
				sB = psCompare->asArguments[0];
			}
			if ((uiCompare == uiOpcode) && sA.Equals(sInitial) && sB.Equals(sBound))
			{
				return true;
			}
		}
		pcBlock = mcDominators.GetImmediate(pcBlock);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Linear test replacement: a loop with two counters going up by 1
// together, c from a constant c0 and v from v0 (both phis of its head with
// one way in and one way round), that only uses c for its test c + 1 < N
// (ult or slt, N not changed by the loop) tests v + 1 != v0 - c0 + N
// instead, and c goes (RemoveDeadCode).  Each time round c + 1 <= N, as
// the loop is only entered when c0 < N (LoopGuarded) and leaves when
// c + 1 reaches N, so < is !=; and v - c is v0 - c0 throughout.  The new
// bound is moved out of the loop by HoistInvariants.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceLoopCounters(void)
{
	size			uiLoop;
	size			uiBlock;
	size			uiPhi;
	size			uiOutside;
	CIRBlock*		pcBlock;
	CIRBlock*		pcHead;
	SIRInstruction*	psCompare;
	SIRInstruction*	psNew;
	SIRPhi*			psCounter;
	SIRPhi*			psOther;
	SIRReference	sCounter;
	SIRReference	sStepped;
	SIRReference	sBound;
	SIRReference	sOtherStepped;
	SIRReference	sOtherInitial;
	SIRReference	sCounterInitial;
	SIRReference	sDifference;
	SIRReference	sNewBound;
	SIRReference	sTest;
	uint16			uiOpcode;
	bool			bRoundOnTrue;
	bool			bChanged;

	ComputeLoops();
	bChanged = false;
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		pcHead = mcLoops.GetBlock(uiLoop, 0);
		if (pcHead->mapcPredecessors.NumElements() != 2)
		{
			continue;
		}
		for (uiBlock = 0; uiBlock < mcLoops.NumBlocks(uiLoop); uiBlock++)
		{
			pcBlock = mcLoops.GetBlock(uiLoop, uiBlock);
			if ((pcBlock->msJump.uiKind != IRJ_Branch) || !pcBlock->msJump.sArgument.IsTemporary() || (mcDefinitions.NumUses(pcBlock->msJump.sArgument) != 1))
			{
				continue;
			}
			psCompare = mcDefinitions.Instruction(pcBlock->msJump.sArgument);
			if ((psCompare == NULL) || ((psCompare->uiOpcode != IRO_ULt) && (psCompare->uiOpcode != IRO_SLt)) || (mcDefinitions.Block(psCompare->sResult) != pcBlock))
			{
				continue;
			}
			bRoundOnTrue = mcLoops.Contains(uiLoop, pcBlock->msJump.pcThen) && !mcLoops.Contains(uiLoop, pcBlock->msJump.pcElse);
			if (!bRoundOnTrue)
			{
				continue;
			}
			uiOpcode = psCompare->uiOpcode;
			sStepped = psCompare->asArguments[0];
			sBound = psCompare->asArguments[1];
			if (!sStepped.IsTemporary() || !mcLoops.IsInvariant(uiLoop, sBound, &mcDefinitions) || (mcDefinitions.NumUses(sStepped) != 2))
			{
				continue;
			}

			//c: a phi of the head, c0 a constant from outside and c + 1 from inside; c's only use is c + 1.
			psCounter = NULL;
			for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
			{
				sCounter = pcHead->masPhis.Get(uiPhi)->sResult;
				if (StepsByOne(sStepped, sCounter))
				{
					psCounter = pcHead->masPhis.Get(uiPhi);
					break;
				}
			}
			if ((psCounter == NULL) || (psCounter->asArguments.NumElements() != 2) || (mcDefinitions.NumUses(sCounter) != 1))
			{
				continue;
			}
			uiOutside = mcLoops.Contains(uiLoop, psCounter->asArguments.Get(0)->pcBlock) ? 1 : 0;
			sCounterInitial = psCounter->asArguments.Get(uiOutside)->sValue;
			if (mcLoops.Contains(uiLoop, psCounter->asArguments.Get(uiOutside)->pcBlock) || !psCounter->asArguments.Get(1 - uiOutside)->sValue.Equals(sStepped) ||
				!sCounterInitial.IsConstant() || !LoopGuarded(uiLoop, uiOpcode, sCounterInitial, sBound))
			{
				continue;
			}

			//v: another phi of the head of the same type, v + 1 worked out in this block going round.
			psOther = NULL;
			for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
			{
				psOther = pcHead->masPhis.Get(uiPhi);
				if ((psOther != psCounter) && psOther->sType.Equals(psCounter->sType) && (psOther->asArguments.NumElements() == 2) &&
					!mcLoops.Contains(uiLoop, psOther->asArguments.Get(uiOutside)->pcBlock) &&
					(psOther->asArguments.Get(uiOutside)->pcBlock == psCounter->asArguments.Get(uiOutside)->pcBlock))
				{
					sOtherStepped = psOther->asArguments.Get(1 - uiOutside)->sValue;
					if (StepsByOne(sOtherStepped, psOther->sResult) && (mcDefinitions.Block(sOtherStepped) == pcBlock))
					{
						break;
					}
				}
				psOther = NULL;
			}
			if (psOther == NULL)
			{
				continue;
			}
			sOtherInitial = psOther->asArguments.Get(uiOutside)->sValue;

			//v + 1 != v0 + (N - c0).
			sDifference = mpcFunction->AddTemporary(psCounter->sType);
			sNewBound = mpcFunction->AddTemporary(psCounter->sType);
			sTest = mpcFunction->AddTemporary(IRType(IRTK_I1));
			psNew = pcBlock->masInstructions.Add();
			memset(psNew, 0, sizeof(SIRInstruction));
			psNew->uiOpcode = IRO_Sub;
			psNew->sType = psCounter->sType;
			psNew->sResult = sDifference;
			psNew->asArguments[0] = sBound;
			psNew->asArguments[1] = sCounterInitial;
			psNew = pcBlock->masInstructions.Add();
			memset(psNew, 0, sizeof(SIRInstruction));
			psNew->uiOpcode = IRO_Add;
			psNew->sType = psCounter->sType;
			psNew->sResult = sNewBound;
			psNew->asArguments[0] = sOtherInitial;
			psNew->asArguments[1] = sDifference;
			psNew = pcBlock->masInstructions.Add();
			memset(psNew, 0, sizeof(SIRInstruction));
			psNew->uiOpcode = IRO_Ne;
			psNew->sType = psCounter->sType;
			psNew->sResult = sTest;
			psNew->asArguments[0] = sOtherStepped;
			psNew->asArguments[1] = sNewBound;
			pcBlock->msJump.sArgument = sTest;
			bChanged = true;
			FindDefinitions();
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// True if an instruction of pcBlock from uiStart up to uiEnd may write the
// iBytes at sRoot + lliOffset: a store that may alias them (MayAlias) or
// anything else with an effect.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::WritesBetween(CIRBlock* pcBlock, size uiStart, size uiEnd, SIRReference sRoot, int64 lliOffset, int iBytes)
{
	size			ui;
	SIRInstruction*	psInstruction;
	SIRAvailable	sStore;

	for (ui = uiStart; ui < uiEnd; ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		if (psInstruction->uiOpcode == IRO_Store)
		{
			SplitAddress(psInstruction->asArguments[1], &sStore.sRoot, &sStore.lliOffset);
			sStore.sType = psInstruction->sType;
			sStore.sValue = IRNone();
			if (MayAlias(&sStore, sRoot, lliOffset, iBytes))
			{
				return true;
			}
		}
		else if (HasEffect(psInstruction))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the load at uiLoad in pcFrom to just before its only use, at uiUse
// in pcTo, if nothing between can write what it reads: the rest of pcFrom,
// the blocks on the ways from pcFrom to pcTo, and pcTo up to the use.
// Every loop pcTo is in must have pcFrom in it, or the use could be done
// again without the load (and the load isn't done more often).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SinkLoad(CIRBlock* pcFrom, size uiLoad, CIRBlock* pcTo, size uiUse)
{
	SIRInstruction			sLoad;
	SIRReference			sRoot;
	int64					lliOffset;
	int						iBytes;
	CArrayTemplate<uint8>	abForward;
	CArrayTemplate<uint8>	abBackward;
	CArrayIRBlocks			apcWork;
	CIRBlock*				pcBlock;
	CIRBlock*				pcNext;
	size					ui;
	size					uiLoop;
	bool					bWrites;

	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		if (mcLoops.Contains(uiLoop, pcTo) && !mcLoops.Contains(uiLoop, pcFrom))
		{
			return false;
		}
	}
	sLoad = *pcFrom->masInstructions.Get(uiLoad);
	SplitAddress(sLoad.asArguments[0], &sRoot, &lliOffset);
	iBytes = mpcModule->GetSize(sLoad.sType);
	if (WritesBetween(pcFrom, uiLoad + 1, pcFrom->masInstructions.NumElements(), sRoot, lliOffset, iBytes) ||
		WritesBetween(pcTo, 0, uiUse, sRoot, lliOffset, iBytes))
	{
		return false;
	}

	//The blocks reached from pcFrom (not going on from pcTo) that reach pcTo (not going back past pcFrom).
	abForward.Init();
	abBackward.Init();
	abForward.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0);
	abBackward.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0);
	apcWork.Init();
	apcWork.Add(pcFrom);
	while (apcWork.NumElements() > 0)
	{
		pcBlock = apcWork.Pop();
		for (ui = 0; ui < pcBlock->NumSuccessors(); ui++)
		{
			pcNext = pcBlock->GetSuccessor(ui);
			if (!*abForward.Get(pcNext->muiIndex))
			{
				*abForward.Get(pcNext->muiIndex) = true;
				if ((pcNext != pcTo) && (pcNext != pcFrom))
				{
					apcWork.Add(pcNext);
				}
			}
		}
	}
	apcWork.Add(pcTo);
	while (apcWork.NumElements() > 0)
	{
		pcBlock = apcWork.Pop();
		for (ui = 0; ui < pcBlock->mapcPredecessors.NumElements(); ui++)
		{
			pcNext = pcBlock->mapcPredecessors.GetPtr(ui);
			if (!*abBackward.Get(pcNext->muiIndex))
			{
				*abBackward.Get(pcNext->muiIndex) = true;
				if ((pcNext != pcFrom) && (pcNext != pcTo))
				{
					apcWork.Add(pcNext);
				}
			}
		}
	}
	bWrites = false;
	for (ui = 0; !bWrites && (ui < mpcFunction->NumBlocks()); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if ((pcBlock != pcFrom) && (pcBlock != pcTo) && *abForward.Get(ui) && *abBackward.Get(ui))
		{
			bWrites = WritesBetween(pcBlock, 0, pcBlock->masInstructions.NumElements(), sRoot, lliOffset, iBytes);
		}
	}
	apcWork.Kill();
	abBackward.Kill();
	abForward.Kill();
	if (bWrites)
	{
		return false;
	}

	pcFrom->masInstructions.RemoveAt(uiLoad, true);
	pcTo->masInstructions.InsertAt(&sLoad, uiUse);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The loop's one way out (from a block in it to one outside), if it has
// only one and is entered from one block.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SingleExit(size uiLoop, CIRBlock** ppcFrom, CIRBlock** ppcTo)
{
	CArrayIRLoopExits	asExits;
	bool				bSingle;

	if (mcLoops.GetLoop(uiLoop)->pcEntering == NULL)
	{
		return false;
	}
	asExits.Init();
	mcLoops.GetExits(uiLoop, &asExits);
	bSingle = asExits.NumElements() == 1;
	if (bSingle)
	{
		*ppcFrom = asExits.Get(0)->pcFrom;
		*ppcTo = asExits.Get(0)->pcTo;
	}
	asExits.Kill();
	return bSingle;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a block outside the loop reads sValue (an instruction, a phi or
// a jump).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::UsedOutside(size uiLoop, SIRReference sValue)
{
	size			ui;
	size			uiIndex;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if (mcLoops.Contains(uiLoop, pcBlock))
		{
			continue;
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			if (psInstruction->asArguments[0].Equals(sValue) || psInstruction->asArguments[1].Equals(sValue))
			{
				return true;
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				if (psPhi->asArguments.Get(uiArgument)->sValue.Equals(sValue))
				{
					return true;
				}
			}
		}
		if (pcBlock->msJump.sArgument.Equals(sValue))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks outside the loop read sNew where they read sValue.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::ReplaceOutside(size uiLoop, SIRReference sValue, SIRReference sNew)
{
	size			ui;
	size			uiIndex;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if (mcLoops.Contains(uiLoop, pcBlock))
		{
			continue;
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				if (psInstruction->asArguments[uiArgument].Equals(sValue))
				{
					psInstruction->asArguments[uiArgument] = sNew;
				}
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				psArgument = psPhi->asArguments.Get(uiArgument);
				if (psArgument->sValue.Equals(sValue))
				{
					psArgument->sValue = sNew;
				}
			}
		}
		if (pcBlock->msJump.sArgument.Equals(sValue))
		{
			pcBlock->msJump.sArgument = sNew;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Final values: a loop with one way out, left when v + 1 != v0 + d is
// false (v a phi of its head going up by 1 from v0, d not changed by it,
// and the loop only entered when 0 < d, LoopGuarded) goes round d times
// (d < 2^n, as d is not 0).  After it v + 1 is v0 + d, and a value w
// stepped beside v (a phi of the head from w0, its value going round w + t
// or ptradd w, t, t a constant) is w0 + d * t: those are worked out in the
// loop's preheader and blocks outside read them instead.  The loop may
// then be dead (DeleteDeadLoops).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceExitValues(void)
{
	size			uiLoop;
	size			uiPhi;
	size			uiOutside;
	CIRBlock*		pcFrom;
	CIRBlock*		pcTo;
	CIRBlock*		pcHead;
	CIRBlock*		pcPreheader;
	SIRInstruction*	psTest;
	SIRInstruction*	psBound;
	SIRInstruction*	psStep;
	SIRPhi*			psCounter;
	SIRPhi*			psPhi;
	SIRReference	sStepped;
	SIRReference	sBound;
	SIRReference	sInitial;
	SIRReference	sCount;
	SIRReference	sOffset;
	SIRReference	sNew;
	SIRReference	sValue;
	SIRType			sType;
	int64			lliStep;
	bool			bRoundOnTrue;
	bool			bChanged;

	ComputeLoops();
	bChanged = false;
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		if (!SingleExit(uiLoop, &pcFrom, &pcTo) || (pcFrom->msJump.uiKind != IRJ_Branch) || !pcFrom->msJump.sArgument.IsTemporary())
		{
			continue;
		}
		pcHead = mcLoops.GetLoop(uiLoop)->pcHead;
		psTest = mcDefinitions.Instruction(pcFrom->msJump.sArgument);
		bRoundOnTrue = pcFrom->msJump.pcElse == pcTo;
		if ((psTest == NULL) || ((psTest->uiOpcode != (bRoundOnTrue ? IRO_Ne : IRO_Eq))))
		{
			continue;
		}

		//v + 1 against v0 + d.
		sStepped = psTest->asArguments[0];
		sBound = psTest->asArguments[1];
		psCounter = NULL;
		for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
		{
			if (StepsByOne(sStepped, pcHead->masPhis.Get(uiPhi)->sResult))
			{
				psCounter = pcHead->masPhis.Get(uiPhi);
				break;
			}
		}
		if ((psCounter == NULL) || (psCounter->asArguments.NumElements() != 2) || !mcLoops.IsInvariant(uiLoop, sBound, &mcDefinitions))
		{
			continue;
		}
		uiOutside = mcLoops.Contains(uiLoop, psCounter->asArguments.Get(0)->pcBlock) ? 1 : 0;
		if (mcLoops.Contains(uiLoop, psCounter->asArguments.Get(uiOutside)->pcBlock) || !psCounter->asArguments.Get(1 - uiOutside)->sValue.Equals(sStepped))
		{
			continue;
		}
		sInitial = psCounter->asArguments.Get(uiOutside)->sValue;
		psBound = mcDefinitions.Instruction(sBound);
		if ((psBound == NULL) || (psBound->uiOpcode != IRO_Add) || !(psBound->asArguments[0].Equals(sInitial) || psBound->asArguments[1].Equals(sInitial)))
		{
			continue;
		}
		sCount = psBound->asArguments[0].Equals(sInitial) ? psBound->asArguments[1] : psBound->asArguments[0];
		if (!(sCount.IsConstant() ? (mpcFunction->GetSigned(sCount) != 0) : LoopGuarded(uiLoop, IRO_ULt, mpcFunction->AddInteger(psCounter->sType, 0), sCount)))
		{
			continue;
		}

		//Something outside reads v + 1 or a value stepped beside it.
		if (!UsedOutside(uiLoop, sStepped))
		{
			for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
			{
				psPhi = pcHead->masPhis.Get(uiPhi);
				if ((psPhi->asArguments.NumElements() == 2) &&
					(UsedOutside(uiLoop, psPhi->asArguments.Get(0)->sValue) || UsedOutside(uiLoop, psPhi->asArguments.Get(1)->sValue)))
				{
					break;
				}
			}
			if (uiPhi == pcHead->masPhis.NumElements())
			{
				continue;
			}
		}

		pcPreheader = mcLoops.GetLoop(uiLoop)->pcPreheader;
		if (pcPreheader == NULL)
		{
			pcPreheader = AddPreheader(uiLoop);
		}
		if (UsedOutside(uiLoop, sStepped))
		{
			ReplaceOutside(uiLoop, sStepped, sBound);
			bChanged = true;
		}
		for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
		{
			psPhi = pcHead->masPhis.Get(uiPhi);
			if ((psPhi == psCounter) || (psPhi->asArguments.NumElements() != 2))
			{
				continue;
			}
			uiOutside = (psPhi->asArguments.Get(0)->pcBlock == pcPreheader) ? 0 : 1;
			if (psPhi->asArguments.Get(uiOutside)->pcBlock != pcPreheader)
			{
				continue;
			}
			sValue = psPhi->asArguments.Get(1 - uiOutside)->sValue;
			psStep = mcDefinitions.Instruction(sValue);
			if ((psStep == NULL) || !mcLoops.Contains(uiLoop, mcDefinitions.Block(sValue)) || !psStep->asArguments[0].Equals(psPhi->sResult) ||
				!psStep->asArguments[1].IsConstant() || !UsedOutside(uiLoop, sValue))
			{
				continue;
			}
			lliStep = mpcFunction->GetSigned(psStep->asArguments[1]);
			sType = psPhi->sType;
			if ((psStep->uiOpcode == IRO_PtrAdd) && sType.IsPointer())
			{
				//w0 + zext(d) * t, the offset an i32.
				sOffset = sCount;
				if (!mpcFunction->GetTemporaryType(sCount).Is(IRTK_I32) && !(sCount.IsConstant()))
				{
					sOffset = mpcFunction->Add(pcPreheader, IRO_ZExt, IRType(IRTK_I32), sCount);
				}
				else if (sCount.IsConstant())
				{
					sOffset = mpcFunction->AddInteger(IRType(IRTK_I32), (int64)IRTruncate(psCounter->sType, (uint64)mpcFunction->GetSigned(sCount)));
				}
				if (lliStep != 1)
				{
					sOffset = mpcFunction->Add(pcPreheader, IRO_Mul, IRType(IRTK_I32), sOffset, mpcFunction->AddInteger(IRType(IRTK_I32), lliStep));
				}
				sNew = mpcFunction->Add(pcPreheader, IRO_PtrAdd, sType, psPhi->asArguments.Get(uiOutside)->sValue, sOffset);
			}
			else if ((psStep->uiOpcode == IRO_Add) && sType.Equals(psCounter->sType))
			{
				sOffset = (lliStep == 1) ? sCount : mpcFunction->Add(pcPreheader, IRO_Mul, sType, sCount, mpcFunction->AddInteger(sType, lliStep));
				sNew = mpcFunction->Add(pcPreheader, IRO_Add, sType, psPhi->asArguments.Get(uiOutside)->sValue, sOffset);
			}
			else
			{
				continue;
			}
			ReplaceOutside(uiLoop, sValue, sNew);
			bChanged = true;
		}
		if (bChanged)
		{
			FindDefinitions();
			return true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A loop that does nothing (no store, call or volatile load) and whose
// values nothing outside reads goes: its preheader jumps to its one way out,
// whose phis take from the preheader what they took from the loop (values
// from before it).  C11 6.8.5 and C++ [intro.progress] let such a loop be
// assumed to end.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::DeleteDeadLoops(void)
{
	size			uiLoop;
	size			uiBlock;
	size			uiIndex;
	size			uiPhi;
	size			uiArgument;
	CIRBlock*		pcFrom;
	CIRBlock*		pcTo;
	CIRBlock*		pcBlock;
	CIRBlock*		pcPreheader;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	bool			bDead;

	ComputeLoops();
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		if (!SingleExit(uiLoop, &pcFrom, &pcTo))
		{
			continue;
		}
		bDead = true;
		for (uiBlock = 0; bDead && (uiBlock < mcLoops.NumBlocks(uiLoop)); uiBlock++)
		{
			pcBlock = mcLoops.GetBlock(uiLoop, uiBlock);
			for (uiIndex = 0; bDead && (uiIndex < pcBlock->masInstructions.NumElements()); uiIndex++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiIndex);
				bDead = !HasEffect(psInstruction) && !(psInstruction->sResult.IsTemporary() && UsedOutside(uiLoop, psInstruction->sResult));
			}
			for (uiPhi = 0; bDead && (uiPhi < pcBlock->masPhis.NumElements()); uiPhi++)
			{
				bDead = !UsedOutside(uiLoop, pcBlock->masPhis.Get(uiPhi)->sResult);
			}
		}
		if (!bDead)
		{
			continue;
		}

		pcPreheader = mcLoops.GetLoop(uiLoop)->pcPreheader;
		if (pcPreheader == NULL)
		{
			pcPreheader = AddPreheader(uiLoop);
		}
		RedirectJump(pcPreheader, mcLoops.GetLoop(uiLoop)->pcHead, pcTo);
		RenamePhiArguments(pcTo, pcFrom, pcPreheader);

		//The head's phis lose the way in (it is unreachable now, and goes with SimplifyCFG).
		for (uiPhi = 0; uiPhi < mcLoops.GetLoop(uiLoop)->pcHead->masPhis.NumElements(); uiPhi++)
		{
			psPhi = mcLoops.GetLoop(uiLoop)->pcHead->masPhis.Get(uiPhi);
			for (uiArgument = psPhi->asArguments.NumElements(); uiArgument > 0; uiArgument--)
			{
				psArgument = psPhi->asArguments.Get(uiArgument - 1);
				if (psArgument->pcBlock == pcPreheader)
				{
					psPhi->asArguments.RemoveAt(uiArgument - 1, true);
				}
			}
		}
		mpcFunction->ComputePredecessors();
		RemoveUnreachable();
		FindDefinitions();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The same instruction in both blocks a brif goes to (each reached only
// from it), reading values from before them, is worked out once at the end
// of the branching block (before the comparison it branches on), and the
// second block's uses read the first's.  Only what can't trap and doesn't
// read memory (IROptimiserHoistable, but loads).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::HoistCommon(void)
{
	CIRBlock*		pcBlock;
	CIRBlock*		pcThen;
	CIRBlock*		pcElse;
	SIRInstruction*	psFirst;
	SIRInstruction*	psSecond;
	SIRInstruction	sMoved;
	size			ui;
	size			uiFirst;
	size			uiSecond;
	size			uiInsert;
	int				iArgument;
	bool			bOutside;
	bool			bChanged;

	FindDefinitions();
	ClearReplacements();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		pcThen = pcBlock->msJump.pcThen;
		pcElse = pcBlock->msJump.pcElse;
		if ((pcBlock->msJump.uiKind != IRJ_Branch) || (pcThen == pcElse) || (pcThen == pcBlock) || (pcElse == pcBlock) ||
			(pcThen->mapcPredecessors.NumElements() != 1) || (pcElse->mapcPredecessors.NumElements() != 1))
		{
			continue;
		}
		for (uiFirst = 0; uiFirst < pcThen->masInstructions.NumElements(); uiFirst++)
		{
			psFirst = pcThen->masInstructions.Get(uiFirst);
			if ((psFirst->uiOpcode == IRO_Load) || !IROptimiserHoistable(mpcFunction, psFirst))
			{
				continue;
			}
			bOutside = true;
			for (iArgument = 0; iArgument < 2; iArgument++)
			{
				bOutside &= !psFirst->asArguments[iArgument].IsTemporary() || (mcDefinitions.Block(psFirst->asArguments[iArgument]) != pcThen);
			}
			if (!bOutside)
			{
				continue;
			}
			for (uiSecond = 0; uiSecond < pcElse->masInstructions.NumElements(); uiSecond++)
			{
				psSecond = pcElse->masInstructions.Get(uiSecond);
				if (IROptimiserSameInstruction(mpcFunction, psFirst, psSecond))
				{
					break;
				}
			}
			if (uiSecond == pcElse->masInstructions.NumElements())
			{
				continue;
			}

			//Before the comparison the brif reads, if that is last.
			Replace(psSecond->sResult, psFirst->sResult);
			pcElse->masInstructions.RemoveAt(uiSecond, true);
			sMoved = *psFirst;
			pcThen->masInstructions.RemoveAt(uiFirst, true);
			uiInsert = pcBlock->masInstructions.NumElements();
			if ((uiInsert > 0) && pcBlock->masInstructions.Get(uiInsert - 1)->sResult.Equals(pcBlock->msJump.sArgument) &&
				!sMoved.asArguments[0].Equals(pcBlock->msJump.sArgument) && !sMoved.asArguments[1].Equals(pcBlock->msJump.sArgument))
			{
				uiInsert--;
			}
			pcBlock->masInstructions.InsertAt(&sMoved, uiInsert);
			bChanged = true;
			ApplyReplacements();
			ClearReplacements();
			FindDefinitions();
			uiFirst = (size)-1;
		}
	}
	return bChanged;
}


#define IR_REDUCE_VALUES	32		//The most values a counter's web may have (ReduceMultiplies).


//////////////////////////////////////////////////////////////////////////
//
// pcBlock's phi of sResult, else NULL.
//
//////////////////////////////////////////////////////////////////////////
static SIRPhi* IROptimiserFindPhi(CIRBlock* pcBlock, SIRReference sResult)
{
	size	ui;

	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		if (pcBlock->masPhis.Get(ui)->sResult.Equals(sResult))
		{
			return pcBlock->masPhis.Get(ui);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds sValue to the web (ReduceMultiplies) if it isn't in it: false if it
// can't be (another type, or too many values).
//
//////////////////////////////////////////////////////////////////////////
static bool IROptimiserAddToWeb(SIRReference* asValues, size* puiValues, SIRReference sValue, SIRType sType, CIRFunction* pcFunction)
{
	size	ui;

	if (!(sValue.IsTemporary() && pcFunction->GetTemporaryType(sValue).Equals(sType)) && !(sValue.IsConstant() && pcFunction->GetConstant(sValue)->sType.Equals(sType)))
	{
		return false;
	}
	for (ui = 0; ui < *puiValues; ui++)
	{
		if (asValues[ui].Equals(sValue))
		{
			return true;
		}
	}
	if (*puiValues == IR_REDUCE_VALUES)
	{
		return false;
	}
	asValues[(*puiValues)++] = sValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// In a loop with one way in, a mul of i by a value w the loop doesn't
// change, i one of a web of values the loop works out from each other
// alone (each a phi of values of the web or from before the loop, or a
// value of the web plus a constant c: a counter i0, i + 1 going round, or
// one stepped on some ways round only), becomes a web of its own: v * w
// for each value from before the loop worked out in the preheader, a phi
// for each phi, + w * c for each add.  Integers wrap the same way either
// way, so it is exact (a 16 bit int's y * width, say).  The multiply each
// time round becomes adds.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReduceMultiplies(void)
{
	size			uiLoop;
	size			uiBlock;
	size			uiIndex;
	size			ui;
	size			uj;
	size			uiValues;
	size			uiArgument;
	CIRBlock*		pcBlock;
	CIRBlock*		pcPreheader;
	CIRBlock*		pcValue;
	SIRInstruction*	psMul;
	SIRInstruction*	psAdd;
	SIRInstruction	sNew;
	SIRPhi*			psPhi;
	SIRPhi*			psProductPhi;
	SIRReference	asValues[IR_REDUCE_VALUES];
	SIRReference	asProducts[IR_REDUCE_VALUES];
	SIRReference	sCounter;
	SIRReference	sFactor;
	SIRReference	sValue;
	SIRReference	sResult;
	SIRReference	sProduct;
	SIRType			sType;
	bool			bWeb;
	int				iArgument;
	int32			iIndex;

	ComputeLoops();
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		if (mcLoops.GetLoop(uiLoop)->pcEntering == NULL)
		{
			continue;
		}
		for (uiBlock = 0; uiBlock < mcLoops.NumBlocks(uiLoop); uiBlock++)
		{
			pcBlock = mcLoops.GetBlock(uiLoop, uiBlock);
			for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
			{
				psMul = pcBlock->masInstructions.Get(uiIndex);
				if ((psMul->uiOpcode != IRO_Mul) || !psMul->sResult.IsTemporary() || !psMul->sType.IsInteger())
				{
					continue;
				}
				sType = psMul->sType;
				sResult = psMul->sResult;		//psMul moves when instructions are put in its block.

				//mul i, w (either way round): i in the loop, w not changed by it.
				for (iArgument = 0; iArgument < 2; iArgument++)
				{
					sCounter = psMul->asArguments[iArgument];
					sFactor = psMul->asArguments[1 - iArgument];
					if (sCounter.IsTemporary() && mcDefinitions.Block(sCounter) && mcLoops.Contains(uiLoop, mcDefinitions.Block(sCounter)) &&
						mcLoops.IsInvariant(uiLoop, sFactor, &mcDefinitions))
					{
						break;
					}
				}
				if (iArgument == 2)
				{
					continue;
				}

				//The web: phis and adds of a constant in the loop, reaching values from before it.
				asValues[0] = sCounter;
				uiValues = 1;
				bWeb = true;
				for (ui = 0; bWeb && (ui < uiValues); ui++)
				{
					sValue = asValues[ui];
					pcValue = sValue.IsTemporary() ? mcDefinitions.Block(sValue) : NULL;
					if ((pcValue == NULL) || !mcLoops.Contains(uiLoop, pcValue))
					{
						//From before the loop: its product is worked out in the preheader.
						continue;
					}
					psPhi = mcDefinitions.Phi(sValue);
					psAdd = mcDefinitions.Instruction(sValue);
					if (psPhi)
					{
						for (uiArgument = 0; bWeb && (uiArgument < psPhi->asArguments.NumElements()); uiArgument++)
						{
							bWeb = IROptimiserAddToWeb(asValues, &uiValues, psPhi->asArguments.Get(uiArgument)->sValue, sType, mpcFunction);
						}
					}
					else if (psAdd && (psAdd->uiOpcode == IRO_Add) && psAdd->asArguments[1].IsConstant() && psAdd->sType.Equals(sType))
					{
						bWeb = IROptimiserAddToWeb(asValues, &uiValues, psAdd->asArguments[0], sType, mpcFunction);
					}
					else
					{
						bWeb = false;
					}
				}
				if (!bWeb)
				{
					continue;
				}

				//A product for each value: made first (phis' arguments may come later), then filled in.
				pcPreheader = mcLoops.GetLoop(uiLoop)->pcPreheader;
				if (pcPreheader == NULL)
				{
					pcPreheader = AddPreheader(uiLoop);
					FindDefinitions();
				}
				for (ui = 0; ui < uiValues; ui++)
				{
					pcValue = asValues[ui].IsTemporary() ? mcDefinitions.Block(asValues[ui]) : NULL;
					if ((pcValue == NULL) || !mcLoops.Contains(uiLoop, pcValue))
					{
						asProducts[ui] = mpcFunction->Add(pcPreheader, IRO_Mul, sType, asValues[ui], sFactor);
					}
					else
					{
						asProducts[ui] = mpcFunction->AddTemporary(sType);
					}
				}
				for (ui = 0; ui < uiValues; ui++)
				{
					pcValue = asValues[ui].IsTemporary() ? mcDefinitions.Block(asValues[ui]) : NULL;
					if ((pcValue == NULL) || !mcLoops.Contains(uiLoop, pcValue))
					{
						continue;
					}
					if (mcDefinitions.Phi(asValues[ui]))
					{
						//The block's phis move when one is added: found again by their results.
						pcValue->AddPhi(sType, asProducts[ui]);
						psProductPhi = IROptimiserFindPhi(pcValue, asProducts[ui]);
						psPhi = IROptimiserFindPhi(pcValue, asValues[ui]);
						for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
						{
							sValue = psPhi->asArguments.Get(uiArgument)->sValue;
							for (uj = 0; !asValues[uj].Equals(sValue); uj++)
							{
							}
							pcValue->AddPhiArgument(psProductPhi, psPhi->asArguments.Get(uiArgument)->pcBlock, asProducts[uj]);
						}
						FindDefinitions();
					}
					else
					{
						//p + w * c just after v + c.
						psAdd = mcDefinitions.Instruction(asValues[ui]);
						for (uj = 0; !asValues[uj].Equals(psAdd->asArguments[0]); uj++)
						{
						}
						sProduct = (mpcFunction->GetSigned(psAdd->asArguments[1]) == 1) ? sFactor :
							mpcFunction->Add(pcPreheader, IRO_Mul, sType, sFactor, psAdd->asArguments[1]);
						memset(&sNew, 0, sizeof(SIRInstruction));
						sNew.uiOpcode = IRO_Add;
						sNew.sType = sType;
						sNew.sResult = asProducts[ui];
						sNew.asArguments[0] = asProducts[uj];
						sNew.asArguments[1] = sProduct;
						iIndex = mcDefinitions.Index(asValues[ui]);
						pcValue->masInstructions.InsertAt(&sNew, (size)iIndex + 1);
						FindDefinitions();
					}
				}
				ClearReplacements();
				Replace(sResult, asProducts[0]);
				ApplyReplacements();
				FindDefinitions();
				return true;
			}
		}
	}
	return false;
}


#define IR_SINK_LIMIT	32		//The most loads SinkLoads moves in a function each time.


//////////////////////////////////////////////////////////////////////////
//
// A load whose only use is an instruction in another block goes to just
// before it (SinkLoad): the ways that don't use it don't load it, and a
// target can read it straight into its use (the W65C816 copies a byte
// through A).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SinkLoads(void)
{
	CArrayTemplate<CIRBlock*>	apcUseBlock;	//Per temporary: the block of its one use by an instruction, else NULL.
	CArrayTemplate<size>		auiUseIndex;
	size						ui;
	size						uiInstruction;
	size						uiArgument;
	size						uiTemporary;
	CIRBlock*					pcBlock;
	SIRInstruction*				psInstruction;
	SIRReference				sValue;
	bool						bSunk;
	bool						bChanged;
	int							iSinks;

	ComputeLoops();
	apcUseBlock.Init();
	auiUseIndex.Init();
	bChanged = false;
	for (iSinks = 0; iSinks < IR_SINK_LIMIT; iSinks++)
	{
		//Where each temporary with one use is used (the positions change with each load moved).
		apcUseBlock.FakeSetUsedElements(0);
		auiUseIndex.FakeSetUsedElements(0);
		apcUseBlock.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
		auiUseIndex.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
		for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				for (uiArgument = 0; uiArgument < 2; uiArgument++)
				{
					sValue = psInstruction->asArguments[uiArgument];
					if (sValue.IsTemporary() && (mcDefinitions.NumUses(sValue) == 1))
					{
						*apcUseBlock.Get(sValue.Index()) = pcBlock;
						*auiUseIndex.Get(sValue.Index()) = uiInstruction;
					}
				}
			}
		}

		bSunk = false;
		for (ui = 0; !bSunk && (ui < mpcFunction->NumBlocks()); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uiInstruction = pcBlock->masInstructions.NumElements(); !bSunk && (uiInstruction > 0); uiInstruction--)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction - 1);
				if ((psInstruction->uiOpcode != IRO_Load) || (psInstruction->uiFlags & IRF_Volatile) || !psInstruction->sResult.IsTemporary())
				{
					continue;
				}
				uiTemporary = psInstruction->sResult.Index();
				if ((*apcUseBlock.Get(uiTemporary) == NULL) || (*apcUseBlock.Get(uiTemporary) == pcBlock))
				{
					continue;
				}
				bSunk = SinkLoad(pcBlock, uiInstruction - 1, *apcUseBlock.Get(uiTemporary), *auiUseIndex.Get(uiTemporary));
			}
		}
		if (!bSunk)
		{
			break;
		}
		bChanged = true;
		FindDefinitions();
	}
	auiUseIndex.Kill();
	apcUseBlock.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// The counter sCounter if sValue is it (bExtended false) or it sign
// extended (bExtended true), for ReduceStrength; IRNone otherwise.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::ExtendedCounter(SIRReference sValue, SIRReference sCounter, bool bExtended)
{
	SIRInstruction*	psExtend;

	if (!bExtended)
	{
		return sValue.Equals(sCounter) ? sCounter : IRNone();
	}
	psExtend = sValue.IsTemporary() ? mcDefinitions.Instruction(sValue) : NULL;
	if ((psExtend == NULL) || (psExtend->uiOpcode != IRO_SExt) || !psExtend->sType.Is(IRTK_I32) || !psExtend->asArguments[0].Equals(sCounter))
	{
		return IRNone();
	}
	return sCounter;
}


//////////////////////////////////////////////////////////////////////////
//
// Strength reduction: in a loop with one block going round to its head and
// a counter i (a phi of init, then i + c or i - c), an address p + i (when
// the target's loads and stores don't add it for nothing),
// p + (i << s) or p + i * k with p not changed by the loop becomes a
// pointer of its own: p + init * k before the loop, stepped by c * k going
// round.  The shift or multiply and the add each time round become one
// add.  A counter narrower than 32 bits is the target's int (msLibraryInt,
// the W65C816's 16 bits) sign extended in the index: an int stepped past
// its end is undefined, so its extension steps with it.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReduceStrength(void)
{
	size					uiLoop;
	size					uiPhi;
	size					uiArgument;
	size					uiBlock;
	size					uiInstruction;
	CIRBlock*				pcHead;
	CIRBlock*				pcBlock;
	CIRBlock*				pcEntering;
	CIRBlock*				pcLatch;
	SIRPhi*					psPhi;
	SIRPhi*					psNewPhi;
	SIRInstruction*			psNext;
	SIRInstruction*			psScaled;
	SIRInstruction*			psAddress;
	SIRReference			sCounter;
	SIRReference			sBase;
	SIRReference			sAddress;
	SIRReference			sInit;
	SIRReference			sNext;
	SIRReference			sStart;
	SIRReference			sPointer;
	SIRReference			sStepped;
	SIRReference			sIndex;
	SIRType					sType;
	int64					lliStep;
	int64					lliScale;
	bool					bChanged;
	bool					bExtended;

	ComputeLoops();
	ClearReplacements();
	bChanged = false;
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		//One way in and one way round.
		pcHead = mcLoops.GetLoop(uiLoop)->pcHead;
		pcEntering = mcLoops.GetLoop(uiLoop)->pcEntering;
		if (!pcEntering || (pcHead->mapcPredecessors.NumElements() != 2))
		{
			continue;
		}
		pcLatch = mcLoops.GetLatch(uiLoop, 0);

		for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
		{
			//The counter: i = phi [entering, init], [latch, i + c].
			psPhi = pcHead->masPhis.Get(uiPhi);
			sCounter = psPhi->sResult;
			bExtended = !psPhi->sType.Is(IRTK_I32);
			if ((bExtended && !psPhi->sType.Equals(msLibraryInt)) || (psPhi->asArguments.NumElements() != 2))
			{
				continue;
			}
			sInit = IRNone();
			sNext = IRNone();
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				if (psPhi->asArguments.Get(uiArgument)->pcBlock == pcEntering)
				{
					sInit = psPhi->asArguments.Get(uiArgument)->sValue;
				}
				else
				{
					sNext = psPhi->asArguments.Get(uiArgument)->sValue;
				}
			}
			psNext = sNext.IsTemporary() ? mcDefinitions.Instruction(sNext) : NULL;
			if (sInit.IsNone() || (psNext == NULL) || ((psNext->uiOpcode != IRO_Add) && (psNext->uiOpcode != IRO_Sub)) || !psNext->asArguments[0].Equals(sCounter) || !psNext->asArguments[1].IsConstant())
			{
				continue;
			}
			lliStep = mpcFunction->GetSigned(psNext->asArguments[1]);
			if (psNext->uiOpcode == IRO_Sub)
			{
				lliStep = -lliStep;
			}
			sType = IRType(IRTK_I32);

			//Addresses p + (i << s) or p + i * k in the loop (i sign extended when narrower).
			for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
			{
				pcBlock = mpcFunction->GetBlock(uiBlock);
				if (!mcLoops.Contains(uiLoop, pcBlock))
				{
					continue;
				}
				for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
				{
					psAddress = pcBlock->masInstructions.Get(uiInstruction);
					if ((psAddress->uiOpcode != IRO_PtrAdd) || !psAddress->asArguments[1].IsTemporary() || !Replacement(psAddress->sResult).Equals(psAddress->sResult) || !mcLoops.IsInvariant(uiLoop, psAddress->asArguments[0], &mcDefinitions))
					{
						continue;
					}
					psScaled = mcDefinitions.Instruction(psAddress->asArguments[1]);
					sIndex = ExtendedCounter(psAddress->asArguments[1], sCounter, bExtended);
					if (sIndex.Equals(sCounter))
					{
						//p + i, unless loads and stores add it for nothing.
						if (mbIndexedAddresses)
						{
							continue;
						}
						lliScale = 1;
					}
					else if ((psScaled == NULL) || (psScaled->uiOpcode == IRO_SExt) || !ExtendedCounter(psScaled->asArguments[0], sCounter, bExtended).Equals(sCounter) || !psScaled->asArguments[1].IsConstant())
					{
						continue;
					}
					else if ((psScaled->uiOpcode == IRO_Shl) && (mpcFunction->GetSigned(psScaled->asArguments[1]) >= 0) && (mpcFunction->GetSigned(psScaled->asArguments[1]) < 16))
					{
						lliScale = (int64)1 << mpcFunction->GetSigned(psScaled->asArguments[1]);
					}
					else if (psScaled->uiOpcode == IRO_Mul)
					{
						lliScale = mpcFunction->GetSigned(psScaled->asArguments[1]);
					}
					else
					{
						continue;
					}

					if (pcEntering->msJump.uiKind != IRJ_Jump)
					{
						pcEntering = AddPreheader(uiLoop);
					}

					//p + init * k before the loop; the pointer steps by c * k going round.  (Adding to the latch may
					//move this block's instructions.)
					sBase = psAddress->asArguments[0];
					sAddress = psAddress->sResult;
					if (sInit.IsConstant())
					{
						sStart = mpcFunction->AddInteger(sType, mpcFunction->GetSigned(sInit) * lliScale);
					}
					else
					{
						sStart = bExtended ? mpcFunction->Add(pcEntering, IRO_SExt, sType, sInit) : sInit;
						sStart = (lliScale == 1) ? sStart : mpcFunction->Add(pcEntering, IRO_Mul, sType, sStart, mpcFunction->AddInteger(sType, lliScale));
					}
					if (!sStart.IsConstant() || (mpcFunction->GetSigned(sStart) != 0))
					{
						sStart = mpcFunction->Add(pcEntering, IRO_PtrAdd, IRType(IRTK_Pointer), sBase, sStart);
					}
					else
					{
						sStart = sBase;
					}
					sPointer = mpcFunction->AddTemporary(IRType(IRTK_Pointer));
					sStepped = mpcFunction->Add(pcLatch, IRO_PtrAdd, IRType(IRTK_Pointer), sPointer, mpcFunction->AddInteger(sType, lliStep * lliScale));
					psNewPhi = pcHead->AddPhi(IRType(IRTK_Pointer), sPointer);
					pcHead->AddPhiArgument(psNewPhi, pcEntering, sStart);
					pcHead->AddPhiArgument(psNewPhi, pcLatch, sStepped);
					Replace(sAddress, sPointer);
					bChanged = true;

					//AddPhi may have moved the head's phis.
					psPhi = pcHead->masPhis.Get(uiPhi);
					FindDefinitions();
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
// Loop invariant code motion: in a loop entered from only one block, an
// instruction IROptimiserHoistable allows whose operands are all worked out
// outside the loop moves to the end of that block (or of a block put on its
// way to the head, when it branches: AddPreheader).  A load only moves if
// the loop makes no calls and stores nothing that may be where it reads
// (see MayAlias).  Loads moved may be done when the loop wouldn't have done
// them, which the 65C64 doesn't mind.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::HoistInvariants(void)
{
	CArrayIRAvailables		asStores;
	size					uiLoop;
	size					uiBlock;
	size					uiInstruction;
	size					uiStore;
	CIRBlock*				pcBlock;
	CIRBlock*				pcEntering;
	SIRInstruction*			psInstruction;
	SIRInstruction			sMoved;
	SIRReference			sRoot;
	int64					lliOffset;
	bool					bCalls;
	bool					bInvariant;
	bool					bChanged;
	bool					bPreheader;

	ComputeLoops();
	asStores.Init();
	bChanged = false;

	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		pcEntering = mcLoops.GetLoop(uiLoop)->pcEntering;
		if (!pcEntering)
		{
			continue;
		}
		bPreheader = pcEntering->msJump.uiKind != IRJ_Jump;

		//What the loop writes.
		bCalls = false;
		asStores.FakeSetUsedElements(0);
		for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
		{
			pcBlock = mpcFunction->GetBlock(uiBlock);
			if (!mcLoops.Contains(uiLoop, pcBlock))
			{
				continue;
			}
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				if (psInstruction->uiOpcode == IRO_Store)
				{
					SplitAddress(psInstruction->asArguments[1], &sRoot, &lliOffset);
					AddAvailable(&asStores, sRoot, lliOffset, psInstruction->sType, IRNone());
					bCalls |= asStores.NumElements() >= IR_AVAILABLE_LIMIT;
				}
				else if (HasEffect(psInstruction) && (psInstruction->uiOpcode != IRO_Arg))
				{
					bCalls = true;
				}
			}
		}

		//The loop's blocks in order (dominators before what they dominate), moving what can be.
		for (uiBlock = 0; uiBlock < mcDominators.NumReachable(); uiBlock++)
		{
			pcBlock = mcDominators.GetReversePostOrder(uiBlock);
			if (!mcLoops.Contains(uiLoop, pcBlock))
			{
				continue;
			}
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				if (!IROptimiserHoistable(mpcFunction, psInstruction))
				{
					continue;
				}
				bInvariant = mcLoops.IsInvariant(uiLoop, psInstruction->asArguments[0], &mcDefinitions) && mcLoops.IsInvariant(uiLoop, psInstruction->asArguments[1], &mcDefinitions);
				if (bInvariant && (psInstruction->uiOpcode == IRO_Load))
				{
					bInvariant = !bCalls;
					SplitAddress(psInstruction->asArguments[0], &sRoot, &lliOffset);
					for (uiStore = 0; bInvariant && (uiStore < asStores.NumElements()); uiStore++)
					{
						bInvariant = !MayAlias(asStores.Get(uiStore), sRoot, lliOffset, mpcModule->GetSize(psInstruction->sType));
					}
				}
				if (!bInvariant)
				{
					continue;
				}
				if (bPreheader)
				{
					pcEntering = AddPreheader(uiLoop);
					bPreheader = false;
				}
				sMoved = *psInstruction;
				pcBlock->masInstructions.RemoveAt(uiInstruction, true);
				pcEntering->masInstructions.Add(&sMoved);
				uiInstruction--;
				bChanged = true;

				//Moving shifts the block's later instructions.
				FindDefinitions();
			}
		}
	}
	asStores.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// sValue as a copy of a loop's head sees it: what pasMap gives for a
// temporary of the head, else itself.
//
//////////////////////////////////////////////////////////////////////////
static SIRReference IROptimiserMapped(CArrayIRReferences* pasMap, SIRReference sValue)
{
	SIRReference	sMapped;

	if (!sValue.IsTemporary() || (sValue.Index() >= pasMap->NumElements()))
	{
		return sValue;
	}
	sMapped = *pasMap->Get(sValue.Index());
	return sMapped.IsNone() ? sValue : sMapped;
}


//////////////////////////////////////////////////////////////////////////
//
// True if loop uiLoop can be rotated (RotateLoops): one latch that jumps
// to the head (any number of blocks outside may), and a head of at most IR_ROTATE_LIMIT
// instructions without effects ending in a brif to *ppcBody, in the loop
// and entered only from the head, or *ppcExit, outside it, and phis that
// the latch passes values from outside the head.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::CanRotate(size uiLoop, CIRBlock** ppcBody, CIRBlock** ppcExit)
{
	SIRLoop*		psLoop;
	CIRBlock*		pcHead;
	CIRBlock*		pcLatch;
	SIRInstruction*	psInstruction;
	SIRPhiArgument*	psArgument;
	size			ui;

	psLoop = mcLoops.GetLoop(uiLoop);
	pcHead = psLoop->pcHead;
	if ((mcLoops.NumLatches(uiLoop) != 1) || (pcHead->mapcPredecessors.NumElements() < 2))
	{
		return false;
	}
	pcLatch = mcLoops.GetLatch(uiLoop, 0);
	if ((pcLatch == pcHead) || (pcLatch->msJump.uiKind != IRJ_Jump) || (pcHead->msJump.uiKind != IRJ_Branch) || (pcHead->masInstructions.NumElements() > IR_ROTATE_LIMIT))
	{
		return false;
	}

	if (mcLoops.Contains(uiLoop, pcHead->msJump.pcThen) && !mcLoops.Contains(uiLoop, pcHead->msJump.pcElse))
	{
		*ppcBody = pcHead->msJump.pcThen;
		*ppcExit = pcHead->msJump.pcElse;
	}
	else if (mcLoops.Contains(uiLoop, pcHead->msJump.pcElse) && !mcLoops.Contains(uiLoop, pcHead->msJump.pcThen))
	{
		*ppcBody = pcHead->msJump.pcElse;
		*ppcExit = pcHead->msJump.pcThen;
	}
	else
	{
		return false;
	}
	if (((*ppcBody)->mapcPredecessors.NumElements() != 1) || (*ppcBody == pcHead))
	{
		return false;
	}

	for (ui = 0; ui < pcHead->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcHead->masInstructions.Get(ui);
		if (HasEffect(psInstruction) || !psInstruction->sResult.IsTemporary())
		{
			return false;
		}
	}

	//The latch's copy reads the phis as the values it passes them, which mustn't be the head's own.
	for (ui = 0; ui < pcHead->masPhis.NumElements(); ui++)
	{
		psArgument = pcHead->FindPhiArgument(pcHead->masPhis.Get(ui), pcLatch);
		if (!psArgument || (mcDefinitions.Block(psArgument->sValue) == pcHead))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The head's instructions copied to the end of pcBlock, each temporary of
// the head read as pasMap gives it (and the copies' results added to it),
// with pcBlock's jump the head's brif.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::CopyHead(CIRBlock* pcHead, CIRBlock* pcBlock, CArrayIRReferences* pasMap)
{
	SIRInstruction	sCopy;
	size			ui;

	for (ui = 0; ui < pcHead->masInstructions.NumElements(); ui++)
	{
		sCopy = *pcHead->masInstructions.Get(ui);
		sCopy.asArguments[0] = IROptimiserMapped(pasMap, sCopy.asArguments[0]);
		sCopy.asArguments[1] = IROptimiserMapped(pasMap, sCopy.asArguments[1]);
		sCopy.sResult = mpcFunction->AddTemporary(mpcFunction->GetTemporaryType(sCopy.sResult));
		*pasMap->Get(pcHead->masInstructions.Get(ui)->sResult.Index()) = sCopy.sResult;
		pcBlock->masInstructions.Add(&sCopy);
	}
	pcBlock->Branch(IROptimiserMapped(pasMap, pcHead->msJump.sArgument), pcHead->msJump.pcThen, pcHead->msJump.pcElse);
}


//////////////////////////////////////////////////////////////////////////
//
// The phis of pcBlock that took a value from pcHead take it from the
// preheader and the latch instead, as each sees it.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SplitPhiArguments(CIRBlock* pcBlock, CIRBlock* pcHead, CIRBlock* pcPreheader, CArrayIRReferences* pasPreheader, CIRBlock* pcLatch, CArrayIRReferences* pasLatch)
{
	size			ui;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sValue;

	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		psPhi = pcBlock->masPhis.Get(ui);
		psArgument = pcBlock->FindPhiArgument(psPhi, pcHead);
		if (psArgument)
		{
			sValue = psArgument->sValue;
			psArgument->pcBlock = pcPreheader;
			psArgument->sValue = IROptimiserMapped(pasPreheader, sValue);
			pcBlock->AddPhiArgument(psPhi, pcLatch, IROptimiserMapped(pasLatch, sValue));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Uses of sValue are what pcUpdater says reaches them: in a phi, what its
// predecessor ends with; elsewhere, what the block starts with (in the
// preheader and the latch, before their copies of the head define it).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::RewriteUses(SIRReference sValue, CIRSSAUpdater* pcUpdater)
{
	size			ui;
	size			uiIndex;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRInstruction*	psInstruction;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				psArgument = psPhi->asArguments.Get(uiArgument);
				if (psArgument->sValue.Equals(sValue))
				{
					psArgument->sValue = pcUpdater->ValueAtEnd(psArgument->pcBlock);
				}
				psPhi = pcBlock->masPhis.Get(uiIndex);
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				if (psInstruction->asArguments[uiArgument].Equals(sValue))
				{
					psInstruction->asArguments[uiArgument] = pcUpdater->ValueAtStart(pcBlock);
				}
			}
		}
		if (pcBlock->msJump.sArgument.Equals(sValue))
		{
			pcBlock->msJump.sArgument = pcUpdater->ValueAtStart(pcBlock);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Loop rotation: a loop whose head tests and whose latch jumps back to it
// (CanRotate) tests at the bottom instead.  The head is copied to the end of
// the preheader (going in only if the first test passes) and to the end of
// the latch (going round while it passes), with its phis read as the values
// each brings, and goes; the loop's body is its head now.  Each time round
// takes one jump fewer.  The head's values are each defined twice then,
// which the SSA updater puts phis in for.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RotateLoops(void)
{
	CArrayIRReferences	asPreheader;		//Per temporary of the head: what the preheader's copy reads for it.
	CArrayIRReferences	asLatch;
	CArrayIRReferences	asValues;		//The head's phis and results.
	CIRSSAUpdater		cUpdater;
	size				uiLoop;
	size				ui;
	size				uiTemporaries;
	CIRBlock*			pcHead;
	CIRBlock*			pcPreheader;
	CIRBlock*			pcLatch;
	CIRBlock*			pcBody;
	CIRBlock*			pcExit;
	SIRPhi*				psPhi;
	SIRReference		sValue;
	SIRType				sType;
	bool				bChanged;

	asPreheader.Init();
	asLatch.Init();
	asValues.Init();
	cUpdater.Init(mpcFunction);
	bChanged = false;
	ComputeLoops();
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		if (!CanRotate(uiLoop, &pcBody, &pcExit))
		{
			continue;
		}
		pcHead = mcLoops.GetLoop(uiLoop)->pcHead;
		pcLatch = mcLoops.GetLatch(uiLoop, 0);
		pcPreheader = mcLoops.GetLoop(uiLoop)->pcPreheader;
		if (!pcPreheader)
		{
			pcPreheader = mcLoops.GetLoop(uiLoop)->pcEntering ? AddPreheader(uiLoop) : AddMergedPreheader(uiLoop);
		}

		//The head's phis are the values each way in brings.
		uiTemporaries = mpcFunction->NumTemporaries();
		asPreheader.FakeSetUsedElements(0);
		asPreheader.GrowToAtLeastNumElements(uiTemporaries, true, 0);
		asLatch.FakeSetUsedElements(0);
		asLatch.GrowToAtLeastNumElements(uiTemporaries, true, 0);
		asValues.FakeSetUsedElements(0);
		for (ui = 0; ui < pcHead->masPhis.NumElements(); ui++)
		{
			psPhi = pcHead->masPhis.Get(ui);
			*asPreheader.Get(psPhi->sResult.Index()) = pcHead->FindPhiArgument(psPhi, pcPreheader)->sValue;
			*asLatch.Get(psPhi->sResult.Index()) = pcHead->FindPhiArgument(psPhi, pcLatch)->sValue;
			asValues.Add(&psPhi->sResult);
		}
		for (ui = 0; ui < pcHead->masInstructions.NumElements(); ui++)
		{
			asValues.Add(&pcHead->masInstructions.Get(ui)->sResult);
		}

		//The copies, the body and the exit taking from the preheader and the latch, and the head gone.
		CopyHead(pcHead, pcPreheader, &asPreheader);
		CopyHead(pcHead, pcLatch, &asLatch);
		SplitPhiArguments(pcBody, pcHead, pcPreheader, &asPreheader, pcLatch, &asLatch);
		SplitPhiArguments(pcExit, pcHead, pcPreheader, &asPreheader, pcLatch, &asLatch);
		mpcFunction->RemoveBlock(pcHead);
		mpcFunction->ComputePredecessors();

		//Each of the head's values is defined at the ends of the preheader and the latch.
		for (ui = 0; ui < asValues.NumElements(); ui++)
		{
			sValue = *asValues.Get(ui);
			sType = mpcFunction->GetTemporaryType(sValue);
			cUpdater.Begin(sType);
			cUpdater.Define(pcPreheader, IROptimiserMapped(&asPreheader, sValue));
			cUpdater.Define(pcLatch, IROptimiserMapped(&asLatch, sValue));
			RewriteUses(sValue, &cUpdater);
		}
		bChanged = true;
		ComputeLoops();
		uiLoop = (size)-1;
	}
	cUpdater.Kill();
	asValues.Kill();
	asLatch.Kill();
	asPreheader.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the low bits of uiOpcode's result depend only on the low bits of
// its operands (a shift left by a constant less than iBits too).
//
//////////////////////////////////////////////////////////////////////////
static bool IROptimiserNarrows(CIRFunction* pcFunction, SIRInstruction* psInstruction, int iBits)
{
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_Sub:
	case IRO_Mul:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
		return true;
	case IRO_Shl:
		return psInstruction->asArguments[1].IsConstant() && (pcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits < (uint64)iBits);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// sValue's low bits as a value of the narrower sType with no instruction: a
// constant truncated, or the value an extension from sType extended; IRNone
// when it takes one.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::FreeNarrow(SIRReference sValue, SIRType sType)
{
	SIRInstruction*	psDefinition;

	if (sValue.IsConstant())
	{
		return mpcFunction->AddConstant(sType, IRTruncate(sType, mpcFunction->GetConstant(sValue)->ulliBits));
	}
	psDefinition = mcDefinitions.Instruction(sValue);
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
SIRReference CIROptimiser::Narrowed(SIRReference sValue, SIRType sType, CArrayIRInstructions* pasNew, int iDepth)
{
	SIRInstruction*	psDefinition;
	SIRInstruction	sNew;
	SIRReference	sNarrow;

	sNarrow = FreeNarrow(sValue, sType);
	if (!sNarrow.IsNone())
	{
		return sNarrow;
	}
	psDefinition = mcDefinitions.Instruction(sValue);
	memset(&sNew, 0, sizeof(SIRInstruction));
	if (psDefinition && (iDepth < IR_NARROW_DEPTH) && (mcDefinitions.NumUses(sValue) == 1) && IROptimiserNarrows(mpcFunction, psDefinition, sType.IntegerBits()))
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


#define IR_BOUND_DEPTH	6


//////////////////////////////////////////////////////////////////////////
//
// The most sValue can be, unsigned: a constant; the narrow type's most for a
// zext; the smaller of an and's; x >> c, x % c and x / c of x's; the sum of
// an add's that can't wrap; the most of a phi's arguments, each as the
// branch to the phi bounds it (EdgeBound) or as it is.  Else all ones of its
// type.  mcDefinitions must be found.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIROptimiser::UpperBound(SIRReference sValue, int iDepth)
{
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	CIRBlock*		pcBlock;
	SIRType			sType;
	uint64			ulliMost;
	uint64			ulliFirst;
	uint64			ulliSecond;
	uint64			ulliConstant;
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

	psInstruction = mcDefinitions.Instruction(sValue);
	if (psInstruction)
	{
		ulliConstant = psInstruction->asArguments[1].IsConstant() ? IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) : 0;
		switch (psInstruction->uiOpcode)
		{
		case IRO_ZExt:
			return IRTruncate(mpcFunction->TypeOf(psInstruction->asArguments[0]), UpperBound(psInstruction->asArguments[0], iDepth - 1));
		case IRO_And:
			ulliFirst = UpperBound(psInstruction->asArguments[0], iDepth - 1);
			ulliSecond = UpperBound(psInstruction->asArguments[1], iDepth - 1);
			return (ulliFirst < ulliSecond) ? ulliFirst : ulliSecond;
		case IRO_LShr:
			if (psInstruction->asArguments[1].IsConstant() && (ulliConstant < 64))
			{
				return UpperBound(psInstruction->asArguments[0], iDepth - 1) >> ulliConstant;
			}
			return ulliMost;
		case IRO_URem:
			if (psInstruction->asArguments[1].IsConstant() && (ulliConstant != 0))
			{
				ulliFirst = UpperBound(psInstruction->asArguments[0], iDepth - 1);
				return (ulliFirst < ulliConstant - 1) ? ulliFirst : ulliConstant - 1;
			}
			return ulliMost;
		case IRO_UDiv:
			if (psInstruction->asArguments[1].IsConstant() && (ulliConstant != 0))
			{
				return UpperBound(psInstruction->asArguments[0], iDepth - 1) / ulliConstant;
			}
			return ulliMost;
		case IRO_Add:
			ulliFirst = UpperBound(psInstruction->asArguments[0], iDepth - 1);
			ulliSecond = UpperBound(psInstruction->asArguments[1], iDepth - 1);
			if ((ulliFirst <= ulliMost - ulliSecond) && (ulliSecond <= ulliMost))
			{
				return ulliFirst + ulliSecond;
			}
			return ulliMost;
		default:
			return ulliMost;
		}
	}

	psPhi = mcDefinitions.Phi(sValue);
	if (psPhi)
	{
		pcBlock = mcDefinitions.Block(sValue);
		ulliFirst = 0;
		for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
		{
			psArgument = psPhi->asArguments.Get(ui);
			ulliSecond = EdgeBound(psArgument->pcBlock, pcBlock, psArgument->sValue);
			if (ulliSecond == (uint64)-1)
			{
				ulliSecond = UpperBound(psArgument->sValue, iDepth - 1);
			}
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
// The most sValue can be going from pcFrom to pcTo when pcFrom branches
// there only if an unsigned comparison of sValue with a constant says so
// (v < n, v <= n, or not v >= n and the like), else all ones.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIROptimiser::EdgeBound(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue)
{
	SIRInstruction*	psCompare;
	SIRReference	sConstant;
	uint16			uiOpcode;
	uint64			ulliConstant;

	if ((pcFrom->msJump.uiKind != IRJ_Branch) || (pcFrom->msJump.pcThen == pcFrom->msJump.pcElse))
	{
		return (uint64)-1;
	}
	psCompare = mcDefinitions.Instruction(pcFrom->msJump.sArgument);
	if (!psCompare)
	{
		return (uint64)-1;
	}
	uiOpcode = psCompare->uiOpcode;
	if (psCompare->asArguments[0].Equals(sValue))
	{
		sConstant = psCompare->asArguments[1];
	}
	else if (psCompare->asArguments[1].Equals(sValue))
	{
		//n > v is v < n.
		sConstant = psCompare->asArguments[0];
		uiOpcode = IRSwappedComparison(uiOpcode);
	}
	else
	{
		return (uint64)-1;
	}
	if (!sConstant.IsConstant())
	{
		return (uint64)-1;
	}
	if (pcFrom->msJump.pcElse == pcTo)
	{
		uiOpcode = IRInvertedComparison(uiOpcode);
	}
	else if (pcFrom->msJump.pcThen != pcTo)
	{
		return (uint64)-1;
	}
	ulliConstant = IRTruncate(psCompare->sType, mpcFunction->GetConstant(sConstant)->ulliBits);
	if ((uiOpcode == IRO_ULt) && (ulliConstant != 0))
	{
		return ulliConstant - 1;
	}
	if (uiOpcode == IRO_ULe)
	{
		return ulliConstant;
	}
	return (uint64)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// A new instruction at uiIndex in pcBlock's instructions; sType is as
// CIRFunction::Add takes it.  It has the source line of the instruction it
// goes before.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::InsertInstruction(CIRBlock* pcBlock, size uiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond)
{
	SIRInstruction	sMoved;
	SIRReference	sResult;
	size			uiLast;

	sResult = mpcFunction->Add(pcBlock, uiOpcode, sType, sFirst, sSecond);
	uiLast = pcBlock->masInstructions.NumElements() - 1;
	sMoved = *pcBlock->masInstructions.Get(uiLast);
	if ((sMoved.uiSource == 0) && (uiIndex < uiLast))
	{
		//Part of the work of the instruction it goes before: its source line too (for listings and profiles).
		sMoved.uiSource = pcBlock->masInstructions.Get(uiIndex)->uiSource;
	}
	pcBlock->masInstructions.RemoveAt(uiLast, true);
	pcBlock->masInstructions.InsertAt(&sMoved, uiIndex);
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A signed x / 2^n or x % 2^n is shifts and adds, not the target's divide:
// a negative x has 2^n - 1 added first, so the shift rounds towards 0.
//   t = x + ((x >> (bits - 1)) >>> (bits - n))
//   x / 2^n = t >> n
//   x % 2^n = x - (t & -2^n)
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SignedPowerDivisions(void)
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
bool CIROptimiser::BoundDivisions(void)
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
	mbBranchRanges = true;
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
			ulliBound = ((uiOpcode == IRO_UDiv) || (uiOpcode == IRO_URem)) ? UpperBound(psInstruction->asArguments[0], IR_BOUND_DEPTH) : (uint64)-1;
			if ((ulliBound >= ulliDivisor * 2) && RangeAt(pcBlock, psInstruction->asArguments[0], &lliMin, &lliMax) && (lliMin >= 0))
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
	mbBranchRanges = false;

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


#define IR_DIVISION_RANGE	0x20000		//The most values ConstantDivisions checks a reciprocal for.
#define IR_DIVISION_TABLE_SAVING	50	//The cycles a byte divide's table saves over its reciprocal (a hot one's, IRF_Hot).


//////////////////////////////////////////////////////////////////////////
//
// The signed range of sValue as an integer of its type: a constant's; a
// sext's of the narrower value's; x + c's, x - c's and c - x's moved by c
// (when it can't wrap); a divide's or remainder's by a constant
// (DivisionRange); x + y's, x - y's, x * c's and shifts' by a constant
// (ArithmeticRange); a phi's of all its values (up to iDepth definitions
// back); else from 0 to UpperBound when that is below the type's sign bit
// (a zext, an and).  With mbBranchRanges an operand's range is narrowed by
// the branches to the instruction's block (RangeAt) and a phi's values by
// the branches to the phi on their way.  False if nothing narrower than the
// type is known.  mcDefinitions must be found.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SignedRange(SIRReference sValue, int64* plliMin, int64* plliMax, int iDepth)
{
	SIRInstruction*	psInstruction;
	SIRType			sType;
	SIRType			sNarrow;
	int				iBits;
	uint64			ulliBound;
	int64			lliConstant;
	int64			lliArgumentMin;
	int64			lliArgumentMax;
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
		*plliMin = IRSignExtend(sType, mpcFunction->GetConstant(sValue)->ulliBits);
		*plliMax = *plliMin;
		return true;
	}
	psInstruction = sValue.IsTemporary() ? mcDefinitions.Instruction(sValue) : NULL;
	if (psInstruction && ((psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_Sub)) && psInstruction->asArguments[1].IsConstant())
	{
		//x + c, x - c: x's range moved, when it can't wrap.
		lliConstant = IRSignExtend(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
		if ((iDepth > 0) && RangeAt(mcDefinitions.Block(sValue), psInstruction->asArguments[0], plliMin, plliMax, iDepth - 1))
		{
			if (psInstruction->uiOpcode == IRO_Sub)
			{
				lliConstant = -lliConstant;
			}
			*plliMin += lliConstant;
			*plliMax += lliConstant;
			return (*plliMin >= -((int64)1 << (iBits - 1))) && (*plliMax < ((int64)1 << (iBits - 1)));
		}
	}
	if (psInstruction && (psInstruction->uiOpcode == IRO_Sub) && psInstruction->asArguments[0].IsConstant() && (iDepth > 0))
	{
		//c - x: x's range turned over and moved by c, when it can't wrap.
		lliConstant = IRSignExtend(sType, mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits);
		if (RangeAt(mcDefinitions.Block(sValue), psInstruction->asArguments[1], &lliArgumentMin, &lliArgumentMax, iDepth - 1))
		{
			*plliMin = lliConstant - lliArgumentMax;
			*plliMax = lliConstant - lliArgumentMin;
			return (*plliMin >= -((int64)1 << (iBits - 1))) && (*plliMax < ((int64)1 << (iBits - 1)));
		}
	}
	if (psInstruction && (iDepth > 0) && DivisionRange(psInstruction, plliMin, plliMax, iDepth - 1))
	{
		return true;
	}
	if (psInstruction && (iDepth > 0) && ArithmeticRange(psInstruction, plliMin, plliMax, iDepth - 1))
	{
		return true;
	}
	psPhi = sValue.IsTemporary() ? mcDefinitions.Phi(sValue) : NULL;
	if (psPhi && (iDepth > 0))
	{
		//A phi: the range of all its values (each narrowed by the branches on its way).
		for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
		{
			if (!RangeAt(psPhi->asArguments.Get(ui)->pcBlock, psPhi->asArguments.Get(ui)->sValue, &lliArgumentMin, &lliArgumentMax, iDepth - 1, mcDefinitions.Block(sValue)))
			{
				return false;
			}
			*plliMin = ((ui == 0) || (lliArgumentMin < *plliMin)) ? lliArgumentMin : *plliMin;
			*plliMax = ((ui == 0) || (lliArgumentMax > *plliMax)) ? lliArgumentMax : *plliMax;
		}
		return psPhi->asArguments.NumElements() > 0;
	}
	if (psInstruction && (psInstruction->uiOpcode == IRO_SExt))
	{
		sNarrow = mpcFunction->TypeOf(psInstruction->asArguments[0]);
		if (sNarrow.IsInteger() && (sNarrow.IntegerBits() < iBits))
		{
			if ((iDepth == 0) || !SignedRange(psInstruction->asArguments[0], plliMin, plliMax, iDepth - 1))
			{
				*plliMin = -((int64)1 << (sNarrow.IntegerBits() - 1));
				*plliMax = ((int64)1 << (sNarrow.IntegerBits() - 1)) - 1;
			}
			return true;
		}
	}
	ulliBound = UpperBound(sValue, IR_BOUND_DEPTH);
	if (ulliBound < ((uint64)1 << (iBits - 1)))
	{
		*plliMin = 0;
		*plliMax = (int64)ulliBound;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The signed range of sValue where pcBlock runs (on its way to pcTo when
// pcTo isn't NULL): SignedRange's, or the type's when that isn't known,
// narrowed by the branches there (BranchRange, DominatingRange) when
// mbBranchRanges.  False if nothing narrower than the type is known.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RangeAt(CIRBlock* pcBlock, SIRReference sValue, int64* plliMin, int64* plliMax, int iDepth, CIRBlock* pcTo)
{
	SIRType	sType;
	int64	lliLeast;
	int64	lliMost;
	bool	bKnown;

	bKnown = SignedRange(sValue, plliMin, plliMax, iDepth);
	if (!mbBranchRanges || !pcBlock)
	{
		return bKnown;
	}
	sType = mpcFunction->TypeOf(sValue);
	if (!sType.IsInteger() || (sType.IntegerBits() > 32))
	{
		return bKnown;
	}
	lliLeast = -((int64)1 << (sType.IntegerBits() - 1));
	lliMost = ((int64)1 << (sType.IntegerBits() - 1)) - 1;
	if (!bKnown)
	{
		*plliMin = lliLeast;
		*plliMax = lliMost;
	}
	if (pcTo)
	{
		BranchRange(pcBlock, pcTo, sValue, plliMin, plliMax);
	}
	DominatingRange(pcBlock, sValue, plliMin, plliMax);
	if (*plliMin > *plliMax)
	{
		//No value gets here (the branches contradict): any range will do.
		*plliMax = *plliMin;
	}
	return bKnown || (*plliMin > lliLeast) || (*plliMax < lliMost);
}


//////////////////////////////////////////////////////////////////////////
//
// The signed range of x + y, x - y (both ranges known), x * c, x << c,
// x >> c (arithmetic, or logical of an x never negative) and a trunc of x
// that fits the narrower type, when it can't wrap.  False for anything
// else.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ArithmeticRange(SIRInstruction* psInstruction, int64* plliMin, int64* plliMax, int iDepth)
{
	CIRBlock*	pcBlock;
	SIRType		sType;
	uint16		uiOpcode;
	int64		lliMin;
	int64		lliMax;
	int64		lliOtherMin;
	int64		lliOtherMax;
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
	pcBlock = mcDefinitions.Block(psInstruction->sResult);
	if (!RangeAt(pcBlock, psInstruction->asArguments[0], &lliMin, &lliMax, iDepth))
	{
		return false;
	}
	if (uiOpcode == IRO_Trunc)
	{
		*plliMin = lliMin;
		*plliMax = lliMax;
	}
	else if ((uiOpcode == IRO_Add) || (uiOpcode == IRO_Sub))
	{
		if (!RangeAt(pcBlock, psInstruction->asArguments[1], &lliOtherMin, &lliOtherMax, iDepth))
		{
			return false;
		}
		*plliMin = (uiOpcode == IRO_Add) ? lliMin + lliOtherMin : lliMin - lliOtherMax;
		*plliMax = (uiOpcode == IRO_Add) ? lliMax + lliOtherMax : lliMax - lliOtherMin;
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
			*plliMin = (lliConstant >= 0) ? lliMin * lliConstant : lliMax * lliConstant;
			*plliMax = (lliConstant >= 0) ? lliMax * lliConstant : lliMin * lliConstant;
		}
		else if ((uiOpcode == IRO_AShr) || (lliMin >= 0))
		{
			*plliMin = lliMin >> lliConstant;
			*plliMax = lliMax >> lliConstant;
		}
		else
		{
			return false;
		}
	}
	return (*plliMin >= -((int64)1 << (iBits - 1))) && (*plliMax < ((int64)1 << (iBits - 1)));
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
bool CIROptimiser::DivisionRange(SIRInstruction* psInstruction, int64* plliMin, int64* plliMax, int iDepth)
{
	SIRType	sType;
	uint16	uiOpcode;
	int64	lliDivisor;
	int64	lliMin;
	int64	lliMax;
	int		iBits;
	bool	bSigned;

	uiOpcode = psInstruction->uiOpcode;
	sType = psInstruction->sType;
	if (((uiOpcode != IRO_SDiv) && (uiOpcode != IRO_SRem) && (uiOpcode != IRO_UDiv) && (uiOpcode != IRO_URem)) ||
		!psInstruction->asArguments[1].IsConstant() || !sType.IsInteger() || (sType.IntegerBits() > 32))
	{
		return false;
	}
	iBits = sType.IntegerBits();
	bSigned = (uiOpcode == IRO_SDiv) || (uiOpcode == IRO_SRem);
	lliDivisor = bSigned ? IRSignExtend(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) :
						   (int64)IRTruncate(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
	if (lliDivisor < 2)
	{
		return false;
	}
	if (!RangeAt(mcDefinitions.Block(psInstruction->sResult), psInstruction->asArguments[0], &lliMin, &lliMax, iDepth))
	{
		lliMin = -((int64)1 << (iBits - 1));
		lliMax = ((int64)1 << (iBits - 1)) - 1;
	}
	if (!bSigned && (lliMin < 0))
	{
		lliMin = 0;
		lliMax = ((int64)1 << iBits) - 1;
	}
	if ((uiOpcode == IRO_SDiv) || (uiOpcode == IRO_UDiv))
	{
		//Rounding towards 0 keeps the order.
		*plliMin = lliMin / lliDivisor;
		*plliMax = lliMax / lliDivisor;
	}
	else
	{
		*plliMin = (lliMin < 0) ? ((lliMin > -lliDivisor) ? lliMin : -(lliDivisor - 1)) : 0;
		*plliMax = (lliMax > 0) ? ((lliMax < lliDivisor) ? lliMax : lliDivisor - 1) : 0;
		if ((lliMin >= 0) && (lliMax < lliDivisor))
		{
			*plliMin = lliMin;
		}
		if ((lliMax <= 0) && (lliMin > -lliDivisor))
		{
			*plliMax = lliMax;
		}
	}
	return (*plliMax < ((int64)1 << (iBits - 1)));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether x / divisor and x % divisor for x from lliMin to lliMax can be
// looked up in a table of a byte's (an i8 divide marked IRF_Hot): x fits a
// signed byte (or an unsigned one when it is never negative).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::IsByteTableDivision(int64 lliMin, int64 lliMax, int64 lliDivisor)
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
bool CIROptimiser::DivisionReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, uint64* pulliMultiplier, int* piShift)
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
	lliMost = bSigned ? (((int64)1 << (iBits - 1)) - 1) : (((int64)1 << iBits) - 1);
	lliLeast = bSigned ? -((int64)1 << (iBits - 1)) : 0;
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
bool CIROptimiser::FindReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, uint64* pulliMultiplier, int* piShift, int* piBits, int* piPreShift)
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
bool CIROptimiser::DivisibilityTests(void)
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
			psRemainder = mcDefinitions.Instruction(psInstruction->asArguments[0]);
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
			if (bSigned && (!SignedRange(sValue, &lliMin, &lliMax) || (lliMin < 0)))
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
// (mbBranchRanges).  A 32 bit divide by a wide c = 2^k * m (m under 65536)
// that isn't made a multiply is split (SplitDivisor).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ConstantDivisions(void)
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
	mbBranchRanges = true;
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
			if (!RangeAt(pcBlock, psInstruction->asArguments[0], &lliMin, &lliMax) || (!bSigned && (lliMin < 0)))
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
	mbBranchRanges = false;

	//Last first, so each instruction's index is still where it was found.
	for (ui = asFound.NumElements(); ui > 0; ui--)
	{
		psFound = asFound.Get(ui - 1);
		pcBlock = psFound->pcBlock;
		uiInstruction = psFound->uiInstruction;
		psInstruction = pcBlock->masInstructions.Get(uiInstruction);
		uiOpcode = psInstruction->uiOpcode;
		sType = psInstruction->sType;
		sValue = psInstruction->asArguments[0];
		bSigned = (uiOpcode == IRO_SDiv) || (uiOpcode == IRO_SRem);
		lliDivisor = bSigned ? IRSignExtend(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) : (int64)IRTruncate(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
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
			else if (HasDivision(pcBlock, uiOpcode == IRO_SRem ? IRO_SDiv : IRO_UDiv, sValue, psInstruction->asArguments[1]))
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
bool CIROptimiser::SplitDivisor(SIRInstruction* psInstruction, int64 lliMin, int64 lliMax)
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
// Whether pcBlock has a uiOpcode of sValue by sDivisor.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::HasDivision(CIRBlock* pcBlock, uint16 uiOpcode, SIRReference sValue, SIRReference sDivisor)
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




//////////////////////////////////////////////////////////////////////////
//
// Whether pcFrom's branch to pcTo is taken only when sValue isn't null
// (a brif on sValue != 0 to its then, or on sValue == 0 to its else).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::BranchNonNull(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue)
{
	SIRInstruction*	psCompare;
	SIRReference	sOther;

	if ((pcFrom->msJump.uiKind != IRJ_Branch) || (pcFrom->msJump.pcThen == pcFrom->msJump.pcElse) || !pcFrom->msJump.sArgument.IsTemporary())
	{
		return false;
	}
	psCompare = mcDefinitions.Instruction(pcFrom->msJump.sArgument);
	if (!psCompare || ((psCompare->uiOpcode != IRO_Eq) && (psCompare->uiOpcode != IRO_Ne)))
	{
		return false;
	}
	if (psCompare->asArguments[0].Equals(sValue))
	{
		sOther = psCompare->asArguments[1];
	}
	else if (psCompare->asArguments[1].Equals(sValue))
	{
		sOther = psCompare->asArguments[0];
	}
	else
	{
		return false;
	}
	if (!sOther.IsConstant() || (mpcFunction->GetConstant(sOther)->ulliBits != 0))
	{
		return false;
	}
	return (psCompare->uiOpcode == IRO_Ne) ? (pcFrom->msJump.pcThen == pcTo) : (pcFrom->msJump.pcElse == pcTo);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the pointer sValue is never null where pcBlock runs: a global's
// address, a slot, a constant other than 0; tested not null on the way to
// pcBlock (while blocks have one predecessor); a pointer added to one that
// isn't null (C++'s pointer arithmetic stays in an object); a phi whose
// every value isn't null on its way in.  mcDefinitions and the
// predecessors must be found.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NonNullAt(CIRBlock* pcBlock, SIRReference sValue, int iDepth)
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

	pcDefinition = mcDefinitions.Block(sValue);
	psInstruction = mcDefinitions.Instruction(sValue);
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
	psPhi = mcDefinitions.Phi(sValue);
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
bool CIROptimiser::FoldNullTests(void)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sPointer;
	size			ui;
	size			uj;
	bool			bChanged;

	FindDefinitions();
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
// Narrows the signed range [*plliMin, *plliMax] of sValue in pcBlock by the
// branches pcBlock is reached through: while a block has one predecessor,
// that predecessor's branch on a compare of sValue with a constant holds
// (the way to the block), up to IR_RANGE_DEPTH blocks up.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::DominatingRange(CIRBlock* pcBlock, SIRReference sValue, int64* plliMin, int64* plliMax)
{
	CIRBlock*		pcTo;
	CIRBlock*		pcFrom;
	int				iDepth;

	pcTo = pcBlock;
	for (iDepth = 0; (iDepth < IR_RANGE_DEPTH) && (pcTo->mapcPredecessors.NumElements() == 1); iDepth++)
	{
		pcFrom = pcTo->mapcPredecessors.GetPtr(0);
		BranchRange(pcFrom, pcTo, sValue, plliMin, plliMax);
		pcTo = pcFrom;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Narrows the signed range [*plliMin, *plliMax] of sValue by pcFrom's
// branch to pcTo, when it is on a compare of sValue with a constant.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::BranchRange(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue, int64* plliMin, int64* plliMax)
{
	SIRInstruction*	psCompare;
	SIRReference	sConstant;
	uint16			uiOpcode;
	int64			lliConstant;

	psCompare = ((pcFrom->msJump.uiKind == IRJ_Branch) && (pcFrom->msJump.pcThen != pcFrom->msJump.pcElse)) ? mcDefinitions.Instruction(pcFrom->msJump.sArgument) : NULL;
	if (psCompare && (psCompare->asArguments[0].Equals(sValue) || psCompare->asArguments[1].Equals(sValue)))
	{
		uiOpcode = psCompare->uiOpcode;
		sConstant = psCompare->asArguments[1];
		if (psCompare->asArguments[1].Equals(sValue))
		{
			sConstant = psCompare->asArguments[0];
			uiOpcode = IRSwappedComparison(uiOpcode);
		}
		if (sConstant.IsConstant())
		{
			if (pcFrom->msJump.pcElse == pcTo)
			{
				uiOpcode = IRInvertedComparison(uiOpcode);
			}
			lliConstant = mpcFunction->GetSigned(sConstant);
			switch (uiOpcode)
			{
			case IRO_SLt:	*plliMax = (lliConstant - 1 < *plliMax) ? lliConstant - 1 : *plliMax;	break;
			case IRO_SLe:	*plliMax = (lliConstant < *plliMax) ? lliConstant : *plliMax;			break;
			case IRO_SGt:	*plliMin = (lliConstant + 1 > *plliMin) ? lliConstant + 1 : *plliMin;	break;
			case IRO_SGe:	*plliMin = (lliConstant > *plliMin) ? lliConstant : *plliMin;			break;
			case IRO_ULt:
			case IRO_ULe:
				//Below a constant that is positive as signed: 0 up to it.
				if (lliConstant > 0)
				{
					*plliMin = (*plliMin < 0) ? 0 : *plliMin;
					lliConstant -= (uiOpcode == IRO_ULt) ? 1 : 0;
					*plliMax = (lliConstant < *plliMax) ? lliConstant : *plliMax;
				}
				break;
			default:
				break;
			}
		}
	}
}


#define IR_UNSWITCH_INSTRUCTIONS	48		//The most instructions and phis a loop unswitched may have...
#define IR_UNSWITCH_LOOPS			4		//...and the most loops unswitched in a function each time it is optimised.


//////////////////////////////////////////////////////////////////////////
//
// pcBlock's copy when it is one of the loop's blocks (apcBlocks, copied to
// apcCopies), else pcBlock.
//
//////////////////////////////////////////////////////////////////////////
static CIRBlock* IROptimiserCopyOf(CArrayIRBlocks* papcBlocks, CArrayIRBlocks* papcCopies, CIRBlock* pcBlock)
{
	size	ui;

	for (ui = 0; ui < papcBlocks->NumElements(); ui++)
	{
		if (papcBlocks->GetPtr(ui) == pcBlock)
		{
			return papcCopies->GetPtr(ui);
		}
	}
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcBlock is one of the loop's blocks or their copies.
//
//////////////////////////////////////////////////////////////////////////
static bool IROptimiserInEither(CArrayIRBlocks* papcBlocks, CArrayIRBlocks* papcCopies, CIRBlock* pcBlock)
{
	size	ui;

	for (ui = 0; ui < papcBlocks->NumElements(); ui++)
	{
		if ((papcBlocks->GetPtr(ui) == pcBlock) || (papcCopies->GetPtr(ui) == pcBlock))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// pcBlock's phis lose their arguments from pcFrom (an edge taken away).
//
//////////////////////////////////////////////////////////////////////////
static void IROptimiserRemovePhiArguments(CIRBlock* pcBlock, CIRBlock* pcFrom)
{
	size	ui;
	size	uiArgument;
	SIRPhi*	psPhi;

	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		psPhi = pcBlock->masPhis.Get(ui);
		for (uiArgument = psPhi->asArguments.NumElements(); uiArgument > 0; uiArgument--)
		{
			if (psPhi->asArguments.Get(uiArgument - 1)->pcBlock == pcFrom)
			{
				psPhi->asArguments.RemoveAt(uiArgument - 1, true);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The uses of sValue outside the loop's blocks and their copies are what
// pcUpdater gives (sValue or its copy, or a phi of both).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::RewriteOutsideUses(SIRReference sValue, CArrayIRBlocks* papcBlocks, CArrayIRBlocks* papcCopies, CIRSSAUpdater* pcUpdater)
{
	size			ui;
	size			uiIndex;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRPhiArgument*	psArgument;
	SIRInstruction*	psInstruction;
	bool			bInside;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		bInside = IROptimiserInEither(papcBlocks, papcCopies, pcBlock);
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			for (uiArgument = 0; uiArgument < pcBlock->masPhis.Get(uiIndex)->asArguments.NumElements(); uiArgument++)
			{
				psArgument = pcBlock->masPhis.Get(uiIndex)->asArguments.Get(uiArgument);
				if (psArgument->sValue.Equals(sValue) && !IROptimiserInEither(papcBlocks, papcCopies, psArgument->pcBlock))
				{
					psArgument->sValue = pcUpdater->ValueAtEnd(psArgument->pcBlock);
				}
			}
		}
		if (bInside)
		{
			continue;
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				if (psInstruction->asArguments[uiArgument].Equals(sValue))
				{
					psInstruction->asArguments[uiArgument] = pcUpdater->ValueAtStart(pcBlock);
					psInstruction = pcBlock->masInstructions.Get(uiIndex);
				}
			}
		}
		if (pcBlock->msJump.sArgument.Equals(sValue))
		{
			pcBlock->msJump.sArgument = pcUpdater->ValueAtStart(pcBlock);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A loop (of at most IR_UNSWITCH_INSTRUCTIONS) with a branch on a condition
// that is the same each time round is two loops: the loop is copied, the
// branch goes one way in the loop and the other way in the copy, and the
// preheader branches on the condition to one or the other.  The blocks the
// loop leaves to take the copy's values from it too; values used after the
// loop are phis of both (CIRSSAUpdater).  One loop at a time, innermost
// first, at most IR_UNSWITCH_LOOPS each Optimise.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::UnswitchLoops(void)
{
	CArrayIRReferences	asMap;
	CArrayIRBlocks		apcBlocks;
	CArrayIRBlocks		apcCopies;
	CArrayTemplate<SIRUnswitchedValue>	asValues;
	SIRUnswitchedValue*	psValue;
	CIRSSAUpdater		cUpdater;
	SIRInstruction		sCopy;
	SIRLoop*			psLoop;
	CIRBlock*			pcBlock;
	CIRBlock*			pcCopy;
	CIRBlock*			pcSwitch;
	CIRBlock*			pcSwitchCopy;
	CIRBlock*			pcPreheader;
	CIRBlock*			pcSuccessor;
	CIRBlock*			pcThen;
	CIRBlock*			pcElse;
	SIRPhi*				psPhi;
	SIRPhi*				psCopyPhi;
	SIRPhiArgument*		psArgument;
	SIRReference		sCondition;
	SIRReference		sValue;
	size				uiLoop;
	size				ui;
	size				uiIndex;
	size				uiArgument;
	size				uiSize;
	size				uiSuccessor;

	if (miUnswitched >= IR_UNSWITCH_LOOPS)
	{
		return false;
	}
	ComputeLoops();
	for (uiLoop = mcLoops.NumLoops(); uiLoop > 0; uiLoop--)
	{
		psLoop = mcLoops.GetLoop(uiLoop - 1);
		uiSize = 0;
		pcSwitch = NULL;
		for (ui = 0; ui < mcLoops.NumBlocks(uiLoop - 1); ui++)
		{
			pcBlock = mcLoops.GetBlock(uiLoop - 1, ui);
			uiSize += pcBlock->masInstructions.NumElements() + pcBlock->masPhis.NumElements();
			if (!pcSwitch && (pcBlock->msJump.uiKind == IRJ_Branch) && (pcBlock->msJump.pcThen != pcBlock->msJump.pcElse) &&
				pcBlock->msJump.sArgument.IsTemporary() && mcLoops.IsInvariant(uiLoop - 1, pcBlock->msJump.sArgument, &mcDefinitions))
			{
				pcSwitch = pcBlock;
			}
		}
		if (!pcSwitch || (uiSize > IR_UNSWITCH_INSTRUCTIONS))
		{
			continue;
		}

		pcPreheader = psLoop->pcPreheader;
		if (!pcPreheader)
		{
			pcPreheader = psLoop->pcEntering ? AddPreheader(uiLoop - 1) : AddMergedPreheader(uiLoop - 1);
			psLoop = mcLoops.GetLoop(uiLoop - 1);
		}

		//The copy's blocks and a temporary for each value the loop defines.
		apcBlocks.Init();
		apcCopies.Init();
		asMap.Init();
		asMap.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
		for (ui = 0; ui < mcLoops.NumBlocks(uiLoop - 1); ui++)
		{
			pcBlock = mcLoops.GetBlock(uiLoop - 1, ui);
			apcBlocks.Add(pcBlock);
			apcCopies.Add(mpcFunction->AddBlock());
			for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
			{
				sValue = pcBlock->masPhis.Get(uiIndex)->sResult;
				*asMap.Get(sValue.Index()) = mpcFunction->AddTemporary(mpcFunction->GetTemporaryType(sValue));
			}
			for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
			{
				sValue = pcBlock->masInstructions.Get(uiIndex)->sResult;
				if (sValue.IsTemporary())
				{
					*asMap.Get(sValue.Index()) = mpcFunction->AddTemporary(mpcFunction->GetTemporaryType(sValue));
				}
			}
		}

		//The copies: their phis, instructions and jumps taking the copies' values and blocks.
		for (ui = 0; ui < apcBlocks.NumElements(); ui++)
		{
			pcBlock = apcBlocks.GetPtr(ui);
			pcCopy = apcCopies.GetPtr(ui);
			for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
			{
				psPhi = pcBlock->masPhis.Get(uiIndex);
				psCopyPhi = pcCopy->AddPhi(psPhi->sType, IROptimiserMapped(&asMap, psPhi->sResult));
				for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
				{
					psArgument = psPhi->asArguments.Get(uiArgument);
					pcCopy->AddPhiArgument(psCopyPhi, IROptimiserCopyOf(&apcBlocks, &apcCopies, psArgument->pcBlock), IROptimiserMapped(&asMap, psArgument->sValue));
				}
			}
			for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
			{
				sCopy = *pcBlock->masInstructions.Get(uiIndex);
				sCopy.asArguments[0] = IROptimiserMapped(&asMap, sCopy.asArguments[0]);
				sCopy.asArguments[1] = IROptimiserMapped(&asMap, sCopy.asArguments[1]);
				sCopy.sResult = IROptimiserMapped(&asMap, sCopy.sResult);
				pcCopy->masInstructions.Add(&sCopy);
			}
			switch (pcBlock->msJump.uiKind)
			{
			case IRJ_Jump:
				pcCopy->Jump(IROptimiserCopyOf(&apcBlocks, &apcCopies, pcBlock->msJump.pcThen));
				break;
			case IRJ_Branch:
				pcCopy->Branch(IROptimiserMapped(&asMap, pcBlock->msJump.sArgument), IROptimiserCopyOf(&apcBlocks, &apcCopies, pcBlock->msJump.pcThen), IROptimiserCopyOf(&apcBlocks, &apcCopies, pcBlock->msJump.pcElse));
				break;
			case IRJ_Return:
				if (pcBlock->msJump.sType.IsNone())
				{
					pcCopy->Return();
				}
				else
				{
					pcCopy->Return(pcBlock->msJump.sType, IROptimiserMapped(&asMap, pcBlock->msJump.sArgument));
				}
				break;
			default:
				pcCopy->Unreachable();
				break;
			}

			//The blocks the loop leaves to take the copy's values from the copy.
			for (uiSuccessor = 0; uiSuccessor < pcBlock->NumSuccessors(); uiSuccessor++)
			{
				pcSuccessor = pcBlock->GetSuccessor(uiSuccessor);
				if (mcLoops.Contains(uiLoop - 1, pcSuccessor) || ((uiSuccessor == 1) && (pcSuccessor == pcBlock->GetSuccessor(0))))
				{
					continue;
				}
				for (uiIndex = 0; uiIndex < pcSuccessor->masPhis.NumElements(); uiIndex++)
				{
					psPhi = pcSuccessor->masPhis.Get(uiIndex);
					psArgument = pcSuccessor->FindPhiArgument(psPhi, pcBlock);
					if (psArgument)
					{
						sValue = IROptimiserMapped(&asMap, psArgument->sValue);
						pcSuccessor->AddPhiArgument(psPhi, pcCopy, sValue);
					}
				}
			}
		}

		//The branch goes one way in the loop and the other in the copy; the preheader chooses.
		pcSwitchCopy = IROptimiserCopyOf(&apcBlocks, &apcCopies, pcSwitch);
		sCondition = pcSwitch->msJump.sArgument;
		pcThen = pcSwitch->msJump.pcThen;
		pcElse = pcSwitch->msJump.pcElse;
		pcSwitch->Jump(pcThen);
		IROptimiserRemovePhiArguments(pcElse, pcSwitch);
		pcSwitchCopy->Jump(IROptimiserCopyOf(&apcBlocks, &apcCopies, pcElse));
		IROptimiserRemovePhiArguments(IROptimiserCopyOf(&apcBlocks, &apcCopies, pcThen), pcSwitchCopy);
		pcPreheader->Branch(sCondition, psLoop->pcHead, IROptimiserCopyOf(&apcBlocks, &apcCopies, psLoop->pcHead));
		mpcFunction->Renumber();
		mpcFunction->ComputePredecessors();

		//The values the loop defines (with their blocks, and whether each block and its copy can still be reached),
		//before the blocks the branch no longer reaches go: an unreachable cycle would send the updater round it.
		mcDominators.Compute(mpcFunction);
		asValues.Init();
		for (ui = 0; ui < apcBlocks.NumElements(); ui++)
		{
			pcBlock = apcBlocks.GetPtr(ui);
			for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements() + pcBlock->masInstructions.NumElements(); uiIndex++)
			{
				sValue = (uiIndex < pcBlock->masPhis.NumElements()) ? pcBlock->masPhis.Get(uiIndex)->sResult : pcBlock->masInstructions.Get(uiIndex - pcBlock->masPhis.NumElements())->sResult;
				if (sValue.IsTemporary())
				{
					psValue = asValues.Add();
					psValue->sValue = sValue;
					psValue->uiBlock = ui;
					psValue->bReached = mcDominators.IsReachable(pcBlock);
					psValue->bCopyReached = mcDominators.IsReachable(apcCopies.GetPtr(ui));
				}
			}
		}
		RemoveUnreachable();
		mpcFunction->Renumber();
		mpcFunction->ComputePredecessors();

		//Values used after the loop come from the loop or the copy.
		cUpdater.Init(mpcFunction);
		for (ui = 0; ui < asValues.NumElements(); ui++)
		{
			psValue = asValues.Get(ui);
			if (psValue->bReached || psValue->bCopyReached)
			{
				cUpdater.Begin(mpcFunction->GetTemporaryType(psValue->sValue));
				if (psValue->bReached)
				{
					cUpdater.Define(apcBlocks.GetPtr(psValue->uiBlock), psValue->sValue);
				}
				if (psValue->bCopyReached)
				{
					cUpdater.Define(apcCopies.GetPtr(psValue->uiBlock), IROptimiserMapped(&asMap, psValue->sValue));
				}
				RewriteOutsideUses(psValue->sValue, &apcBlocks, &apcCopies, &cUpdater);
			}
		}
		cUpdater.Kill();
		asValues.Kill();
		asMap.Kill();
		apcCopies.Kill();
		apcBlocks.Kill();
		miUnswitched++;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// In a loop, ptradd p, (a + b) with p and a the same each time round (and
// the add only used there) is ptradd (ptradd p, a), b: the inner ptradd is
// then hoisted out of the loop (HoistInvariants), leaving one add a time
// round instead of two.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SplitLoopAddresses(void)
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
		iLoop = mcLoops.Innermost(pcBlock);
		if (iLoop < 0)
		{
			continue;
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psAddress = pcBlock->masInstructions.Get(uiInstruction);
			if ((psAddress->uiOpcode != IRO_PtrAdd) || (psAddress->uiFlags != 0) || !psAddress->asArguments[1].IsTemporary() ||
				!mcLoops.IsInvariant((size)iLoop, psAddress->asArguments[0], &mcDefinitions) || (mcDefinitions.NumUses(psAddress->asArguments[1]) != 1))
			{
				continue;
			}
			psAdd = mcDefinitions.Instruction(psAddress->asArguments[1]);
			if (!psAdd || (psAdd->uiOpcode != IRO_Add))
			{
				continue;
			}
			for (i = 0; i < 2; i++)
			{
				if (!psAdd->asArguments[i].IsConstant() && mcLoops.IsInvariant((size)iLoop, psAdd->asArguments[i], &mcDefinitions) &&
					!mcLoops.IsInvariant((size)iLoop, psAdd->asArguments[1 - i], &mcDefinitions))
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
	}
	asFound.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the branches pcBlock is reached through keep the integer sValue 0
// or more (DominatingRange).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NotNegative(CIRBlock* pcBlock, SIRReference sValue)
{
	int		iBits;
	int64	lliMin;
	int64	lliMax;

	iBits = mpcFunction->TypeOf(sValue).IntegerBits();
	if ((iBits < 2) || (iBits > 32) || sValue.IsConstant())
	{
		return false;
	}
	lliMin = -((int64)1 << (iBits - 1));
	lliMax = ((int64)1 << (iBits - 1)) - 1;
	DominatingRange(pcBlock, sValue, &lliMin, &lliMax);
	return lliMin >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A mul of an extended i8 or i16 value by a constant whose product fits 16
// bits unsigned (the value 0 or more, its range from the branches the block
// is reached through, DominatingRange, or a zext's bits) is a 16 bit mul,
// zero extended: (long)y * 320 with y from 0 to 199 is a 16 bit y * 320.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowMultiplies(void)
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
	int64								lliMin;
	int64								lliMax;
	int									iBits;
	size								ui;
	size								uiInstruction;
	bool								bChanged;

	//Found first: the inserts move the definitions DominatingRange reads.
	FindDefinitions();
	mpcFunction->ComputePredecessors();
	sShort = IRType(IRTK_I16);
	asFound.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sType = psInstruction->sType;
			if ((psInstruction->uiOpcode != IRO_Mul) || !psInstruction->sResult.IsTemporary() || !sType.IsInteger() || (sType.IntegerBits() <= 16) ||
				!psInstruction->asArguments[1].IsConstant())
			{
				continue;
			}
			ulliConstant = IRTruncate(sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
			psExtension = mcDefinitions.Instruction(psInstruction->asArguments[0]);
			if ((ulliConstant == 0) || (ulliConstant > 0xFFFF) || !psExtension || ((psExtension->uiOpcode != IRO_SExt) && (psExtension->uiOpcode != IRO_ZExt)))
			{
				continue;
			}
			sNarrow = psExtension->asArguments[0];
			iBits = mpcFunction->TypeOf(sNarrow).IntegerBits();
			if ((iBits != 8) && (iBits != 16))
			{
				continue;
			}
			//The compares are of the narrow value as it is (a signed compare sees it signed); a zext of one that may be
			//negative is up to all its bits.
			lliMin = -((int64)1 << (iBits - 1));
			lliMax = ((int64)1 << (iBits - 1)) - 1;
			DominatingRange(pcBlock, sNarrow, &lliMin, &lliMax);
			if ((lliMin < 0) && (psExtension->uiOpcode == IRO_ZExt))
			{
				lliMin = 0;
				lliMax = ((int64)1 << iBits) - 1;
			}
			if ((lliMin < 0) || ((uint64)lliMax * ulliConstant > 0xFFFF))
			{
				continue;
			}
			psFound = asFound.Add();
			psFound->pcBlock = pcBlock;
			psFound->uiInstruction = uiInstruction;
			psFound->sNarrow = sNarrow;
		}
	}

	//Last first, so each instruction's index is still where it was found.
	bChanged = asFound.NumElements() > 0;
	for (ui = asFound.NumElements(); ui > 0; ui--)
	{
		psFound = asFound.Get(ui - 1);
		pcBlock = psFound->pcBlock;
		uiInstruction = psFound->uiInstruction;
		ulliConstant = mpcFunction->GetConstant(pcBlock->masInstructions.Get(uiInstruction)->asArguments[1])->ulliBits;
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
bool CIROptimiser::NarrowTruncations(void)
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

	FindDefinitions();
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
			psWide = (sInstruction.uiOpcode == IRO_Trunc) && sInstruction.sResult.IsTemporary() && sType.IsInteger() && !sType.Is(IRTK_I1) ? mcDefinitions.Instruction(sInstruction.asArguments[0]) : NULL;
			if (psWide && (mcDefinitions.NumUses(sInstruction.asArguments[0]) == 1) && IROptimiserNarrows(mpcFunction, psWide, sType.IntegerBits()) &&
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


//////////////////////////////////////////////////////////////////////////
//
// The counter of a loop (ReplaceLoopIdioms) from its head pcHead to its
// latch pcLatch: i = phi [init, i + 1] in the head, the latch going round
// while i + 1 < n (signed or unsigned) or i + 1 != n with n not changed by
// the loop.  The way in must have tested init the same way (as RotateLoops
// leaves it), or init and n be constants that pass it, so the loop goes
// round n - init times.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::MatchIdiomCounter(size uiLoop, CIRBlock* pcHead, CIRBlock* pcLatch, SIRIdiomCounter* psCounter)
{
	SIRInstruction*	psCompare;
	uint16			uiOpcode;

	psCompare = mcDefinitions.Instruction(pcLatch->msJump.sArgument);
	if ((pcLatch->msJump.uiKind != IRJ_Branch) || (psCompare == NULL) || !IsIRIntegerComparison(psCompare->uiOpcode) || !mcLoops.GetLoop(uiLoop)->pcEntering)
	{
		return false;
	}
	uiOpcode = (pcLatch->msJump.pcThen == pcHead) ? psCompare->uiOpcode : IRInvertedComparison(psCompare->uiOpcode);
	if (((uiOpcode != IRO_SLt) && (uiOpcode != IRO_ULt) && (uiOpcode != IRO_Ne)) || !mcLoops.IsInvariant(uiLoop, psCompare->asArguments[1], &mcDefinitions))
	{
		return false;
	}
	psCounter->uiOpcode = uiOpcode;
	psCounter->sNext = psCompare->asArguments[0];
	psCounter->sBound = psCompare->asArguments[1];
	if (!MatchIdiomStep(uiLoop, pcHead, pcLatch, psCounter->sNext, psCounter) || !psCounter->sNext.Equals(psCompare->asArguments[0]))
	{
		return false;
	}
	return GuardedBy(uiLoop, uiOpcode, psCounter->sInit, psCounter->sBound);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether loop uiLoop is only entered when sLeft uiOpcode sRight (an
// integer comparison): the way in branches on it (as RotateLoops leaves
// it), past blocks of only a jump (preheaders), or both are constants for
// which it holds.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::GuardedBy(size uiLoop, uint16 uiOpcode, SIRReference sLeft, SIRReference sRight)
{
	SIRInstruction*	psGuard;
	CIRBlock*		pcEntering;
	CIRBlock*		pcTarget;
	SIRType			sType;
	int				iSteps;
	uint64			ulliLeft;
	uint64			ulliRight;
	int64			lliLeft;
	int64			lliRight;

	pcEntering = mcLoops.GetLoop(uiLoop)->pcEntering;
	if (!pcEntering)
	{
		return false;
	}
	pcTarget = mcLoops.GetLoop(uiLoop)->pcHead;
	for (iSteps = 0; (iSteps < 2) && (pcEntering->msJump.uiKind == IRJ_Jump) && (pcEntering->masPhis.NumElements() == 0) &&
					 (pcEntering->mapcPredecessors.NumElements() == 1); iSteps++)
	{
		pcTarget = pcEntering;
		pcEntering = pcEntering->mapcPredecessors.GetPtr(0);
	}
	if ((pcEntering->msJump.uiKind == IRJ_Branch) && (pcEntering->msJump.pcThen != pcEntering->msJump.pcElse))
	{
		psGuard = mcDefinitions.Instruction(pcEntering->msJump.sArgument);
		if ((psGuard == NULL) || !IsIRIntegerComparison(psGuard->uiOpcode) || ((pcEntering->msJump.pcThen != pcTarget) && (pcEntering->msJump.pcElse != pcTarget)))
		{
			return false;
		}
		return (((pcEntering->msJump.pcThen == pcTarget) ? psGuard->uiOpcode : IRInvertedComparison(psGuard->uiOpcode)) == uiOpcode) &&
			   psGuard->asArguments[0].Equals(sLeft) && psGuard->asArguments[1].Equals(sRight);
	}
	if (!sLeft.IsConstant() || !sRight.IsConstant())
	{
		return false;
	}
	sType = mpcFunction->TypeOf(sLeft);
	ulliLeft = IRTruncate(sType, mpcFunction->GetConstant(sLeft)->ulliBits);
	ulliRight = IRTruncate(sType, mpcFunction->GetConstant(sRight)->ulliBits);
	lliLeft = IRSignExtend(sType, ulliLeft);
	lliRight = IRSignExtend(sType, ulliRight);
	switch (uiOpcode)
	{
	case IRO_SLt:	return lliLeft < lliRight;
	case IRO_SLe:	return lliLeft <= lliRight;
	case IRO_SGt:	return lliLeft > lliRight;
	case IRO_SGe:	return lliLeft >= lliRight;
	case IRO_ULt:	return ulliLeft < ulliRight;
	case IRO_ULe:	return ulliLeft <= ulliRight;
	case IRO_UGt:	return ulliLeft > ulliRight;
	case IRO_UGe:	return ulliLeft >= ulliRight;
	case IRO_Eq:	return ulliLeft == ulliRight;
	default:		return ulliLeft != ulliRight;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sValue is a counter stepped by 1 round the loop, or its next
// value: a phi i of the head taking [init] on the way in and i + 1 from the
// latch (sValue i or i + 1).  psCounter gets the phi, i + 1 and init.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::MatchIdiomStep(size uiLoop, CIRBlock* pcHead, CIRBlock* pcLatch, SIRReference sValue, SIRIdiomCounter* psCounter)
{
	SIRInstruction*	psNext;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	size			ui;

	psNext = mcDefinitions.Instruction(sValue);
	if (psNext && (psNext->uiOpcode == IRO_Add) && psNext->asArguments[1].IsConstant() && (mpcFunction->GetSigned(psNext->asArguments[1]) == 1))
	{
		psCounter->sCounter = psNext->asArguments[0];
		psCounter->sNext = sValue;
	}
	else
	{
		psCounter->sCounter = sValue;
		psCounter->sNext = IRNone();
	}
	psPhi = mcDefinitions.Phi(psCounter->sCounter);
	if ((psPhi == NULL) || (mcDefinitions.Block(psCounter->sCounter) != pcHead) || (psPhi->asArguments.NumElements() != 2))
	{
		return false;
	}
	psCounter->sInit = IRNone();
	for (ui = 0; ui < 2; ui++)
	{
		psArgument = psPhi->asArguments.Get(ui);
		if (psArgument->pcBlock == pcLatch)
		{
			psNext = mcDefinitions.Instruction(psArgument->sValue);
			if ((psNext == NULL) || (psNext->uiOpcode != IRO_Add) || !psNext->asArguments[0].Equals(psCounter->sCounter) || !psNext->asArguments[1].IsConstant() ||
				(mpcFunction->GetSigned(psNext->asArguments[1]) != 1) || (psCounter->sNext.IsNone() ? false : !psCounter->sNext.Equals(psArgument->sValue)))
			{
				return false;
			}
			psCounter->sNext = psArgument->sValue;
		}
		else if (mcLoops.Contains(uiLoop, psArgument->pcBlock))
		{
			return false;
		}
		else
		{
			psCounter->sInit = psArgument->sValue;
		}
	}
	return !psCounter->sInit.IsNone();
}


//////////////////////////////////////////////////////////////////////////
//
// An address the loop steps lliBytes a time round: a pointer phi of the
// head stepped by lliBytes from the latch (or that step, its next value),
// or base + i * lliBytes with the
// base not changed by the loop and i the counter or its next value
// (extended to the pointer's offset the way it can't wrap: sext for a
// signed bound or none, zext for an unsigned bound).  A counter that
// psCounter doesn't have yet (no sCounter) is the index's.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::MatchIdiomAddress(size uiLoop, CIRBlock* pcHead, CIRBlock* pcLatch, SIRReference sAddress, SIRIdiomCounter* psCounter, int64 lliBytes, SIRIdiomAddress* psAddress)
{
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRInstruction*	psAdd;
	SIRInstruction*	psIndex;
	SIRReference	sIndex;
	SIRIdiomCounter	sIndexCounter;
	bool			bEntering;
	size			ui;

	psAddress->sPhiStart = IRNone();
	psAddress->sBase = IRNone();
	psAddress->lliScale = 1;
	psAddress->uiExtension = IRO_None;
	psAddress->bNext = false;
	if (!sAddress.IsTemporary() || !mpcFunction->TypeOf(sAddress).Equals(IRType(IRTK_Pointer)))
	{
		return false;
	}

	//A pointer phi: [way in, start], [latch, ptradd phi, lliBytes]; or that ptradd (the phi's next value).
	psAdd = mcDefinitions.Instruction(sAddress);
	if (psAdd && (psAdd->uiOpcode == IRO_PtrAdd) && psAdd->asArguments[1].IsConstant() && (mpcFunction->GetSigned(psAdd->asArguments[1]) == lliBytes) &&
		mcDefinitions.Phi(psAdd->asArguments[0]) && (mcDefinitions.Block(psAdd->asArguments[0]) == pcHead))
	{
		psAddress->bNext = true;
		psAddress->lliScale = lliBytes;
		sAddress = psAdd->asArguments[0];
	}
	psPhi = mcDefinitions.Phi(sAddress);
	if (psPhi)
	{
		if ((mcDefinitions.Block(sAddress) != pcHead) || (psPhi->asArguments.NumElements() != 2))
		{
			return false;
		}
		bEntering = false;
		for (ui = 0; ui < 2; ui++)
		{
			psArgument = psPhi->asArguments.Get(ui);
			if (psArgument->pcBlock == pcLatch)
			{
				psAdd = mcDefinitions.Instruction(psArgument->sValue);
				if ((psAdd == NULL) || (psAdd->uiOpcode != IRO_PtrAdd) || !psAdd->asArguments[0].Equals(sAddress) || !psAdd->asArguments[1].IsConstant() ||
					(mpcFunction->GetSigned(psAdd->asArguments[1]) != lliBytes))
				{
					return false;
				}
			}
			else if (!mcLoops.Contains(uiLoop, psArgument->pcBlock))
			{
				bEntering = true;
				psAddress->sPhiStart = psArgument->sValue;
			}
		}
		return bEntering;
	}

	//base + index.
	psAdd = mcDefinitions.Instruction(sAddress);
	if ((psAdd == NULL) || (psAdd->uiOpcode != IRO_PtrAdd) || !mcLoops.Contains(uiLoop, mcDefinitions.Block(sAddress)) ||
		!mcLoops.IsInvariant(uiLoop, psAdd->asArguments[0], &mcDefinitions))
	{
		return false;
	}
	psAddress->sBase = psAdd->asArguments[0];
	sIndex = psAdd->asArguments[1];
	psIndex = mcDefinitions.Instruction(sIndex);
	if (psIndex && ((psIndex->uiOpcode == IRO_Shl) || (psIndex->uiOpcode == IRO_Mul)) && psIndex->asArguments[1].IsConstant())
	{
		psAddress->lliScale = (psIndex->uiOpcode == IRO_Shl) ? ((int64)1 << (mpcFunction->GetSigned(psIndex->asArguments[1]) & 31)) : mpcFunction->GetSigned(psIndex->asArguments[1]);
		sIndex = psIndex->asArguments[0];
		psIndex = mcDefinitions.Instruction(sIndex);
	}
	if (psIndex && ((psIndex->uiOpcode == IRO_SExt) || (psIndex->uiOpcode == IRO_ZExt)))
	{
		psAddress->uiExtension = psIndex->uiOpcode;
		sIndex = psIndex->asArguments[0];
	}
	if (psAddress->lliScale != lliBytes)
	{
		return false;
	}

	//The counter, or its next value.
	if (psCounter->sCounter.IsNone())
	{
		if (!MatchIdiomStep(uiLoop, pcHead, pcLatch, sIndex, &sIndexCounter))
		{
			return false;
		}
		sIndexCounter.sBound = IRNone();
		sIndexCounter.uiOpcode = IRO_SLt;
		*psCounter = sIndexCounter;
	}
	if (sIndex.Equals(psCounter->sNext))
	{
		psAddress->bNext = true;
	}
	else if (!sIndex.Equals(psCounter->sCounter))
	{
		return false;
	}
	if (psAddress->uiExtension == IRO_None)
	{
		return mpcFunction->TypeOf(psCounter->sCounter).Is(IRTK_I32);
	}
	return ((psAddress->uiExtension == IRO_SExt) && (psCounter->uiOpcode == IRO_SLt)) || ((psAddress->uiExtension == IRO_ZExt) && (psCounter->uiOpcode == IRO_ULt));
}


//////////////////////////////////////////////////////////////////////////
//
// The address a loop's first time round uses, worked out at the end of
// pcBlock.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::IdiomStart(CIRBlock* pcBlock, SIRIdiomAddress* psAddress, SIRIdiomCounter* psCounter)
{
	SIRReference	sOffset;
	SIRType			sOffsetType;

	sOffsetType = IRType(IRTK_I32);
	if (!psAddress->sPhiStart.IsNone())
	{
		if (psAddress->bNext)
		{
			return mpcFunction->Add(pcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), psAddress->sPhiStart, mpcFunction->AddInteger(sOffsetType, psAddress->lliScale));
		}
		return psAddress->sPhiStart;
	}
	sOffset = psCounter->sInit;
	if (psAddress->bNext)
	{
		sOffset = mpcFunction->Add(pcBlock, IRO_Add, mpcFunction->TypeOf(sOffset), sOffset, mpcFunction->AddInteger(mpcFunction->TypeOf(sOffset), 1));
	}
	if (psAddress->uiExtension != IRO_None)
	{
		sOffset = mpcFunction->Add(pcBlock, psAddress->uiExtension, sOffsetType, sOffset);
	}
	if (psAddress->lliScale != 1)
	{
		sOffset = mpcFunction->Add(pcBlock, IRO_Mul, sOffsetType, sOffset, mpcFunction->AddInteger(sOffsetType, psAddress->lliScale));
	}
	return mpcFunction->Add(pcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), psAddress->sBase, sOffset);
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes a loop of n - init times round, lliBytes each, fills or copies
// (a size_t), worked out at the end of pcBlock.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::IdiomBytes(CIRBlock* pcBlock, SIRIdiomCounter* psCounter, int64 lliBytes)
{
	SIRType			sType;
	SIRType			sSize;
	SIRReference	sTimes;

	sType = mpcFunction->TypeOf(psCounter->sInit);
	sSize = IRType(IRTK_I32);
	sTimes = mpcFunction->Add(pcBlock, IRO_Sub, sType, psCounter->sBound, psCounter->sInit);
	if (!sType.Is(IRTK_I32))
	{
		sTimes = mpcFunction->Add(pcBlock, IRO_ZExt, sSize, sTimes);
	}
	if (lliBytes != 1)
	{
		sTimes = mpcFunction->Add(pcBlock, IRO_Mul, sSize, sTimes, mpcFunction->AddInteger(sSize, lliBytes));
	}
	return sTimes;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcBlock does nothing with an effect but iLoads loads, none
// volatile: they go in apsLoads in order.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::OnlyLoads(CIRBlock* pcBlock, int iLoads, SIRInstruction** apsLoads)
{
	SIRInstruction*	psInstruction;
	size			ui;
	int				iFound;

	iFound = 0;
	for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		if (psInstruction->uiOpcode == IRO_Load)
		{
			if ((iFound == iLoads) || (psInstruction->uiFlags & IRF_Volatile))
			{
				return false;
			}
			apsLoads[iFound] = psInstruction;
			iFound++;
		}
		else if (!IsIRPure(psInstruction->uiOpcode))
		{
			return false;
		}
	}
	return iFound == iLoads;
}


//////////////////////////////////////////////////////////////////////////
//
// A call of the library's szName (memset, strlen ...) at the end of pcBlock
// with the arguments that aren't IRNone, worked out before it.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::LibraryCall(CIRBlock* pcBlock, const char* szName, SIRType sResultType, SIRReference sFirst, SIRType sFirstType, SIRReference sSecond, SIRType sSecondType, SIRReference sThird, SIRType sThirdType)
{
	mpcFunction->Argument(pcBlock, sFirstType, sFirst);
	if (!sSecond.IsNone())
	{
		mpcFunction->Argument(pcBlock, sSecondType, sSecond);
	}
	if (!sThird.IsNone())
	{
		mpcFunction->Argument(pcBlock, sThirdType, sThird);
	}
	return mpcFunction->Call(pcBlock, sResultType, IRReference(IRRK_Symbol, mpcModule->AddSymbol(szName)));
}


//////////////////////////////////////////////////////////////////////////
//
// Loop idioms: a loop of one block (as RotateLoops leaves a simple loop)
// whose values are only used in it, that does nothing but store a value
// the loop doesn't change through an address stepped an element at a time
// (MatchIdiomAddress), counted n - init times (MatchIdiomCounter), is a
// memset on the way in: of bytes, or of wider elements whose bytes are all
// the same constant.  One that only stores what it loads, element for
// element, is a memmove on the way in when the destination is at or below
// the source or past its end (as the loop would copy), else the loop.  A
// known count of IR_IDIOM_MIN_BYTES or fewer stays a loop.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceLoopIdioms(void)
{
	SIRIdiomCounter		sCounter;
	SIRIdiomAddress		sDestination;
	SIRIdiomAddress		sSource;
	SIRInstruction*		psStore;
	SIRInstruction*		psLoad;
	SIRInstruction*		psInstruction;
	SIRInstruction		sStore;
	SIRLoop*			psLoop;
	CIRBlock*			pcLoop;
	CIRBlock*			pcExit;
	CIRBlock*			pcPreheader;
	CIRBlock*			pcCall;
	SIRPhiArgument*		psArgument;
	SIRReference		sDestinationStart;
	SIRReference		sSourceStart;
	SIRReference		sBytes;
	SIRReference		sValue;
	SIRReference		sTest;
	SIRType				sPointer;
	uint64				ulliValue;
	int64				lliBytes;
	int64				lliTimes;
	size				uiLoop;
	size				ui;
	int					iStores;
	int					iLoads;
	bool				bChanged;
	bool				bSame;
	char*				szName;

	szName = mpcModule->GetSymbolName(mpcFunction->muiSymbol);
	if (msLibraryInt.IsNone() || (strcmp(szName, "_memset") == 0) || (strcmp(szName, "_memmove") == 0) || (strcmp(szName, "_memcpy") == 0) ||
		(strcmp(szName, "_memcmp") == 0) || (strcmp(szName, "_strlen") == 0) || (strcmp(szName, "_memchr") == 0) || (strcmp(szName, "_strchr") == 0))
	{
		//The library's own loops stay loops.
		return false;
	}
	sPointer = IRType(IRTK_Pointer);
	bChanged = false;
	ComputeLoops();
	mpcFunction->CountUses(&mauiUses);
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		if (ReplaceStrlen(uiLoop) || ReplaceMemcmp(uiLoop) || ReplaceMemchr(uiLoop) || ReplaceStrchr(uiLoop))
		{
			bChanged = true;
			ComputeLoops();
			mpcFunction->CountUses(&mauiUses);
			uiLoop = (size)-1;
			continue;
		}
		psLoop = mcLoops.GetLoop(uiLoop);
		pcLoop = psLoop->pcHead;
		if ((mcLoops.NumBlocks(uiLoop) != 1) || (mcLoops.NumLatches(uiLoop) != 1) || !psLoop->pcEntering || (pcLoop->msJump.uiKind != IRJ_Branch) ||
			!OnlyUsedInBlock(pcLoop))
		{
			continue;
		}
		pcExit = (pcLoop->msJump.pcThen == pcLoop) ? pcLoop->msJump.pcElse : pcLoop->msJump.pcThen;
		if (pcExit == pcLoop)
		{
			continue;
		}

		//One store (and a load for a copy); everything else pure.
		psStore = NULL;
		psLoad = NULL;
		iStores = 0;
		iLoads = 0;
		for (ui = 0; ui < pcLoop->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcLoop->masInstructions.Get(ui);
			if (psInstruction->uiOpcode == IRO_Store)
			{
				psStore = psInstruction;
				iStores++;
			}
			else if (psInstruction->uiOpcode == IRO_Load)
			{
				psLoad = psInstruction;
				iLoads++;
			}
			else if (!IsIRPure(psInstruction->uiOpcode))
			{
				iStores = 2;
			}
		}
		if ((iStores != 1) || (iLoads > 1) || (psStore->uiFlags & IRF_Volatile) || (psLoad && (psLoad->uiFlags & IRF_Volatile)) ||
			!MatchIdiomCounter(uiLoop, pcLoop, pcLoop, &sCounter))
		{
			continue;
		}
		lliBytes = mpcModule->GetSize(psStore->sType);
		if (!MatchIdiomAddress(uiLoop, pcLoop, pcLoop, psStore->asArguments[1], &sCounter, lliBytes, &sDestination))
		{
			continue;
		}
		if (psLoad)
		{
			if (!psLoad->sResult.Equals(psStore->asArguments[0]) || !psLoad->sType.Equals(psStore->sType) || (*mauiUses.Get(psLoad->sResult.Index()) != 1) ||
				!MatchIdiomAddress(uiLoop, pcLoop, pcLoop, psLoad->asArguments[0], &sCounter, lliBytes, &sSource))
			{
				continue;
			}
		}
		else
		{
			//The same byte in every byte of each element.
			sValue = psStore->asArguments[0];
			if (!mcLoops.IsInvariant(uiLoop, sValue, &mcDefinitions) || !psStore->sType.IsInteger())
			{
				continue;
			}
			if (lliBytes != 1)
			{
				if (!sValue.IsConstant())
				{
					continue;
				}
				ulliValue = IRTruncate(psStore->sType, mpcFunction->GetConstant(sValue)->ulliBits);
				bSame = true;
				for (ui = 1; ui < (size)lliBytes; ui++)
				{
					bSame &= ((ulliValue >> (8 * ui)) & 0xFF) == (ulliValue & 0xFF);
				}
				if (!bSame)
				{
					continue;
				}
			}
		}
		if (sCounter.sInit.IsConstant() && sCounter.sBound.IsConstant())
		{
			lliTimes = mpcFunction->GetSigned(sCounter.sBound) - mpcFunction->GetSigned(sCounter.sInit);
			if ((lliTimes > 0) && (lliTimes * lliBytes <= IR_IDIOM_MIN_BYTES))
			{
				continue;
			}
		}

		//The call on the way in.
		sStore = *psStore;
		pcPreheader = AddPreheader(uiLoop);
		sDestinationStart = IdiomStart(pcPreheader, &sDestination, &sCounter);
		sBytes = IdiomBytes(pcPreheader, &sCounter, lliBytes);
		if (!psLoad)
		{
			sValue = sStore.asArguments[0];
			if (sValue.IsConstant())
			{
				sValue = mpcFunction->AddConstant(msLibraryInt, mpcFunction->GetConstant(sValue)->ulliBits & 0xFF);
			}
			else if (!mpcFunction->TypeOf(sValue).Equals(msLibraryInt))
			{
				sValue = mpcFunction->Add(pcPreheader, IRO_ZExt, msLibraryInt, sValue);
			}
			LibraryCall(pcPreheader, "_memset", sPointer, sDestinationStart, sPointer, sValue, msLibraryInt, sBytes, IRType(IRTK_I32));
			pcCall = pcPreheader;
			RemovePhiArguments(pcLoop, pcPreheader);
			pcPreheader->Jump(pcExit);
		}
		else
		{
			sSourceStart = IdiomStart(pcPreheader, &sSource, &sCounter);
			sTest = mpcFunction->Add(pcPreheader, IRO_ULe, sPointer, sDestinationStart, sSourceStart);
			sValue = mpcFunction->Add(pcPreheader, IRO_PtrAdd, sPointer, sSourceStart, sBytes);
			sValue = mpcFunction->Add(pcPreheader, IRO_UGe, sPointer, sDestinationStart, sValue);
			sTest = mpcFunction->Add(pcPreheader, IRO_Or, IRType(IRTK_I1), sTest, sValue);
			pcCall = mpcFunction->AddBlock();
			LibraryCall(pcCall, "_memmove", sPointer, sDestinationStart, sPointer, sSourceStart, sPointer, sBytes, IRType(IRTK_I32));
			pcCall->Jump(pcExit);
			pcPreheader->Branch(sTest, pcCall, pcLoop);
		}

		//The exit's phis take from the call's block what they took from the loop (none of the loop's own values).
		for (ui = 0; ui < pcExit->masPhis.NumElements(); ui++)
		{
			psArgument = pcExit->FindPhiArgument(pcExit->masPhis.Get(ui), pcLoop);
			if (psArgument)
			{
				pcExit->AddPhiArgument(pcExit->masPhis.Get(ui), pcCall, psArgument->sValue);
			}
		}
		bChanged = true;
		ComputeLoops();
		mpcFunction->CountUses(&mauiUses);
		uiLoop = (size)-1;
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// True if what the loop works out is only used in the loop (by its
// instructions, jumps and phis).  mauiUses must be counted.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::OnlyUsedInLoop(size uiLoop)
{
	CArrayTemplate<uint32>	auiInside;
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRPhi*					psPhi;
	SIRReference			sValue;
	size					ui;
	size					uiIndex;
	size					uiArgument;
	bool					bOnly;

	auiInside.Init();
	auiInside.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	for (ui = 0; ui < mcLoops.NumBlocks(uiLoop); ui++)
	{
		pcBlock = mcLoops.GetBlock(uiLoop, ui);
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				sValue = psPhi->asArguments.Get(uiArgument)->sValue;
				if (sValue.IsTemporary())
				{
					(*auiInside.Get(sValue.Index()))++;
				}
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				if (psInstruction->asArguments[uiArgument].IsTemporary())
				{
					(*auiInside.Get(psInstruction->asArguments[uiArgument].Index()))++;
				}
			}
		}
		if (pcBlock->msJump.sArgument.IsTemporary())
		{
			(*auiInside.Get(pcBlock->msJump.sArgument.Index()))++;
		}
	}

	bOnly = true;
	for (ui = 0; bOnly && (ui < mcLoops.NumBlocks(uiLoop)); ui++)
	{
		pcBlock = mcLoops.GetBlock(uiLoop, ui);
		for (uiIndex = 0; bOnly && (uiIndex < pcBlock->masPhis.NumElements()); uiIndex++)
		{
			sValue = pcBlock->masPhis.Get(uiIndex)->sResult;
			bOnly = *mauiUses.Get(sValue.Index()) == *auiInside.Get(sValue.Index());
		}
		for (uiIndex = 0; bOnly && (uiIndex < pcBlock->masInstructions.NumElements()); uiIndex++)
		{
			sValue = pcBlock->masInstructions.Get(uiIndex)->sResult;
			bOnly = !sValue.IsTemporary() || (*mauiUses.Get(sValue.Index()) == *auiInside.Get(sValue.Index()));
		}
	}
	auiInside.Kill();
	return bOnly;
}


//////////////////////////////////////////////////////////////////////////
//
// strlen: a loop of one block that loads a byte through an address stepped
// one byte a time round (MatchIdiomAddress) and goes round while it isn't
// 0, doing nothing else.  It stops after strlen(the first address) times
// round, so each value that leaves it through the exit's phis (a phi stepped
// by a constant c, or its next value) is start + len * c (or (len + 1) * c),
// worked out on the way in instead.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceStrlen(size uiLoop)
{
	SIRIdiomCounter	sCounter;
	SIRIdiomAddress	sAddress;
	SIRLoop*		psLoop;
	CIRBlock*		pcLoop;
	CIRBlock*		pcExit;
	CIRBlock*		pcPreheader;
	SIRInstruction*	psLoad;
	SIRInstruction*	psCompare;
	SIRPhi*			psExitPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sLength;
	SIRReference	sValue;
	SIRReference	sStart;
	SIRReference	sTimes;
	SIRType			sType;
	CArrayIRReferences	asValues;
	size			ui;
	size			uj;
	uint16			uiOpcode;
	int64			lliStep;
	bool			bNext;
	bool			bResult;
	uint32			uiExitUses;

	psLoop = mcLoops.GetLoop(uiLoop);
	pcLoop = psLoop->pcHead;
	if ((mcLoops.NumBlocks(uiLoop) != 1) || (mcLoops.NumLatches(uiLoop) != 1) || !psLoop->pcEntering || (pcLoop->msJump.uiKind != IRJ_Branch))
	{
		return false;
	}
	pcExit = (pcLoop->msJump.pcThen == pcLoop) ? pcLoop->msJump.pcElse : pcLoop->msJump.pcThen;
	if (pcExit == pcLoop)
	{
		return false;
	}

	//One byte load, round while it isn't 0.
	if (!OnlyLoads(pcLoop, 1, &psLoad) || !psLoad->sType.Is(IRTK_I8) || (*mauiUses.Get(psLoad->sResult.Index()) != 1))
	{
		return false;
	}
	psCompare = mcDefinitions.Instruction(pcLoop->msJump.sArgument);
	if ((psCompare == NULL) || ((psCompare->uiOpcode != IRO_Ne) && (psCompare->uiOpcode != IRO_Eq)))
	{
		return false;
	}
	uiOpcode = (pcLoop->msJump.pcThen == pcLoop) ? psCompare->uiOpcode : IRInvertedComparison(psCompare->uiOpcode);
	if ((uiOpcode != IRO_Ne) || !((psCompare->asArguments[0].Equals(psLoad->sResult) && psCompare->asArguments[1].IsConstant() && (mpcFunction->GetConstant(psCompare->asArguments[1])->ulliBits == 0)) ||
								   (psCompare->asArguments[1].Equals(psLoad->sResult) && psCompare->asArguments[0].IsConstant() && (mpcFunction->GetConstant(psCompare->asArguments[0])->ulliBits == 0))))
	{
		return false;
	}
	sCounter.sCounter = IRNone();
	sCounter.sNext = IRNone();
	if (!MatchIdiomAddress(uiLoop, pcLoop, pcLoop, psLoad->asArguments[0], &sCounter, 1, &sAddress))
	{
		return false;
	}

	//What leaves the loop: only through the exit's phis, and each a stepped phi or its next value.
	asValues.Init();
	bResult = true;
	for (ui = 0; bResult && (ui < pcExit->masPhis.NumElements()); ui++)
	{
		psArgument = pcExit->FindPhiArgument(pcExit->masPhis.Get(ui), pcLoop);
		if (psArgument && (mcDefinitions.Block(psArgument->sValue) == pcLoop))
		{
			asValues.Add(&psArgument->sValue);
		}
	}
	for (ui = 0; bResult && (ui < pcLoop->masPhis.NumElements() + pcLoop->masInstructions.NumElements()); ui++)
	{
		sValue = (ui < pcLoop->masPhis.NumElements()) ? pcLoop->masPhis.Get(ui)->sResult : pcLoop->masInstructions.Get(ui - pcLoop->masPhis.NumElements())->sResult;
		if (!sValue.IsTemporary())
		{
			continue;
		}
		uiExitUses = 0;
		for (uj = 0; uj < asValues.NumElements(); uj++)
		{
			uiExitUses += asValues.Get(uj)->Equals(sValue) ? 1 : 0;
		}
		if (uiExitUses > 0)
		{
			bResult = (MatchLinear(uiLoop, sValue, &sStart, &lliStep, &bNext));
		}
	}
	if (bResult)
	{
		//Nothing else outside the loop uses its values.
		for (ui = 0; ui < asValues.NumElements(); ui++)
		{
			(*mauiUses.Get(asValues.Get(ui)->Index()))--;
		}
		bResult = OnlyUsedInLoop(uiLoop);
		for (ui = 0; ui < asValues.NumElements(); ui++)
		{
			(*mauiUses.Get(asValues.Get(ui)->Index()))++;
		}
	}
	if (!bResult)
	{
		asValues.Kill();
		return false;
	}

	//The length on the way in, and the values that leave from it.
	pcPreheader = AddPreheader(uiLoop);
	sStart = IdiomStart(pcPreheader, &sAddress, &sCounter);
	sLength = LibraryCall(pcPreheader, "_strlen", IRType(IRTK_I32), sStart, IRType(IRTK_Pointer));
	for (ui = 0; ui < pcExit->masPhis.NumElements(); ui++)
	{
		psExitPhi = pcExit->masPhis.Get(ui);
		psArgument = pcExit->FindPhiArgument(psExitPhi, pcLoop);
		if (!psArgument)
		{
			continue;
		}
		sValue = psArgument->sValue;
		if (mcDefinitions.Block(sValue) == pcLoop)
		{
			MatchLinear(uiLoop, sValue, &sStart, &lliStep, &bNext);
			sType = mpcFunction->TypeOf(sValue);
			sTimes = bNext ? mpcFunction->Add(pcPreheader, IRO_Add, IRType(IRTK_I32), sLength, mpcFunction->AddInteger(IRType(IRTK_I32), 1)) : sLength;
			if (sType.IsPointer())
			{
				sTimes = mpcFunction->Add(pcPreheader, IRO_Mul, IRType(IRTK_I32), sTimes, mpcFunction->AddInteger(IRType(IRTK_I32), lliStep));
				sValue = mpcFunction->Add(pcPreheader, IRO_PtrAdd, sType, sStart, sTimes);
			}
			else
			{
				if (!sType.Is(IRTK_I32))
				{
					sTimes = mpcFunction->Add(pcPreheader, (sType.IntegerBits() < 32) ? IRO_Trunc : IRO_ZExt, sType, sTimes);
				}
				sTimes = mpcFunction->Add(pcPreheader, IRO_Mul, sType, sTimes, mpcFunction->AddInteger(sType, lliStep));
				sValue = mpcFunction->Add(pcPreheader, IRO_Add, sType, sStart, sTimes);
			}
		}
		pcExit->AddPhiArgument(pcExit->masPhis.Get(ui), pcPreheader, sValue);
	}
	RemovePhiArguments(pcLoop, pcPreheader);
	pcPreheader->Jump(pcExit);
	asValues.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sValue, worked out in the loop, is start + k * step the k-th time
// round (from 0): a phi of its head taking start on the way in and phi +
// step (an add or ptradd of a constant) from the loop, or that next value
// (*pbNext: start + (k + 1) * step).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::MatchLinear(size uiLoop, SIRReference sValue, SIRReference* psStart, int64* plliStep, bool* pbNext)
{
	SIRInstruction*	psStep;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sPhi;
	size			ui;

	*pbNext = false;
	sPhi = sValue;
	psStep = mcDefinitions.Instruction(sValue);
	if (psStep)
	{
		if (((psStep->uiOpcode != IRO_Add) && (psStep->uiOpcode != IRO_PtrAdd)) || !psStep->asArguments[1].IsConstant())
		{
			return false;
		}
		sPhi = psStep->asArguments[0];
		*pbNext = true;
	}
	psPhi = mcDefinitions.Phi(sPhi);
	if ((psPhi == NULL) || (mcDefinitions.Block(sPhi) != mcLoops.GetLoop(uiLoop)->pcHead) || (psPhi->asArguments.NumElements() != 2))
	{
		return false;
	}
	*psStart = IRNone();
	*plliStep = 0;
	for (ui = 0; ui < 2; ui++)
	{
		psArgument = psPhi->asArguments.Get(ui);
		if (mcLoops.Contains(uiLoop, psArgument->pcBlock))
		{
			psStep = mcDefinitions.Instruction(psArgument->sValue);
			if ((psStep == NULL) || ((psStep->uiOpcode != IRO_Add) && (psStep->uiOpcode != IRO_PtrAdd)) || !psStep->asArguments[0].Equals(sPhi) ||
				!psStep->asArguments[1].IsConstant() || (*pbNext && !psArgument->sValue.Equals(sValue)))
			{
				return false;
			}
			*plliStep = mpcFunction->GetSigned(psStep->asArguments[1]);
		}
		else
		{
			*psStart = psArgument->sValue;
		}
	}
	return !psStart->IsNone();
}


//////////////////////////////////////////////////////////////////////////
//
// memcmp (as equal or not): a loop of a head that loads an element through
// each of two addresses stepped an element a time and leaves for one exit
// when they differ, and a latch that counts n - init times round
// (MatchIdiomCounter) and leaves for another, nothing else, with nothing
// it works out used outside.  On the way in memcmp of the bytes goes to the
// first exit if it isn't 0, else the second.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceMemcmp(size uiLoop)
{
	SIRIdiomCounter	sCounter;
	SIRIdiomAddress	sFirst;
	SIRIdiomAddress	sSecond;
	SIRLoop*		psLoop;
	CIRBlock*		pcHead;
	CIRBlock*		pcLatch;
	CIRBlock*		pcDiffer;
	CIRBlock*		pcSame;
	CIRBlock*		pcPreheader;
	SIRInstruction*	psCompare;
	SIRInstruction*	apsLoads[2];
	SIRInstruction*	psFirst;
	SIRInstruction*	psSecond;
	SIRPhiArgument*	psArgument;
	SIRReference	sFirstStart;
	SIRReference	sSecondStart;
	SIRReference	sBytes;
	SIRReference	sResult;
	SIRReference	sTest;
	int64			lliBytes;
	int64			lliTimes;
	size			ui;
	uint16			uiOpcode;

	psLoop = mcLoops.GetLoop(uiLoop);
	pcHead = psLoop->pcHead;
	if ((mcLoops.NumBlocks(uiLoop) != 2) || (mcLoops.NumLatches(uiLoop) != 1) || !psLoop->pcEntering || (pcHead->msJump.uiKind != IRJ_Branch))
	{
		return false;
	}
	pcLatch = mcLoops.GetLatch(uiLoop, 0);
	if ((pcLatch == pcHead) || (pcLatch->msJump.uiKind != IRJ_Branch) || (pcLatch->masPhis.NumElements() != 0))
	{
		return false;
	}
	if (pcHead->msJump.pcThen == pcLatch)
	{
		pcDiffer = pcHead->msJump.pcElse;
	}
	else if (pcHead->msJump.pcElse == pcLatch)
	{
		pcDiffer = pcHead->msJump.pcThen;
	}
	else
	{
		return false;
	}
	pcSame = (pcLatch->msJump.pcThen == pcHead) ? pcLatch->msJump.pcElse : pcLatch->msJump.pcThen;
	if ((pcSame == pcHead) || (pcSame == pcDiffer) || mcLoops.Contains(uiLoop, pcDiffer) || mcLoops.Contains(uiLoop, pcSame))
	{
		return false;
	}

	//Two loads compared for equal in the head (and nothing with an effect in either block).
	if (!OnlyLoads(pcHead, 2, apsLoads) || !OnlyLoads(pcLatch, 0, NULL))
	{
		return false;
	}
	psFirst = apsLoads[0];
	psSecond = apsLoads[1];
	if (!psFirst->sType.Equals(psSecond->sType) || !psFirst->sType.IsInteger())
	{
		return false;
	}
	psCompare = mcDefinitions.Instruction(pcHead->msJump.sArgument);
	if ((psCompare == NULL) || ((psCompare->uiOpcode != IRO_Eq) && (psCompare->uiOpcode != IRO_Ne)))
	{
		return false;
	}
	uiOpcode = (pcHead->msJump.pcThen == pcLatch) ? psCompare->uiOpcode : IRInvertedComparison(psCompare->uiOpcode);
	if ((uiOpcode != IRO_Eq) ||
		!((psCompare->asArguments[0].Equals(psFirst->sResult) && psCompare->asArguments[1].Equals(psSecond->sResult)) ||
		  (psCompare->asArguments[0].Equals(psSecond->sResult) && psCompare->asArguments[1].Equals(psFirst->sResult))))
	{
		return false;
	}
	if (!MatchIdiomCounter(uiLoop, pcHead, pcLatch, &sCounter) || !OnlyUsedInLoop(uiLoop))
	{
		return false;
	}
	lliBytes = mpcModule->GetSize(psFirst->sType);
	if (!MatchIdiomAddress(uiLoop, pcHead, pcLatch, psFirst->asArguments[0], &sCounter, lliBytes, &sFirst) ||
		!MatchIdiomAddress(uiLoop, pcHead, pcLatch, psSecond->asArguments[0], &sCounter, lliBytes, &sSecond))
	{
		return false;
	}
	if (sCounter.sInit.IsConstant() && sCounter.sBound.IsConstant())
	{
		lliTimes = mpcFunction->GetSigned(sCounter.sBound) - mpcFunction->GetSigned(sCounter.sInit);
		if ((lliTimes > 0) && (lliTimes * lliBytes <= IR_IDIOM_MIN_BYTES))
		{
			return false;
		}
	}

	//memcmp on the way in, to the exits.
	pcPreheader = AddPreheader(uiLoop);
	sFirstStart = IdiomStart(pcPreheader, &sFirst, &sCounter);
	sSecondStart = IdiomStart(pcPreheader, &sSecond, &sCounter);
	sBytes = IdiomBytes(pcPreheader, &sCounter, lliBytes);
	sResult = LibraryCall(pcPreheader, "_memcmp", msLibraryInt, sFirstStart, IRType(IRTK_Pointer), sSecondStart, IRType(IRTK_Pointer), sBytes, IRType(IRTK_I32));
	sTest = mpcFunction->Add(pcPreheader, IRO_Ne, msLibraryInt, sResult, mpcFunction->AddInteger(msLibraryInt, 0));
	for (ui = 0; ui < pcDiffer->masPhis.NumElements(); ui++)
	{
		psArgument = pcDiffer->FindPhiArgument(pcDiffer->masPhis.Get(ui), pcHead);
		if (psArgument)
		{
			pcDiffer->AddPhiArgument(pcDiffer->masPhis.Get(ui), pcPreheader, psArgument->sValue);
		}
	}
	for (ui = 0; ui < pcSame->masPhis.NumElements(); ui++)
	{
		psArgument = pcSame->FindPhiArgument(pcSame->masPhis.Get(ui), pcLatch);
		if (psArgument)
		{
			pcSame->AddPhiArgument(pcSame->masPhis.Get(ui), pcPreheader, psArgument->sValue);
		}
	}
	RemovePhiArguments(pcHead, pcPreheader);
	pcPreheader->Branch(sTest, pcDiffer, pcSame);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the values the loop works out are only used, outside it, by
// pcFound's phis (from pcFrom) or in blocks pcFound dominates, and are then
// phis of the loop stepped by a constant (MatchLinear, not next values).
// The values used are added to pasUsed.  mauiUses must be counted.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::FoundValues(size uiLoop, CIRBlock* pcFrom, CIRBlock* pcFound, CArrayIRReferences* pasUsed)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sValue;
	SIRReference	sStart;
	int64			lliStep;
	size			ui;
	size			uiIndex;
	size			uiArgument;
	bool			bNext;
	bool			bAllowed;

	pasUsed->FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if (mcLoops.Contains(uiLoop, pcBlock) || !mcDominators.IsReachable(pcBlock))
		{
			continue;
		}
		bAllowed = mcDominators.Dominates(pcFound, pcBlock);
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				psArgument = psPhi->asArguments.Get(uiArgument);
				sValue = psArgument->sValue;
				if (mcDefinitions.Block(sValue) && mcLoops.Contains(uiLoop, mcDefinitions.Block(sValue)))
				{
					if (!((pcBlock == pcFound) && (psArgument->pcBlock == pcFrom)) && !(bAllowed && (pcBlock != pcFound)))
					{
						return false;
					}
					pasUsed->Add(&sValue);
				}
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements() + 1; uiIndex++)
		{
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				if (uiIndex < pcBlock->masInstructions.NumElements())
				{
					psInstruction = pcBlock->masInstructions.Get(uiIndex);
					sValue = psInstruction->asArguments[uiArgument];
				}
				else
				{
					sValue = (uiArgument == 0) ? pcBlock->msJump.sArgument : IRNone();
				}
				if (sValue.IsTemporary() && mcDefinitions.Block(sValue) && mcLoops.Contains(uiLoop, mcDefinitions.Block(sValue)))
				{
					if (!bAllowed)
					{
						return false;
					}
					pasUsed->Add(&sValue);
				}
			}
		}
	}
	for (ui = 0; ui < pasUsed->NumElements(); ui++)
	{
		if (!MatchLinear(uiLoop, *pasUsed->Get(ui), &sStart, &lliStep, &bNext) || bNext)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The values a search loop found at k times round (sTimes, an i32): each in
// pasUsed (a phi of the head stepped by a constant) is start + k * step,
// worked out at the end of pcBlock; the uses outside the loop (by pcFound's
// phis from pcFrom, which then take them from pcBlock, and in the blocks
// pcFound dominates) use them.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::FoundAt(size uiLoop, CIRBlock* pcBlock, SIRReference sTimes, CIRBlock* pcFrom, CIRBlock* pcFound, CArrayIRReferences* pasUsed)
{
	CIRBlock*		pcUser;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sValue;
	SIRReference	sStart;
	SIRReference	sOffset;
	SIRReference	sNew;
	SIRType			sType;
	int64			lliStep;
	size			ui;
	size			uj;
	size			uiIndex;
	size			uiArgument;
	bool			bNext;

	for (ui = 0; ui < pasUsed->NumElements(); ui++)
	{
		sValue = *pasUsed->Get(ui);
		for (uj = 0; uj < ui; uj++)
		{
			if (pasUsed->Get(uj)->Equals(sValue))
			{
				break;
			}
		}
		if (uj < ui)
		{
			continue;
		}
		MatchLinear(uiLoop, sValue, &sStart, &lliStep, &bNext);
		sType = mpcFunction->TypeOf(sValue);
		if (sType.IsPointer())
		{
			sOffset = mpcFunction->Add(pcBlock, IRO_Mul, IRType(IRTK_I32), sTimes, mpcFunction->AddInteger(IRType(IRTK_I32), lliStep));
			sNew = mpcFunction->Add(pcBlock, IRO_PtrAdd, sType, sStart, sOffset);
		}
		else
		{
			sOffset = sType.Is(IRTK_I32) ? sTimes : mpcFunction->Add(pcBlock, (sType.IntegerBits() < 32) ? IRO_Trunc : IRO_ZExt, sType, sTimes);
			sOffset = mpcFunction->Add(pcBlock, IRO_Mul, sType, sOffset, mpcFunction->AddInteger(sType, lliStep));
			sNew = mpcFunction->Add(pcBlock, IRO_Add, sType, sStart, sOffset);
		}

		//The uses outside the loop.
		for (uiIndex = 0; uiIndex < pcFound->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcFound->masPhis.Get(uiIndex);
			psArgument = pcFound->FindPhiArgument(psPhi, pcFrom);
			if (psArgument && psArgument->sValue.Equals(sValue))
			{
				psArgument->pcBlock = pcBlock;
				psArgument->sValue = sNew;
			}
		}
		for (uiIndex = 0; uiIndex < mpcFunction->NumBlocks(); uiIndex++)
		{
			pcUser = mpcFunction->GetBlock(uiIndex);
			if (mcLoops.Contains(uiLoop, pcUser) || (pcUser == pcBlock) || !mcDominators.IsReachable(pcUser) || !mcDominators.Dominates(pcFound, pcUser))
			{
				continue;
			}
			for (uj = 0; uj < pcUser->masPhis.NumElements(); uj++)
			{
				psPhi = pcUser->masPhis.Get(uj);
				for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
				{
					if ((pcUser != pcFound) && psPhi->asArguments.Get(uiArgument)->sValue.Equals(sValue))
					{
						psPhi->asArguments.Get(uiArgument)->sValue = sNew;
					}
				}
			}
			for (uj = 0; uj < pcUser->masInstructions.NumElements(); uj++)
			{
				psInstruction = pcUser->masInstructions.Get(uj);
				for (uiArgument = 0; uiArgument < 2; uiArgument++)
				{
					if (psInstruction->asArguments[uiArgument].Equals(sValue))
					{
						psInstruction->asArguments[uiArgument] = sNew;
					}
				}
			}
			if (pcUser->msJump.sArgument.Equals(sValue))
			{
				pcUser->msJump.sArgument = sNew;
			}
		}
	}
	//Phis of pcFound from pcFrom that took a value from outside the loop take it from pcBlock too.
	for (uiIndex = 0; uiIndex < pcFound->masPhis.NumElements(); uiIndex++)
	{
		psArgument = pcFound->FindPhiArgument(pcFound->masPhis.Get(uiIndex), pcFrom);
		if (psArgument)
		{
			psArgument->pcBlock = pcBlock;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// memchr: a loop of a head that loads a byte through an address stepped a
// byte a time round and leaves for one exit (found) when it is c (not
// changed by the loop), and a latch that counts n - init times round
// (MatchIdiomCounter) and leaves for another, nothing else.  On the way in
// memchr: not found (NULL) goes to the second exit, found to a block that
// works out what found uses at k = the byte's offset times round
// (FoundValues), then to the first.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceMemchr(size uiLoop)
{
	SIRIdiomCounter		sCounter;
	SIRIdiomAddress		sAddress;
	CArrayIRReferences	asUsed;
	SIRLoop*			psLoop;
	CIRBlock*			pcHead;
	CIRBlock*			pcLatch;
	CIRBlock*			pcFound;
	CIRBlock*			pcMissing;
	CIRBlock*			pcPreheader;
	CIRBlock*			pcAt;
	SIRInstruction*		psLoad;
	SIRInstruction*		psCompare;
	SIRPhiArgument*		psArgument;
	SIRReference		sValue;
	SIRReference		sStart;
	SIRReference		sResult;
	SIRReference		sTimes;
	SIRReference		sTest;
	size				ui;
	uint16				uiOpcode;
	bool				bResult;

	psLoop = mcLoops.GetLoop(uiLoop);
	pcHead = psLoop->pcHead;
	if ((mcLoops.NumBlocks(uiLoop) != 2) || (mcLoops.NumLatches(uiLoop) != 1) || !psLoop->pcEntering || (pcHead->msJump.uiKind != IRJ_Branch))
	{
		return false;
	}
	pcLatch = mcLoops.GetLatch(uiLoop, 0);
	if ((pcLatch == pcHead) || (pcLatch->msJump.uiKind != IRJ_Branch) || (pcLatch->masPhis.NumElements() != 0))
	{
		return false;
	}
	if (pcHead->msJump.pcThen == pcLatch)
	{
		pcFound = pcHead->msJump.pcElse;
	}
	else if (pcHead->msJump.pcElse == pcLatch)
	{
		pcFound = pcHead->msJump.pcThen;
	}
	else
	{
		return false;
	}
	pcMissing = (pcLatch->msJump.pcThen == pcHead) ? pcLatch->msJump.pcElse : pcLatch->msJump.pcThen;
	if ((pcMissing == pcHead) || (pcMissing == pcFound) || mcLoops.Contains(uiLoop, pcFound) || mcLoops.Contains(uiLoop, pcMissing) ||
		(pcFound->mapcPredecessors.NumElements() != 1))
	{
		return false;
	}

	//One byte load compared with c in the head, nothing with an effect in either block.
	if (!OnlyLoads(pcHead, 1, &psLoad) || !OnlyLoads(pcLatch, 0, NULL) || !psLoad->sType.Is(IRTK_I8))
	{
		return false;
	}
	psCompare = mcDefinitions.Instruction(pcHead->msJump.sArgument);
	if ((psCompare == NULL) || ((psCompare->uiOpcode != IRO_Eq) && (psCompare->uiOpcode != IRO_Ne)))
	{
		return false;
	}
	uiOpcode = (pcHead->msJump.pcThen == pcFound) ? psCompare->uiOpcode : IRInvertedComparison(psCompare->uiOpcode);
	if (uiOpcode != IRO_Eq)
	{
		return false;
	}
	if (psCompare->asArguments[0].Equals(psLoad->sResult))
	{
		sValue = psCompare->asArguments[1];
	}
	else if (psCompare->asArguments[1].Equals(psLoad->sResult))
	{
		sValue = psCompare->asArguments[0];
	}
	else
	{
		return false;
	}
	if (!mcLoops.IsInvariant(uiLoop, sValue, &mcDefinitions) || !MatchIdiomCounter(uiLoop, pcHead, pcLatch, &sCounter) ||
		!MatchIdiomAddress(uiLoop, pcHead, pcLatch, psLoad->asArguments[0], &sCounter, 1, &sAddress))
	{
		return false;
	}
	asUsed.Init();
	bResult = FoundValues(uiLoop, pcHead, pcFound, &asUsed);
	for (ui = 0; bResult && (ui < pcMissing->masPhis.NumElements()); ui++)
	{
		psArgument = pcMissing->FindPhiArgument(pcMissing->masPhis.Get(ui), pcLatch);
		bResult = !psArgument || mcLoops.IsInvariant(uiLoop, psArgument->sValue, &mcDefinitions);
	}
	if (!bResult)
	{
		asUsed.Kill();
		return false;
	}

	//memchr on the way in; found works out its values from the offset.
	pcPreheader = AddPreheader(uiLoop);
	sStart = IdiomStart(pcPreheader, &sAddress, &sCounter);
	if (sValue.IsConstant())
	{
		sValue = mpcFunction->AddConstant(msLibraryInt, mpcFunction->GetConstant(sValue)->ulliBits & 0xFF);
	}
	else
	{
		sValue = mpcFunction->Add(pcPreheader, IRO_ZExt, msLibraryInt, sValue);
	}
	sTimes = IdiomBytes(pcPreheader, &sCounter, 1);
	sResult = LibraryCall(pcPreheader, "_memchr", IRType(IRTK_Pointer), sStart, IRType(IRTK_Pointer), sValue, msLibraryInt, sTimes, IRType(IRTK_I32));
	sTest = mpcFunction->Add(pcPreheader, IRO_Eq, IRType(IRTK_Pointer), sResult, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0));
	pcAt = mpcFunction->AddBlock();
	sTimes = mpcFunction->Add(pcAt, IRO_PtrToI, IRType(IRTK_I32), sResult);
	sTimes = mpcFunction->Add(pcAt, IRO_Sub, IRType(IRTK_I32), sTimes, mpcFunction->Add(pcAt, IRO_PtrToI, IRType(IRTK_I32), sStart));
	FoundAt(uiLoop, pcAt, sTimes, pcHead, pcFound, &asUsed);
	pcAt->Jump(pcFound);
	for (ui = 0; ui < pcMissing->masPhis.NumElements(); ui++)
	{
		psArgument = pcMissing->FindPhiArgument(pcMissing->masPhis.Get(ui), pcLatch);
		if (psArgument)
		{
			pcMissing->AddPhiArgument(pcMissing->masPhis.Get(ui), pcPreheader, psArgument->sValue);
		}
	}
	RemovePhiArguments(pcHead, pcPreheader);
	pcPreheader->Branch(sTest, pcMissing, pcAt);
	asUsed.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// strchr, as a rotated "while (*p != c) { if (*p == 0) ...; p++; }" leaves
// it: a loop of a head with a pointer phi p and a phi b of the byte at p
// (loaded from p's start on the way in) that leaves for one exit (zero)
// when b is 0, and a latch that steps p by one, loads the byte there and
// goes round while it isn't c (not changed by the loop), else leaves for
// another (found) with the new p.  On the way in: b 0 goes to zero, else
// strchr(p + 1, c), NULL to zero, else to found with that pointer (and
// with it less one for p).  Nothing else the loop works out is used
// outside.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceStrchr(size uiLoop)
{
	SIRLoop*		psLoop;
	CIRBlock*		pcHead;
	CIRBlock*		pcLatch;
	CIRBlock*		pcZero;
	CIRBlock*		pcFound;
	CIRBlock*		pcEntering;
	CIRBlock*		pcPreheader;
	CIRBlock*		pcCall;
	SIRInstruction*	psZeroTest;
	SIRInstruction*	psStep;
	SIRInstruction*	psLoad;
	SIRInstruction*	psTest;
	SIRInstruction*	psFirstLoad;
	SIRPhi*			psPointer;
	SIRPhi*			psByte;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sPointer;
	SIRReference	sByte;
	SIRReference	sStart;
	SIRReference	sFirst;
	SIRReference	sValue;
	SIRReference	sResult;
	SIRReference	sTest;
	SIRReference	sBefore;
	SIRReference	sZero;
	size			ui;
	uint16			uiOpcode;
	bool			bOnly;

	psLoop = mcLoops.GetLoop(uiLoop);
	pcHead = psLoop->pcHead;
	pcEntering = psLoop->pcEntering;
	if ((mcLoops.NumBlocks(uiLoop) != 2) || (mcLoops.NumLatches(uiLoop) != 1) || !pcEntering || (pcHead->msJump.uiKind != IRJ_Branch) ||
		(pcHead->masPhis.NumElements() != 2) || (pcHead->masInstructions.NumElements() != 1))
	{
		return false;
	}
	pcLatch = mcLoops.GetLatch(uiLoop, 0);
	if ((pcLatch == pcHead) || (pcLatch->msJump.uiKind != IRJ_Branch) || (pcLatch->masPhis.NumElements() != 0) || (pcLatch->masInstructions.NumElements() != 3))
	{
		return false;
	}

	//The head: b == 0 to zero, else the latch.
	psZeroTest = pcHead->masInstructions.Get(0);
	if (!pcHead->msJump.sArgument.Equals(psZeroTest->sResult) || ((psZeroTest->uiOpcode != IRO_Eq) && (psZeroTest->uiOpcode != IRO_Ne)) ||
		!psZeroTest->asArguments[1].IsConstant() || (mpcFunction->GetConstant(psZeroTest->asArguments[1])->ulliBits != 0))
	{
		return false;
	}
	if (pcHead->msJump.pcThen == pcLatch)
	{
		pcZero = pcHead->msJump.pcElse;
		uiOpcode = IRInvertedComparison(psZeroTest->uiOpcode);
	}
	else if (pcHead->msJump.pcElse == pcLatch)
	{
		pcZero = pcHead->msJump.pcThen;
		uiOpcode = psZeroTest->uiOpcode;
	}
	else
	{
		return false;
	}
	if (uiOpcode != IRO_Eq)
	{
		return false;
	}
	sByte = psZeroTest->asArguments[0];
	psByte = mcDefinitions.Phi(sByte);
	if (!psByte || (mcDefinitions.Block(sByte) != pcHead) || !psByte->sType.Is(IRTK_I8))
	{
		return false;
	}
	psPointer = (pcHead->masPhis.Get(0) == psByte) ? pcHead->masPhis.Get(1) : pcHead->masPhis.Get(0);
	sPointer = psPointer->sResult;
	if (!psPointer->sType.IsPointer())
	{
		return false;
	}

	//The latch: p + 1, the byte there, round while it isn't c, else found.
	psStep = pcLatch->masInstructions.Get(0);
	psLoad = pcLatch->masInstructions.Get(1);
	psTest = pcLatch->masInstructions.Get(2);
	if ((psStep->uiOpcode != IRO_PtrAdd) || !psStep->asArguments[0].Equals(sPointer) || !psStep->asArguments[1].IsConstant() || (mpcFunction->GetSigned(psStep->asArguments[1]) != 1) ||
		(psLoad->uiOpcode != IRO_Load) || !psLoad->sType.Is(IRTK_I8) || (psLoad->uiFlags & IRF_Volatile) || !psLoad->asArguments[0].Equals(psStep->sResult) ||
		((psTest->uiOpcode != IRO_Ne) && (psTest->uiOpcode != IRO_Eq)) || !pcLatch->msJump.sArgument.Equals(psTest->sResult))
	{
		return false;
	}
	if (psTest->asArguments[0].Equals(psLoad->sResult))
	{
		sValue = psTest->asArguments[1];
	}
	else if (psTest->asArguments[1].Equals(psLoad->sResult))
	{
		sValue = psTest->asArguments[0];
	}
	else
	{
		return false;
	}
	pcFound = (pcLatch->msJump.pcThen == pcHead) ? pcLatch->msJump.pcElse : pcLatch->msJump.pcThen;
	uiOpcode = (pcLatch->msJump.pcThen == pcHead) ? psTest->uiOpcode : IRInvertedComparison(psTest->uiOpcode);
	if ((uiOpcode != IRO_Ne) || (pcFound == pcHead) || (pcFound == pcZero) || mcLoops.Contains(uiLoop, pcFound) || mcLoops.Contains(uiLoop, pcZero) ||
		!mcLoops.IsInvariant(uiLoop, sValue, &mcDefinitions))
	{
		return false;
	}

	//The phis come round from the latch as p + 1 and its byte; in, b is the byte at p's start.
	psArgument = pcHead->FindPhiArgument(psPointer, pcLatch);
	if (!psArgument || !psArgument->sValue.Equals(psStep->sResult))
	{
		return false;
	}
	psArgument = pcHead->FindPhiArgument(psByte, pcLatch);
	if (!psArgument || !psArgument->sValue.Equals(psLoad->sResult))
	{
		return false;
	}
	psArgument = pcHead->FindPhiArgument(psPointer, pcEntering);
	if (!psArgument)
	{
		return false;
	}
	sStart = psArgument->sValue;
	psArgument = pcHead->FindPhiArgument(psByte, pcEntering);
	if (!psArgument)
	{
		return false;
	}
	sFirst = psArgument->sValue;
	psFirstLoad = mcDefinitions.Instruction(sFirst);
	if (!psFirstLoad || (psFirstLoad->uiOpcode != IRO_Load) || !psFirstLoad->asArguments[0].Equals(sStart) || (psFirstLoad->uiFlags & IRF_Volatile))
	{
		return false;
	}

	//Only found's phis from the latch take the loop's values, p + 1 or p: else the loop's values stay in it.
	for (ui = 0; ui < pcFound->masPhis.NumElements(); ui++)
	{
		psArgument = pcFound->FindPhiArgument(pcFound->masPhis.Get(ui), pcLatch);
		if (psArgument && IsLoopValue(uiLoop, psArgument->sValue))
		{
			if (!psArgument->sValue.Equals(psStep->sResult) && !psArgument->sValue.Equals(sPointer))
			{
				return false;
			}
			(*mauiUses.Get(psArgument->sValue.Index()))--;
		}
	}
	bOnly = OnlyUsedInLoop(uiLoop);
	for (ui = 0; ui < pcFound->masPhis.NumElements(); ui++)
	{
		psArgument = pcFound->FindPhiArgument(pcFound->masPhis.Get(ui), pcLatch);
		if (psArgument && IsLoopValue(uiLoop, psArgument->sValue))
		{
			(*mauiUses.Get(psArgument->sValue.Index()))++;
		}
	}
	if (!bOnly)
	{
		return false;
	}

	//On the way in.
	pcPreheader = AddPreheader(uiLoop);
	pcCall = mpcFunction->AddBlock();
	sZero = mpcFunction->Add(pcPreheader, IRO_Eq, IRType(IRTK_I8), sFirst, mpcFunction->AddInteger(IRType(IRTK_I8), 0));
	sValue = sValue.IsConstant() ? mpcFunction->AddConstant(msLibraryInt, mpcFunction->GetConstant(sValue)->ulliBits & 0xFF) :
								   mpcFunction->Add(pcCall, IRO_ZExt, msLibraryInt, sValue);
	sTest = mpcFunction->Add(pcCall, IRO_PtrAdd, IRType(IRTK_Pointer), sStart, mpcFunction->AddInteger(IRType(IRTK_I32), 1));
	sResult = LibraryCall(pcCall, "_strchr", IRType(IRTK_Pointer), sTest, IRType(IRTK_Pointer), sValue, msLibraryInt);
	sBefore = mpcFunction->Add(pcCall, IRO_PtrAdd, IRType(IRTK_Pointer), sResult, mpcFunction->AddInteger(IRType(IRTK_I32), -1));
	pcCall->Branch(mpcFunction->Add(pcCall, IRO_Eq, IRType(IRTK_Pointer), sResult, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0)), pcZero, pcFound);
	for (ui = 0; ui < pcZero->masPhis.NumElements(); ui++)
	{
		psPhi = pcZero->masPhis.Get(ui);
		psArgument = pcZero->FindPhiArgument(psPhi, pcHead);
		if (psArgument)
		{
			sValue = psArgument->sValue;
			pcZero->AddPhiArgument(pcZero->masPhis.Get(ui), pcPreheader, sValue);
			pcZero->AddPhiArgument(pcZero->masPhis.Get(ui), pcCall, sValue);
		}
	}
	for (ui = 0; ui < pcFound->masPhis.NumElements(); ui++)
	{
		psPhi = pcFound->masPhis.Get(ui);
		psArgument = pcFound->FindPhiArgument(psPhi, pcLatch);
		if (psArgument)
		{
			sValue = psArgument->sValue;
			if (sValue.Equals(psStep->sResult))
			{
				sValue = sResult;
			}
			else if (sValue.Equals(sPointer))
			{
				sValue = sBefore;
			}
			pcFound->AddPhiArgument(pcFound->masPhis.Get(ui), pcCall, sValue);
		}
	}
	RemovePhiArguments(pcHead, pcPreheader);
	pcPreheader->Branch(sZero, pcZero, pcCall);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is worked out in the loop.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::IsLoopValue(size uiLoop, SIRReference sValue)
{
	CIRBlock*	pcBlock;

	pcBlock = mcDefinitions.Block(sValue);
	return pcBlock && mcLoops.Contains(uiLoop, pcBlock);
}
