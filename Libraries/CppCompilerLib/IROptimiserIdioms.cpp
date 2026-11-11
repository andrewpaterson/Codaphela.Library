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
#include <string.h>
#include "IROptimiserInternal.h"


//The optimiser's loop idioms: loops that fill, copy, compare or search memory replaced by library calls.


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
	uint16			uiOpcode;

	psCompare = mcDefinitions.Instruction(pcLatch->msJump.sArgument);
	if ((pcLatch->msJump.uiKind != IRJ_Branch) || (psCompare == NULL) || !IsIRIntegerComparison(psCompare->uiOpcode) || !mcLoops.GetLoop(uiLoop)->pcEntering)
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
	return GuardedBy(uiLoop, uiOpcode, psCounter->sInit, psCounter->sBound);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether loop uiLoop is only entered when sLeft uiOpcode sRight (an
// integer comparison): the way in branches on it (as RotateLoops leaves
// it), past blocks of only a jump (preheaders), or both are constants for
// which it holds (IRCompareIntegers).  Only the nearest branch counts, not
// LoopGuarded's walk up the dominators: a loop an idiom has put its own
// check in front of (the memmove's overlap test) is then not matched again.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::GuardedBy(size uiLoop, uint16 uiOpcode, SIRReference sLeft, SIRReference sRight)
{
	SIRInstruction*	psGuard;
	CIRBlock*		pcEntering;
	CIRBlock*		pcTarget;
	int				iSteps;

	pcEntering = mcLoops.GetLoop(uiLoop)->pcEntering;
	if (!pcEntering)
	{
		return false;
	}
	pcTarget = mcLoops.GetLoop(uiLoop)->pcHead;
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
			   psGuard->asArguments[0].Equals(sLeft) && psGuard->asArguments[1].Equals(sRight);
	}
	return sLeft.IsConstant() && sRight.IsConstant() &&
		   IRCompareIntegers(uiOpcode, mpcFunction->TypeOf(sLeft), mpcFunction->GetConstant(sLeft)->ulliBits, mpcFunction->GetConstant(sRight)->ulliBits);
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
// Whether pcBlock does nothing with an effect but iLoads loads, none
// volatile: they go in apsLoads in order.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::OnlyLoads(CIRBlock* pcBlock, int iLoads, SIRInstruction** apsLoads)
{
	SIRInstruction*	psInstruction;
	size			ui;
	int				iFound;

	iFound = 0;
	for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		if (psInstruction->uiOpcode == IRO_Load)
		{
			if ((iFound == iLoads) || (psInstruction->uiFlags & IRF_Volatile))
			{
				return false;
			}
			apsLoads[iFound] = psInstruction;
			iFound++;
		}
		else if (!IsIRPure(psInstruction->uiOpcode))
		{
			return false;
		}
	}
	return iFound == iLoads;
}


