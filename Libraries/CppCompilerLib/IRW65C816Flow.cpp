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
#include <stdlib.h>
#include <string.h>
#include "IRW65C816BackendInternal.h"
#include "IRW65C816Code.h"


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
	SIRW65C816Line	sLine;
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
// Whether the tdc on line uiLine takes a pointer to the frame (clc, adc n
// with no tcs after: not IR816MovesS), with n in *piFrame.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816FrameAddress(CIRW65C816Code* pcCode, size uiLine, int32* piFrame)
{
	SIRW65C816Line	sLine;

	if ((uiLine + 1 >= pcCode->NumLines()) || !pcCode->Instruction(uiLine + 1, &sLine))
	{
		return false;
	}
	if (sLine.uiMnemonic != IR816M_Clc)
	{
		return false;
	}
	if ((uiLine + 2 >= pcCode->NumLines()) || !pcCode->Instruction(uiLine + 2, &sLine))
	{
		return false;
	}
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


//The widths A and X have at a line of the code: 16, 8, or 0 when they aren't known here.
struct SIR816FlowWidths
{
	int		iA;
	int		iX;
};


//////////////////////////////////////////////////////////////////////////
//
// The widths after a .width directive.
//
//////////////////////////////////////////////////////////////////////////
static void IR816DirectiveWidths(const char* szLine, SIR816FlowWidths* psWidths)
{
	if (strncmp(szLine, "\t\t.width (A, X), 16", 19) == 0)
	{
		psWidths->iA = 16;
		psWidths->iX = 16;
	}
	else if (strncmp(szLine, "\t\t.width A, 8", 13) == 0)
	{
		psWidths->iA = 8;
	}
	else if (strncmp(szLine, "\t\t.width", 8) == 0)
	{
		psWidths->iA = 0;
		psWidths->iX = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The widths after an instruction.
//
//////////////////////////////////////////////////////////////////////////
static void IR816InstructionWidths(SIRW65C816Line* psLine, SIR816FlowWidths* psWidths)
{
	size	ui;
	char	c;

	if (psLine->uiFlags & IR816MF_WIDTH)
	{
		for (ui = 0; ui < psLine->uiOperand; ui++)
		{
			c = psLine->szOperand[ui];
			if (c == 'm')
			{
				psWidths->iA = (psLine->uiMnemonic == IR816M_Sep) ? 8 : 16;
			}
			else if (c == 'x')
			{
				psWidths->iX = (psLine->uiMnemonic == IR816M_Sep) ? 8 : 16;
			}
			else if ((c >= '0') && (c <= '9'))
			{
				psWidths->iA = 0;
				psWidths->iX = 0;
			}
		}
	}
	else if ((psLine->uiMnemonic == IR816M_Plp) || (psLine->uiMnemonic == IR816M_Xce) || (psLine->uiMnemonic == IR816M_Rti))
	{
		psWidths->iA = 0;
		psWidths->iX = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A direct page store's bytes (SIR816FlowLine::iBytes) at these widths.
//
//////////////////////////////////////////////////////////////////////////
static int IR816StoreBytes(SIRW65C816Line* psLine, SIR816FlowWidths* psWidths)
{
	int		iWidth;

	iWidth = ((psLine->uiMnemonic == IR816M_Stx) || (psLine->uiMnemonic == IR816M_Sty)) ? psWidths->iX : psWidths->iA;
	return (iWidth == 16) ? 2 : (iWidth == 8) ? 1 : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A call's line (psLine), psBefore the instruction before it or NULL.  False
// if with bStores what it does to the direct page can't be known.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816FlowCall(SIR816FlowLine* psLine, SIR816FlowLine* psBefore, bool bStores)
{
	const char*	szOperand;

	szOperand = psLine->sLine.szOperand;
	if (bStores && (psLine->sLine.uiOperand > 3) && (strncmp(&szOperand[psLine->sLine.uiOperand - 3], IR816_FRAME_OPERANDS, 3) == 0))
	{
		return false;
	}
	if (strncmp(szOperand, IR816_RUNTIME_PREFIX, IR816_RUNTIME_PREFIX_LENGTH) == 0)
	{
		if (bStores && IR816IndexedRoutine(szOperand, psLine->sLine.uiOperand))
		{
			psLine->iFrame = IR816LoadedX(psBefore);
			if (psLine->iFrame < 0)
			{
				return false;
			}
			psLine->bIndexedFrame = true;
		}
		psLine->bRuntime = true;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What an instruction's operand (psLine) reaches of the direct page.  False
// if with bStores that can't be known.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816FlowOperand(SIR816FlowLine* psLine, SIR816FlowLine* psBefore, bool bStores, SIR816FlowWidths* psWidths)
{
	const char*	szOperand;
	size		ui;
	int32		iX;
	int32		iLoaded;

	szOperand = psLine->sLine.szOperand;

	//[D + n + X] after ldx m is [D + n + m]: a frame place in the direct page, or past it (big frames).
	if (bStores && IR816IndexedDirect(szOperand, psLine->sLine.uiOperand, &iX))
	{
		iLoaded = IR816LoadedX(psBefore);
		if (iLoaded < 0)
		{
			return false;
		}
		iX += iLoaded;
		psLine->sLine.bDirect = iX < IR816_DIRECT_PAGE;
		psLine->sLine.iOffset = iX;
		if (psLine->sLine.bDirect && (psLine->sLine.uiFlags & IR816MF_STORE))
		{
			psLine->iBytes = IR816StoreBytes(&psLine->sLine, psWidths);
		}
		return true;
	}

	//The direct page only as [D + n] or a pointer there.
	if (psLine->sLine.bDirect)
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
			psLine->iBytes = IR816StoreBytes(&psLine->sLine, psWidths);
		}
		return true;
	}
	if ((psLine->sLine.uiOperand > 1) && (szOperand[0] == '[') && (szOperand[1] == '['))
	{
		if (IR816PlainDirect(szOperand, psLine->sLine.uiOperand, 1, &psLine->iPointer) && IR816PointerOnDirectPage(psLine->iPointer))
		{
			psLine->bPointer = true;
			return true;
		}
		return !bStores;
	}
	if (bStores)
	{
		for (ui = 0; ui + 5 <= psLine->sLine.uiOperand; ui++)
		{
			if (strncmp(&szOperand[ui], "[D + ", 5) == 0)
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The code's lines, what each reads and writes, and the widths.  *pbFrame is
// whether the code points D at a frame of its own.  False if it can't be
// followed (IR816BuildFlowLabelled).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ReadFlowLines(CIRW65C816Code* pcCode, bool bStores, CArrayTemplate<SIR816FlowLine>* pasLines, bool* pbFrame)
{
	SIR816FlowLine*		psLine;
	SIR816FlowLine*		psBefore;
	SIR816FlowWidths	sWidths;
	const char*			szLine;
	const char*			szEnd;
	size				uiLine;
	int					iBefore;	//The instruction before's line, with no label between, or -1.

	sWidths.iA = 16;
	sWidths.iX = 16;
	*pbFrame = false;
	iBefore = -1;
	for (uiLine = 0; uiLine < pcCode->NumLines(); uiLine++)
	{
		szLine = pcCode->Text(uiLine);
		szEnd = szLine + pcCode->GetText(uiLine)->Length();
		psLine = pasLines->Add();
		if (!psLine)
		{
			return false;
		}
		memset(psLine, 0, sizeof(SIR816FlowLine));
		psLine->szLine = szLine;
		psLine->szEnd = szEnd;
		psLine->iKind = IR816FL_OTHER;
		if ((szLine[0] != '\t') && (szEnd > szLine + 1) && (szEnd[-1] == ':'))
		{
			psLine->iKind = IR816FL_LABEL;
			sWidths.iA = 0;
			sWidths.iX = 0;
			iBefore = -1;
			continue;
		}
		if ((szLine[0] == '\t') && (szLine[1] == '\t') && (szLine[2] == '.'))
		{
			IR816DirectiveWidths(szLine, &sWidths);
			continue;
		}
		if (!IR816IsInstruction(szLine) || (szLine[2] == ';') || (szEnd - szLine < 3))
		{
			continue;
		}
		psLine->iKind = IR816FL_INSTRUCTION;
		IR816Decode(szLine, &psLine->sLine);
		if ((psLine->sLine.uiMnemonic == IR816M_Unknown) || (psLine->sLine.uiFlags & IR816MF_INTERRUPT))
		{
			return false;
		}
		psBefore = (iBefore >= 0) ? pasLines->Get(iBefore) : NULL;

		//D's value pushed once it is the frame's (phd) may be passed on as a pointer, and taken (tdc, but to move S) as
		//one unless it is a pointer into the frame (clc, adc n): the bytes from n on may be reached through that.
		if (bStores && (psLine->sLine.uiMnemonic == IR816M_Phd) && *pbFrame)
		{
			return false;
		}
		if (bStores && (psLine->sLine.uiMnemonic == IR816M_Tdc) && !IR816MovesS(pcCode, uiLine))
		{
			if (!IR816FrameAddress(pcCode, uiLine, &psLine->iFrame))
			{
				return false;
			}
			psLine->bFrameAddress = true;
		}
		if (psLine->sLine.uiMnemonic == IR816M_Tcd)
		{
			*pbFrame = true;
		}
		if ((psLine->sLine.uiFlags & IR816MF_CALL) && !IR816FlowCall(psLine, psBefore, bStores))
		{
			return false;
		}
		if (!IR816FlowOperand(psLine, psBefore, bStores, &sWidths))
		{
			return false;
		}
		IR816InstructionWidths(&psLine->sLine, &sWidths);
		iBefore = (int)pasLines->NumElements() - 1;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line may go elsewhere, and so ends its block (a call comes back).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816EndsFlowBlock(SIR816FlowLine* psLine)
{
	return (psLine->iKind == IR816FL_INSTRUCTION) && (psLine->sLine.uiFlags & (IR816MF_LEAVES | IR816MF_ENDS_FLOW)) && !(psLine->sLine.uiFlags & IR816MF_CALL) && (psLine->sLine.uiMnemonic != IR816M_Bit);
}


//////////////////////////////////////////////////////////////////////////
//
// The line of the label a line's operand starts with, or -1.
//
//////////////////////////////////////////////////////////////////////////
static int IR816OperandLabel(CArrayTemplate<SIR816FlowLabel>* pasLabels, SIR816FlowLine* psLine)
{
	const char*	szOperand;
	size		uiLabel;

	szOperand = psLine->sLine.szOperand;
	for (uiLabel = 0; (uiLabel < psLine->sLine.uiOperand) && (szOperand[uiLabel] != ' '); uiLabel++)
	{
	}
	return (uiLabel > 0) ? IR816FindFlowLabel(pasLabels, szOperand, uiLabel) : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The code's labels, sorted by name.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AddFlowLabels(CArrayTemplate<SIR816FlowLine>* pasLines, CArrayTemplate<SIR816FlowLabel>* pasLabels)
{
	SIR816FlowLabel*	psLabel;
	SIR816FlowLine*		psLine;
	size				ui;

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
}


//////////////////////////////////////////////////////////////////////////
//
// The blocks.  One starting at a label that isn't a local one (Ln) may also
// be entered from outside the code.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AddFlowBlocks(CArrayTemplate<SIR816FlowLine>* pasLines, CArrayTemplate<SIR816FlowBlock>* pasBlocks, CArrayTemplate<int>* paiBlockOf)
{
	SIR816FlowLine*		psLine;
	SIR816FlowBlock*	psBlock;
	size				ui;
	int					iBlock;

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
		iBlock = (int)pasBlocks->NumElements() - 1;
		paiBlockOf->Add(&iBlock);
		if (IR816EndsFlowBlock(psLine))
		{
			psBlock->uiEnd = ui + 1;
			psBlock = NULL;
		}
	}
	if (psBlock)
	{
		psBlock->uiEnd = pasLines->NumElements();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An indirect call pushes where it comes back to (per Ln - 1), then the
// address it goes to, and goes there with an rtl: that label is gone to from
// outside the code.  False if it isn't a label of the code's.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816MarkReturnLabels(CArrayTemplate<SIR816FlowLine>* pasLines, CArrayTemplate<SIR816FlowBlock>* pasBlocks, CArrayTemplate<int>* paiBlockOf, CArrayTemplate<SIR816FlowLabel>* pasLabels)
{
	SIR816FlowLine*	psLine;
	size			ui;
	int				iTarget;

	for (ui = 0; ui < pasLines->NumElements(); ui++)
	{
		psLine = pasLines->Get(ui);
		if ((psLine->iKind == IR816FL_INSTRUCTION) && (psLine->sLine.uiMnemonic == IR816M_Per))
		{
			iTarget = IR816OperandLabel(pasLabels, psLine);
			if (iTarget < 0)
			{
				return false;
			}
			pasBlocks->Get(*paiBlockOf->Get(iTarget))->bEntered = true;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the block uiBlock goes.  False if that can't be known.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LinkFlowBlock(size uiBlock, bool bFrame, CArrayTemplate<SIR816FlowLine>* pasLines, CArrayTemplate<SIR816FlowBlock>* pasBlocks, CArrayTemplate<int>* paiBlockOf, CArrayTemplate<SIR816FlowLabel>* pasLabels)
{
	SIR816FlowBlock*	psBlock;
	SIR816FlowLine*		psLast;
	SIR816FlowLine*		psPrevious;
	size				ui;
	int					iTarget;
	uint16				uiMnemonic;

	psBlock = pasBlocks->Get(uiBlock);
	psLast = NULL;
	psPrevious = NULL;
	for (ui = psBlock->uiFirst; ui < psBlock->uiEnd; ui++)
	{
		if (pasLines->Get(ui)->iKind == IR816FL_INSTRUCTION)
		{
			psPrevious = psLast;
			psLast = pasLines->Get(ui);
		}
	}
	if (!psLast || !IR816EndsFlowBlock(psLast))
	{
		//Falls through to the next block, or off the end of the code.
		if (uiBlock + 1 < pasBlocks->NumElements())
		{
			psBlock->aiNext[0] = (int)uiBlock + 1;
		}
		else
		{
			psBlock->bLeavesAll = true;
		}
		return true;
	}

	iTarget = IR816OperandLabel(pasLabels, psLast);
	uiMnemonic = psLast->sLine.uiMnemonic;
	if (psLast->sLine.uiFlags & IR816MF_BRANCH)
	{
		if (iTarget < 0)
		{
			return false;
		}
		psBlock->aiNext[0] = *paiBlockOf->Get(iTarget);
		if (!(psLast->sLine.uiFlags & IR816MF_ENDS_FLOW))
		{
			if (uiBlock + 1 < pasBlocks->NumElements())
			{
				psBlock->aiNext[1] = (int)uiBlock + 1;
			}
			else
			{
				psBlock->bLeavesAll = true;
			}
		}
		return true;
	}
	if ((uiMnemonic == IR816M_Rtl) || (uiMnemonic == IR816M_Rts) || (uiMnemonic == IR816M_Rti))
	{
		//A return after the caller's D is pulled back (pld) in code that pointed D at a frame of its own: the frame
		//is dead.  Any other rtl may be an indirect call (pha, rtl) or code without a frame (D the caller's).
		psBlock->bLeavesAll = !(bFrame && psPrevious && (psPrevious->sLine.uiMnemonic == IR816M_Pld));
		return true;
	}
	if (((uiMnemonic == IR816M_Jml) || (uiMnemonic == IR816M_Jmp)) && (iTarget >= 0))
	{
		psBlock->aiNext[0] = *paiBlockOf->Get(iTarget);
		return true;
	}
	if ((uiMnemonic == IR816M_Stp) || ((uiMnemonic == IR816M_Jml) && (psLast->sLine.uiOperand > 0) && (psLast->sLine.szOperand[0] != '[')))
	{
		psBlock->bLeavesAll = true;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's code as lines (one each of the code's) and blocks (from a label, or
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
static bool IR816BuildFlowLabelled(CIRW65C816Code* pcCode, bool bStores, CArrayTemplate<SIR816FlowLine>* pasLines, CArrayTemplate<SIR816FlowBlock>* pasBlocks, CArrayTemplate<int>* paiBlockOf, CArrayTemplate<SIR816FlowLabel>* pasLabels)
{
	size	ui;
	bool	bFrame;

	if (!IR816ReadFlowLines(pcCode, bStores, pasLines, &bFrame))
	{
		return false;
	}
	IR816AddFlowLabels(pasLines, pasLabels);
	IR816AddFlowBlocks(pasLines, pasBlocks, paiBlockOf);
	if (!IR816MarkReturnLabels(pasLines, pasBlocks, paiBlockOf, pasLabels))
	{
		return false;
	}
	for (ui = 0; ui < pasBlocks->NumElements(); ui++)
	{
		if (!IR816LinkFlowBlock(ui, bFrame, pasLines, pasBlocks, paiBlockOf, pasLabels))
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
static bool IR816BuildFlow(CIRW65C816Code* pcCode, bool bStores, CArrayTemplate<SIR816FlowLine>* pasLines, CArrayTemplate<SIR816FlowBlock>* pasBlocks, CArrayTemplate<int>* paiBlockOf)
{
	CArrayTemplate<SIR816FlowLabel>	asLabels;
	bool							bResult;

	asLabels.Init(CIRW65C816Backend::GetScratch());
	bResult = IR816BuildFlowLabelled(pcCode, bStores, pasLines, pasBlocks, paiBlockOf, &asLabels);
	asLabels.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The code's lines that abRemove marks taken out (the flow lines, which
// point into them, aren't used after).
//
//////////////////////////////////////////////////////////////////////////
static void IR816WriteFlow(CIRW65C816Code* pcCode, CArrayTemplate<bool>* pabRemove)
{
	pcCode->RemoveMarked(pabRemove);
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
bool CIRW65C816Backend::RemoveFlowDeadStores(CChars* pszDest, size uiStart)
{
	CIRW65C816Code	cCode;
	bool		bRemoved;

	cCode.Init();
	cCode.Load(pszDest, uiStart);
	bRemoved = RemoveFlowDeadStores(&cCode);
	if (bRemoved)
	{
		cCode.Write(pszDest, uiStart);
	}
	cCode.Kill();
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::RemoveFlowDeadStores(CIRW65C816Code* pcCode)
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

	asLines.Init(CIRW65C816Backend::GetScratch());
	asBlocks.Init(CIRW65C816Backend::GetScratch());
	aiBlockOf.Init(CIRW65C816Backend::GetScratch());
	if (!IR816BuildFlow(pcCode, true, &asLines, &asBlocks, &aiBlockOf))
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
	abRemove.Init(CIRW65C816Backend::GetScratch());
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
		IR816WriteFlow(pcCode, &abRemove);
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
static void IR816FlowHeld(SIR816FlowLine* psLine, SIRW65C816Held* asHeld)
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
static void IR816MeetHeld(SIRW65C816Held* asHeld, SIRW65C816Held* asOther)
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
static bool IR816SameHeld(SIRW65C816Held* asHeld, SIRW65C816Held* asOther)
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
bool CIRW65C816Backend::PeepholeFlowRegisters(CIRW65C816Code* pcCode)
{
	CArrayTemplate<SIR816FlowLine>	asLines;
	CArrayTemplate<SIR816FlowBlock>	asBlocks;
	CArrayTemplate<int>				aiBlockOf;
	CArrayTemplate<bool>			abRemove;
	CArrayTemplate<SIRW65C816Held>		asIn;		//Three per block: what A, X and Y hold at its start.
	CArrayTemplate<SIRW65C816Held>		asOut;		//And at its end.
	CArrayTemplate<bool>			abDone;		//Whether its end is worked out yet.
	CArrayTemplate<int>				aiFromStart;	//Per block (and one more): where its blocks gone to from start in aiFrom.
	CArrayTemplate<int>				aiFrom;
	CArrayTemplate<int>				aiFill;
	SIR816FlowLine*					psLine;
	SIR816FlowBlock*				psBlock;
	SIRW65C816Held						asHeld[3];
	SIRW65C816Held						asNone[3];
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

	asLines.Init(CIRW65C816Backend::GetScratch());
	asBlocks.Init(CIRW65C816Backend::GetScratch());
	aiBlockOf.Init(CIRW65C816Backend::GetScratch());
	if (!IR816BuildFlow(pcCode, false, &asLines, &asBlocks, &aiBlockOf))
	{
		aiBlockOf.Kill();
		asBlocks.Kill();
		asLines.Kill();
		return false;
	}

	memset(asNone, 0, sizeof(asNone));
	IR816ForgetHeld(asNone, 0, 2);
	asIn.Init(CIRW65C816Backend::GetScratch());
	asOut.Init(CIRW65C816Backend::GetScratch());
	abDone.Init(CIRW65C816Backend::GetScratch());
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
	aiFromStart.Init(CIRW65C816Backend::GetScratch());
	aiFrom.Init(CIRW65C816Backend::GetScratch());
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
	aiFill.Init(CIRW65C816Backend::GetScratch());
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
	abRemove.Init(CIRW65C816Backend::GetScratch());
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
				if ((iRegister >= 0) && IR816Holds(asHeld, iRegister, &psLine->sLine) && !IR816ZNReadAfter(pcCode, uj))
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
		IR816WriteFlow(pcCode, &abRemove);
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

