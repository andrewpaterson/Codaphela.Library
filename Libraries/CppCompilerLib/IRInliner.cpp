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
#include "IRLoops.h"
#include "IRInliner.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::Init(CIRModule* pcModule, size uiMaxInstructions, size uiMaxLoopInstructions, size uiMaxInlines)
{
	mpcModule = pcModule;
	mpcCaller = NULL;
	muiMaxInstructions = uiMaxInstructions;
	muiMaxLoopInstructions = uiMaxLoopInstructions;
	muiMaxInlines = uiMaxInlines;
	masMap.Init();
	mapcCopies.Init();
	masReturns.Init();
	masThrows.Init();
	mbJoinThrows = false;
	mpcThrowsTo = NULL;
	mabInLoop.Init();
	maiArgumentOffsets.Init();
	masArguments.Init();
	mapcRecursiveCallers.Init();
	mapcRecursiveCallees.Init();
	msArguments = IRNone();
	mpcImports = NULL;
	mpaszMissing = NULL;
	mpaszNotImported = NULL;
	mpcCalleeModule = NULL;
	muiImportSource = 0;
	mpcProfile = NULL;
	muiHotInstructions = 0;
	mbForwardersJump = false;
	mbCallerForwards = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::Kill(void)
{
	mapcRecursiveCallees.Kill();
	mapcRecursiveCallers.Kill();
	masArguments.Kill();
	maiArgumentOffsets.Kill();
	mabInLoop.Kill();
	masThrows.Kill();
	masReturns.Kill();
	mapcCopies.Kill();
	masMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRInliner::NumInstructions(CIRFunction* pcFunction)
{
	size	ui;
	size	uiCount;

	uiCount = 0;
	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		uiCount += pcFunction->GetBlock(ui)->masInstructions.NumElements() + pcFunction->GetBlock(ui)->masPhis.NumElements();
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcCallee calls itself and has been inlined into mpcCaller already.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::InlinedRecursive(CIRFunction* pcCallee)
{
	size	ui;

	for (ui = 0; ui < mapcRecursiveCallers.NumElements(); ui++)
	{
		if ((*mapcRecursiveCallers.Get(ui) == mpcCaller) && (*mapcRecursiveCallees.Get(ui) == pcCallee))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction calls itself directly.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::CallsItself(CIRFunction* pcFunction)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	size			ui;
	size			uiInstruction;

	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		pcBlock = pcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if ((psInstruction->uiOpcode == IRO_Call) && psInstruction->asArguments[0].IsSymbol() && (psInstruction->asArguments[0].Index() == pcFunction->muiSymbol))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if calls of pcCallee (from mpcCaller) may be replaced by its body:
// a call in a loop by a larger one, and a call (the one at uiCall in
// pcBlock, if given) passing constants the callee compares by a larger one
// again, as those comparisons fold.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::CanInline(CIRFunction* pcCallee, bool bInLoop, CIRBlock* pcCallBlock, size uiCall, size uiHotLimit)
{
	size			ui;
	size			uiLimit;
	size			uiCompared;
	CIRBlock*		pcBlock;
	bool			bReturns;

	if ((pcCallee == NULL) || (pcCallee == mpcCaller) || (pcCallee->muiAttributes & (IRA_Variadic | IRA_NoInline)) || (pcCallee->NumBlocks() == 0))
	{
		return false;
	}
	if ((NumInstructions(mpcCaller) + NumInstructions(pcCallee) > IR_INLINE_MAX_CALLER) || InlinedRecursive(pcCallee))
	{
		//A recursive function's calls of itself would be inlined again in the next round, and again: the caller
		//doubles each round.  It is inlined once.
		return false;
	}
	uiLimit = bInLoop ? muiMaxLoopInstructions : muiMaxInstructions;
	if (uiHotLimit > uiLimit)
	{
		uiLimit = uiHotLimit;
	}
	uiCompared = pcCallBlock ? NumComparedConstants(pcCallee, pcCallBlock, uiCall) : 0;
	uiLimit += IR_INLINE_CONSTANT_BONUS * uiCompared;
	if (mbCallerForwards && (uiHotLimit == 0) && (uiCompared == 0) && !IsThunk(pcCallee))
	{
		//The caller is a jump to the callee: a copy would save only the jump, and copy the work.
		return false;
	}
	if (NumInstructions(pcCallee) > uiLimit)
	{
		return false;
	}
	if ((uiHotLimit == 0) && pcCallBlock && !GrowsWithin(pcCallee, pcCallBlock->masInstructions.Get(uiCall)->asArguments[0], uiLimit))
	{
		return false;
	}
	bReturns = false;
	for (ui = 0; ui < pcCallee->NumBlocks(); ui++)
	{
		pcBlock = pcCallee->GetBlock(ui);
		bReturns |= pcBlock->msJump.uiKind == IRJ_Return;
		if ((pcBlock->msJump.uiKind == IRJ_Throw) && !CalleeNeverThrows(pcCallee))
		{
			//Its throws go to the test the caller makes of the call (InlineCall).  With no test (a front end always writes
			//one after a call that may throw) a throw would become the caller's own: it is left a call.
			if (!pcCallBlock || (uiCall + 1 >= pcCallBlock->masInstructions.NumElements()) || (pcCallBlock->masInstructions.Get(uiCall + 1)->uiOpcode != IRO_Thrown))
			{
				return false;
			}
		}
	}
	return bReturns;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the callee (of this module, or of the imports') is marked as never
// throwing: a throw in it is not reached.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::CalleeNeverThrows(CIRFunction* pcCallee)
{
	return (mpcCalleeModule ? mpcCalleeModule : mpcModule)->IsNoThrow(pcCallee->muiSymbol);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether inlining every call of pcCallee (sSymbol) left in mpcCaller grows
// it by at most uiLimit instructions: each would be a copy, less the call
// and its arguments that the copy replaces.  One call always does (the
// callee is no bigger than uiLimit); a function called from many places in
// one caller (43 calls of CTypeNames::AddType in Init) stays a call.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::GrowsWithin(CIRFunction* pcCallee, SIRReference sSymbol, size uiLimit)
{
	size	uiInstructions;
	size	uiReplaced;
	size	uiCopies;

	uiInstructions = NumInstructions(pcCallee);
	uiReplaced = 1 + pcCallee->masParameters.NumElements();
	if (uiInstructions <= uiReplaced)
	{
		return true;
	}
	uiCopies = NumCallsOf(sSymbol);
	return uiCopies * (uiInstructions - uiReplaced) <= uiLimit;
}


//////////////////////////////////////////////////////////////////////////
//
// How many calls of sSymbol mpcCaller makes.
//
//////////////////////////////////////////////////////////////////////////
size CIRInliner::NumCallsOf(SIRReference sSymbol)
{
	size				uiBlock;
	size				ui;
	size				uiCalls;
	CIRBlock*			pcBlock;
	SIRInstruction*		psInstruction;

	uiCalls = 0;
	for (uiBlock = 0; uiBlock < mpcCaller->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcCaller->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if ((psInstruction->uiOpcode == IRO_Call) && psInstruction->asArguments[0].Equals(sSymbol))
			{
				uiCalls++;
			}
		}
	}
	return uiCalls;
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the call's arguments are constants (or symbols) whose
// parameter the callee compares or branches on.
//
//////////////////////////////////////////////////////////////////////////
size CIRInliner::NumComparedConstants(CIRFunction* pcCallee, CIRBlock* pcCallBlock, size uiCall)
{
	size			uiFirst;
	size			uiParameter;
	size			uiBlock;
	size			uiInstruction;
	size			uiCount;
	SIRReference	sArgument;
	SIRReference	sParameter;
	SIRInstruction*	psInstruction;
	CIRBlock*		pcBlock;
	bool			bCompared;

	uiFirst = uiCall;
	while ((uiFirst > 0) && (pcCallBlock->masInstructions.Get(uiFirst - 1)->uiOpcode == IRO_Arg))
	{
		uiFirst--;
	}
	uiCount = 0;
	for (uiParameter = 0; (uiParameter < uiCall - uiFirst) && (uiParameter < pcCallee->masParameters.NumElements()); uiParameter++)
	{
		sArgument = pcCallBlock->masInstructions.Get(uiFirst + uiParameter)->asArguments[0];
		if (!sArgument.IsConstant() && !sArgument.IsSymbol())
		{
			continue;
		}
		sParameter = *pcCallee->masParameters.Get(uiParameter);
		bCompared = false;
		for (uiBlock = 0; !bCompared && (uiBlock < pcCallee->NumBlocks()); uiBlock++)
		{
			pcBlock = pcCallee->GetBlock(uiBlock);
			bCompared = pcBlock->msJump.sArgument.Equals(sParameter) && (pcBlock->msJump.uiKind == IRJ_Branch);
			for (uiInstruction = 0; !bCompared && (uiInstruction < pcBlock->masInstructions.NumElements()); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				bCompared = IsIRIntegerComparison(psInstruction->uiOpcode) && (psInstruction->asArguments[0].Equals(sParameter) || psInstruction->asArguments[1].Equals(sParameter));
			}
		}
		if (bCompared)
		{
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// The function psInstruction calls when it is a direct call (a callee of
// this module or an import, mpcCalleeModule set for an import), else NULL.
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CIRInliner::DirectCallee(SIRInstruction* psInstruction)
{
	CIRFunction*	pcCallee;

	if ((psInstruction->uiOpcode != IRO_Call) || !psInstruction->asArguments[0].IsSymbol())
	{
		return NULL;
	}
	pcCallee = Callee(psInstruction->asArguments[0], true);
	mpcCalleeModule = (pcCallee && (pcCallee != mpcModule->FindFunction(psInstruction->asArguments[0].Index()))) ? mpcImports->GetModule() : NULL;
	return pcCallee;
}


//////////////////////////////////////////////////////////////////////////
//
// Inlines pcCallee at the call uiInstruction of block uiBlock.  The rest of
// the block is moved to a new block; the new blocks are in the block's
// loop.  False if it couldn't be.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::InlineAt(size uiBlock, size uiInstruction, CIRFunction* pcCallee)
{
	size	uiBlocks;
	bool	bInLoop;

	bInLoop = *mabInLoop.Get(uiBlock);
	uiBlocks = mpcCaller->NumBlocks();
	if (!InlineCall(mpcCaller->GetBlock(uiBlock), uiInstruction, pcCallee))
	{
		return false;
	}
	if (CallsItself(pcCallee))
	{
		mapcRecursiveCallers.Add(&mpcCaller);
		mapcRecursiveCallees.Add(&pcCallee);
	}
	for (; uiBlocks < mpcCaller->NumBlocks(); uiBlocks++)
	{
		mabInLoop.Add(&bInLoop);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// With a profile: inlines the call that was made most (of those that can
// be, a hot one's callee up to muiHotInstructions; calls the profile
// doesn't know last, in order).  False if none can be.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::InlineHottest(void)
{
	size			uiBlock;
	size			uiInstruction;
	size			uiBestBlock;
	size			uiBestInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	CIRFunction*	pcCallee;
	CIRFunction*	pcBest;
	CIRModule*		pcBestModule;
	uint64			ulliCount;
	uint64			ulliBest;
	bool			bInLoop;
	bool			bFound;

	bFound = false;
	ulliBest = 0;
	uiBestBlock = 0;
	uiBestInstruction = 0;
	pcBest = NULL;
	pcBestModule = NULL;
	for (uiBlock = 0; uiBlock < mpcCaller->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcCaller->GetBlock(uiBlock);
		bInLoop = *mabInLoop.Get(uiBlock);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if ((psInstruction->uiOpcode != IRO_Call) || !psInstruction->asArguments[0].IsSymbol())
			{
				continue;
			}
			ulliCount = mpcProfile->CallCount(mpcCaller, psInstruction);
			if (bFound && ((ulliCount == IR_INLINE_UNKNOWN) || ((ulliBest != IR_INLINE_UNKNOWN) && (ulliCount <= ulliBest))))
			{
				continue;
			}
			pcCallee = DirectCallee(psInstruction);
			if (!CanInline(pcCallee, bInLoop, pcBlock, uiInstruction, HotLimit(ulliCount)))
			{
				continue;
			}
			bFound = true;
			ulliBest = ulliCount;
			uiBestBlock = uiBlock;
			uiBestInstruction = uiInstruction;
			pcBest = pcCallee;
			pcBestModule = mpcCalleeModule;
		}
	}
	if (!bFound)
	{
		return false;
	}

	mpcCalleeModule = pcBestModule;
	return InlineAt(uiBestBlock, uiBestInstruction, pcBest);
}


//////////////////////////////////////////////////////////////////////////
//
// How big a callee a call made ulliCount times in the profiled run may
// inline: muiHotInstructions when it is hot (inlining it saves
// IR_INLINE_HOT_SAVING cycles each time, and together that is at least
// 1/IR_INLINE_HOT_SHARE of the run), else 0 (or not known).
//
//////////////////////////////////////////////////////////////////////////
size CIRInliner::HotLimit(uint64 ulliCount)
{
	if ((ulliCount == IR_INLINE_UNKNOWN) || (mpcProfile->ProfileCycles() == 0))
	{
		return 0;
	}
	if (ulliCount * IR_INLINE_HOT_SAVING * IR_INLINE_HOT_SHARE >= mpcProfile->ProfileCycles())
	{
		return muiHotInstructions;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A profile of the program: the hottest calls are inlined first, and a hot
// one's callee may have up to uiHotInstructions instructions.  NULL for
// none.
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::SetProfile(CIRInlineProfile* pcProfile, size uiHotInstructions)
{
	mpcProfile = pcProfile;
	muiHotInstructions = uiHotInstructions;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function that only passes a call on (IsForwarder) is a jump to
// its callee in the back end: it then inlines only callees no bigger than
// itself (IsThunk), unless the call is hot or passes a constant the callee
// compares.
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::SetForwardersJump(bool bForwardersJump)
{
	mbForwardersJump = bForwardersJump;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction only passes a call on, as a back end's jump would
// (CIRW65C816Backend::Forwarder): one block of a named call whose arguments
// are parameters, constants or a parameter plus or minus a constant
// ('this' moved to a member, at most one), returning nothing or what the call returns.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::IsForwarder(CIRFunction* pcFunction)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRInstruction*	psCall;
	SIRReference	sAdjusted;
	SIRReference	sValue;
	size			ui;

	if (pcFunction->NumBlocks() != 1)
	{
		return false;
	}
	pcBlock = pcFunction->GetBlock(0);
	if (pcBlock->msJump.uiKind != IRJ_Return)
	{
		return false;
	}
	psCall = NULL;
	sAdjusted = IRNone();
	for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		sValue = psInstruction->asArguments[0];
		if (psCall)
		{
			return false;
		}
		if (psInstruction->uiOpcode == IRO_Call)
		{
			if (!sValue.IsSymbol())
			{
				return false;
			}
			psCall = psInstruction;
		}
		else if (psInstruction->uiOpcode == IRO_Arg)
		{
			if (!sValue.IsConstant() && !IsParameter(pcFunction, sValue) && !(!sAdjusted.IsNone() && sValue.Equals(sAdjusted)))
			{
				return false;
			}
		}
		else if (((psInstruction->uiOpcode == IRO_PtrAdd) || (psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_Sub)) && sAdjusted.IsNone() &&
				 IsParameter(pcFunction, sValue) && psInstruction->asArguments[1].IsConstant())
		{
			sAdjusted = psInstruction->sResult;
		}
		else
		{
			return false;
		}
	}
	return psCall && (pcBlock->msJump.sType.IsNone() || pcBlock->msJump.sArgument.Equals(psCall->sResult));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sValue is one of pcFunction's parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::IsParameter(CIRFunction* pcFunction, SIRReference sValue)
{
	size	ui;

	for (ui = 0; ui < pcFunction->masParameters.NumElements(); ui++)
	{
		if (sValue.Equals(*pcFunction->masParameters.Get(ui)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction is no bigger than a forwarder: one block, returning,
// of at most IR_INLINE_THUNK_INSTRUCTIONS instructions besides a call's
// arguments.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::IsThunk(CIRFunction* pcFunction)
{
	CIRBlock*	pcBlock;
	size		ui;
	size		uiInstructions;

	if (pcFunction->NumBlocks() != 1)
	{
		return false;
	}
	pcBlock = pcFunction->GetBlock(0);
	uiInstructions = 0;
	for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
	{
		if (pcBlock->masInstructions.Get(ui)->uiOpcode != IRO_Arg)
		{
			uiInstructions++;
		}
	}
	return (pcBlock->msJump.uiKind == IRJ_Return) && (uiInstructions <= IR_INLINE_THUNK_INSTRUCTIONS);
}


//////////////////////////////////////////////////////////////////////////
//
// Inlines the calls in pcCaller of small functions of the module (and the
// calls the copies make).  Returns true if any were.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::InlineCalls(CIRFunction* pcCaller)
{
	bool	bResult;

	//Imports are read while other units may be exporting (CIRImports::BeginRead).
	if (mpcImports)
	{
		mpcImports->BeginRead();
	}
	bResult = InlineCallsReading(pcCaller);
	if (mpcImports)
	{
		mpcImports->EndRead();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// InlineCalls, with the imports (if any) being read.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::InlineCallsReading(CIRFunction* pcCaller)
{
	size			uiBlock;
	size			uiInstruction;
	size			uiInlines;
	CIRBlock*		pcBlock;
	CIRFunction*	pcCallee;
	bool			bInLoop;

	mpcCaller = pcCaller;
	FindLoops();

	//Taken before anything is inlined: a thunk's copy leaves the caller a forwarder (once optimised), its call too.
	mbCallerForwards = mbForwardersJump && IsForwarder(pcCaller);
	uiInlines = 0;
	if (mpcProfile)
	{
		while ((uiInlines < muiMaxInlines) && InlineHottest())
		{
			uiInlines++;
		}
		mpcCaller = NULL;
		return uiInlines > 0;
	}
	for (uiBlock = 0; (uiBlock < pcCaller->NumBlocks()) && (uiInlines < muiMaxInlines); uiBlock++)
	{
		pcBlock = pcCaller->GetBlock(uiBlock);
		bInLoop = *mabInLoop.Get(uiBlock);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			pcCallee = DirectCallee(pcBlock->masInstructions.Get(uiInstruction));

			//The rest of the block is moved to a new block, which is looked at later.
			if (CanInline(pcCallee, bInLoop, pcBlock, uiInstruction) && InlineAt(uiBlock, uiInstruction, pcCallee))
			{
				uiInlines++;
				break;
			}
		}
	}
	mpcCaller = NULL;
	mbCallerForwards = false;
	return uiInlines > 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The function a call of sSymbol calls: the module's, else (with no body
// here) another unit's from mpcImports, else NULL.  bMissing adds the name
// of one in neither to mpaszMissing; mpaszNotImported gets it either way.
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CIRInliner::Callee(SIRReference sSymbol, bool bMissing)
{
	CIRFunction*	pcCallee;

	pcCallee = mpcModule->FindFunction(sSymbol.Index());
	if ((pcCallee && (pcCallee->NumBlocks() != 0)) || !mpcImports)
	{
		return pcCallee;
	}
	pcCallee = mpcImports->Find(mpcModule->GetSymbolName(sSymbol.Index()));
	if (!pcCallee && mpaszNotImported)
	{
		mpaszNotImported->AddIfUnique(mpcModule->GetSymbolName(sSymbol.Index()));
	}
	if (!pcCallee && bMissing && mpaszMissing)
	{
		mpaszMissing->AddIfUnique(mpcModule->GetSymbolName(sSymbol.Index()));
	}
	return pcCallee;
}


//////////////////////////////////////////////////////////////////////////
//
// Callees with no body in the module are looked for in pcImports (other
// units' functions); the names of those not there either are added to
// paszMissing when it isn't NULL (if the inliner was told to look for them),
// and to paszNotImported when it isn't NULL (always).
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::SetImports(CIRImports* pcImports, CArrayChars* paszMissing, CArrayChars* paszNotImported)
{
	mpcImports = pcImports;
	mpaszMissing = paszMissing;
	mpaszNotImported = paszNotImported;
}


//////////////////////////////////////////////////////////////////////////
//
// Which of the caller's blocks are in a loop (CIRLoops),
// worked out only if a call's callee is small enough for a loop but not
// otherwise.
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::FindLoops(void)
{
	CIRDominators	cDominators;
	CIRLoops		cLoops;
	size			ui;
	bool			bInLoop;

	bInLoop = false;
	mabInLoop.FakeSetUsedElements(0);
	if (!HasLoopCandidate())
	{
		for (ui = 0; ui < mpcCaller->NumBlocks(); ui++)
		{
			mabInLoop.Add(&bInLoop);
		}
		return;
	}

	mpcCaller->Renumber();
	mpcCaller->ComputePredecessors();
	cDominators.Init();
	mpcModule->Count(CPPCC_Dominators);
	cDominators.Compute(mpcCaller);
	cLoops.Init();
	cLoops.Compute(mpcCaller, &cDominators);
	for (ui = 0; ui < mpcCaller->NumBlocks(); ui++)
	{
		bInLoop = cLoops.Depth(mpcCaller->GetBlock(ui)) > 0;
		mabInLoop.Add(&bInLoop);
	}
	cLoops.Kill();
	cDominators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// True if the caller calls a function only small enough to inline in a
// loop.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::HasLoopCandidate(void)
{
	size			uiBlock;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	CIRFunction*	pcCallee;

	for (uiBlock = 0; uiBlock < mpcCaller->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcCaller->GetBlock(uiBlock);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if ((psInstruction->uiOpcode == IRO_Call) && psInstruction->asArguments[0].IsSymbol())
			{
				pcCallee = Callee(psInstruction->asArguments[0], false);
				if (CanInline(pcCallee, true, pcBlock, uiInstruction) && !CanInline(pcCallee, false, pcBlock, uiInstruction))
				{
					return true;
				}
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The callee's debug value markers are of its variables, not the
// caller's: they aren't copied.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::CopiesDebugValues(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// What the callee's operand is in the caller: a parameter the argument, a
// temporary its copy, a constant the caller's constant.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRInliner::CopyValue(SIRReference sValue)
{
	SIRReference*	psMapped;
	SIRConstant*	psConstant;
	SIRReference	sSymbol;

	if (sValue.IsTemporary())
	{
		psMapped = masMap.Get(sValue.Index());
		if (psMapped->IsNone())
		{
			*psMapped = mpcCaller->AddTemporary(mpcCallee->GetTemporaryType(sValue));
		}
		return *psMapped;
	}
	if (sValue.IsConstant())
	{
		psConstant = mpcCallee->GetConstant(sValue);
		return mpcCaller->AddConstant(psConstant->sType, psConstant->ulliBits);
	}
	if (sValue.IsSymbol() && mpcCalleeModule)
	{
		//Another unit's function: its symbols are the same names here.
		sSymbol = mpcModule->Symbol(mpcCalleeModule->GetSymbolName(sValue.Index()));
		mpcModule->CopySymbolAttributes(sSymbol.Index(), mpcCalleeModule, sValue.Index(), IRSA_Carried);
		return sSymbol;
	}
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the callee takes a parameter's address (argaddr).
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::TakesArgumentAddress(CIRFunction* pcCallee)
{
	size		ui;
	size		uiInstruction;
	CIRBlock*	pcBlock;

	for (ui = 0; ui < pcCallee->NumBlocks(); ui++)
	{
		pcBlock = pcCallee->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			if (pcBlock->masInstructions.Get(uiInstruction)->uiOpcode == IRO_ArgAddress)
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes a value of the type takes as a parameter.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRInliner::TypeBytes(SIRType sType)
{
	if (sType.IsPointer())
	{
		return mpcModule->GetPointerSize(sType.uiSpace);
	}
	return (sType.Bits() + 7) / 8;
}


//////////////////////////////////////////////////////////////////////////
//
// For a callee that takes its parameters' addresses: a slot in the
// caller's entry block laid out as the parameters (each at its own size,
// one after another, as they are pushed: an object passed by value as
// pieces is its bytes), and the arguments (masArguments) stored to it at
// the end of pcBlock.  msArguments is the slot, IRNone for a callee that
// takes no parameter's address.
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::StoreArguments(CIRBlock* pcBlock, CIRFunction* pcCallee)
{
	size			ui;
	int32			iOffset;
	SIRType			sType;
	SIRReference	sArgument;
	SIRReference	sAddress;

	msArguments = IRNone();
	maiArgumentOffsets.FakeSetUsedElements(0);
	if (!TakesArgumentAddress(pcCallee))
	{
		return;
	}
	iOffset = 0;
	for (ui = 0; ui < pcCallee->masParameters.NumElements(); ui++)
	{
		maiArgumentOffsets.Add(&iOffset);
		iOffset += TypeBytes(pcCallee->GetTemporaryType(*pcCallee->masParameters.Get(ui)));
	}
	msArguments = mpcCaller->Slot(mpcCaller->GetEntry(), IRType(IRTK_Pointer), (uint32)iOffset, 4);
	for (ui = 0; ui < pcCallee->masParameters.NumElements(); ui++)
	{
		sType = pcCallee->GetTemporaryType(*pcCallee->masParameters.Get(ui));
		sArgument = *masArguments.Get(ui);
		sAddress = msArguments;
		if (*maiArgumentOffsets.Get(ui) != 0)
		{
			sAddress = mpcCaller->Add(pcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), msArguments, mpcCaller->AddInteger(IRIntegerType(mpcModule->GetPointerSize(0)), *maiArgumentOffsets.Get(ui)));
		}
		mpcCaller->Store(pcBlock, sType, sArgument, sAddress);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the call at uiCall in pcBlock (and the args before it) with a
// copy of pcCallee.  Returns false (changing nothing) if the arguments
// don't match its parameters.
//
// A thrown straight after the call, with nothing after it but a brif on it
// and read by nothing else (what a front end writes), needs no test once the
// callee is copied: each of the callee's returns goes on to where the brif
// went when nothing was thrown, and each of its throws jumps straight to the
// caller's code for the exception.  In any other shape the thrown becomes a
// phi at the head of the rest of the block, false from each return and true
// from each throw, which jump there too (the result's phi has a zero from
// them, which nothing reads).  A callee that never throws has no throw that
// is reached (one behind the test of a call to another that never throws
// becomes unreachable), so the caller's code for its exception goes.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::InlineCall(CIRBlock* pcBlock, size uiCall, CIRFunction* pcCallee)
{
	size					uiFirst;
	size					ui;
	size					uiSuccessor;
	size					uiPhi;
	CIRBlock*				pcRest;
	CIRBlock*				pcCopy;
	CIRBlock*				pcSuccessor;
	SIRPhiArgument*			psArgument;
	SIRInstruction			sCall;
	SIRInstruction			sThrown;
	SIRPhi*					psPhi;
	SIRInlineReturn*		psReturn;
	CArrayTemplate<uint32>	auiUses;
	CIRBlock*				pcLanding;
	SIRReference			sValue;
	size					uiRest;
	size					uiArgument;
	bool					bTested;
	bool					bDirect;

	uiFirst = uiCall;
	while ((uiFirst > 0) && (pcBlock->masInstructions.Get(uiFirst - 1)->uiOpcode == IRO_Arg))
	{
		uiFirst--;
	}
	if (uiCall - uiFirst != pcCallee->masParameters.NumElements())
	{
		return false;
	}
	sCall = *pcBlock->masInstructions.Get(uiCall);
	uiRest = uiCall + 1;
	bTested = (uiRest < pcBlock->masInstructions.NumElements()) && (pcBlock->masInstructions.Get(uiRest)->uiOpcode == IRO_Thrown);
	if (bTested)
	{
		sThrown = *pcBlock->masInstructions.Get(uiRest);
		uiRest++;
	}
	mbJoinThrows = bTested && !CalleeNeverThrows(pcCallee);
	bDirect = bTested && (uiRest == pcBlock->masInstructions.NumElements()) && (pcBlock->msJump.uiKind == IRJ_Branch) && pcBlock->msJump.sArgument.Equals(sThrown.sResult);
	if (bDirect)
	{
		auiUses.Init();
		mpcCaller->CountUses(&auiUses);
		bDirect = *auiUses.Get(sThrown.sResult.Index()) == 1;
		auiUses.Kill();
	}

	//Another unit's function's sources are its tokens, not this unit's: its copy is the call's line's code (a listing
	//shows it under the call).
	muiImportSource = mpcCalleeModule ? sCall.uiSource : 0;

	//The parameters are the arguments; the callee's other temporaries get copies when they are first met.
	masMap.FakeSetUsedElements(0);
	masMap.GrowToAtLeastNumElements(pcCallee->NumTemporaries(), true, 0);
	masArguments.FakeSetUsedElements(0);
	for (ui = 0; ui < pcCallee->masParameters.NumElements(); ui++)
	{
		masArguments.Add(&pcBlock->masInstructions.Get(uiFirst + ui)->asArguments[0]);
		*masMap.Get(pcCallee->masParameters.Get(ui)->Index()) = *masArguments.Get(ui);
	}

	//The rest of the block, after the call, goes to a block of its own with the block's jump.
	pcRest = mpcCaller->AddBlock();
	pcRest->TakeInstructions(pcBlock, uiRest, 0);
	pcRest->msJump = pcBlock->msJump;
	for (uiSuccessor = 0; uiSuccessor < pcRest->NumSuccessors(); uiSuccessor++)
	{
		pcSuccessor = pcRest->GetSuccessor(uiSuccessor);
		for (uiPhi = 0; uiPhi < pcSuccessor->masPhis.NumElements(); uiPhi++)
		{
			psArgument = pcSuccessor->FindPhiArgument(pcSuccessor->masPhis.Get(uiPhi), pcBlock);
			if (psArgument)
			{
				psArgument->pcBlock = pcRest;
			}
		}
	}
	pcBlock->TruncateInstructions(uiFirst);
	StoreArguments(pcBlock, pcCallee);

	//The copies of the callee's blocks, made first so jumps can name them.
	mapcCopies.FakeSetUsedElements(0);
	for (ui = 0; ui < pcCallee->NumBlocks(); ui++)
	{
		mapcCopies.Add(mpcCaller->AddBlock());
		pcCallee->GetBlock(ui)->muiIndex = ui;
	}
	masReturns.FakeSetUsedElements(0);
	masThrows.FakeSetUsedElements(0);
	pcLanding = bDirect ? pcRest->msJump.pcThen : NULL;
	mpcThrowsTo = bDirect ? pcLanding : pcRest;
	mpcCallee = pcCallee;
	mpcRest = pcRest;
	for (ui = 0; ui < pcCallee->NumBlocks(); ui++)
	{
		pcCopy = mapcCopies.GetPtr(ui);
		CopyBlock(pcCallee->GetBlock(ui), pcCopy);
		if (mpcCalleeModule)
		{
			pcCopy->msJump.uiSource = muiImportSource;
		}
	}
	pcBlock->Jump(mapcCopies.GetPtr(0));

	//The call's result is a phi of the values returned (and a zero from each throw, where it isn't read).
	if (sCall.sResult.IsTemporary())
	{
		psPhi = pcRest->AddPhi(mpcCaller->GetTemporaryType(sCall.sResult), sCall.sResult);
		for (ui = 0; ui < masReturns.NumElements(); ui++)
		{
			psReturn = masReturns.Get(ui);
			pcRest->AddPhiArgument(psPhi, psReturn->pcBlock, psReturn->sValue);
		}
		for (ui = 0; !bDirect && (ui < masThrows.NumElements()); ui++)
		{
			pcRest->AddPhiArgument(psPhi, masThrows.Get(ui)->pcBlock, mpcCaller->AddConstant(mpcCaller->GetTemporaryType(sCall.sResult), 0));
		}
	}

	if (bDirect)
	{
		//The returns go on; the throws have gone to the caller's code for the exception, whose phis have from each of them
		//what they had from the block with the test (the call's result isn't a value there: a zero).
		for (uiPhi = 0; uiPhi < pcLanding->masPhis.NumElements(); uiPhi++)
		{
			psPhi = pcLanding->masPhis.Get(uiPhi);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				if (psPhi->asArguments.Get(uiArgument)->pcBlock == pcRest)
				{
					sValue = psPhi->asArguments.Get(uiArgument)->sValue;
					if (sValue.Equals(sCall.sResult))
					{
						sValue = mpcCaller->AddConstant(mpcCaller->GetTemporaryType(sCall.sResult), 0);
					}
					psPhi->asArguments.RemoveAt(uiArgument, true);
					for (ui = 0; ui < masThrows.NumElements(); ui++)
					{
						pcLanding->AddPhiArgument(psPhi, masThrows.Get(ui)->pcBlock, sValue);
					}
					break;
				}
			}
		}
		pcRest->Jump(pcRest->msJump.pcElse);
	}
	else if (bTested && sThrown.sResult.IsTemporary())
	{
		//The call's thrown is a phi of which way the callee was left.
		psPhi = pcRest->AddPhi(IRType(IRTK_I1), sThrown.sResult);
		for (ui = 0; ui < masReturns.NumElements(); ui++)
		{
			pcRest->AddPhiArgument(psPhi, masReturns.Get(ui)->pcBlock, mpcCaller->AddConstant(IRType(IRTK_I1), 0));
		}
		for (ui = 0; ui < masThrows.NumElements(); ui++)
		{
			pcRest->AddPhiArgument(psPhi, masThrows.Get(ui)->pcBlock, mpcCaller->AddConstant(IRType(IRTK_I1), 1));
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The copy of one of the callee's blocks (each has one before any is
// filled).
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRInliner::CopyOf(CIRBlock* pcBlock)
{
	return mapcCopies.GetPtr(pcBlock->muiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The operands of a callee's instruction in the caller.  Another unit's
// instruction has the call's source: its own are that unit's tokens.
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::CopyOperands(SIRInstruction* psFrom, SIRInstruction* psCopy)
{
	if (mpcCalleeModule)
	{
		psCopy->uiSource = muiImportSource;
	}
	if ((psFrom->uiOpcode == IRO_ArgAddress) && !msArguments.IsNone())
	{
		//A parameter's address is its place in the arguments' slot.
		psCopy->uiOpcode = IRO_PtrAdd;
		psCopy->asArguments[0] = msArguments;
		psCopy->asArguments[1] = mpcCaller->AddInteger(IRIntegerType(mpcModule->GetPointerSize(0)), *maiArgumentOffsets.Get((size)mpcCallee->GetConstant(psFrom->asArguments[0])->ulliBits));
	}
	else
	{
		CIRBlockCopier::CopyOperands(psFrom, psCopy);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The callee returns: its copy goes on to the rest of the call's block,
// which takes the value returned from masReturns.
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::CopyReturn(CIRBlock* pcFrom, CIRBlock* pcCopy)
{
	SIRInlineReturn*	psReturn;

	psReturn = masReturns.Add();
	psReturn->pcBlock = pcCopy;
	psReturn->sValue = pcFrom->msJump.sType.IsNone() ? IRNone() : CopyValue(pcFrom->msJump.sArgument);
	pcCopy->Jump(mpcRest);
}


//////////////////////////////////////////////////////////////////////////
//
// The callee throws: its copy goes to where the call's exception is seen
// to, when the call is tested.
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::CopyThrow(CIRBlock* pcFrom, CIRBlock* pcCopy)
{
	SIRInlineReturn*	psReturn;

	if (mbJoinThrows)
	{
		psReturn = masThrows.Add();
		psReturn->pcBlock = pcCopy;
		psReturn->sValue = IRNone();
		pcCopy->Jump(mpcThrowsTo);
	}
	else
	{
		//Not reached: the callee never throws (CanInline, CalleeNeverThrows).
		pcCopy->Unreachable();
	}
}
