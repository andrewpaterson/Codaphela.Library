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
#include "IRLiveSets.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveSets::Init(void)
{
	mpcFunction = NULL;
	muiWords = 0;
	mauiUsed.Init();
	mauiDefined.Init();
	mauiLiveIn.Init();
	mauiLiveOut.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// With the sets allocated from pcMalloc (a back end's scratch memory).
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveSets::Init(CMallocator* pcMalloc)
{
	mpcFunction = NULL;
	muiWords = 0;
	mauiUsed.Init(pcMalloc);
	mauiDefined.Init(pcMalloc);
	mauiLiveIn.Init(pcMalloc);
	mauiLiveOut.Init(pcMalloc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveSets::Kill(void)
{
	mauiLiveOut.Kill();
	mauiLiveIn.Kill();
	mauiDefined.Kill();
	mauiUsed.Kill();
	mpcFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Empty sets for each block of pcFunction.
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveSets::Start(CIRFunction* pcFunction)
{
	size	uiSize;

	mpcFunction = pcFunction;
	muiWords = (mpcFunction->NumTemporaries() + 31) / 32;
	uiSize = mpcFunction->NumBlocks() * muiWords + 1;
	mauiUsed.FakeSetUsedElements(0);
	mauiUsed.GrowToAtLeastNumElements(uiSize, true, 0);
	mauiDefined.FakeSetUsedElements(0);
	mauiDefined.GrowToAtLeastNumElements(uiSize, true, 0);
	mauiLiveIn.FakeSetUsedElements(0);
	mauiLiveIn.GrowToAtLeastNumElements(uiSize, true, 0);
	mauiLiveOut.FakeSetUsedElements(0);
	mauiLiveOut.GrowToAtLeastNumElements(uiSize, true, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Block uiBlock reads sValue (if a temporary): it is live in unless the
// block defined it first.
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveSets::Use(size uiBlock, SIRReference sValue)
{
	size	ui;

	if (!sValue.IsTemporary())
	{
		return;
	}
	ui = sValue.Index();
	if (!(mauiDefined.Get(uiBlock * muiWords)[ui / 32] & (1u << (ui % 32))))
	{
		mauiUsed.Get(uiBlock * muiWords)[ui / 32] |= 1u << (ui % 32);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveSets::Define(size uiBlock, SIRReference sValue)
{
	size	ui;

	if (!sValue.IsTemporary())
	{
		return;
	}
	ui = sValue.Index();
	mauiDefined.Get(uiBlock * muiWords)[ui / 32] |= 1u << (ui % 32);
}


//////////////////////////////////////////////////////////////////////////
//
// sValue (if a temporary) is live out of block uiBlock: a phi argument the
// block passes to a successor.
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveSets::PassOut(size uiBlock, SIRReference sValue)
{
	size	ui;

	if (!sValue.IsTemporary())
	{
		return;
	}
	ui = sValue.Index();
	mauiLiveOut.Get(uiBlock * muiWords)[ui / 32] |= 1u << (ui % 32);
}


//////////////////////////////////////////////////////////////////////////
//
// in = used | (out & ~defined), out = the successors' ins, the blocks
// last first, until nothing changes.
//
//////////////////////////////////////////////////////////////////////////
void CIRLiveSets::Solve(void)
{
	size		uiBlock;
	size		uiSuccessor;
	size		uiWord;
	CIRBlock*	pcBlock;
	uint32*		puiUsed;
	uint32*		puiDefined;
	uint32*		puiIn;
	uint32*		puiOut;
	uint32*		puiSuccessorIn;
	uint32		uiIn;
	bool		bChanged;

	do
	{
		bChanged = false;
		for (uiBlock = mpcFunction->NumBlocks(); uiBlock > 0; uiBlock--)
		{
			pcBlock = mpcFunction->GetBlock(uiBlock - 1);
			puiOut = LiveOut(uiBlock - 1);
			for (uiSuccessor = 0; uiSuccessor < pcBlock->NumSuccessors(); uiSuccessor++)
			{
				puiSuccessorIn = LiveIn(pcBlock->GetSuccessor(uiSuccessor)->muiIndex);
				for (uiWord = 0; uiWord < muiWords; uiWord++)
				{
					puiOut[uiWord] |= puiSuccessorIn[uiWord];
				}
			}
			puiIn = LiveIn(uiBlock - 1);
			puiUsed = mauiUsed.Get((uiBlock - 1) * muiWords);
			puiDefined = mauiDefined.Get((uiBlock - 1) * muiWords);
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRLiveSets::NumWords(void)
{
	return muiWords;
}


//////////////////////////////////////////////////////////////////////////
//
// Block uiBlock's live in set: NumWords words, a bit per temporary.
//
//////////////////////////////////////////////////////////////////////////
uint32* CIRLiveSets::LiveIn(size uiBlock)
{
	return mauiLiveIn.Get(uiBlock * muiWords);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32* CIRLiveSets::LiveOut(size uiBlock)
{
	return mauiLiveOut.Get(uiBlock * muiWords);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRLiveSets::IsLiveIn(size uiBlock, size uiTemporary)
{
	return (LiveIn(uiBlock)[uiTemporary / 32] & (1u << (uiTemporary % 32))) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRLiveSets::IsLiveOut(size uiBlock, size uiTemporary)
{
	return (LiveOut(uiBlock)[uiTemporary / 32] & (1u << (uiTemporary % 32))) != 0;
}
