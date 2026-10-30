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
#include "IR65Widths.h"


//////////////////////////////////////////////////////////////////////////
//
// True if the line is "szMnemonic R, value" with R one of A, B, X and Y;
// the register's index and the value.
//
//////////////////////////////////////////////////////////////////////////
static bool IR65WidthsImmediate(CChars* pszLine, const char* szMnemonic, int* piRegister, int64* plliValue)
{
	const char*	sz;
	size		uiLength;
	int			i;
	char*		szEnd;

	sz = pszLine->Text();
	while (*sz == '\t')
	{
		sz++;
	}
	uiLength = strlen(szMnemonic);
	if ((strncmp(sz, szMnemonic, uiLength) != 0) || (sz[uiLength] != ' ') || (sz[uiLength + 2] != ',') || (sz[uiLength + 3] != ' '))
	{
		return false;
	}
	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		if (sz[uiLength + 1] == CIR65Code::RegisterName(i))
		{
			*piRegister = i;
			*plliValue = strtoll(sz + uiLength + 4, &szEnd, 10);
			return (szEnd != sz + uiLength + 4) && (*szEnd == '\0');
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the line is "cpy R1, R2" with R1 and R2 two of A, B, X and Y.
//
//////////////////////////////////////////////////////////////////////////
static bool IR65WidthsCopy(CIR65Code* pcCode, size uiLine, int iDestination, int* piSource)
{
	SIR65CodeLine*	psLine;
	const char*		sz;

	psLine = pcCode->GetLine(uiLine);
	sz = pcCode->GetText(uiLine)->Text();
	while (*sz == '\t')
	{
		sz++;
	}
	if ((psLine->uiKind != IR65CK_Alu) || (strncmp(sz, "cpy ", 4) != 0) || (psLine->iFirst != iDestination) || (psLine->iSecond < 0) || (psLine->iSecond == iDestination) || (strlen(sz) != 8))
	{
		return false;
	}
	*piSource = psLine->iSecond;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Init(void)
{
	mcCode.Init();
	mauiDemand.Init();
	masLive.Init();
	masEntries.Init();
	mauiArrived.Init();
	mpszOut = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Kill(void)
{
	mauiArrived.Kill();
	masEntries.Kill();
	masLive.Kill();
	mauiDemand.Kill();
	mcCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Rewrites the function's code from uiStart (its label first).  Code with
// anything it doesn't know is left as it is.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Optimise(CChars* pszCode, size uiStart)
{
	if (!mcCode.Load(pszCode, uiStart, false))
	{
		return;
	}
	mpszOut = pszCode;
	mcCode.ComputeFlagDemand(&mauiDemand);
	NarrowExtensions();
	do
	{
		ComputeLive();
	}
	while (RemoveExtensions());

	pszCode->SetLength(uiStart);
	Write();
}


//////////////////////////////////////////////////////////////////////////
//
// The bits of each register read before they are written, from the start
// of each line (live in).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::ComputeLive(void)
{
	size			ui;
	size			uiNum;
	SIR65Widths		sWidths;
	SIR65Widths		sReads;
	SIR65Widths		sWrites;
	SIR65Widths		sOut;
	SIR65Widths*	psIn;
	uint8			uiUses;
	uint8			uiIn;
	bool			bChanged;
	int				i;

	uiNum = mcCode.NumLines();
	masLive.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		memset(masLive.Add(), 0, sizeof(SIR65Widths));
	}

	do
	{
		bChanged = false;
		for (ui = uiNum; ui-- > 0;)
		{
			if (!mcCode.IsReached(ui))
			{
				continue;
			}
			LiveOut(ui, &sOut);
			sWidths = *mcCode.GetWidths(ui);
			mcCode.Effect(ui, &sWidths, &sReads, &sWrites, &uiUses);
			psIn = masLive.Get(ui);
			for (i = 0; i < IR65_CODE_REGISTERS; i++)
			{
				uiIn = sReads.auiRegisters[i] | (sOut.auiRegisters[i] & ~sWrites.auiRegisters[i]);
				if (uiIn != psIn->auiRegisters[i])
				{
					psIn->auiRegisters[i] = uiIn;
					bChanged = true;
				}
			}
		}
	}
	while (bChanged);
}


//////////////////////////////////////////////////////////////////////////
//
// The bits read after a line before they are written: its successors'
// live in.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::LiveOut(size uiLine, SIR65Widths* psOut)
{
	SIR65CodeLine*	psLine;
	SIR65Widths*	psNext;
	int				i;

	psLine = mcCode.GetLine(uiLine);
	memset(psOut, 0, sizeof(SIR65Widths));
	if (mcCode.FallsThrough(uiLine) && (uiLine + 1 < mcCode.NumLines()))
	{
		psNext = masLive.Get(uiLine + 1);
		for (i = 0; i < IR65_CODE_REGISTERS; i++)
		{
			psOut->auiRegisters[i] |= psNext->auiRegisters[i];
		}
	}
	if (psLine->iTarget >= 0)
	{
		psNext = masLive.Get(psLine->iTarget);
		for (i = 0; i < IR65_CODE_REGISTERS; i++)
		{
			psOut->auiRegisters[i] |= psNext->auiRegisters[i];
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The back end extends an i8 or i16 kept in a 32 bit register with "and R,
// 255" (zero) or "xor R, 128 / sub R, 128" (sign).  After "cpy R1, R2" the
// copy can do it, with R2 that narrow: "cpy R1, R2:s" (the sign extend
// sets n and z alike, but not c and v, so only when nothing reads those).
// A sign extend on its own is a wthx when nothing reads the flags it sets.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::NarrowExtensions(void)
{
	size	ui;
	size	uiNum;
	int		iRegister;
	int		iSub;
	int		iSource;
	int64	lliValue;
	int64	lliSub;
	uint8	uiBits;
	uint8	uiDemand;
	CChars	szInstruction;

	uiNum = mcCode.NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		if (!mcCode.IsReached(ui))
		{
			continue;
		}
		if (IR65WidthsImmediate(mcCode.GetText(ui), "and", &iRegister, &lliValue) && ((lliValue == 0xFF) || (lliValue == 0xFFFF)) && (mcCode.GetWidths(ui)->auiRegisters[iRegister] == 32))
		{
			uiBits = (lliValue == 0xFF) ? 8 : 16;
			if ((ui > 0) && mcCode.IsReached(ui - 1) && IR65WidthsCopy(&mcCode, ui - 1, iRegister, &iSource) && (mcCode.GetWidths(ui - 1)->auiRegisters[iSource] == 32))
			{
				//cpy R1, R2 / and R1, 255 is cpy R1, R2 with R2 8 bits wide (the same flags).
				mcCode.GetWidths(ui - 1)->auiRegisters[iSource] = uiBits;
				mcCode.Remove(ui);
			}
			continue;
		}
		if (!IR65WidthsImmediate(mcCode.GetText(ui), "xor", &iRegister, &lliValue) || ((lliValue != 0x80) && (lliValue != 0x8000)) || (mcCode.GetWidths(ui)->auiRegisters[iRegister] != 32))
		{
			continue;
		}
		if ((ui + 1 >= uiNum) || !IR65WidthsImmediate(mcCode.GetText(ui + 1), "sub", &iSub, &lliSub) || (iSub != iRegister) || (lliSub != lliValue))
		{
			continue;
		}
		uiBits = (lliValue == 0x80) ? 8 : 16;
		uiDemand = *mauiDemand.Get(ui + 1);
		if ((ui > 0) && mcCode.IsReached(ui - 1) && IR65WidthsCopy(&mcCode, ui - 1, iRegister, &iSource) && (mcCode.GetWidths(ui - 1)->auiRegisters[iSource] == 32) && ((uiDemand & (IR65_FLAG_C | IR65_FLAG_V)) == 0))
		{
			szInstruction.Init("cpy ");
			szInstruction.Append(CIR65Code::RegisterName(iRegister))->Append(", ")->Append(CIR65Code::RegisterName(iSource))->Append(":s");
			if (mcCode.Replace(ui - 1, szInstruction.Text()))
			{
				mcCode.GetWidths(ui - 1)->auiRegisters[iSource] = uiBits;
				mcCode.Remove(ui);
				mcCode.Remove(ui + 1);
			}
			szInstruction.Kill();
		}
		else if (uiDemand == 0)
		{
			szInstruction.Init("wthx ");
			szInstruction.Append(CIR65Code::RegisterName(iRegister))->Append(", 32:s");
			if (mcCode.Replace(ui, szInstruction.Text()))
			{
				mcCode.GetWidths(ui)->auiRegisters[iRegister] = uiBits;
				mcCode.Remove(ui + 1);
			}
			szInstruction.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A wthx whose extended bits are never read becomes a wth (only the width
// changes).  True if one did.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Widths::RemoveExtensions(void)
{
	size			ui;
	SIR65CodeLine*	psLine;
	SIR65Widths		sOut;
	uint8			uiFrom;
	uint8			uiExtended;
	bool			bRemoved;

	bRemoved = false;
	for (ui = 0; ui < mcCode.NumLines(); ui++)
	{
		psLine = mcCode.GetLine(ui);
		if ((psLine->uiKind != IR65CK_Extend) || !mcCode.IsReached(ui))
		{
			continue;
		}
		LiveOut(ui, &sOut);
		uiFrom = mcCode.GetWidths(ui)->auiRegisters[psLine->iFirst];
		uiExtended = CIR65Code::Bands(psLine->uiBits) & ~CIR65Code::Bands(uiFrom);
		if ((sOut.auiRegisters[psLine->iFirst] & uiExtended) == 0)
		{
			psLine->uiKind = IR65CK_Width;
			psLine->uiRegisters = (uint8)(1 << psLine->iFirst);
			psLine->iFirst = -1;
			bRemoved = true;
		}
	}
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
// The width the first line from uiFirst on that uses the register needs,
// looking only down the code that runs straight on; 0 if a branch or the
// end comes first.
//
//////////////////////////////////////////////////////////////////////////
uint8 CIR65Widths::NextNeed(size uiFirst, int iRegister)
{
	size			ui;
	SIR65CodeLine*	psLine;
	SIR65Widths		sWidths;
	SIR65Widths		sReads;
	SIR65Widths		sWrites;
	uint8			uiUses;

	for (ui = uiFirst; ui < mcCode.NumLines(); ui++)
	{
		psLine = mcCode.GetLine(ui);
		if (!mcCode.IsReached(ui) || (psLine->uiKind == IR65CK_Branch) || (psLine->uiKind == IR65CK_Jump))
		{
			return 0;
		}
		sWidths = *mcCode.GetWidths(ui);
		mcCode.Effect(ui, &sWidths, &sReads, &sWrites, &uiUses);
		if (uiUses & (1 << iRegister))
		{
			return mcCode.GetWidths(ui)->auiRegisters[iRegister];
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Before going to a label that isn't a loop's: the registers the label's
// code needs at a width they aren't are set to it here, where the label is
// left for (and, for a branch, only when the code it falls through to
// needs the same width), so the ways into the label agree.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Anticipate(size uiLabel, size uiFallThrough)
{
	SIR65Widths	sTarget;
	int			i;

	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		sTarget.auiRegisters[i] = NextNeed(uiLabel + 1, i);
		if ((uiFallThrough < mcCode.NumLines()) && (NextNeed(uiFallThrough, i) != sTarget.auiRegisters[i]))
		{
			sTarget.auiRegisters[i] = 0;
		}
	}
	SetWidths(&sTarget, mcCode.NumLines());
}


//////////////////////////////////////////////////////////////////////////
//
// The widths a loop's label (one a later line jumps back to) starts with
// in the new code: what its code first needs, so the loop sets them once
// before it, else what the code reaching it has (bFromActual), else what
// it had.  They are decided once, by the first way in: one that left a
// register's width unknown (0) has gone there already.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::DecideEntry(size uiLabel, bool bFromActual)
{
	SIR65Widths*	psEntry;
	uint8			uiNeed;
	int				i;

	if (*mauiArrived.Get(uiLabel))
	{
		return;
	}
	*mauiArrived.Get(uiLabel) = 1;
	psEntry = masEntries.Get(uiLabel);
	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		uiNeed = NextNeed(uiLabel + 1, i);
		if (uiNeed == 0)
		{
			uiNeed = bFromActual ? msActual.auiRegisters[i] : mcCode.GetWidths(uiLabel)->auiRegisters[i];
		}
		if (uiNeed == IR65_WIDTH_UNKNOWN)
		{
			uiNeed = 0;
		}
		psEntry->auiRegisters[i] = uiNeed;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The code reaching a label that isn't a loop's (all of it is before the
// label) has the widths msActual.  The label starts with the widths they
// all agree on, and an unknown width (0) for a register they don't.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Arrive(size uiLabel)
{
	SIR65Widths*	psEntry;
	int				i;

	psEntry = masEntries.Get(uiLabel);
	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		if (!*mauiArrived.Get(uiLabel))
		{
			psEntry->auiRegisters[i] = msActual.auiRegisters[i];
		}
		else if (psEntry->auiRegisters[i] != msActual.auiRegisters[i])
		{
			psEntry->auiRegisters[i] = 0;
		}
	}
	*mauiArrived.Get(uiLabel) = 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the wth that set the registers psTarget gives a width (not 0) to
// it.  Two registers going to one width share a wtth, as does a register
// the code from uiNext on next needs at that width.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::SetWidths(SIR65Widths* psTarget, size uiNext)
{
	int		i;
	int		iPair;
	uint8	uiBits;

	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		uiBits = psTarget->auiRegisters[i];
		if ((uiBits == 0) || (msActual.auiRegisters[i] == uiBits))
		{
			continue;
		}
		for (iPair = i + 1; iPair < IR65_CODE_REGISTERS; iPair++)
		{
			if ((psTarget->auiRegisters[iPair] == uiBits) && (msActual.auiRegisters[iPair] != uiBits))
			{
				break;
			}
		}
		if (iPair == IR65_CODE_REGISTERS)
		{
			for (iPair = 0; iPair < IR65_CODE_REGISTERS; iPair++)
			{
				if ((iPair != i) && (psTarget->auiRegisters[iPair] == 0) && (msActual.auiRegisters[iPair] != uiBits) && (NextNeed(uiNext, iPair) == uiBits))
				{
					break;
				}
			}
		}
		if (iPair < IR65_CODE_REGISTERS)
		{
			mpszOut->Append("\t\twtth (")->Append(CIR65Code::RegisterName(i))->Append(", ")->Append(CIR65Code::RegisterName(iPair))->Append("), ")->Append((int32)uiBits)->AppendNewLine();
			msActual.auiRegisters[iPair] = uiBits;
		}
		else
		{
			mpszOut->Append("\t\twth ")->Append(CIR65Code::RegisterName(i))->Append(", ")->Append((int32)uiBits)->AppendNewLine();
		}
		msActual.auiRegisters[i] = uiBits;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the code again without its wth and wtth: each instruction gets
// the widths of the registers it uses set just before it, each label the
// widths it was decided to start with.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Write(void)
{
	size			ui;
	size			uiNum;
	SIR65CodeLine*	psLine;
	SIR65Widths		sWidths;
	SIR65Widths		sReads;
	SIR65Widths		sWrites;
	SIR65Widths		sTarget;
	uint8			uiUses;
	bool			bKnown;
	bool			bFirstLabel;
	int				i;

	uiNum = mcCode.NumLines();
	masEntries.ReInit();
	mauiArrived.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		memset(masEntries.Add(), 0, sizeof(SIR65Widths));
		mauiArrived.Add()[0] = 0;
	}
	memset(&msActual, 32, sizeof(SIR65Widths));
	bKnown = true;
	bFirstLabel = true;

	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mcCode.GetLine(ui);
		if (!mcCode.IsReached(ui) || (psLine->uiKind == IR65CK_Other))
		{
			mcCode.WriteLine(mpszOut, ui);
			continue;
		}

		switch (psLine->uiKind)
		{
		case IR65CK_Label:
			if (bFirstLabel)
			{
				//The function's own label: callers set all four to 32 bits.
				memset(masEntries.Get(ui), 32, sizeof(SIR65Widths));
				*mauiArrived.Get(ui) = 1;
				bFirstLabel = false;
			}
			else if (mcCode.IsJumpedBack(ui))
			{
				DecideEntry(ui, bKnown);
				if (bKnown)
				{
					SetWidths(masEntries.Get(ui), uiNum);
				}
			}
			else if (bKnown)
			{
				Anticipate(ui, uiNum);
				Arrive(ui);
			}
			msActual = *masEntries.Get(ui);
			bKnown = true;
			mcCode.WriteLine(mpszOut, ui);
			break;

		case IR65CK_Width:
			break;

		case IR65CK_Branch:
		case IR65CK_Jump:
			if (mcCode.IsJumpedBack(psLine->iTarget))
			{
				DecideEntry(psLine->iTarget, true);
				SetWidths(masEntries.Get(psLine->iTarget), uiNum);
			}
			else
			{
				Anticipate(psLine->iTarget, (psLine->uiKind == IR65CK_Branch) ? ui + 1 : uiNum);
				Arrive(psLine->iTarget);
			}
			mcCode.WriteLine(mpszOut, ui);
			bKnown = psLine->uiKind == IR65CK_Branch;
			break;

		default:
			sWidths = *mcCode.GetWidths(ui);
			mcCode.Effect(ui, &sWidths, &sReads, &sWrites, &uiUses);
			memset(&sTarget, 0, sizeof(SIR65Widths));
			for (i = 0; i < IR65_CODE_REGISTERS; i++)
			{
				if (uiUses & (1 << i))
				{
					sTarget.auiRegisters[i] = mcCode.GetWidths(ui)->auiRegisters[i];
				}
			}
			SetWidths(&sTarget, ui);
			mcCode.WriteLine(mpszOut, ui);
			for (i = 0; i < IR65_CODE_REGISTERS; i++)
			{
				if (uiUses & (1 << i))
				{
					msActual.auiRegisters[i] = (sWidths.auiRegisters[i] == IR65_WIDTH_UNKNOWN) ? 0 : sWidths.auiRegisters[i];
				}
			}
			bKnown = psLine->uiKind != IR65CK_Return;
			break;
		}
	}
}
