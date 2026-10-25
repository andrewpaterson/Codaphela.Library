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
#include "IRLiveness.h"
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
	mabInLoop.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::Kill(void)
{
	mabInLoop.Kill();
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
// True if calls of pcCallee (from mpcCaller) may be replaced by its body:
// a call in a loop by a larger one, and a call (the one at uiCall in
// pcBlock, if given) passing constants the callee compares by a larger one
// again, as those comparisons fold.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::CanInline(CIRFunction* pcCallee, bool bInLoop, CIRBlock* pcCallBlock, size uiCall)
{
	size			ui;
	size			uiInstruction;
	size			uiLimit;
	CIRBlock*		pcBlock;
	bool			bReturns;

	if ((pcCallee == NULL) || (pcCallee == mpcCaller) || (pcCallee->muiAttributes & IRA_Variadic) || (pcCallee->NumBlocks() == 0))
	{
		return false;
	}
	uiLimit = bInLoop ? muiMaxLoopInstructions : muiMaxInstructions;
	if (pcCallBlock)
	{
		uiLimit += IR_INLINE_CONSTANT_BONUS * NumComparedConstants(pcCallee, pcCallBlock, uiCall);
	}
	if (NumInstructions(pcCallee) > uiLimit)
	{
		return false;
	}
	bReturns = false;
	for (ui = 0; ui < pcCallee->NumBlocks(); ui++)
	{
		pcBlock = pcCallee->GetBlock(ui);
		bReturns |= pcBlock->msJump.uiKind == IRJ_Return;
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			if (pcBlock->masInstructions.Get(uiInstruction)->uiOpcode == IRO_ArgAddress)
			{
				return false;
			}
		}
	}
	return bReturns;
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
// Inlines the calls in pcCaller of small functions of the module (and the
// calls the copies make).  Returns true if any were.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::InlineCalls(CIRFunction* pcCaller)
{
	size			uiBlock;
	size			uiInstruction;
	size			uiInlines;
	size			uiBlocks;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	CIRFunction*	pcCallee;
	bool			bInLoop;

	mpcCaller = pcCaller;
	FindLoops();
	uiInlines = 0;
	for (uiBlock = 0; (uiBlock < pcCaller->NumBlocks()) && (uiInlines < muiMaxInlines); uiBlock++)
	{
		pcBlock = pcCaller->GetBlock(uiBlock);
		bInLoop = *mabInLoop.Get(uiBlock);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if ((psInstruction->uiOpcode != IRO_Call) || !psInstruction->asArguments[0].IsSymbol())
			{
				continue;
			}
			pcCallee = mpcModule->FindFunction(psInstruction->asArguments[0].Index());
			uiBlocks = pcCaller->NumBlocks();
			if (CanInline(pcCallee, bInLoop, pcBlock, uiInstruction) && InlineCall(pcBlock, uiInstruction, pcCallee))
			{
				//The rest of the block was moved to a new block, which is looked at later; the new blocks are in the
				//block's loop.
				for (; uiBlocks < pcCaller->NumBlocks(); uiBlocks++)
				{
					mabInLoop.Add(&bInLoop);
				}
				uiInlines++;
				break;
			}
		}
	}
	mpcCaller = NULL;
	return uiInlines > 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Which of the caller's blocks are in a loop (CIRLiveness's loop depths),
