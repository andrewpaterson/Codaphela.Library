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
#include "IROptimiserInternal.h"


//The optimiser's loop counter passes: counters matched, narrowed and replaced, and multiplies by them reduced in
//strength.


//////////////////////////////////////////////////////////////////////////
//
// True if sStepped is sValue + 1.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::StepsByOne(SIRReference sStepped, SIRReference sValue)
{
	SIRInstruction*	psStep;

	psStep = mcDefinitions.Instruction(sStepped);
	return psStep && (psStep->uiOpcode == IRO_Add) && psStep->asArguments[0].Equals(sValue) && psStep->asArguments[1].IsConstant() &&
		(mpcFunction->GetSigned(psStep->asArguments[1]) == 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psPhi (of loop uiLoop's head) is a counter: two arguments, one
// (*psCounter's uiOutside, the first unless the first's block is in the
// loop) from a block outside the loop, the other phi + c (c a constant;
// phi - c too with IR_COUNTER_SUB).  With IR_COUNTER_INSIDE the other must
// come from a block in the loop.  *psCounter gets the arguments and the
// step.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::MatchCounter(size uiLoop, SIRPhi* psPhi, uint8 uiFlags, SIRLoopCounter* psCounter)
{
	SIRInstruction*	psStep;
	SIRPhiArgument*	psOutside;
	SIRPhiArgument*	psInside;

	if (psPhi->asArguments.NumElements() != 2)
	{
		return false;
	}
	psCounter->uiOutside = mcLoops.Contains(uiLoop, psPhi->asArguments.Get(0)->pcBlock) ? 1 : 0;
	psOutside = psPhi->asArguments.Get(psCounter->uiOutside);
	psInside = psPhi->asArguments.Get(1 - psCounter->uiOutside);
	if (mcLoops.Contains(uiLoop, psOutside->pcBlock) || ((uiFlags & IR_COUNTER_INSIDE) && !mcLoops.Contains(uiLoop, psInside->pcBlock)))
	{
		return false;
	}
	psCounter->sInit = psOutside->sValue;
	psCounter->sNext = psInside->sValue;
	psStep = mcDefinitions.Instruction(psCounter->sNext);
	if ((psStep == NULL) || ((psStep->uiOpcode != IRO_Add) && ((psStep->uiOpcode != IRO_Sub) || !(uiFlags & IR_COUNTER_SUB))) ||
		!psStep->asArguments[0].Equals(psPhi->sResult) || !psStep->asArguments[1].IsConstant())
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
// LoopGuarded of a compare in the form it looks for (a > b as b < a).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowGuardedAs(size uiLoop, uint16 uiOpcode, SIRReference sA, SIRReference sB)
{
	if (IsIRSwappedComparison(uiOpcode))
	{
		return LoopGuarded(uiLoop, IRSwappedComparison(uiOpcode), sB, sA);
	}
	return LoopGuarded(uiLoop, uiOpcode, sA, sB);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the stepped counter sStepped is used only by the counter's phi,
// the compare, and phis after the loop on the way out of pcBlock (the
// block that tests it), which are added to papsExits.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::SteppedExitUses(size uiLoop, CIRBlock* pcBlock, SIRPhi* psCounter, SIRInstruction* psCompare, SIRReference sStepped, CArrayIRExitArguments* papsExits)
{
	SIRUse	sUse;
	bool	bUse;

	for (bUse = FirstUse(&sUse, sStepped); bUse; bUse = NextUse(&sUse))
	{
		if (sUse.eKind == IRUK_Instruction)
		{
			if (sUse.psInstruction != psCompare)
			{
				return false;
			}
		}
		else if (sUse.eKind == IRUK_Jump)
		{
			if (sUse.pcBlock != pcBlock)
			{
				return false;
			}
		}
		else if (sUse.psPhi != psCounter)
		{
			if (mcLoops.Contains(uiLoop, sUse.pcBlock) || (sUse.psArgument->pcBlock != pcBlock))
			{
				return false;
			}
			papsExits->Add(sUse.psArgument);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the loop is only entered when sInitial uiOpcode sBound
// (LoopGuarded), or the same of the narrower values they extend (the
// guard compares those once FoldConstants narrows it; a constant is the
// same constant narrow).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowLoopGuarded(size uiLoop, uint16 uiOpcode, SIRReference sInitial, SIRReference sBound)
{
	SIRInstruction*	psInitial;
	SIRInstruction*	psBound;
	SIRType			sNarrow;
	SIRReference	sNarrowInitial;
	SIRReference	sNarrowBound;

	if (NarrowGuardedAs(uiLoop, uiOpcode, sInitial, sBound))
	{
		return true;
	}
	psInitial = sInitial.IsTemporary() ? mcDefinitions.Instruction(sInitial) : NULL;
	psBound = sBound.IsTemporary() ? mcDefinitions.Instruction(sBound) : NULL;
	if (psInitial && (psInitial->uiOpcode == IRO_SExt))
	{
		sNarrow = mpcFunction->TypeOf(psInitial->asArguments[0]);
	}
	else if (psBound && (psBound->uiOpcode == IRO_SExt))
	{
		sNarrow = mpcFunction->TypeOf(psBound->asArguments[0]);
	}
	else
	{
		return false;
	}

	if (psInitial && (psInitial->uiOpcode == IRO_SExt) && mpcFunction->TypeOf(psInitial->asArguments[0]).Equals(sNarrow))
	{
		sNarrowInitial = psInitial->asArguments[0];
	}
	else if (sInitial.IsConstant())
	{
		sNarrowInitial = mpcFunction->AddInteger(sNarrow, mpcFunction->GetSigned(sInitial));
	}
	else
	{
		return false;
	}
	if (psBound && (psBound->uiOpcode == IRO_SExt) && mpcFunction->TypeOf(psBound->asArguments[0]).Equals(sNarrow))
	{
		sNarrowBound = psBound->asArguments[0];
	}
	else if (sBound.IsConstant())
	{
		sNarrowBound = mpcFunction->AddInteger(sNarrow, mpcFunction->GetSigned(sBound));
	}
	else
	{
		return false;
	}
	return NarrowGuardedAs(uiLoop, uiOpcode, sNarrowInitial, sNarrowBound);
}


//////////////////////////////////////////////////////////////////////////
//
// 32 bit loop counters that stay in 16 bits count in 16 bits
// (NarrowCounter, for each block of each loop that leaves or goes round on
// a compare).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowCounters(void)
{
	size		uiLoop;
	size		uiBlock;
	bool		bChanged;

	ComputeLoops();
	bChanged = false;
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		for (uiBlock = 0; uiBlock < mcLoops.NumBlocks(uiLoop); uiBlock++)
		{
			if (NarrowCounter(uiLoop, mcLoops.GetBlock(uiLoop, uiBlock)))
			{
				//The loops and definitions are found again for the next.
				ComputeLoops();
				bChanged = true;
				break;
			}
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// pcBlock (in the loop) branches round the loop or out of it on a signed
// compare of a counter c (a 32 bit phi of the loop's head from c0 outside
// and c + 1 or c - 1 from inside, or that c + 1 or c - 1) with a bound N
// the loop doesn't change, going round while c moves towards N (up while
// < or <=, down while > or >=).  When c0 and N are known to fit 16 bits
// (SignedRange), with one to spare past c0 in the way it goes (unless the
// loop is only entered while c0 is towards N: NarrowLoopGuarded) and past
// N for <= and >=, c can't leave 16 bits: it stops at N (one past it for
// <= and >=), or one step past c0 when it starts past N.  So it counts in
// a 16 bit phi; the compare is of 16 bits, and other uses get it sign
// extended.  The 32 bit phi and step go (RemoveDeadCode).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::NarrowCounter(size uiLoop, CIRBlock* pcBlock)
{
	CIRBlock*		pcHead;
	CIRBlock*		pcOutside;
	CIRBlock*		pcInside;
	CIRBlock*		pcStepBlock;
	SIRInstruction*	psCompare;
	SIRLoopCounter	sLoopCounter;
	SIRPhi*			psCounter;
	SIRPhi*			psNarrow;
	SIRReference	sCompared;
	SIRReference	sBound;
	SIRReference	sCounter;
	SIRReference	sStepped;
	SIRReference	sInitial;
	SIRReference	sNarrowCounter;
	SIRReference	sNarrowStepped;
	SIRReference	sNarrowInitial;
	SIRReference	sNarrowBound;
	SIRReference	sWideCounter;
	SIRReference	sWideStepped;
	SIRType			sWide;
	SIRType			sNarrowType;
	uint16			uiOpcode;
	size			uiPhi;
	size			uiOutside;
	size			uiStep;
	int64			lliStep;
	int64			lliMin;
	int64			lliMax;
	int64			lliBoundMin;
	int64			lliBoundMax;
	bool			bRoundOnTrue;
	bool			bUp;
	bool			bBoundFirst;
	bool			bInclusive;
	bool			bStartSpare;
	bool			bRewritten;
	CArrayIRExitArguments	asExits;		//The phi arguments after the loop that take c +/- 1 leaving through the test.
	SIRReference	sExitValue;
	size			uiExit;
	SIRReference	sCounterBound;
	SIRInstruction*	psBound;

	if ((pcBlock->msJump.uiKind != IRJ_Branch) || !pcBlock->msJump.sArgument.IsTemporary() || (mcDefinitions.NumUses(pcBlock->msJump.sArgument) != 1))
	{
		return false;
	}
	psCompare = mcDefinitions.Instruction(pcBlock->msJump.sArgument);
	if ((psCompare == NULL) || (mcDefinitions.Block(psCompare->sResult) != pcBlock) || !psCompare->sType.Is(IRTK_I32))
	{
		return false;
	}
	uiOpcode = psCompare->uiOpcode;
	if (!IsIRSignedComparison(uiOpcode))
	{
		return false;
	}
	bRoundOnTrue = mcLoops.Contains(uiLoop, pcBlock->msJump.pcThen);
	if (bRoundOnTrue == mcLoops.Contains(uiLoop, pcBlock->msJump.pcElse))
	{
		return false;
	}

	//Which operand is the counter: the bound is the one the loop doesn't change.
	bBoundFirst = mcLoops.IsInvariant(uiLoop, psCompare->asArguments[0], &mcDefinitions);
	sCompared = psCompare->asArguments[bBoundFirst ? 1 : 0];
	sBound = psCompare->asArguments[bBoundFirst ? 0 : 1];
	if (!sCompared.IsTemporary() || !mcLoops.IsInvariant(uiLoop, sBound, &mcDefinitions) || mcLoops.IsInvariant(uiLoop, sCompared, &mcDefinitions))
	{
		return false;
	}

	//c: a phi of the head with c0 from outside and c +/- 1 from inside (sStepped); the compare is of c or c +/- 1.
	pcHead = mcLoops.GetBlock(uiLoop, 0);
	psCounter = NULL;
	for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
	{
		psCounter = pcHead->masPhis.Get(uiPhi);
		if (psCounter->sType.Is(IRTK_I32) && MatchCounter(uiLoop, psCounter, IR_COUNTER_SUB | IR_COUNTER_INSIDE, &sLoopCounter) &&
			(sCompared.Equals(psCounter->sResult) || sCompared.Equals(sLoopCounter.sNext)))
		{
			break;
		}
		psCounter = NULL;
	}
	if (psCounter == NULL)
	{
		return false;
	}
	sCounter = psCounter->sResult;
	sStepped = sLoopCounter.sNext;
	uiOutside = sLoopCounter.uiOutside;
	lliStep = sLoopCounter.lliStep;
	if ((lliStep != 1) && (lliStep != -1))
	{
		return false;
	}

	//Going round while it moves towards the bound: up while c < N or c <= N, down while c > N or c >= N.
	if (!bRoundOnTrue)
	{
		uiOpcode = (uiOpcode == IRO_SLt) ? IRO_SGe : ((uiOpcode == IRO_SLe) ? IRO_SGt : ((uiOpcode == IRO_SGt) ? IRO_SLe : IRO_SLt));
	}
	if (bBoundFirst)
	{
		uiOpcode = (uiOpcode == IRO_SLt) ? IRO_SGt : ((uiOpcode == IRO_SLe) ? IRO_SGe : ((uiOpcode == IRO_SGt) ? IRO_SLt : IRO_SLe));
	}
	bUp = (uiOpcode == IRO_SLt) || (uiOpcode == IRO_SLe);
	if (bUp != (lliStep == 1))
	{
		return false;
	}

	//A test of c +/- 1 that the phi only otherwise uses is a test of c, so the last step (past N) is never seen:
	//c + 1 <= N is c < N and c + 1 < N + 1 is c < N (down, c - 1 >= N is c > N and c - 1 > N - 1 is c > N).  Leaving
	//through the test, c +/- 1 is N +/- 1 when the loop is only entered with c0 <= N (or >=), so a phi after the loop
	//that takes it on that way out takes N +/- 1 worked out before the loop instead.
	bRewritten = false;
	asExits.Init();
	if (!sCompared.Equals(sCounter) && SteppedExitUses(uiLoop, pcBlock, psCounter, psCompare, sStepped, &asExits))
	{
		sCounterBound = IRNone();
		if ((uiOpcode == IRO_SLe) || (uiOpcode == IRO_SGe))
		{
			sCounterBound = sBound;
		}
		else
		{
			psBound = sBound.IsTemporary() ? mcDefinitions.Instruction(sBound) : NULL;
			if (psBound && psBound->asArguments[1].IsConstant() &&
				(((psBound->uiOpcode == IRO_Add) && (mpcFunction->GetSigned(psBound->asArguments[1]) == lliStep)) ||
				 ((psBound->uiOpcode == IRO_Sub) && (mpcFunction->GetSigned(psBound->asArguments[1]) == -lliStep))))
			{
				sCounterBound = psBound->asArguments[0];
			}
		}
		if (!sCounterBound.IsNone() && ((asExits.NumElements() == 0) || NarrowLoopGuarded(uiLoop, bUp ? IRO_SLe : IRO_SGe, psCounter->asArguments.Get(uiOutside)->sValue, sCounterBound)))
		{
			sBound = sCounterBound;
			sCompared = sCounter;
			uiOpcode = bUp ? IRO_SLt : IRO_SGt;
			bRewritten = true;
		}
	}
	if (!bRewritten)
	{
		asExits.Kill();
		asExits.Init();
	}

	//c0 and N fit 16 bits.  The values seen stay between them, with one to spare for a step past one: past N for <=
	//and >= (< and > stop at N), and past c0 (when it starts past N) in the way it goes, when the stepped value is seen
	//past the test (tested, or used after it) and the loop isn't only entered towards N.
	sInitial = psCounter->asArguments.Get(uiOutside)->sValue;
	if (!SignedRange(sInitial, &lliMin, &lliMax) || !SignedRange(sBound, &lliBoundMin, &lliBoundMax))
	{
		asExits.Kill();
		return false;
	}
	bInclusive =(uiOpcode == IRO_SLe) || (uiOpcode == IRO_SGe);
	bStartSpare = (!sCompared.Equals(sCounter) || (mcDefinitions.NumUses(sStepped) > (bRewritten ? 2u + asExits.NumElements() : 1u))) && !NarrowLoopGuarded(uiLoop, uiOpcode, sInitial, sBound);
	if ((lliMin < -32768) || (lliMax > 32767) || (lliBoundMin < -32768) || (lliBoundMax > 32767) ||
		(bUp && ((bStartSpare && (lliMax > 32766)) || (bInclusive && (lliBoundMax > 32766)))) ||
		(!bUp && ((bStartSpare && (lliMin < -32767)) || (bInclusive && (lliBoundMin < -32767)))))
	{
		asExits.Kill();
		return false;
	}

	//The 16 bit counter: c0 and N narrowed outside, c' +/- 1 after the step.
	sWide = IRType(IRTK_I32);
	sNarrowType = IRType(IRTK_I16);
	pcOutside = psCounter->asArguments.Get(uiOutside)->pcBlock;
	sNarrowInitial = sInitial.IsConstant() ? mpcFunction->AddInteger(sNarrowType, mpcFunction->GetSigned(sInitial)) : mpcFunction->Add(pcOutside, IRO_Trunc, sNarrowType, sInitial, IRNone());
	sNarrowBound = sBound.IsConstant() ? mpcFunction->AddInteger(sNarrowType, mpcFunction->GetSigned(sBound)) : mpcFunction->Add(pcOutside, IRO_Trunc, sNarrowType, sBound, IRNone());
	if (asExits.NumElements() > 0)
	{
		//N +/- 1, what c +/- 1 is leaving through the test.
		sExitValue = mpcFunction->Add(pcOutside, IRO_Add, sWide, sBound, mpcFunction->AddInteger(sWide, lliStep));
		for (uiExit = 0; uiExit < asExits.NumElements(); uiExit++)
		{
			asExits.GetPtr(uiExit)->sValue = sExitValue;
		}
	}
	asExits.Kill();
	sNarrowCounter = mpcFunction->AddTemporary(sNarrowType);
	//AddPhi may move the head's phis, psCounter too.
	pcInside = psCounter->asArguments.Get(1 - uiOutside)->pcBlock;
	psCounter = NULL;
	psNarrow = pcHead->AddPhi(sNarrowType, sNarrowCounter);
	pcStepBlock = mcDefinitions.Block(sStepped);
	for (uiStep = 0; (uiStep < pcStepBlock->masInstructions.NumElements()) && !pcStepBlock->masInstructions.Get(uiStep)->sResult.Equals(sStepped); uiStep++)
	{
	}
	sNarrowStepped = InsertInstruction(pcStepBlock, uiStep + 1, IRO_Add, sNarrowType, sNarrowCounter, mpcFunction->AddInteger(sNarrowType, lliStep));
	sWideStepped = InsertInstruction(pcStepBlock, uiStep + 2, IRO_SExt, sWide, sNarrowStepped, IRNone());
	pcHead->AddPhiArgument(psNarrow, pcOutside, sNarrowInitial);
	pcHead->AddPhiArgument(psNarrow, pcInside, sNarrowStepped);
	sWideCounter = InsertInstruction(pcHead, 0, IRO_SExt, sWide, sNarrowCounter, IRNone());

	//The compare is of the 16 bit values.
	FindDefinitions();
	psCompare = mcDefinitions.Instruction(pcBlock->msJump.sArgument);
	psCompare->sType = sNarrowType;
	if (bRewritten)
	{
		//c < N (or c > N) going round, its inverse leaving.
		psCompare->uiOpcode = bRoundOnTrue ? uiOpcode : IRInvertedComparison(uiOpcode);
		psCompare->asArguments[0] = sNarrowCounter;
		psCompare->asArguments[1] = sNarrowBound;
	}
	else
	{
		psCompare->asArguments[bBoundFirst ? 0 : 1] = sNarrowBound;
		psCompare->asArguments[bBoundFirst ? 1 : 0] = sCompared.Equals(sCounter) ? sNarrowCounter : sNarrowStepped;
	}

	//Everything else that used c or c +/- 1 has them sign extended.
	ClearReplacements();
	Replace(sCounter, sWideCounter);
	Replace(sStepped, sWideStepped);
	ApplyReplacements();
	FindDefinitions();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if loop uiLoop is only entered when sInitial uiOpcode sBound holds:
// its head, or a block up IR_GUARD_DEPTH immediate dominators from it, is
// reached (but from inside the loop) only from a branch on that compare
// (either way round: b > a is a < b), the way it holds.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::LoopGuarded(size uiLoop, uint16 uiOpcode, SIRReference sInitial, SIRReference sBound)
{
	CIRBlock*		pcBlock;
	CIRBlock*		pcFrom;
	CIRBlock*		pcPredecessor;
	SIRInstruction*	psCompare;
	uint16			uiCompare;
	SIRReference	sA;
	SIRReference	sB;
	size			ui;
	int				iDepth;

	pcBlock = mcLoops.GetBlock(uiLoop, 0);
	for (iDepth = 0; pcBlock && (iDepth < IR_GUARD_DEPTH); iDepth++)
	{
		pcFrom = NULL;
		for (ui = 0; ui < pcBlock->mapcPredecessors.NumElements(); ui++)
		{
			pcPredecessor = pcBlock->mapcPredecessors.GetPtr(ui);
			if (!mcLoops.Contains(uiLoop, pcPredecessor))
			{
				pcFrom = (pcFrom == NULL) ? pcPredecessor : NULL;
				if (pcFrom == NULL)
				{
					break;
				}
			}
		}
		psCompare = (pcFrom && (pcFrom->msJump.uiKind == IRJ_Branch) && (pcFrom->msJump.pcThen != pcFrom->msJump.pcElse)) ? mcDefinitions.Instruction(pcFrom->msJump.sArgument) : NULL;
		if (psCompare && IsIRIntegerComparison(psCompare->uiOpcode))
		{
			uiCompare = (pcFrom->msJump.pcThen == pcBlock) ? psCompare->uiOpcode : IRInvertedComparison(psCompare->uiOpcode);
			sA = psCompare->asArguments[0];
			sB = psCompare->asArguments[1];
			if (IsIRSwappedComparison(uiCompare))
			{
				uiCompare = IRSwappedComparison(uiCompare);
				sA = psCompare->asArguments[1];
				sB = psCompare->asArguments[0];
			}
			if ((uiCompare == uiOpcode) && sA.Equals(sInitial) && sB.Equals(sBound))
			{
				return true;
			}
		}
		pcBlock = mcDominators.GetImmediate(pcBlock);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Linear test replacement: a loop with two counters going up by 1
// together, c from a constant c0 and v from v0 (both phis of its head with
// one way in and one way round), that only uses c for its test c + 1 < N
// (ult or slt, N not changed by the loop) tests v + 1 != v0 - c0 + N
// instead, and c goes (RemoveDeadCode).  Each time round c + 1 <= N, as
// the loop is only entered when c0 < N (LoopGuarded) and leaves when
// c + 1 reaches N, so < is !=; and v - c is v0 - c0 throughout.  The new
// bound is moved out of the loop by HoistInvariants.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceLoopCounters(void)
{
	size			uiLoop;
	size			uiBlock;
	size			uiPhi;
	size			uiOutside;
	CIRBlock*		pcBlock;
	CIRBlock*		pcHead;
	SIRInstruction*	psCompare;
	SIRInstruction*	psNew;
	SIRPhi*			psCounter;
	SIRPhi*			psOther;
	SIRLoopCounter	sLoopCounter;
	SIRReference	sStepped;
	SIRReference	sBound;
	SIRReference	sOtherStepped;
	SIRReference	sOtherInitial;
	SIRReference	sCounterInitial;
	SIRReference	sDifference;
	SIRReference	sNewBound;
	SIRReference	sTest;
	uint16			uiOpcode;
	bool			bRoundOnTrue;
	bool			bChanged;

	ComputeLoops();
	bChanged = false;
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		pcHead = mcLoops.GetBlock(uiLoop, 0);
		if (pcHead->mapcPredecessors.NumElements() != 2)
		{
			continue;
		}
		for (uiBlock = 0; uiBlock < mcLoops.NumBlocks(uiLoop); uiBlock++)
		{
			pcBlock = mcLoops.GetBlock(uiLoop, uiBlock);
			if ((pcBlock->msJump.uiKind != IRJ_Branch) || !pcBlock->msJump.sArgument.IsTemporary() || (mcDefinitions.NumUses(pcBlock->msJump.sArgument) != 1))
			{
				continue;
			}
			psCompare = mcDefinitions.Instruction(pcBlock->msJump.sArgument);
			if ((psCompare == NULL) || ((psCompare->uiOpcode != IRO_ULt) && (psCompare->uiOpcode != IRO_SLt)) || (mcDefinitions.Block(psCompare->sResult) != pcBlock))
			{
				continue;
			}
			bRoundOnTrue = mcLoops.Contains(uiLoop, pcBlock->msJump.pcThen) && !mcLoops.Contains(uiLoop, pcBlock->msJump.pcElse);
			if (!bRoundOnTrue)
			{
				continue;
			}
			uiOpcode = psCompare->uiOpcode;
			sStepped = psCompare->asArguments[0];
			sBound = psCompare->asArguments[1];
			if (!sStepped.IsTemporary() || !mcLoops.IsInvariant(uiLoop, sBound, &mcDefinitions) || (mcDefinitions.NumUses(sStepped) != 2))
			{
				continue;
			}

			//c: a phi of the head, c0 a constant from outside and c + 1 from inside; c's only use is c + 1.
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
			if ((psCounter == NULL) || (mcDefinitions.NumUses(psCounter->sResult) != 1))
			{
				continue;
			}
			uiOutside = sLoopCounter.uiOutside;
			sCounterInitial = sLoopCounter.sInit;
			if (!sCounterInitial.IsConstant() || !LoopGuarded(uiLoop, uiOpcode, sCounterInitial, sBound))
			{
				continue;
			}

			//v: another phi of the head of the same type, v + 1 worked out in this block going round.
			psOther = NULL;
			for (uiPhi = 0; uiPhi < pcHead->masPhis.NumElements(); uiPhi++)
			{
				psOther = pcHead->masPhis.Get(uiPhi);
				if ((psOther != psCounter) && psOther->sType.Equals(psCounter->sType) && (psOther->asArguments.NumElements() == 2) &&
					!mcLoops.Contains(uiLoop, psOther->asArguments.Get(uiOutside)->pcBlock) &&
					(psOther->asArguments.Get(uiOutside)->pcBlock == psCounter->asArguments.Get(uiOutside)->pcBlock))
				{
					sOtherStepped = psOther->asArguments.Get(1 - uiOutside)->sValue;
					if (StepsByOne(sOtherStepped, psOther->sResult) && (mcDefinitions.Block(sOtherStepped) == pcBlock))
					{
						break;
					}
				}
				psOther = NULL;
			}
			if (psOther == NULL)
			{
				continue;
			}
			sOtherInitial = psOther->asArguments.Get(uiOutside)->sValue;

			//v + 1 != v0 + (N - c0).
			sDifference = mpcFunction->AddTemporary(psCounter->sType);
			sNewBound = mpcFunction->AddTemporary(psCounter->sType);
			sTest = mpcFunction->AddTemporary(IRType(IRTK_I1));
			psNew = pcBlock->masInstructions.Add();
			memset(psNew, 0, sizeof(SIRInstruction));
			psNew->uiOpcode = IRO_Sub;
			psNew->uiSource = pcBlock->msJump.uiSource;
			psNew->sType = psCounter->sType;
			psNew->sResult = sDifference;
			psNew->asArguments[0] = sBound;
			psNew->asArguments[1] = sCounterInitial;
			psNew = pcBlock->masInstructions.Add();
			memset(psNew, 0, sizeof(SIRInstruction));
			psNew->uiOpcode = IRO_Add;
			psNew->uiSource = pcBlock->msJump.uiSource;
			psNew->sType = psCounter->sType;
			psNew->sResult = sNewBound;
			psNew->asArguments[0] = sOtherInitial;
			psNew->asArguments[1] = sDifference;
			psNew = pcBlock->masInstructions.Add();
			memset(psNew, 0, sizeof(SIRInstruction));
			psNew->uiOpcode = IRO_Ne;
			psNew->uiSource = pcBlock->msJump.uiSource;
			psNew->sType = psCounter->sType;
			psNew->sResult = sTest;
			psNew->asArguments[0] = sOtherStepped;
			psNew->asArguments[1] = sNewBound;
			pcBlock->msJump.sArgument = sTest;
			bChanged = true;
			FindDefinitions();
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds sValue to the web (ReduceMultiplies) if it isn't in it: false if it
// can't be (another type, or too many values).
//
//////////////////////////////////////////////////////////////////////////
static bool IROptimiserAddToWeb(SIRReference* asValues, size* puiValues, SIRReference sValue, SIRType sType, CIRFunction* pcFunction)
{
	size	ui;

	if (!(sValue.IsTemporary() && pcFunction->GetTemporaryType(sValue).Equals(sType)) && !(sValue.IsConstant() && pcFunction->GetConstant(sValue)->sType.Equals(sType)))
	{
		return false;
	}
	for (ui = 0; ui < *puiValues; ui++)
	{
		if (asValues[ui].Equals(sValue))
		{
			return true;
		}
	}
	if (*puiValues == IR_REDUCE_VALUES)
	{
		return false;
	}
	asValues[(*puiValues)++] = sValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// In a loop with one way in, a mul of i by a value w the loop doesn't
// change, i one of a web of values the loop works out from each other
// alone (each a phi of values of the web or from before the loop, or a
// value of the web plus a constant c: a counter i0, i + 1 going round, or
// one stepped on some ways round only), becomes a web of its own: v * w
// for each value from before the loop worked out in the preheader, a phi
// for each phi, + w * c for each add.  Integers wrap the same way either
// way, so it is exact (a 16 bit int's y * width, say).  The multiply each
// time round becomes adds.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReduceMultiplies(void)
{
	size			uiLoop;
	size			uiBlock;
	size			uiIndex;
	size			ui;
	size			uj;
	size			uiValues;
	size			uiArgument;
	CIRBlock*		pcBlock;
	CIRBlock*		pcPreheader;
	CIRBlock*		pcValue;
	SIRInstruction*	psMul;
	SIRInstruction*	psAdd;
	SIRInstruction	sNew;
	SIRPhi*			psPhi;
	SIRPhi*			psProductPhi;
	SIRReference	asValues[IR_REDUCE_VALUES];
	SIRReference	asProducts[IR_REDUCE_VALUES];
	SIRReference	sCounter;
	SIRReference	sFactor;
	SIRReference	sValue;
	SIRReference	sResult;
	SIRReference	sProduct;
	SIRType			sType;
	bool			bWeb;
	int				iArgument;
	int32			iIndex;

	ComputeLoops();
	for (uiLoop = 0; uiLoop < mcLoops.NumLoops(); uiLoop++)
	{
		if (mcLoops.GetLoop(uiLoop)->pcEntering == NULL)
		{
			continue;
		}
		for (uiBlock = 0; uiBlock < mcLoops.NumBlocks(uiLoop); uiBlock++)
		{
			pcBlock = mcLoops.GetBlock(uiLoop, uiBlock);
			for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
			{
				psMul = pcBlock->masInstructions.Get(uiIndex);
				if ((psMul->uiOpcode != IRO_Mul) || !psMul->sResult.IsTemporary() || !psMul->sType.IsInteger())
				{
					continue;
				}
				sType = psMul->sType;
				sResult = psMul->sResult;		//psMul moves when instructions are put in its block.

				//mul i, w (either way round): i in the loop, w not changed by it.
				for (iArgument = 0; iArgument < 2; iArgument++)
				{
					sCounter = psMul->asArguments[iArgument];
					sFactor = psMul->asArguments[1 - iArgument];
					if (sCounter.IsTemporary() && mcDefinitions.Block(sCounter) && mcLoops.Contains(uiLoop, mcDefinitions.Block(sCounter)) &&
						mcLoops.IsInvariant(uiLoop, sFactor, &mcDefinitions))
					{
						break;
					}
				}
				if (iArgument == 2)
				{
					continue;
				}

				//The web: phis and adds of a constant in the loop, reaching values from before it.
				asValues[0] = sCounter;
				uiValues = 1;
				bWeb = true;
				for (ui = 0; bWeb && (ui < uiValues); ui++)
				{
					sValue = asValues[ui];
					pcValue = sValue.IsTemporary() ? mcDefinitions.Block(sValue) : NULL;
					if ((pcValue == NULL) || !mcLoops.Contains(uiLoop, pcValue))
					{
						//From before the loop: its product is worked out in the preheader.
						continue;
					}
					psPhi = mcDefinitions.Phi(sValue);
					psAdd = mcDefinitions.Instruction(sValue);
					if (psPhi)
					{
						for (uiArgument = 0; bWeb && (uiArgument < psPhi->asArguments.NumElements()); uiArgument++)
						{
							bWeb = IROptimiserAddToWeb(asValues, &uiValues, psPhi->asArguments.Get(uiArgument)->sValue, sType, mpcFunction);
						}
					}
					else if (psAdd && (psAdd->uiOpcode == IRO_Add) && psAdd->asArguments[1].IsConstant() && psAdd->sType.Equals(sType))
					{
						bWeb = IROptimiserAddToWeb(asValues, &uiValues, psAdd->asArguments[0], sType, mpcFunction);
					}
					else
					{
						bWeb = false;
					}
				}
				if (!bWeb)
				{
					continue;
				}

				//A product for each value: made first (phis' arguments may come later), then filled in.
				pcPreheader = mcLoops.GetLoop(uiLoop)->pcPreheader;
				if (pcPreheader == NULL)
				{
					pcPreheader = AddPreheader(uiLoop);
					FindDefinitions();
				}
				for (ui = 0; ui < uiValues; ui++)
				{
					pcValue = asValues[ui].IsTemporary() ? mcDefinitions.Block(asValues[ui]) : NULL;
					if ((pcValue == NULL) || !mcLoops.Contains(uiLoop, pcValue))
					{
						asProducts[ui] = mpcFunction->Add(pcPreheader, IRO_Mul, sType, asValues[ui], sFactor);
					}
					else
					{
						asProducts[ui] = mpcFunction->AddTemporary(sType);
					}
				}
				for (ui = 0; ui < uiValues; ui++)
				{
					pcValue = asValues[ui].IsTemporary() ? mcDefinitions.Block(asValues[ui]) : NULL;
					if ((pcValue == NULL) || !mcLoops.Contains(uiLoop, pcValue))
					{
						continue;
					}
					if (mcDefinitions.Phi(asValues[ui]))
					{
						//The block's phis move when one is added: found again by their results.
						pcValue->AddPhi(sType, asProducts[ui]);
						psProductPhi = pcValue->FindPhi(asProducts[ui]);
						psPhi = pcValue->FindPhi(asValues[ui]);
						for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
						{
							sValue = psPhi->asArguments.Get(uiArgument)->sValue;
							for (uj = 0; !asValues[uj].Equals(sValue); uj++)
							{
							}
							pcValue->AddPhiArgument(psProductPhi, psPhi->asArguments.Get(uiArgument)->pcBlock, asProducts[uj]);
						}
						FindDefinitions();
					}
					else
					{
						//p + w * c just after v + c.
						psAdd = mcDefinitions.Instruction(asValues[ui]);
						for (uj = 0; !asValues[uj].Equals(psAdd->asArguments[0]); uj++)
						{
						}
						sProduct = (mpcFunction->GetSigned(psAdd->asArguments[1]) == 1) ? sFactor :
							mpcFunction->Add(pcPreheader, IRO_Mul, sType, sFactor, psAdd->asArguments[1]);
						memset(&sNew, 0, sizeof(SIRInstruction));
						sNew.uiOpcode = IRO_Add;
						sNew.uiSource = psAdd->uiSource;
						sNew.sType = sType;
						sNew.sResult = asProducts[ui];
						sNew.asArguments[0] = asProducts[uj];
						sNew.asArguments[1] = sProduct;
						iIndex = mcDefinitions.Index(asValues[ui]);
						pcValue->masInstructions.InsertAt(&sNew, (size)iIndex + 1);
						FindDefinitions();
					}
				}
				ClearReplacements();
				Replace(sResult, asProducts[0]);
				ApplyReplacements();
				FindDefinitions();
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The counter sCounter if sValue is it (bExtended false) or it sign
// extended (bExtended true), or zero extended when bZeroExtends, for
// ReduceStrength; IRNone otherwise.  *puiExtend is the extension (IRO_SExt
// or IRO_ZExt), IRO_Copy when there is none.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::ExtendedCounter(SIRReference sValue, SIRReference sCounter, bool bExtended, bool bZeroExtends, uint16* puiExtend)
{
	SIRInstruction*	psExtend;

	*puiExtend = IRO_Copy;
	if (!bExtended)
	{
		return sValue.Equals(sCounter) ? sCounter : IRNone();
	}
	psExtend = sValue.IsTemporary() ? mcDefinitions.Instruction(sValue) : NULL;
	if ((psExtend == NULL) || ((psExtend->uiOpcode != IRO_SExt) && ((psExtend->uiOpcode != IRO_ZExt) || !bZeroExtends)) || !psExtend->sType.Is(IRTK_I32) || !psExtend->asArguments[0].Equals(sCounter))
	{
		return IRNone();
	}
	*puiExtend = psExtend->uiOpcode;
	return sCounter;
}


//////////////////////////////////////////////////////////////////////////
//
// The most sValue can be when pcBlock's branch goes to pcStay (and the
// branch's other way is elsewhere): when the branch is on sValue ult b,
// b - 1 for a constant b and the type's highest - 1 for any other; when it
// is on sValue ule b, a constant b.  *psBound and *puiCompare are b and
// the compare (ult or ule).  False when the branch doesn't hold sValue
// below something.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::UnsignedMaximum(CIRBlock* pcBlock, CIRBlock* pcStay, SIRReference sValue, uint64 ulliMask, uint64* pulliMax, SIRReference* psBound, uint16* puiCompare)
{
	SIRInstruction*	psCompare;
	uint16			uiCompare;
	SIRReference	sA;
	SIRReference	sB;
	uint64			ulliBound;

	if ((pcBlock->msJump.uiKind != IRJ_Branch) || (pcBlock->msJump.pcThen == pcBlock->msJump.pcElse) || !pcBlock->msJump.sArgument.IsTemporary())
	{
		return false;
	}
	psCompare = mcDefinitions.Instruction(pcBlock->msJump.sArgument);
	if ((psCompare == NULL) || !IsIRIntegerComparison(psCompare->uiOpcode))
	{
		return false;
	}
	if (pcBlock->msJump.pcThen == pcStay)
	{
		uiCompare = psCompare->uiOpcode;
	}
	else if (pcBlock->msJump.pcElse == pcStay)
	{
		uiCompare = IRInvertedComparison(psCompare->uiOpcode);
	}
	else
	{
		return false;
	}
	sA = psCompare->asArguments[0];
	sB = psCompare->asArguments[1];
	if (!sA.Equals(sValue))
	{
		//b ugt sValue is sValue ult b and b uge sValue is sValue ule b.
		if (!sB.Equals(sValue))
		{
			return false;
		}
		uiCompare = IRSwappedComparison(uiCompare);
		sB = sA;
	}

	ulliBound = sB.IsConstant() ? ((uint64)mpcFunction->GetSigned(sB) & ulliMask) : 0;
	if (uiCompare == IRO_ULt)
	{
		if (!sB.IsConstant())
		{
			*pulliMax = ulliMask - 1;
		}
		else if (ulliBound != 0)
		{
			*pulliMax = ulliBound - 1;
		}
		else
		{
			return false;
		}
	}
	else if ((uiCompare == IRO_ULe) && sB.IsConstant())
	{
		*pulliMax = ulliBound;
	}
	else
	{
		return false;
	}
	*psBound = sB;
	*puiCompare = uiCompare;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if counter sCounter of loop uiLoop (a phi of its head: sInit coming
// in, sNext = sCounter + lliStep going round from pcLatch) never wraps
// while the loop goes round, so its zero extension steps with it.  Either
// the head only goes on into the loop while sCounter is at most a maximum
// (UnsignedMaximum), or the latch only goes round while sNext is and the
// loop is only entered when sInit is too (a constant, or LoopGuarded); so
// each value stepped from is at most the maximum, and the maximum +
// lliStep must still fit.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::CounterUnwrapped(size uiLoop, CIRBlock* pcLatch, SIRReference sCounter, SIRReference sInit, SIRReference sNext, int64 lliStep, uint64 ulliMask)
{
	CIRBlock*		pcHead;
	CIRBlock*		pcInside;
	SIRReference	sBound;
	uint64			ulliMax;
	uint16			uiCompare;

	if (lliStep <= 0)
	{
		return false;
	}

	//The head's test on the counter, going on in when it holds.
	pcHead = mcLoops.GetLoop(uiLoop)->pcHead;
	if ((pcHead->msJump.uiKind == IRJ_Branch) && (mcLoops.Contains(uiLoop, pcHead->msJump.pcThen) != mcLoops.Contains(uiLoop, pcHead->msJump.pcElse)))
	{
		pcInside = mcLoops.Contains(uiLoop, pcHead->msJump.pcThen) ? pcHead->msJump.pcThen : pcHead->msJump.pcElse;
		if (UnsignedMaximum(pcHead, pcInside, sCounter, ulliMask, &ulliMax, &sBound, &uiCompare))
		{
			return ulliMax + (uint64)lliStep <= ulliMask;
		}
	}

	//The latch's test on the stepped counter, going round when it holds.
	if ((pcLatch->msJump.uiKind != IRJ_Branch) || mcLoops.Contains(uiLoop, (pcLatch->msJump.pcThen == pcHead) ? pcLatch->msJump.pcElse : pcLatch->msJump.pcThen) ||
		!UnsignedMaximum(pcLatch, pcHead, sNext, ulliMask, &ulliMax, &sBound, &uiCompare) || (ulliMax + (uint64)lliStep > ulliMask))
	{
		return false;
	}
	if (sInit.IsConstant())
	{
		return ((uint64)mpcFunction->GetSigned(sInit) & ulliMask) <= ulliMax;
	}
	return LoopGuarded(uiLoop, uiCompare, sInit, sBound);
}


//////////////////////////////////////////////////////////////////////////
//
// Strength reduction: in a loop with one block going round to its head and
// a counter i (a phi of init, then i + c or i - c), an address p + i (when
// the target's loads and stores don't add it for nothing),
// p + (i << s) or p + i * k with p not changed by the loop becomes a
// pointer of its own: p + init * k before the loop, stepped by c * k going
// round.  The shift or multiply and the add each time round become one
// add.  A counter narrower than 32 bits is the target's int (msLibraryInt,
// the W65C816's 16 bits) sign extended in the index: an int stepped past
// its end is undefined, so its extension steps with it.  An unsigned one is
// zero extended, and only steps with it when the loop's tests keep it from
// wrapping (CounterUnwrapped).
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
	SIRReference			sIndex;
	SIRType					sType;
	int64					lliStep;
	int64					lliScale;
	uint64					ulliMask;
	uint16					uiExtend;
	bool					bChanged;
	bool					bExtended;
	bool					bZeroExtends;

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
			bExtended = !psPhi->sType.Is(IRTK_I32);
			if ((bExtended && !psPhi->sType.Equals(msLibraryInt)) || (psPhi->asArguments.NumElements() != 2))
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
			sType = IRType(IRTK_I32);
			ulliMask = bExtended ? (uint64)IRUnsignedMax(psPhi->sType.Bits()) : 0;
			bZeroExtends = bExtended && CounterUnwrapped(uiLoop, pcLatch, sCounter, sInit, sNext, lliStep, ulliMask);

			//Addresses p + (i << s) or p + i * k in the loop (i sign or zero extended when narrower).
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
					sIndex = ExtendedCounter(psAddress->asArguments[1], sCounter, bExtended, bZeroExtends, &uiExtend);
					if (sIndex.Equals(sCounter))
					{
						//p + i, unless loads and stores add it for nothing.
						if (mbIndexedAddresses)
						{
							continue;
						}
						lliScale = 1;
					}
					else if ((psScaled == NULL) || (psScaled->uiOpcode == IRO_SExt) || (psScaled->uiOpcode == IRO_ZExt) || !ExtendedCounter(psScaled->asArguments[0], sCounter, bExtended, bZeroExtends, &uiExtend).Equals(sCounter) || !psScaled->asArguments[1].IsConstant())
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
						sStart = mpcFunction->AddInteger(sType, ((uiExtend == IRO_ZExt) ? (int64)((uint64)mpcFunction->GetSigned(sInit) & ulliMask) : mpcFunction->GetSigned(sInit)) * lliScale);
					}
					else
					{
						sStart = bExtended ? mpcFunction->Add(pcEntering, uiExtend, sType, sInit) : sInit;
						sStart = (lliScale == 1) ? sStart : mpcFunction->Add(pcEntering, IRO_Mul, sType, sStart, mpcFunction->AddInteger(sType, lliScale));
					}
					if (!sStart.IsConstant() || (mpcFunction->GetSigned(sStart) != 0))
					{
						sStart = mpcFunction->Add(pcEntering, IRO_PtrAdd, IRType(IRTK_Pointer), sBase, sStart);
					}
					else
					{
						sStart = sBase;
					}
					sPointer = mpcFunction->AddTemporary(IRType(IRTK_Pointer));
					sStepped = mpcFunction->Add(pcLatch, IRO_PtrAdd, IRType(IRTK_Pointer), sPointer, mpcFunction->AddInteger(sType, lliStep * lliScale));
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
