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
#include "IROptimiser.h"


#define IR_OPTIMISE_ROUNDS		16
#define IR_ROTATE_LIMIT			8		//The most instructions a loop's head may have for RotateLoops to copy it.
#define IR_THREAD_LIMIT			8		//The most instructions a block may have for ThreadPhis to work out.
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
	msLibraryInt = IRType(IRTK_None);
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
		bChanged |= NarrowTruncations();
		bChanged |= RemoveCommonSubexpressions();
		bChanged |= RemoveRedundantLoads();
		bChanged |= RotateLoops();
		bChanged |= LessLoopTests();
		bChanged |= ReduceStrength();
		bChanged |= HoistInvariants();
		bChanged |= ReplaceLoopIdioms();
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
// True if pcBlock's phis and results are only used in pcBlock (by its
// instructions, jump or phis; so a predecessor can go round it without
// them).  mauiUses must be counted.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::OnlyUsedInBlock(CIRBlock* pcBlock)
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
		if (!bPure || !OnlyUsedInBlock(pcBlock))
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
			else if (UndoesExtension(psInstruction, &sValue) || ConvertPhi(psInstruction, &sValue) || IsIdentity(psInstruction, &sValue))
			{
				//The value's uses go up by the result's (ConvertPhi mustn't change a phi something else now uses).
				Replace(psInstruction->sResult, sValue);
				if (sValue.IsTemporary() && psInstruction->sResult.IsTemporary())
				{
					*mauiUses.Get(sValue.Index()) += *mauiUses.Get(psInstruction->sResult.Index());
				}
				bChanged = true;
			}
			else if ((psInstruction->uiOpcode == IRO_SExt) && (mpcFunction->TypeOf(psInstruction->asArguments[0]).IntegerBits() > 1) &&
					 Extended(psInstruction->asArguments[0], mpcFunction->TypeOf(psInstruction->asArguments[0]).IntegerBits() - 1, false, 0))
			{
				//A sext of a value whose top bit is 0 is a zext (cheaper where a sign must be worked out).
				psInstruction->uiOpcode = IRO_ZExt;
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
// x | 0, x ^ 0, a shift by 0, x * 1, x & all ones, ptradd x, 0 (and 0 + x
// and the like for a commutative op).
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
		if (ulliBits != 1)
		{
			return false;
		}
		break;
	case IRO_And:
		if (ulliBits != IRTruncate(psInstruction->sType, (uint64)-1))
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
// a counter i (a phi of init, then i + c or i - c), an address p + i (when
// the target's loads and stores don't add it for nothing),
// p + (i << s) or p + i * k with p not changed by the loop becomes a
// pointer of its own: p + init * k before the loop, stepped by c * k going
// round.  The shift or multiply and the add each time round become one
// add.
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
	SIRType					sType;
	int64					lliStep;
	int64					lliScale;
	bool					bChanged;

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
			sType = psPhi->sType;

			//Addresses p + (i << s) or p + i * k in the loop.
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
					if (psAddress->asArguments[1].Equals(sCounter))
					{
						//p + i, unless loads and stores add it for nothing.
						if (mbIndexedAddresses)
						{
							continue;
						}
						lliScale = 1;
					}
					else if ((psScaled == NULL) || !psScaled->asArguments[0].Equals(sCounter) || !psScaled->asArguments[1].IsConstant())
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
						sStart = (lliScale == 1) ? sInit : AppendInstruction(pcEntering, IRO_Mul, sType, sInit, mpcFunction->AddInteger(sType, lliScale));
					}
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
// to the head, a preheader, and a head of at most IR_ROTATE_LIMIT
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
	if ((mcLoops.NumLatches(uiLoop) != 1) || !psLoop->pcEntering || (pcHead->mapcPredecessors.NumElements() != 2))
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
			pcPreheader = AddPreheader(uiLoop);
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
	SIRInstruction*	psGuard;
	CIRBlock*		pcEntering;
	CIRBlock*		pcTarget;
	SIRType			sType;
	int				iSteps;
	uint64			ulliInit;
	uint64			ulliBound;
	uint16			uiOpcode;

	pcEntering = mcLoops.GetLoop(uiLoop)->pcEntering;
	psCompare = mcDefinitions.Instruction(pcLatch->msJump.sArgument);
	if ((pcLatch->msJump.uiKind != IRJ_Branch) || (psCompare == NULL) || !IsIRIntegerComparison(psCompare->uiOpcode) || !pcEntering)
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

	//The way in: its test, before the blocks of only a jump (preheaders) on the way.
	pcTarget = pcHead;
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
			   psGuard->asArguments[0].Equals(psCounter->sInit) && psGuard->asArguments[1].Equals(psCounter->sBound);
	}
	if (!psCounter->sInit.IsConstant() || !psCounter->sBound.IsConstant())
	{
		return false;
	}
	sType = mpcFunction->TypeOf(psCounter->sInit);
	ulliInit = IRTruncate(sType, mpcFunction->GetConstant(psCounter->sInit)->ulliBits);
	ulliBound = IRTruncate(sType, mpcFunction->GetConstant(psCounter->sBound)->ulliBits);
	switch (uiOpcode)
	{
	case IRO_SLt:
		return IRSignExtend(sType, ulliInit) < IRSignExtend(sType, ulliBound);
	case IRO_ULt:
		return ulliInit < ulliBound;
	default:
		return ulliInit != ulliBound;
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
// A call of the library's szName (memset or memmove) at the end of pcBlock.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::LibraryCall(CIRBlock* pcBlock, const char* szName, SIRReference sFirst, SIRType sFirstType, SIRReference sSecond, SIRType sSecondType, SIRReference sBytes)
{
	mpcFunction->Argument(pcBlock, sFirstType, sFirst);
	mpcFunction->Argument(pcBlock, sSecondType, sSecond);
	mpcFunction->Argument(pcBlock, IRType(IRTK_I32), sBytes);
	return mpcFunction->Call(pcBlock, IRType(IRTK_Pointer), IRReference(IRRK_Symbol, mpcModule->AddSymbol(szName)));
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
		(strcmp(szName, "_memcmp") == 0) || (strcmp(szName, "_strlen") == 0))
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
		if (ReplaceStrlen(uiLoop) || ReplaceMemcmp(uiLoop))
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
			LibraryCall(pcPreheader, "_memset", sDestinationStart, sPointer, sValue, msLibraryInt, sBytes);
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
			LibraryCall(pcCall, "_memmove", sDestinationStart, sPointer, sSourceStart, sPointer, sBytes);
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
	SIRInstruction*	psInstruction;
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
	int				iLoads;
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
	psLoad = NULL;
	iLoads = 0;
	for (ui = 0; ui < pcLoop->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcLoop->masInstructions.Get(ui);
		if (psInstruction->uiOpcode == IRO_Load)
		{
			psLoad = psInstruction;
			iLoads++;
		}
		else if (!IsIRPure(psInstruction->uiOpcode))
		{
			return false;
		}
	}
	if ((iLoads != 1) || !psLoad->sType.Is(IRTK_I8) || (psLoad->uiFlags & IRF_Volatile) || (*mauiUses.Get(psLoad->sResult.Index()) != 1))
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
			bResult = (MatchLinear(pcLoop, sValue, &sStart, &lliStep, &bNext));
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
	mpcFunction->Argument(pcPreheader, IRType(IRTK_Pointer), sStart);
	sLength = mpcFunction->Call(pcPreheader, IRType(IRTK_I32), IRReference(IRRK_Symbol, mpcModule->AddSymbol("_strlen")));
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
			MatchLinear(pcLoop, sValue, &sStart, &lliStep, &bNext);
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
// Whether sValue, worked out in the loop block pcLoop, is start + k * step
// the k-th time round (from 0): a phi of the block taking start on the way
// in and phi + step (an add or ptradd of a constant) from the loop, or that
// next value (*pbNext: start + (k + 1) * step).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::MatchLinear(CIRBlock* pcLoop, SIRReference sValue, SIRReference* psStart, int64* plliStep, bool* pbNext)
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
	if ((psPhi == NULL) || (mcDefinitions.Block(sPhi) != pcLoop) || (psPhi->asArguments.NumElements() != 2))
	{
		return false;
	}
	*psStart = IRNone();
	*plliStep = 0;
	for (ui = 0; ui < 2; ui++)
	{
		psArgument = psPhi->asArguments.Get(ui);
		if (psArgument->pcBlock == pcLoop)
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
	SIRInstruction*	psInstruction;
	SIRInstruction*	psCompare;
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
	psFirst = NULL;
	psSecond = NULL;
	for (ui = 0; ui < pcHead->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcHead->masInstructions.Get(ui);
		if (psInstruction->uiOpcode == IRO_Load)
		{
			if (psSecond)
			{
				return false;
			}
			if (psFirst)
			{
				psSecond = psInstruction;
			}
			else
			{
				psFirst = psInstruction;
			}
		}
		else if (!IsIRPure(psInstruction->uiOpcode))
		{
			return false;
		}
	}
	for (ui = 0; ui < pcLatch->masInstructions.NumElements(); ui++)
	{
		if (!IsIRPure(pcLatch->masInstructions.Get(ui)->uiOpcode))
		{
			return false;
		}
	}
	if (!psSecond || !psFirst->sType.Equals(psSecond->sType) || !psFirst->sType.IsInteger() || (psFirst->uiFlags & IRF_Volatile) || (psSecond->uiFlags & IRF_Volatile))
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
	mpcFunction->Argument(pcPreheader, IRType(IRTK_Pointer), sFirstStart);
	mpcFunction->Argument(pcPreheader, IRType(IRTK_Pointer), sSecondStart);
	mpcFunction->Argument(pcPreheader, IRType(IRTK_I32), sBytes);
	sResult = mpcFunction->Call(pcPreheader, msLibraryInt, IRReference(IRRK_Symbol, mpcModule->AddSymbol("_memcmp")));
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
