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
#include <stdlib.h>
#include <string.h>
#include "IR816BackendInternal.h"


//The W65C816 back end's passes over a function's blocks of assembly text (IR816BuildFlow): stores dead on every
//way on (RemoveFlowDeadStores) and loads of what a register holds across labels (PeepholeFlowRegisters).


//The direct page bytes RemoveFlowDeadStores tracks, a bit each.
struct SIR816LiveBytes
{
	uint64	aulli[IR816_DIRECT_PAGE / 64];
};


//What a line of the code is to RemoveFlowDeadStores.
#define IR816FL_OTHER			0	//A comment, a blank line or a directive.
#define IR816FL_LABEL			1
#define IR816FL_INSTRUCTION		2


//A line of the code RemoveFlowDeadStores works on.
struct SIR816FlowLine
{
	const char*	szLine;
	const char*	szEnd;
	SIR816Line	sLine;
	int			iKind;		//IR816FL_.
	int			iBytes;		//A direct page store's bytes: 2, 1, or 0 when the width isn't known here.
	bool		bRuntime;	//A call to a runtime routine: it reads the scratch bytes.
	bool		bPointer;	//Reads through the pointer at [[D + iPointer]...]: its three bytes.
	int32		iPointer;
	bool		bFrameAddress;	//tdc, clc, adc iFrame: a pointer to the frame at D + iFrame, through which any byte from there may be reached.
	bool		bIndexedFrame;	//A 64 bit shift routine's call, on the eight bytes at D + iFrame (X, from the ldx before it).
	int32		iFrame;
};


//A block of RemoveFlowDeadStores' lines: from a label, or after a line that may go elsewhere, to the next.
struct SIR816FlowBlock
{
	size			uiFirst;
	size			uiEnd;
	int				aiNext[2];	//The blocks it goes on to, -1 for none.
	bool			bLeavesAll;	//It leaves the code with every byte live (a tail call, stp, the code's end).
	bool			bEntered;	//It may be gone to from outside the code (a function's entry, an indirect call's return).
	SIR816LiveBytes	sIn;
	SIR816LiveBytes	sOut;
};


//////////////////////////////////////////////////////////////////////////
//
// iBytes bytes from iByte set live (bLive) or dead, or whether any is.
//
//////////////////////////////////////////////////////////////////////////
static void IR816SetLive(SIR816LiveBytes* psLive, int32 iByte, int iBytes, bool bLive)
{
	int		i;
	int32	iBit;

	for (i = 0; i < iBytes; i++)
	{
		iBit = iByte + i;
		if ((iBit < 0) || (iBit >= IR816_DIRECT_PAGE))
		{
			continue;
		}
		if (bLive)
		{
			psLive->aulli[iBit / 64] |= (uint64)1 << (iBit % 64);
		}
		else
		{
			psLive->aulli[iBit / 64] &= ~((uint64)1 << (iBit % 64));
		}
	}
}


static bool IR816AnyLive(SIR816LiveBytes* psLive, int32 iByte, int iBytes)
{
	int		i;
	int32	iBit;

	for (i = 0; i < iBytes; i++)
	{
		iBit = iByte + i;
		if ((iBit >= 0) && (iBit < IR816_DIRECT_PAGE) && (psLive->aulli[iBit / 64] & ((uint64)1 << (iBit % 64))))
		{
			return true;
		}
	}
	return false;
}


