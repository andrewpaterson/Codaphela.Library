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
#include "BaseLib/StringHelper.h"
#include "IR65Widths.h"


//////////////////////////////////////////////////////////////////////////
//
// True if line uiLine, not removed, is "uiMnemonic R, value" with R one of
// A, B, X and Y; the register's index and the value.
//
//////////////////////////////////////////////////////////////////////////
static bool IR65WidthsImmediate(CIR65Code* pcCode, size uiLine, uint8 uiMnemonic, int* piRegister, int64* plliValue)
{
	const char*	sz;
	size		uiLength;
	int			i;
	char*		szEnd;

	if (pcCode->IsRemoved(uiLine) || (pcCode->Mnemonic(uiLine) != uiMnemonic))
	{
		return false;
	}
	sz = pcCode->OperandsText(uiLine, &uiLength);
	if ((uiLength < 3) || (sz[1] != ',') || (sz[2] != ' '))
	{
		return false;
	}
	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		if (sz[0] == CIR65Code::RegisterName(i))
		{
			*piRegister = i;
			*plliValue = strtoll(sz + 3, &szEnd, 10);
			return (szEnd != sz + 3) && (*szEnd == '\0');
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if line uiLine, not removed, is "cpy R1, R2" with R1 and R2 two of
// A, B, X and Y.
//
//////////////////////////////////////////////////////////////////////////
static bool IR65WidthsCopy(CIR65Code* pcCode, size uiLine, int iDestination, int* piSource)
{
	SIR65CodeLine*	psLine;
	size			uiLength;

	if (pcCode->IsRemoved(uiLine))
	{
		return false;
	}
	psLine = pcCode->GetLine(uiLine);
	pcCode->OperandsText(uiLine, &uiLength);
	if ((psLine->uiKind != IR65CK_Alu) || (pcCode->Mnemonic(uiLine) != IR65M_Cpy) || (psLine->iFirst != iDestination) || (psLine->iSecond < 0) || (psLine->iSecond == iDestination) || (uiLength != 4))
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
	mpcCode = NULL;
	mauiDemand.Init();
	masLive.Init();
	masEntries.Init();
	mauiArrived.Init();
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
}


//////////////////////////////////////////////////////////////////////////
//
// Writes pcCode again with its registers' widths set only where they must
// be; true if it took anything out or changed its number of lines.  Code
// with anything it doesn't know is left as it is.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Widths::Run(CIR65Code* pcCode)
{
	size	uiLines;
	bool	bRound;
	bool	bChanged;

	mpcCode = pcCode;
	if (!mpcCode->Analyse(false))
	{
		return false;
	}
	uiLines = mpcCode->NumLines();
	mpcCode->ComputeFlagDemand(&mauiDemand);
	NarrowExtensions();
	bChanged = false;
	do
	{
		ComputeLive();
		bRound = RemoveExtensions() || UseTests();
		bChanged |= bRound;
	}
	while (bRound);

	Write();
	return bChanged || (mpcCode->NumLines() != uiLines);
}


//////////////////////////////////////////////////////////////////////////
//
// The pass on pszCode from uiStart (its label first), as a whole function:
// its code is written again if it could be read.  For the tests.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Optimise(CChars* pszCode, size uiStart)
{
	CIR65Code	cCode;

	cCode.Init();
	cCode.SetText(pszCode, uiStart);
	Run(&cCode);
	if (cCode.IsLoaded())
	{
		pszCode->SetLength(uiStart);
		cCode.Write(pszCode);
	}
	cCode.Kill();
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

	uiNum = mpcCode->NumLines();
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
			if (!mpcCode->IsReached(ui))
			{
				continue;
			}
			LiveOut(ui, &sOut);
			sWidths = *mpcCode->GetWidths(ui);
			mpcCode->Effect(ui, &sWidths, &sReads, &sWrites, &uiUses);
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

	psLine = mpcCode->GetLine(uiLine);
	memset(psOut, 0, sizeof(SIR65Widths));
	if (mpcCode->FallsThrough(uiLine) && (uiLine + 1 < mpcCode->NumLines()))
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
// "cpy A, R / and A, value / bras z (or brac z)" with the bits of A they
// write read by neither way on is "tst R, value / bras z": the same n and z, A untouched, one
// instruction less.  Only with A and R the same width.  True if one was.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Widths::UseTests(void)
{
	size			ui;
	size			uiNum;
	SIR65CodeLine*	psCopy;
	SIR65CodeLine*	psAnd;
	SIR65CodeLine*	psBranch;
	SIR65Widths		sOut;
	SIR65Widths*	psWidths;
	const char*		szCopy;
	const char*		szAnd;
	const char*		szBranch;
	size			uiCopy;
	size			uiAnd;
	size			uiBranch;
	CChars			szInstruction;
	bool			bChanged;

	bChanged = false;
	uiNum = mpcCode->NumLines();
	for (ui = 0; ui + 2 < uiNum; ui++)
	{
		if (!mpcCode->IsReached(ui) || mpcCode->IsRemoved(ui) || mpcCode->IsRemoved(ui + 1) || mpcCode->IsRemoved(ui + 2))
		{
			continue;
		}
		psCopy = mpcCode->GetLine(ui);
		psAnd = mpcCode->GetLine(ui + 1);
		psBranch = mpcCode->GetLine(ui + 2);
		if ((psCopy->uiKind != IR65CK_Alu) || (psAnd->uiKind != IR65CK_Alu) || (psBranch->uiKind != IR65CK_Branch))
		{
			continue;
		}
		if ((psCopy->iFirst != IR65CR_A) || (psCopy->iSecond < 0) || (psCopy->iSecond == IR65CR_A) || (psAnd->iFirst != IR65CR_A) || (psAnd->iSecond >= 0))
		{
			continue;
		}
		szCopy = mpcCode->OperandsText(ui, &uiCopy);
		szAnd = mpcCode->OperandsText(ui + 1, &uiAnd);
		szBranch = mpcCode->OperandsText(ui + 2, &uiBranch);
		if ((mpcCode->Mnemonic(ui) != IR65M_Cpy) || (uiCopy != 4) || (strncmp(szCopy, "A, ", 3) != 0) || (mpcCode->Mnemonic(ui + 1) != IR65M_And) || (uiAnd < 3) || (strncmp(szAnd, "A, ", 3) != 0) ||
			(szAnd[3] == '[') || !((mpcCode->Mnemonic(ui + 2) == IR65M_Bras) || (mpcCode->Mnemonic(ui + 2) == IR65M_Brac)) || (uiBranch < 3) || (strncmp(szBranch, "z, ", 3) != 0))
		{
			continue;
		}
		psWidths = mpcCode->GetWidths(ui);
		if (psWidths->auiRegisters[IR65CR_A] != psWidths->auiRegisters[psCopy->iSecond])
		{
			continue;
		}
		//The bits of A the cpy and and write (at A's width) are read by neither way on (a wider A's upper bits
		//aren't touched either way).
		LiveOut(ui + 2, &sOut);
		if (sOut.auiRegisters[IR65CR_A] & CIR65Code::Bands(psWidths->auiRegisters[IR65CR_A]))
		{
			continue;
		}
		szInstruction.Init();
		szInstruction.Append(CIR65Code::RegisterName(psCopy->iSecond))->Append(", ")->Append(szAnd + 3);
		if (mpcCode->ReplaceInstruction(ui + 1, IR65M_Tst, szInstruction.Text()))
		{
			mpcCode->Remove(ui);
			bChanged = true;
		}
		szInstruction.Kill();
	}
	return bChanged;
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

	uiNum = mpcCode->NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		if (!mpcCode->IsReached(ui))
		{
			continue;
		}
		if (IR65WidthsImmediate(mpcCode, ui, IR65M_And, &iRegister, &lliValue) && ((lliValue == 0xFF) || (lliValue == 0xFFFF)) && (mpcCode->GetWidths(ui)->auiRegisters[iRegister] == 32))
		{
			uiBits = (lliValue == 0xFF) ? 8 : 16;
			if ((ui > 0) && mpcCode->IsReached(ui - 1) && IR65WidthsCopy(mpcCode, ui - 1, iRegister, &iSource) && (mpcCode->GetWidths(ui - 1)->auiRegisters[iSource] == 32))
			{
				//cpy R1, R2 / and R1, 255 is cpy R1, R2 with R2 8 bits wide (the same flags).
				mpcCode->GetWidths(ui - 1)->auiRegisters[iSource] = uiBits;
				mpcCode->Remove(ui);
			}
			continue;
		}
		if (!IR65WidthsImmediate(mpcCode, ui, IR65M_Xor, &iRegister, &lliValue) || ((lliValue != 0x80) && (lliValue != 0x8000)) || (mpcCode->GetWidths(ui)->auiRegisters[iRegister] != 32))
		{
			continue;
		}
		if ((ui + 1 >= uiNum) || !IR65WidthsImmediate(mpcCode, ui + 1, IR65M_Sub, &iSub, &lliSub) || (iSub != iRegister) || (lliSub != lliValue))
		{
			continue;
		}
		uiBits = (lliValue == 0x80) ? 8 : 16;
		uiDemand = *mauiDemand.Get(ui + 1);
		if ((ui > 0) && mpcCode->IsReached(ui - 1) && IR65WidthsCopy(mpcCode, ui - 1, iRegister, &iSource) && (mpcCode->GetWidths(ui - 1)->auiRegisters[iSource] == 32) && ((uiDemand & (IR65_FLAG_C | IR65_FLAG_V)) == 0))
		{
			szInstruction.Init();
			szInstruction.Append(CIR65Code::RegisterName(iRegister))->Append(", ")->Append(CIR65Code::RegisterName(iSource))->Append(":s");
			if (mpcCode->ReplaceInstruction(ui - 1, IR65M_Cpy, szInstruction.Text()))
			{
				mpcCode->GetWidths(ui - 1)->auiRegisters[iSource] = uiBits;
				mpcCode->Remove(ui);
				mpcCode->Remove(ui + 1);
			}
			szInstruction.Kill();
		}
		else if (uiDemand == 0)
		{
			szInstruction.Init();
			szInstruction.Append(CIR65Code::RegisterName(iRegister))->Append(", 32:s");
			if (mpcCode->ReplaceInstruction(ui, IR65M_Wthx, szInstruction.Text()))
			{
				mpcCode->GetWidths(ui)->auiRegisters[iRegister] = uiBits;
				mpcCode->Remove(ui + 1);
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
	for (ui = 0; ui < mpcCode->NumLines(); ui++)
	{
		psLine = mpcCode->GetLine(ui);
		if ((psLine->uiKind != IR65CK_Extend) || !mpcCode->IsReached(ui))
		{
			continue;
		}
		LiveOut(ui, &sOut);
		uiFrom = mpcCode->GetWidths(ui)->auiRegisters[psLine->iFirst];
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

	for (ui = uiFirst; ui < mpcCode->NumLines(); ui++)
	{
		psLine = mpcCode->GetLine(ui);
		if (!mpcCode->IsReached(ui) || (psLine->uiKind == IR65CK_Branch) || (psLine->uiKind == IR65CK_Jump))
		{
			return 0;
		}
		sWidths = *mpcCode->GetWidths(ui);
		mpcCode->Effect(ui, &sWidths, &sReads, &sWrites, &uiUses);
		if (uiUses & (1 << iRegister))
		{
			return mpcCode->GetWidths(ui)->auiRegisters[iRegister];
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
		if ((uiFallThrough < mpcCode->NumLines()) && (NextNeed(uiFallThrough, i) != sTarget.auiRegisters[i]))
		{
			sTarget.auiRegisters[i] = 0;
		}
	}
	SetWidths(&sTarget, mpcCode->NumLines());
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
			uiNeed = bFromActual ? msActual.auiRegisters[i] : mpcCode->GetWidths(uiLabel)->auiRegisters[i];
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
	CChars	szLine;
	int		i;
	int		iPair;
	uint8	uiBits;
	uint8	uiMnemonic;

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
			uiMnemonic = IR65M_Wtth;
			szLine.Init("(");
			szLine.Append(CIR65Code::RegisterName(i))->Append(", ")->Append(CIR65Code::RegisterName(iPair))->Append("), ")->Append((int32)uiBits);
			msActual.auiRegisters[iPair] = uiBits;
		}
		else
		{
			uiMnemonic = IR65M_Wth;
			szLine.Init();
			szLine.Append(CIR65Code::RegisterName(i))->Append(", ")->Append((int32)uiBits);
		}
		mpcCode->AddLine(uiMnemonic, szLine.Text());
		szLine.Kill();
		msActual.auiRegisters[i] = uiBits;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the code again (CIR65Code::BeginLines) without its wth and wtth:
// each instruction gets
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

	uiNum = mpcCode->NumLines();
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

	mpcCode->BeginLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mpcCode->GetLine(ui);
		if (!mpcCode->IsReached(ui) || (psLine->uiKind == IR65CK_Other))
		{
			mpcCode->KeepLine(ui);
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
			else if (mpcCode->IsJumpedBack(ui))
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
			mpcCode->KeepLine(ui);
			break;

		case IR65CK_Width:
			break;

		case IR65CK_Branch:
		case IR65CK_Jump:
			if (mpcCode->IsJumpedBack(psLine->iTarget))
			{
				DecideEntry(psLine->iTarget, true);
				SetWidths(masEntries.Get(psLine->iTarget), uiNum);
			}
			else
			{
				Anticipate(psLine->iTarget, (psLine->uiKind == IR65CK_Branch) ? ui + 1 : uiNum);
				Arrive(psLine->iTarget);
			}
			mpcCode->KeepLine(ui);
			bKnown = psLine->uiKind == IR65CK_Branch;
			break;

		default:
			sWidths = *mpcCode->GetWidths(ui);
			mpcCode->Effect(ui, &sWidths, &sReads, &sWrites, &uiUses);
			memset(&sTarget, 0, sizeof(SIR65Widths));
			for (i = 0; i < IR65_CODE_REGISTERS; i++)
			{
				if (uiUses & (1 << i))
				{
					sTarget.auiRegisters[i] = mpcCode->GetWidths(ui)->auiRegisters[i];
				}
			}
			SetWidths(&sTarget, ui);
			mpcCode->KeepLine(ui);
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
	mpcCode->EndLines();
}
