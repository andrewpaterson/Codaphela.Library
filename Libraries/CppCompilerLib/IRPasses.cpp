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
	mpcPipeline = NULL;
	muiDivideRemainders = 0;
	mpszWarnings = NULL;
	mbRestrict = false;
	miUnswitched = 0;
	msLibraryInt = IRType(IRTK_None);
	msLibrarySize = IRType(IRTK_I32);
	mpcProfile = NULL;
	mpcDivisionCosts = NULL;
	mcDominators.Init();
	mcDefinitions.Init();
	mcLoops.Init();
	mcRanges.Init(this);
	mcAddresses.Init(this);
	masReplacements.Init();
	masArgAddresses.Init();
	mpcBlockPasses = NULL;
	mpcCounterPasses = NULL;
	mpcFoldingPasses = NULL;
	mpcMemoryPasses = NULL;
	mpcRangePasses = NULL;
	mbDominatorsFresh = false;
	mbDefinitionsFresh = false;
	mbLoopsFresh = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPassContext::Kill(void)
{
	masArgAddresses.Kill();
	masReplacements.Kill();
	mcLoops.Kill();
	mcDefinitions.Kill();
	mcDominators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function has changed (a pass says it changed it, or another function
// is being optimised): the analyses no longer describe it.
//
//////////////////////////////////////////////////////////////////////////
void CIRPassContext::Changed(void)
{
	mbDominatorsFresh = false;
	mbDefinitionsFresh = false;
	mbLoopsFresh = false;
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
// Each block's index is its place in the function.  When that changes any,
// what was worked out by the old indices is no longer good.
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::Renumber(void)
{
	if (!mpcFunction->IsNumbered())
	{
		mpcFunction->Renumber();
		mpcContext->Changed();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Works the dominators out (the blocks numbered, with their predecessors).
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::ComputeDominators(void)
{
	mpcModule->Count(CPPCC_Dominators);
	mpcContext->mcDominators.Compute(mpcFunction);
	mpcContext->mbDominatorsFresh = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The definitions as they are, if nothing has changed the function since
// they were found; else found (FindDefinitions).  For the start of a pass.
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::NeedDefinitions(void)
{
	if (!mpcContext->mbDefinitionsFresh)
	{
		FindDefinitions();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The loops (and the blocks' numbers and predecessors, the dominators and
// the definitions) as they are, if nothing has changed the function since
// ComputeLoops worked them out; else worked out.  For the start of a pass.
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::NeedLoops(void)
{
	if (!mpcContext->mbLoopsFresh)
	{
		ComputeLoops();
	}
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
// anything else stays as it is), and every debug value marker's.
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::ApplyReplacements(void)
{
	SIRUse			sUse;
	bool			bUse;
	CIRBlock*		pcBlock;
	SIRDebugValue*	psValue;
	size			ui;
	size			uj;

	for (bUse = mpcFunction->FirstTemporaryUse(&sUse); bUse; bUse = mpcFunction->NextUse(&sUse))
	{
		*sUse.psSlot = Replacement(*sUse.psSlot);
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masDebugValues.NumElements(); uj++)
		{
			psValue = pcBlock->masDebugValues.Get(uj);
			psValue->sValue = Replacement(psValue->sValue);
		}
	}
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
	mpcContext->mbDefinitionsFresh = true;
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
	Renumber();
	mpcFunction->ComputePredecessors();
	ComputeDominators();
	FindDefinitions();
	mpcContext->mcLoops.Compute(mpcFunction, &mpcContext->mcDominators);
	mpcContext->mbLoopsFresh = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves pcBlock's instruction at uiFrom to uiTo (an index in the
// instructions without it).
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::MoveInstruction(CIRBlock* pcBlock, size uiFrom, size uiTo)
{
	pcBlock->MoveInstruction(uiFrom, uiTo);
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
	pcBlock->RemoveInstruction(uiLast);
	pcBlock->InsertInstruction(&sMoved, uiIndex);
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A new instruction at uiIndex in pcBlock's instructions whose result is
// sResult, a temporary made for it before (one of several that are worked
// out from each other, so each must exist before the instructions do),
// with the source line uiSource.
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::InsertInstruction(CIRBlock* pcBlock, size uiIndex, uint16 uiOpcode, SIRType sType, SIRReference sResult, SIRReference sFirst, SIRReference sSecond, uint32 uiSource)
{
	SIRInstruction	sNew;

	memset(&sNew, 0, sizeof(SIRInstruction));
	sNew.uiOpcode = uiOpcode;
	sNew.uiSource = uiSource;
	sNew.sType = sType;
	sNew.sResult = sResult;
	sNew.asArguments[0] = sFirst;
	sNew.asArguments[1] = sSecond;
	pcBlock->InsertInstruction(&sNew, uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// A new instruction at the end of pcBlock's, with the source line uiSource:
// that of the instruction or jump it is part of the work of.  sType is as
// CIRFunction::Add takes it, and its result is a new temporary.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRPasses::AddInstruction(CIRBlock* pcBlock, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond, uint32 uiSource)
{
	SIRReference	sResult;

	sResult = mpcFunction->Add(pcBlock, uiOpcode, sType, sFirst, sSecond);
	pcBlock->masInstructions.Get(pcBlock->masInstructions.NumElements() - 1)->uiSource = uiSource;
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
				bWrites = (psInstruction->uiFlags & IRF_Volatile) || !mpcContext->mcAddresses.IsOwnFrame(psInstruction->asArguments[1]);
			}
			else if (psInstruction->uiOpcode != IRO_Arg)
			{
				bWrites = MayWriteMemory(psInstruction);
			}
		}
	}
	return bWrites;
}


//////////////////////////////////////////////////////////////////////////
//
// What pcFrom's branch says going to pcTo (SIREdgeCondition).  False if it
// doesn't branch two ways, one of them to pcTo, on an integer comparison.
// mpcContext->mcDefinitions must be found.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::EdgeCondition(CIRBlock* pcFrom, CIRBlock* pcTo, SIREdgeCondition* psCondition)
{
	return psCondition->Init(&pcFrom->msJump, pcTo, mpcContext->mcDefinitions.Instruction(pcFrom->msJump.sArgument));
}


//////////////////////////////////////////////////////////////////////////
//
// The two ways of pcBlock's branch when one stays in loop uiLoop and the
// other leaves it: *ppcInside and *ppcOutside.  False if it doesn't
// branch, or both ways stay or both leave.  mpcContext->mcLoops must be
// found.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::LoopWays(size uiLoop, CIRBlock* pcBlock, CIRBlock** ppcInside, CIRBlock** ppcOutside)
{
	bool	bThen;
	bool	bElse;

	if (pcBlock->msJump.uiKind != IRJ_Branch)
	{
		return false;
	}
	bThen = mpcContext->mcLoops.Contains(uiLoop, pcBlock->msJump.pcThen);
	bElse = mpcContext->mcLoops.Contains(uiLoop, pcBlock->msJump.pcElse);
	if (bThen == bElse)
	{
		return false;
	}
	*ppcInside = bThen ? pcBlock->msJump.pcThen : pcBlock->msJump.pcElse;
	*ppcOutside = bThen ? pcBlock->msJump.pcElse : pcBlock->msJump.pcThen;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psPhi (of loop uiLoop's head) is a counter: two arguments, one
// (*psCounter's uiOutside, the first unless the first's block is in the
// loop) from a block outside the loop, the other phi + c, c a constant.
// The step is an add; a sub too with IR_COUNTER_SUB (lliStep is then -c),
// and a ptradd too with IR_COUNTER_POINTER.  With IR_COUNTER_INSIDE the
// other must come from a block in the loop.  *psCounter gets the arguments
// and the step.  What more a pass wants of its counter (that it steps by 1,
// or comes round from the latch) it asks of what it is given.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::MatchCounter(size uiLoop, SIRPhi* psPhi, uint8 uiFlags, SIRLoopCounter* psCounter)
{
	SIRInstruction*	psStep;
	SIRPhiArgument*	psOutside;
	SIRPhiArgument*	psInside;

	if (psPhi->asArguments.NumElements() != 2)
	{
		return false;
	}
	psCounter->uiOutside = mpcContext->mcLoops.Contains(uiLoop, psPhi->asArguments.Get(0)->pcBlock) ? 1 : 0;
	psOutside = psPhi->asArguments.Get(psCounter->uiOutside);
	psInside = psPhi->asArguments.Get(1 - psCounter->uiOutside);
	if (mpcContext->mcLoops.Contains(uiLoop, psOutside->pcBlock) || ((uiFlags & IR_COUNTER_INSIDE) && !mpcContext->mcLoops.Contains(uiLoop, psInside->pcBlock)))
	{
		return false;
	}
	psCounter->sPhi = psPhi->sResult;
	psCounter->sInit = psOutside->sValue;
	psCounter->sNext = psInside->sValue;
	psStep = mpcContext->mcDefinitions.Instruction(psCounter->sNext);
	if ((psStep == NULL) || !psStep->asArguments[0].Equals(psPhi->sResult) || !psStep->asArguments[1].IsConstant() ||
		!((psStep->uiOpcode == IRO_Add) || ((psStep->uiOpcode == IRO_Sub) && (uiFlags & IR_COUNTER_SUB)) || ((psStep->uiOpcode == IRO_PtrAdd) && (uiFlags & IR_COUNTER_POINTER))))
	{
		return false;
	}
	psCounter->lliStep = mpcFunction->GetSigned(psStep->asArguments[1]);
	if (psStep->uiOpcode == IRO_Sub)
	{
		psCounter->lliStep = -psCounter->lliStep;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether loop uiLoop has one way in and one way round: one block outside
// it enters its head (*ppcEntering) and one block inside goes round to it
// (*ppcLatch), so each phi of the head (*ppcHead) has a value from each.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPasses::SimpleLoop(size uiLoop, CIRBlock** ppcHead, CIRBlock** ppcEntering, CIRBlock** ppcLatch)
{
	*ppcHead = mpcContext->mcLoops.GetLoop(uiLoop)->pcHead;
	*ppcEntering = mpcContext->mcLoops.GetLoop(uiLoop)->pcEntering;
	if (!*ppcEntering || ((*ppcHead)->mapcPredecessors.NumElements() != 2))
	{
		return false;
	}
	*ppcLatch = mpcContext->mcLoops.GetLatch(uiLoop, 0);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A phi of pcHead for a value the loop steps: sPhi (a temporary of sType,
// not yet defined) is sStart coming in from pcEntering and sStepped coming
// round from pcLatch.  The head's phis may move.
//
//////////////////////////////////////////////////////////////////////////
void CIRPasses::AddCounterPhi(CIRBlock* pcHead, SIRType sType, SIRReference sPhi, CIRBlock* pcEntering, SIRReference sStart, CIRBlock* pcLatch, SIRReference sStepped)
{
	SIRPhi*	psPhi;

	psPhi = pcHead->AddPhi(sType, sPhi);
	pcHead->AddPhiArgument(psPhi, pcEntering, sStart);
	pcHead->AddPhiArgument(psPhi, pcLatch, sStepped);
}


//////////////////////////////////////////////////////////////////////////
//
// sDivisor & -(sTest widened to sType), worked out from uiIndex in pcBlock:
// the divisor when the test (x >= c) holds, else 0, which taken from x
// leaves x % c for an x below twice c.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRPasses::InsertDivisorMask(CIRBlock* pcBlock, size uiIndex, SIRType sType, SIRReference sTest, SIRReference sDivisor)
{
	SIRReference	sMask;

	sMask = InsertInstruction(pcBlock, uiIndex, IRO_ZExt, sType, sTest, IRNone());
	sMask = InsertInstruction(pcBlock, uiIndex + 1, IRO_Neg, sType, sMask, IRNone());
	return InsertInstruction(pcBlock, uiIndex + 2, IRO_And, sType, sMask, sDivisor);
}


//////////////////////////////////////////////////////////////////////////
//
// (After the context's module is set: an analysis reads it from there.)
//
//////////////////////////////////////////////////////////////////////////
void CIRPassAnalysis::Init(CIRPassContext* pcContext)
{
	mpcContext = pcContext;
	mpcModule = pcContext->mpcModule;
	mpcFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The function it is asked about from now (NULL for none).
//
//////////////////////////////////////////////////////////////////////////
void CIRPassAnalysis::Begin(CIRFunction* pcFunction)
{
	mpcFunction = pcFunction;
}
