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


//The optimiser's control flow passes: unreachable and empty blocks removed, known branches folded and threaded, blocks
//joined, trivial phis and dead code removed.


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
	SIRPhi*			psBlockPhi;

	for (uiPhi = 0; uiPhi < pcTarget->masPhis.NumElements(); uiPhi++)
	{
		psArgument = pcTarget->FindPhiArgument(pcTarget->masPhis.Get(uiPhi), pcBlock);
		sValue = psArgument ? psArgument->sValue : IRNone();
		psBlockPhi = pcBlock->FindPhi(sValue);
		if (psBlockPhi)
		{
			//One of pcBlock's phis: what pcPredecessor gives it.
			psThrough = pcBlock->FindPhiArgument(psBlockPhi, pcPredecessor);
			sValue = psThrough ? psThrough->sValue : IRNone();
		}
		pcTarget->AddPhiArgument(pcTarget->masPhis.Get(uiPhi), pcPredecessor, sValue);
	}
	RedirectJump(pcPredecessor, pcBlock, pcTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks the entry can't reach go; their arguments to reachable blocks' phis
// too.  The blocks must be numbered (CIRFunction::Renumber); the entry is
// the first.  A search from the entry finds the reachable ones: the
// dominators aren't needed.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RemoveUnreachable(void)
{
	size			ui;
	size			uiSuccessor;
	size			uiBlocks;
	size			uiReached;
	size			uiTop;
	CIRBlock*		pcBlock;
	CIRBlock*		pcSuccessor;
	CArrayIRBlocks	apcUnreachable;

	uiBlocks = mpcFunction->NumBlocks();
	if (uiBlocks == 0)
	{
		return false;
	}

	//Each block is put on the search once, when it is first reached, so the search needs no more than a place per block.
	mabReached.FakeSetUsedElements(0);
	mabReached.GrowToAtLeastNumElements(uiBlocks, true, 0);
	maiReachSearch.FakeSetUsedElements(0);
	maiReachSearch.GrowToAtLeastNumElements(uiBlocks);
	*mabReached.Get(0) = 1;
	*maiReachSearch.Get(0) = 0;
	uiTop = 1;
	uiReached = 1;
	while (uiTop > 0)
	{
		uiTop--;
		pcBlock = mpcFunction->GetBlock(*maiReachSearch.Get(uiTop));
		for (uiSuccessor = 0; uiSuccessor < pcBlock->NumSuccessors(); uiSuccessor++)
		{
			pcSuccessor = pcBlock->GetSuccessor(uiSuccessor);
			if (pcSuccessor && (*mabReached.Get(pcSuccessor->muiIndex) == 0))
			{
				*mabReached.Get(pcSuccessor->muiIndex) = 1;
				*maiReachSearch.Get(uiTop) = (int32)pcSuccessor->muiIndex;
				uiTop++;
				uiReached++;
			}
		}
	}
	if (uiReached == uiBlocks)
	{
		return false;
	}

	apcUnreachable.Init();
	for (ui = 0; ui < uiBlocks; ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if (*mabReached.Get(ui) == 0)
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

	if (!pcSuccessor || (pcSuccessor == pcBlock))
	{
		return;
	}
	for (ui = 0; ui < pcSuccessor->masPhis.NumElements(); ui++)
	{
		psArgument = pcSuccessor->FindPhiArgument(pcSuccessor->masPhis.Get(ui), pcBlock);
		if (psArgument && psArgument->sValue.IsTemporary() && pcBlock->FindPhi(psArgument->sValue))
		{
			(*pauiInside->Get(psArgument->sValue.Index()))++;
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
	CArrayIRBlocks	apcBlocks;
	bool			bOnly;

	apcBlocks.Init();
	apcBlocks.Add(pcBlock);
	bOnly = OnlyUsedIn(&apcBlocks, bSuccessorPhis);
	apcBlocks.Kill();
	return bOnly;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the phis and results of papcBlocks are only used in them (by
// their instructions, jumps or phis): every use mauiUses counts of each is
// one of theirs.  With bSuccessorPhis a block's phis may also be the values
// its successors' phis take from it (CountSuccessorPhiUses).  mauiUses must
// be counted.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::OnlyUsedIn(CArrayIRBlocks* papcBlocks, bool bSuccessorPhis)
{
	CArrayTemplate<uint32>	auiInside;
	CIRBlock*				pcBlock;
	SIRReference			sValue;
	SIRUse					sUse;
	size					ui;
	size					uiIndex;
	bool					bUse;
	bool					bOnly;

	auiInside.Init();
	auiInside.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	for (bUse = FirstTemporaryUse(&sUse, papcBlocks); bUse; bUse = NextUse(&sUse))
	{
		(*auiInside.Get(sUse.psSlot->Index()))++;
	}
	for (ui = 0; bSuccessorPhis && (ui < papcBlocks->NumElements()); ui++)
	{
		pcBlock = papcBlocks->GetPtr(ui);
		CountSuccessorPhiUses(pcBlock, pcBlock->msJump.pcThen, &auiInside);
		if (pcBlock->msJump.pcElse != pcBlock->msJump.pcThen)
		{
			CountSuccessorPhiUses(pcBlock, pcBlock->msJump.pcElse, &auiInside);
		}
	}

	bOnly = true;
	for (ui = 0; bOnly && (ui < papcBlocks->NumElements()); ui++)
	{
		pcBlock = papcBlocks->GetPtr(ui);
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
		psPhi = pcBlock->FindPhi(pcBlock->msJump.sArgument);

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
	bool			bPhis;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		pcNext = pcBlock->msJump.pcThen;
		if ((pcBlock->msJump.uiKind != IRJ_Jump) || (pcNext == pcBlock) || (pcNext == mpcFunction->GetEntry()) || (pcNext->mapcPredecessors.NumElements() != 1))
		{
			continue;
		}

		bPhis = pcNext->masPhis.NumElements() != 0;
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

		//The phis' uses are their values now: the steps after this one (ThreadPhis' OnlyUsedInBlock) count uses.
		if (bPhis)
		{
			ApplyReplacements();
		}
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
			if (IRHasEffect(psInstruction) || !psInstruction->sResult.IsTemporary())
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
			if (!IRHasEffect(psInstruction) && psInstruction->sResult.IsTemporary() && !*abLive.Get(psInstruction->sResult.Index()))
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
