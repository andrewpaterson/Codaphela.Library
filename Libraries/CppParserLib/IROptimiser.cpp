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
#include "IRInliner.h"
#include "IROptimiserInternal.h"


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
	mbDivideRemainders = false;
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
	mabReached.Init();
	maiReachSearch.Init();
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
	maiReachSearch.Kill();
	mabReached.Kill();
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
// Whether the target's 16, 32 and 64 bit divides leave the remainder where a
// urem or srem straight after them with the same operands can take it (the
// W65C816's R816_UDiv16, R816_SDiv32 ...: D + 5).  ShareDivisions then puts
// such a urem right after its udiv instead of making it a multiply.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetDivideRemainders(bool bDivideRemainders)
{
	mbDivideRemainders = bDivideRemainders;
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
		bChanged |= mbConstantDivisions && ShareDivisions();
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
// Every use of a temporary is what the replacements make it (Replacement;
// anything else stays as it is).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::ApplyReplacements(void)
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
bool CIROptimiser::FirstUse(SIRUse* psUse, SIRReference sValue, CArrayIRBlocks* papcBlocks, bool bTemporaries)
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
bool CIROptimiser::FirstTemporaryUse(SIRUse* psUse, CArrayIRBlocks* papcBlocks)
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
bool CIROptimiser::NextUse(SIRUse* psUse)
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
void CIROptimiser::SetUse(SIRUse* psUse, SIRReference sNew)
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
bool CIROptimiser::MayWriteMemory(SIRInstruction* psInstruction)
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
bool CIROptimiser::IsOwnFrame(SIRReference sAddress)
{
	SIRInstruction*	psDefinition;
	int				iSteps;

	for (iSteps = 0; iSteps < 64; iSteps++)
	{
		psDefinition = mcDefinitions.Instruction(sAddress);
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
// Whether pcFunction may write memory its callers can see: a store not to
// its own frame, a volatile load or store, a call to a function not known
// to write nothing (CIRModule::IsNoWrites), an indirect call, or anything
// else with an effect.  CCPP65IRFrontEnd::MarkNoWrites works the unit's
// functions out with it, together.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::WritesMemory(CIRFunction* pcFunction)
{
	size			uiBlock;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	bool			bWrites;

	mpcFunction = pcFunction;
	FindDefinitions();
	bWrites = false;
	for (uiBlock = 0; (uiBlock < pcFunction->NumBlocks()) && !bWrites; uiBlock++)
	{
		pcBlock = pcFunction->GetBlock(uiBlock);
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
	mpcFunction = NULL;
	return bWrites;
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
// Moves pcBlock's instruction at uiFrom to uiTo (an index in the
// instructions without it).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::MoveInstruction(CIRBlock* pcBlock, size uiFrom, size uiTo)
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
