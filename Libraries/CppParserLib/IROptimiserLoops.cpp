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
#include "IRSSAUpdater.h"
#include "IROptimiserInternal.h"


//The optimiser's loop passes: preheaders, fewer loop tests, loads sunk, exit values replaced, dead loops deleted,
//invariants hoisted, and loops rotated and unswitched.


//////////////////////////////////////////////////////////////////////////
//
// True if an instruction may be moved out of a loop when its operands are
// worked out outside it: integer operations, conversions, comparisons and
// loads that aren't volatile.  One that may trap (IsIRMayTrap: division)
// only by a constant that can't: not 0, and not -1 for a signed one (the
// most negative value divided by it overflows).
//
//////////////////////////////////////////////////////////////////////////
bool IROptimiserHoistable(CIRFunction* pcFunction, SIRInstruction* psInstruction)
{
	uint64	ulliDivisor;
	bool	bSigned;

	if (IsIRMayTrap(psInstruction->uiOpcode))
	{
		if (!psInstruction->asArguments[1].IsConstant() || !psInstruction->sResult.IsTemporary())
		{
			return false;
		}
		bSigned = (psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem);
		ulliDivisor = IRTruncate(psInstruction->sType, pcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
		return (ulliDivisor != 0) && (!bSigned || (ulliDivisor != IRTruncate(psInstruction->sType, (uint64)-1)));
	}
	switch (psInstruction->uiOpcode)
	{
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
							   (iStep > 0 ? (lliValue < IRSignedMax(iBits)) : (lliValue > IRSignedMin(iBits)));
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
	uint32					uiSource;
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

			//Adding to the block may move its instructions (psCompare too).
			sType = psCompare->sType;
			uiSource = psCompare->uiSource;
			sStepped = mpcFunction->AddTemporary(sType);
			sTest = mpcFunction->AddTemporary(IRType(IRTK_I1));
			psNew = pcBlock->masInstructions.Add();
			psNew->uiOpcode = bStepB ? IRO_Add : IRO_Sub;
			psNew->uiFlags = 0;
			psNew->uiNamed = 0;
			psNew->uiSource = uiSource;
			psNew->sType = sType;
			psNew->sResult = sStepped;
			psNew->asArguments[0] = bStepB ? sB : sA;
			psNew->asArguments[1] = mpcFunction->AddInteger(sType, 1);
			psNew = pcBlock->masInstructions.Add();
			psNew->uiOpcode = IRO_SLt;
			psNew->uiFlags = 0;
			psNew->uiNamed = 0;
			psNew->uiSource = uiSource;
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
// A preheader for a loop entered from several blocks (each loop before it
// leaving straight to its head, say): the blocks outside that jumped to the
// head jump to it, phis in it take what they passed the head's phis, and the
// head's phis take those from it.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIROptimiser::AddMergedPreheader(size uiLoop)
{
	CIRBlock*		pcPreheader;
	CIRBlock*		pcHead;
	CIRBlock*		pcPredecessor;
	SIRPhi*			psPhi;
	SIRPhi*			psMerged;
	SIRPhiArgument*	psArgument;
	SIRType			sType;
	size			ui;
	size			uiArgument;

	pcHead = mcLoops.GetLoop(uiLoop)->pcHead;
	pcPreheader = mpcFunction->AddBlock();
	for (ui = 0; ui < pcHead->masPhis.NumElements(); ui++)
	{
		psPhi = pcHead->masPhis.Get(ui);
		sType = mpcFunction->GetTemporaryType(psPhi->sResult);
		psMerged = pcPreheader->AddPhi(sType, mpcFunction->AddTemporary(sType));
		for (uiArgument = psPhi->asArguments.NumElements(); uiArgument > 0; uiArgument--)
		{
			psArgument = psPhi->asArguments.Get(uiArgument - 1);
			if (!mcLoops.Contains(uiLoop, psArgument->pcBlock))
			{
				pcPreheader->AddPhiArgument(psMerged, psArgument->pcBlock, psArgument->sValue);
				psPhi->asArguments.RemoveAt(uiArgument - 1, true);
			}
		}
		pcHead->AddPhiArgument(psPhi, pcPreheader, psMerged->sResult);
	}
	for (ui = 0; ui < pcHead->mapcPredecessors.NumElements(); ui++)
	{
		pcPredecessor = pcHead->mapcPredecessors.GetPtr(ui);
		if (!mcLoops.Contains(uiLoop, pcPredecessor))
		{
			RedirectJump(pcPredecessor, pcHead, pcPreheader);
		}
	}
	pcPreheader->Jump(pcHead);
	mpcFunction->ComputePredecessors();
	mcLoops.SetPreheader(uiLoop, pcPreheader);
	return pcPreheader;
}


//////////////////////////////////////////////////////////////////////////
//
// True if an instruction of pcBlock from uiStart up to uiEnd may write the
// iBytes at sRoot + lliOffset: a store that may alias them (MayAlias) or
// anything else with an effect.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::WritesBetween(CIRBlock* pcBlock, size uiStart, size uiEnd, SIRReference sRoot, int64 lliOffset, int iBytes)
{
	size			ui;
	SIRInstruction*	psInstruction;
	SIRAvailable	sStore;

	for (ui = uiStart; ui < uiEnd; ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		if (psInstruction->uiOpcode == IRO_Store)
		{
			SplitAddress(psInstruction->asArguments[1], &sStore.sRoot, &sStore.lliOffset);
			sStore.sType = psInstruction->sType;
			sStore.sValue = IRNone();
			if (MayAlias(&sStore, sRoot, lliOffset, iBytes))
			{
				return true;
			}
		}
		else if (MayWriteMemory(psInstruction))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the load at uiLoad in pcFrom to just before its only use, at uiUse
// in pcTo, if nothing between can write what it reads: the rest of pcFrom,
// the blocks on the ways from pcFrom to pcTo, and pcTo up to the use.
// Every loop pcTo is in must have pcFrom in it, or the use could be done
// again without the load (and the load isn't done more often).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SinkLoad(CIRBlock* pcFrom, size uiLoad, CIRBlock* pcTo, size uiUse)
{
	SIRInstruction			sLoad;
	SIRReference			sRoot;
	int64					lliOffset;
	int						iBytes;
	CArrayTemplate<uint8>	abForward;
	CArrayTemplate<uint8>	abBackward;
	CArrayIRBlocks			apcWork;
	CIRBlock*				pcBlock;
	CIRBlock*				pcNext;
	size					ui;
	size					uiLoop;
	bool					bWrites;

	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		if (mcLoops.Contains(uiLoop, pcTo) && !mcLoops.Contains(uiLoop, pcFrom))
		{
			return false;
		}
	}
	sLoad = *pcFrom->masInstructions.Get(uiLoad);
	SplitAddress(sLoad.asArguments[0], &sRoot, &lliOffset);
	iBytes = mpcModule->GetSize(sLoad.sType);
	if (WritesBetween(pcFrom, uiLoad + 1, pcFrom->masInstructions.NumElements(), sRoot, lliOffset, iBytes) ||
		WritesBetween(pcTo, 0, uiUse, sRoot, lliOffset, iBytes))
	{
		return false;
	}

	//The blocks reached from pcFrom (not going on from pcTo) that reach pcTo (not going back past pcFrom).
	abForward.Init();
	abBackward.Init();
	abForward.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0);
	abBackward.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0);
	apcWork.Init();
	apcWork.Add(pcFrom);
	while (apcWork.NumElements() > 0)
	{
		pcBlock = apcWork.Pop();
		for (ui = 0; ui < pcBlock->NumSuccessors(); ui++)
		{
			pcNext = pcBlock->GetSuccessor(ui);
			if (!*abForward.Get(pcNext->muiIndex))
			{
				*abForward.Get(pcNext->muiIndex) = true;
				if ((pcNext != pcTo) && (pcNext != pcFrom))
				{
					apcWork.Add(pcNext);
				}
			}
		}
	}
	apcWork.Add(pcTo);
	while (apcWork.NumElements() > 0)
	{
		pcBlock = apcWork.Pop();
		for (ui = 0; ui < pcBlock->mapcPredecessors.NumElements(); ui++)
		{
			pcNext = pcBlock->mapcPredecessors.GetPtr(ui);
			if (!*abBackward.Get(pcNext->muiIndex))
			{
				*abBackward.Get(pcNext->muiIndex) = true;
				if ((pcNext != pcFrom) && (pcNext != pcTo))
				{
					apcWork.Add(pcNext);
				}
			}
		}
	}
	bWrites = false;
	for (ui = 0; !bWrites && (ui < mpcFunction->NumBlocks()); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if ((pcBlock != pcFrom) && (pcBlock != pcTo) && *abForward.Get(ui) && *abBackward.Get(ui))
		{
			bWrites = WritesBetween(pcBlock, 0, pcBlock->masInstructions.NumElements(), sRoot, lliOffset, iBytes);
		}
	}
	apcWork.Kill();
	abBackward.Kill();
	abForward.Kill();
	if (bWrites)
	{
		return false;
	}

	pcFrom->masInstructions.RemoveAt(uiLoad, true);
	pcTo->masInstructions.InsertAt(&sLoad, uiUse);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The loop's one way out (from a block in it to one outside), if it has
// only one and is entered from one block.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SingleExit(size uiLoop, CIRBlock** ppcFrom, CIRBlock** ppcTo)
{
	CArrayIRLoopExits	asExits;
	bool				bSingle;

	if (mcLoops.GetLoop(uiLoop)->pcEntering == NULL)
	{
		return false;
	}
	asExits.Init();
	mcLoops.GetExits(uiLoop, &asExits);
	bSingle = asExits.NumElements() == 1;
	if (bSingle)
	{
		*ppcFrom = asExits.Get(0)->pcFrom;
		*ppcTo = asExits.Get(0)->pcTo;
	}
	asExits.Kill();
	return bSingle;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a block outside the loop reads sValue (an instruction, a phi or
// a jump).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::UsedOutside(size uiLoop, SIRReference sValue)
{
	SIRUse	sUse;
	bool	bUse;

	for (bUse = FirstUse(&sUse, sValue); bUse; bUse = NextUse(&sUse))
	{
		if (!mcLoops.Contains(uiLoop, sUse.pcBlock))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks outside the loop read sNew where they read sValue.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::ReplaceOutside(size uiLoop, SIRReference sValue, SIRReference sNew)
{
	SIRUse	sUse;
	bool	bUse;

	for (bUse = FirstUse(&sUse, sValue); bUse; bUse = NextUse(&sUse))
	{
		if (!mcLoops.Contains(uiLoop, sUse.pcBlock))
		{
			*sUse.psSlot = sNew;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Final values: a loop with one way out, left when v + 1 != v0 + d is
// false (v a phi of its head going up by 1 from v0, d not changed by it,
// and the loop only entered when 0 < d, LoopGuarded) goes round d times
// (d < 2^n, as d is not 0).  After it v + 1 is v0 + d, and a value w
// stepped beside v (a phi of the head from w0, its value going round w + t
// or ptradd w, t, t a constant) is w0 + d * t: those are worked out in the
// loop's preheader and blocks outside read them instead.  The loop may
// then be dead (DeleteDeadLoops).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceExitValues(void)
{
	size			uiLoop;
	size			uiPhi;
	size			uiOutside;
	CIRBlock*		pcFrom;
	CIRBlock*		pcTo;
	CIRBlock*		pcHead;
	CIRBlock*		pcPreheader;
	SIRInstruction*	psTest;
	SIRInstruction*	psBound;
	SIRInstruction*	psStep;
	SIRPhi*			psCounter;
	SIRPhi*			psPhi;
	SIRLoopCounter	sLoopCounter;
	SIRReference	sStepped;
	SIRReference	sBound;
	SIRReference	sInitial;
	SIRReference	sCount;
	SIRReference	sOffset;
	SIRReference	sNew;
	SIRReference	sValue;
	SIRType			sType;
	int64			lliStep;
	bool			bRoundOnTrue;
	bool			bChanged;

	ComputeLoops();
	bChanged = false;
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		if (!SingleExit(uiLoop, &pcFrom, &pcTo) || (pcFrom->msJump.uiKind != IRJ_Branch) || !pcFrom->msJump.sArgument.IsTemporary())
		{
			continue;
		}
		pcHead = mcLoops.GetLoop(uiLoop)->pcHead;
		psTest = mcDefinitions.Instruction(pcFrom->msJump.sArgument);
		bRoundOnTrue = pcFrom->msJump.pcElse == pcTo;
		if ((psTest == NULL) || ((psTest->uiOpcode != (bRoundOnTrue ? IRO_Ne : IRO_Eq))))
		{
			continue;
		}

		//v + 1 against v0 + d.
		sStepped = psTest->asArguments[0];
		sBound = psTest->asArguments[1];
		psCounter = NULL;
		for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
		{
			psCounter = pcHead->masPhis.Get(uiPhi);
			if (MatchCounter(uiLoop, psCounter, 0, &sLoopCounter) && (sLoopCounter.lliStep == 1) && sLoopCounter.sNext.Equals(sStepped))
			{
				break;
			}
			psCounter = NULL;
		}
		if ((psCounter == NULL) || !mcLoops.IsInvariant(uiLoop, sBound, &mcDefinitions))
		{
			continue;
		}
		sInitial = sLoopCounter.sInit;
		psBound = mcDefinitions.Instruction(sBound);
		if ((psBound == NULL) || (psBound->uiOpcode != IRO_Add) || !(psBound->asArguments[0].Equals(sInitial) || psBound->asArguments[1].Equals(sInitial)))
		{
			continue;
		}
		sCount = psBound->asArguments[0].Equals(sInitial) ? psBound->asArguments[1] : psBound->asArguments[0];
		if (!(sCount.IsConstant() ? (mpcFunction->GetSigned(sCount) != 0) : LoopGuarded(uiLoop, IRO_ULt, mpcFunction->AddInteger(psCounter->sType, 0), sCount)))
		{
			continue;
		}

		//Something outside reads v + 1 or a value stepped beside it.
		if (!UsedOutside(uiLoop, sStepped))
		{
			for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
			{
				psPhi = pcHead->masPhis.Get(uiPhi);
				if ((psPhi->asArguments.NumElements() == 2) &&
					(UsedOutside(uiLoop, psPhi->asArguments.Get(0)->sValue) || UsedOutside(uiLoop, psPhi->asArguments.Get(1)->sValue)))
				{
					break;
				}
			}
			if (uiPhi == pcHead->masPhis.NumElements())
			{
				continue;
			}
		}

		pcPreheader = mcLoops.GetLoop(uiLoop)->pcPreheader;
		if (pcPreheader == NULL)
		{
			pcPreheader = AddPreheader(uiLoop);
		}
		if (UsedOutside(uiLoop, sStepped))
		{
			ReplaceOutside(uiLoop, sStepped, sBound);
			bChanged = true;
		}
		for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
		{
			psPhi = pcHead->masPhis.Get(uiPhi);
			if ((psPhi == psCounter) || (psPhi->asArguments.NumElements() != 2))
			{
				continue;
			}
			uiOutside = (psPhi->asArguments.Get(0)->pcBlock == pcPreheader) ? 0 : 1;
			if (psPhi->asArguments.Get(uiOutside)->pcBlock != pcPreheader)
			{
				continue;
			}
			sValue = psPhi->asArguments.Get(1 - uiOutside)->sValue;
			psStep = mcDefinitions.Instruction(sValue);
			if ((psStep == NULL) || !mcLoops.Contains(uiLoop, mcDefinitions.Block(sValue)) || !psStep->asArguments[0].Equals(psPhi->sResult) ||
				!psStep->asArguments[1].IsConstant() || !UsedOutside(uiLoop, sValue))
			{
				continue;
			}
			lliStep = mpcFunction->GetSigned(psStep->asArguments[1]);
			sType = psPhi->sType;
			if ((psStep->uiOpcode == IRO_PtrAdd) && sType.IsPointer())
			{
				//w0 + zext(d) * t, the offset an i32.
				sOffset = sCount;
				if (!mpcFunction->GetTemporaryType(sCount).Is(IRTK_I32) && !(sCount.IsConstant()))
				{
					sOffset = mpcFunction->Add(pcPreheader, IRO_ZExt, IRType(IRTK_I32), sCount);
				}
				else if (sCount.IsConstant())
				{
					sOffset = mpcFunction->AddInteger(IRType(IRTK_I32), (int64)IRTruncate(psCounter->sType, (uint64)mpcFunction->GetSigned(sCount)));
				}
				if (lliStep != 1)
				{
					sOffset = mpcFunction->Add(pcPreheader, IRO_Mul, IRType(IRTK_I32), sOffset, mpcFunction->AddInteger(IRType(IRTK_I32), lliStep));
				}
				sNew = mpcFunction->Add(pcPreheader, IRO_PtrAdd, sType, psPhi->asArguments.Get(uiOutside)->sValue, sOffset);
			}
			else if ((psStep->uiOpcode == IRO_Add) && sType.Equals(psCounter->sType))
			{
				sOffset = (lliStep == 1) ? sCount : mpcFunction->Add(pcPreheader, IRO_Mul, sType, sCount, mpcFunction->AddInteger(sType, lliStep));
				sNew = mpcFunction->Add(pcPreheader, IRO_Add, sType, psPhi->asArguments.Get(uiOutside)->sValue, sOffset);
			}
			else
			{
				continue;
			}
			ReplaceOutside(uiLoop, sValue, sNew);
			bChanged = true;
		}
		if (bChanged)
		{
			FindDefinitions();
			return true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A loop that does nothing (no store, call or volatile load) and whose
// values nothing outside reads goes: its preheader jumps to its one way out,
// whose phis take from the preheader what they took from the loop (values
// from before it).  C11 6.8.5 and C++ [intro.progress] let such a loop be
// assumed to end.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::DeleteDeadLoops(void)
{
	size			uiLoop;
	size			uiBlock;
	size			uiIndex;
	size			uiPhi;
	size			uiArgument;
	CIRBlock*		pcFrom;
	CIRBlock*		pcTo;
	CIRBlock*		pcBlock;
	CIRBlock*		pcPreheader;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	bool			bDead;

	ComputeLoops();
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		if (!SingleExit(uiLoop, &pcFrom, &pcTo))
		{
			continue;
		}
		bDead = true;
		for (uiBlock = 0; bDead && (uiBlock < mcLoops.NumBlocks(uiLoop)); uiBlock++)
		{
			pcBlock = mcLoops.GetBlock(uiLoop, uiBlock);
			for (uiIndex = 0; bDead && (uiIndex < pcBlock->masInstructions.NumElements()); uiIndex++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiIndex);
				bDead = !HasEffect(psInstruction) && !(psInstruction->sResult.IsTemporary() && UsedOutside(uiLoop, psInstruction->sResult));
			}
			for (uiPhi = 0; bDead && (uiPhi < pcBlock->masPhis.NumElements()); uiPhi++)
			{
				bDead = !UsedOutside(uiLoop, pcBlock->masPhis.Get(uiPhi)->sResult);
			}
		}
		if (!bDead)
		{
			continue;
		}

		pcPreheader = mcLoops.GetLoop(uiLoop)->pcPreheader;
		if (pcPreheader == NULL)
		{
			pcPreheader = AddPreheader(uiLoop);
		}
		RedirectJump(pcPreheader, mcLoops.GetLoop(uiLoop)->pcHead, pcTo);
		RenamePhiArguments(pcTo, pcFrom, pcPreheader);

		//The head's phis lose the way in (it is unreachable now, and goes with SimplifyCFG).
		for (uiPhi = 0; uiPhi < mcLoops.GetLoop(uiLoop)->pcHead->masPhis.NumElements(); uiPhi++)
		{
			psPhi = mcLoops.GetLoop(uiLoop)->pcHead->masPhis.Get(uiPhi);
			for (uiArgument = psPhi->asArguments.NumElements(); uiArgument > 0; uiArgument--)
			{
				psArgument = psPhi->asArguments.Get(uiArgument - 1);
				if (psArgument->pcBlock == pcPreheader)
				{
					psPhi->asArguments.RemoveAt(uiArgument - 1, true);
				}
			}
		}
		mpcFunction->ComputePredecessors();
		RemoveUnreachable();
		FindDefinitions();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The same instruction in both blocks a brif goes to (each reached only
// from it), reading values from before them, is worked out once at the end
// of the branching block (before the comparison it branches on), and the
// second block's uses read the first's.  Only what can't trap and doesn't
// read memory (IROptimiserHoistable, but loads).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::HoistCommon(void)
{
	CIRBlock*		pcBlock;
	CIRBlock*		pcThen;
	CIRBlock*		pcElse;
	SIRInstruction*	psFirst;
	SIRInstruction*	psSecond;
	SIRInstruction	sMoved;
	size			ui;
	size			uiFirst;
	size			uiSecond;
	size			uiInsert;
	int				iArgument;
	bool			bOutside;
	bool			bChanged;

	FindDefinitions();
	ClearReplacements();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		pcThen = pcBlock->msJump.pcThen;
		pcElse = pcBlock->msJump.pcElse;
		if ((pcBlock->msJump.uiKind != IRJ_Branch) || (pcThen == pcElse) || (pcThen == pcBlock) || (pcElse == pcBlock) ||
			(pcThen->mapcPredecessors.NumElements() != 1) || (pcElse->mapcPredecessors.NumElements() != 1))
		{
			continue;
		}
		for (uiFirst = 0; uiFirst < pcThen->masInstructions.NumElements(); uiFirst++)
		{
			psFirst = pcThen->masInstructions.Get(uiFirst);
			if ((psFirst->uiOpcode == IRO_Load) || !IROptimiserHoistable(mpcFunction, psFirst))
			{
				continue;
			}
			bOutside = true;
			for (iArgument = 0; iArgument < 2; iArgument++)
			{
				bOutside &= !psFirst->asArguments[iArgument].IsTemporary() || (mcDefinitions.Block(psFirst->asArguments[iArgument]) != pcThen);
			}
			if (!bOutside)
			{
				continue;
			}
			psSecond = NULL;
			for (uiSecond = 0; uiSecond < pcElse->masInstructions.NumElements(); uiSecond++)
			{
				psSecond = pcElse->masInstructions.Get(uiSecond);
				if (IROptimiserSameInstruction(mpcFunction, psFirst, psSecond))
				{
					break;
				}
			}
			if (uiSecond == pcElse->masInstructions.NumElements())
			{
				continue;
			}

			//Before the comparison the brif reads, if that is last.
			Replace(psSecond->sResult, psFirst->sResult);
			pcElse->masInstructions.RemoveAt(uiSecond, true);
			sMoved = *psFirst;
			pcThen->masInstructions.RemoveAt(uiFirst, true);
			uiInsert = pcBlock->masInstructions.NumElements();
			if ((uiInsert > 0) && pcBlock->masInstructions.Get(uiInsert - 1)->sResult.Equals(pcBlock->msJump.sArgument) &&
				!sMoved.asArguments[0].Equals(pcBlock->msJump.sArgument) && !sMoved.asArguments[1].Equals(pcBlock->msJump.sArgument))
			{
				uiInsert--;
			}
			pcBlock->masInstructions.InsertAt(&sMoved, uiInsert);
			bChanged = true;
			ApplyReplacements();
			ClearReplacements();
			FindDefinitions();
			uiFirst = (size)-1;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A load whose only use is an instruction in another block goes to just
// before it (SinkLoad): the ways that don't use it don't load it, and a
// target can read it straight into its use (the W65C816 copies a byte
// through A).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SinkLoads(void)
{
	CArrayTemplate<CIRBlock*>	apcUseBlock;	//Per temporary: the block of its one use by an instruction, else NULL.
	CArrayTemplate<size>		auiUseIndex;
	size						ui;
	size						uiInstruction;
	size						uiArgument;
	size						uiTemporary;
	CIRBlock*					pcBlock;
	SIRInstruction*				psInstruction;
	SIRReference				sValue;
	bool						bSunk;
	bool						bChanged;
	int							iSinks;

	ComputeLoops();
	apcUseBlock.Init();
	auiUseIndex.Init();
	bChanged = false;
	for (iSinks = 0; iSinks < IR_SINK_LIMIT; iSinks++)
	{
		//Where each temporary with one use is used (the positions change with each load moved).
		apcUseBlock.FakeSetUsedElements(0);
		auiUseIndex.FakeSetUsedElements(0);
		apcUseBlock.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
		auiUseIndex.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
		for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				for (uiArgument = 0; uiArgument < 2; uiArgument++)
				{
					sValue = psInstruction->asArguments[uiArgument];
					if (sValue.IsTemporary() && (mcDefinitions.NumUses(sValue) == 1))
					{
						*apcUseBlock.Get(sValue.Index()) = pcBlock;
						*auiUseIndex.Get(sValue.Index()) = uiInstruction;
					}
				}
			}
		}

		bSunk = false;
		for (ui = 0; !bSunk && (ui < mpcFunction->NumBlocks()); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uiInstruction = pcBlock->masInstructions.NumElements(); !bSunk && (uiInstruction > 0); uiInstruction--)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction - 1);
				if ((psInstruction->uiOpcode != IRO_Load) || (psInstruction->uiFlags & IRF_Volatile) || !psInstruction->sResult.IsTemporary())
				{
					continue;
				}
				uiTemporary = psInstruction->sResult.Index();
				if ((*apcUseBlock.Get(uiTemporary) == NULL) || (*apcUseBlock.Get(uiTemporary) == pcBlock))
				{
					continue;
				}
				bSunk = SinkLoad(pcBlock, uiInstruction - 1, *apcUseBlock.Get(uiTemporary), *auiUseIndex.Get(uiTemporary));
			}
		}
		if (!bSunk)
		{
			break;
		}
		bChanged = true;
		FindDefinitions();
	}
	auiUseIndex.Kill();
	apcUseBlock.Kill();
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
				else if (MayWriteMemory(psInstruction) && (psInstruction->uiOpcode != IRO_Arg))
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
// to the head (any number of blocks outside may), and a head of at most IR_ROTATE_LIMIT
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
	if ((mcLoops.NumLatches(uiLoop) != 1) || (pcHead->mapcPredecessors.NumElements() < 2))
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
	SIRUse	sUse;
	bool	bUse;

	//The updater may add phis (SetUse finds the use again).
	for (bUse = FirstUse(&sUse, sValue); bUse; bUse = NextUse(&sUse))
	{
		SetUse(&sUse, (sUse.eKind == IRUK_Phi) ? pcUpdater->ValueAtEnd(sUse.psArgument->pcBlock) : pcUpdater->ValueAtStart(sUse.pcBlock));
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
			pcPreheader = mcLoops.GetLoop(uiLoop)->pcEntering ? AddPreheader(uiLoop) : AddMergedPreheader(uiLoop);
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
// pcBlock's copy when it is one of the loop's blocks (apcBlocks, copied to
// apcCopies), else pcBlock.
//
//////////////////////////////////////////////////////////////////////////
static CIRBlock* IROptimiserCopyOf(CArrayIRBlocks* papcBlocks, CArrayIRBlocks* papcCopies, CIRBlock* pcBlock)
{
	size	ui;

	for (ui = 0; ui < papcBlocks->NumElements(); ui++)
	{
		if (papcBlocks->GetPtr(ui) == pcBlock)
		{
			return papcCopies->GetPtr(ui);
		}
	}
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcBlock is one of the loop's blocks or their copies.
//
//////////////////////////////////////////////////////////////////////////
static bool IROptimiserInEither(CArrayIRBlocks* papcBlocks, CArrayIRBlocks* papcCopies, CIRBlock* pcBlock)
{
	size	ui;

	for (ui = 0; ui < papcBlocks->NumElements(); ui++)
	{
		if ((papcBlocks->GetPtr(ui) == pcBlock) || (papcCopies->GetPtr(ui) == pcBlock))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The uses of sValue outside the loop's blocks and their copies are what
// pcUpdater gives (sValue or its copy, or a phi of both).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::RewriteOutsideUses(SIRReference sValue, CArrayIRBlocks* papcBlocks, CArrayIRBlocks* papcCopies, CIRSSAUpdater* pcUpdater)
{
	SIRUse	sUse;
	bool	bUse;

	//A phi's argument by the block it comes from; anything else by its own block.  The updater may add phis (SetUse
	//finds the use again).
	for (bUse = FirstUse(&sUse, sValue); bUse; bUse = NextUse(&sUse))
	{
		if (sUse.eKind == IRUK_Phi)
		{
			if (!IROptimiserInEither(papcBlocks, papcCopies, sUse.psArgument->pcBlock))
			{
				SetUse(&sUse, pcUpdater->ValueAtEnd(sUse.psArgument->pcBlock));
			}
		}
		else if (!IROptimiserInEither(papcBlocks, papcCopies, sUse.pcBlock))
		{
			SetUse(&sUse, pcUpdater->ValueAtStart(sUse.pcBlock));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A loop (of at most IR_UNSWITCH_INSTRUCTIONS) with a branch on a condition
// that is the same each time round is two loops: the loop is copied, the
// branch goes one way in the loop and the other way in the copy, and the
// preheader branches on the condition to one or the other.  The blocks the
// loop leaves to take the copy's values from it too; values used after the
// loop are phis of both (CIRSSAUpdater).  One loop at a time, innermost
// first, at most IR_UNSWITCH_LOOPS each Optimise.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::UnswitchLoops(void)
{
	CArrayIRReferences	asMap;
	CArrayIRBlocks		apcBlocks;
	CArrayIRBlocks		apcCopies;
	CArrayTemplate<SIRUnswitchedValue>	asValues;
	SIRUnswitchedValue*	psValue;
	CIRSSAUpdater		cUpdater;
	SIRInstruction		sCopy;
	SIRLoop*			psLoop;
	CIRBlock*			pcBlock;
	CIRBlock*			pcCopy;
	CIRBlock*			pcSwitch;
	CIRBlock*			pcSwitchCopy;
	CIRBlock*			pcPreheader;
	CIRBlock*			pcSuccessor;
	CIRBlock*			pcThen;
	CIRBlock*			pcElse;
	SIRPhi*				psPhi;
	SIRPhi*				psCopyPhi;
	SIRPhiArgument*		psArgument;
	SIRReference		sCondition;
	SIRReference		sValue;
	size				uiLoop;
	size				ui;
	size				uiIndex;
	size				uiArgument;
	size				uiSize;
	size				uiSuccessor;

	if (miUnswitched >= IR_UNSWITCH_LOOPS)
	{
		return false;
	}
	ComputeLoops();
	for (uiLoop = mcLoops.NumLoops(); uiLoop > 0; uiLoop--)
	{
		psLoop = mcLoops.GetLoop(uiLoop - 1);
		uiSize = 0;
		pcSwitch = NULL;
		for (ui = 0; ui < mcLoops.NumBlocks(uiLoop - 1); ui++)
		{
			pcBlock = mcLoops.GetBlock(uiLoop - 1, ui);
			uiSize += pcBlock->masInstructions.NumElements() + pcBlock->masPhis.NumElements();
			if (!pcSwitch && (pcBlock->msJump.uiKind == IRJ_Branch) && (pcBlock->msJump.pcThen != pcBlock->msJump.pcElse) &&
				pcBlock->msJump.sArgument.IsTemporary() && mcLoops.IsInvariant(uiLoop - 1, pcBlock->msJump.sArgument, &mcDefinitions))
			{
				pcSwitch = pcBlock;
			}
		}
		if (!pcSwitch || (uiSize > IR_UNSWITCH_INSTRUCTIONS))
		{
			continue;
		}

		pcPreheader = psLoop->pcPreheader;
		if (!pcPreheader)
		{
			pcPreheader = psLoop->pcEntering ? AddPreheader(uiLoop - 1) : AddMergedPreheader(uiLoop - 1);
			psLoop = mcLoops.GetLoop(uiLoop - 1);
		}

		//The copy's blocks and a temporary for each value the loop defines.
		apcBlocks.Init();
		apcCopies.Init();
		asMap.Init();
		asMap.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
		for (ui = 0; ui < mcLoops.NumBlocks(uiLoop - 1); ui++)
		{
			pcBlock = mcLoops.GetBlock(uiLoop - 1, ui);
			apcBlocks.Add(pcBlock);
			apcCopies.Add(mpcFunction->AddBlock());
			for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
			{
				sValue = pcBlock->masPhis.Get(uiIndex)->sResult;
				*asMap.Get(sValue.Index()) = mpcFunction->AddTemporary(mpcFunction->GetTemporaryType(sValue));
			}
			for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
			{
				sValue = pcBlock->masInstructions.Get(uiIndex)->sResult;
				if (sValue.IsTemporary())
				{
					*asMap.Get(sValue.Index()) = mpcFunction->AddTemporary(mpcFunction->GetTemporaryType(sValue));
				}
			}
		}

		//The copies: their phis, instructions and jumps taking the copies' values and blocks.
		for (ui = 0; ui < apcBlocks.NumElements(); ui++)
		{
			pcBlock = apcBlocks.GetPtr(ui);
			pcCopy = apcCopies.GetPtr(ui);
			for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
			{
				psPhi = pcBlock->masPhis.Get(uiIndex);
				psCopyPhi = pcCopy->AddPhi(psPhi->sType, IROptimiserMapped(&asMap, psPhi->sResult));
				for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
				{
					psArgument = psPhi->asArguments.Get(uiArgument);
					pcCopy->AddPhiArgument(psCopyPhi, IROptimiserCopyOf(&apcBlocks, &apcCopies, psArgument->pcBlock), IROptimiserMapped(&asMap, psArgument->sValue));
				}
			}
			for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
			{
				sCopy = *pcBlock->masInstructions.Get(uiIndex);
				sCopy.asArguments[0] = IROptimiserMapped(&asMap, sCopy.asArguments[0]);
				sCopy.asArguments[1] = IROptimiserMapped(&asMap, sCopy.asArguments[1]);
				sCopy.sResult = IROptimiserMapped(&asMap, sCopy.sResult);
				pcCopy->masInstructions.Add(&sCopy);
			}
			switch (pcBlock->msJump.uiKind)
			{
			case IRJ_Jump:
				pcCopy->Jump(IROptimiserCopyOf(&apcBlocks, &apcCopies, pcBlock->msJump.pcThen));
				break;
			case IRJ_Branch:
				pcCopy->Branch(IROptimiserMapped(&asMap, pcBlock->msJump.sArgument), IROptimiserCopyOf(&apcBlocks, &apcCopies, pcBlock->msJump.pcThen), IROptimiserCopyOf(&apcBlocks, &apcCopies, pcBlock->msJump.pcElse));
				break;
			case IRJ_Return:
				if (pcBlock->msJump.sType.IsNone())
				{
					pcCopy->Return();
				}
				else
				{
					pcCopy->Return(pcBlock->msJump.sType, IROptimiserMapped(&asMap, pcBlock->msJump.sArgument));
				}
				break;
			default:
				pcCopy->Unreachable();
				break;
			}

			//The blocks the loop leaves to take the copy's values from the copy.
			for (uiSuccessor = 0; uiSuccessor < pcBlock->NumSuccessors(); uiSuccessor++)
			{
				pcSuccessor = pcBlock->GetSuccessor(uiSuccessor);
				if (mcLoops.Contains(uiLoop - 1, pcSuccessor) || ((uiSuccessor == 1) && (pcSuccessor == pcBlock->GetSuccessor(0))))
				{
					continue;
				}
				for (uiIndex = 0; uiIndex < pcSuccessor->masPhis.NumElements(); uiIndex++)
				{
					psPhi = pcSuccessor->masPhis.Get(uiIndex);
					psArgument = pcSuccessor->FindPhiArgument(psPhi, pcBlock);
					if (psArgument)
					{
						sValue = IROptimiserMapped(&asMap, psArgument->sValue);
						pcSuccessor->AddPhiArgument(psPhi, pcCopy, sValue);
					}
				}
			}
		}

		//The branch goes one way in the loop and the other in the copy; the preheader chooses.
		pcSwitchCopy = IROptimiserCopyOf(&apcBlocks, &apcCopies, pcSwitch);
		sCondition = pcSwitch->msJump.sArgument;
		pcThen = pcSwitch->msJump.pcThen;
		pcElse = pcSwitch->msJump.pcElse;
		pcSwitch->Jump(pcThen);
		RemovePhiArguments(pcElse, pcSwitch);
		pcSwitchCopy->Jump(IROptimiserCopyOf(&apcBlocks, &apcCopies, pcElse));
		RemovePhiArguments(IROptimiserCopyOf(&apcBlocks, &apcCopies, pcThen), pcSwitchCopy);
		pcPreheader->Branch(sCondition, psLoop->pcHead, IROptimiserCopyOf(&apcBlocks, &apcCopies, psLoop->pcHead));
		mpcFunction->Renumber();
		mpcFunction->ComputePredecessors();

		//The values the loop defines (with their blocks, and whether each block and its copy can still be reached),
		//before the blocks the branch no longer reaches go: an unreachable cycle would send the updater round it.
		mcDominators.Compute(mpcFunction);
		asValues.Init();
		for (ui = 0; ui < apcBlocks.NumElements(); ui++)
		{
			pcBlock = apcBlocks.GetPtr(ui);
			for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements() + pcBlock->masInstructions.NumElements(); uiIndex++)
			{
				sValue = (uiIndex < pcBlock->masPhis.NumElements()) ? pcBlock->masPhis.Get(uiIndex)->sResult : pcBlock->masInstructions.Get(uiIndex - pcBlock->masPhis.NumElements())->sResult;
				if (sValue.IsTemporary())
				{
					psValue = asValues.Add();
					psValue->sValue = sValue;
					psValue->uiBlock = ui;
					psValue->bReached = mcDominators.IsReachable(pcBlock);
					psValue->bCopyReached = mcDominators.IsReachable(apcCopies.GetPtr(ui));
				}
			}
		}
		RemoveUnreachable();
		mpcFunction->Renumber();
		mpcFunction->ComputePredecessors();

		//Values used after the loop come from the loop or the copy.
		cUpdater.Init(mpcFunction);
		for (ui = 0; ui < asValues.NumElements(); ui++)
		{
			psValue = asValues.Get(ui);
			if (psValue->bReached || psValue->bCopyReached)
			{
				cUpdater.Begin(mpcFunction->GetTemporaryType(psValue->sValue));
				if (psValue->bReached)
				{
					cUpdater.Define(apcBlocks.GetPtr(psValue->uiBlock), psValue->sValue);
				}
				if (psValue->bCopyReached)
				{
					cUpdater.Define(apcCopies.GetPtr(psValue->uiBlock), IROptimiserMapped(&asMap, psValue->sValue));
				}
				RewriteOutsideUses(psValue->sValue, &apcBlocks, &apcCopies, &cUpdater);
			}
		}
		cUpdater.Kill();
		asValues.Kill();
		asMap.Kill();
		apcCopies.Kill();
		apcBlocks.Kill();
		miUnswitched++;
		return true;
	}
	return false;
}
