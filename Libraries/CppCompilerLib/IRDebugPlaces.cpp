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
#include "IRDebugPlaces.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRDebugPlaces::Init(void)
{
	mpcFunction = NULL;
	mcLiveness.Init();
	muiSlots = 0;
	masEntries.Init();
	masValues.Init();
	mpcBlock = NULL;
	muiNext = 0;
	miPosition = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRDebugPlaces::Kill(void)
{
	masValues.Kill();
	masEntries.Kill();
	mcLiveness.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Starts on pcFunction as it is about to be written (its blocks in the
// order they are written, numbered).  False if it has no debug value
// markers: there is nothing to describe.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDebugPlaces::Begin(CIRFunction* pcFunction, CIRDominators* pcDominators)
{
	size			uiBlock;
	size			ui;
	CIRBlock*		pcBlock;
	SIRDebugValue*	psMarker;

	mpcFunction = pcFunction;
	mpcBlock = NULL;
	muiSlots = 0;
	for (uiBlock = 0; uiBlock < pcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = pcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masDebugValues.NumElements(); ui++)
		{
			psMarker = pcBlock->masDebugValues.Get(ui);
			if (psMarker->uiSlot >= muiSlots)
			{
				muiSlots = psMarker->uiSlot + 1;
			}
		}
	}
	if (muiSlots == 0)
	{
		return false;
	}

	mcLiveness.Compute(pcFunction, pcDominators);
	ComputeEntries();
	masValues.ReInit();
	masValues.GrowToAtLeastNumElements(muiSlots, true, 0);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What each slot holds as each block starts, until nothing changes: a
// block not yet reached from the entry (round a loop's back edge) is left
// out of its successors' agreement, so a loop that keeps a value keeps it.
// (Arrays of references are filled with IRNone as zero bytes.)
//
//////////////////////////////////////////////////////////////////////////
void CIRDebugPlaces::ComputeEntries(void)
{
	CArrayIRReferences		asExits;		//Per block, per slot: what it holds as the block ends.
	CArrayTemplate<bool>	abReached;
	CArrayIRReferences		asValues;
	CIRBlock*				pcBlock;
	size					uiBlock;
	size					uiNext;
	size					uiBlocks;
	bool					bChanged;

	uiBlocks = mpcFunction->NumBlocks();
	masEntries.ReInit();
	masEntries.GrowToAtLeastNumElements(uiBlocks * muiSlots, true, 0);
	asExits.Init();
	asExits.GrowToAtLeastNumElements(uiBlocks * muiSlots, true, 0);
	abReached.Init();
	abReached.GrowToAtLeastNumElements(uiBlocks, true, 0);
	asValues.Init();
	asValues.GrowToAtLeastNumElements(muiSlots, true, 0);

	do
	{
		bChanged = false;
		for (uiBlock = 0; uiBlock < uiBlocks; uiBlock++)
		{
			pcBlock = mpcFunction->GetBlock(uiBlock);
			if (!Enter(pcBlock, asValues.GetData(), &asExits, &abReached) && (uiBlock != 0))
			{
				continue;
			}
			if (memcmp(asValues.GetData(), masEntries.Get(uiBlock * muiSlots), muiSlots * sizeof(SIRReference)) != 0)
			{
				memcpy(masEntries.Get(uiBlock * muiSlots), asValues.GetData(), muiSlots * sizeof(SIRReference));
				bChanged = true;
			}
			uiNext = 0;
			Apply(pcBlock, pcBlock->masInstructions.NumElements(), &uiNext, asValues.GetData());
			if (!*abReached.Get(uiBlock) || (memcmp(asValues.GetData(), asExits.Get(uiBlock * muiSlots), muiSlots * sizeof(SIRReference)) != 0))
			{
				memcpy(asExits.Get(uiBlock * muiSlots), asValues.GetData(), muiSlots * sizeof(SIRReference));
				*abReached.Get(uiBlock) = true;
				bChanged = true;
			}
		}
	}
	while (bChanged);

	asValues.Kill();
	abReached.Kill();
	asExits.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What psValues holds as pcBlock starts: per slot, the value its reached
// predecessors (pabReached, their ends in pasExits) all agree on, else
// the block's phi that takes from each the value it ends with (MatchPhi),
// else none.  False if none of them is reached yet.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDebugPlaces::Enter(CIRBlock* pcBlock, SIRReference* psValues, CArrayIRReferences* pasExits, CArrayTemplate<bool>* pabReached)
{
	CIRBlock*		pcPredecessor;
	SIRReference*	psExit;
	SIRReference*	psFirst;
	size			ui;
	size			uiSlot;

	psFirst = NULL;
	for (uiSlot = 0; uiSlot < muiSlots; uiSlot++)
	{
		psValues[uiSlot] = IRNone();
	}
	for (ui = 0; ui < pcBlock->mapcPredecessors.NumElements(); ui++)
	{
		pcPredecessor = *pcBlock->mapcPredecessors.Get(ui);
		if (*pabReached->Get(pcPredecessor->muiIndex))
		{
			psFirst = pasExits->Get(pcPredecessor->muiIndex * muiSlots);
			break;
		}
	}
	if (!psFirst)
	{
		return false;
	}

	for (uiSlot = 0; uiSlot < muiSlots; uiSlot++)
	{
		psValues[uiSlot] = psFirst[uiSlot];
		for (ui = 0; ui < pcBlock->mapcPredecessors.NumElements(); ui++)
		{
			pcPredecessor = *pcBlock->mapcPredecessors.Get(ui);
			psExit = pasExits->Get(pcPredecessor->muiIndex * muiSlots);
			if (*pabReached->Get(pcPredecessor->muiIndex) && !psExit[uiSlot].Equals(psFirst[uiSlot]))
			{
				psValues[uiSlot] = MatchPhi(pcBlock, uiSlot, pasExits, pabReached);
				break;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The phi of pcBlock that takes from each reached predecessor the value
// slot uiSlot holds as it ends: the slot's value as the block starts.
// None if there is no such phi.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRDebugPlaces::MatchPhi(CIRBlock* pcBlock, size uiSlot, CArrayIRReferences* pasExits, CArrayTemplate<bool>* pabReached)
{
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	size			uiPhi;
	size			ui;

	for (uiPhi = 0; uiPhi < pcBlock->masPhis.NumElements(); uiPhi++)
	{
		psPhi = pcBlock->masPhis.Get(uiPhi);
		for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
		{
			psArgument = psPhi->asArguments.Get(ui);
			if (*pabReached->Get(psArgument->pcBlock->muiIndex) && !psArgument->sValue.Equals(*pasExits->Get(psArgument->pcBlock->muiIndex * muiSlots + uiSlot)))
			{
				break;
			}
		}
		if (ui == psPhi->asArguments.NumElements())
		{
			return psPhi->sResult;
		}
	}
	return IRNone();
}


//////////////////////////////////////////////////////////////////////////
//
// pcBlock's markers from *puiNext that are before instruction uiBefore or
// earlier (its jump when it is the number of instructions) applied to
// psValues; *puiNext moves past them.
//
//////////////////////////////////////////////////////////////////////////
void CIRDebugPlaces::Apply(CIRBlock* pcBlock, size uiBefore, size* puiNext, SIRReference* psValues)
{
	SIRDebugValue*	psMarker;

	for (; *puiNext < pcBlock->masDebugValues.NumElements(); (*puiNext)++)
	{
		psMarker = pcBlock->masDebugValues.Get(*puiNext);
		if (psMarker->uiBefore > uiBefore)
		{
			break;
		}
		psValues[psMarker->uiSlot] = psMarker->sValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// To just before pcBlock's instruction uiInstruction (its jump when it is
// the number of instructions): a block's instructions in order, the
// blocks in any order.
//
//////////////////////////////////////////////////////////////////////////
void CIRDebugPlaces::Move(CIRBlock* pcBlock, size uiInstruction)
{
	if (pcBlock != mpcBlock)
	{
		memcpy(masValues.GetData(), masEntries.Get(pcBlock->muiIndex * muiSlots), muiSlots * sizeof(SIRReference));
		mpcBlock = pcBlock;
		muiNext = 0;
	}
	Apply(pcBlock, uiInstruction, &muiNext, masValues.GetData());
	miPosition = (uiInstruction < pcBlock->masInstructions.NumElements()) ? mcLiveness.Position(pcBlock, uiInstruction) : mcLiveness.Jump(pcBlock);
}


size CIRDebugPlaces::NumSlots(void) const { return muiSlots; }


//////////////////////////////////////////////////////////////////////////
//
// The temporary slot uiSlot holds where Move moved to, while it is live
// there; else none.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRDebugPlaces::Value(size uiSlot)
{
	SIRReference	sValue;
	SIRRange*		psRanges;
	size			uiRanges;
	size			ui;

	sValue = *masValues.Get(uiSlot);
	if (!sValue.IsTemporary())
	{
		return IRNone();
	}
	psRanges = mcLiveness.GetRanges(sValue);
	uiRanges = mcLiveness.NumRanges(sValue);
	for (ui = 0; ui < uiRanges; ui++)
	{
		if ((miPosition >= psRanges[ui].iFrom) && (miPosition < psRanges[ui].iTo))
		{
			return sValue;
		}
	}
	return IRNone();
}