static bool IR816NoneLive(SIR816LiveBytes* psLive)
{
	size	ui;

	for (ui = 0; ui < IR816_DIRECT_PAGE / 64; ui++)
	{
		if (psLive->aulli[ui] != 0)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes live before the line, from those live after it: a direct page
// store's bytes are dead (but only the low one when its width isn't known),
// a read's ([D + n]: two, [[D + n]:24 ...]: the pointer's three, a runtime
// routine: the scratch bytes) live.  Before the direct page moves (pld, tcd)
// none are live if none are after, else all are (the bytes after are
// another frame's).
//
//////////////////////////////////////////////////////////////////////////
static void IR816FlowBack(SIR816FlowLine* psLine, SIR816LiveBytes* psLive)
{
	if (psLine->iKind != IR816FL_INSTRUCTION)
	{
		return;
	}
	if (psLine->sLine.uiFlags & IR816MF_SETS_D)
	{
		if (!IR816NoneLive(psLive))
		{
			memset(psLive, 0xFF, sizeof(SIR816LiveBytes));
		}
		return;
	}
	if (psLine->bRuntime)
	{
		IR816SetLive(psLive, IR816_SCRATCH_FIRST, IR816_SCRATCH_END - IR816_SCRATCH_FIRST, true);
		if (psLine->bIndexedFrame)
		{
			IR816SetLive(psLive, psLine->iFrame, 8, true);
		}
		return;
	}
	if (psLine->bPointer)
	{
		IR816SetLive(psLive, psLine->iPointer, 3, true);
	}
	if (psLine->sLine.bDirect)
	{
		if (psLine->sLine.uiFlags & IR816MF_STORE)
		{
			IR816SetLive(psLive, psLine->sLine.iOffset, (psLine->iBytes == 2) ? 2 : 1, false);
		}
		else
		{
			IR816SetLive(psLive, psLine->sLine.iOffset, 2, true);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operand is exactly [D + n], or starts [[D + n] (a pointer's),
// with n in *piOffset; any other mention of the direct page ([D + X],
// [D + n + X], [D + n + m]) is neither.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816PlainDirect(const char* szOperand, size uiOperand, size uiFrom, int32* piOffset)
{
	size	ui;

	if ((uiOperand < uiFrom + 6) || (strncmp(&szOperand[uiFrom], "[D + ", 5) != 0))
	{
		return false;
	}
	for (ui = uiFrom + 5; (ui < uiOperand) && (szOperand[ui] >= '0') && (szOperand[ui] <= '9'); ui++)
	{
	}
	if ((ui == uiFrom + 5) || (ui >= uiOperand) || (szOperand[ui] != ']'))
	{
		return false;
	}
	if ((uiFrom == 0) && (ui + 1 != uiOperand))
	{
		return false;
	}
	*piOffset = atoi(&szOperand[uiFrom + 5]);
	return true;
}


//A label of the code (IR816BuildFlow): its name (uiLength characters, without the ':') and its line.
struct SIR816FlowLabel
{
	const char*	szLabel;
	size		uiLength;
	int			iLine;
};


//////////////////////////////////////////////////////////////////////////
//
// Orders labels by name (then line, so the first of a name comes first).
//
//////////////////////////////////////////////////////////////////////////
static int IR816CompareFlowLabels(const void* pv1, const void* pv2)
{
	const SIR816FlowLabel*	ps1;
	const SIR816FlowLabel*	ps2;
	int						iResult;

	ps1 = (const SIR816FlowLabel*)pv1;
	ps2 = (const SIR816FlowLabel*)pv2;
	iResult = memcmp(ps1->szLabel, ps2->szLabel, (ps1->uiLength < ps2->uiLength) ? ps1->uiLength : ps2->uiLength);
	if (iResult == 0)
	{
		iResult = (ps1->uiLength < ps2->uiLength) ? -1 : ((ps1->uiLength > ps2->uiLength) ? 1 : 0);
	}
	if (iResult == 0)
	{
		iResult = ps1->iLine - ps2->iLine;
	}
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The first line labelled szLabel (uiLabel characters), or -1, in pasLabels
// (sorted by IR816CompareFlowLabels).
//
//////////////////////////////////////////////////////////////////////////
static int IR816FindFlowLabel(CArrayTemplate<SIR816FlowLabel>* pasLabels, const char* szLabel, size uiLabel)
{
	size				uiLow;
	size				uiHigh;
	size				uiMiddle;
	SIR816FlowLabel*	psLabel;
	int					iResult;

	uiLow = 0;
	uiHigh = pasLabels->NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		psLabel = pasLabels->Get(uiMiddle);
		iResult = memcmp(psLabel->szLabel, szLabel, (psLabel->uiLength < uiLabel) ? psLabel->uiLength : uiLabel);
		if (iResult == 0)
		{
			iResult = (psLabel->uiLength < uiLabel) ? -1 : ((psLabel->uiLength > uiLabel) ? 1 : 0);
		}
		if (iResult < 0)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	if (uiLow < pasLabels->NumElements())
	{
		psLabel = pasLabels->Get(uiLow);
		if ((psLabel->uiLength == uiLabel) && (memcmp(psLabel->szLabel, szLabel, uiLabel) == 0))
		{
			return psLabel->iLine;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the tdc at szLine takes a pointer to the frame (clc, adc n with
// no tcs after: not IR816MovesS), with n in *piFrame.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816FrameAddress(const char* szLine, int32* piFrame)
{
	SIR816Line	sLine;

	szLine = IR816NextLine(szLine);
	if (!szLine || !IR816IsInstruction(szLine))
	{
		return false;
	}
	IR816Decode(szLine, &sLine);
	if (sLine.uiMnemonic != IR816M_Clc)
	{
		return false;
	}
	szLine = IR816NextLine(szLine);
	if (!szLine || !IR816IsInstruction(szLine))
	{
		return false;
	}
	IR816Decode(szLine, &sLine);
	if ((sLine.uiMnemonic != IR816M_Adc) || (sLine.uiOperand == 0) || (sLine.szOperand[0] < '0') || (sLine.szOperand[0] > '9'))
	{
		return false;
	}
	*piFrame = atoi(sLine.szOperand);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operand is exactly [D + X] or [D + n + X], with n (or 0) in
// *piOffset.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IndexedDirect(const char* szOperand, size uiOperand, int32* piOffset)
{
	size	ui;

	if ((uiOperand == 7) && (strncmp(szOperand, "[D + X]", 7) == 0))
	{
		*piOffset = 0;
		return true;
	}
	if ((uiOperand < 11) || (strncmp(szOperand, "[D + ", 5) != 0) || (strncmp(&szOperand[uiOperand - 5], " + X]", 5) != 0))
	{
		return false;
	}
	for (ui = 5; ui < uiOperand - 5; ui++)
	{
		if ((szOperand[ui] < '0') || (szOperand[ui] > '9'))
		{
			return false;
		}
	}
	*piOffset = atoi(&szOperand[5]);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The constant an ldx (psLine, the instruction before the one using X)
// loads into X, or -1.
//
//////////////////////////////////////////////////////////////////////////
static int32 IR816LoadedX(SIR816FlowLine* psLine)
{
	size	ui;

	if (!psLine || (psLine->sLine.uiMnemonic != IR816M_Ldx) || (psLine->sLine.uiOperand == 0))
	{
		return -1;
	}
	for (ui = 0; ui < psLine->sLine.uiOperand; ui++)
	{
		if ((psLine->sLine.szOperand[ui] < '0') || (psLine->sLine.szOperand[ui] > '9'))
		{
			return -1;
		}
	}
	return atoi(psLine->sLine.szOperand);
}


//////////////////////////////////////////////////////////////////////////
//
// The function's code from szText as lines and blocks (from a label, or
// after a line that may go elsewhere, to the next; a call comes back) and
// where each block goes.  False if it can't be followed (a jump table, an
// unknown instruction or jump, a branch to a label it doesn't have), or,
// with bStores, if the direct page might be reached other than as [D + n]
// and through pointers at [D + n] (as IR816FindReads: D's value pushed or
// taken other than as a pointer into the frame, the direct page indexed by
// an X not loaded with a constant just before, frame operand calls), when
// what writes and reads it can't be known.  A pointer into the frame
// (bFrameAddress) and [D + n + X] after ldx m ([D + n + m], or past the
// direct page) are known, as are an indexed runtime routine's bytes
// (bIndexedFrame, after ldx m).  pasLabels (empty)
// gets the code's labels, sorted to find where a branch goes.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816BuildFlowLabelled(const char* szText, bool bStores, CArrayTemplate<SIR816FlowLine>* pasLines, CArrayTemplate<SIR816FlowBlock>* pasBlocks, CArrayTemplate<int>* paiBlockOf, CArrayTemplate<SIR816FlowLabel>* pasLabels)
{
	SIR816FlowLabel*	psLabel;
	SIR816FlowLine*		psLine;
	SIR816FlowLine*		psLast;
	SIR816FlowLine*		psPrevious;
	SIR816FlowBlock*	psBlock;
	const char*			szLine;
	const char*			szEnd;
	const char*			szOperand;
	size				ui;
	size				uj;
	size				uiLabel;
	int					iTarget;
	int					iWidthA;
	int					iWidthX;
	int					i;
	int32				iX;
	bool				bFrame;
	bool				bIndexed;
	int					iBefore;	//The instruction before's line, with no label between, or -1.

	//The lines, what each reads and writes, and the widths.
	iWidthA = 16;
	iWidthX = 16;
	bFrame = false;
	iBefore = -1;
	for (szLine = szText; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		psLine = pasLines->Add();
		memset(psLine, 0, sizeof(SIR816FlowLine));
		psLine->szLine = szLine;
		psLine->szEnd = szEnd;
		psLine->iKind = IR816FL_OTHER;
		if ((szLine[0] != '\t') && (szEnd > szLine + 1) && (szEnd[-1] == ':'))
		{
			psLine->iKind = IR816FL_LABEL;
			iWidthA = 0;
			iWidthX = 0;
			iBefore = -1;
			continue;
		}
		if ((szLine[0] == '\t') && (szLine[1] == '\t') && (szLine[2] == '.'))
		{
			if (strncmp(szLine, "\t\t.width (A, X), 16", 19) == 0)
			{
				iWidthA = 16;
				iWidthX = 16;
			}
			else if (strncmp(szLine, "\t\t.width A, 8", 13) == 0)
			{
				iWidthA = 8;
			}
			else if (strncmp(szLine, "\t\t.width", 8) == 0)
			{
				iWidthA = 0;
				iWidthX = 0;
			}
			continue;
		}
		if (!IR816IsInstruction(szLine) || (szLine[2] == ';') || (szEnd - szLine < 3))
		{
			continue;
		}
		psLine->iKind = IR816FL_INSTRUCTION;
		IR816Decode(szLine, &psLine->sLine);
		szOperand = psLine->sLine.szOperand;
		if ((psLine->sLine.uiMnemonic == IR816M_Unknown) || (psLine->sLine.uiFlags & IR816MF_INTERRUPT))
		{
			return false;
		}

		//D's value pushed once it is the frame's (phd) may be passed on as a pointer, and taken (tdc, but to move S) as
		//one unless it is a pointer into the frame (clc, adc n): the bytes from n on may be reached through that.
		if (bStores && (psLine->sLine.uiMnemonic == IR816M_Phd) && bFrame)
		{
			return false;
		}
		if (bStores && (psLine->sLine.uiMnemonic == IR816M_Tdc) && !IR816MovesS(szLine))
		{
			if (!IR816FrameAddress(szLine, &psLine->iFrame))
			{
				return false;
			}
			psLine->bFrameAddress = true;
		}
		if (psLine->sLine.uiMnemonic == IR816M_Tcd)
		{
			bFrame = true;
		}
		if (psLine->sLine.uiFlags & IR816MF_CALL)
		{
			if (bStores && (psLine->sLine.uiOperand > 3) && (strncmp(&szOperand[psLine->sLine.uiOperand - 3], IR816_FRAME_OPERANDS, 3) == 0))
			{
				return false;
			}
			if (strncmp(szOperand, IR816_RUNTIME_PREFIX, IR816_RUNTIME_PREFIX_LENGTH) == 0)
			{
				if (bStores && IR816IndexedRoutine(szOperand, psLine->sLine.uiOperand))
				{
					psLine->iFrame = IR816LoadedX((iBefore >= 0) ? pasLines->Get(iBefore) : NULL);
					if (psLine->iFrame < 0)
					{
						return false;
					}
					psLine->bIndexedFrame = true;
				}
				psLine->bRuntime = true;
			}
		}

		//[D + n + X] after ldx m is [D + n + m]: a frame place in the direct page, or past it (big frames).
		bIndexed = bStores && IR816IndexedDirect(szOperand, psLine->sLine.uiOperand, &iX);
		if (bIndexed)
		{
			if (IR816LoadedX((iBefore >= 0) ? pasLines->Get(iBefore) : NULL) < 0)
			{
				return false;
			}
			iX += IR816LoadedX(pasLines->Get(iBefore));
			psLine->sLine.bDirect = iX < IR816_DIRECT_PAGE;
			psLine->sLine.iOffset = iX;
			if (psLine->sLine.bDirect && (psLine->sLine.uiFlags & IR816MF_STORE))
			{
				i = ((psLine->sLine.uiMnemonic == IR816M_Stx) || (psLine->sLine.uiMnemonic == IR816M_Sty)) ? iWidthX : iWidthA;
				psLine->iBytes = (i == 16) ? 2 : (i == 8) ? 1 : 0;
			}
		}

		//The direct page only as [D + n] or a pointer there.
		else if (psLine->sLine.bDirect)
		{
			if (!IR816PlainDirect(szOperand, psLine->sLine.uiOperand, 0, &psLine->sLine.iOffset) || (psLine->sLine.iOffset + 1 >= IR816_DIRECT_PAGE))
			{
				if (bStores)
				{
					return false;
				}
				psLine->sLine.bDirect = false;
			}
			if (psLine->sLine.uiFlags & IR816MF_STORE)
			{
				i = ((psLine->sLine.uiMnemonic == IR816M_Stx) || (psLine->sLine.uiMnemonic == IR816M_Sty)) ? iWidthX : iWidthA;
				psLine->iBytes = (i == 16) ? 2 : (i == 8) ? 1 : 0;
			}
		}
		else if ((psLine->sLine.uiOperand > 1) && (szOperand[0] == '[') && (szOperand[1] == '['))
		{
			if (!IR816PlainDirect(szOperand, psLine->sLine.uiOperand, 1, &psLine->iPointer) || (psLine->iPointer + 2 >= IR816_DIRECT_PAGE))
			{
				if (bStores)
				{
					return false;
				}
			}
			else
			{
				psLine->bPointer = true;
			}
		}
		else if (bStores)
		{
			for (ui = 0; ui + 5 <= psLine->sLine.uiOperand; ui++)
			{
				if (strncmp(&szOperand[ui], "[D + ", 5) == 0)
				{
					return false;
				}
			}
		}

		//The widths after it.
		if (psLine->sLine.uiFlags & IR816MF_WIDTH)
		{
			for (ui = 0; ui < psLine->sLine.uiOperand; ui++)
			{
				if (szOperand[ui] == 'm')
				{
					iWidthA = (psLine->sLine.uiMnemonic == IR816M_Sep) ? 8 : 16;
				}
				else if (szOperand[ui] == 'x')
				{
					iWidthX = (psLine->sLine.uiMnemonic == IR816M_Sep) ? 8 : 16;
				}
				else if ((szOperand[ui] >= '0') && (szOperand[ui] <= '9'))
				{
					iWidthA = 0;
					iWidthX = 0;
				}
			}
		}
		else if ((psLine->sLine.uiMnemonic == IR816M_Plp) || (psLine->sLine.uiMnemonic == IR816M_Xce) || (psLine->sLine.uiMnemonic == IR816M_Rti))
		{
			iWidthA = 0;
			iWidthX = 0;
		}
		iBefore = (int)pasLines->NumElements() - 1;
	}

	//The labels, by name.
	for (ui = 0; ui < pasLines->NumElements(); ui++)
	{
		psLine = pasLines->Get(ui);
		if (psLine->iKind == IR816FL_LABEL)
		{
			psLabel = pasLabels->Add();
			psLabel->szLabel = psLine->szLine;
			psLabel->uiLength = (size)(psLine->szEnd - psLine->szLine) - 1;
			psLabel->iLine = (int)ui;
		}
	}
	pasLabels->QuickSort(&IR816CompareFlowLabels);

	//The blocks.  One starting at a label that isn't a local one (Ln) may also be entered from outside the code.
	psBlock = NULL;
	for (ui = 0; ui < pasLines->NumElements(); ui++)
	{
		psLine = pasLines->Get(ui);
		if (!psBlock || (psLine->iKind == IR816FL_LABEL))
		{
			if (psBlock)
			{
				psBlock->uiEnd = ui;
			}
			psBlock = pasBlocks->Add();
			memset(psBlock, 0, sizeof(SIR816FlowBlock));
			psBlock->uiFirst = ui;
			psBlock->aiNext[0] = -1;
			psBlock->aiNext[1] = -1;
			psBlock->bEntered = (ui == 0) || ((psLine->iKind == IR816FL_LABEL) && !((psLine->szLine[0] == 'L') && (psLine->szLine[1] >= '0') && (psLine->szLine[1] <= '9')));
		}
		i = (int)pasBlocks->NumElements() - 1;
		paiBlockOf->Add(&i);
		if ((psLine->iKind == IR816FL_INSTRUCTION) && (psLine->sLine.uiFlags & (IR816MF_LEAVES | IR816MF_ENDS_FLOW)) && !(psLine->sLine.uiFlags & IR816MF_CALL) && (psLine->sLine.uiMnemonic != IR816M_Bit))
		{
			psBlock->uiEnd = ui + 1;
			psBlock = NULL;
		}
	}
	if (psBlock)
	{
		psBlock->uiEnd = pasLines->NumElements();
	}

	//An indirect call pushes where it comes back to (per Ln - 1), then the address it goes to, and goes there with an
	//rtl: that label is gone to from outside the code.
	for (ui = 0; ui < pasLines->NumElements(); ui++)
	{
		psLine = pasLines->Get(ui);
		if ((psLine->iKind == IR816FL_INSTRUCTION) && (psLine->sLine.uiMnemonic == IR816M_Per))
		{
			szOperand = psLine->sLine.szOperand;
			for (uiLabel = 0; (uiLabel < psLine->sLine.uiOperand) && (szOperand[uiLabel] != ' '); uiLabel++)
			{
			}
			iTarget = (uiLabel > 0) ? IR816FindFlowLabel(pasLabels, szOperand, uiLabel) : -1;
			if (iTarget < 0)
			{
				return false;
			}
			pasBlocks->Get(*paiBlockOf->Get(iTarget))->bEntered = true;
		}
	}

	//Where each block goes.
	for (ui = 0; ui < pasBlocks->NumElements(); ui++)
	{
		psBlock = pasBlocks->Get(ui);
		psLast = NULL;
		psPrevious = NULL;
		for (uj = psBlock->uiFirst; uj < psBlock->uiEnd; uj++)
		{
			if (pasLines->Get(uj)->iKind == IR816FL_INSTRUCTION)
			{
				psPrevious = psLast;
				psLast = pasLines->Get(uj);
			}
		}
		if (!psLast || !(psLast->sLine.uiFlags & (IR816MF_LEAVES | IR816MF_ENDS_FLOW)) || (psLast->sLine.uiFlags & IR816MF_CALL) || (psLast->sLine.uiMnemonic == IR816M_Bit))
		{
			//Falls through to the next block, or off the end of the code.
			if (ui + 1 < pasBlocks->NumElements())
			{
				psBlock->aiNext[0] = (int)ui + 1;
			}
			else
			{
				psBlock->bLeavesAll = true;
			}
			continue;
		}
		szOperand = psLast->sLine.szOperand;
		for (uiLabel = 0; (uiLabel < psLast->sLine.uiOperand) && (szOperand[uiLabel] != ' '); uiLabel++)
		{
		}
		iTarget = (uiLabel > 0) ? IR816FindFlowLabel(pasLabels, szOperand, uiLabel) : -1;
		if (psLast->sLine.uiFlags & IR816MF_BRANCH)
		{
			if (iTarget < 0)
			{
				return false;
			}
			psBlock->aiNext[0] = *paiBlockOf->Get(iTarget);
			if (!(psLast->sLine.uiFlags & IR816MF_ENDS_FLOW))
			{
				if (ui + 1 < pasBlocks->NumElements())
				{
					psBlock->aiNext[1] = (int)ui + 1;
				}
				else
				{
					psBlock->bLeavesAll = true;
				}
			}
		}
		else if ((psLast->sLine.uiMnemonic == IR816M_Rtl) || (psLast->sLine.uiMnemonic == IR816M_Rts) || (psLast->sLine.uiMnemonic == IR816M_Rti))
		{
			//A return after the caller's D is pulled back (pld) in code that pointed D at a frame of its own: the frame
			//is dead.  Any other rtl may be an indirect call (pha, rtl) or code without a frame (D the caller's).
			psBlock->bLeavesAll = !(bFrame && psPrevious && (psPrevious->sLine.uiMnemonic == IR816M_Pld));
		}
		else if (((psLast->sLine.uiMnemonic == IR816M_Jml) || (psLast->sLine.uiMnemonic == IR816M_Jmp)) && (iTarget >= 0))
		{
			psBlock->aiNext[0] = *paiBlockOf->Get(iTarget);
		}
		else if ((psLast->sLine.uiMnemonic == IR816M_Stp) || ((psLast->sLine.uiMnemonic == IR816M_Jml) && (psLast->sLine.uiOperand > 0) && (szOperand[0] != '[')))
		{
			psBlock->bLeavesAll = true;
		}
		else
		{
			return false;
		}
	}
	return pasBlocks->NumElements() > 0;
}


//////////////////////////////////////////////////////////////////////////
//
// IR816BuildFlowLabelled with a table of the labels of its own.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816BuildFlow(const char* szText, bool bStores, CArrayTemplate<SIR816FlowLine>* pasLines, CArrayTemplate<SIR816FlowBlock>* pasBlocks, CArrayTemplate<int>* paiBlockOf)
{
	CArrayTemplate<SIR816FlowLabel>	asLabels;
	bool							bResult;

	asLabels.Init();
	bResult = IR816BuildFlowLabelled(szText, bStores, pasLines, pasBlocks, paiBlockOf, &asLabels);
	asLabels.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The lines of pasLines but those abRemove marks, as the code from uiStart.
//
//////////////////////////////////////////////////////////////////////////
static void IR816WriteFlow(CChars* pszDest, size uiStart, CArrayTemplate<SIR816FlowLine>* pasLines, CArrayTemplate<bool>* pabRemove)
{
	CChars			szOut;
	size			ui;
	SIR816FlowLine*	psLine;

	szOut.Init();
	for (ui = 0; ui < pasLines->NumElements(); ui++)
	{
		if (!*pabRemove->Get(ui))
		{
			psLine = pasLines->Get(ui);
			szOut.Append(psLine->szLine, (size)(psLine->szEnd - psLine->szLine));
			szOut.AppendNewLine();
		}
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut.Text(), szOut.Length());
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Stores to a direct page word that no way on from them reads before it is
// written again or the function returns, across the function's branches
// and labels (RemoveDeadStores only sees straight line code, and
// RemoveUnreadStores only words nothing reads): the frame is dead after the
// rtl, and a place shared by values is often stored for one that the code
// after a branch doesn't use.  Liveness of each byte of the direct page,
// back over the code's blocks (IR816BuildFlow) until nothing changes.
// True if any went.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::RemoveFlowDeadStores(CChars* pszDest, size uiStart)
{
	CArrayTemplate<SIR816FlowLine>	asLines;
	CArrayTemplate<SIR816FlowBlock>	asBlocks;
	CArrayTemplate<int>				aiBlockOf;
	CArrayTemplate<bool>			abRemove;
	SIR816FlowLine*					psLine;
	SIR816FlowBlock*				psBlock;
	SIR816FlowBlock*				psNext;
	SIR816LiveBytes					sLive;
	size							ui;
	size							uj;
	int								i;
	bool							bChanged;
	bool							bRemoved;
	bool							bFalse;
	int32							iReached;	//The lowest byte a pointer into the frame may reach (bFrameAddress).

	asLines.Init();
	asBlocks.Init();
	aiBlockOf.Init();
	if (!IR816BuildFlow(pszDest->Text() + uiStart, true, &asLines, &asBlocks, &aiBlockOf))
	{
		aiBlockOf.Kill();
		asBlocks.Kill();
		asLines.Kill();
		return false;
	}

	//Liveness back over the blocks until it settles.
	do
	{
		bChanged = false;
		for (ui = asBlocks.NumElements(); ui > 0; ui--)
		{
			psBlock = asBlocks.Get(ui - 1);
			if (psBlock->bLeavesAll)
			{
				memset(&sLive, 0xFF, sizeof(SIR816LiveBytes));
			}
			else
			{
				memset(&sLive, 0, sizeof(SIR816LiveBytes));
			}
			for (i = 0; i < 2; i++)
			{
				if (psBlock->aiNext[i] >= 0)
				{
					psNext = asBlocks.Get(psBlock->aiNext[i]);
					for (uj = 0; uj < IR816_DIRECT_PAGE / 64; uj++)
					{
						sLive.aulli[uj] |= psNext->sIn.aulli[uj];
					}
				}
			}
			psBlock->sOut = sLive;
			for (uj = psBlock->uiEnd; uj > psBlock->uiFirst; uj--)
			{
				IR816FlowBack(asLines.Get(uj - 1), &sLive);
			}
			if (memcmp(&sLive, &psBlock->sIn, sizeof(SIR816LiveBytes)) != 0)
			{
				psBlock->sIn = sLive;
				bChanged = true;
			}
		}
	}
	while (bChanged);

	//The bytes from the lowest pointer into the frame on may be read through it: their stores stay.
	iReached = IR816_DIRECT_PAGE;
	for (ui = 0; ui < asLines.NumElements(); ui++)
	{
		psLine = asLines.Get(ui);
		if (psLine->bFrameAddress && (psLine->iFrame < iReached))
		{
			iReached = (psLine->iFrame < 0) ? 0 : psLine->iFrame;
		}
	}

	//The stores none of whose bytes are live after them.
	abRemove.Init();
	bFalse = false;
	for (ui = 0; ui < asLines.NumElements(); ui++)
	{
		abRemove.Add(&bFalse);
	}
	bRemoved = false;
	for (ui = 0; ui < asBlocks.NumElements(); ui++)
	{
		psBlock = asBlocks.Get(ui);
		sLive = psBlock->sOut;
		for (uj = psBlock->uiEnd; uj > psBlock->uiFirst; uj--)
		{
			psLine = asLines.Get(uj - 1);
			if ((psLine->iKind == IR816FL_INSTRUCTION) && psLine->sLine.bDirect && (psLine->sLine.uiFlags & IR816MF_STORE) &&
				(psLine->sLine.iOffset + 2 <= iReached) && !IR816AnyLive(&sLive, psLine->sLine.iOffset, (psLine->iBytes == 1) ? 1 : 2))
			{
				*abRemove.Get(uj - 1) = true;
				bRemoved = true;
				continue;
			}
			IR816FlowBack(psLine, &sLive);
		}
	}
	if (bRemoved)
	{
		IR816WriteFlow(pszDest, uiStart, &asLines, &abRemove);
	}
	abRemove.Kill();
	aiBlockOf.Kill();
	asBlocks.Kill();
	asLines.Kill();
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
// What A, X and Y hold after the line (IR816UpdateHeld), but a branch or a
// jump to a label changes nothing (the block it goes to starts with what
// is known there), and a label or a directive forgets all.
//
//////////////////////////////////////////////////////////////////////////
static void IR816FlowHeld(SIR816FlowLine* psLine, SIR816Held* asHeld)
{
	if (psLine->iKind == IR816FL_INSTRUCTION)
	{
		if (!(psLine->sLine.uiFlags & IR816MF_BRANCH) && !(((psLine->sLine.uiMnemonic == IR816M_Jml) || (psLine->sLine.uiMnemonic == IR816M_Jmp)) && (psLine->sLine.uiOperand > 0) && (psLine->sLine.szOperand[0] == 'L')))
		{
			IR816UpdateHeld(asHeld, &psLine->sLine);
		}
	}
	else if ((psLine->iKind == IR816FL_LABEL) || ((psLine->szLine[0] == '\t') && (psLine->szLine[1] == '\t') && (psLine->szLine[2] == '.')))
	{
		IR816ForgetHeld(asHeld, 0, 2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What both hold, in asHeld: each register keeps only the operands psOther
// knows it holds too.
//
//////////////////////////////////////////////////////////////////////////
static void IR816MeetHeld(SIR816Held* asHeld, SIR816Held* asOther)
{
	int		i;
	int		j;
	int		k;
	bool	bBoth;

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < IR816_HELD; j++)
		{
			if (asHeld[i].aszOperand[j][0] == '\0')
			{
				continue;
			}
			bBoth = false;
			for (k = 0; k < IR816_HELD; k++)
			{
				bBoth |= (strcmp(asHeld[i].aszOperand[j], asOther[i].aszOperand[k]) == 0);
			}
			if (!bBoth)
			{
				memset(asHeld[i].aszOperand[j], 0, IR816_HELD_LENGTH);
				asHeld[i].aiOffset[j] = -1;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether every register holds the same operands in both (in any order).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SameHeld(SIR816Held* asHeld, SIR816Held* asOther)
{
	int		i;
	int		j;
	int		k;
	int		iHeld;
	int		iOther;
	bool	bBoth;

	for (i = 0; i < 3; i++)
	{
		iHeld = 0;
		iOther = 0;
		for (j = 0; j < IR816_HELD; j++)
		{
			iOther += (asOther[i].aszOperand[j][0] != '\0') ? 1 : 0;
			if (asHeld[i].aszOperand[j][0] == '\0')
			{
				continue;
			}
			iHeld++;
			bBoth = false;
			for (k = 0; k < IR816_HELD; k++)
			{
				bBoth |= (strcmp(asHeld[i].aszOperand[j], asOther[i].aszOperand[k]) == 0);
			}
			if (!bBoth)
			{
				return false;
			}
		}
		if (iHeld != iOther)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// PeepholeRegisters across labels: a block starts with what A, X and Y
// hold at the end of every block that goes to it (the same direct page
// word or constant), worked out forward over the blocks (IR816BuildFlow)
// until nothing changes, where PeepholeRegisters knows nothing after a
// label.  A block a label outside the code may go to (a function's entry)
// starts with nothing known.  Then the loads of what the register holds go
// (but before something reads the N and Z they set).  True if any went.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::PeepholeFlowRegisters(CChars* pszDest, size uiStart)
{
	CArrayTemplate<SIR816FlowLine>	asLines;
	CArrayTemplate<SIR816FlowBlock>	asBlocks;
	CArrayTemplate<int>				aiBlockOf;
	CArrayTemplate<bool>			abRemove;
	CArrayTemplate<SIR816Held>		asIn;		//Three per block: what A, X and Y hold at its start.
	CArrayTemplate<SIR816Held>		asOut;		//And at its end.
	CArrayTemplate<bool>			abDone;		//Whether its end is worked out yet.
	CArrayTemplate<int>				aiFromStart;	//Per block (and one more): where its blocks gone to from start in aiFrom.
	CArrayTemplate<int>				aiFrom;
	CArrayTemplate<int>				aiFill;
	SIR816FlowLine*					psLine;
	SIR816FlowBlock*				psBlock;
	SIR816Held						asHeld[3];
	SIR816Held						asNone[3];
	size							ui;
	size							uj;
	int								i;
	int								iRegister;
	int								iTo;
	int								iFrom;
	bool							bChanged;
	bool							bRemoved;
	bool							bFalse;
	bool							bFirst;

	asLines.Init();
	asBlocks.Init();
	aiBlockOf.Init();
	if (!IR816BuildFlow(pszDest->Text() + uiStart, false, &asLines, &asBlocks, &aiBlockOf))
	{
		aiBlockOf.Kill();
		asBlocks.Kill();
		asLines.Kill();
		return false;
	}

	memset(asNone, 0, sizeof(asNone));
	IR816ForgetHeld(asNone, 0, 2);
	asIn.Init();
	asOut.Init();
	abDone.Init();
	bFalse = false;
	for (ui = 0; ui < asBlocks.NumElements(); ui++)
	{
		for (i = 0; i < 3; i++)
		{
			asIn.Add(&asNone[i]);
			asOut.Add(&asNone[i]);
		}
		abDone.Add(&bFalse);
	}

	//The blocks going to each block, in block order: block ui's are aiFrom[aiFromStart[ui]] up to aiFromStart[ui + 1].
	aiFromStart.Init();
	aiFrom.Init();
	aiFromStart.GrowToAtLeastNumElements(asBlocks.NumElements() + 1, true, 0);
	for (ui = 0; ui < asBlocks.NumElements(); ui++)
	{
		psBlock = asBlocks.Get(ui);
		for (i = 0; i < 2; i++)
		{
			if ((psBlock->aiNext[i] >= 0) && ((i == 0) || (psBlock->aiNext[1] != psBlock->aiNext[0])))
			{
				(*aiFromStart.Get(psBlock->aiNext[i] + 1))++;
			}
		}
	}
	for (ui = 0; ui < asBlocks.NumElements(); ui++)
	{
		*aiFromStart.Get(ui + 1) += *aiFromStart.Get(ui);
	}
	aiFrom.GrowToAtLeastNumElements(*aiFromStart.Get(asBlocks.NumElements()), true, 0);
	aiFill.Init();
	aiFill.GrowToAtLeastNumElements(asBlocks.NumElements(), true, 0);
	for (ui = 0; ui < asBlocks.NumElements(); ui++)
	{
		psBlock = asBlocks.Get(ui);
		for (i = 0; i < 2; i++)
		{
			if ((psBlock->aiNext[i] >= 0) && ((i == 0) || (psBlock->aiNext[1] != psBlock->aiNext[0])))
			{
				iTo = psBlock->aiNext[i];
				*aiFrom.Get(*aiFromStart.Get(iTo) + *aiFill.Get(iTo)) = (int)ui;
				(*aiFill.Get(iTo))++;
			}
		}
	}
	aiFill.Kill();

	//Forward over the blocks until it settles: each starts with what all the blocks worked out so far that go to it
	//end with (nothing, if one may be entered from outside).
	do
	{
		bChanged = false;
		for (ui = 0; ui < asBlocks.NumElements(); ui++)
		{
			psBlock = asBlocks.Get(ui);
			bFirst = true;
			if (psBlock->bEntered)
			{
				memcpy(asHeld, asNone, sizeof(asHeld));
				bFirst = false;
			}
			for (iFrom = *aiFromStart.Get(ui); iFrom < *aiFromStart.Get(ui + 1); iFrom++)
			{
				uj = (size)*aiFrom.Get(iFrom);
				if (!*abDone.Get(uj))
				{
					continue;
				}
				if (bFirst)
				{
					memcpy(asHeld, asOut.Get(uj * 3), sizeof(asHeld));
					bFirst = false;
				}
				else
				{
					IR816MeetHeld(asHeld, asOut.Get(uj * 3));
				}
			}
			if (bFirst)
			{
				memcpy(asHeld, asNone, sizeof(asHeld));
			}
			memcpy(asIn.Get(ui * 3), asHeld, sizeof(asHeld));
			for (uj = psBlock->uiFirst; uj < psBlock->uiEnd; uj++)
			{
				psLine = asLines.Get(uj);
				if ((uj == psBlock->uiFirst) && (psLine->iKind == IR816FL_LABEL))
				{
					continue;
				}
				IR816FlowHeld(psLine, asHeld);
			}
			//Once worked out, a block's end only loses what it holds (a register holds two operands at most, so knowing more
			//at its start may leave less known at its end): the loop settles.  Dropping what is known is always safe.
			if (*abDone.Get(ui))
			{
				IR816MeetHeld(asHeld, asOut.Get(ui * 3));
			}
			if (!*abDone.Get(ui) || !IR816SameHeld(asOut.Get(ui * 3), asHeld))
			{
				memcpy(asOut.Get(ui * 3), asHeld, sizeof(asHeld));
				*abDone.Get(ui) = true;
				bChanged = true;
			}
		}
	}
	while (bChanged);

	//The loads of what the register holds.
	abRemove.Init();
	for (ui = 0; ui < asLines.NumElements(); ui++)
	{
		abRemove.Add(&bFalse);
	}
	bRemoved = false;
	for (ui = 0; ui < asBlocks.NumElements(); ui++)
	{
		psBlock = asBlocks.Get(ui);
		memcpy(asHeld, asIn.Get(ui * 3), sizeof(asHeld));
		for (uj = psBlock->uiFirst; uj < psBlock->uiEnd; uj++)
		{
			psLine = asLines.Get(uj);
			if ((uj == psBlock->uiFirst) && (psLine->iKind == IR816FL_LABEL))
			{
				continue;
			}
			if (psLine->iKind == IR816FL_INSTRUCTION)
			{
				iRegister = (psLine->sLine.uiMnemonic == IR816M_Lda) ? 0 : (psLine->sLine.uiMnemonic == IR816M_Ldx) ? 1 : (psLine->sLine.uiMnemonic == IR816M_Ldy) ? 2 : -1;
				if ((iRegister >= 0) && IR816Holds(asHeld, iRegister, &psLine->sLine) && !IR816ZNReadAfter(psLine->szLine))
				{
					*abRemove.Get(uj) = true;
					bRemoved = true;
					continue;
				}
			}
			IR816FlowHeld(psLine, asHeld);
		}
	}
	if (bRemoved)
	{
		IR816WriteFlow(pszDest, uiStart, &asLines, &abRemove);
	}
	abRemove.Kill();
	aiFrom.Kill();
	aiFromStart.Kill();
	abDone.Kill();
	asOut.Kill();
	asIn.Kill();
	aiBlockOf.Kill();
	asBlocks.Kill();
	asLines.Kill();
	return bRemoved;
}

