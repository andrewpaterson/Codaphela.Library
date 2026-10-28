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
#include "BaseLib/IntegerHelper.h"
#include "IROptimiser.h"


#define IR_OPTIMISE_ROUNDS		16


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::Init(CIRModule* pcModule)
{
	mpcModule = pcModule;
	mpcFunction = NULL;
	mcDominators.Init();
	masReplacements.Init();
	mauiUses.Init();
	masPromotions.Init();
	maiPromotion.Init();
	maiPhiPromotion.Init();
	masValues.Init();
	maiValueLinks.Init();
	maiTops.Init();
	maiPushedPromotions.Init();
	mapcFrontiers.Init();
	maiFrontierStarts.Init();
	maiChildren.Init();
	maiChildStarts.Init();
	mapsDefinitions.Init();
	mapsPhis.Init();
	masArgAddresses.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::Kill(void)
{
	masArgAddresses.Kill();
	mapsPhis.Kill();
	mapsDefinitions.Kill();
	maiChildStarts.Kill();
	maiChildren.Kill();
	maiFrontierStarts.Kill();
	mapcFrontiers.Kill();
	maiPushedPromotions.Kill();
	maiTops.Kill();
	maiValueLinks.Kill();
	masValues.Kill();
	maiPhiPromotion.Kill();
	maiPromotion.Kill();
	masPromotions.Kill();
	mauiUses.Kill();
	masReplacements.Kill();
	mcDominators.Kill();
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
	bAny = false;
	for (iRound = 0; iRound < IR_OPTIMISE_ROUNDS; iRound++)
	{
		bChanged = SimplifyCFG();
		bChanged |= SplitSlots();
		bChanged |= PromoteSlots();
		bChanged |= RemoveTrivialPhis();
		bChanged |= FoldConstants();
		bChanged |= RemoveCommonSubexpressions();
		bChanged |= RemoveRedundantLoads();
		bChanged |= LessLoopTests();
		bChanged |= ReduceStrength();
		bChanged |= HoistInvariants();
		bChanged |= RemoveDeadCode();
		if (!bChanged)
		{
			break;
		}
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
// Stores, args, calls and volatile loads do something besides make a value.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::HasEffect(SIRInstruction* psInstruction)
{
	switch (psInstruction->uiOpcode)
	{
	case IRO_Store:
	case IRO_Arg:
	case IRO_Call:
		return true;
	case IRO_Load:
		return (psInstruction->uiFlags & IRF_Volatile) != 0;
	default:
		return psInstruction->uiOpcode >= IRO_NumGeneric;
	}
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
		bChanged = RemoveUnreachable() || FoldBranches() || SkipEmptyBlocks() || ThreadBranches() || DuplicateReturns() || JoinBlocks();
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

	for (uiPhi = 0; uiPhi < pcTarget->masPhis.NumElements(); uiPhi++)
	{
		psArgument = pcTarget->FindPhiArgument(pcTarget->masPhis.Get(uiPhi), pcBlock);
		pcTarget->AddPhiArgument(pcTarget->masPhis.Get(uiPhi), pcPredecessor, psArgument ? psArgument->sValue : IRNone());
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
// Instructions and phis without effects whose results nothing uses go.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RemoveDeadCode(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	bool			bChanged;
	bool			bAny;

	bAny = false;
	do
	{
		bChanged = false;
		mpcFunction->CountUses(&mauiUses);
		for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uiInstruction = pcBlock->masInstructions.NumElements(); uiInstruction > 0; uiInstruction--)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction - 1);
				if (!HasEffect(psInstruction) && psInstruction->sResult.IsTemporary() && (*mauiUses.Get(psInstruction->sResult.Index()) == 0))
				{
					pcBlock->masInstructions.RemoveAt(uiInstruction - 1, true);
					bChanged = true;
				}
			}
			for (uiInstruction = pcBlock->masPhis.NumElements(); uiInstruction > 0; uiInstruction--)
			{
				psPhi = pcBlock->masPhis.Get(uiInstruction - 1);
				if (*mauiUses.Get(psPhi->sResult.Index()) == 0)
				{
					psPhi->asArguments.Kill();
					pcBlock->masPhis.RemoveAt(uiInstruction - 1, true);
					bChanged = true;
				}
			}
		}
		bAny |= bChanged;
	} while (bChanged);
	return bAny;
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
	ComputeFrontiers();
	ComputeChildren();
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
// Each block's dominance frontier (Cooper, Harvey and Kennedy): the join
// blocks it reaches before its dominance ends.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::ComputeFrontiers(void)
{
	size					ui;
	size					uiPredecessor;
	size					uiNumBlocks;
	CIRBlock*				pcBlock;
	CIRBlock*				pcRunner;
	CIRBlock*				pcIdom;
	CArrayTemplate<int32>	aiPairs;		//Block index, frontier block index, ...
	CArrayTemplate<int32>	aiStamps;
	int32					iPair[2];
	int32*					piStart;

	uiNumBlocks = mpcFunction->NumBlocks();
	aiPairs.Init();
	aiStamps.Init();
	aiStamps.GrowToAtLeastNumElements(uiNumBlocks, true, 0xFF);
	for (ui = 0; ui < uiNumBlocks; ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if ((pcBlock->mapcPredecessors.NumElements() < 2) || !mcDominators.IsReachable(pcBlock))
		{
			continue;
		}
		pcIdom = mcDominators.GetImmediate(pcBlock);
		for (uiPredecessor = 0; uiPredecessor < pcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			pcRunner = pcBlock->mapcPredecessors.GetPtr(uiPredecessor);
			while (pcRunner && (pcRunner != pcIdom) && mcDominators.IsReachable(pcRunner))
			{
				if (*aiStamps.Get(pcRunner->muiIndex) != (int32)ui)
				{
					*aiStamps.Get(pcRunner->muiIndex) = (int32)ui;
					iPair[0] = (int32)pcRunner->muiIndex;
					iPair[1] = (int32)ui;
					aiPairs.Add(&iPair[0]);
					aiPairs.Add(&iPair[1]);
				}
				pcRunner = mcDominators.GetImmediate(pcRunner);
			}
		}
	}

	//Grouped by block: maiFrontierStarts[b] to [b + 1].
	maiFrontierStarts.FakeSetUsedElements(0);
	maiFrontierStarts.GrowToAtLeastNumElements(uiNumBlocks + 1, true, 0);
	for (ui = 0; ui < aiPairs.NumElements(); ui += 2)
	{
		(*maiFrontierStarts.Get(*aiPairs.Get(ui) + 1))++;
	}
	for (ui = 1; ui <= uiNumBlocks; ui++)
	{
		*maiFrontierStarts.Get(ui) += *maiFrontierStarts.Get(ui - 1);
	}
	mapcFrontiers.FakeSetUsedElements(0);
	mapcFrontiers.GrowToAtLeastNumElements(aiPairs.NumElements() / 2 + 1, true, 0);
	aiStamps.FakeSetUsedElements(0);
	aiStamps.GrowToAtLeastNumElements(uiNumBlocks, true, 0);
	for (ui = 0; ui < aiPairs.NumElements(); ui += 2)
	{
		piStart = aiStamps.Get(*aiPairs.Get(ui));
		*mapcFrontiers.Get(*maiFrontierStarts.Get(*aiPairs.Get(ui)) + *piStart) = mpcFunction->GetBlock(*aiPairs.Get(ui + 1));
		(*piStart)++;
	}
	aiStamps.Kill();
	aiPairs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The dominator tree's children of each block, from maiChildStarts.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::ComputeChildren(void)
{
	size					ui;
	size					uiNumBlocks;
	CIRBlock*				pcParent;
	int32*					piNext;
	CArrayTemplate<int32>	aiNext;

	uiNumBlocks = mpcFunction->NumBlocks();
	maiChildStarts.FakeSetUsedElements(0);
	maiChildStarts.GrowToAtLeastNumElements(uiNumBlocks + 1, true, 0);
	for (ui = 0; ui < uiNumBlocks; ui++)
	{
		pcParent = mcDominators.GetImmediate(mpcFunction->GetBlock(ui));
		if (pcParent)
		{
			(*maiChildStarts.Get(pcParent->muiIndex + 1))++;
		}
	}
	for (ui = 1; ui <= uiNumBlocks; ui++)
	{
		*maiChildStarts.Get(ui) += *maiChildStarts.Get(ui - 1);
	}
	maiChildren.FakeSetUsedElements(0);
	maiChildren.GrowToAtLeastNumElements(*maiChildStarts.Get(uiNumBlocks) + 1, true, 0);
	aiNext.Init();
	aiNext.GrowToAtLeastNumElements(uiNumBlocks, true, 0);
	for (ui = 0; ui < uiNumBlocks; ui++)
	{
		pcParent = mcDominators.GetImmediate(mpcFunction->GetBlock(ui));
		if (pcParent)
		{
			piNext = aiNext.Get(pcParent->muiIndex);
			*maiChildren.Get(*maiChildStarts.Get(pcParent->muiIndex) + *piNext) = (int32)ui;
			(*piNext)++;
		}
	}
	aiNext.Kill();
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
			for (uiFrontier = *maiFrontierStarts.Get(pcBlock->muiIndex); uiFrontier < (size)*maiFrontierStarts.Get(pcBlock->muiIndex + 1); uiFrontier++)
			{
				pcFrontier = mapcFrontiers.GetPtr(uiFrontier);
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
		if (psFrame->uiNextChild + *maiChildStarts.Get(psFrame->pcBlock->muiIndex) < (size)*maiChildStarts.Get(psFrame->pcBlock->muiIndex + 1))
		{
			pcChild = mpcFunction->GetBlock(*maiChildren.Get(*maiChildStarts.Get(psFrame->pcBlock->muiIndex) + psFrame->uiNextChild));
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
// extended values are narrowed (NarrowComparison) and extensions that are
// undone go (UndoesExtension, ConvertPhi).
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
			else if (UndoesExtension(psInstruction, &sValue) || ConvertPhi(psInstruction, &sValue))
			{
				//The value's uses go up by the result's (ConvertPhi mustn't change a phi something else now uses).
				Replace(psInstruction->sResult, sValue);
				if (sValue.IsTemporary() && psInstruction->sResult.IsTemporary())
				{
					*mauiUses.Get(sValue.Index()) += *mauiUses.Get(psInstruction->sResult.Index());
				}
				bChanged = true;
			}
			else if (((psInstruction->uiOpcode == IRO_Mul) || (psInstruction->uiOpcode == IRO_UDiv) || (psInstruction->uiOpcode == IRO_URem)) && psInstruction->asArguments[1].IsConstant())
			{
				//x * 2^n is x << n; unsigned x / 2^n is x >> n and x % 2^n is x & (2^n - 1).
				ulliResult = IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
				if ((ulliResult > 1) && ((ulliResult & (ulliResult - 1)) == 0))
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
	psDefinition = Definition(psInstruction->asArguments[0]);
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

	if (!IsIRIntegerConversion(psInstruction->uiOpcode) || !psInstruction->sResult.IsTemporary() || !psInstruction->asArguments[0].IsTemporary() || (psInstruction->asArguments[0].Index() >= mapsPhis.NumElements()))
	{
		return false;
	}
	psPhi = *mapsPhis.Get(psInstruction->asArguments[0].Index());
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
	psDefinition = Definition(sValue);
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
	psPhi = (sValue.Index() < mapsPhis.NumElements()) ? *mapsPhis.Get(sValue.Index()) : NULL;
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
	psKnown = ((psJump->uiKind == IRJ_Branch) && (psJump->pcThen != psJump->pcElse)) ? Definition(psJump->sArgument) : NULL;
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
// True if two instructions work out the same value.
//
//////////////////////////////////////////////////////////////////////////
static bool IROptimiserSameInstruction(CIRFunction* pcFunction, SIRInstruction* psEarlier, SIRInstruction* psInstruction)
{
	return (psEarlier->uiOpcode == psInstruction->uiOpcode) && psEarlier->sType.Equals(psInstruction->sType) && (psEarlier->uiFlags == psInstruction->uiFlags) &&
		psEarlier->asArguments[0].Equals(psInstruction->asArguments[0]) && psEarlier->asArguments[1].Equals(psInstruction->asArguments[1]) &&
		psEarlier->sResult.IsTemporary() && pcFunction->GetTemporaryType(psEarlier->sResult).Equals(pcFunction->GetTemporaryType(psInstruction->sResult));
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
			if (HasEffect(psInstruction) || (psInstruction->uiOpcode == IRO_Load) || (psInstruction->uiOpcode == IRO_Slot) || (psInstruction->uiOpcode == IRO_Arg) || !psInstruction->sResult.IsTemporary())
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
// Each temporary's defining instruction (mapsDefinitions), NULL for
// parameters and phis, its phi (mapsPhis), and each parameter's first
// argaddr.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::FindDefinitions(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	size			uiParameter;

	mapsDefinitions.FakeSetUsedElements(0);
	mapsDefinitions.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
	mapsPhis.FakeSetUsedElements(0);
	mapsPhis.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
	masArgAddresses.FakeSetUsedElements(0);
	masArgAddresses.GrowToAtLeastNumElements(mpcFunction->masParameters.NumElements(), true, 0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masPhis.NumElements(); uiInstruction++)
		{
			psPhi = pcBlock->masPhis.Get(uiInstruction);
			*mapsPhis.Get(psPhi->sResult.Index()) = psPhi;
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->sResult.IsTemporary())
			{
				*mapsDefinitions.Get(psInstruction->sResult.Index()) = psInstruction;
				uiParameter = (psInstruction->uiOpcode == IRO_ArgAddress) ? (size)mpcFunction->GetSigned(psInstruction->asArguments[0]) : SIZE_MAX;
				if ((uiParameter < masArgAddresses.NumElements()) && masArgAddresses.Get(uiParameter)->IsNone())
				{
					*masArgAddresses.Get(uiParameter) = psInstruction->sResult;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction FindDefinitions found defining sValue; NULL if it isn't
// a temporary an instruction defines.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIROptimiser::Definition(SIRReference sValue)
{
	if (!sValue.IsTemporary() || (sValue.Index() >= mapsDefinitions.NumElements()))
	{
		return NULL;
	}
	return *mapsDefinitions.Get(sValue.Index());
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
		psDefinition = Definition(*psRoot);
		if ((psDefinition == NULL) || (psDefinition->uiOpcode != IRO_PtrAdd) || !psDefinition->asArguments[1].IsConstant())
		{
			break;
		}
		*plliOffset += mpcFunction->GetSigned(psDefinition->asArguments[1]);
		*psRoot = psDefinition->asArguments[0];
	}

	//Every argaddr of a parameter is the same memory: the first is its root.
	psDefinition = Definition(*psRoot);
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

	psDefinition = Definition(sRoot);
	return psDefinition && ((psDefinition->uiOpcode == IRO_Slot) || (psDefinition->uiOpcode == IRO_ArgAddress));
}


//////////////////////////////////////////////////////////////////////////
//
// False only if the two accesses can't touch the same bytes: the same root
// at offsets that don't overlap, different symbols, different slots or
// argaddrs, or a slot and a symbol.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::MayAlias(SIRAvailable* psAvailable, SIRReference sRoot, int64 lliOffset, int iBytes)
{
	bool	bFrame1;
	bool	bFrame2;

	if (psAvailable->sRoot.Equals(sRoot))
	{
		return (psAvailable->lliOffset < lliOffset + iBytes) && (lliOffset < psAvailable->lliOffset + mpcModule->GetSize(psAvailable->sType));
	}
	bFrame1 = IsFrameRoot(psAvailable->sRoot);
	bFrame2 = IsFrameRoot(sRoot);
	if ((bFrame1 || psAvailable->sRoot.IsSymbol()) && (bFrame2 || sRoot.IsSymbol()))
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

	psDefinition = Definition(sValue);
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
// The slot and constant offset an address is (a slot, or a ptradd of one
// and a constant); false if it is neither.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SlotOffset(SIRReference sAddress, SIRReference* psSlot, int64* plliOffset)
{
	SIRInstruction*	psDefinition;

	psDefinition = Definition(sAddress);
	if (psDefinition && (psDefinition->uiOpcode == IRO_PtrAdd) && psDefinition->asArguments[1].IsConstant())
	{
		*plliOffset = mpcFunction->GetSigned(psDefinition->asArguments[1]);
		psDefinition = Definition(psDefinition->asArguments[0]);
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
		if ((psAccess->lliOffset < 0) || (psAccess->lliOffset + mpcModule->GetSize(psAccess->sType) > mpcFunction->GetSigned(Definition(psAccess->sSlot)->asArguments[0])))
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
// A block between pcEntering and the loop head pcHead, which pcEntering's
// branch now goes to and which jumps to the head.
//
//////////////////////////////////////////////////////////////////////////
//
// The loop pcHead heads, if a block jumps back to it: its blocks are marked
// with pcHead's index in paiLoop (found from the blocks that jump back),
// and *ppcEntering is the one block outside that enters it (pcHead when
// several do, NULL when none).  False if pcHead heads no loop.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::FindLoop(CIRBlock* pcHead, CArrayTemplate<int32>* paiLoop, CArrayIRBlocks* papcStack, CIRBlock** ppcEntering)
{
	size		uiPredecessor;
	int32		iHead;
	CIRBlock*	pcBlock;
	CIRBlock*	pcPredecessor;

	iHead = (int32)pcHead->muiIndex;
	*ppcEntering = NULL;
	*paiLoop->Get(iHead) = iHead;
	papcStack->FakeSetUsedElements(0);
	for (uiPredecessor = 0; uiPredecessor < pcHead->mapcPredecessors.NumElements(); uiPredecessor++)
	{
		pcPredecessor = pcHead->mapcPredecessors.GetPtr(uiPredecessor);
		if (mcDominators.IsBackEdge(pcPredecessor, pcHead))
		{
			papcStack->Push(pcPredecessor);
		}
		else
		{
			*ppcEntering = *ppcEntering ? pcHead : pcPredecessor;
		}
	}
	if (papcStack->NumElements() == 0)
	{
		return false;
	}
	while (papcStack->NumElements() > 0)
	{
		pcBlock = papcStack->Pop();
		if (*paiLoop->Get(pcBlock->muiIndex) == iHead)
		{
			continue;
		}
		*paiLoop->Get(pcBlock->muiIndex) = iHead;
		for (uiPredecessor = 0; uiPredecessor < pcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			papcStack->Push(pcBlock->mapcPredecessors.GetPtr(uiPredecessor));
		}
	}
	return true;
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
	psDefinition = Definition(sValue);
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
	CArrayTemplate<int32>	aiLoop;			//Per block index: the head's index of the loop it was last found in.
	CArrayTemplate<int32>	aiDefined;		//Per temporary: the index of the block that defines it, -1 for a parameter.
	CArrayIRBlocks			apcStack;
	size					ui;
	size					uiBlock;
	int32					iHead;
	CIRBlock*				pcHead;
	CIRBlock*				pcBlock;
	CIRBlock*				pcEntering;
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

	mpcFunction->Renumber();
	mpcFunction->ComputePredecessors();
	mcDominators.Compute(mpcFunction);
	FindDefinitions();
	aiDefined.Init();
	FindDefiningBlocks(&aiDefined);
	aiLoop.Init();
	aiLoop.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0xFF);
	apcStack.Init();
	bChanged = false;
	for (ui = 0; ui < mcDominators.NumReachable(); ui++)
	{
		pcHead = mcDominators.GetReversePostOrder(ui);
		iHead = (int32)pcHead->muiIndex;
		if (!FindLoop(pcHead, &aiLoop, &apcStack, &pcEntering))
		{
			continue;
		}
		for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
		{
			pcBlock = mpcFunction->GetBlock(uiBlock);
			if ((*aiLoop.Get(uiBlock) != iHead) || (pcBlock->msJump.uiKind != IRJ_Branch) || !pcBlock->msJump.sArgument.IsTemporary())
			{
				continue;
			}
			psCompare = Definition(pcBlock->msJump.sArgument);
			if ((psCompare == NULL) || !IsIRSignedComparison(psCompare->uiOpcode))
			{
				continue;
			}

			//The way round the loop and the way out.
			if ((*aiLoop.Get(pcBlock->msJump.pcThen->muiIndex) == iHead) && (*aiLoop.Get(pcBlock->msJump.pcElse->muiIndex) != iHead))
			{
				pcInside = pcBlock->msJump.pcThen;
				pcOutside = pcBlock->msJump.pcElse;
				uiOpcode = psCompare->uiOpcode;
			}
			else if ((*aiLoop.Get(pcBlock->msJump.pcElse->muiIndex) == iHead) && (*aiLoop.Get(pcBlock->msJump.pcThen->muiIndex) != iHead))
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
			if (!IsLoopInvariant(sB, &aiDefined, &aiLoop, iHead) || !CanStep(sB, 1))
			{
				if (!IsLoopInvariant(sA, &aiDefined, &aiLoop, iHead) || !CanStep(sA, -1))
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

			//The instructions added may have moved the block's, which the definitions point at.
			FindDefinitions();
		}
	}
	apcStack.Kill();
	aiLoop.Kill();
	aiDefined.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is a constant, a parameter or worked out outside the
// loop iHead heads.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::IsLoopInvariant(SIRReference sValue, CArrayTemplate<int32>* paiDefined, CArrayTemplate<int32>* paiLoop, int32 iHead)
{
	int32	iDefined;

	if (!sValue.IsTemporary())
	{
		return true;
	}
	iDefined = *paiDefined->Get(sValue.Index());
	return (iDefined < 0) || (*paiLoop->Get(iDefined) != iHead);
}


//////////////////////////////////////////////////////////////////////////
//
// Per temporary: the index of the block that defines it, -1 for a
// parameter.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::FindDefiningBlocks(CArrayTemplate<int32>* paiDefined)
{
	size		ui;
	size		uiBlock;
	CIRBlock*	pcBlock;

	paiDefined->FakeSetUsedElements(0);
	paiDefined->GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0xFF);
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			*paiDefined->Get(pcBlock->masPhis.Get(ui)->sResult.Index()) = (int32)uiBlock;
		}
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			if (pcBlock->masInstructions.Get(ui)->sResult.IsTemporary())
			{
				*paiDefined->Get(pcBlock->masInstructions.Get(ui)->sResult.Index()) = (int32)uiBlock;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
CIRBlock* CIROptimiser::AddPreheader(CIRBlock* pcEntering, CIRBlock* pcHead)
{
	CIRBlock*	pcPreheader;

	pcPreheader = mpcFunction->AddBlock();
	pcPreheader->Jump(pcHead);
	RedirectJump(pcEntering, pcHead, pcPreheader);
	RenamePhiArguments(pcHead, pcEntering, pcPreheader);
	mpcFunction->ComputePredecessors();
	return pcPreheader;
}


//////////////////////////////////////////////////////////////////////////
//
// A new instruction at the end of pcBlock's instructions; sType is its
// result's (ptr for a ptradd).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::AppendInstruction(CIRBlock* pcBlock, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond)
{
	SIRInstruction*	psNew;

	psNew = pcBlock->masInstructions.Add();
	psNew->uiOpcode = uiOpcode;
	psNew->uiFlags = 0;
	psNew->uiNamed = 0;
	psNew->sType = sType;
	psNew->sResult = mpcFunction->AddTemporary(sType);
	psNew->asArguments[0] = sFirst;
	psNew->asArguments[1] = sSecond;
	return psNew->sResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Strength reduction: in a loop with one block going round to its head and
// a counter i (a phi of init, then i + c), an address p + (i << s) or
// p + i * k with p not changed by the loop becomes a pointer of its own: p +
// init * k before the loop, stepped by c * k going round.  The shift or
// multiply and the add each time round become one add.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReduceStrength(void)
{
	CArrayTemplate<int32>	aiLoop;			//Per block index: the head's index of the loop it was last found in.
	CArrayTemplate<int32>	aiDefined;		//Per temporary: the index of the block that defines it, -1 for a parameter.
	CArrayIRBlocks			apcStack;
	size					ui;
	size					uiPhi;
	size					uiArgument;
	size					uiBlock;
	size					uiInstruction;
	int32					iHead;
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
	SIRType					sType;
	int64					lliStep;
	int64					lliScale;
	bool					bChanged;

	mpcFunction->Renumber();
	mpcFunction->ComputePredecessors();
	mcDominators.Compute(mpcFunction);
	FindDefinitions();
	aiDefined.Init();
	FindDefiningBlocks(&aiDefined);
	aiLoop.Init();
	aiLoop.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0xFF);
	apcStack.Init();
	ClearReplacements();
	bChanged = false;
	for (ui = 0; ui < mcDominators.NumReachable(); ui++)
	{
		pcHead = mcDominators.GetReversePostOrder(ui);
		iHead = (int32)pcHead->muiIndex;
		if (!FindLoop(pcHead, &aiLoop, &apcStack, &pcEntering) || !pcEntering || (pcEntering == pcHead) || (pcHead->mapcPredecessors.NumElements() != 2))
		{
			continue;
		}
		pcLatch = (pcHead->mapcPredecessors.GetPtr(0) == pcEntering) ? pcHead->mapcPredecessors.GetPtr(1) : pcHead->mapcPredecessors.GetPtr(0);

		for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
		{
			//The counter: i = phi [entering, init], [latch, i + c].
			psPhi = pcHead->masPhis.Get(uiPhi);
			sCounter = psPhi->sResult;
			if (!psPhi->sType.Is(IRTK_I32) || (psPhi->asArguments.NumElements() != 2))
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
			psNext = sNext.IsTemporary() ? Definition(sNext) : NULL;
			if (sInit.IsNone() || (psNext == NULL) || (psNext->uiOpcode != IRO_Add) || !psNext->asArguments[0].Equals(sCounter) || !psNext->asArguments[1].IsConstant())
			{
				continue;
			}
			lliStep = mpcFunction->GetSigned(psNext->asArguments[1]);
			sType = psPhi->sType;

			//Addresses p + (i << s) or p + i * k in the loop.
			for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
			{
				pcBlock = mpcFunction->GetBlock(uiBlock);
				if (*aiLoop.Get(uiBlock) != iHead)
				{
					continue;
				}
				for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
				{
					psAddress = pcBlock->masInstructions.Get(uiInstruction);
					if ((psAddress->uiOpcode != IRO_PtrAdd) || !psAddress->asArguments[1].IsTemporary() || !Replacement(psAddress->sResult).Equals(psAddress->sResult) || !IsLoopInvariant(psAddress->asArguments[0], &aiDefined, &aiLoop, iHead))
					{
						continue;
					}
					psScaled = Definition(psAddress->asArguments[1]);
					if ((psScaled == NULL) || !psScaled->asArguments[0].Equals(sCounter) || !psScaled->asArguments[1].IsConstant())
					{
						continue;
					}
					if ((psScaled->uiOpcode == IRO_Shl) && (mpcFunction->GetSigned(psScaled->asArguments[1]) >= 0) && (mpcFunction->GetSigned(psScaled->asArguments[1]) < 16))
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
						pcEntering = AddPreheader(pcEntering, pcHead);
						aiLoop.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0xFF);
						*aiLoop.Get(pcEntering->muiIndex) = -1;
					}

					//p + init * k before the loop; the pointer steps by c * k going round.  (Adding to the latch may
					//move this block's instructions.)
					sBase = psAddress->asArguments[0];
					sAddress = psAddress->sResult;
					sStart = sInit.IsConstant() ? mpcFunction->AddInteger(sType, mpcFunction->GetSigned(sInit) * lliScale) : AppendInstruction(pcEntering, IRO_Mul, sType, sInit, mpcFunction->AddInteger(sType, lliScale));
					if (!sStart.IsConstant() || (mpcFunction->GetSigned(sStart) != 0))
					{
						sStart = AppendInstruction(pcEntering, IRO_PtrAdd, IRType(IRTK_Pointer), sBase, sStart);
					}
					else
					{
						sStart = sBase;
					}
					sPointer = mpcFunction->AddTemporary(IRType(IRTK_Pointer));
					sStepped = AppendInstruction(pcLatch, IRO_PtrAdd, IRType(IRTK_Pointer), sPointer, mpcFunction->AddInteger(sType, lliStep * lliScale));
					psNewPhi = pcHead->AddPhi(IRType(IRTK_Pointer), sPointer);
					pcHead->AddPhiArgument(psNewPhi, pcEntering, sStart);
					pcHead->AddPhiArgument(psNewPhi, pcLatch, sStepped);
					Replace(sAddress, sPointer);
					bChanged = true;

					//AddPhi may have moved the head's phis, and the instructions added the entering block's and the
					//latch's instructions, which the definitions point at.
					psPhi = pcHead->masPhis.Get(uiPhi);
					FindDefinitions();
				}
			}
		}
	}
	apcStack.Kill();
	aiLoop.Kill();
	aiDefined.Kill();
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
	CArrayTemplate<int32>	aiLoop;			//Per block index: the head's index of the loop it was last found in.
	CArrayTemplate<int32>	aiDefined;		//Per temporary: the index of the block that defines it, -1 for a parameter.
	CArrayIRBlocks			apcStack;
	CArrayIRAvailables		asStores;
	size					ui;
	size					uiBlock;
	size					uiInstruction;
	size					uiStore;
	int32					iHead;
	int32					iDefined;
	CIRBlock*				pcHead;
	CIRBlock*				pcBlock;
	CIRBlock*				pcEntering;
	SIRInstruction*			psInstruction;
	SIRInstruction			sMoved;
	SIRReference			sRoot;
	SIRReference			sValue;
	int64					lliOffset;
	bool					bCalls;
	bool					bInvariant;
	bool					bChanged;
	bool					bPreheader;
	int						iArgument;

	mpcFunction->Renumber();
	mpcFunction->ComputePredecessors();
	mcDominators.Compute(mpcFunction);
	FindDefinitions();
	aiLoop.Init();
	aiLoop.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0xFF);
	aiDefined.Init();
	FindDefiningBlocks(&aiDefined);
	apcStack.Init();
	asStores.Init();
	bChanged = false;

	for (ui = 0; ui < mcDominators.NumReachable(); ui++)
	{
		pcHead = mcDominators.GetReversePostOrder(ui);
		iHead = (int32)pcHead->muiIndex;
		if (!FindLoop(pcHead, &aiLoop, &apcStack, &pcEntering))
		{
			continue;
		}
		if (!pcEntering || (pcEntering == pcHead))
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
			if (*aiLoop.Get(uiBlock) != iHead)
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
		for (uiBlock = ui; uiBlock < mcDominators.NumReachable(); uiBlock++)
		{
			pcBlock = mcDominators.GetReversePostOrder(uiBlock);
			if (*aiLoop.Get(pcBlock->muiIndex) != iHead)
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
				bInvariant = true;
				for (iArgument = 0; iArgument < 2; iArgument++)
				{
					sValue = psInstruction->asArguments[iArgument];
					if (sValue.IsTemporary())
					{
						iDefined = *aiDefined.Get(sValue.Index());
						bInvariant &= (iDefined < 0) || (*aiLoop.Get(iDefined) != iHead);
					}
				}
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
					pcEntering = AddPreheader(pcEntering, pcHead);
					aiLoop.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0xFF);
					bPreheader = false;
				}
				sMoved = *psInstruction;
				pcBlock->masInstructions.RemoveAt(uiInstruction, true);
				pcEntering->masInstructions.Add(&sMoved);
				*aiDefined.Get(sMoved.sResult.Index()) = (int32)pcEntering->muiIndex;
				uiInstruction--;
				bChanged = true;

				//Moving shifts the block's later instructions and may move the entering block's: the definitions
				//(Definition, used by SplitAddress) point at them.
				FindDefinitions();
			}
		}
	}
	asStores.Kill();
	apcStack.Kill();
	aiDefined.Kill();
	aiLoop.Kill();
	return bChanged;
}
