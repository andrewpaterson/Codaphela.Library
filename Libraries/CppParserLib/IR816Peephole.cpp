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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IR816BackendInternal.h"


//The W65C816 back end's passes over straight line code that take out what isn't needed (Peephole,
//PeepholeStores, PeepholeTransfers, PeepholeRegisters and PeepholeMasks).


//////////////////////////////////////////////////////////////////////////
//
// Whether the operand is the accumulator ("asl A").
//
//////////////////////////////////////////////////////////////////////////
static bool IR816OperandIsA(SIR816Line* psLine)
{
	return (psLine->uiOperand == 1) && (psLine->szOperand[0] == 'A');
}


//////////////////////////////////////////////////////////////////////////
//
// The end of the last label line after the line ending at szEnd (labels
// change nothing on the way through), or szEnd when none follows.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816SkipLabels(const char* szEnd)
{
	const char*	szNext;

	while ((*szEnd == '\n') && (szEnd[1] != '\0') && (szEnd[1] != '\t'))
	{
		szNext = strchr(szEnd + 1, '\n');
		if (szNext == NULL)
		{
			break;
		}
		szEnd = szNext;
	}
	return szEnd;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the next instruction after szEnd (the end of a line, past any
// labels) branches on Z or N.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816BranchesOnZN(const char* szEnd)
{
	SIR816Line	sLine;

	szEnd = IR816SkipLabels(szEnd);
	if ((*szEnd != '\n') || !IR816IsInstruction(szEnd + 1))
	{
		return false;
	}
	IR816Decode(szEnd + 1, &sLine);
	return (sLine.uiFlags & IR816MF_BRANCH_ZN) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The first instruction at or after szLine, past labels and directives
// (.width follows every label); NULL at the end.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816NextInstruction(const char* szLine)
{
	const char*	szEnd;

	while (szLine && *szLine)
	{
		if ((szLine[0] == '\t') && (szLine[1] == '\t') && (szLine[2] != '.') && (szLine[2] != '\0') && (szLine[2] != '\n'))
		{
			return szLine;
		}
		szEnd = strchr(szLine, '\n');
		szLine = szEnd ? szEnd + 1 : NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the run of conditional branches from the instruction at or after
// szLine (past labels and directives) has a branch on the carry: a 32 bit
// unsigned compare branches on Z then C ("L: beq L2 / bcs L3").
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CarryReadAt(const char* szLine)
{
	const char*	szEnd;
	SIR816Line	sLine;

	for (szLine = IR816NextInstruction(szLine); szLine; szLine = IR816NextInstruction(szEnd + 1))
	{
		IR816Decode(szLine, &sLine);
		if (sLine.uiFlags & IR816MF_BRANCH_C)
		{
			return true;
		}
		if (!(sLine.uiFlags & IR816MF_BRANCH) || (sLine.uiFlags & IR816MF_ENDS_FLOW) || (sLine.uiMnemonic == IR816M_Unknown))
		{
			//Not a conditional branch.
			return false;
		}
		szEnd = strchr(szLine, '\n');
		if (szEnd == NULL)
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the carry may be read after the branch on the line after szEnd:
// after it, or where it branches to (IR816CarryReadAt; branching on the
// carry is the only way the code reads a carry it didn't set).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CarryReadAfterBranch(const char* szText, const char* szEnd)
{
	const char*	szBranch;
	const char*	szAfter;
	const char*	szLabel;
	const char*	szFound;
	char		szTarget[40];
	size		uiLabel;

	szBranch = IR816SkipLabels(szEnd) + 1;
	szAfter = strchr(szBranch, '\n');
	if ((szAfter == NULL) || IR816CarryReadAt(szAfter + 1))
	{
		return true;
	}

	//"\t\tbne L63": the label's line is "L63:".
	szLabel = szBranch + 6;
	uiLabel = (size)(szAfter - szLabel);
	if ((uiLabel == 0) || (uiLabel + 3 > sizeof(szTarget)))
	{
		return true;
	}
	szTarget[0] = '\n';
	memcpy(&szTarget[1], szLabel, uiLabel);
	szTarget[1 + uiLabel] = ':';
	szTarget[2 + uiLabel] = '\n';
	szTarget[3 + uiLabel] = '\0';
	szFound = strstr(szText, szTarget);
	if (szFound == NULL)
	{
		return true;
	}
	return IR816CarryReadAt(szFound + uiLabel + 3);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction sets N and Z from all of A (as wide as it is).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SetsZNFromA(SIR816Line* psLine)
{
	if (psLine->uiFlags & IR816MF_ZN_FROM_A)
	{
		return true;
	}
	return (psLine->uiFlags & IR816MF_SHIFT_STEP) && IR816OperandIsA(psLine);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the carry may be read after the line at szLine before an
// instruction sets it again without reading it (a label, a directive, a
// branch, a jump, a return, a width change or the end might lead to a
// read).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CarryReadLater(const char* szLine)
{
	SIR816Line	sLine;

	for (szLine = IR816NextLine(szLine); szLine && *szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return true;
		}
		IR816Decode(szLine, &sLine);
		if ((sLine.uiFlags & IR816MF_SETS_CARRY) && !(sLine.uiFlags & IR816MF_READS_CARRY))
		{
			return false;
		}
		if (sLine.uiFlags & (IR816MF_LEAVES | IR816MF_READS_CARRY | IR816MF_WIDTH))
		{
			return true;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the "and 0xFF" before szLine is done for nothing: A's low byte is
// all that is used before the next "and 0xFF" masks it again, as only clc,
// sec, adc, sbc, ora, eor, and and asl A come between (their low bytes
// don't depend on the high bytes), and the carry those leave isn't read
// after it (IR816CarryReadLater).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816MaskRedone(const char* szLine)
{
	SIR816Line	sLine;
	bool		bCarry;

	bCarry = false;
	for (; szLine && *szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
		if ((sLine.uiMnemonic == IR816M_And) && IR816OperandIs(&sLine, "0xFF", 4))
		{
			return !bCarry || !IR816CarryReadLater(szLine);
		}
		if ((sLine.uiMnemonic == IR816M_Adc) || (sLine.uiMnemonic == IR816M_Sbc) || ((sLine.uiMnemonic == IR816M_Asl) && IR816OperandIsA(&sLine)))
		{
			bCarry = true;
		}
		else if (!(sLine.uiFlags & (IR816MF_CARRY | IR816MF_LOGIC)))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the line at szLine is "uiMnemonic szOperand" (szOperand as long
// as uiOperand).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LineIs(const char* szLine, uint16 uiMnemonic, const char* szOperand, size uiOperand)
{
	SIR816Line	sLine;

	if (!szLine || !IR816IsInstruction(szLine))
	{
		return false;
	}
	IR816Decode(szLine, &sLine);
	return (sLine.uiMnemonic == uiMnemonic) && IR816OperandIs(&sLine, szOperand, uiOperand);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the operand is indexed by c ('X' or 'Y': "+ X" or ", X").
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IndexedBy(const char* szOperand, size uiOperand, char c)
{
	size	ui;

	for (ui = 0; ui + 2 < uiOperand; ui++)
	{
		if (((szOperand[ui] == '+') || (szOperand[ui] == ',')) && (szOperand[ui + 1] == ' ') && (szOperand[ui + 2] == c))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The line "ldy w" after szLine (not the next line), when the lines from
// szLine to it are straight line code that neither uses Y nor names a byte
// near w; else NULL.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816LoadYLater(const char* szLine, const char* szWord, size uiWord)
{
	SIR816Line	sLine;
	const char*	szFirst;

	szFirst = szLine;
	for (; szLine && *szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine) || !strchr(szLine, '\n'))
		{
			return NULL;
		}
		IR816Decode(szLine, &sLine);
		if ((sLine.uiMnemonic == IR816M_Ldy) && IR816OperandIs(&sLine, szWord, uiWord))
		{
			return (szLine == szFirst) ? NULL : szLine;
		}
		if ((sLine.uiFlags & (IR816MF_LEAVES | IR816MF_INTERRUPT | IR816MF_SETS_D | IR816MF_USES_Y)) || IR816IndexedBy(sLine.szOperand, sLine.uiOperand, 'Y') ||
			IR816MentionsNear(sLine.szOperand, sLine.uiOperand, atoi(szWord + 5)))
		{
			return NULL;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Stores of A to a direct page word w that the next lines read back,
// after Peephole:
//   sta w / dec w / lda w  ->  dec A / sta w  (inc the same)
//   sta w / ldy w          ->  sta w / tay    (ldx: tax)
// and the store is left out too when w is written again before it is read
// (IR816DeadStore).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeStores(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	const char*	szNext;
	const char*	szAfter;
	SIR816Line	sLine;
	bool		bDirect;

	szOut.Init(CIR816Backend::GetScratch());
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		bDirect = false;
		if (IR816IsInstruction(szLine))
		{
			IR816Decode(szLine, &sLine);
			bDirect = (sLine.uiMnemonic == IR816M_Sta) && sLine.bDirect && (*szEnd == '\n');
		}
		szNext = bDirect ? IR816NextLine(szLine) : NULL;
		if (szNext && (IR816LineIs(szNext, IR816M_Dec, sLine.szOperand, sLine.uiOperand) || IR816LineIs(szNext, IR816M_Inc, sLine.szOperand, sLine.uiOperand)) &&
			IR816LineIs(IR816NextLine(szNext), IR816M_Lda, sLine.szOperand, sLine.uiOperand))
		{
			szAfter = IR816NextLine(IR816NextLine(szNext));
			szOut.Append("\t\t")->Append((szNext[2] == 'd') ? "dec" : "inc")->Append(" A\n");
			if (!szAfter || !IR816DeadStore(szAfter, sLine.szOperand, sLine.uiOperand))
			{
				szOut.Append(szLine, (size)(szEnd - szLine));
				szOut.AppendNewLine();
			}
			szEnd = strchr(IR816NextLine(szNext), '\n');
			if (!szEnd)
			{
				szEnd = szLine + StrLen(szLine);
			}
			continue;
		}
		szAfter = szNext ? IR816LoadYLater(szNext, sLine.szOperand, sLine.uiOperand) : NULL;
		if (szAfter && !IR816ZNReadAfter(szLine) && !IR816ZNReadAfter(szAfter))
		{
			//sta w ... ldy w (Y and w untouched between): tay at once.
			if (!IR816NextLine(szAfter) || !IR816DeadStore(IR816NextLine(szAfter), sLine.szOperand, sLine.uiOperand))
			{
				szOut.Append(szLine, (size)(szEnd - szLine));
				szOut.AppendNewLine();
			}
			szOut.Append("\t\ttay\n");
			for (szNext = IR816NextLine(szLine); szNext != szAfter; szNext = IR816NextLine(szNext))
			{
				szOut.Append(szNext, (size)(strchr(szNext, '\n') - szNext));
				szOut.AppendNewLine();
			}
			szEnd = IR816LineEnd(szAfter);
			continue;
		}
		if (szNext && (IR816LineIs(szNext, IR816M_Ldy, sLine.szOperand, sLine.uiOperand) || IR816LineIs(szNext, IR816M_Ldx, sLine.szOperand, sLine.uiOperand)))
		{
			szAfter = IR816NextLine(szNext);
			if (!szAfter || !IR816DeadStore(szAfter, sLine.szOperand, sLine.uiOperand))
			{
				szOut.Append(szLine, (size)(szEnd - szLine));
				szOut.AppendNewLine();
			}
			szOut.Append((szNext[4] == 'y') ? "\t\ttay\n" : "\t\ttax\n");
			szEnd = IR816LineEnd(szNext);
			continue;
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut);
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction reads X: a transfer, store, step, compare or push
// of it (IR816MF_READS_X), or an X indexed operand.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ReadsX(SIR816Line* psLine)
{
	return (psLine->uiFlags & IR816MF_READS_X) || IR816IndexedBy(psLine->szOperand, psLine->uiOperand, 'X');
}


//////////////////////////////////////////////////////////////////////////
//
// Whether X is written before it is read after the line at szLine (tax,
// ldx, plx or tyx first, in straight line code).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816XDeadAfter(const char* szLine)
{
	SIR816Line	sLine;

	for (szLine = IR816NextLine(szLine); szLine && *szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
		if (IR816ReadsX(&sLine))
		{
			return false;
		}
		if (sLine.uiFlags & IR816MF_WRITES_X)
		{
			return true;
		}
		if (sLine.uiFlags & (IR816MF_LEAVES | IR816MF_INTERRUPT))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// tax ... txy is tay ... when the instructions between don't use X or Y
// (and don't leave the straight line code), X is written again before it
// is read after the txy (IR816XDeadAfter) and nothing reads the N and Z
// the txy set (IR816ZNReadAfter).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeTransfers(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	const char*	szScan;
	const char*	szTransfer;
	SIR816Line	sLine;

	szOut.Init(CIR816Backend::GetScratch());
	szTransfer = NULL;
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		if (szLine == szTransfer)
		{
			//The txy that the tay before made.
			szTransfer = NULL;
			continue;
		}
		if (!szTransfer && ((size)(szEnd - szLine) == 5) && (strncmp(szLine, "\t\ttax", 5) == 0) && (*szEnd == '\n'))
		{
			for (szScan = IR816NextLine(szLine); szScan && *szScan; szScan = IR816NextLine(szScan))
			{
				if (!IR816IsInstruction(szScan))
				{
					break;
				}
				IR816Decode(szScan, &sLine);
				if (sLine.uiMnemonic == IR816M_Txy)
				{
					if (IR816XDeadAfter(szScan) && !IR816ZNReadAfter(szScan))
					{
						szTransfer = szScan;
					}
					break;
				}
				if ((sLine.uiFlags & (IR816MF_LEAVES | IR816MF_READS_X | IR816MF_WRITES_X | IR816MF_USES_Y)) || (sLine.uiMnemonic == IR816M_Tsx) ||
					IR816IndexedBy(sLine.szOperand, sLine.uiOperand, 'X') || IR816IndexedBy(sLine.szOperand, sLine.uiOperand, 'Y'))
				{
					break;
				}
			}
			if (szTransfer)
			{
				szOut.Append("\t\ttay\n");
				continue;
			}
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut);
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A load of what the register already holds (a direct page word or a
// constant, from a load, store or transfer earlier in the same straight
// line code) taken out of the code from uiStart, when nothing reads the N
// and Z it sets (IR816ZNReadAfter).  What a register holds is forgotten
// when the register may change, when the word is written (a write through
// a pointer or an index may write any of the frame's words), and at labels,
// directives, calls, jumps and width changes.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeRegisters(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	SIR816Line	sLine;
	SIR816Held	asHeld[3];
	int			iRegister;

	szOut.Init(CIR816Backend::GetScratch());
	IR816ForgetHeld(asHeld, 0, 2);
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		if (!IR816IsInstruction(szLine))
		{
			IR816ForgetHeld(asHeld, 0, 2);
		}
		else if (szLine[2] != ';')
		{
			IR816Decode(szLine, &sLine);
			iRegister = (sLine.uiMnemonic == IR816M_Lda) ? 0 : (sLine.uiMnemonic == IR816M_Ldx) ? 1 : (sLine.uiMnemonic == IR816M_Ldy) ? 2 : -1;
			if ((iRegister >= 0) && IR816Holds(asHeld, iRegister, &sLine) && !IR816ZNReadAfter(szLine))
			{
				continue;
			}
			IR816UpdateHeld(asHeld, &sLine);
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut);
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The "and 0xFF"s done for nothing (IR816MaskRedone) taken out of the code
// from uiStart, after Peephole has kept values in A (so only the masking
// is left between two masks).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeMasks(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;

	szOut.Init(CIR816Backend::GetScratch());
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		if (((size)(szEnd - szLine) == 10) && (strncmp(szLine, "\t\tand 0xFF", 10) == 0) && (*szEnd == '\n') && IR816MaskRedone(szEnd + 1))
		{
			continue;
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut);
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the operand names szWord anywhere in it.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816OperandMentions(SIR816Line* psLine, const char* szWord, size uiWord)
{
	size	ui;

	for (ui = 0; ui + uiWord <= psLine->uiOperand; ui++)
	{
		if (strncmp(psLine->szOperand + ui, szWord, uiWord) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value stored to the word szWord (as "[D + 12]") by the line
// before szLine can stay in A instead: the next line that mentions the word
// loads it (and nothing after the load reads the N and Z it sets) or pushes
// it (pei, which becomes pha), and nothing before that changes A or goes
// anywhere (only clc, sec, ldx, ldy, stores, tax, tay, pha, pea, pei, phk and
// cmp come between).  Its one use (Store) is that load or push.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816KeepsInA(const char* szLine, const char* szWord, size uiWord)
{
	SIR816Line	sLine;

	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
		if (IR816OperandMentions(&sLine, szWord, uiWord))
		{
			return (sLine.uiOperand == uiWord) && (((sLine.uiMnemonic == IR816M_Lda) && !IR816ZNReadAfter(szLine)) || (sLine.uiMnemonic == IR816M_Pei));
		}
		if (!(sLine.uiFlags & (IR816MF_CARRY | IR816MF_LOAD_INDEX | IR816MF_STORE | IR816MF_PUSH_WORD | IR816MF_COPIES_A)) && (sLine.uiMnemonic != IR816M_Cmp))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value stored to the word szWord by the line before szLine can
// stay in A as the first operand of a commutative instruction instead (adc,
// and, ora or eor of the word straight after an lda of something else, with
// only clc or sec before it): *pszLoad is that lda's line and *pszUse the
// instruction's, which becomes the instruction on the lda's operand.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SwapsInA(const char* szLine, const char* szWord, size uiWord, const char** pszLoad, const char** pszUse)
{
	const char*	szLoad;
	SIR816Line	sLine;

	while (szLine && ((strncmp(szLine, "\t\tclc\n", 6) == 0) || (strncmp(szLine, "\t\tsec\n", 6) == 0)))
	{
		szLine = IR816NextLine(szLine);
	}
	if (!szLine || (strncmp(szLine, "\t\tlda ", 6) != 0))
	{
		return false;
	}
	szLoad = szLine;
	IR816Decode(szLine, &sLine);
	if (IR816OperandMentions(&sLine, szWord, uiWord))
	{
		return false;
	}
	if ((sLine.uiOperand == 0) || (sLine.uiOperand > 40) || (sLine.szOperand[sLine.uiOperand - 1] == 'A') || (strchr(sLine.szOperand, ';') && (strchr(sLine.szOperand, ';') < sLine.szOperand + sLine.uiOperand)))
	{
		return false;
	}
	szLine = IR816NextLine(szLine);
	if (!szLine || ((szLine[0] != '\t') || (szLine[1] != '\t')))
	{
		return false;
	}
	IR816Decode(szLine, &sLine);
	if ((!(sLine.uiFlags & IR816MF_LOGIC) && (sLine.uiMnemonic != IR816M_Adc)) || !IR816OperandIs(&sLine, szWord, uiWord))
	{
		return false;
	}
	*pszLoad = szLoad;
	*pszUse = szLine;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether A and the direct page word it holds (Peephole) stay as they are
// after the instruction: it changes neither (ldx, ldy, clc, sec, cmp, tax,
// tay, pha, pea, per, phk and the branches on Z, N and C), or it changes
// memory other than that word (a store, or a shift or step of memory; a
// store through a pointer might be to it, an address taken local).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816KeepsAWord(SIR816Line* psLine, const char* szA)
{
	if ((psLine->uiFlags & (IR816MF_LOAD_INDEX | IR816MF_CARRY | IR816MF_COPIES_A | IR816MF_BRANCH_ZN | IR816MF_BRANCH_C)) ||
		(psLine->uiMnemonic == IR816M_Cmp) || (psLine->uiMnemonic == IR816M_Pea) || (psLine->uiMnemonic == IR816M_Per) || (psLine->uiMnemonic == IR816M_Phk))
	{
		return true;
	}
	if ((psLine->uiFlags & (IR816MF_STORE | IR816MF_SHIFT_STEP)) && !IR816OperandIsA(psLine))
	{
		return psLine->bDirect && !IR816OperandIs(psLine, szA, StrLen(szA));
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the constant Y holds (Peephole) is changed by the instruction:
// it writes Y (IR816MF_WRITES_Y), calls, changes the widths, or is a bra,
// brl, jml or rtl.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ChangesYConstant(SIR816Line* psLine)
{
	return (psLine->uiFlags & (IR816MF_WRITES_Y | IR816MF_CALL | IR816MF_WIDTH)) ||
		   (psLine->uiMnemonic == IR816M_Rtl) || (psLine->uiMnemonic == IR816M_Brl) || (psLine->uiMnemonic == IR816M_Bra) || (psLine->uiMnemonic == IR816M_Jml);
}


//////////////////////////////////////////////////////////////////////////
//
// Removes instructions that change nothing from the code written from
// uiStart on, in straight line code (a label, a call or a width change
// forgets what is known): a value's store and its one load when A keeps it
// between them (IR816KeepsInA, for a store Store marked), an lda of the
// direct page word A already has (but
// not before a branch on its Z or N), an sta of it back to that word, an lda
// of a direct page word or a constant that the next lda replaces, an ldy of
// the constant Y already has, adc 0 after clc and an lda (the carry stays
// clear), and cmp 0 after an instruction that set N and Z from A when only a
// branch on Z or N follows (not a branch on the carry it sets, after the
// branch or where it goes).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Peephole(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szText;
	const char*	szLine;
	const char*	szEnd;
	SIR816Line	sLine;
	char		szA[32];		//The direct page operand A holds ("[D + 12]"), or empty.
	char		szKept[32];		//The word whose store was left out, its value kept in A for its next lda, or empty.
	const char*	szSwapLoad;		//An lda left out: the value kept in A is the first operand instead (IR816SwapsInA)...
	const char*	szSwapUse;		//...of this instruction, which takes the lda's operand.
	char		szSwapped[64];
	char		szY[32];		//The constant Y holds, or empty.
	size		uiLength;
	size		uiDeadLoad;		//Where in szOut the last line starts if it is an lda of a direct page word or a constant, else -1.
	bool		bClearCarry;
	bool		bZNFromA;		//The last instruction kept set N and Z from A.
	bool		bKeep;

	szText = pszDest->Text() + uiStart;
	szOut.Init(CIR816Backend::GetScratch());
	szA[0] = '\0';
	szY[0] = '\0';
	szKept[0] = '\0';
	szSwapLoad = NULL;
	szSwapUse = NULL;
	bClearCarry = false;
	bZNFromA = false;
	uiDeadLoad = (size)-1;
	for (szLine = szText; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		uiLength = (size)(szEnd - szLine);
		bKeep = true;

		//A label, or a directive: nothing is known after it.
		if ((uiLength < 3) || !IR816IsInstruction(szLine))
		{
			szA[0] = '\0';
			szY[0] = '\0';
			bClearCarry = false;
			bZNFromA = false;
			uiDeadLoad = (size)-1;
			szOut.Append(szLine, uiLength);
			szOut.AppendNewLine();
			continue;
		}

		//The mnemonic and the operand.
		IR816Decode(szLine, &sLine);
		if (sLine.bMarked)
		{
			uiLength -= StrLen(IR816_KEEP_IN_A);
		}

		//A value kept in A instead of its store and load, or as the first operand of a commutative instruction.
		if (sLine.bMarked && sLine.bDirect && (sLine.uiMnemonic == IR816M_Sta) && (*szEnd == '\n') && IR816KeepsInA(szEnd + 1, sLine.szOperand, sLine.uiOperand))
		{
			memcpy(szKept, sLine.szOperand, sLine.uiOperand);
			szKept[sLine.uiOperand] = '\0';
			continue;
		}
		if (sLine.bMarked && sLine.bDirect && (szKept[0] == '\0') && (sLine.uiMnemonic == IR816M_Sta) && (*szEnd == '\n') &&
			IR816SwapsInA(szEnd + 1, sLine.szOperand, sLine.uiOperand, &szSwapLoad, &szSwapUse))
		{
			continue;
		}
		if (szLine == szSwapLoad)
		{
			//The lda's operand goes to the instruction it was loaded for.
			snprintf(szSwapped, sizeof(szSwapped), "%.*s", (int)sLine.uiOperand, sLine.szOperand);
			szSwapLoad = NULL;
			continue;
		}
		if (szLine == szSwapUse)
		{
			szOut.Append("\t\t");
			szOut.Append(sLine.szMnemonic);
			szOut.Append(" ");
			szOut.Append(szSwapped);
			szOut.AppendNewLine();
			szSwapUse = NULL;
			szA[0] = '\0';
			bClearCarry = false;
			bZNFromA = true;
			uiDeadLoad = (size)-1;
			continue;
		}
		if (((sLine.uiMnemonic == IR816M_Lda) || (sLine.uiMnemonic == IR816M_Pei)) && IR816OperandIs(&sLine, szKept, StrLen(szKept)))
		{
			szKept[0] = '\0';
			if (sLine.uiMnemonic == IR816M_Pei)
			{
				szOut.Append("\t\tpha\n");
				uiDeadLoad = (size)-1;
			}
			continue;
		}
		if ((sLine.uiMnemonic == IR816M_Pei) && IR816OperandIs(&sLine, szA, StrLen(szA)))
		{
			//A holds the word: pha pushes it for less.
			szOut.Append("\t\tpha\n");
			uiDeadLoad = (size)-1;
			continue;
		}

		if ((sLine.uiMnemonic == IR816M_Lda) && sLine.bDirect && IR816OperandIs(&sLine, szA, StrLen(szA)) && !IR816BranchesOnZN(szEnd))
		{
			bKeep = false;
		}
		else if ((sLine.uiMnemonic == IR816M_Ldy) && (sLine.uiOperand < 30) && IR816OperandIs(&sLine, szY, StrLen(szY)))
		{
			bKeep = false;
		}
		else if ((sLine.uiMnemonic == IR816M_Adc) && IR816OperandIs(&sLine, "0", 1) && bClearCarry)
		{
			bKeep = false;
		}
		else if ((sLine.uiMnemonic == IR816M_Sta) && sLine.bDirect && IR816OperandIs(&sLine, szA, StrLen(szA)))
		{
			//The word already holds A.
			bKeep = false;
		}
		else if ((sLine.uiMnemonic == IR816M_Cmp) && IR816OperandIs(&sLine, "0", 1) && bZNFromA && IR816BranchesOnZN(szEnd) &&
				 !IR816CarryReadAfterBranch(szText, szEnd))
		{
			bKeep = false;
		}
		if (!bKeep)
		{
			continue;
		}
		if ((sLine.uiMnemonic == IR816M_Lda) && (uiDeadLoad != (size)-1))
		{
			//The lda before it loaded A for nothing.
			szOut.SetLength(uiDeadLoad);
		}
		uiDeadLoad = ((sLine.uiMnemonic == IR816M_Lda) && (sLine.bDirect || IR816IsDecimal(sLine.szOperand))) ? szOut.Length() : (size)-1;
		szOut.Append(szLine, uiLength);
		szOut.AppendNewLine();
		if (!(sLine.uiFlags & IR816MF_STORE) && (sLine.uiMnemonic != IR816M_Pha))
		{
			//Stores change neither A nor the flags.
			bZNFromA = IR816SetsZNFromA(&sLine);
		}

		//What A and Y hold after it.
		if (((sLine.uiMnemonic == IR816M_Lda) || (sLine.uiMnemonic == IR816M_Sta)) && sLine.bDirect)
		{
			memcpy(szA, sLine.szOperand, sLine.uiOperand);
			szA[sLine.uiOperand] = '\0';
		}
		else if (!IR816KeepsAWord(&sLine, szA))
		{
			szA[0] = '\0';
		}

		if ((sLine.uiMnemonic == IR816M_Ldy) && (sLine.uiOperand < 30) && IR816IsDecimal(sLine.szOperand))
		{
			memcpy(szY, sLine.szOperand, sLine.uiOperand);
			szY[sLine.uiOperand] = '\0';
		}
		else if (IR816ChangesYConstant(&sLine))
		{
			szY[0] = '\0';
		}
		if (sLine.uiFlags & (IR816MF_WIDTH | IR816MF_CALL | IR816MF_SETS_D))
		{
			szA[0] = '\0';
		}
		bClearCarry = (sLine.uiMnemonic == IR816M_Clc) || (bClearCarry && (sLine.uiMnemonic == IR816M_Lda));
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut.Text(), szOut.Length());
	szOut.Kill();
	RemoveDeadStores(pszDest, uiStart);
}
