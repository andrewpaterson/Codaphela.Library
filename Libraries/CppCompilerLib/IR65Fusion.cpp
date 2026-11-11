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
#include "IR65BackendInternal.h"


//The 65C64 back end's fused instructions: which instructions are left for the next one to write (CanFuse, Fuse),
//sign extended loads, and loads deferred into the next instruction's memory operand (CanDefer).


//////////////////////////////////////////////////////////////////////////
//
// Whether instruction uiIndex of pcBlock isn't written in its turn, as the
// W65C816 back end's CanFuse: the next instruction written (its consumer,
// WriteInstruction) writes it.  A load it reads as a memory operand
// (CanDefer), a narrow load its sext sign extends (ExtendOf), a div its
// partner gives the other result of (PairedDivide), or a result only the
// consumer uses, which it takes from A (bInA, MarkInA).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::CanFuse(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIRInstruction*	psPartner;
	SIR65Fused*		psLast;

	psLast = (masFused.NumElements() > 0) ? masFused.Get(masFused.NumElements() - 1) : NULL;
	if (psLast && ((psLast->uiKind == IR65FU_Extended) || (psLast->uiKind == IR65FU_Paired)))
	{
		//The sext or partner that writes the instruction before it is written in its turn.
		return false;
	}
	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	if (psInstruction->uiOpcode == IRO_Load)
	{
		//PureMemoryOperand asks what A will hold when the consumer starts.
		msAvailable = InAAfterFused();
		if (CanDefer(pcBlock, uiIndex))
		{
			Fuse(pcBlock, uiIndex, IR65FU_Deferred);
			return true;
		}
		if (ExtendOf(pcBlock, uiIndex))
		{
			Fuse(pcBlock, uiIndex, IR65FU_Extended);
			return true;
		}
	}
	psPartner = PairedDivide(pcBlock, uiIndex);
	if (psPartner && !Skipped(psPartner))
	{
		Fuse(pcBlock, uiIndex, IR65FU_Paired);
		return true;
	}
	if (psInstruction->sResult.IsTemporary() && PlaceOf(psInstruction->sResult)->bInA)
	{
		Fuse(pcBlock, uiIndex, IR65FU_InA);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Fuse(CIRBlock* pcBlock, size uiIndex, uint8 uiKind)
{
	SIR65Fused*	psFused;

	psFused = masFused.Add();
	psFused->psInstruction = pcBlock->masInstructions.Get(uiIndex);
	psFused->uiIndex = uiIndex;
	psFused->uiKind = uiKind;
}


//////////////////////////////////////////////////////////////////////////
//
// What A holds when the consumer of the instructions CanFuse has left
// starts: the last one taken from A, else what the last instruction
// written left there (a deferred load doesn't change A).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIR65Backend::InAAfterFused(void)
{
	size	ui;

	for (ui = masFused.NumElements(); ui > 0; ui--)
	{
		if (masFused.Get(ui - 1)->uiKind == IR65FU_InA)
		{
			return masFused.Get(ui - 1)->psInstruction->sResult;
		}
	}
	return msInA;
}


//////////////////////////////////////////////////////////////////////////
//
// The sext just after the narrow load uiLoad when it is the load's only
// use: the load is sign extended as it is widened, giving the sext's result
// (Load).  NULL for none.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR65Backend::ExtendOf(CIRBlock* pcBlock, size uiLoad)
{
	SIRInstruction*	psLoad;
	SIRInstruction*	psExtend;

	psLoad = pcBlock->masInstructions.Get(uiLoad);
	psExtend = pcBlock->masInstructions.SafeGet(uiLoad + 1);
	if ((psLoad->uiOpcode != IRO_Load) || (ValueBytes(psLoad->sType) >= 4) || (psExtend == NULL) || (psExtend->uiOpcode != IRO_SExt) || !psExtend->asArguments[0].Equals(psLoad->sResult) ||
		!psLoad->sResult.IsTemporary() || (*mauiUses.Get(psLoad->sResult.Index()) != 1) || IsWide(psExtend->sType) || Skipped(psExtend))
	{
		return NULL;
	}
	return psExtend;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a 4 byte load left in A for the next instruction can be deferred
// (IR65FU_Deferred): not written, its address needing no instructions and
// using no register the next instruction changes before reading it, so the
// next instruction reads memory itself (add A, [8 + X]; cpy Y, [[4 + W]]).
// Not before mul, div or ashr, nor just after another deferred load.  Sets
// mszDeferred.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::CanDefer(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psLoad;
	SIRInstruction*	psNext;
	size			uiNext;
	const char*		szWork;
	char			cRegister;
	SIRReference	sResult;
	CChars			szOperand;

	psLoad = pcBlock->masInstructions.Get(uiIndex);
	sResult = psLoad->sResult;
	if (((masFused.NumElements() > 0) && (masFused.Get(masFused.NumElements() - 1)->uiKind == IR65FU_Deferred)) || !sResult.IsTemporary() || !PlaceOf(sResult)->bInA || (psLoad->uiFlags & IRF_Volatile) || (ValueBytes(psLoad->sType) != 4))
	{
		return false;
	}
	uiNext = NextWritten(pcBlock, uiIndex, pcBlock->masInstructions.NumElements());
	psNext = pcBlock->masInstructions.SafeGet(uiNext);
	if (psNext)
	{
		if (IR65OpcodeFlags(psNext->uiOpcode) & IR65F_NO_DEFER)
		{
			return false;
		}
		szWork = WorkRegister(psNext);
	}
	else
	{
		szWork = "A";
	}

	szOperand.Init();
	cRegister = '\0';
	if (!PureMemoryOperand(psLoad->asArguments[0], &szOperand, &cRegister) || (cRegister == szWork[0]))
	{
		szOperand.Kill();
		return false;
	}
	mszDeferred.Kill();
	mszDeferred.Init(&szOperand);
	szOperand.Kill();
	return true;
}
