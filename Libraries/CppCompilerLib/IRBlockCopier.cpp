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
#include "IRBlockCopier.h"


//////////////////////////////////////////////////////////////////////////
//
// pcCopy (empty, with no jump) becomes pcFrom's copy.  Every block pcFrom
// names must have its copy by now (CopyOf): the copies are made first, then
// filled.
//
//////////////////////////////////////////////////////////////////////////
void CIRBlockCopier::CopyBlock(CIRBlock* pcFrom, CIRBlock* pcCopy)
{
	CopyPhis(pcFrom, pcCopy);
	CopyInstructions(pcFrom, pcCopy);
	CopyJump(pcFrom, pcCopy);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlockCopier::CopyPhis(CIRBlock* pcFrom, CIRBlock* pcCopy)
{
	SIRPhi*			psFrom;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRReference	sValue;
	size			ui;
	size			uiArgument;

	for (ui = 0; ui < pcFrom->masPhis.NumElements(); ui++)
	{
		psFrom = pcFrom->masPhis.Get(ui);
		sValue = CopyValue(psFrom->sResult);
		psPhi = pcCopy->AddPhi(psFrom->sType, sValue);
		for (uiArgument = 0; uiArgument < psFrom->asArguments.NumElements(); uiArgument++)
		{
			psArgument = psFrom->asArguments.Get(uiArgument);
			sValue = CopyValue(psArgument->sValue);
			pcCopy->AddPhiArgument(psPhi, CopyOf(psArgument->pcBlock), sValue);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pcFrom's instructions added to the end of pcCopy's: the operands first
// (CopyOperands), then the result.  Its debug value markers go with them
// when the copier keeps them (CopiesDebugValues).
//
//////////////////////////////////////////////////////////////////////////
void CIRBlockCopier::CopyInstructions(CIRBlock* pcFrom, CIRBlock* pcCopy)
{
	SIRInstruction*	psInstruction;
	SIRInstruction	sCopy;
	SIRDebugValue*	psValue;
	size			uiFirst;
	size			ui;

	uiFirst = pcCopy->masInstructions.NumElements();
	for (ui = 0; ui < pcFrom->masInstructions.NumElements(); ui++)
	{
		psInstruction = pcFrom->masInstructions.Get(ui);
		sCopy = *psInstruction;
		CopyOperands(psInstruction, &sCopy);
		sCopy.sResult = CopyValue(psInstruction->sResult);
		pcCopy->masInstructions.Add(&sCopy);
	}
	for (ui = 0; CopiesDebugValues() && (ui < pcFrom->masDebugValues.NumElements()); ui++)
	{
		psValue = pcFrom->masDebugValues.Get(ui);
		pcCopy->AddDebugValue(uiFirst + psValue->uiBefore, psValue->uiSlot, CopyValue(psValue->sValue));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a copy keeps the debug value markers: their values are taken
// through CopyValue, so a copier that makes values as it goes (or copies
// into another function) doesn't.
//
//////////////////////////////////////////////////////////////////////////
bool CIRBlockCopier::CopiesDebugValues(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// pcCopy leaves as pcFrom does, to the copies of where pcFrom goes.  A
// block with no jump yet is copied as one.
//
//////////////////////////////////////////////////////////////////////////
void CIRBlockCopier::CopyJump(CIRBlock* pcFrom, CIRBlock* pcCopy)
{
	SIRJump*		psJump;
	SIRReference	sCondition;

	psJump = &pcFrom->msJump;
	switch (psJump->uiKind)
	{
	case IRJ_None:
		break;
	case IRJ_Jump:
		pcCopy->Jump(CopyOf(psJump->pcThen));
		break;
	case IRJ_Branch:
		sCondition = CopyValue(psJump->sArgument);
		pcCopy->Branch(sCondition, CopyOf(psJump->pcThen), CopyOf(psJump->pcElse));
		break;
	case IRJ_Return:
		CopyReturn(pcFrom, pcCopy);
		break;
	case IRJ_Throw:
		CopyThrow(pcFrom, pcCopy);
		break;
	default:
		pcCopy->Unreachable();
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// *psCopy (psFrom, copied) gets its operands: each psFrom's as it is in the
// copy.
//
//////////////////////////////////////////////////////////////////////////
void CIRBlockCopier::CopyOperands(SIRInstruction* psFrom, SIRInstruction* psCopy)
{
	psCopy->asArguments[0] = CopyValue(psFrom->asArguments[0]);
	psCopy->asArguments[1] = CopyValue(psFrom->asArguments[1]);
}


//////////////////////////////////////////////////////////////////////////
//
// pcFrom returns: pcCopy returns the same, the value as it is in the copy.
//
//////////////////////////////////////////////////////////////////////////
void CIRBlockCopier::CopyReturn(CIRBlock* pcFrom, CIRBlock* pcCopy)
{
	if (pcFrom->msJump.sType.IsNone())
	{
		pcCopy->Return();
	}
	else
	{
		pcCopy->Return(pcFrom->msJump.sType, CopyValue(pcFrom->msJump.sArgument));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlockCopier::CopyThrow(CIRBlock* pcFrom, CIRBlock* pcCopy)
{
	pcCopy->Throw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRMappedCopier::Init(void)
{
	masMap.Init();
	mapcBlocks.Init();
	mapcCopies.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRMappedCopier::Kill(void)
{
	mapcCopies.Kill();
	mapcBlocks.Kill();
	masMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A new copy within pcFunction: no blocks, and each temporary it has now
// itself.
//
//////////////////////////////////////////////////////////////////////////
void CIRMappedCopier::Begin(CIRFunction* pcFunction)
{
	masMap.FakeSetUsedElements(0);
	masMap.GrowToAtLeastNumElements(pcFunction->NumTemporaries(), true, 0);
	mapcBlocks.FakeSetUsedElements(0);
	mapcCopies.FakeSetUsedElements(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRMappedCopier::AddBlock(CIRBlock* pcBlock, CIRBlock* pcCopy)
{
	mapcBlocks.Add(pcBlock);
	mapcCopies.Add(pcCopy);
}


//////////////////////////////////////////////////////////////////////////
//
// Temporary sValue (one the function had at Begin) is sCopy in the copy.
//
//////////////////////////////////////////////////////////////////////////
void CIRMappedCopier::SetValue(SIRReference sValue, SIRReference sCopy)
{
	*masMap.Get(sValue.Index()) = sCopy;
}


//////////////////////////////////////////////////////////////////////////
//
// sValue as the copy sees it: what the map gives for a temporary, else
// itself.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRMappedCopier::Value(SIRReference sValue)
{
	SIRReference	sMapped;

	if (!sValue.IsTemporary() || (sValue.Index() >= masMap.NumElements()))
	{
		return sValue;
	}
	sMapped = *masMap.Get(sValue.Index());
	return sMapped.IsNone() ? sValue : sMapped;
}


//////////////////////////////////////////////////////////////////////////
//
// pcBlock's copy when it is one of the blocks copied, else pcBlock.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRMappedCopier::Block(CIRBlock* pcBlock)
{
	size	ui;

	for (ui = 0; ui < mapcBlocks.NumElements(); ui++)
	{
		if (mapcBlocks.GetPtr(ui) == pcBlock)
		{
			return mapcCopies.GetPtr(ui);
		}
	}
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRMappedCopier::CopyValue(SIRReference sValue)
{
	return Value(sValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRMappedCopier::CopyOf(CIRBlock* pcBlock)
{
	return Block(pcBlock);
}
