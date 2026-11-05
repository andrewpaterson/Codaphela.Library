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
#include "IROptimiserInternal.h"


//The optimiser's promotion of slots to temporaries (SSA construction): phis placed, and the slots' loads and stores
//renamed to values.


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
