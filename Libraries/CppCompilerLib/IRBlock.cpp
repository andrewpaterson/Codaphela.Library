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
#include "IRBlock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::Init(uint32* puiSource, const char* szName)
{
	mpuiSource = puiSource;
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
const char* CIRBlock::GetName(void) const
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

	//NULL if the block can't hold another (its array would pass its size, which it reports).
	psInstruction = masInstructions.Add();
	if (!psInstruction)
	{
		return NULL;
	}
	memset(psInstruction, 0, sizeof(SIRInstruction));
	psInstruction->uiOpcode = uiOpcode;
	psInstruction->sType = sType;
	psInstruction->sResult = sResult;
	psInstruction->asArguments[0] = sArgument0;
	psInstruction->asArguments[1] = sArgument1;
	psInstruction->uiSource = *mpuiSource;
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
// The phi whose result is sResult, else NULL.  A pointer to it lasts only
// until a phi is added to the block.
//
//////////////////////////////////////////////////////////////////////////
SIRPhi* CIRBlock::FindPhi(SIRReference sResult)
{
	size	ui;
	SIRPhi*	psPhi;

	for (ui = 0; ui < masPhis.NumElements(); ui++)
	{
		psPhi = masPhis.Get(ui);
		if (psPhi->sResult.Equals(sResult))
		{
			return psPhi;
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
	msJump.uiSource = *mpuiSource;
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
// The function is left with an exception (the exception itself is the runtime's: the IR only says which way out).
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::Throw(void)
{
	SetJump(IRJ_Throw, IRType(IRTK_None), IRNone(), NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRBlock::HasJump(void) const
{
	return msJump.uiKind != IRJ_None;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRBlock::NumSuccessors(void) const
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
// True if one of the block's phis or instructions makes sValue.
//
//////////////////////////////////////////////////////////////////////////
bool CIRBlock::Defines(SIRReference sValue)
{
	size	ui;

	for (ui = 0; ui < masPhis.NumElements(); ui++)
	{
		if (masPhis.Get(ui)->sResult.Equals(sValue))
		{
			return true;
		}
	}
	for (ui = 0; ui < masInstructions.NumElements(); ui++)
	{
		if (masInstructions.Get(ui)->sResult.Equals(sValue))
		{
			return true;
		}
	}
	return false;
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


//////////////////////////////////////////////////////////////////////////
//
// Stores, args, calls, a target's instructions (none are IROP_Pure) and
// volatile loads do something besides make a value.
//
//////////////////////////////////////////////////////////////////////////
bool IRHasEffect(SIRInstruction* psInstruction)
{
	if (psInstruction->uiOpcode == IRO_Load)
	{
		return (psInstruction->uiFlags & IRF_Volatile) != 0;
	}
	return !IsIRPure(psInstruction->uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
// What psJump says going to pcTo, psBranchedOn being the instruction that
// defines what it branches on (NULL if none does).  False unless it is a
// branch two ways, one of them to pcTo, on an integer comparison.
//
//////////////////////////////////////////////////////////////////////////
bool SIREdgeCondition::Init(SIRJump* psJump, CIRBlock* pcTo, SIRInstruction* psBranchedOn)
{
	if ((psJump->uiKind != IRJ_Branch) || (psJump->pcThen == psJump->pcElse) || ((psJump->pcThen != pcTo) && (psJump->pcElse != pcTo)) ||
		!psBranchedOn || !IsIRIntegerComparison(psBranchedOn->uiOpcode))
	{
		return false;
	}
	psCompare = psBranchedOn;
	uiOpcode = (psJump->pcThen == pcTo) ? psBranchedOn->uiOpcode : IRInvertedComparison(psBranchedOn->uiOpcode);
	sLeft = psBranchedOn->asArguments[0];
	sRight = psBranchedOn->asArguments[1];
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The same condition written the other way round: a < b is b > a.
//
//////////////////////////////////////////////////////////////////////////
void SIREdgeCondition::Swap(void)
{
	SIRReference	sWasLeft;

	sWasLeft = sLeft;
	sLeft = sRight;
	sRight = sWasLeft;
	uiOpcode = IRSwappedComparison(uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the condition with sValue on the left.  False if it compares
// sValue with nothing.
//
//////////////////////////////////////////////////////////////////////////
bool SIREdgeCondition::PutLeft(SIRReference sValue)
{
	if (sLeft.Equals(sValue))
	{
		return true;
	}
	if (!sRight.Equals(sValue))
	{
		return false;
	}
	Swap();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a greater than or a less or equal the other way round (they are
// the comparisons IsIRSwappedComparison names), so that two conditions
// that say the same thing are written the same.
//
//////////////////////////////////////////////////////////////////////////
void SIREdgeCondition::Unswap(void)
{
	if (IsIRSwappedComparison(uiOpcode))
	{
		Swap();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Removes pcPredecessor's arguments from this block's phis.
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::RemovePhiArguments(CIRBlock* pcPredecessor)
{
	size			ui;
	size			uiArgument;
	SIRPhi*			psPhi;

	for (ui = 0; ui < masPhis.NumElements(); ui++)
	{
		psPhi = masPhis.Get(ui);
		for (uiArgument = psPhi->asArguments.NumElements(); uiArgument > 0; uiArgument--)
		{
			if (psPhi->asArguments.Get(uiArgument - 1)->pcBlock == pcPredecessor)
			{
				psPhi->asArguments.RemoveAt(uiArgument - 1, true);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// This block's phis take from pcNew what they took from pcOld: pcNew is the
// block that comes here now in pcOld's place.
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::RenamePhiArguments(CIRBlock* pcOld, CIRBlock* pcNew)
{
	size			ui;
	size			uiArgument;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;

	for (ui = 0; ui < masPhis.NumElements(); ui++)
	{
		psPhi = masPhis.Get(ui);
		for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
		{
			psArgument = psPhi->asArguments.Get(uiArgument);
			if (psArgument->pcBlock == pcOld)
			{
				psArgument->pcBlock = pcNew;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// This block's phis take from pcNew (which comes to it now too) what they take
// from pcFrom.
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::CopyPhiArguments(CIRBlock* pcFrom, CIRBlock* pcNew)
{
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	size			ui;

	for (ui = 0; ui < masPhis.NumElements(); ui++)
	{
		psPhi = masPhis.Get(ui);
		psArgument = FindPhiArgument(psPhi, pcFrom);
		if (psArgument)
		{
			AddPhiArgument(psPhi, pcNew, psArgument->sValue);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// This block's jump goes to pcNew where it went to pcOld.  A brif to one block
// twice becomes a jmp.
//
//////////////////////////////////////////////////////////////////////////
void CIRBlock::RedirectJump(CIRBlock* pcOld, CIRBlock* pcNew)
{
	if (msJump.pcThen == pcOld)
	{
		msJump.pcThen = pcNew;
	}
	if ((msJump.uiKind == IRJ_Branch) && (msJump.pcElse == pcOld))
	{
		msJump.pcElse = pcNew;
	}
	if ((msJump.uiKind == IRJ_Branch) && (msJump.pcThen == msJump.pcElse))
	{
		Jump(msJump.pcThen);
	}
}
