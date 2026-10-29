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
#include "IRLoops.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRLoops::Init(void)
{
	mpcFunction = NULL;
	mpcDominators = NULL;
	masLoops.Init();
	mapcBlocks.Init();
	mapcLatches.Init();
	maiInnermost.Init();
	mapcStack.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRLoops::Kill(void)
{
	mapcStack.Kill();
	maiInnermost.Kill();
	mapcLatches.Kill();
	mapcBlocks.Kill();
	masLoops.Kill();
	mpcDominators = NULL;
	mpcFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The heads are taken in reverse post order, so a loop is found before
// the loops inside it (its head dominates theirs): a block's innermost loop
// is the last found with it in.
//
//////////////////////////////////////////////////////////////////////////
void CIRLoops::Compute(CIRFunction* pcFunction, CIRDominators* pcDominators)
{
	size	ui;

	mpcFunction = pcFunction;
	mpcDominators = pcDominators;
	masLoops.FakeSetUsedElements(0);
	mapcBlocks.FakeSetUsedElements(0);
	mapcLatches.FakeSetUsedElements(0);
	maiInnermost.FakeSetUsedElements(0);
	maiInnermost.GrowToAtLeastNumElements(pcFunction->NumBlocks(), true, 0xFF);
	for (ui = 0; ui < pcDominators->NumReachable(); ui++)
	{
		AddLoop(pcDominators->GetReversePostOrder(ui));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The loop pcHead heads, if any block jumps back to it: its blocks are found
// going back from the latches to the head.
//
//////////////////////////////////////////////////////////////////////////
void CIRLoops::AddLoop(CIRBlock* pcHead)
{
	SIRLoop*	psLoop;
	CIRBlock*	pcPredecessor;
	CIRBlock*	pcBlock;
	size		ui;
	int32		iLoop;
	int32		iParent;
	bool		bSeveral;

	iLoop = (int32)masLoops.NumElements();
	iParent = *maiInnermost.Get(pcHead->muiIndex);
	psLoop = NULL;
	bSeveral = false;
	for (ui = 0; ui < pcHead->mapcPredecessors.NumElements(); ui++)
	{
		pcPredecessor = pcHead->mapcPredecessors.GetPtr(ui);
		if (!mpcDominators->IsReachable(pcPredecessor) || !mpcDominators->IsBackEdge(pcPredecessor, pcHead))
		{
			continue;
		}
		if (!psLoop)
		{
			psLoop = masLoops.Add();
			psLoop->pcHead = pcHead;
			psLoop->pcEntering = NULL;
			psLoop->pcPreheader = NULL;
			psLoop->iParent = iParent;
			psLoop->iDepth = (iParent >= 0) ? masLoops.Get(iParent)->iDepth + 1 : 1;
			psLoop->iFirstLatch = (int32)mapcLatches.NumElements();
			psLoop->iNumLatches = 0;
		}
		mapcLatches.Add(pcPredecessor);
		psLoop->iNumLatches++;
	}
	if (!psLoop)
	{
		return;
	}

	//The blocks, the head first.
	psLoop->iFirstBlock = (int32)mapcBlocks.NumElements();
	mapcBlocks.Add(pcHead);
	*maiInnermost.Get(pcHead->muiIndex) = iLoop;
	mapcStack.FakeSetUsedElements(0);
	for (ui = 0; ui < (size)psLoop->iNumLatches; ui++)
	{
		mapcStack.Push(mapcLatches.GetPtr(psLoop->iFirstLatch + ui));
	}
	while (mapcStack.NumElements() > 0)
	{
		pcBlock = mapcStack.Pop();
		if (!mpcDominators->IsReachable(pcBlock) || (*maiInnermost.Get(pcBlock->muiIndex) == iLoop))
		{
			continue;
		}
		*maiInnermost.Get(pcBlock->muiIndex) = iLoop;
		mapcBlocks.Add(pcBlock);
		for (ui = 0; ui < pcBlock->mapcPredecessors.NumElements(); ui++)
		{
			mapcStack.Push(pcBlock->mapcPredecessors.GetPtr(ui));
		}
	}
	psLoop->iNumBlocks = (int32)mapcBlocks.NumElements() - psLoop->iFirstBlock;

	//The way in.
	for (ui = 0; ui < pcHead->mapcPredecessors.NumElements(); ui++)
	{
		pcPredecessor = pcHead->mapcPredecessors.GetPtr(ui);
		if (mpcDominators->IsReachable(pcPredecessor) && !Contains(iLoop, pcPredecessor))
		{
			bSeveral |= (psLoop->pcEntering != NULL) && (psLoop->pcEntering != pcPredecessor);
			psLoop->pcEntering = pcPredecessor;
		}
	}
	if (bSeveral)
	{
		psLoop->pcEntering = NULL;
	}
	if (psLoop->pcEntering && (psLoop->pcEntering->NumSuccessors() == 1))
	{
		psLoop->pcPreheader = psLoop->pcEntering;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRLoops::NumLoops(void)
{
	return masLoops.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRLoop* CIRLoops::GetLoop(size uiLoop)
{
	return masLoops.Get(uiLoop);
}


//////////////////////////////////////////////////////////////////////////
//
// The innermost loop pcBlock is in, -1 for none.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRLoops::Innermost(CIRBlock* pcBlock)
{
	if (pcBlock->muiIndex >= maiInnermost.NumElements())
	{
		return -1;
	}
	return *maiInnermost.Get(pcBlock->muiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// How many loops pcBlock is in.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRLoops::Depth(CIRBlock* pcBlock)
{
	int32	iLoop;

	iLoop = Innermost(pcBlock);
	return (iLoop >= 0) ? masLoops.Get(iLoop)->iDepth : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcBlock is in loop uiLoop, or a loop inside it.
//
//////////////////////////////////////////////////////////////////////////
bool CIRLoops::Contains(size uiLoop, CIRBlock* pcBlock)
{
	int32	iLoop;

	for (iLoop = Innermost(pcBlock); iLoop >= (int32)uiLoop; iLoop = masLoops.Get(iLoop)->iParent)
	{
		if (iLoop == (int32)uiLoop)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRLoops::IsHead(CIRBlock* pcBlock)
{
	int32	iLoop;

	iLoop = Innermost(pcBlock);
	return (iLoop >= 0) && (masLoops.Get(iLoop)->pcHead == pcBlock);
}


//////////////////////////////////////////////////////////////////////////
//
// The blocks of a loop and the loops inside it, the head first.
//
//////////////////////////////////////////////////////////////////////////
size CIRLoops::NumBlocks(size uiLoop)
{
	return (size)masLoops.Get(uiLoop)->iNumBlocks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRLoops::GetBlock(size uiLoop, size uiIndex)
{
	return mapcBlocks.GetPtr(masLoops.Get(uiLoop)->iFirstBlock + uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The blocks that jump back to a loop's head.
//
//////////////////////////////////////////////////////////////////////////
size CIRLoops::NumLatches(size uiLoop)
{
	return (size)masLoops.Get(uiLoop)->iNumLatches;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRLoops::GetLatch(size uiLoop, size uiIndex)
{
	return mapcLatches.GetPtr(masLoops.Get(uiLoop)->iFirstLatch + uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The edges out of a loop, in the order of its blocks.
//
//////////////////////////////////////////////////////////////////////////
void CIRLoops::GetExits(size uiLoop, CArrayIRLoopExits* pasExits)
{
	size			ui;
	size			uiSuccessor;
	CIRBlock*		pcBlock;
	CIRBlock*		pcSuccessor;
	SIRLoopExit*	psExit;

	pasExits->FakeSetUsedElements(0);
	for (ui = 0; ui < NumBlocks(uiLoop); ui++)
	{
		pcBlock = GetBlock(uiLoop, ui);
		for (uiSuccessor = 0; uiSuccessor < pcBlock->NumSuccessors(); uiSuccessor++)
		{
			pcSuccessor = pcBlock->GetSuccessor(uiSuccessor);
			if (!Contains(uiLoop, pcSuccessor))
			{
				psExit = pasExits->Add();
				psExit->pcFrom = pcBlock;
				psExit->pcTo = pcSuccessor;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if the loop doesn't change sValue: a constant, a symbol, a
// parameter, or a temporary defined outside it.
//
//////////////////////////////////////////////////////////////////////////
bool CIRLoops::IsInvariant(size uiLoop, SIRReference sValue, CIRDefinitions* pcDefinitions)
{
	CIRBlock*	pcBlock;

	if (!sValue.IsTemporary())
	{
		return true;
	}
	pcBlock = pcDefinitions->Block(sValue);
	return (pcBlock == NULL) || !Contains(uiLoop, pcBlock);
}


//////////////////////////////////////////////////////////////////////////
//
// pcPreheader, a block just added between a loop's entering block and its
// head, is the loop's way in now.  It is in the loops around the loop (but
// not in their lists of blocks).
//
//////////////////////////////////////////////////////////////////////////
void CIRLoops::SetPreheader(size uiLoop, CIRBlock* pcPreheader)
{
	SIRLoop*	psLoop;

	psLoop = masLoops.Get(uiLoop);
	psLoop->pcEntering = pcPreheader;
	psLoop->pcPreheader = pcPreheader;
	maiInnermost.GrowToAtLeastNumElements(pcPreheader->muiIndex + 1, true, 0xFF);
	*maiInnermost.Get(pcPreheader->muiIndex) = psLoop->iParent;
}
