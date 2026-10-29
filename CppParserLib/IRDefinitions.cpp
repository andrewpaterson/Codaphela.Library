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
#include "IRDefinitions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRDefinitions::Init(void)
{
	mpcFunction = NULL;
	masDefinitions.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRDefinitions::Kill(void)
{
	masDefinitions.Kill();
	mpcFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRDefinitions::Compute(CIRFunction* pcFunction)
{
	size			ui;
	size			uiIndex;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRDefinition*	psDefinition;

	mpcFunction = pcFunction;
	masDefinitions.FakeSetUsedElements(0);
	masDefinitions.GrowToAtLeastNumElements(pcFunction->NumTemporaries(), true, 0);
	for (ui = 0; ui < pcFunction->NumTemporaries(); ui++)
	{
		psDefinition = masDefinitions.Get(ui);
		psDefinition->iInstruction = -1;
		psDefinition->iPhi = -1;
	}

	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		pcBlock = pcFunction->GetBlock(ui);
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			psDefinition = Get(psPhi->sResult);
			if (psDefinition)
			{
				psDefinition->pcBlock = pcBlock;
				psDefinition->iPhi = (int32)uiIndex;
			}
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				Use(psPhi->asArguments.Get(uiArgument)->sValue);
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			psDefinition = Get(psInstruction->sResult);
			if (psDefinition)
			{
				psDefinition->pcBlock = pcBlock;
				psDefinition->iInstruction = (int32)uiIndex;
			}
			Use(psInstruction->asArguments[0]);
			Use(psInstruction->asArguments[1]);
		}
		Use(pcBlock->msJump.sArgument);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction that defines sValue; NULL for a constant, a symbol, a
// parameter or a phi.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIRDefinitions::Instruction(SIRReference sValue)
{
	SIRDefinition*	psDefinition;

	psDefinition = Get(sValue);
	if (!psDefinition || !psDefinition->pcBlock || (psDefinition->iInstruction < 0))
	{
		return NULL;
	}
	return psDefinition->pcBlock->masInstructions.Get(psDefinition->iInstruction);
}


//////////////////////////////////////////////////////////////////////////
//
// The phi that defines sValue; NULL if no phi does.
//
//////////////////////////////////////////////////////////////////////////
SIRPhi* CIRDefinitions::Phi(SIRReference sValue)
{
	SIRDefinition*	psDefinition;

	psDefinition = Get(sValue);
	if (!psDefinition || !psDefinition->pcBlock || (psDefinition->iPhi < 0))
	{
		return NULL;
	}
	return psDefinition->pcBlock->masPhis.Get(psDefinition->iPhi);
}


//////////////////////////////////////////////////////////////////////////
//
// The block that defines sValue; NULL for a constant, a symbol or a
// parameter.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRDefinitions::Block(SIRReference sValue)
{
	SIRDefinition*	psDefinition;

	psDefinition = Get(sValue);
	return psDefinition ? psDefinition->pcBlock : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRDefinitions::NumUses(SIRReference sValue)
{
	SIRDefinition*	psDefinition;

	psDefinition = Get(sValue);
	return psDefinition ? psDefinition->uiUses : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// True if an instruction with uiOpcode defines sValue.
//
//////////////////////////////////////////////////////////////////////////
bool CIRDefinitions::IsDefinedBy(SIRReference sValue, uint16 uiOpcode)
{
	SIRInstruction*	psInstruction;

	psInstruction = Instruction(sValue);
	return psInstruction && (psInstruction->uiOpcode == uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if sValue isn't one of the function's temporaries.
//
//////////////////////////////////////////////////////////////////////////
SIRDefinition* CIRDefinitions::Get(SIRReference sValue)
{
	if (!sValue.IsTemporary() || (sValue.Index() >= masDefinitions.NumElements()))
	{
		return NULL;
	}
	return masDefinitions.Get(sValue.Index());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRDefinitions::Use(SIRReference sValue)
{
	SIRDefinition*	psDefinition;

	psDefinition = Get(sValue);
	if (psDefinition)
	{
		psDefinition->uiUses++;
	}
}
