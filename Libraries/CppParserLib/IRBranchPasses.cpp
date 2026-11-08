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
#include "BaseLib/ArrayTemplate.h"
#include "IRBranchPasses.h"


//The passes over a function's branches that the 65C64's and the W65C816's text passes share (CIR65Peephole,
//CIR816Backend::PeepholeBranches), over the code as a CIRBranchCode.  Target is the label line a branch, a jump
//(or a W65C816 brl) goes to, IR_NO_LINE for none.  IsEntry is a line run however the lines before it end: the
//function's label, a label something other than a branch names (a jump table), data.  Retarget sends a branch or
//jump to another label with its condition; Invert sends a branch there with the opposite condition, false if it
//has none.


//////////////////////////////////////////////////////////////////////////
//
// The first line from uiLine on that runs (not a label, blank or taken
// out); NumLines if there is none.
//
//////////////////////////////////////////////////////////////////////////
size IRNextInstruction(CIRBranchCode* pcCode, size uiLine)
{
	size		uiNum;
	EIRCodeFlow	eFlow;

	uiNum = pcCode->NumLines();
	for (; uiLine < uiNum; uiLine++)
	{
		eFlow = pcCode->Flow(uiLine);
		if ((eFlow != IRCF_Label) && (eFlow != IRCF_None))
		{
			return uiLine;
		}
	}
	return uiNum;
}


//////////////////////////////////////////////////////////////////////////
//
// A branch or jump (or a brl) to a label whose code is a bra goes where
// that bra goes, through IR_BRANCH_HOPS bras at most.
//
//////////////////////////////////////////////////////////////////////////
bool IRThreadJumps(CIRBranchCode* pcCode)
{
	size		ui;
	size		uiNum;
	size		uiTarget;
	size		uiNext;
	EIRCodeFlow	eFlow;
	int			iHops;
	bool		bChanged;

	bChanged = false;
	uiNum = pcCode->NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		eFlow = pcCode->Flow(ui);
		uiTarget = pcCode->Target(ui);
		if (((eFlow != IRCF_Branch) && (eFlow != IRCF_Jump) && (eFlow != IRCF_End)) || (uiTarget == IR_NO_LINE))
		{
			continue;
		}
		for (iHops = 0; iHops < IR_BRANCH_HOPS; iHops++)
		{
			uiNext = IRNextInstruction(pcCode, uiTarget);
			if ((uiNext >= uiNum) || (pcCode->Flow(uiNext) != IRCF_Jump) || (pcCode->Target(uiNext) == IR_NO_LINE) || (pcCode->Target(uiNext) == uiTarget))
			{
				break;
			}
			uiTarget = pcCode->Target(uiNext);
		}
		if (uiTarget != pcCode->Target(ui))
		{
			bChanged |= pcCode->Retarget(ui, uiTarget);
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A branch or jump to a label with nothing that runs between it and the
// label goes.
//
//////////////////////////////////////////////////////////////////////////
bool IRRemoveBranchesToNext(CIRBranchCode* pcCode)
{
	size		ui;
	size		uiNum;
	size		uiTarget;
	EIRCodeFlow	eFlow;
	bool		bChanged;

	bChanged = false;
	uiNum = pcCode->NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		eFlow = pcCode->Flow(ui);
		if ((eFlow != IRCF_Branch) && (eFlow != IRCF_Jump))
		{
			continue;
		}
		uiTarget = pcCode->Target(ui);
		if ((uiTarget != IR_NO_LINE) && (uiTarget > ui) && (IRNextInstruction(pcCode, ui + 1) >= uiTarget))
		{
			pcCode->Remove(ui);
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
//			bras z, L1			->			brac z, L2
//			bra L2
//	L1:								L1:
//
// (when nothing else goes to the bra: no label between).
//
//////////////////////////////////////////////////////////////////////////
bool IRInvertBranches(CIRBranchCode* pcCode)
{
	size	ui;
	size	uiNum;
	size	uiJump;
	size	uiTarget;
	size	uiJumpTarget;
	bool	bChanged;

	bChanged = false;
	uiNum = pcCode->NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		if (pcCode->Flow(ui) != IRCF_Branch)
		{
			continue;
		}
		for (uiJump = ui + 1; (uiJump < uiNum) && (pcCode->Flow(uiJump) == IRCF_None); uiJump++)
		{
		}
		if ((uiJump >= uiNum) || (pcCode->Flow(uiJump) != IRCF_Jump))
		{
			continue;
		}
		uiTarget = pcCode->Target(ui);
		uiJumpTarget = pcCode->Target(uiJump);
		if ((uiTarget == IR_NO_LINE) || (uiJumpTarget == IR_NO_LINE) || (uiTarget <= uiJump) || (IRNextInstruction(pcCode, uiJump + 1) < uiTarget) || (uiJumpTarget == uiTarget))
		{
			continue;
		}
		if (pcCode->Invert(ui, uiJumpTarget))
		{
			pcCode->Remove(uiJump);
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out the lines (labels too) that nothing reaches from the entries
// (IsEntry: the function's label first).
//
//////////////////////////////////////////////////////////////////////////
bool IRRemoveUnreached(CIRBranchCode* pcCode)
{
	size					ui;
	size					uiNum;
	size					uiTarget;
	EIRCodeFlow				eFlow;
	CArrayTemplate<uint8>	auiReached;
	CArrayTemplate<size>	auiStack;
	bool					bChanged;

	uiNum = pcCode->NumLines();
	auiReached.Init();
	auiReached.GrowToAtLeastNumElements(uiNum, true, 0);
	auiStack.Init();
	for (ui = uiNum; ui > 0; ui--)
	{
		if (pcCode->IsEntry(ui - 1))
		{
			auiStack.Add()[0] = ui - 1;
		}
	}
	while (auiStack.Pop(&ui))
	{
		for (; (ui < uiNum) && !*auiReached.Get(ui); ui++)
		{
			*auiReached.Get(ui) = 1;
			uiTarget = pcCode->Target(ui);
			if (uiTarget != IR_NO_LINE)
			{
				auiStack.Add()[0] = uiTarget;
			}
			eFlow = pcCode->Flow(ui);
			if ((eFlow == IRCF_Jump) || (eFlow == IRCF_End))
			{
				break;
			}
		}
	}
	auiStack.Kill();

	bChanged = false;
	for (ui = 0; ui < uiNum; ui++)
	{
		if (!*auiReached.Get(ui) && (pcCode->Flow(ui) != IRCF_None))
		{
			pcCode->Remove(ui);
			bChanged = true;
		}
	}
	auiReached.Kill();
	return bChanged;
}
