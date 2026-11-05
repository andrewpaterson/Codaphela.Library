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
#include "IRBackendPasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBackendPasses::Init(void)
{
	mpcFunction = NULL;
	mcDominators.Init();
	mauiUses.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBackendPasses::Kill(void)
{
	mauiUses.Kill();
	mcDominators.Kill();
	mpcFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the blocks in reverse post order, a brif's else visited first so its
// then block comes straight after it (a loop's body after its test, which
// MoveLoopTests then moves to the bottom).  Blocks nothing reaches go last.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackendPasses::OrderBlocks(CIRFunction* pcFunction)
{
	CArrayIRBlocks	apcOrder;
	size			ui;

	mpcFunction = pcFunction;
	mcDominators.Compute(mpcFunction, true);
	apcOrder.Init();
	for (ui = 0; ui < mcDominators.NumReachable(); ui++)
	{
		apcOrder.Add(mcDominators.GetReversePostOrder(ui));
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		if (!mcDominators.IsReachable(mpcFunction->GetBlock(ui)))
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
// nothing between writes memory (a store, a call, a volatile load) or
// pushes arguments, so the use can take it from A or as a memory operand.
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
				if ((psUse->uiOpcode == IRO_Store) || (psUse->uiOpcode == IRO_Call) || (psUse->uiOpcode == IRO_Arg) || ((psUse->uiOpcode == IRO_Load) && (psUse->uiFlags & IRF_Volatile)))
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
