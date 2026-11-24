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
// The line starting at szLine (to its new line) is the instruction szText
// (without its tabs or new line).  Returns the line after it, or NULL.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816ExitLine(const char* szLine, const char* szText)
{
	size	uiText;

	uiText = strlen(szText);
	if ((szLine[0] != '\t') || (szLine[1] != '\t') || (strncmp(szLine + 2, szText, uiText) != 0) || (szLine[2 + uiText] != '\n'))
	{
		return NULL;
	}
	return szLine + 2 + uiText + 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The line starting at szLine is szMnemonic and a decimal number ("adc 36"),
// or with szOpen and szClose a number between them ("lda [D + 33]").
// Returns the line after it, or NULL.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816ExitNumberLine(const char* szLine, const char* szMnemonic, const char* szOpen, const char* szClose)
{
	size	uiMnemonic;
	size	uiOpen;
	size	uiClose;

	uiMnemonic = strlen(szMnemonic);
	uiOpen = strlen(szOpen);
	uiClose = strlen(szClose);
	if ((szLine[0] != '\t') || (szLine[1] != '\t') || (strncmp(szLine + 2, szMnemonic, uiMnemonic) != 0) || (strncmp(szLine + 2 + uiMnemonic, szOpen, uiOpen) != 0))
	{
		return NULL;
	}
	szLine += 2 + uiMnemonic + uiOpen;
	if ((*szLine < '0') || (*szLine > '9'))
	{
		return NULL;
	}
	while ((*szLine >= '0') && (*szLine <= '9'))
	{
		szLine++;
	}
	if ((strncmp(szLine, szClose, uiClose) != 0) || (szLine[uiClose] != '\n'))
	{
		return NULL;
	}
	return szLine + uiClose + 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The exits of the function in pszCode (after AlignFrame) keep A in Y while
// S moves back only when they must.  An exit is
//
//		tay / tdc / clc / adc n / tcs / tya / pld / rtl
//
// or, with the frame on a page, tay / lda [D + n] / tcs / tya / pld / rtl.
// With nothing returned in A (bNoResult) the tay and tya go.  After an lda
// of a constant the lda is done once S is back instead; after an lda of a
// direct page word it is an ldy (the word can't be read once S is back: an
// interrupt may have pushed over it), and the tay goes.  A throw's exit (a
// sec before the rtl) returns nothing in A whatever the function returns.
//
//////////////////////////////////////////////////////////////////////////
void IR816TidyExits(CChars* pszCode, bool bNoResult)
{
	CChars		sz;
	const char*	szCode;
	const char*	szStart;
	const char*	szAt;
	const char*	szMove;
	const char*	szMoved;
	const char*	szEnd;
	const char*	szLoad;
	const char*	szCopied;
	const char*	szSec;
	bool		bConstant;
	bool		bDirect;
	bool		bAny;
	bool		bNothing;

	szStart = pszCode->Text();
	if (strstr(szStart, "\t\ttay\n") == NULL)
	{
		return;
	}
	sz.Init(CIRW65C816Backend::GetScratch());
	szCopied = szStart;
	bAny = false;
	for (szCode = strstr(szStart, "\t\ttay\n"); szCode; szCode = strstr(szCode + 1, "\t\ttay\n"))
	{
		if ((szCode != szStart) && (szCode[-1] != '\n'))
		{
			continue;
		}
		if (szCode < szCopied)
		{
			continue;
		}

		//S's move back: tdc / clc / adc n / tcs, or lda [D + n] / tcs.
		szMove = szCode + 6;
		szAt = IR816ExitLine(szMove, "tdc");
		if (szAt)
		{
			szAt = IR816ExitLine(szAt, "clc");
			szAt = szAt ? IR816ExitNumberLine(szAt, "adc ", "", "") : NULL;
		}
		else
		{
			szAt = IR816ExitNumberLine(szMove, "lda ", "[D + ", "]");
		}
		szAt = szAt ? IR816ExitLine(szAt, "tcs") : NULL;
		szMoved = szAt;
		szAt = szAt ? IR816ExitLine(szAt, "tya") : NULL;
		szEnd = szAt;
		szAt = szAt ? IR816ExitLine(szAt, "pld") : NULL;
		szSec = szAt ? IR816ExitLine(szAt, "sec") : NULL;
		bNothing = bNoResult || (szSec != NULL);
		szAt = szSec ? szSec : szAt;
		szAt = szAt ? IR816ExitLine(szAt, "rtl") : NULL;
		if (!szAt)
		{
			continue;
		}

		//The line before the tay.
		szLoad = NULL;
		bConstant = false;
		bDirect = false;
		if (!bNothing && (szCode - szCopied >= 2))
		{
			for (szLoad = szCode - 1; (szLoad > szCopied) && (szLoad[-1] != '\n'); szLoad--)
			{
			}
			if (IR816ExitNumberLine(szLoad, "lda ", "", "") == szCode)
			{
				bConstant = true;
			}
			else if (IR816ExitNumberLine(szLoad, "lda ", "[D + ", "]") == szCode)
			{
				bDirect = true;
			}
		}
		if (!bNothing && !bConstant && !bDirect)
		{
			continue;
		}

		bAny = true;
		if (bNothing)
		{
			sz.Append(szCopied, (size)(szCode - szCopied));
			sz.Append(szMove, (size)(szMoved - szMove));
		}
		else if (bConstant)
		{
			sz.Append(szCopied, (size)(szLoad - szCopied));
			sz.Append(szMove, (size)(szMoved - szMove));
			sz.Append(szLoad, (size)(szCode - szLoad));
		}
		else
		{
			sz.Append(szCopied, (size)(szLoad - szCopied));
			sz.Append("\t\tldy ");
			sz.Append(szLoad + 6, (size)(szCode - (szLoad + 6)));
			sz.Append(szMove, (size)(szEnd - szMove));
		}
		szCopied = szEnd;
	}
	if (bAny)
	{
		sz.Append(szCopied);
		pszCode->Kill();
		pszCode->Init(sz);
	}
	sz.Kill();
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
// Gives the function in pszCode (as GenerateFunction wrote it, psFrame from
// GetFrame) D on a page, where its direct page accesses cost a cycle less,
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
bool CIRW65C816Backend::AlignFrame(CChars* pszCode, const char* szLabel, const SIRW65C816Frame* psFrame, bool bRecursive, CAsm65Profile* pcProfile)
{
	CIRW65C816Code				cCode;
	CChars					sz;
	CChars					szPrologue;
	CChars					szEpilogue;
	CChars					szAligned;
	CChars					szAlignedEpilogue;
	CChars					szCall;
	CChars					szBody;
	const char*				szCode;
	const char*				szBodyLabel;
	int32					iRegisterWords;
	int32					iTop;
	int32					iWords;
	int32					iSavedS;
	int32					iCopied;
	int32					i;
	uint64					ulliCost;
	SAsm65FunctionProfile*	psProfile;
	bool					bBig;
	bool					bWorth;
	bool					bAllRead;
	uint8					abRead[IR816_DIRECT_PAGE];
	bool					abCopy[IR816_DIRECT_PAGE / 2];
	bool					abCopyRegister[IR816_ARGUMENT_REGISTERS];

	iTop = psFrame->Top();
	iWords = (psFrame->iArgumentBytes + 1) / 2;
	iRegisterWords = psFrame->iRegisterWords;
	szCode = pszCode->Text();
	szBodyLabel = NULL;
	bBig = !IR816OnDirectPage(iTop);
	if ((psFrame->iFrameBytes == 0) || (bBig && (psFrame->iSavedS < 0)) || psFrame->bVariadic || bRecursive)
	{
		return false;
	}

	//Argument words nothing reads aren't copied into the frame (a frame past the page pushes them all into place);
	//nor are register words.
	if (!bBig)
	{
		cCode.Init();
		cCode.Load(pszCode, 0);
		bAllRead = IR816FindReads(&cCode, 0, abRead);
		cCode.Kill();
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
	szCall.Init("jsl ");
	szCall.Append(szLabel);
	if (iRegisterWords > 0)
	{
		szBody.Init("\n");
		szBody.Append(szLabel);
		szBody.Append('.');
		szBodyLabel = strstr(szCode, szBody.Text());
		szBody.Kill();
		if (szBodyLabel == NULL)
		{
			szCall.Kill();
			return false;
		}
		szCall.Append('.');
		bRecursive = strstr(szBodyLabel, szCall.Text()) != NULL;
	}
	else
	{
		szCall.AppendNewLine();
		bRecursive = strstr(szCode, szCall.Text()) != NULL;
	}
	szCall.Kill();
	if (bRecursive)
	{
		return false;
	}

	ulliCost = (uint64)(IR816_ALIGN_CYCLES + IR816_ALIGN_WORD_CYCLES * iCopied + (bBig ? IR816_ALIGN_BIG_CYCLES : 0));
	if (pcProfile)
	{
		//Measured: never seen recursing, and the accesses well past the cost of its calls.
		psProfile = NULL;
		if (szBodyLabel)
		{
			//The body's label (to the colon).
			szBody.Init(szBodyLabel + 1, 0, (int)(strchr(szBodyLabel + 1, ':') - (szBodyLabel + 1)));
			psProfile = pcProfile->Get(szBody.Text());
			szBody.Kill();
		}
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
	if (!bWorth)
	{
		return false;
	}

	iSavedS = psFrame->SavedS();
	szPrologue.Init(CIRW65C816Backend::GetScratch());
	if (iRegisterWords > 0)
	{
		IR816AppendRegisterFrameEntry(&szPrologue, iRegisterWords, psFrame->iFrameBytes);
	}
	else
	{
		IR816AppendFrameEntry(&szPrologue, psFrame->iFrameBytes);
	}
	szEpilogue.Init(CIRW65C816Backend::GetScratch());
	IR816AppendFrameExit(&szEpilogue, psFrame->iFrameBytes);
	if (strstr(szCode, szPrologue.Text()) == NULL)
	{
		szPrologue.Kill();
		szEpilogue.Kill();
		return false;
	}

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
	szAlignedEpilogue.Init("\t\ttay\n\t\tlda [D + ");
	szAlignedEpilogue.Append(iSavedS);
	szAlignedEpilogue.Append("]\n\t\ttcs\n\t\ttya\n\t\tpld\n");

	sz.Init(CIRW65C816Backend::GetScratch());
	while (*szCode)
	{
		if (strncmp(szCode, szPrologue.Text(), szPrologue.Length()) == 0)
		{
			sz.Append(szAligned);
			szCode += szPrologue.Length();
		}
		else if (strncmp(szCode, szEpilogue.Text(), szEpilogue.Length()) == 0)
		{
			sz.Append(szAlignedEpilogue);
			szCode += szEpilogue.Length();
		}
		else
		{
			sz.Append(*szCode);
			szCode++;
		}
	}
	pszCode->Kill();
	pszCode->Init(sz);
	sz.Kill();
	szAligned.Kill();
	szAlignedEpilogue.Kill();
	szPrologue.Kill();
	szEpilogue.Kill();
	return true;
}



//////////////////////////////////////////////////////////////////////////
//
// The first instruction line at or after szLine (labels, directives and
// comments skipped), NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816Instruction(const char* szLine)
{
	while (szLine && (*szLine != '\0'))
	{
		if ((szLine[0] == '\t') && (szLine[1] == '\t') && (szLine[2] >= 'a') && (szLine[2] <= 'z'))
		{
			return szLine;
		}
		szLine = strchr(szLine, '\n');
		if (!szLine)
		{
			return NULL;
		}
		szLine++;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816NextLine(const char* szLine)
{
	szLine = strchr(szLine, '\n');
	return szLine ? szLine + 1 : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction line is szMnemonic's.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsMnemonic(const char* szLine, const char* szMnemonic)
{
	size	uiLength;

	uiLength = strlen(szMnemonic);
	return (strncmp(szLine + 2, szMnemonic, uiLength) == 0) && ((szLine[2 + uiLength] == ' ') || (szLine[2 + uiLength] == '\n'));
}


//////////////////////////////////////////////////////////////////////////
//
// Where a register entry's body is in szCode (from its plain label on):
// after its own label, szLabel and a suffix (_Add__Fii.ax:), past the
// stub that loads the registers.  MAX_SIZE if it has none.
//
//////////////////////////////////////////////////////////////////////////
static size IR816FindRegisterBody(const char* szCode, const char* szLabel)
{
	const char*	szAt;
	const char*	szEnd;
	size		uiLength;

	uiLength = strlen(szLabel);
	for (szAt = szCode; szAt; szAt = IR816NextLine(szAt))
	{
		if ((strncmp(szAt, szLabel, uiLength) == 0) && (szAt[uiLength] == '.'))
		{
			szEnd = strchr(szAt, '\n');
			if (szEnd && (szEnd[-1] == ':'))
			{
				return (size)(szEnd + 1 - szCode);
			}
		}
		if ((szAt[0] == '\t') && IR816IsMnemonic(szAt, "phd"))
		{
			return MAX_SIZE;
		}
	}
	return MAX_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
// Where in the aligned prologue from szLine S as it was is stored (st? [D +
// iPlace]), the line after it; NULL if it isn't.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816FindSavedS(const char* szLine, int32 iPlace)
{
	CChars		szStore;
	const char*	szAt;

	szStore.Init(CIRW65C816Backend::GetScratch());
	szStore.Append(" [D + ");
	szStore.Append(iPlace);
	szStore.Append("]\n");
	szAt = strstr(szLine, szStore.Text());
	if (szAt && ((strncmp(szAt - 5, "\t\tsta", 5) == 0) || (strncmp(szAt - 5, "\t\tstx", 5) == 0)))
	{
		szAt += szStore.Length();
	}
	else
	{
		szAt = NULL;
	}
	szStore.Kill();
	return szAt;
}


//////////////////////////////////////////////////////////////////////////
//
// For a debugger (the frame model, Debugger65C64Plan.txt), read from its
// finished code: a .dframe where the function's frame is made, its frame
// address (S before the jsl) found from D: after "tcd" for a frame of its
// own, D plus what the prologue pushed and subtracted plus phd's and jsl's
// pushes; after S is kept in an aligned frame (AlignFrame), the S kept
// there plus those; with the caller's D phd saved (.dsaved).  At the
// body's start for code with no frame (.dframe none: DropFrame took it
// away, or it never had one), found by following its code.  Its variables
// are from D (.dbase).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::DescribeFrame(CChars* pszCode, const char* szLabel, CIRWrittenFunction* pcWritten)
{
	const char*		szCode;
	const char*		szLine;
	const char*		szAfter;
	SIRW65C816Frame	sFrame;
	CChars			szDirective;
	size			uiBody;
	size			uiAt;
	int32			iBelow;
	bool			bAligned;

	szCode = pszCode->Text();
	uiBody = CCPP65DebugWriter::FindAfterLabel(szCode, szLabel);
	if (uiBody == MAX_SIZE)
	{
		return;
	}
	uiAt = IR816FindRegisterBody(szCode + uiBody, szLabel);
	if (uiAt != MAX_SIZE)
	{
		uiBody += uiAt;
	}

	szDirective.Init(CIRW65C816Backend::GetScratch());
	szAfter = NULL;
	szLine = IR816Instruction(szCode + uiBody);
	if (szLine && IR816IsMnemonic(szLine, "phd"))
	{
		iBelow = 0;
		bAligned = false;
		for (szLine = IR816Instruction(IR816NextLine(szLine)); szLine && !IR816IsMnemonic(szLine, "tcd"); szLine = IR816Instruction(IR816NextLine(szLine)))
		{
			if (IR816IsMnemonic(szLine, "pha") || IR816IsMnemonic(szLine, "phx") || IR816IsMnemonic(szLine, "phy"))
			{
				iBelow += 2;
			}
			else if (IR816IsMnemonic(szLine, "sbc"))
			{
				iBelow += atoi(szLine + 6);
			}
			else if (IR816IsMnemonic(szLine, "and"))
			{
				bAligned = true;
			}
		}
		if (szLine && bAligned && pcWritten)
		{
			sFrame = ((CIRW65C816Written*)pcWritten)->msFrame;
			iBelow = sFrame.SavedS();
			szAfter = IR816FindSavedS(szLine, iBelow);
			szDirective.Append("\t\t.dframe [D + ")->Append(iBelow)->Append("], ")->Append(IR816_FRAME_FROM_PHD);
		}
		else if (szLine && !bAligned)
		{
			szAfter = IR816NextLine(szLine);
			szDirective.Append("\t\t.dframe D, ")->Append(iBelow + IR816_FRAME_FROM_PHD);
		}
		szDirective.AppendNewLine()->Append("\t\t.dsaved D, ")->Append(-IR816_FRAME_FROM_PHD + 1);
	}
	if (szAfter)
	{
		uiAt = (size)(szAfter - szCode);
	}
	else
	{
		//No frame, or one this doesn't know: its caller is found from S.
		szDirective.Kill();
		szDirective.Init(CIRW65C816Backend::GetScratch());
		szDirective.Append("\t\t.dframe none");
		uiAt = uiBody;
	}
	szDirective.AppendNewLine()->Append("\t\t.dbase D")->AppendNewLine();
	pszCode->Insert(uiAt, &szDirective);
	szDirective.Kill();
}
