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
#include "CPP65DebugWriter.h"


#define IR816_FRAME_FROM_PHD	5		//S after phd is a function's frame address less jsl's 3 bytes and phd's 2.


//The W65C816 back end's frames: the text of a function's frame entry and exit, taken out when nothing uses the
//frame (DropFrame) and pointed at a page where that saves cycles (AlignFrame).


//////////////////////////////////////////////////////////////////////////
//
// The frame's set up as Prologue writes it (after the label and the
// directives): D saved, S moved down iFrameBytes and D pointed at it.
//
//////////////////////////////////////////////////////////////////////////
void IR816AddFrameEntry(CIRW65C816Code* pcCode, int32 iFrameBytes)
{
	pcCode->AppendInstruction(IR816M_Phd);
	pcCode->AppendInstruction(IR816M_Tsc);
	pcCode->AppendInstruction(IR816M_Sec);
	pcCode->AppendInstruction(IR816M_Sbc, iFrameBytes);
	pcCode->AppendInstruction(IR816M_Tcs);
	pcCode->AppendInstruction(IR816M_Tcd);
}


//////////////////////////////////////////////////////////////////////////
//
// A register function's frame set up as Prologue writes it (after its
// body's label): D saved, the register words pushed (Y's, X's, then A's,
// so A's is lowest) to be the frame's top, S moved down the rest of the
// frame and D pointed at it.
//
//////////////////////////////////////////////////////////////////////////
void IR816AddRegisterFrameEntry(CIRW65C816Code* pcCode, int32 iRegisterWords, int32 iFrameBytes)
{
	int		i;
	int32	iRest;

	pcCode->AppendInstruction(IR816M_Phd);
	for (i = iRegisterWords - 1; i >= 0; i--)
	{
		pcCode->AppendInstruction((i == 2) ? IR816M_Phy : ((i == 1) ? IR816M_Phx : IR816M_Pha));
	}
	pcCode->AppendInstruction(IR816M_Tsc);
	iRest = iFrameBytes - 2 * iRegisterWords;
	if (iRest > 0)
	{
		pcCode->AppendInstruction(IR816M_Sec);
		pcCode->AppendInstruction(IR816M_Sbc, iRest);
		pcCode->AppendInstruction(IR816M_Tcs);
	}
	pcCode->AppendInstruction(IR816M_Tcd);
}


//////////////////////////////////////////////////////////////////////////
//
// The frame's removal as Epilogue writes it (before the rtl): A kept in Y
// while S moves back up iFrameBytes, then D restored.
//
//////////////////////////////////////////////////////////////////////////
void IR816AddFrameExit(CIRW65C816Code* pcCode, int32 iFrameBytes)
{
	pcCode->AppendInstruction(IR816M_Tay);
	pcCode->AppendInstruction(IR816M_Tdc);
	pcCode->AppendInstruction(IR816M_Clc);
	pcCode->AppendInstruction(IR816M_Adc, iFrameBytes);
	pcCode->AppendInstruction(IR816M_Tcs);
	pcCode->AppendInstruction(IR816M_Tya);
	pcCode->AppendInstruction(IR816M_Pld);
}


