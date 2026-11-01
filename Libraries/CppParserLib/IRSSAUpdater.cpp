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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRSSAUpdater::Init(CIRFunction* pcFunction)
{
	mpcFunction = pcFunction;
	msType = IRType(IRTK_None);
	masEnd.Init();
	masStart.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRSSAUpdater::Kill(void)
{
	masStart.Kill();
	masEnd.Kill();
	mpcFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A new value of sType with no definitions yet.  The blocks must be
// numbered.
//
//////////////////////////////////////////////////////////////////////////
void CIRSSAUpdater::Begin(SIRType sType)
{
	msType = sType;
	masEnd.FakeSetUsedElements(0);
	masEnd.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0);
	masStart.FakeSetUsedElements(0);
	masStart.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The value is sValue at the end of pcBlock (and after its uses in it).
//
//////////////////////////////////////////////////////////////////////////
void CIRSSAUpdater::Define(CIRBlock* pcBlock, SIRReference sValue)
{
	*masEnd.Get(pcBlock->muiIndex) = sValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The value as pcBlock starts: for a block nothing reaches (no
// predecessors, or only a cycle of blocks with one each), 0 (it is never
// used).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRSSAUpdater::ValueAtStart(CIRBlock* pcBlock)
{
	SIRReference	sValue;
	SIRReference	sArgument;
	SIRPhi*			psPhi;
	size			uiPhi;
	size			ui;

	sValue = *masStart.Get(pcBlock->muiIndex);
	if (!sValue.IsNone())
	{
		return sValue;
	}
	if (pcBlock->mapcPredecessors.NumElements() == 0)
	{
		sValue = mpcFunction->AddConstant(msType, 0);
	}
	else if (IsUnreachedCycle(pcBlock))
	{
		sValue = mpcFunction->AddConstant(msType, 0);
	}
	else if (pcBlock->mapcPredecessors.NumElements() == 1)
	{
		sValue = ValueAtEnd(pcBlock->mapcPredecessors.GetPtr(0));
	}
	else
	{
		//The phi is the value here before its arguments are found, so a loop back to it ends.
		sValue = mpcFunction->AddTemporary(msType);
		*masStart.Get(pcBlock->muiIndex) = sValue;
		pcBlock->AddPhi(msType, sValue);
		uiPhi = pcBlock->masPhis.NumElements() - 1;
		for (ui = 0; ui < pcBlock->mapcPredecessors.NumElements(); ui++)
		{
			sArgument = ValueAtEnd(pcBlock->mapcPredecessors.GetPtr(ui));
			psPhi = pcBlock->masPhis.Get(uiPhi);
			pcBlock->AddPhiArgument(psPhi, pcBlock->mapcPredecessors.GetPtr(ui), sArgument);
		}
	}
	*masStart.Get(pcBlock->muiIndex) = sValue;
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcBlock is in a cycle of blocks each with only the one before it
// as a predecessor (a block that is its own only predecessor): nothing
// reaches them, and looking back through them would never end.  A loop
// that is reached has a head with more than one predecessor, which
// ValueAtStart gives its phi before looking round the loop.
//
//////////////////////////////////////////////////////////////////////////
bool CIRSSAUpdater::IsUnreachedCycle(CIRBlock* pcBlock)
{
	CIRBlock*	pcBack;
	size		ui;

	pcBack = pcBlock;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		if (pcBack->mapcPredecessors.NumElements() != 1)
		{
			return false;
		}
		pcBack = pcBack->mapcPredecessors.GetPtr(0);
		if (pcBack == pcBlock)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRSSAUpdater::ValueAtEnd(CIRBlock* pcBlock)
{
	SIRReference	sValue;

	sValue = *masEnd.Get(pcBlock->muiIndex);
	if (!sValue.IsNone())
	{
		return sValue;
	}
	return ValueAtStart(pcBlock);
}
