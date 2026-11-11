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
#include "IRBackendPasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBackendPasses::Init(void)
{
	mpcFunction = NULL;
	mcDominators.Init();
	mcLoops.Init();
	masStack.Init();
	maiVisited.Init();
	mauiUses.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBackendPasses::Kill(void)
{
	mauiUses.Kill();
	maiVisited.Kill();
	masStack.Kill();
	mcLoops.Kill();
	mcDominators.Kill();
	mpcFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the blocks in reverse post order, a brif's else visited first so its
// then block comes straight after it (a loop's body after its test, which
// MoveLoopTests then moves to the bottom).  With bLoopsTogether, of a
// block's successors the search visits first those leaving the most loops it
// is in, so they finish first and go after the loop: a loop's blocks stay
// together (a latch found first would otherwise go last, after all the code
// after the loop, and the W65C816 places the loop's values as if live over
// all of it).  Blocks nothing reaches go last.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackendPasses::OrderBlocks(CIRFunction* pcFunction, bool bLoopsTogether)
{
	CArrayIRBlocks	apcOrder;
	SIRSearch*		psTop;
	SIRSearch*		psNew;
	CIRBlock*		pcBlock;
	CIRBlock*		pcSuccessor;
	CIRBlock*		pcNext;
	size			ui;
	size			uiSuccessor;
	int32			iDepth;
	int32			iNextDepth;

	mpcFunction = pcFunction;
	mcDominators.Compute(mpcFunction);
	if (bLoopsTogether)
	{
		mcLoops.Compute(mpcFunction, &mcDominators);
	}
	maiVisited.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		*maiVisited.Add() = 0;
	}

	apcOrder.Init();
	masStack.FakeSetUsedElements(0);
	psNew = masStack.Add();
	psNew->iBlock = 0;
	psNew->iNext = 0;
	*maiVisited.Get(0) = 1;
	while (masStack.NumElements() > 0)
	{
		psTop = masStack.Tail();
		pcBlock = mpcFunction->GetBlock(psTop->iBlock);

		//The successor not yet reached in the fewest of the block's loops, the last (a brif's else) of those.
		pcNext = NULL;
		iNextDepth = 0;
		for (uiSuccessor = pcBlock->NumSuccessors(); uiSuccessor > 0; uiSuccessor--)
		{
			pcSuccessor = pcBlock->GetSuccessor(uiSuccessor - 1);
			if (pcSuccessor && (*maiVisited.Get(pcSuccessor->muiIndex) == 0))
			{
				iDepth = bLoopsTogether ? SharedDepth(pcBlock, pcSuccessor) : 0;
				if ((pcNext == NULL) || (iDepth < iNextDepth))
				{
					pcNext = pcSuccessor;
					iNextDepth = iDepth;
				}
			}
		}

		if (pcNext)
		{
			*maiVisited.Get(pcNext->muiIndex) = 1;
			psNew = masStack.Add();
			psNew->iBlock = (int32)pcNext->muiIndex;
			psNew->iNext = 0;
		}
		else
		{
			apcOrder.Add(pcBlock);
			masStack.Pop();
		}
	}
	apcOrder.Reverse();

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		if (*maiVisited.Get(ui) == 0)
		{
			apcOrder.Add(mpcFunction->GetBlock(ui));
		}
	}
	mpcFunction->SetBlockOrder(&apcOrder);
	mpcFunction->ComputePredecessors();
	apcOrder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The depth of the innermost loop both pcBlock and pcSuccessor are in, 0
// when they share none.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRBackendPasses::SharedDepth(CIRBlock* pcBlock, CIRBlock* pcSuccessor)
{
	int32	iLoop;

	for (iLoop = mcLoops.Innermost(pcBlock); iLoop >= 0; iLoop = mcLoops.GetLoop(iLoop)->iParent)
	{
		if (mcLoops.Contains(iLoop, pcSuccessor))
		{
			return mcLoops.GetLoop(iLoop)->iDepth;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves a loop's test to after its body: a block that branches into the
// block after it or out of the loop, and that the last block of the loop
// jumps back to, goes after that block.  The loop is entered by a bra to
// the test, and each time round takes one branch.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackendPasses::MoveLoopTests(CIRFunction* pcFunction)
{
	CArrayIRBlocks	apcOrder;
	CArrayIRBlocks	apcHeads;
	CArrayIRBlocks	apcLatches;
	size			ui;
	size			uiPredecessor;
	size			uiHead;
	size			uiLatch;
	CIRBlock*		pcHead;
	CIRBlock*		pcLatch;
	CIRBlock*		pcPredecessor;
	CIRBlock*		pcOutside;
	SIRJump*		psJump;

	mpcFunction = pcFunction;
	mcDominators.Compute(mpcFunction);
	apcHeads.Init();
	apcLatches.Init();
	for (ui = 1; ui + 1 < mpcFunction->NumBlocks(); ui++)
	{
		pcHead = mpcFunction->GetBlock(ui);
		psJump = &pcHead->msJump;
		if ((psJump->uiKind != IRJ_Branch) || !mcDominators.IsReachable(pcHead))
		{
			continue;
		}
		if (psJump->pcThen == mpcFunction->GetBlock(ui + 1))
		{
			pcOutside = psJump->pcElse;
		}
		else if (psJump->pcElse == mpcFunction->GetBlock(ui + 1))
		{
			pcOutside = psJump->pcThen;
		}
		else
		{
			continue;
		}

		pcLatch = NULL;
		for (uiPredecessor = 0; uiPredecessor < pcHead->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			pcPredecessor = pcHead->mapcPredecessors.GetPtr(uiPredecessor);
			if ((pcPredecessor->muiIndex > ui) && mcDominators.IsBackEdge(pcPredecessor, pcHead) &&
				((pcLatch == NULL) || (pcPredecessor->muiIndex > pcLatch->muiIndex)))
			{
				pcLatch = pcPredecessor;
			}
		}
		if ((pcLatch == NULL) || (pcLatch->msJump.uiKind != IRJ_Jump) || ((pcOutside->muiIndex > ui) && (pcOutside->muiIndex <= pcLatch->muiIndex)))
		{
			continue;
		}
		apcHeads.Add(pcHead);
		apcLatches.Add(pcLatch);
	}

	if (apcHeads.NumElements() > 0)
	{
		apcOrder.Init();
		apcOrder.Copy(&mpcFunction->mapcBlocks);
		for (ui = 0; ui < apcHeads.NumElements(); ui++)
		{
			pcHead = apcHeads.GetPtr(ui);
			uiHead = apcOrder.Find(pcHead);
			apcOrder.RemoveAt(uiHead, true);
			uiLatch = apcOrder.Find(apcLatches.GetPtr(ui));
			apcOrder.InsertAt(&pcHead, uiLatch + 1);
		}
		mpcFunction->SetBlockOrder(&apcOrder);
		apcOrder.Kill();
	}
	apcLatches.Kill();
	apcHeads.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Moves a load used once, later in its block, to just before its use when
// nothing between has an effect (IRHasEffect: a store, a call, an arg, a
// volatile load ...), so the use can take it from A or as a memory operand.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackendPasses::SinkLoads(CIRFunction* pcFunction)
{
	size			ui;
	size			uiLoad;
	size			uiUse;
	CIRBlock*		pcBlock;
	SIRInstruction*	psLoad;
	SIRInstruction*	psUse;
	SIRInstruction	sLoad;
	SIRReference	sResult;
	bool			bBlocked;

	mpcFunction = pcFunction;
	mpcFunction->CountUses(&mauiUses);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiLoad = pcBlock->masInstructions.NumElements(); uiLoad > 0; uiLoad--)
		{
			psLoad = pcBlock->masInstructions.Get(uiLoad - 1);
			sResult = psLoad->sResult;
			if ((psLoad->uiOpcode != IRO_Load) || (psLoad->uiFlags & IRF_Volatile) || !sResult.IsTemporary() || (*mauiUses.Get(sResult.Index()) != 1))
			{
				continue;
			}
			bBlocked = false;
			for (uiUse = uiLoad; uiUse < pcBlock->masInstructions.NumElements(); uiUse++)
			{
				psUse = pcBlock->masInstructions.Get(uiUse);
				if (psUse->asArguments[0].Equals(sResult) || psUse->asArguments[1].Equals(sResult))
				{
					break;
				}
				if (IRHasEffect(psUse))
				{
					bBlocked = true;
					break;
				}
			}
			if (bBlocked || (uiUse >= pcBlock->masInstructions.NumElements()) || (uiUse == uiLoad) ||
				(pcBlock->masInstructions.Get(uiUse)->uiOpcode == IRO_Arg) || (pcBlock->masInstructions.Get(uiUse)->uiOpcode == IRO_Call))
			{
				continue;
			}
			sLoad = *psLoad;
			pcBlock->masInstructions.RemoveAt(uiLoad - 1, true);
			pcBlock->masInstructions.InsertAt(&sLoad, uiUse - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A loop's step p2 = p + q (add, sub or ptradd; p a phi and p2 the value it
// takes from round the loop: *p++, i + 1) moves down its block past the
// instructions after it that read p, so p dies where p2 is made and the two
// can share a place (the 65C64's add Y, 1, the W65C816's StepInPlace)
// instead of p2 being copied back to p each time round.  Never past a use
// of p2.  Past a comparison of p the block's brif takes, though that is
// then not fused (written last, as both back ends need): sharing the place
// is worth more on both (measured 2026-11-05).  p read by a call's arg is
// read where the call is, so the step goes after the call, never among the
// args (the 65C64 needs them together).  Moving an add, sub or ptradd
// changes nothing else: they don't touch memory.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackendPasses::SinkSteps(CIRFunction* pcFunction)
{
	CArrayTemplate<int32>	aiDefinitionBlocks;	//Per temporary: the block defining it, else -1.
	size					ui;
	size					uiBlock;
	size					uiPhi;
	size					uiArgument;
	CIRBlock*				pcBlock;
	CIRBlock*				pcHead;
	SIRPhi*					psPhi;
	SIRInstruction*			psInstruction;
	int32					iNone;
	SIRReference			sValue;

	mpcFunction = pcFunction;
	iNone = -1;
	aiDefinitionBlocks.Init();
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		aiDefinitionBlocks.Add(&iNone);
	}
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if (psInstruction->sResult.IsTemporary() && (psInstruction->sResult.Index() < aiDefinitionBlocks.NumElements()))
			{
				*aiDefinitionBlocks.Get(psInstruction->sResult.Index()) = (int32)uiBlock;
			}
		}
	}

	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcHead = mpcFunction->GetBlock(uiBlock);
		for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
		{
			psPhi = pcHead->masPhis.Get(uiPhi);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				sValue = psPhi->asArguments.Get(uiArgument)->sValue;
				if (!sValue.IsTemporary() || (sValue.Index() >= aiDefinitionBlocks.NumElements()) || (*aiDefinitionBlocks.Get(sValue.Index()) < 0))
				{
					continue;
				}
				SinkStep(mpcFunction->GetBlock((size)*aiDefinitionBlocks.Get(sValue.Index())), sValue, psPhi->sResult);
			}
		}
	}
	aiDefinitionBlocks.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// SinkSteps for the instruction in pcBlock making sStep, when it is a step
// of the phi sPhi.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackendPasses::SinkStep(CIRBlock* pcBlock, SIRReference sStep, SIRReference sPhi)
{
	size			uiStep;
	size			uiLater;
	size			uiLast;
	SIRInstruction*	psStep;
	SIRInstruction*	psLater;
	SIRInstruction	sMoved;
	bool			bStep;

	for (uiStep = 0; (uiStep < pcBlock->masInstructions.NumElements()) && !pcBlock->masInstructions.Get(uiStep)->sResult.Equals(sStep); uiStep++)
	{
	}
	if (uiStep >= pcBlock->masInstructions.NumElements())
	{
		return;
	}
	psStep = pcBlock->masInstructions.Get(uiStep);
	switch (psStep->uiOpcode)
	{
	case IRO_Add:
		bStep = psStep->asArguments[0].Equals(sPhi) || psStep->asArguments[1].Equals(sPhi);
		break;
	case IRO_Sub:
	case IRO_PtrAdd:
		bStep = psStep->asArguments[0].Equals(sPhi);
		break;
	default:
		bStep = false;
		break;
	}
	if (!bStep || psStep->asArguments[0].Equals(psStep->asArguments[1]))
	{
		return;
	}


	//The last instruction after it that reads p, with none between using p2; a call's arg is read at the call.
	uiLast = uiStep;
	for (uiLater = uiStep + 1; uiLater < pcBlock->masInstructions.NumElements(); uiLater++)
	{
		psLater = pcBlock->masInstructions.Get(uiLater);
		if (psLater->asArguments[0].Equals(sStep) || psLater->asArguments[1].Equals(sStep))
		{
			break;
		}
		if (psLater->asArguments[0].Equals(sPhi) || psLater->asArguments[1].Equals(sPhi))
		{
			uiLast = uiLater;
		}
	}
	while ((uiLast > uiStep) && (pcBlock->masInstructions.Get(uiLast)->uiOpcode == IRO_Arg))
	{
		psLater = pcBlock->masInstructions.SafeGet(uiLast + 1);
		if ((psLater == NULL) || psLater->asArguments[0].Equals(sStep) || psLater->asArguments[1].Equals(sStep))
		{
			return;
		}
		uiLast++;
	}
	if (uiLast == uiStep)
	{
		return;
	}
	sMoved = *psStep;
	pcBlock->masInstructions.RemoveAt(uiStep, true);
	pcBlock->masInstructions.InsertAt(&sMoved, uiLast);
}