//////////////////////////////////////////////////////////////////////////
//
// The frame's set up (IR816AddFrameEntry) as text, for DropFrame and
// AlignFrame, which find it and replace it.
//
//////////////////////////////////////////////////////////////////////////
void IR816AppendFrameEntry(CChars* psz, int32 iFrameBytes)
{
	CIRW65C816Code	cCode;

	cCode.Init();
	IR816AddFrameEntry(&cCode, iFrameBytes);
	cCode.Write(psz, psz->Length());
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A register function's frame set up (IR816AddRegisterFrameEntry) as text,
// for AlignFrame.
//
//////////////////////////////////////////////////////////////////////////
void IR816AppendRegisterFrameEntry(CChars* psz, int32 iRegisterWords, int32 iFrameBytes)
{
	CIRW65C816Code	cCode;

	cCode.Init();
	IR816AddRegisterFrameEntry(&cCode, iRegisterWords, iFrameBytes);
	cCode.Write(psz, psz->Length());
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The frame's removal (IR816AddFrameExit) as text, for DropFrame and
// AlignFrame.
//
//////////////////////////////////////////////////////////////////////////
void IR816AppendFrameExit(CChars* psz, int32 iFrameBytes)
{
	CIRW65C816Code	cCode;

	cCode.Init();
	IR816AddFrameExit(&cCode, iFrameBytes);
	cCode.Write(psz, psz->Length());
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The exits of the function's code keep A in Y while S moves back only
// when they must.  An exit is
//
//		tay / tdc / clc / adc n / tcs / tya / pld / rtl
//
// or, with the frame on a page, tay / lda [D + n] / tcs / tya / pld / rtl.
// With nothing returned in A (bNoResult) the tay and tya go.  After an lda
// of a constant the lda is done once S is back instead; after an lda of a
// direct page word it is an ldy (the word can't be read once S is back: an
// interrupt may have pushed over it), and the tay goes.  A throw's exit (a
// sec before the rtl) returns nothing in A whatever the function returns.
// The lines kept are the code's own (their notes with them).
//
//////////////////////////////////////////////////////////////////////////
void IR816TidyExits(CIRW65C816Code* pcCode, bool bNoResult)
{
	CIRW65C816Code	cOut;
	SIRW65C816Line	sLoad;
	CChars			szLoad;
	size			uiLines;
	size			ui;
	size			uiCopied;
	size			uiMove;
	size			uiMoved;
	size			uiTya;
	size			uiPld;
	size			uiRtl;
	bool			bSec;
	bool			bNothing;
	bool			bConstant;
	bool			bDirect;
	bool			bAny;

	uiLines = pcCode->NumLines();
	cOut.Init();
	uiCopied = 0;
	bAny = false;
	for (ui = 0; ui < uiLines; ui++)
	{
		if ((ui < uiCopied) || !pcCode->IsBare(ui, IR816M_Tay))
		{
			continue;
		}

		//S's move back: tdc / clc / adc n / tcs, or lda [D + n] / tcs.
		uiMove = ui + 1;
		if ((uiMove + 2 < uiLines) && pcCode->IsBare(uiMove, IR816M_Tdc) && pcCode->IsBare(uiMove + 1, IR816M_Clc) && pcCode->IsNumberLine(uiMove + 2, IR816M_Adc, "", ""))
		{
			uiMoved = uiMove + 3;
		}
		else if ((uiMove < uiLines) && pcCode->IsNumberLine(uiMove, IR816M_Lda, "[D + ", "]"))
		{
			uiMoved = uiMove + 1;
		}
		else
		{
			continue;
		}
		uiTya = uiMoved + 1;
		uiPld = uiTya + 1;
		if ((uiPld >= uiLines) || !pcCode->IsBare(uiMoved, IR816M_Tcs) || !pcCode->IsBare(uiTya, IR816M_Tya) || !pcCode->IsBare(uiPld, IR816M_Pld))
		{
			continue;
		}
		bSec = (uiPld + 1 < uiLines) && pcCode->IsBare(uiPld + 1, IR816M_Sec);
		uiRtl = bSec ? uiPld + 2 : uiPld + 1;
		if ((uiRtl >= uiLines) || !pcCode->IsBare(uiRtl, IR816M_Rtl))
		{
			continue;
		}
		bNothing = bNoResult || bSec;

		//The line before the tay.
		bConstant = false;
		bDirect = false;
		if (!bNothing && (ui > uiCopied))
		{
			bConstant = pcCode->IsNumberLine(ui - 1, IR816M_Lda, "", "");
			bDirect = !bConstant && pcCode->IsNumberLine(ui - 1, IR816M_Lda, "[D + ", "]");
		}
		if (!bNothing && !bConstant && !bDirect)
		{
			continue;
		}

		bAny = true;
		if (bNothing)
		{
			IR816CopyLines(&cOut, pcCode, uiCopied, ui);
			IR816CopyLines(&cOut, pcCode, uiMove, uiMoved + 1);
		}
		else if (bConstant)
		{
			IR816CopyLines(&cOut, pcCode, uiCopied, ui - 1);
			IR816CopyLines(&cOut, pcCode, uiMove, uiMoved + 1);
			cOut.AppendLine(pcCode, ui - 1);
		}
		else
		{
			IR816CopyLines(&cOut, pcCode, uiCopied, ui - 1);
			pcCode->Decoded(ui - 1, &sLoad);
			szLoad.Init(CIRW65C816Backend::GetScratch());
			szLoad.AppendSubString(sLoad.szOperand, sLoad.uiOperand);
			cOut.AppendInstruction(IR816M_Ldy, szLoad.Text());
			szLoad.Kill();
			IR816CopyLines(&cOut, pcCode, uiMove, uiTya + 1);
		}
		uiCopied = uiTya + 1;
	}
	if (bAny)
	{
		IR816CopyLines(&cOut, pcCode, uiCopied, uiLines);
		pcCode->Take(&cOut);
	}
	cOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// IR816TidyExits over the code in pszCode, as text (the tests').
//
//////////////////////////////////////////////////////////////////////////
void IR816TidyExits(CChars* pszCode, bool bNoResult)
{
	CIRW65C816Code	cCode;

	if (strstr(pszCode->Text(), "\t\ttay\n") == NULL)
	{
		return;
	}
	cCode.Init();
	cCode.Load(pszCode, 0);
	IR816TidyExits(&cCode, bNoResult);
	cCode.Write(pszCode, 0);
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// pcFrom's lines from uiFrom up to (not including) uiTo added after
// pcTo's last.
//
//////////////////////////////////////////////////////////////////////////
void IR816CopyLines(CIRW65C816Code* pcTo, CIRW65C816Code* pcFrom, size uiFrom, size uiTo)
{
	size	ui;

	for (ui = uiFrom; ui < uiTo; ui++)
	{
		pcTo->AppendLine(pcFrom, ui);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Line uiLine is an instruction that leaves the carry as it is and can
// stand between S's move back and the rtl (a load or a transfer, or pld).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ExitKeepsCarry(CIRW65C816Code* pcCode, size uiLine)
{
	SIRW65C816Line	sLine;

	if (pcCode->IsBare(uiLine, IR816M_Pld) || pcCode->IsBare(uiLine, IR816M_Tya) || pcCode->IsBare(uiLine, IR816M_Tay) || pcCode->IsBare(uiLine, IR816M_Tax) ||
		pcCode->IsBare(uiLine, IR816M_Txa))
	{
		return true;
	}
	return pcCode->Instruction(uiLine, &sLine) && !pcCode->IsComment(uiLine) && (sLine.uiOperand > 0) &&
		   ((sLine.uiMnemonic == IR816M_Lda) || (sLine.uiMnemonic == IR816M_Ldx) || (sLine.uiMnemonic == IR816M_Ldy));
}


//////////////////////////////////////////////////////////////////////////
//
// Exceptions (ExceptionsPlan.txt): a function returns with the carry
// clear unless it threw, whether or not it can throw (it may be called
// through a pointer, and that call is tested).  The usual exit's move of S
// back up (clc / adc n / tcs, then only loads, transfers and pld) leaves it
// clear.  Every other rtl in the code gets a clc before it: the exits of a
// function with no frame (DropFrame) or with its frame on a page
// (AlignFrame).  Not a throw's exit (sec before it), nor the rtl that is a
// call through a pointer (after the pha or sta [S + 1] that pushed where it
// goes).  Written last, after every pass that looks at an exit.
//
//////////////////////////////////////////////////////////////////////////
void IR816ClearExitCarries(CIRW65C816Code* pcCode)
{
	SIRW65C816Line	sLine;
	size			ui;
	size			uiLine;
	bool			bClear;

	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		if (!pcCode->IsBare(ui, IR816M_Rtl))
		{
			continue;
		}
		bClear = false;
		if (ui > 0)
		{
			if (pcCode->IsBare(ui - 1, IR816M_Sec) || pcCode->IsBare(ui - 1, IR816M_Clc) || pcCode->IsBare(ui - 1, IR816M_Pha) || pcCode->IsWith(ui - 1, IR816M_Sta, "[S + 1]"))
			{
				bClear = true;
			}
			for (uiLine = ui; !bClear && (uiLine > 0) && IR816ExitKeepsCarry(pcCode, uiLine - 1); uiLine--)
			{
			}
			if (!bClear && (uiLine >= 3))
			{
				bClear = pcCode->IsBare(uiLine - 1, IR816M_Tcs) && pcCode->Decoded(uiLine - 2, &sLine) && (sLine.uiMnemonic == IR816M_Adc) && (sLine.uiOperand > 0) &&
						 pcCode->IsBare(uiLine - 3, IR816M_Clc);
			}
		}
		if (!bClear)
		{
			pcCode->InsertInstruction(ui, IR816M_Clc);
			ui++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// IR816ClearExitCarries over the code in pszCode from uiStart, as text
// (the tests', and FinishFunction's after AlignFrame).
//
//////////////////////////////////////////////////////////////////////////
void IR816ClearExitCarries(CChars* pszCode, size uiStart)
{
	CIRW65C816Code	cCode;

	if (strstr(pszCode->Text() + uiStart, "\t\trtl\n") == NULL)
	{
		return;
	}
	cCode.Init();
	cCode.Load(pszCode, uiStart);
	IR816ClearExitCarries(&cCode);
	cCode.Write(pszCode, uiStart);
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the code calls a routine given places in the frame as numbers
// (a 64 bit shift routine's in X, a soft float frame operand entry's in X,
// Y and A), which DropFrame can't move.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816PassesFrameOffsets(CIRW65C816Code* pcCode)
{
	SIRW65C816Line	sLine;
	size			ui;
	size			uiSuffix;

	uiSuffix = StrLen(IR816_FRAME_OPERANDS);
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		if (pcCode->Instruction(ui, &sLine) && (sLine.uiMnemonic == IR816M_Jsl) &&
			(IR816IndexedRoutine(sLine.szOperand, sLine.uiOperand) || ((sLine.uiOperand > uiSuffix) && (strncmp(sLine.szOperand + sLine.uiOperand - uiSuffix, IR816_FRAME_OPERANDS, uiSuffix) == 0))))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// szText (uiLength long) with each direct page offset (the number after
// "D + ") iBy less, into psz.  With pbFrame given (and szText left out of
// psz) false when an offset is iBy or less: a byte of the frame.
//
//////////////////////////////////////////////////////////////////////////
static void IR816MoveDirectPage(const char* szText, size uiLength, int32 iBy, CChars* psz, bool* pbFrame)
{
	size	ui;
	size	uiEnd;
	int32	iOffset;

	for (ui = 0; ui < uiLength;)
	{
		if ((ui + 4 <= uiLength) && (strncmp(szText + ui, "D + ", 4) == 0))
		{
			iOffset = 0;
			for (uiEnd = ui + 4; (uiEnd < uiLength) && (szText[uiEnd] >= '0') && (szText[uiEnd] <= '9'); uiEnd++)
			{
				iOffset = iOffset * 10 + (szText[uiEnd] - '0');
			}
			if (pbFrame && (iOffset <= iBy))
			{
				*pbFrame = true;
			}
			if (psz)
			{
				psz->Append("D + ");
				psz->Append(iOffset - iBy);
			}
			ui = uiEnd;
		}
		else
		{
			if (psz)
			{
				psz->Append(szText[ui]);
			}
			ui++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function's code (mpcCode) without its frame when it never reads or
// writes the frame's bytes (D + 1 to D + frame size; its temporaries may
// all have been kept in A or fused) or D but to move S back: S isn't moved
// down and back, so D is S and each D + n is n - frame size.  Without any
// D + n, D isn't saved and set either.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::DropFrame(void)
{
	CIRW65C816Code	cPrologue;
	CIRW65C816Code	cEpilogue;
	CIRW65C816Code	cCode;
	CChars			sz;
	SIRW65C816Line	sLine;
	const char*		szMentions;
	size			uiMentions;
	size			ui;
	int				iEpilogues;
	int				iTdcs;
	bool			bPrologue;
	bool			bFrame;
	bool			bUsesD;

	if (msFrame.iFrameBytes == 0)
	{
		return;
	}
	bFrame = false;
	bUsesD = false;
	for (ui = 0; ui < mpcCode->NumLines(); ui++)
	{
		szMentions = mpcCode->Mentions(ui, &uiMentions);
		if (IR816OperandsHave(szMentions, uiMentions, "[D + X]"))
		{
			return;
		}
		bUsesD |= IR816OperandsHave(szMentions, uiMentions, "D + ");
		IR816MoveDirectPage(szMentions, uiMentions, msFrame.iFrameBytes, NULL, &bFrame);
	}
	if (bFrame || IR816PassesFrameOffsets(mpcCode))
	{
		return;
	}

	cPrologue.Init();
	IR816AddFrameEntry(&cPrologue, msFrame.iFrameBytes);
	cEpilogue.Init();
	IR816AddFrameExit(&cEpilogue, msFrame.iFrameBytes);
	iEpilogues = 0;
	iTdcs = 0;
	bPrologue = false;
	for (ui = 0; ui < mpcCode->NumLines(); ui++)
	{
		iEpilogues += mpcCode->LinesAt(ui, &cEpilogue) ? 1 : 0;
		iTdcs += mpcCode->IsBare(ui, IR816M_Tdc) ? 1 : 0;
		bPrologue |= mpcCode->LinesAt(ui, &cPrologue);
	}
	if ((iTdcs != iEpilogues) || !bPrologue)
	{
		cPrologue.Kill();
		cEpilogue.Kill();
		return;
	}

	//Without D + anything it needn't set D either.
	cCode.Init();
	sz.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; ui < mpcCode->NumLines(); ui++)
	{
		if (mpcCode->LinesAt(ui, &cPrologue))
		{
			if (bUsesD)
			{
				cCode.AppendInstruction(IR816M_Phd);
				cCode.AppendInstruction(IR816M_Tsc);
				cCode.AppendInstruction(IR816M_Tcd);
			}
			ui += cPrologue.NumLines() - 1;
		}
		else if (mpcCode->LinesAt(ui, &cEpilogue))
		{
			if (bUsesD)
			{
				cCode.AppendInstruction(IR816M_Pld);
			}
			ui += cEpilogue.NumLines() - 1;
		}
		else if (mpcCode->IsInstructionLine(ui) && mpcCode->Instruction(ui, &sLine) && IR816OperandsHave(sLine.szOperand, sLine.uiOperand, "D + "))
		{
			sz.Clear();
			IR816MoveDirectPage(sLine.szOperand, sLine.uiOperand, msFrame.iFrameBytes, &sz, NULL);
			cCode.AppendInstruction(sLine.uiMnemonic, sz.Text(), sLine.bMarked);
		}
		else if (!mpcCode->IsInstructionLine(ui) && (strstr(mpcCode->Text(ui), "D + ") != NULL))
		{
			sz.Clear();
			IR816MoveDirectPage(mpcCode->Text(ui), StrLen(mpcCode->Text(ui)), msFrame.iFrameBytes, &sz, NULL);
			cCode.Append(sz.Text());
		}
		else
		{
			cCode.AppendLine(mpcCode, ui);
		}
	}
	mpcCode->Take(&cCode);
	sz.Kill();
	cCode.Kill();
	cPrologue.Kill();
	cEpilogue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The lines in szText (each ending with a new line).
//
//////////////////////////////////////////////////////////////////////////
int32 IR816CountLines(const char* szText)
{
	int32	iLines;

	for (iLines = 0; *szText != '\0'; szText++)
	{
		iLines += (*szText == '\n') ? 1 : 0;
	}
	return iLines;
}


//////////////////////////////////////////////////////////////////////////
//
// Where an aligned frame keeps S as it was (AlignFrame): in the page
// (iSavedS) when the frame is past it, else where the caller's D was (the
// link's first bytes).  The epilogue reads it back, and a debugger
// (DescribeFrame's [D + SavedS] rule).
//
//////////////////////////////////////////////////////////////////////////
int32 SIRW65C816Frame::SavedS(void) const
{
	return IR816OnDirectPage(Top()) ? Link() : iSavedS;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives the function in pszCode (as GenerateFunction wrote it, and what it
// kept of it, pcWritten) D on a page, where its direct page accesses cost a cycle less,
// when it's worth it: estimated, or measured by pcProfile (IRW65C816Backend.h).
// Never when it may recurse (bRecursive: in a cycle of calls the unit can
// see; or calling itself), nor when variadic (its arguments are walked past
// the named ones).  D (and S) go down to the page below the frame, the
// arguments and the link bytes; the arguments are copied up to the frame
// (from X, S as it was) where they were, and S as it was is kept where the
// caller's D was (at D + frame size + 1) for the epilogue.  A frame past
// the page (its far bytes are reached through X) has its arguments pushed
// into place and S kept in psFrame->iSavedS (in the page).  Argument words
// nothing reads (IR816FindReads) aren't copied.  Returns true if
// it was aligned.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::AlignFrame(CChars* pszCode, const char* szLabel, CIRW65C816Written* pcWritten, bool bRecursive, CAsm65Profile* pcProfile)
{
	const SIRW65C816Frame*	psFrame;
	SIRW65C816Facts*		psFacts;
	CChars					sz;
	CChars					szAligned;
	CChars					szAlignedEpilogue;
	const char*				szLine;
	const char*				szEnd;
	SIRW65C816Exit*			psExit;
	int32					iRegisterWords;
	int32					iTop;
	int32					iWords;
	int32					iSavedS;
	int32					iCopied;
	int32					iLine;
	int32					iSkip;
	int32					i;
	size					uiExit;
	uint64					ulliCost;
	SAsm65FunctionProfile*	psProfile;
	bool					bBig;
	bool					bWorth;
	bool					bAllRead;
	uint8					abRead[IR816_DIRECT_PAGE];
	bool					abCopy[IR816_DIRECT_PAGE / 2];
	bool					abCopyRegister[IR816_ARGUMENT_REGISTERS];

	psFrame = &pcWritten->msFrame;
	psFacts = &pcWritten->msFacts;
	iTop = psFrame->Top();
	iWords = (psFrame->iArgumentBytes + 1) / 2;
	iRegisterWords = psFrame->iRegisterWords;
	bBig = !IR816OnDirectPage(iTop);
	if ((psFrame->iFrameBytes == 0) || (bBig && (psFrame->iSavedS < 0)) || psFrame->bVariadic || bRecursive)
	{
		return false;
	}

	//Argument words nothing reads aren't copied into the frame (a frame past the page pushes them all into place);
	//nor are register words.
	if (!bBig)
	{
		bAllRead = psFacts->bAllRead;
		memcpy(abRead, psFacts->abRead, sizeof(abRead));
	}
	else
	{
		bAllRead = true;
	}
	iCopied = bBig ? iWords : 0;
	for (i = 0; !bBig && (i < iWords); i++)
	{
		abCopy[i] = bAllRead || abRead[psFrame->ArgumentWord(i)] || abRead[psFrame->ArgumentWord(i) + 1];
		iCopied += abCopy[i] ? 1 : 0;
	}
	for (i = 0; i < iRegisterWords; i++)
	{
		abCopyRegister[i] = bAllRead || abRead[psFrame->RegisterWord(i)] || abRead[psFrame->RegisterWord(i) + 1];
		iCopied += abCopyRegister[i] ? 1 : 0;
	}

	//A call of itself is recursion: of its body, for a register function (its stub, before the body, calls it too).
	if (((iRegisterWords > 0) && !psFacts->bBody) || psFacts->bCallsItself)
	{
		return false;
	}

	ulliCost = (uint64)(IR816_ALIGN_CYCLES + IR816_ALIGN_WORD_CYCLES * iCopied + (bBig ? IR816_ALIGN_BIG_CYCLES : 0));
	if (pcProfile)
	{
		//Measured: never seen recursing, and the accesses well past the cost of its calls.
		psProfile = (iRegisterWords > 0) ? pcProfile->Get(pcWritten->mszBody.Text()) : NULL;
		if (psProfile == NULL)
		{
			psProfile = pcProfile->Get(szLabel);
		}
		bWorth = psProfile && !psProfile->bRecursive && (psProfile->ulliCalls > 0) &&
				 (psProfile->ulliDirectPage >= IR816_ALIGN_GAIN * ulliCost * psProfile->ulliCalls) &&
				 (psProfile->ulliDirectPage - ulliCost * psProfile->ulliCalls >= pcProfile->NumCycles() / IR816_ALIGN_SHARE);
	}
	else
	{
		//Estimated: loops, whose accesses are well past the cost of a call.
		bWorth = psFrame->bLoops && ((uint64)psFrame->iDirectPageWeight >= IR816_ALIGN_GAIN * ulliCost);
	}
	if (!bWorth || (psFacts->iPrologue < 0))
	{
		return false;
	}

	iSavedS = psFrame->SavedS();
	//S goes down before anything is written below where it was (an interrupt pushes there).  A register function's
	//register words are pushed first, as its prologue pushes them (X is then S below them), and copied to the frame's
	//top with the arguments.
	szAligned.Init("\t\tphd\n");
	for (i = iRegisterWords - 1; i >= 0; i--)
	{
		szAligned.Append((i == 2) ? "\t\tphy\n" : ((i == 1) ? "\t\tphx\n" : "\t\tpha\n"));
	}
	szAligned.Append("\t\ttsc\n\t\ttax\n\t\tsec\n\t\tsbc ");
	szAligned.Append(iTop);
	szAligned.Append("\n\t\tand 0xFF00\n");
	if (!bBig)
	{
		szAligned.Append("\t\ttcs\n\t\ttcd\n");
		for (i = 0; i < iRegisterWords; i++)
		{
			if (!abCopyRegister[i])
			{
				continue;
			}
			szAligned.Append("\t\tlda [");
			szAligned.Append(1 + 2 * i);
			szAligned.Append(" + X]:24\n\t\tsta [D + ");
			szAligned.Append(psFrame->RegisterWord(i));
			szAligned.Append("]\n");
		}
		for (i = 0; i < iWords; i++)
		{
			if (!abCopy[i])
			{
				continue;
			}
			szAligned.Append("\t\tlda [");
			szAligned.Append(1 + 2 * iRegisterWords + IR816_LINK_BYTES + 2 * i);
			szAligned.Append(" + X]:24\n\t\tsta [D + ");
			szAligned.Append(psFrame->ArgumentWord(i));
			szAligned.Append("]\n");
		}
	}
	else if ((iWords == 0) && (iRegisterWords == 0))
	{
		szAligned.Append("\t\ttcs\n\t\ttcd\n");
	}
	else
	{
		//Past the page the arguments are pushed into place, last first, from S at their top; below them the link
		//bytes are passed and a register function's register words (the frame's top) pushed the same way; then S is D.
		szAligned.Append("\t\ttcd\n\t\tclc\n\t\tadc ");
		szAligned.Append(iTop);
		szAligned.Append("\n\t\ttcs\n");
		for (i = iWords - 1; i >= 0; i--)
		{
			szAligned.Append("\t\tlda [");
			szAligned.Append(1 + 2 * iRegisterWords + IR816_LINK_BYTES + 2 * i);
			szAligned.Append(" + X]:24\n\t\tpha\n");
		}
		if (iRegisterWords > 0)
		{
			szAligned.Append("\t\ttsc\n\t\tsec\n\t\tsbc ");
			szAligned.Append(IR816_LINK_BYTES);
			szAligned.Append("\n\t\ttcs\n");
			for (i = iRegisterWords - 1; i >= 0; i--)
			{
				szAligned.Append("\t\tlda [");
				szAligned.Append(1 + 2 * i);
				szAligned.Append(" + X]:24\n\t\tpha\n");
			}
		}
		szAligned.Append("\t\ttdc\n\t\ttcs\n");
	}
	if (iRegisterWords > 0)
	{
		//S as it was after phd, above the register words.
		szAligned.Append("\t\ttxa\n\t\tclc\n\t\tadc ");
		szAligned.Append(2 * iRegisterWords);
		szAligned.Append("\n\t\tsta [D + ");
	}
	else
	{
		szAligned.Append("\t\tstx [D + ");
	}
	szAligned.Append(iSavedS);
	szAligned.Append("]\n");
	szAlignedEpilogue.Init("\t\tlda [D + ");
	szAlignedEpilogue.Append(iSavedS);
	szAlignedEpilogue.Append("]\n");

	//The printed code's lines: the entry replaced, each exit's move of S back a load of S as it was (tidied as it
	//was), and with exceptions a clc before the exit's rtl (the carry is no longer cleared by the move).
	sz.Init(CIRW65C816Backend::GetScratch());
	iLine = 0;
	iSkip = 0;
	for (szLine = pszCode->Text(); *szLine != '\0'; szLine = szEnd, iLine++)
	{
		szEnd = strchr(szLine, '\n');
		szEnd = szEnd ? szEnd + 1 : szLine + StrLen(szLine);
		if (IRIsMarkerLine(szLine))
		{
			//Not one of the code's lines the facts count: kept where it is.
			sz.AppendSubString(szLine, szEnd);
			iLine--;
			continue;
		}
		if (iSkip > 0)
		{
			iSkip--;
			continue;
		}
		if (iLine == psFacts->iPrologue)
		{
			sz.Append(szAligned);
			iSkip = psFacts->iPrologueLines - 1;
			continue;
		}
		for (uiExit = 0; uiExit < pcWritten->masExits.NumElements(); uiExit++)
		{
			psExit = pcWritten->masExits.Get(uiExit);
			if (iLine == psExit->iMove)
			{
				sz.Append(szAlignedEpilogue);
				iSkip = 2;
				break;
			}
			if ((iLine == psExit->iReturn) && psExit->bClearBefore && mpcOptions->mbExceptions)
			{
				sz.Append("\t\tclc\n");
			}
		}
		if ((uiExit == pcWritten->masExits.NumElements()) || (iLine != pcWritten->masExits.Get(uiExit)->iMove))
		{
			sz.AppendSubString(szLine, szEnd);
		}
	}
	pszCode->Kill();
	pszCode->Init(sz);
	sz.Kill();

	//A debugger's frame directives go after the aligned entry (its last line stores S as it was).
	psFacts->bAligned = true;
	if (psFacts->iDescribeAfter >= 0)
	{
		psFacts->iDescribeAfter = psFacts->iPrologue + IR816CountLines(szAligned.Text());
	}
	szAligned.Kill();
	szAlignedEpilogue.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// For a debugger: where the function's frame is (.dframe: from D, or
// where an aligned entry kept S as it was, or none when it has no frame or
// one this doesn't know: its caller is then found from S), where the
// caller's D was pushed (.dsaved) and that its variables are from D
// (.dbase), after its frame's entry (or after its label) as KeepFacts
// found it, in the lines of the code without its markers.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::DescribeFrame(CChars* pszCode, const char* szLabel, CIRWrittenFunction* pcWritten)
{
	CIRW65C816Written*	pcWritten816;
	SIRW65C816Facts*	psFacts;
	CChars				szDirective;
	const char*			szLine;
	const char*			szEnd;
	int32				iLine;
	int32				iAt;

	pcWritten816 = (CIRW65C816Written*)pcWritten;
	if (!pcWritten816 || (pcWritten816->msFacts.iDescribeBody < 0) || pszCode->Empty())
	{
		//Nothing kept of it, or its code was taken out since.
		return;
	}
	psFacts = &pcWritten816->msFacts;
	szDirective.Init(CIRW65C816Backend::GetScratch());
	if ((psFacts->iDescribeAfter >= 0) && psFacts->bAligned)
	{
		szDirective.Append("\t\t.dframe [D + ")->Append(pcWritten816->msFrame.SavedS())->Append("], ")->Append(IR816_FRAME_FROM_PHD);
		szDirective.AppendNewLine()->Append("\t\t.dsaved D, ")->Append(-IR816_FRAME_FROM_PHD + 1);
		iAt = psFacts->iDescribeAfter;
	}
	else if (psFacts->iDescribeAfter >= 0)
	{
		szDirective.Append("\t\t.dframe D, ")->Append(psFacts->iDescribeBelow + IR816_FRAME_FROM_PHD);
		szDirective.AppendNewLine()->Append("\t\t.dsaved D, ")->Append(-IR816_FRAME_FROM_PHD + 1);
		iAt = psFacts->iDescribeAfter;
	}
	else
	{
		szDirective.Append("\t\t.dframe none");
		iAt = psFacts->iDescribeBody;
	}
	szDirective.AppendNewLine()->Append("\t\t.dbase D")->AppendNewLine();

	//The directives go straight after line iAt - 1 of the code as it was printed: the lines a debugger's markers made
	//since (source markers, and the labels place markers became: CCPP65IRFrontEnd::DebugRanges) aren't counted.
	iLine = 0;
	for (szLine = pszCode->Text(); *szLine != '\0'; szLine = szEnd)
	{
		szEnd = strchr(szLine, '\n');
		szEnd = szEnd ? szEnd + 1 : szLine + StrLen(szLine);
		if (IRIsMarkerLine(szLine))
		{
			continue;
		}
		if (iLine == iAt)
		{
			break;
		}
		iLine++;
		if (iLine == iAt)
		{
			szLine = szEnd;
			break;
		}
	}
	pszCode->Insert((size)(szLine - pszCode->Text()), &szDirective);
	szDirective.Kill();
}