//////////////////////////////////////////////////////////////////////////
//
// A call of the library's szName (memset, strlen ...) at the end of pcBlock
// with the arguments that aren't IRNone, worked out before it.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIROptimiser::LibraryCall(CIRBlock* pcBlock, const char* szName, SIRType sResultType, SIRReference sFirst, SIRType sFirstType, SIRReference sSecond, SIRType sSecondType, SIRReference sThird, SIRType sThirdType)
{
	uint32	uiSymbol;

	mpcFunction->Argument(pcBlock, sFirstType, sFirst);
	if (!sSecond.IsNone())
	{
		mpcFunction->Argument(pcBlock, sSecondType, sSecond);
	}
	if (!sThird.IsNone())
	{
		mpcFunction->Argument(pcBlock, sThirdType, sThird);
	}
	uiSymbol = mpcModule->AddSymbol(szName);
	mpcModule->SetRegisterCall(uiSymbol);
	return mpcFunction->Call(pcBlock, sResultType, IRReference(IRRK_Symbol, uiSymbol));
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
		(strcmp(szName, "_memcmp") == 0) || (strcmp(szName, "_strlen") == 0) || (strcmp(szName, "_memchr") == 0) || (strcmp(szName, "_strchr") == 0))
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
		if (ReplaceStrlen(uiLoop) || ReplaceMemcmp(uiLoop) || ReplaceMemchr(uiLoop) || ReplaceStrchr(uiLoop))
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
			LibraryCall(pcPreheader, "_memset", sPointer, sDestinationStart, sPointer, sValue, msLibraryInt, sBytes, IRType(IRTK_I32));
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
			LibraryCall(pcCall, "_memmove", sPointer, sDestinationStart, sPointer, sSourceStart, sPointer, sBytes, IRType(IRTK_I32));
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
// instructions, jumps and phis: OnlyUsedIn).  mauiUses must be counted.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::OnlyUsedInLoop(size uiLoop)
{
	CArrayIRBlocks	apcBlocks;
	size			ui;
	bool			bOnly;

	apcBlocks.Init();
	for (ui = 0; ui < mcLoops.NumBlocks(uiLoop); ui++)
	{
		apcBlocks.Add(mcLoops.GetBlock(uiLoop, ui));
	}
	bOnly = OnlyUsedIn(&apcBlocks, false);
	apcBlocks.Kill();
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
	if (!OnlyLoads(pcLoop, 1, &psLoad) || !psLoad->sType.Is(IRTK_I8) || (*mauiUses.Get(psLoad->sResult.Index()) != 1))
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
			bResult = (MatchLinear(uiLoop, sValue, &sStart, &lliStep, &bNext));
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
	sLength = LibraryCall(pcPreheader, "_strlen", IRType(IRTK_I32), sStart, IRType(IRTK_Pointer));
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
			MatchLinear(uiLoop, sValue, &sStart, &lliStep, &bNext);
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
// Whether sValue, worked out in the loop, is start + k * step the k-th time
// round (from 0): a phi of its head taking start on the way in and phi +
// step (an add or ptradd of a constant) from the loop, or that next value
// (*pbNext: start + (k + 1) * step).
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::MatchLinear(size uiLoop, SIRReference sValue, SIRReference* psStart, int64* plliStep, bool* pbNext)
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
	if ((psPhi == NULL) || (mcDefinitions.Block(sPhi) != mcLoops.GetLoop(uiLoop)->pcHead) || (psPhi->asArguments.NumElements() != 2))
	{
		return false;
	}
	*psStart = IRNone();
	*plliStep = 0;
	for (ui = 0; ui < 2; ui++)
	{
		psArgument = psPhi->asArguments.Get(ui);
		if (mcLoops.Contains(uiLoop, psArgument->pcBlock))
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
	SIRInstruction*	psCompare;
	SIRInstruction*	apsLoads[2];
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
	if (!OnlyLoads(pcHead, 2, apsLoads) || !OnlyLoads(pcLatch, 0, NULL))
	{
		return false;
	}
	psFirst = apsLoads[0];
	psSecond = apsLoads[1];
	if (!psFirst->sType.Equals(psSecond->sType) || !psFirst->sType.IsInteger())
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
	sResult = LibraryCall(pcPreheader, "_memcmp", msLibraryInt, sFirstStart, IRType(IRTK_Pointer), sSecondStart, IRType(IRTK_Pointer), sBytes, IRType(IRTK_I32));
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


//////////////////////////////////////////////////////////////////////////
//
// Whether the values the loop works out are only used, outside it, by
// pcFound's phis (from pcFrom) or in blocks pcFound dominates, and are then
// phis of the loop stepped by a constant (MatchLinear, not next values).
// The values used are added to pasUsed.  mauiUses must be counted.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::FoundValues(size uiLoop, CIRBlock* pcFrom, CIRBlock* pcFound, CArrayIRReferences* pasUsed)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sValue;
	SIRReference	sStart;
	int64			lliStep;
	size			ui;
	size			uiIndex;
	size			uiArgument;
	bool			bNext;
	bool			bAllowed;

	pasUsed->FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if (mcLoops.Contains(uiLoop, pcBlock) || !mcDominators.IsReachable(pcBlock))
		{
			continue;
		}
		bAllowed = mcDominators.Dominates(pcFound, pcBlock);
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				psArgument = psPhi->asArguments.Get(uiArgument);
				sValue = psArgument->sValue;
				if (mcDefinitions.Block(sValue) && mcLoops.Contains(uiLoop, mcDefinitions.Block(sValue)))
				{
					if (!((pcBlock == pcFound) && (psArgument->pcBlock == pcFrom)) && !(bAllowed && (pcBlock != pcFound)))
					{
						return false;
					}
					pasUsed->Add(&sValue);
				}
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements() + 1; uiIndex++)
		{
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				if (uiIndex < pcBlock->masInstructions.NumElements())
				{
					psInstruction = pcBlock->masInstructions.Get(uiIndex);
					sValue = psInstruction->asArguments[uiArgument];
				}
				else
				{
					sValue = (uiArgument == 0) ? pcBlock->msJump.sArgument : IRNone();
				}
				if (sValue.IsTemporary() && mcDefinitions.Block(sValue) && mcLoops.Contains(uiLoop, mcDefinitions.Block(sValue)))
				{
					if (!bAllowed)
					{
						return false;
					}
					pasUsed->Add(&sValue);
				}
			}
		}
	}
	for (ui = 0; ui < pasUsed->NumElements(); ui++)
	{
		if (!MatchLinear(uiLoop, *pasUsed->Get(ui), &sStart, &lliStep, &bNext) || bNext)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The values a search loop found at k times round (sTimes, an i32): each in
// pasUsed (a phi of the head stepped by a constant) is start + k * step,
// worked out at the end of pcBlock; the uses outside the loop (by pcFound's
// phis from pcFrom, which then take them from pcBlock, and in the blocks
// pcFound dominates) use them.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::FoundAt(size uiLoop, CIRBlock* pcBlock, SIRReference sTimes, CIRBlock* pcFrom, CIRBlock* pcFound, CArrayIRReferences* pasUsed)
{
	CIRBlock*		pcUser;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sValue;
	SIRReference	sStart;
	SIRReference	sOffset;
	SIRReference	sNew;
	SIRType			sType;
	int64			lliStep;
	size			ui;
	size			uj;
	size			uiIndex;
	size			uiArgument;
	bool			bNext;

	for (ui = 0; ui < pasUsed->NumElements(); ui++)
	{
		sValue = *pasUsed->Get(ui);
		for (uj = 0; uj < ui; uj++)
		{
			if (pasUsed->Get(uj)->Equals(sValue))
			{
				break;
			}
		}
		if (uj < ui)
		{
			continue;
		}
		MatchLinear(uiLoop, sValue, &sStart, &lliStep, &bNext);
		sType = mpcFunction->TypeOf(sValue);
		if (sType.IsPointer())
		{
			sOffset = mpcFunction->Add(pcBlock, IRO_Mul, IRType(IRTK_I32), sTimes, mpcFunction->AddInteger(IRType(IRTK_I32), lliStep));
			sNew = mpcFunction->Add(pcBlock, IRO_PtrAdd, sType, sStart, sOffset);
		}
		else
		{
			sOffset = sType.Is(IRTK_I32) ? sTimes : mpcFunction->Add(pcBlock, (sType.IntegerBits() < 32) ? IRO_Trunc : IRO_ZExt, sType, sTimes);
			sOffset = mpcFunction->Add(pcBlock, IRO_Mul, sType, sOffset, mpcFunction->AddInteger(sType, lliStep));
			sNew = mpcFunction->Add(pcBlock, IRO_Add, sType, sStart, sOffset);
		}

		//The uses outside the loop.
		for (uiIndex = 0; uiIndex < pcFound->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcFound->masPhis.Get(uiIndex);
			psArgument = pcFound->FindPhiArgument(psPhi, pcFrom);
			if (psArgument && psArgument->sValue.Equals(sValue))
			{
				psArgument->pcBlock = pcBlock;
				psArgument->sValue = sNew;
			}
		}
		for (uiIndex = 0; uiIndex < mpcFunction->NumBlocks(); uiIndex++)
		{
			pcUser = mpcFunction->GetBlock(uiIndex);
			if (mcLoops.Contains(uiLoop, pcUser) || (pcUser == pcBlock) || !mcDominators.IsReachable(pcUser) || !mcDominators.Dominates(pcFound, pcUser))
			{
				continue;
			}
			for (uj = 0; uj < pcUser->masPhis.NumElements(); uj++)
			{
				psPhi = pcUser->masPhis.Get(uj);
				for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
				{
					if ((pcUser != pcFound) && psPhi->asArguments.Get(uiArgument)->sValue.Equals(sValue))
					{
						psPhi->asArguments.Get(uiArgument)->sValue = sNew;
					}
				}
			}
			for (uj = 0; uj < pcUser->masInstructions.NumElements(); uj++)
			{
				psInstruction = pcUser->masInstructions.Get(uj);
				for (uiArgument = 0; uiArgument < 2; uiArgument++)
				{
					if (psInstruction->asArguments[uiArgument].Equals(sValue))
					{
						psInstruction->asArguments[uiArgument] = sNew;
					}
				}
			}
			if (pcUser->msJump.sArgument.Equals(sValue))
			{
				pcUser->msJump.sArgument = sNew;
			}
		}
	}
	//Phis of pcFound from pcFrom that took a value from outside the loop take it from pcBlock too.
	for (uiIndex = 0; uiIndex < pcFound->masPhis.NumElements(); uiIndex++)
	{
		psArgument = pcFound->FindPhiArgument(pcFound->masPhis.Get(uiIndex), pcFrom);
		if (psArgument)
		{
			psArgument->pcBlock = pcBlock;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// memchr: a loop of a head that loads a byte through an address stepped a
// byte a time round and leaves for one exit (found) when it is c (not
// changed by the loop), and a latch that counts n - init times round
// (MatchIdiomCounter) and leaves for another, nothing else.  On the way in
// memchr: not found (NULL) goes to the second exit, found to a block that
// works out what found uses at k = the byte's offset times round
// (FoundValues), then to the first.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceMemchr(size uiLoop)
{
	SIRIdiomCounter		sCounter;
	SIRIdiomAddress		sAddress;
	CArrayIRReferences	asUsed;
	SIRLoop*			psLoop;
	CIRBlock*			pcHead;
	CIRBlock*			pcLatch;
	CIRBlock*			pcFound;
	CIRBlock*			pcMissing;
	CIRBlock*			pcPreheader;
	CIRBlock*			pcAt;
	SIRInstruction*		psLoad;
	SIRInstruction*		psCompare;
	SIRPhiArgument*		psArgument;
	SIRReference		sValue;
	SIRReference		sStart;
	SIRReference		sResult;
	SIRReference		sTimes;
	SIRReference		sTest;
	size				ui;
	uint16				uiOpcode;
	bool				bResult;

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
		pcFound = pcHead->msJump.pcElse;
	}
	else if (pcHead->msJump.pcElse == pcLatch)
	{
		pcFound = pcHead->msJump.pcThen;
	}
	else
	{
		return false;
	}
	pcMissing = (pcLatch->msJump.pcThen == pcHead) ? pcLatch->msJump.pcElse : pcLatch->msJump.pcThen;
	if ((pcMissing == pcHead) || (pcMissing == pcFound) || mcLoops.Contains(uiLoop, pcFound) || mcLoops.Contains(uiLoop, pcMissing) ||
		(pcFound->mapcPredecessors.NumElements() != 1))
	{
		return false;
	}

	//One byte load compared with c in the head, nothing with an effect in either block.
	if (!OnlyLoads(pcHead, 1, &psLoad) || !OnlyLoads(pcLatch, 0, NULL) || !psLoad->sType.Is(IRTK_I8))
	{
		return false;
	}
	psCompare = mcDefinitions.Instruction(pcHead->msJump.sArgument);
	if ((psCompare == NULL) || ((psCompare->uiOpcode != IRO_Eq) && (psCompare->uiOpcode != IRO_Ne)))
	{
		return false;
	}
	uiOpcode = (pcHead->msJump.pcThen == pcFound) ? psCompare->uiOpcode : IRInvertedComparison(psCompare->uiOpcode);
	if (uiOpcode != IRO_Eq)
	{
		return false;
	}
	if (psCompare->asArguments[0].Equals(psLoad->sResult))
	{
		sValue = psCompare->asArguments[1];
	}
	else if (psCompare->asArguments[1].Equals(psLoad->sResult))
	{
		sValue = psCompare->asArguments[0];
	}
	else
	{
		return false;
	}
	if (!mcLoops.IsInvariant(uiLoop, sValue, &mcDefinitions) || !MatchIdiomCounter(uiLoop, pcHead, pcLatch, &sCounter) ||
		!MatchIdiomAddress(uiLoop, pcHead, pcLatch, psLoad->asArguments[0], &sCounter, 1, &sAddress))
	{
		return false;
	}
	asUsed.Init();
	bResult = FoundValues(uiLoop, pcHead, pcFound, &asUsed);
	for (ui = 0; bResult && (ui < pcMissing->masPhis.NumElements()); ui++)
	{
		psArgument = pcMissing->FindPhiArgument(pcMissing->masPhis.Get(ui), pcLatch);
		bResult = !psArgument || mcLoops.IsInvariant(uiLoop, psArgument->sValue, &mcDefinitions);
	}
	if (!bResult)
	{
		asUsed.Kill();
		return false;
	}

	//memchr on the way in; found works out its values from the offset.
	pcPreheader = AddPreheader(uiLoop);
	sStart = IdiomStart(pcPreheader, &sAddress, &sCounter);
	if (sValue.IsConstant())
	{
		sValue = mpcFunction->AddConstant(msLibraryInt, mpcFunction->GetConstant(sValue)->ulliBits & 0xFF);
	}
	else
	{
		sValue = mpcFunction->Add(pcPreheader, IRO_ZExt, msLibraryInt, sValue);
	}
	sTimes = IdiomBytes(pcPreheader, &sCounter, 1);
	sResult = LibraryCall(pcPreheader, "_memchr", IRType(IRTK_Pointer), sStart, IRType(IRTK_Pointer), sValue, msLibraryInt, sTimes, IRType(IRTK_I32));
	sTest = mpcFunction->Add(pcPreheader, IRO_Eq, IRType(IRTK_Pointer), sResult, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0));
	pcAt = mpcFunction->AddBlock();
	sTimes = mpcFunction->Add(pcAt, IRO_PtrToI, IRType(IRTK_I32), sResult);
	sTimes = mpcFunction->Add(pcAt, IRO_Sub, IRType(IRTK_I32), sTimes, mpcFunction->Add(pcAt, IRO_PtrToI, IRType(IRTK_I32), sStart));
	FoundAt(uiLoop, pcAt, sTimes, pcHead, pcFound, &asUsed);
	pcAt->Jump(pcFound);
	for (ui = 0; ui < pcMissing->masPhis.NumElements(); ui++)
	{
		psArgument = pcMissing->FindPhiArgument(pcMissing->masPhis.Get(ui), pcLatch);
		if (psArgument)
		{
			pcMissing->AddPhiArgument(pcMissing->masPhis.Get(ui), pcPreheader, psArgument->sValue);
		}
	}
	RemovePhiArguments(pcHead, pcPreheader);
	pcPreheader->Branch(sTest, pcMissing, pcAt);
	asUsed.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// strchr, as a rotated "while (*p != c) { if (*p == 0) ...; p++; }" leaves
// it: a loop of a head with a pointer phi p and a phi b of the byte at p
// (loaded from p's start on the way in) that leaves for one exit (zero)
// when b is 0, and a latch that steps p by one, loads the byte there and
// goes round while it isn't c (not changed by the loop), else leaves for
// another (found) with the new p.  On the way in: b 0 goes to zero, else
// strchr(p + 1, c), NULL to zero, else to found with that pointer (and
// with it less one for p).  Nothing else the loop works out is used
// outside.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::ReplaceStrchr(size uiLoop)
{
	SIRLoop*		psLoop;
	CIRBlock*		pcHead;
	CIRBlock*		pcLatch;
	CIRBlock*		pcZero;
	CIRBlock*		pcFound;
	CIRBlock*		pcEntering;
	CIRBlock*		pcPreheader;
	CIRBlock*		pcCall;
	SIRInstruction*	psZeroTest;
	SIRInstruction*	psStep;
	SIRInstruction*	psLoad;
	SIRInstruction*	psTest;
	SIRInstruction*	psFirstLoad;
	SIRPhi*			psPointer;
	SIRPhi*			psByte;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sPointer;
	SIRReference	sByte;
	SIRReference	sStart;
	SIRReference	sFirst;
	SIRReference	sValue;
	SIRReference	sResult;
	SIRReference	sTest;
	SIRReference	sBefore;
	SIRReference	sZero;
	size			ui;
	uint16			uiOpcode;
	bool			bOnly;

	psLoop = mcLoops.GetLoop(uiLoop);
	pcHead = psLoop->pcHead;
	pcEntering = psLoop->pcEntering;
	if ((mcLoops.NumBlocks(uiLoop) != 2) || (mcLoops.NumLatches(uiLoop) != 1) || !pcEntering || (pcHead->msJump.uiKind != IRJ_Branch) ||
		(pcHead->masPhis.NumElements() != 2) || (pcHead->masInstructions.NumElements() != 1))
	{
		return false;
	}
	pcLatch = mcLoops.GetLatch(uiLoop, 0);
	if ((pcLatch == pcHead) || (pcLatch->msJump.uiKind != IRJ_Branch) || (pcLatch->masPhis.NumElements() != 0) || (pcLatch->masInstructions.NumElements() != 3))
	{
		return false;
	}

	//The head: b == 0 to zero, else the latch.
	psZeroTest = pcHead->masInstructions.Get(0);
	if (!pcHead->msJump.sArgument.Equals(psZeroTest->sResult) || ((psZeroTest->uiOpcode != IRO_Eq) && (psZeroTest->uiOpcode != IRO_Ne)) ||
		!psZeroTest->asArguments[1].IsConstant() || (mpcFunction->GetConstant(psZeroTest->asArguments[1])->ulliBits != 0))
	{
		return false;
	}
	if (pcHead->msJump.pcThen == pcLatch)
	{
		pcZero = pcHead->msJump.pcElse;
		uiOpcode = IRInvertedComparison(psZeroTest->uiOpcode);
	}
	else if (pcHead->msJump.pcElse == pcLatch)
	{
		pcZero = pcHead->msJump.pcThen;
		uiOpcode = psZeroTest->uiOpcode;
	}
	else
	{
		return false;
	}
	if (uiOpcode != IRO_Eq)
	{
		return false;
	}
	sByte = psZeroTest->asArguments[0];
	psByte = mcDefinitions.Phi(sByte);
	if (!psByte || (mcDefinitions.Block(sByte) != pcHead) || !psByte->sType.Is(IRTK_I8))
	{
		return false;
	}
	psPointer = (pcHead->masPhis.Get(0) == psByte) ? pcHead->masPhis.Get(1) : pcHead->masPhis.Get(0);
	sPointer = psPointer->sResult;
	if (!psPointer->sType.IsPointer())
	{
		return false;
	}

	//The latch: p + 1, the byte there, round while it isn't c, else found.
	psStep = pcLatch->masInstructions.Get(0);
	psLoad = pcLatch->masInstructions.Get(1);
	psTest = pcLatch->masInstructions.Get(2);
	if ((psStep->uiOpcode != IRO_PtrAdd) || !psStep->asArguments[0].Equals(sPointer) || !psStep->asArguments[1].IsConstant() || (mpcFunction->GetSigned(psStep->asArguments[1]) != 1) ||
		(psLoad->uiOpcode != IRO_Load) || !psLoad->sType.Is(IRTK_I8) || (psLoad->uiFlags & IRF_Volatile) || !psLoad->asArguments[0].Equals(psStep->sResult) ||
		((psTest->uiOpcode != IRO_Ne) && (psTest->uiOpcode != IRO_Eq)) || !pcLatch->msJump.sArgument.Equals(psTest->sResult))
	{
		return false;
	}
	if (psTest->asArguments[0].Equals(psLoad->sResult))
	{
		sValue = psTest->asArguments[1];
	}
	else if (psTest->asArguments[1].Equals(psLoad->sResult))
	{
		sValue = psTest->asArguments[0];
	}
	else
	{
		return false;
	}
	pcFound = (pcLatch->msJump.pcThen == pcHead) ? pcLatch->msJump.pcElse : pcLatch->msJump.pcThen;
	uiOpcode = (pcLatch->msJump.pcThen == pcHead) ? psTest->uiOpcode : IRInvertedComparison(psTest->uiOpcode);
	if ((uiOpcode != IRO_Ne) || (pcFound == pcHead) || (pcFound == pcZero) || mcLoops.Contains(uiLoop, pcFound) || mcLoops.Contains(uiLoop, pcZero) ||
		!mcLoops.IsInvariant(uiLoop, sValue, &mcDefinitions))
	{
		return false;
	}

	//The phis come round from the latch as p + 1 and its byte; in, b is the byte at p's start.
	psArgument = pcHead->FindPhiArgument(psPointer, pcLatch);
	if (!psArgument || !psArgument->sValue.Equals(psStep->sResult))
	{
		return false;
	}
	psArgument = pcHead->FindPhiArgument(psByte, pcLatch);
	if (!psArgument || !psArgument->sValue.Equals(psLoad->sResult))
	{
		return false;
	}
	psArgument = pcHead->FindPhiArgument(psPointer, pcEntering);
	if (!psArgument)
	{
		return false;
	}
	sStart = psArgument->sValue;
	psArgument = pcHead->FindPhiArgument(psByte, pcEntering);
	if (!psArgument)
	{
		return false;
	}
	sFirst = psArgument->sValue;
	psFirstLoad = mcDefinitions.Instruction(sFirst);
	if (!psFirstLoad || (psFirstLoad->uiOpcode != IRO_Load) || !psFirstLoad->asArguments[0].Equals(sStart) || (psFirstLoad->uiFlags & IRF_Volatile))
	{
		return false;
	}

	//Only found's phis from the latch take the loop's values, p + 1 or p: else the loop's values stay in it.
	for (ui = 0; ui < pcFound->masPhis.NumElements(); ui++)
	{
		psArgument = pcFound->FindPhiArgument(pcFound->masPhis.Get(ui), pcLatch);
		if (psArgument && IsLoopValue(uiLoop, psArgument->sValue))
		{
			if (!psArgument->sValue.Equals(psStep->sResult) && !psArgument->sValue.Equals(sPointer))
			{
				return false;
			}
			(*mauiUses.Get(psArgument->sValue.Index()))--;
		}
	}
	bOnly = OnlyUsedInLoop(uiLoop);
	for (ui = 0; ui < pcFound->masPhis.NumElements(); ui++)
	{
		psArgument = pcFound->FindPhiArgument(pcFound->masPhis.Get(ui), pcLatch);
		if (psArgument && IsLoopValue(uiLoop, psArgument->sValue))
		{
			(*mauiUses.Get(psArgument->sValue.Index()))++;
		}
	}
	if (!bOnly)
	{
		return false;
	}

	//On the way in.
	pcPreheader = AddPreheader(uiLoop);
	pcCall = mpcFunction->AddBlock();
	sZero = mpcFunction->Add(pcPreheader, IRO_Eq, IRType(IRTK_I8), sFirst, mpcFunction->AddInteger(IRType(IRTK_I8), 0));
	sValue = sValue.IsConstant() ? mpcFunction->AddConstant(msLibraryInt, mpcFunction->GetConstant(sValue)->ulliBits & 0xFF) :
								   mpcFunction->Add(pcCall, IRO_ZExt, msLibraryInt, sValue);
	sTest = mpcFunction->Add(pcCall, IRO_PtrAdd, IRType(IRTK_Pointer), sStart, mpcFunction->AddInteger(IRType(IRTK_I32), 1));
	sResult = LibraryCall(pcCall, "_strchr", IRType(IRTK_Pointer), sTest, IRType(IRTK_Pointer), sValue, msLibraryInt);
	sBefore = mpcFunction->Add(pcCall, IRO_PtrAdd, IRType(IRTK_Pointer), sResult, mpcFunction->AddInteger(IRType(IRTK_I32), -1));
	pcCall->Branch(mpcFunction->Add(pcCall, IRO_Eq, IRType(IRTK_Pointer), sResult, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0)), pcZero, pcFound);
	for (ui = 0; ui < pcZero->masPhis.NumElements(); ui++)
	{
		psPhi = pcZero->masPhis.Get(ui);
		psArgument = pcZero->FindPhiArgument(psPhi, pcHead);
		if (psArgument)
		{
			sValue = psArgument->sValue;
			pcZero->AddPhiArgument(pcZero->masPhis.Get(ui), pcPreheader, sValue);
			pcZero->AddPhiArgument(pcZero->masPhis.Get(ui), pcCall, sValue);
		}
	}
	for (ui = 0; ui < pcFound->masPhis.NumElements(); ui++)
	{
		psPhi = pcFound->masPhis.Get(ui);
		psArgument = pcFound->FindPhiArgument(psPhi, pcLatch);
		if (psArgument)
		{
			sValue = psArgument->sValue;
			if (sValue.Equals(psStep->sResult))
			{
				sValue = sResult;
			}
			else if (sValue.Equals(sPointer))
			{
				sValue = sBefore;
			}
			pcFound->AddPhiArgument(pcFound->masPhis.Get(ui), pcCall, sValue);
		}
	}
	RemovePhiArguments(pcHead, pcPreheader);
	pcPreheader->Branch(sZero, pcZero, pcCall);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is worked out in the loop.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::IsLoopValue(size uiLoop, SIRReference sValue)
{
	CIRBlock*	pcBlock;

	pcBlock = mcDefinitions.Block(sValue);
	return pcBlock && mcLoops.Contains(uiLoop, pcBlock);
}
