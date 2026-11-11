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
#include "IRInliner.h"
#include "IROptimiserInternal.h"


//////////////////////////////////////////////////////////////////////////
//
// The groups of passes are set by whoever makes them (CIROptimiser::Init).
//
//////////////////////////////////////////////////////////////////////////
void CIRPassContext::Init(CIRModule* pcModule)
{
	mpcModule = pcModule;
	mpcFunction = NULL;
	mbIndexedAddresses = true;
	mbConstantDivisions = false;
	muiDivideRemainders = 0;
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
	masArgAddresses.Init();
	mpcAddressPasses = NULL;
	mpcBlockPasses = NULL;
	mpcCounterPasses = NULL;
	mpcDivisionPasses = NULL;
	mpcFoldingPasses = NULL;
	mpcIdiomPasses = NULL;
	mpcLoopPasses = NULL;
	mpcMemoryPasses = NULL;
	mpcPromotionPasses = NULL;
	mpcRangePasses = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPassContext::Kill(void)
{
	masArgAddresses.Kill();
	mauiUses.Kill();
	masReplacements.Kill();
	mcLoops.Kill();
	mcDefinitions.Kill();
	mcDominators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::Init(CIRPassContext* pcContext)
{
	mpcContext = pcContext;
	mpcModule = pcContext->mpcModule;
	mpcFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::Kill(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
// The function the passes are run on from now, or NULL for none.
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::Begin(CIRFunction* pcFunction)
{
	mpcFunction = pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the profile has the instruction's line run often enough that
// saving ulliSaving cycles each time is 1/IR_INLINE_HOT_SHARE of the run.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::IsHot(SIRInstruction* psInstruction, uint64 ulliSaving)
{
	uint64	ulliCount;

	if (!mpcContext->mpcProfile || (mpcContext->mpcProfile->ProfileCycles() == 0))
	{
		return false;
	}
	ulliCount = mpcContext->mpcProfile->CallCount(mpcFunction, psInstruction);
	return (ulliCount != IR_INLINE_UNKNOWN) && (ulliCount * ulliSaving * IR_INLINE_HOT_SHARE >= mpcContext->mpcProfile->ProfileCycles());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::ClearReplacements(void)
{
	mpcContext->masReplacements.FakeSetUsedElements(0);
	mpcContext->masReplacements.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Uses of the temporary sOld are to be sNew.
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::Replace(SIRReference sOld, SIRReference sNew)
{
	mpcContext->masReplacements.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	*mpcContext->masReplacements.Get(sOld.Index()) = sNew;
}


//////////////////////////////////////////////////////////////////////////
//
// What sReference is after the replacements (following replaced
// replacements).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRPasses::Replacement(SIRReference sReference)
{
	SIRReference*	psReplacement;

	while (sReference.IsTemporary())
	{
		psReplacement = mpcContext->masReplacements.SafeGet(sReference.Index());
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
// Every use of a temporary is what the replacements make it (Replacement;
// anything else stays as it is).
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::ApplyReplacements(void)
{
	SIRUse	sUse;
	bool	bUse;

	for (bUse = FirstTemporaryUse(&sUse); bUse; bUse = NextUse(&sUse))
	{
		*sUse.psSlot = Replacement(*sUse.psSlot);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Starts a walk of sValue's uses (of every temporary with bTemporaries)
// in papcBlocks (NULL for all the function's blocks) at the first; false
// if there is none.  NextUse goes on to the next.  One walk serves the
// passes that look for, count or rewrite the uses of a value, each with its
// own filter on the blocks.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::FirstUse(SIRUse* psUse, SIRReference sValue, CArrayIRBlocks* papcBlocks, bool bTemporaries)
{
	psUse->sValue = sValue;
	psUse->bTemporaries = bTemporaries;
	psUse->papcBlocks = papcBlocks;
	psUse->uiBlock = 0;
	psUse->eKind = IRUK_Phi;
	psUse->uiIndex = 0;
	psUse->uiArgument = (size)-1;
	return NextUse(psUse);
}


//////////////////////////////////////////////////////////////////////////
//
// FirstUse of every temporary instead of one value: each operand that is a
// temporary.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::FirstTemporaryUse(SIRUse* psUse, CArrayIRBlocks* papcBlocks)
{
	return FirstUse(psUse, IRNone(), papcBlocks, true);
}


//////////////////////////////////////////////////////////////////////////
//
// The walk's next use after the one it is at (block by block: the phis'
// arguments, the instructions' operands, then the jump); false when there
// are no more.  The block, phi and instruction are fetched again by their
// indices each step, and the counts read again, so the walker may add
// phis and instructions to the ends of blocks between steps (those added
// to the block it is in, after where it is, are walked too).
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::NextUse(SIRUse* psUse)
{
	size			uiBlocks;
	SIRReference*	psSlot;

	psUse->uiArgument++;
	for (;;)
	{
		uiBlocks = psUse->papcBlocks ? psUse->papcBlocks->NumElements() : mpcFunction->NumBlocks();
		if (psUse->uiBlock >= uiBlocks)
		{
			return false;
		}
		psUse->pcBlock = psUse->papcBlocks ? psUse->papcBlocks->GetPtr(psUse->uiBlock) : mpcFunction->GetBlock(psUse->uiBlock);
		if (psUse->eKind == IRUK_Phi)
		{
			if (psUse->uiIndex >= psUse->pcBlock->masPhis.NumElements())
			{
				psUse->eKind = IRUK_Instruction;
				psUse->uiIndex = 0;
				psUse->uiArgument = 0;
				continue;
			}
			psUse->psPhi = psUse->pcBlock->masPhis.Get(psUse->uiIndex);
			if (psUse->uiArgument >= psUse->psPhi->asArguments.NumElements())
			{
				psUse->uiIndex++;
				psUse->uiArgument = 0;
				continue;
			}
			psUse->psArgument = psUse->psPhi->asArguments.Get(psUse->uiArgument);
			psSlot = &psUse->psArgument->sValue;
		}
		else if (psUse->eKind == IRUK_Instruction)
		{
			if (psUse->uiIndex >= psUse->pcBlock->masInstructions.NumElements())
			{
				psUse->eKind = IRUK_Jump;
				psUse->uiIndex = 0;
				psUse->uiArgument = 0;
				continue;
			}
			if (psUse->uiArgument >= 2)
			{
				psUse->uiIndex++;
				psUse->uiArgument = 0;
				continue;
			}
			psUse->psInstruction = psUse->pcBlock->masInstructions.Get(psUse->uiIndex);
			psSlot = &psUse->psInstruction->asArguments[psUse->uiArgument];
		}
		else
		{
			if (psUse->uiArgument >= 1)
			{
				psUse->uiBlock++;
				psUse->eKind = IRUK_Phi;
				psUse->uiIndex = 0;
				psUse->uiArgument = 0;
				continue;
			}
			psSlot = &psUse->pcBlock->msJump.sArgument;
		}
		if (psUse->bTemporaries ? psSlot->IsTemporary() : psSlot->Equals(psUse->sValue))
		{
			psUse->psSlot = psSlot;
			return true;
		}
		psUse->uiArgument++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The use the walk is at reads sNew.  It is found again by its indices: a
// walk that worked sNew out may have added phis (CIRSSAUpdater), moving the
// block's phis.
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::SetUse(SIRUse* psUse, SIRReference sNew)
{
	CIRBlock*	pcBlock;

	pcBlock = psUse->papcBlocks ? psUse->papcBlocks->GetPtr(psUse->uiBlock) : mpcFunction->GetBlock(psUse->uiBlock);
	if (psUse->eKind == IRUK_Phi)
	{
		psUse->psPhi = pcBlock->masPhis.Get(psUse->uiIndex);
		psUse->psArgument = psUse->psPhi->asArguments.Get(psUse->uiArgument);
		psUse->psSlot = &psUse->psArgument->sValue;
	}
	else if (psUse->eKind == IRUK_Instruction)
	{
		psUse->psInstruction = pcBlock->masInstructions.Get(psUse->uiIndex);
		psUse->psSlot = &psUse->psInstruction->asArguments[psUse->uiArgument];
	}
	else
	{
		psUse->psSlot = &pcBlock->msJump.sArgument;
	}
	*psUse->psSlot = sNew;
}


//////////////////////////////////////////////////////////////////////////
//
// IRHasEffect, except a call to a function that writes no memory its callers
// can see (CIRModule::IsNoWrites): what is known to be in memory is still
// so after it.  (Stores are the caller's to look at: this says only that
// the instruction isn't some other write.)
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::MayWriteMemory(SIRInstruction* psInstruction)
{
	if ((psInstruction->uiOpcode == IRO_Call) && psInstruction->asArguments[0].IsSymbol() && mpcModule->IsNoWrites(psInstruction->asArguments[0].Index()))
	{
		return false;
	}
	return IRHasEffect(psInstruction);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sAddress (through ptradds, any offset) is in the function's own
// frame: a slot or a parameter's argaddr, gone when it returns.  Needs
// FindDefinitions.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::IsOwnFrame(SIRReference sAddress)
{
	SIRInstruction*	psDefinition;
	int				iSteps;

	for (iSteps = 0; iSteps < 64; iSteps++)
	{
		psDefinition = mpcContext->mcDefinitions.Instruction(sAddress);
		if (psDefinition == NULL)
		{
			return false;
		}
		if ((psDefinition->uiOpcode == IRO_Slot) || (psDefinition->uiOpcode == IRO_ArgAddress))
		{
			return true;
		}
		if (psDefinition->uiOpcode != IRO_PtrAdd)
		{
			return false;
		}
		sAddress = psDefinition->asArguments[0];
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Each temporary's definition (mpcContext->mcDefinitions) and each parameter's first
// argaddr.
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::FindDefinitions(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	size			uiParameter;

	mpcContext->mcDefinitions.Compute(mpcFunction);
	mpcContext->masArgAddresses.FakeSetUsedElements(0);
	mpcContext->masArgAddresses.GrowToAtLeastNumElements(mpcFunction->masParameters.NumElements(), true, 0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			uiParameter = (psInstruction->uiOpcode == IRO_ArgAddress) ? (size)mpcFunction->GetSigned(psInstruction->asArguments[0]) : SIZE_MAX;
			if ((uiParameter < mpcContext->masArgAddresses.NumElements()) && mpcContext->masArgAddresses.Get(uiParameter)->IsNone())
			{
				*mpcContext->masArgAddresses.Get(uiParameter) = psInstruction->sResult;
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
void CIRPasses::ComputeLoops(void)
{
	mpcFunction->Renumber();
	mpcFunction->ComputePredecessors();
	mpcModule->Count(CPPCC_Dominators);
	mpcContext->mcDominators.Compute(mpcFunction);
	FindDefinitions();
	mpcContext->mcLoops.Compute(mpcFunction, &mpcContext->mcDominators);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves pcBlock's instruction at uiFrom to uiTo (an index in the
// instructions without it).
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::MoveInstruction(CIRBlock* pcBlock, size uiFrom, size uiTo)
{
	SIRInstruction	sMoved;

	sMoved = *pcBlock->masInstructions.Get(uiFrom);
	pcBlock->masInstructions.RemoveAt(uiFrom, true);
	pcBlock->masInstructions.InsertAt(&sMoved, uiTo);
}


//////////////////////////////////////////////////////////////////////////
//
// A new instruction at uiIndex in pcBlock's instructions; sType is as
// CIRFunction::Add takes it.  It has the source line of the instruction it
// goes before.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRPasses::InsertInstruction(CIRBlock* pcBlock, size uiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond)
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
// Whether the function may write memory its callers can see: a store not to
// its own frame, a volatile load or store, a call to a function not known
// to write nothing (CIRModule::IsNoWrites), an indirect call, or anything
// else with an effect.  CIRInterprocedural::MarkNoWrites works the unit's
// functions out with it, together.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::WritesMemory(void)
{
	size			uiBlock;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	bool			bWrites;

	FindDefinitions();
	bWrites = false;
	for (uiBlock = 0; (uiBlock < mpcFunction->NumBlocks()) && !bWrites; uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (uiInstruction = 0; (uiInstruction < pcBlock->masInstructions.NumElements()) && !bWrites; uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->uiOpcode == IRO_Store)
			{
				bWrites = (psInstruction->uiFlags & IRF_Volatile) || !IsOwnFrame(psInstruction->asArguments[1]);
			}
			else if (psInstruction->uiOpcode != IRO_Arg)
			{
				bWrites = MayWriteMemory(psInstruction);
			}
		}
	}
	return bWrites;
}

