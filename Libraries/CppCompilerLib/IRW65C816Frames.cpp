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


//The W65C816 back end's frames: the text of a function's frame entry and exit, taken out when nothing uses the
//frame (DropFrame) and pointed at a page where that saves cycles (AlignFrame).


//////////////////////////////////////////////////////////////////////////
//
// The frame's set up as Prologue writes it (after the label and the
// directives): D saved, S moved down iFrameBytes and D pointed at it.
// DropFrame and AlignFrame find and replace it by this text.
//
//////////////////////////////////////////////////////////////////////////
void IR816AppendFrameEntry(CChars* psz, int32 iFrameBytes)
{
	psz->Append("\t\tphd\n\t\ttsc\n\t\tsec\n\t\tsbc ");
	psz->Append(iFrameBytes);
	psz->Append("\n\t\ttcs\n\t\ttcd\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A register function's frame set up as Prologue writes it (after its
// body's label): D saved, the register words pushed (Y's, X's, then A's,
// so A's is lowest) to be the frame's top, S moved down the rest of the
// frame and D pointed at it.  AlignFrame finds and replaces it.
//
//////////////////////////////////////////////////////////////////////////
void IR816AppendRegisterFrameEntry(CChars* psz, int32 iRegisterWords, int32 iFrameBytes)
{
	int		i;
	int32	iRest;

	psz->Append("\t\tphd\n");
	for (i = iRegisterWords - 1; i >= 0; i--)
	{
		psz->Append((i == 2) ? "\t\tphy\n" : ((i == 1) ? "\t\tphx\n" : "\t\tpha\n"));
	}
	psz->Append("\t\ttsc\n");
	iRest = iFrameBytes - 2 * iRegisterWords;
	if (iRest > 0)
	{
		psz->Append("\t\tsec\n\t\tsbc ");
		psz->Append(iRest);
		psz->Append("\n\t\ttcs\n");
	}
	psz->Append("\t\ttcd\n");
}


//////////////////////////////////////////////////////////////////////////
//
// The frame's removal as Epilogue writes it (before the rtl): A kept in Y
// while S moves back up iFrameBytes, then D restored.  DropFrame and
// AlignFrame find and replace it by this text.
//
//////////////////////////////////////////////////////////////////////////
void IR816AppendFrameExit(CChars* psz, int32 iFrameBytes)
{
	psz->Append("\t\ttay\n\t\ttdc\n\t\tclc\n\t\tadc ");
	psz->Append(iFrameBytes);
	psz->Append("\n\t\ttcs\n\t\ttya\n\t\tpld\n");
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
// interrupt may have pushed over it), and the tay goes.
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
	bool		bConstant;
	bool		bDirect;
	bool		bAny;

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
		szAt = szAt ? IR816ExitLine(szAt, "rtl") : NULL;
		if (!szAt)
		{
			continue;
		}

		//The line before the tay.
		szLoad = NULL;
		bConstant = false;
		bDirect = false;
		if (!bNoResult && (szCode - szCopied >= 2))
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
		if (!bNoResult && !bConstant && !bDirect)
		{
			continue;
		}

		bAny = true;
		if (bNoResult)
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
// Whether the code calls a routine given places in the frame as numbers
// (a 64 bit shift routine's in X, a soft float frame operand entry's in X,
// Y and A), which DropFrame can't move.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816PassesFrameOffsets(const char* szCode)
{
	const char*	szAt;
	const char*	szEnd;

	for (szAt = strstr(szCode, "\tjsl "); szAt; szAt = strstr(szAt + 1, "\tjsl "))
	{
		szAt += 5;
		for (szEnd = szAt; (*szEnd != '\n') && (*szEnd != '\0'); szEnd++)
		{
		}
		if (IR816IndexedRoutine(szAt, (size)(szEnd - szAt)) ||
			((szEnd - szAt > 3) && (strncmp(szEnd - 3, IR816_FRAME_OPERANDS, 3) == 0)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's code from uiStart without its frame when it never reads
// or writes the frame's bytes (D + 1 to D + frame size; its temporaries may
// all have been kept in A or fused) or D but to move S back: S isn't moved
// down and back, so D is S and each D + n is n - frame size.  Without any
// D + n, D isn't saved and set either.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::DropFrame(size uiStart)
{
	CChars		sz;
	CChars		szPrologue;
	CChars		szEpilogue;
	const char*	szCode;
	const char*	szAt;
	int32		iOffset;
	int			iEpilogues;
	int			iTdcs;
	bool		bUsesD;

	szCode = mpszOut->Text() + uiStart;
	if ((msFrame.iFrameBytes == 0) || (strstr(szCode, "[D + X]") != NULL) || IR816PassesFrameOffsets(szCode))
	{
		return;
	}
	for (szAt = strstr(szCode, "D + "); szAt; szAt = strstr(szAt + 4, "D + "))
	{
		if (atoi(szAt + 4) <= msFrame.iFrameBytes)
		{
			return;
		}
	}
	szPrologue.Init(CIRW65C816Backend::GetScratch());
	IR816AppendFrameEntry(&szPrologue, msFrame.iFrameBytes);
	szEpilogue.Init(CIRW65C816Backend::GetScratch());
	IR816AppendFrameExit(&szEpilogue, msFrame.iFrameBytes);
	iEpilogues = 0;
	for (szAt = strstr(szCode, szEpilogue.Text()); szAt; szAt = strstr(szAt + 1, szEpilogue.Text()))
	{
		iEpilogues++;
	}
	iTdcs = 0;
	for (szAt = strstr(szCode, "\t\ttdc\n"); szAt; szAt = strstr(szAt + 1, "\t\ttdc\n"))
	{
		iTdcs++;
	}
	if ((iTdcs != iEpilogues) || (strstr(szCode, szPrologue.Text()) == NULL))
	{
		szPrologue.Kill();
		szEpilogue.Kill();
		return;
	}

	//Without D + anything it needn't set D either.
	bUsesD = strstr(szCode, "D + ") != NULL;
	sz.Init(CIRW65C816Backend::GetScratch());
	while (*szCode)
	{
		if (strncmp(szCode, szPrologue.Text(), szPrologue.Length()) == 0)
		{
			sz.Append(bUsesD ? "\t\tphd\n\t\ttsc\n\t\ttcd\n" : "");
			szCode += szPrologue.Length();
		}
		else if (strncmp(szCode, szEpilogue.Text(), szEpilogue.Length()) == 0)
		{
			sz.Append(bUsesD ? "\t\tpld\n" : "");
			szCode += szEpilogue.Length();
		}
		else if (strncmp(szCode, "D + ", 4) == 0)
		{
			iOffset = atoi(szCode + 4);
			sz.Append("D + ");
			sz.Append(iOffset - msFrame.iFrameBytes);
			for (szCode += 4; (*szCode >= '0') && (*szCode <= '9'); szCode++)
			{
			}
		}
		else
		{
			sz.Append(*szCode);
			szCode++;
		}
	}
	mpszOut->RemoveEnd(uiStart);
	mpszOut->Append(sz);
	sz.Kill();
	szPrologue.Kill();
	szEpilogue.Kill();
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
bool CIRW65C816Backend::AlignFrame(CChars* pszCode, const char* szLabel, const SIRW65C816Frame* psFrame, bool bRecursive, CAsmW65C816Profile* pcProfile)
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
	SAsmW65C816FunctionProfile*	psProfile;
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

	iSavedS = bBig ? psFrame->iSavedS : psFrame->Link();
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

