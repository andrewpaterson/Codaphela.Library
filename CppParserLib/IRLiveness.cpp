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
#include "IRLiveness.h"


//A range found while the blocks are walked, before they are sorted by temporary.
struct SIRTemporaryRange
{
	int32	iTemporary;
	int32	iFrom;
	int32	iTo;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveness::Init(void)
{
	mpcFunction = NULL;
	muiWords = 0;
	mauiLiveIn.Init();
	mauiLiveOut.Init();
	maiStarts.Init();
	mcLoops.Init();
	masRanges.Init();
	maiFirstRanges.Init();
	mauiWeights.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveness::Kill(void)
{
	mauiWeights.Kill();
	maiFirstRanges.Kill();
	masRanges.Kill();
	mcLoops.Kill();
	maiStarts.Kill();
	mauiLiveOut.Kill();
	mauiLiveIn.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// pcFunction's blocks must be numbered (Renumber) and have their
// predecessors (ComputePredecessors); pcDominators must be computed for
// them.
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveness::Compute(CIRFunction* pcFunction, CIRDominators* pcDominators)
{
	mpcFunction = pcFunction;
	Number();
	mcLoops.Compute(pcFunction, pcDominators);
	ComputeSets();
	BuildRanges();
}


//////////////////////////////////////////////////////////////////////////
//
// The first block starts at 0, where the parameters are defined.
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveness::Number(void)
{
	size		ui;
	int32		iPosition;
	CIRBlock*	pcBlock;

	maiStarts.FakeSetUsedElements(0);
	iPosition = 0;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		maiStarts.Add(&iPosition);
		iPosition += 4 + 2 * (int32)pcBlock->masInstructions.NumElements();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32* CIRLiveness::LiveIn(size uiBlock)
{
	return mauiLiveIn.Get(uiBlock * muiWords);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32* CIRLiveness::LiveOut(size uiBlock)
{
	return mauiLiveOut.Get(uiBlock * muiWords);
}


//////////////////////////////////////////////////////////////////////////
//
// Live in = used before it is defined, or live out and not defined here.
// Live out = the successors' live in and the phi arguments this block
// passes.  Worked backwards over the blocks until nothing changes.
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveness::ComputeSets(void)
{
	CArrayTemplate<uint32>	auiUsed;
	CArrayTemplate<uint32>	auiDefined;
	size					uiNumBlocks;
	size					ui;
	size					uiBlock;
	size					uiInstruction;
	size					uiArgument;
	size					uiSuccessor;
	size					uiWord;
	CIRBlock*				pcBlock;
	CIRBlock*				pcSuccessor;
	SIRInstruction*			psInstruction;
	SIRPhi*					psPhi;
	SIRPhiArgument*			psArgument;
	SIRReference			sValue;
	uint32*					puiUsed;
	uint32*					puiDefined;
	uint32*					puiIn;
	uint32*					puiOut;
	uint32					uiIn;
	bool					bChanged;

	uiNumBlocks = mpcFunction->NumBlocks();
	muiWords = (mpcFunction->NumTemporaries() + 31) / 32;
	mauiLiveIn.FakeSetUsedElements(0);
	mauiLiveIn.GrowToAtLeastNumElements(uiNumBlocks * muiWords, true, 0);
	mauiLiveOut.FakeSetUsedElements(0);
	mauiLiveOut.GrowToAtLeastNumElements(uiNumBlocks * muiWords, true, 0);
	auiUsed.Init();
	auiUsed.GrowToAtLeastNumElements(uiNumBlocks * muiWords, true, 0);
	auiDefined.Init();
	auiDefined.GrowToAtLeastNumElements(uiNumBlocks * muiWords, true, 0);

	//What each block uses before defining (in SSA everything a block uses that it doesn't define), what it defines
	//and the phi arguments it passes on.
	for (uiBlock = 0; uiBlock < uiNumBlocks; uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		puiUsed = auiUsed.Get(uiBlock * muiWords);
		puiDefined = auiDefined.Get(uiBlock * muiWords);
		puiOut = LiveOut(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			sValue = pcBlock->masPhis.Get(ui)->sResult;
			SetBit(sValue.Index(), puiDefined, true);
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				sValue = psInstruction->asArguments[uiArgument];
				if (sValue.IsTemporary())
				{
					SetBit(sValue.Index(), puiUsed, true);
				}
			}
			if (psInstruction->sResult.IsTemporary())
			{
				sValue = psInstruction->sResult;
				SetBit(sValue.Index(), puiDefined, true);
			}
		}
		if (pcBlock->msJump.sArgument.IsTemporary())
		{
			sValue = pcBlock->msJump.sArgument;
			SetBit(sValue.Index(), puiUsed, true);
		}
		for (uiWord = 0; uiWord < muiWords; uiWord++)
		{
			puiUsed[uiWord] &= ~puiDefined[uiWord];
		}
		for (uiSuccessor = 0; uiSuccessor < pcBlock->NumSuccessors(); uiSuccessor++)
		{
			pcSuccessor = pcBlock->GetSuccessor(uiSuccessor);
			for (ui = 0; ui < pcSuccessor->masPhis.NumElements(); ui++)
			{
				psPhi = pcSuccessor->masPhis.Get(ui);
				psArgument = pcSuccessor->FindPhiArgument(psPhi, pcBlock);
				if (psArgument && psArgument->sValue.IsTemporary())
				{
					sValue = psArgument->sValue;
					SetBit(sValue.Index(), puiOut, true);
				}
			}
		}
	}

	do
	{
		bChanged = false;
		for (uiBlock = uiNumBlocks; uiBlock > 0; uiBlock--)
		{
			pcBlock = mpcFunction->GetBlock(uiBlock - 1);
			puiOut = LiveOut(uiBlock - 1);
			for (uiSuccessor = 0; uiSuccessor < pcBlock->NumSuccessors(); uiSuccessor++)
			{
				puiIn = LiveIn(pcBlock->GetSuccessor(uiSuccessor)->muiIndex);
				for (uiWord = 0; uiWord < muiWords; uiWord++)
				{
					puiOut[uiWord] |= puiIn[uiWord];
				}
			}
			puiIn = LiveIn(uiBlock - 1);
			puiUsed = auiUsed.Get((uiBlock - 1) * muiWords);
			puiDefined = auiDefined.Get((uiBlock - 1) * muiWords);
			for (uiWord = 0; uiWord < muiWords; uiWord++)
			{
				uiIn = puiUsed[uiWord] | (puiOut[uiWord] & ~puiDefined[uiWord]);
				if (uiIn != puiIn[uiWord])
				{
					puiIn[uiWord] = uiIn;
					bChanged = true;
				}
			}
		}
	}
	while (bChanged);

	auiDefined.Kill();
	auiUsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// iDepth loops deep: 8 to the depth, at most 8^5.
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveness::AddWeight(SIRReference sValue, int32 iDepth)
{
	uint32*	puiWeight;

	if (!sValue.IsTemporary())
	{
		return;
	}
	puiWeight = mauiWeights.Get(sValue.Index());
	*puiWeight += LoopWeight(iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
// A use's weight at a loop depth: 8 times more per loop, up to five.
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRLiveness::LoopWeight(int32 iDepth)
{
	return 1u << (3 * ((iDepth > 5) ? 5 : iDepth));
}


//////////////////////////////////////////////////////////////////////////
//
// Each block gives a temporary at most one range (in SSA a block can't use
// a value it defines before defining it): from the block's start if it is
// live in (or its definition), to the block's end if it is live out (or
// its last use).  The ranges are sorted by temporary and ranges that touch
// joined.
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveness::BuildRanges(void)
{
	CArrayTemplate<SIRTemporaryRange>	asFound;
	CArrayTemplate<int32>				aiFrom;		//Per temporary, -1 if not in this block.
	CArrayTemplate<int32>				aiTo;
	CArrayTemplate<int32>				aiTouched;
	CArrayTemplate<int32>				aiNext;
	size								uiNumTemporaries;
	size								uiBlock;
	size								ui;
	size								uiArgument;
	size								uiWord;
	int32								iTemporary;
	int32								iStart;
	int32								iEnd;
	int32								iPosition;
	int32								iDepth;
	CIRBlock*							pcBlock;
	SIRInstruction*						psInstruction;
	SIRReference						sValue;
	SIRTemporaryRange*					psFound;
	SIRRange*							psLast;
	SIRRange							sRange;
	uint32*								puiOut;
	uint32								uiBits;

	uiNumTemporaries = mpcFunction->NumTemporaries();
	asFound.Init();
	aiFrom.Init();
	aiFrom.GrowToAtLeastNumElements(uiNumTemporaries, true, -1);
	aiTo.Init();
	aiTo.GrowToAtLeastNumElements(uiNumTemporaries, true, 0);
	aiTouched.Init();
	mauiWeights.FakeSetUsedElements(0);
	mauiWeights.GrowToAtLeastNumElements(uiNumTemporaries, true, 0);

	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		iStart = Start(pcBlock);
		iEnd = Jump(pcBlock) + 2;
		iDepth = LoopDepth(pcBlock);
		aiTouched.FakeSetUsedElements(0);

		puiOut = LiveOut(uiBlock);
		for (uiWord = 0; uiWord < muiWords; uiWord++)
		{
			for (uiBits = puiOut[uiWord]; uiBits != 0; uiBits &= uiBits - 1)
			{
				iTemporary = (int32)FindFirstSetBit((uint64)uiBits) + 32 * (int32)uiWord;
				*aiFrom.Get(iTemporary) = iStart;
				*aiTo.Get(iTemporary) = iEnd;
				aiTouched.Add(&iTemporary);
			}
		}

		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			iTemporary = (int32)pcBlock->masPhis.Get(ui)->sResult.Index();
			if (*aiFrom.Get(iTemporary) < 0)
			{
				*aiFrom.Get(iTemporary) = iStart;
				*aiTo.Get(iTemporary) = iStart + 1;
				aiTouched.Add(&iTemporary);
			}
			AddWeight(pcBlock->masPhis.Get(ui)->sResult, iDepth);
		}

		for (ui = 0; ui <= pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.SafeGet(ui);
			iPosition = Position(pcBlock, ui);
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				if (psInstruction)
				{
					sValue = psInstruction->asArguments[uiArgument];
				}
				else
				{
					sValue = (uiArgument == 0) ? pcBlock->msJump.sArgument : IRNone();
				}
				if (!sValue.IsTemporary())
				{
					continue;
				}
				AddWeight(sValue, iDepth);
				iTemporary = (int32)sValue.Index();
				if (*aiFrom.Get(iTemporary) < 0)
				{
					*aiFrom.Get(iTemporary) = iStart;
					*aiTo.Get(iTemporary) = iPosition + 1;
					aiTouched.Add(&iTemporary);
				}
				else if (*aiTo.Get(iTemporary) < iPosition + 1)
				{
					*aiTo.Get(iTemporary) = iPosition + 1;
				}
			}
			if (psInstruction && psInstruction->sResult.IsTemporary())
			{
				AddWeight(psInstruction->sResult, iDepth);
				iTemporary = (int32)psInstruction->sResult.Index();
				if (*aiFrom.Get(iTemporary) < 0)
				{
					*aiTo.Get(iTemporary) = iPosition + 2;
					aiTouched.Add(&iTemporary);
				}
				*aiFrom.Get(iTemporary) = iPosition + 1;
			}
		}

		for (ui = 0; ui < aiTouched.NumElements(); ui++)
		{
			iTemporary = *aiTouched.Get(ui);
			psFound = asFound.Add();
			psFound->iTemporary = iTemporary;
			psFound->iFrom = *aiFrom.Get(iTemporary);
			psFound->iTo = *aiTo.Get(iTemporary);
			*aiFrom.Get(iTemporary) = -1;
		}
	}

	//Parameters are defined at 0, before the first block's start.
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		AddWeight(*mpcFunction->masParameters.Get(ui), 0);
	}

	//Sorted by temporary (the blocks were walked in order, so each temporary's ranges are in order), touching ranges
	//joined.
	maiFirstRanges.FakeSetUsedElements(0);
	maiFirstRanges.GrowToAtLeastNumElements(uiNumTemporaries + 1, true, 0);
	for (ui = 0; ui < asFound.NumElements(); ui++)
	{
		(*maiFirstRanges.Get(asFound.Get(ui)->iTemporary + 1))++;
	}
	for (ui = 0; ui < uiNumTemporaries; ui++)
	{
		*maiFirstRanges.Get(ui + 1) += *maiFirstRanges.Get(ui);
	}
	aiNext.Init();
	aiNext.Copy(&maiFirstRanges);
	masRanges.FakeSetUsedElements(0);
	masRanges.GrowToAtLeastNumElements(asFound.NumElements(), false);
	for (ui = 0; ui < asFound.NumElements(); ui++)
	{
		psFound = asFound.Get(ui);
		sRange.iFrom = psFound->iFrom;
		sRange.iTo = psFound->iTo;
		*masRanges.Get((*aiNext.Get(psFound->iTemporary))++) = sRange;
	}
	aiNext.Kill();

	//Join in place, moving each temporary's ranges down.
	iPosition = 0;
	for (ui = 0; ui < uiNumTemporaries; ui++)
	{
		iStart = *maiFirstRanges.Get(ui);
		iEnd = *maiFirstRanges.Get(ui + 1);
		*maiFirstRanges.Get(ui) = iPosition;
		psLast = NULL;
		for (; iStart < iEnd; iStart++)
		{
			sRange = *masRanges.Get(iStart);
			if (psLast && (psLast->iTo >= sRange.iFrom))
			{
				if (sRange.iTo > psLast->iTo)
				{
					psLast->iTo = sRange.iTo;
				}
				continue;
			}
			psLast = masRanges.Get(iPosition);
			*psLast = sRange;
			iPosition++;
		}
	}
	*maiFirstRanges.Get(uiNumTemporaries) = iPosition;
	masRanges.FakeSetUsedElements(iPosition);

	//A parameter's first range starts at 0.
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		sValue = *mpcFunction->masParameters.Get(ui);
		if (NumRanges(sValue) > 0)
		{
			GetRanges(sValue)->iFrom = 0;
		}
	}

	aiTouched.Kill();
	aiTo.Kill();
	aiFrom.Kill();
	asFound.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIRLiveness::Start(CIRBlock* pcBlock)
{
	return *maiStarts.Get(pcBlock->muiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIRLiveness::Position(CIRBlock* pcBlock, size uiInstruction)
{
	return Start(pcBlock) + 2 + 2 * (int32)uiInstruction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIRLiveness::Jump(CIRBlock* pcBlock)
{
	return Position(pcBlock, pcBlock->masInstructions.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIRLiveness::LoopDepth(CIRBlock* pcBlock)
{
	return mcLoops.Depth(pcBlock);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRLiveness::NumRanges(SIRReference sTemporary)
{
	return (size)(*maiFirstRanges.Get(sTemporary.Index() + 1) - *maiFirstRanges.Get(sTemporary.Index()));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRRange* CIRLiveness::GetRanges(SIRReference sTemporary)
{
	return masRanges.Get(*maiFirstRanges.Get(sTemporary.Index()));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRLiveness::GetWeight(SIRReference sTemporary)
{
	return *mauiWeights.Get(sTemporary.Index());
}


//////////////////////////////////////////////////////////////////////////
//
// Both lists sorted and not overlapping themselves.
//
//////////////////////////////////////////////////////////////////////////
bool CIRLiveness::Intersect(SIRRange* psRanges1, size uiNum1, SIRRange* psRanges2, size uiNum2)
{
	size	ui1;
	size	ui2;

	ui1 = 0;
	ui2 = 0;
	while ((ui1 < uiNum1) && (ui2 < uiNum2))
	{
		if (psRanges1[ui1].iTo <= psRanges2[ui2].iFrom)
		{
			ui1++;
		}
		else if (psRanges2[ui2].iTo <= psRanges1[ui1].iFrom)
		{
			ui2++;
		}
		else
		{
			return true;
		}
	}
	return false;
}
