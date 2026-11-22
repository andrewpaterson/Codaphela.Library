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
#include "IRThrows.h"


//////////////////////////////////////////////////////////////////////////
//
// True if the instruction is the thrown of a call to a function that never
// throws: it is false.
//
//////////////////////////////////////////////////////////////////////////
bool IRNeverThrown(CIRModule* pcModule, CIRBlock* pcBlock, size uiInstruction)
{
	SIRInstruction*	psCall;

	if ((uiInstruction == 0) || (pcBlock->masInstructions.Get(uiInstruction)->uiOpcode != IRO_Thrown))
	{
		return false;
	}
	psCall = pcBlock->masInstructions.Get(uiInstruction - 1);
	return (psCall->uiOpcode == IRO_Call) && psCall->asArguments[0].IsSymbol() && pcModule->IsNoThrow(psCall->asArguments[0].Index());
}


//////////////////////////////////////////////////////////////////////////
//
// Which ways pcBlock's brif can go, as far as it can be told without
// working anything out.  A brif on a constant goes one way, and so does one
// on the thrown of a call to a function that never throws (the way that
// isn't taken is the code for its exception, until the optimiser takes it
// away), or on a phi of the block's whose values are all the same constant
// (what the inliner makes of the thrown of a call to such a function).
//
//////////////////////////////////////////////////////////////////////////
static void IRBranchWays(CIRModule* pcModule, CIRFunction* pcFunction, CIRBlock* pcBlock, bool* pbThen, bool* pbElse)
{
	SIRReference	sCondition;
	SIRPhi*			psPhi;
	size			uiInstruction;
	size			uiArgument;

	*pbThen = true;
	*pbElse = true;
	sCondition = pcBlock->msJump.sArgument;
	if (sCondition.IsConstant())
	{
		*pbThen = (pcFunction->GetConstant(sCondition)->ulliBits & 1) != 0;
		*pbElse = !*pbThen;
	}
	if (!sCondition.IsTemporary())
	{
		return;
	}
	for (uiInstruction = pcBlock->masInstructions.NumElements(); uiInstruction > 0; uiInstruction--)
	{
		if (pcBlock->masInstructions.Get(uiInstruction - 1)->sResult.Equals(sCondition))
		{
			*pbThen = !IRNeverThrown(pcModule, pcBlock, uiInstruction - 1);
			break;
		}
	}
	psPhi = pcBlock->FindPhi(sCondition);
	if (psPhi)
	{
		*pbThen = false;
		*pbElse = false;
		for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
		{
			if (!psPhi->asArguments.Get(uiArgument)->sValue.IsConstant())
			{
				*pbThen = true;
				*pbElse = true;
			}
			else if (pcFunction->GetConstant(psPhi->asArguments.Get(uiArgument)->sValue)->ulliBits & 1)
			{
				*pbThen = true;
			}
			else
			{
				*pbElse = true;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if a block that ends in throw can be reached from the function's
// entry, a brif going only the ways it can be told to go (IRBranchWays).
//
//////////////////////////////////////////////////////////////////////////
bool IRMayThrow(CIRModule* pcModule, CIRFunction* pcFunction)
{
	CArrayTemplate<uint8>	auiReached;
	CArrayTemplate<size>	auiStack;
	CIRBlock*				pcBlock;
	SIRJump*				psJump;
	size					ui;
	bool					bThen;
	bool					bElse;
	bool					bThrows;

	if (pcFunction->NumBlocks() == 0)
	{
		return true;
	}
	pcFunction->Renumber();
	auiReached.Init();
	auiReached.GrowToAtLeastNumElements(pcFunction->NumBlocks(), true, 0);
	auiStack.Init();
	auiStack.Add()[0] = pcFunction->GetEntry()->muiIndex;
	bThrows = false;
	while (!bThrows && auiStack.Pop(&ui))
	{
		if (*auiReached.Get(ui))
		{
			continue;
		}
		*auiReached.Get(ui) = 1;
		pcBlock = pcFunction->GetBlock(ui);
		psJump = &pcBlock->msJump;
		if (psJump->uiKind == IRJ_Throw)
		{
			bThrows = true;
		}
		else if (psJump->uiKind == IRJ_Jump)
		{
			auiStack.Add()[0] = psJump->pcThen->muiIndex;
		}
		else if (psJump->uiKind == IRJ_Branch)
		{
			IRBranchWays(pcModule, pcFunction, pcBlock, &bThen, &bElse);
			if (bThen)
			{
				auiStack.Add()[0] = psJump->pcThen->muiIndex;
			}
			if (bElse)
			{
				auiStack.Add()[0] = psJump->pcElse->muiIndex;
			}
		}
	}
	auiStack.Kill();
	auiReached.Kill();
	return bThrows;
}