// worked out only if a call's callee is small enough for a loop but not
// otherwise.
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::FindLoops(void)
{
	CIRDominators	cDominators;
	CIRLiveness		cLiveness;
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
	cDominators.Compute(mpcCaller);
	cLiveness.Init();
	cLiveness.ComputeLoops(mpcCaller, &cDominators);
	for (ui = 0; ui < mpcCaller->NumBlocks(); ui++)
	{
		bInLoop = cLiveness.LoopDepth(mpcCaller->GetBlock(ui)) > 0;
		mabInLoop.Add(&bInLoop);
	}
	cLiveness.Kill();
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
				pcCallee = mpcModule->FindFunction(psInstruction->asArguments[0].Index());
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
// What a callee's operand is in the caller: a parameter the argument, a
// temporary its copy, a constant the caller's constant.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRInliner::Map(SIRReference sValue, CIRFunction* pcCallee)
{
	SIRReference*	psMapped;
	SIRConstant*	psConstant;

	if (sValue.IsTemporary())
	{
		psMapped = masMap.Get(sValue.Index());
		if (psMapped->IsNone())
		{
			*psMapped = mpcCaller->AddTemporary(pcCallee->GetTemporaryType(sValue));
		}
		return *psMapped;
	}
	if (sValue.IsConstant())
	{
		psConstant = pcCallee->GetConstant(sValue);
		return mpcCaller->AddConstant(psConstant->sType, psConstant->ulliBits);
	}
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the call at uiCall in pcBlock (and the args before it) with a
// copy of pcCallee.  Returns false (changing nothing) if the arguments
// don't match its parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInliner::InlineCall(CIRBlock* pcBlock, size uiCall, CIRFunction* pcCallee)
{
	size					uiFirst;
	size					ui;
	size					uiSuccessor;
	size					uiPhi;
	CIRBlock*				pcRest;
	CIRBlock*				pcSuccessor;
	SIRPhiArgument*			psArgument;
	SIRInstruction			sCall;
	SIRPhi*					psPhi;
	SIRInlineReturn*		psReturn;

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

	//The parameters are the arguments; the callee's other temporaries get copies when they are first met.
	masMap.FakeSetUsedElements(0);
	masMap.GrowToAtLeastNumElements(pcCallee->NumTemporaries(), true, 0);
	for (ui = 0; ui < pcCallee->masParameters.NumElements(); ui++)
	{
		*masMap.Get(pcCallee->masParameters.Get(ui)->Index()) = pcBlock->masInstructions.Get(uiFirst + ui)->asArguments[0];
	}

	//The rest of the block, after the call, goes to a block of its own with the block's jump.
	pcRest = mpcCaller->AddBlock();
	if (uiCall + 1 < pcBlock->masInstructions.NumElements())
	{
		pcRest->masInstructions.InsertBlockAt(pcBlock->masInstructions.Get(uiCall + 1), pcBlock->masInstructions.NumElements() - (uiCall + 1), 0);
	}
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
	pcBlock->masInstructions.FakeSetUsedElements(uiFirst);

	//The copies of the callee's blocks, made first so jumps can name them.
	mapcCopies.FakeSetUsedElements(0);
	for (ui = 0; ui < pcCallee->NumBlocks(); ui++)
	{
		mapcCopies.Add(mpcCaller->AddBlock());
		pcCallee->GetBlock(ui)->muiIndex = ui;
	}
	masReturns.FakeSetUsedElements(0);
	for (ui = 0; ui < pcCallee->NumBlocks(); ui++)
	{
		CopyBlock(pcCallee->GetBlock(ui), pcCallee, pcRest);
	}
	pcBlock->Jump(mapcCopies.GetPtr(0));

	//The call's result is a phi of the values returned.
	if (sCall.sResult.IsTemporary())
	{
		psPhi = pcRest->AddPhi(mpcCaller->GetTemporaryType(sCall.sResult), sCall.sResult);
		for (ui = 0; ui < masReturns.NumElements(); ui++)
		{
			psReturn = masReturns.Get(ui);
			pcRest->AddPhiArgument(psPhi, psReturn->pcBlock, psReturn->sValue);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of one of the callee's blocks: its phis, instructions and jump,
// with a return jumping to pcRest.
//
//////////////////////////////////////////////////////////////////////////
void CIRInliner::CopyBlock(CIRBlock* pcFrom, CIRFunction* pcCallee, CIRBlock* pcRest)
{
	CIRBlock*			pcCopy;
	size				ui;
	size				uiArgument;
	SIRPhi*				psFrom;
	SIRPhi*				psPhi;
	SIRPhiArgument*		psArgument;
	SIRInstruction*		psInstruction;
	SIRInstruction		sCopy;
	SIRJump*			psJump;
	SIRInlineReturn*	psReturn;

	pcCopy = mapcCopies.GetPtr(pcFrom->muiIndex);
	for (ui = 0; ui < pcFrom->masPhis.NumElements(); ui++)
	{
		psFrom = pcFrom->masPhis.Get(ui);
		psPhi = pcCopy->AddPhi(psFrom->sType, Map(psFrom->sResult, pcCallee));
		for (uiArgument = 0; uiArgument < psFrom->asArguments.NumElements(); uiArgument++)
		{
			psArgument = psFrom->asArguments.Get(uiArgument);
			pcCopy->AddPhiArgument(psPhi, mapcCopies.GetPtr(psArgument->pcBlock->muiIndex), Map(psArgument->sValue, pcCallee));
		}
	}
	for (ui = 0; ui < pcFrom->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcFrom->masInstructions.Get(ui);
		sCopy = *psInstruction;
		sCopy.asArguments[0] = Map(psInstruction->asArguments[0], pcCallee);
		sCopy.asArguments[1] = Map(psInstruction->asArguments[1], pcCallee);
		sCopy.sResult = Map(psInstruction->sResult, pcCallee);
		pcCopy->masInstructions.Add(&sCopy);
	}

	psJump = &pcFrom->msJump;
	switch (psJump->uiKind)
	{
	case IRJ_Jump:
		pcCopy->Jump(mapcCopies.GetPtr(psJump->pcThen->muiIndex));
		break;
	case IRJ_Branch:
		pcCopy->Branch(Map(psJump->sArgument, pcCallee), mapcCopies.GetPtr(psJump->pcThen->muiIndex), mapcCopies.GetPtr(psJump->pcElse->muiIndex));
		break;
	case IRJ_Return:
		psReturn = masReturns.Add();
		psReturn->pcBlock = pcCopy;
		psReturn->sValue = psJump->sType.IsNone() ? IRNone() : Map(psJump->sArgument, pcCallee);
		pcCopy->Jump(pcRest);
		break;
	default:
		pcCopy->Unreachable();
		break;
	}
}
