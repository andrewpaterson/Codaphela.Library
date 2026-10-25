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
#include "BaseLib/StringHelper.h"
#include "IR65Flags.h"


#define IR65_FLAG_ROUNDS	8		//The most times the compares are looked at again after some went.


//////////////////////////////////////////////////////////////////////////
//
// True if the line is a cmp (an Alu that writes nothing).
//
//////////////////////////////////////////////////////////////////////////
static bool IR65FlagsIsCompare(SIR65CodeLine* psLine)
{
	return (psLine->uiKind == IR65CK_Alu) && !psLine->bWritesFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// The text of an instruction without its leading tabs.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR65FlagsInstruction(CChars* pszLine)
{
	const char*	sz;

	sz = pszLine->Text();
	while (*sz == '\t')
	{
		sz++;
	}
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Flags::Init(void)
{
	mcCode.Init();
	masStates.Init();
	mauiDemand.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Flags::Kill(void)
{
	mauiDemand.Kill();
	masStates.Kill();
	mcCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Rewrites the function's code from uiStart (its label first) without the
// compares whose flags are already set.  Code with anything it doesn't know
// is left as it is.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Flags::Optimise(CChars* pszCode, size uiStart)
{
	size	ui;
	int		iRound;
	bool	bRemoved;

	if (!mcCode.Load(pszCode, uiStart))
	{
		return;
	}

	bRemoved = false;
	for (iRound = 0; iRound < IR65_FLAG_ROUNDS; iRound++)
	{
		ComputeStates();
		mcCode.ComputeFlagDemand(&mauiDemand);
		if (!RemoveCompares())
		{
			break;
		}
		bRemoved = true;
	}
	if (!bRemoved)
	{
		return;
	}

	pszCode->SetLength(uiStart);
	for (ui = 0; ui < mcCode.NumLines(); ui++)
	{
		mcCode.WriteLine(pszCode, ui);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What the flags hold as each line starts: nothing at the function's
// label, then what the lines before it leave (nothing where two ways in
// disagree).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Flags::ComputeStates(void)
{
	size			ui;
	size			uiNum;
	size			uiNext;
	size			auiTo[2];
	int				iTo;
	SIR65FlagState	sState;
	SIR65FlagState*	psTo;
	SIR65CodeLine*	psLine;
	bool			bChanged;

	uiNum = mcCode.NumLines();
	masStates.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		memset(masStates.Add(), 0, sizeof(SIR65FlagState));
	}
	masStates.Get(0)->uiKind = IR65FK_None;

	do
	{
		bChanged = false;
		for (ui = 0; ui < uiNum; ui++)
		{
			if (!mcCode.IsReached(ui) || (masStates.Get(ui)->uiKind == IR65FK_Unknown))
			{
				continue;
			}
			psLine = mcCode.GetLine(ui);
			sState = *masStates.Get(ui);
			Transfer(ui, &sState);

			auiTo[0] = (mcCode.FallsThrough(ui) && (ui + 1 < uiNum)) ? ui + 1 : uiNum;
			auiTo[1] = (psLine->iTarget >= 0) ? (size)psLine->iTarget : uiNum;
			for (iTo = 0; iTo < 2; iTo++)
			{
				uiNext = auiTo[iTo];
				if (uiNext >= uiNum)
				{
					continue;
				}
				psTo = masStates.Get(uiNext);
				if (psTo->uiKind == IR65FK_Unknown)
				{
					*psTo = sState;
					bChanged = true;
				}
				else if ((psTo->uiKind != IR65FK_None) && (memcmp(psTo, &sState, sizeof(SIR65FlagState)) != 0))
				{
					//Two ways in agree on less: the register without the copy, or nothing.
					if ((psTo->uiKind == sState.uiKind) && (psTo->iRegister == sState.iRegister) && (psTo->uiBits == sState.uiBits) && (psTo->iCompare == sState.iCompare))
					{
						bChanged |= psTo->iCopy != -1;
						psTo->iCopy = -1;
					}
					else
					{
						memset(psTo, 0, sizeof(SIR65FlagState));
						psTo->uiKind = IR65FK_None;
						bChanged = true;
					}
				}
			}
		}
	}
	while (bChanged);
}


//////////////////////////////////////////////////////////////////////////
//
// What the flags hold after the line, from what they held before it.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Flags::Transfer(size uiLine, SIR65FlagState* psState)
{
	SIR65CodeLine*	psLine;
	SIR65CodeLine*	psCompare;
	SIR65Widths		sWidths;
	SIR65Widths		sReads;
	SIR65Widths		sWrites;
	uint8			uiUses;
	uint8			uiCompareUses;
	int				i;
	bool			bWritesMemory;

	psLine = mcCode.GetLine(uiLine);
	sWidths = *mcCode.GetWidths(uiLine);
	mcCode.Effect(uiLine, &sWidths, &sReads, &sWrites, &uiUses);

	if (psLine->uiFlagsWritten != 0)
	{
		memset(psState, 0, sizeof(SIR65FlagState));
		psState->uiKind = IR65FK_None;
		psState->iRegister = -1;
		psState->iCopy = -1;
		if (IR65FlagsIsCompare(psLine))
		{
			psState->uiKind = IR65FK_Compare;
			psState->iCompare = (int32)uiLine;
			if (IsCompareWithZero(uiLine))
			{
				psState->iRegister = psLine->iFirst;
				psState->uiBits = mcCode.GetWidths(uiLine)->auiRegisters[psLine->iFirst];
			}
		}
		else if (psLine->bResultFlags && (psLine->iFirst >= 0))
		{
			//The value written to the register, at the operation's width (the register's).
			psState->uiKind = IR65FK_Value;
			psState->iRegister = psLine->iFirst;
			psState->uiBits = mcCode.GetWidths(uiLine)->auiRegisters[psLine->iFirst];
			if (!psLine->bReadsFirst && (psLine->uiKind == IR65CK_Alu) && (psLine->iSecond >= 0) && (mcCode.GetWidths(uiLine)->auiRegisters[psLine->iSecond] >= psState->uiBits))
			{
				//cpy R1, R2: R2 is the same value at R1's width.
				psState->iCopy = psLine->iSecond;
			}
		}
		else if (psLine->bResultFlags && psLine->bFirstMemory && !psLine->bReadsFirst && (psLine->iSecond >= 0))
		{
			//cpy [m], R: the value stored, R at its width.
			psState->uiKind = IR65FK_Value;
			psState->iRegister = psLine->iSecond;
			psState->uiBits = mcCode.GetWidths(uiLine)->auiRegisters[psLine->iSecond];
		}
		return;
	}

	if ((psState->uiKind != IR65FK_Value) && (psState->uiKind != IR65FK_Compare))
	{
		return;
	}
	if (((psState->iRegister >= 0) && (sWrites.auiRegisters[psState->iRegister] != 0)) || ((psState->iCopy >= 0) && (sWrites.auiRegisters[psState->iCopy] != 0)))
	{
		psState->uiKind = IR65FK_None;
		return;
	}
	if (psState->uiKind == IR65FK_Compare)
	{
		//The compare's operands must not change: its registers, and memory if it reads any.
		psCompare = mcCode.GetLine(psState->iCompare);
		uiCompareUses = psCompare->uiAddresses;
		if (psCompare->iFirst >= 0)
		{
			uiCompareUses |= (uint8)(1 << psCompare->iFirst);
		}
		if (psCompare->iSecond >= 0)
		{
			uiCompareUses |= (uint8)(1 << psCompare->iSecond);
		}
		for (i = 0; i < IR65_CODE_REGISTERS; i++)
		{
			if ((uiCompareUses & (1 << i)) && (sWrites.auiRegisters[i] != 0))
			{
				psState->uiKind = IR65FK_None;
				return;
			}
		}
		bWritesMemory = psLine->bFirstMemory && (psLine->bWritesFirst || (psLine->uiKind == IR65CK_Pull));
		if (bWritesMemory && mcCode.GetText(psState->iCompare)->Contains('['))
		{
			psState->uiKind = IR65FK_None;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the compares that can go: one the flags already hold (an earlier
// identical cmp of unchanged operands, not itself going), and a cmp R, 0
// after n and z were set from R when only n and z are read after it.  True
// if any did.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Flags::RemoveCompares(void)
{
	CArrayTemplate<uint8>	auiNew;		//Per line: 1 for a compare found to go this time.
	size					ui;
	SIR65CodeLine*			psLine;
	SIR65FlagState*			psState;
	bool					bRemoved;
	uint8					uiBits;

	bRemoved = false;
	auiNew.Init();
	for (ui = 0; ui < mcCode.NumLines(); ui++)
	{
		auiNew.Add()[0] = 0;
	}
	for (ui = 0; ui < mcCode.NumLines(); ui++)
	{
		psLine = mcCode.GetLine(ui);
		if (!mcCode.IsReached(ui) || !IR65FlagsIsCompare(psLine))
		{
			continue;
		}
		psState = masStates.Get(ui);
		if (IsCompareWithZero(ui) && ((psState->uiKind == IR65FK_Value) || (psState->uiKind == IR65FK_Compare)) && ((psState->iRegister == psLine->iFirst) || (psState->iCopy == psLine->iFirst)))
		{
			uiBits = mcCode.GetWidths(ui)->auiRegisters[psLine->iFirst];
			if ((psState->uiBits == uiBits) && ((*mauiDemand.Get(ui) & ~(IR65_FLAG_N | IR65_FLAG_Z)) == 0))
			{
				*auiNew.Get(ui) = 1;
				bRemoved = true;
			}
		}
	}
	for (ui = 0; ui < mcCode.NumLines(); ui++)
	{
		psLine = mcCode.GetLine(ui);
		if (!mcCode.IsReached(ui) || *auiNew.Get(ui) || !IR65FlagsIsCompare(psLine))
		{
			continue;
		}
		psState = masStates.Get(ui);
		if ((psState->uiKind == IR65FK_Compare) && !*auiNew.Get(psState->iCompare) && SameCompare(psState->iCompare, ui))
		{
			*auiNew.Get(ui) = 1;
			bRemoved = true;
		}
	}
	for (ui = 0; ui < mcCode.NumLines(); ui++)
	{
		if (*auiNew.Get(ui))
		{
			mcCode.Remove(ui);
		}
	}
	auiNew.Kill();
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the line is cmp R, 0 with R one of A, B, X and Y.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Flags::IsCompareWithZero(size uiLine)
{
	SIR65CodeLine*	psLine;

	psLine = mcCode.GetLine(uiLine);
	return IR65FlagsIsCompare(psLine) && (psLine->iFirst >= 0) && mcCode.GetText(uiLine)->EndsWith(", 0");
}


//////////////////////////////////////////////////////////////////////////
//
// True if two compares are the same instruction at the same widths, so
// (with their operands unchanged) they set the same flags.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Flags::SameCompare(size uiLine1, size uiLine2)
{
	SIR65Widths	sWidths;
	SIR65Widths	sReads;
	SIR65Widths	sWrites;
	uint8		uiUses;
	int			i;

	if (StringCompare(IR65FlagsInstruction(mcCode.GetText(uiLine1)), IR65FlagsInstruction(mcCode.GetText(uiLine2))) != 0)
	{
		return false;
	}
	sWidths = *mcCode.GetWidths(uiLine2);
	mcCode.Effect(uiLine2, &sWidths, &sReads, &sWrites, &uiUses);
	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		if ((uiUses & (1 << i)) && (mcCode.GetWidths(uiLine1)->auiRegisters[i] != mcCode.GetWidths(uiLine2)->auiRegisters[i]))
		{
			return false;
		}
	}
	return true;
}
