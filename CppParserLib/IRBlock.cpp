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
#include "IRBlock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::Init(const char* szName)
{
	if (szName)
	{
		mszName.Init(szName);
	}
	else
	{
		mszName.Init();
	}
	masPhis.Init();
	masInstructions.Init();
	memset(&msJump, 0, sizeof(SIRJump));
	mapcPredecessors.Init();
	muiIndex = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::Kill(void)
{
	size	ui;

	mapcPredecessors.Kill();
	masInstructions.Kill();
	for (ui = 0; ui < masPhis.NumElements(); ui++)
	{
		masPhis.Get(ui)->asArguments.Kill();
	}
	masPhis.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIRBlock::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIRBlock::AddInstruction(uint16 uiOpcode, SIRType sType, SIRReference sResult, SIRReference sArgument0, SIRReference sArgument1)
{
	SIRInstruction*	psInstruction;

	psInstruction = masInstructions.Add();
	memset(psInstruction, 0, sizeof(SIRInstruction));
	psInstruction->uiOpcode = uiOpcode;
	psInstruction->sType = sType;
	psInstruction->sResult = sResult;
	psInstruction->asArguments[0] = sArgument0;
	psInstruction->asArguments[1] = sArgument1;
	return psInstruction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRPhi* CIRBlock::AddPhi(SIRType sType, SIRReference sResult)
{
	SIRPhi*	psPhi;

	psPhi = masPhis.Add();
	psPhi->sResult = sResult;
	psPhi->sType = sType;
	psPhi->asArguments.Init();
	return psPhi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::AddPhiArgument(SIRPhi* psPhi, CIRBlock* pcBlock, SIRReference sValue)
{
	SIRPhiArgument*	psArgument;

	psArgument = psPhi->asArguments.Add();
	psArgument->pcBlock = pcBlock;
	psArgument->sValue = sValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRPhiArgument* CIRBlock::FindPhiArgument(SIRPhi* psPhi, CIRBlock* pcBlock)
{
	size			ui;
	SIRPhiArgument*	psArgument;

	for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
	{
		psArgument = psPhi->asArguments.Get(ui);
		if (psArgument->pcBlock == pcBlock)
		{
			return psArgument;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::SetJump(EIRJumpKind eKind, SIRType sType, SIRReference sArgument, CIRBlock* pcThen, CIRBlock* pcElse)
{
	msJump.uiKind = (uint8)eKind;
	msJump.sType = sType;
	msJump.sArgument = sArgument;
	msJump.pcThen = pcThen;
	msJump.pcElse = pcElse;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::Jump(CIRBlock* pcTarget)
{
	SetJump(IRJ_Jump, IRType(IRTK_None), IRNone(), pcTarget, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::Branch(SIRReference sCondition, CIRBlock* pcThen, CIRBlock* pcElse)
{
	SetJump(IRJ_Branch, IRType(IRTK_None), sCondition, pcThen, pcElse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::Return(SIRType sType, SIRReference sValue)
{
	SetJump(IRJ_Return, sType, sValue, NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::Return(void)
{
	SetJump(IRJ_Return, IRType(IRTK_None), IRNone(), NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::Unreachable(void)
{
	SetJump(IRJ_Unreachable, IRType(IRTK_None), IRNone(), NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRBlock::HasJump(void)
{
	return msJump.uiKind != IRJ_None;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRBlock::NumSuccessors(void)
{
	if (msJump.uiKind == IRJ_Jump)
	{
		return 1;
	}
	else if (msJump.uiKind == IRJ_Branch)
	{
		return 2;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRBlock::GetSuccessor(size uiIndex)
{
	return uiIndex == 0 ? msJump.pcThen : msJump.pcElse;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRBlock::HasPredecessor(CIRBlock* pcBlock)
{
	return mapcPredecessors.Find(pcBlock) != ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// The address a load reads or a store writes; IRNone for other
// instructions.
//
//////////////////////////////////////////////////////////////////////////
SIRReference IRAddressOf(SIRInstruction* psInstruction)
{
	if (psInstruction->uiOpcode == IRO_Load)
	{
		return psInstruction->asArguments[0];
	}
	if (psInstruction->uiOpcode == IRO_Store)
	{
		return psInstruction->asArguments[1];
	}
	return IRNone();
}
