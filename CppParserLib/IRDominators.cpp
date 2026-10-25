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
#include "IRDominators.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRDominators::Init(void)
{
	mpcFunction = NULL;
	mbLastFirst = false;
	maiImmediate.Init();
	maiPostOrder.Init();
	mapcReversePostOrder.Init();
	maiTreePre.Init();
	maiTreePost.Init();
	maiChildStart.Init();
	maiChildren.Init();
	masStack.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRDominators::Kill(void)
{
	masStack.Kill();
	maiChildren.Kill();
	maiChildStart.Kill();
	maiTreePost.Kill();
	maiTreePre.Kill();
	mapcReversePostOrder.Kill();
	maiPostOrder.Kill();
	maiImmediate.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// uiNumBlocks entries of -1, keeping the array's memory.
//
//////////////////////////////////////////////////////////////////////////
void CIRDominators::Reset(CArrayIRBlockNumbers* paiNumbers, size uiNumBlocks)
{
	paiNumbers->FakeSetUsedElements(0);
	paiNumbers->GrowToAtLeastNumElements(uiNumBlocks, true, 0xFF);
}


//////////////////////////////////////////////////////////////////////////
//
// bLastFirst orders the search (and so the reverse post order) with a
// brif's else block before its then block, which puts the then block
// straight after the brif; the dominators are the same either way.
//
//////////////////////////////////////////////////////////////////////////
void CIRDominators::Compute(CIRFunction* pcFunction, bool bLastFirst)
{
	size	uiNumBlocks;

	mpcFunction = pcFunction;
	mbLastFirst = bLastFirst;
	uiNumBlocks = pcFunction->NumBlocks();
	Reset(&maiImmediate, uiNumBlocks);
	Reset(&maiPostOrder, uiNumBlocks);
	Reset(&maiTreePre, uiNumBlocks);
	Reset(&maiTreePost, uiNumBlocks);
	mapcReversePostOrder.FakeSetUsedElements(0);
	if (uiNumBlocks == 0)
	{
		return;
	}

	ComputePostOrder();
	ComputeImmediate();
	NumberTree();
}


//////////////////////////////////////////////////////////////////////////
//
// Post order by depth first search from the entry, without recursion.
//
//////////////////////////////////////////////////////////////////////////
void CIRDominators::ComputePostOrder(void)
{
	SIRSearch*	psTop;
	SIRSearch*	psNew;
	CIRBlock*	pcBlock;
	CIRBlock*	pcSuccessor;
	int32		iPostOrder;

	masStack.FakeSetUsedElements(0);
	iPostOrder = 0;
	psNew = masStack.Add();
	psNew->iBlock = 0;
	psNew->iNext = 0;
	*maiPostOrder.Get(0) = -2;
	while (masStack.NumElements() > 0)
	{
		psTop = masStack.Tail();
		pcBlock = mpcFunction->GetBlock(psTop->iBlock);
		if ((size)psTop->iNext < pcBlock->NumSuccessors())
		{
			pcSuccessor = pcBlock->GetSuccessor(mbLastFirst ? pcBlock->NumSuccessors() - 1 - (size)psTop->iNext : (size)psTop->iNext);
			psTop->iNext++;
			if (pcSuccessor && (*maiPostOrder.Get(pcSuccessor->muiIndex) == -1))
			{
				*maiPostOrder.Get(pcSuccessor->muiIndex) = -2;
				psNew = masStack.Add();
				psNew->iBlock = (int32)pcSuccessor->muiIndex;
				psNew->iNext = 0;
			}
		}
		else
		{
			*maiPostOrder.Get(pcBlock->muiIndex) = iPostOrder;
			iPostOrder++;
			mapcReversePostOrder.Add(pcBlock);
			masStack.Pop();
		}
	}
	mapcReversePostOrder.Reverse();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRDominators::ComputeImmediate(void)
{
	size		ui;
	size		uiPredecessor;
	CIRBlock*	pcBlock;
	int32		iNew;
	int32		iPredecessor;
	bool		bChanged;

	*maiImmediate.Get(0) = 0;
	do
	{
		bChanged = false;
		for (ui = 1; ui < mapcReversePostOrder.NumElements(); ui++)
		{
			pcBlock = mapcReversePostOrder.GetPtr(ui);
			iNew = -1;
			for (uiPredecessor = 0; uiPredecessor < pcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
			{
				iPredecessor = (int32)pcBlock->mapcPredecessors.GetPtr(uiPredecessor)->muiIndex;
				if (*maiImmediate.Get(iPredecessor) != -1)
				{
					iNew = (iNew == -1) ? iPredecessor : Intersect(iPredecessor, iNew);
				}
			}
			if (*maiImmediate.Get(pcBlock->muiIndex) != iNew)
			{
				*maiImmediate.Get(pcBlock->muiIndex) = iNew;
				bChanged = true;
			}
		}
	} while (bChanged);
}


//////////////////////////////////////////////////////////////////////////
//
// Numbers the dominator tree in pre and post order: a dominates b when a's
// numbers enclose b's.
//
//////////////////////////////////////////////////////////////////////////
void CIRDominators::NumberTree(void)
{
	size		ui;
	size		uiNumBlocks;
	int32		iParent;
	int32*		piStart;
	SIRSearch*	psTop;
	SIRSearch*	psNew;
	int32		iPre;
	int32		iPost;
	int32		iChild;

	//Children of each block, grouped by counting.
	uiNumBlocks = mpcFunction->NumBlocks();
	maiChildStart.FakeSetUsedElements(0);
	maiChildStart.GrowToAtLeastNumElements(uiNumBlocks + 1, true, 0);
	for (ui = 1; ui < uiNumBlocks; ui++)
	{
		iParent = *maiImmediate.Get(ui);
		if (iParent != -1)
		{
			(*maiChildStart.Get(iParent + 1))++;
		}
	}
	for (ui = 1; ui <= uiNumBlocks; ui++)
	{
		*maiChildStart.Get(ui) += *maiChildStart.Get(ui - 1);
	}
	maiChildren.FakeSetUsedElements(0);
	maiChildren.GrowToAtLeastNumElements(*maiChildStart.Get(uiNumBlocks) + 1, true, 0);
	for (ui = 1; ui < uiNumBlocks; ui++)
	{
		iParent = *maiImmediate.Get(ui);
		if (iParent != -1)
		{
			piStart = maiChildStart.Get(iParent);
			*maiChildren.Get(*piStart) = (int32)ui;
			(*piStart)++;
		}
	}
	//maiChildStart[b] is now where b's children end and b + 1's start; b's start is maiChildStart[b - 1] (0 for b 0).

	masStack.FakeSetUsedElements(0);
	iPre = 0;
	iPost = 0;
	psNew = masStack.Add();
	psNew->iBlock = 0;
	psNew->iNext = 0;
	*maiTreePre.Get(0) = iPre++;
	while (masStack.NumElements() > 0)
	{
		psTop = masStack.Tail();
		iChild = ((psTop->iBlock == 0) ? 0 : *maiChildStart.Get(psTop->iBlock - 1)) + psTop->iNext;
		if (iChild < *maiChildStart.Get(psTop->iBlock))
		{
			psTop->iNext++;
			iChild = *maiChildren.Get(iChild);
			*maiTreePre.Get(iChild) = iPre++;
			psNew = masStack.Add();
			psNew->iBlock = iChild;
			psNew->iNext = 0;
		}
		else
		{
			*maiTreePost.Get(psTop->iBlock) = iPost++;
			masStack.Pop();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIRDominators::Intersect(int32 iBlock1, int32 iBlock2)
{
	while (iBlock1 != iBlock2)
	{
		while (*maiPostOrder.Get(iBlock1) < *maiPostOrder.Get(iBlock2))
		{
			iBlock1 = *maiImmediate.Get(iBlock1);
		}
		while (*maiPostOrder.Get(iBlock2) < *maiPostOrder.Get(iBlock1))
		{
			iBlock2 = *maiImmediate.Get(iBlock2);
		}
	}
	return iBlock1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRDominators::IsReachable(CIRBlock* pcBlock)
{
	return *maiPostOrder.Get(pcBlock->muiIndex) >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL for the entry and unreachable blocks.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRDominators::GetImmediate(CIRBlock* pcBlock)
{
	int32	iImmediate;

	iImmediate = *maiImmediate.Get(pcBlock->muiIndex);
	if ((iImmediate == -1) || (iImmediate == (int32)pcBlock->muiIndex))
	{
		return NULL;
	}
	return mpcFunction->GetBlock(iImmediate);
}


//////////////////////////////////////////////////////////////////////////
//
// A block dominates itself.  Nothing dominates an unreachable block.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDominators::Dominates(CIRBlock* pcDominator, CIRBlock* pcBlock)
{
	if (!IsReachable(pcBlock) || !IsReachable(pcDominator))
	{
		return false;
	}
	return (*maiTreePre.Get(pcDominator->muiIndex) <= *maiTreePre.Get(pcBlock->muiIndex)) &&
		   (*maiTreePost.Get(pcBlock->muiIndex) <= *maiTreePost.Get(pcDominator->muiIndex));
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcFrom jumping to pcHead goes round a loop: pcHead dominates it.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDominators::IsBackEdge(CIRBlock* pcFrom, CIRBlock* pcHead)
{
	return Dominates(pcHead, pcFrom);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRDominators::NumReachable(void)
{
	return mapcReversePostOrder.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRDominators::GetReversePostOrder(size uiIndex)
{
	return mapcReversePostOrder.GetPtr(uiIndex);
}
