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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IR816BackendInternal.h"
#include "IRW65C816Code.h"


//The W65C816 back end's passes over straight line code that take out what isn't needed (Peephole,
//PeepholeStores, PeepholeTransfers, PeepholeRegisters and PeepholeMasks), on the decoded code (CIRW65C816Code).  A pass
//decides from the code as it was when it began (a line it takes out still counts for the lines before it): it marks
//the lines to take out, or writes its new code when it puts lines in.


#define IR816_NO_LINE	((size)-1)


//////////////////////////////////////////////////////////////////////////
//
// Whether the operand is the accumulator ("asl A").
//
//////////////////////////////////////////////////////////////////////////
static bool IR816OperandIsA(SIRW65C816Line* psLine)
{
	return (psLine->uiOperand == 1) && (psLine->szOperand[0] == 'A');
}


//////////////////////////////////////////////////////////////////////////
//
// The last line of the labels after line uiLine (any line that doesn't start
// with a tab: labels change nothing on the way through), or uiLine when none
// follows.
//
//////////////////////////////////////////////////////////////////////////
static size IR816SkipLabels(CIRW65C816Code* pcCode, size uiLine)
{
	while ((uiLine + 1 < pcCode->NumLines()) && (pcCode->Text(uiLine + 1)[0] != '\t'))
	{
		uiLine++;
	}
	return uiLine;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the next instruction after line uiLine (past any labels) branches
// on Z or N.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816BranchesOnZN(CIRW65C816Code* pcCode, size uiLine)
{
	SIRW65C816Line	sLine;

	uiLine = IR816SkipLabels(pcCode, uiLine) + 1;
	if ((uiLine >= pcCode->NumLines()) || !pcCode->Instruction(uiLine, &sLine))
	{
		return false;
	}
	return (sLine.uiFlags & IR816MF_BRANCH_ZN) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The first instruction at or after line uiLine, past labels and directives
// (.width follows every label); IR816_NO_LINE at the end.
//
//////////////////////////////////////////////////////////////////////////
static size IR816NextInstruction(CIRW65C816Code* pcCode, size uiLine)
{
	const char*	szLine;

	for (; uiLine < pcCode->NumLines(); uiLine++)
	{
		szLine = pcCode->Text(uiLine);
		if ((szLine[0] == '\t') && (szLine[1] == '\t') && (szLine[2] != '.') && (szLine[2] != '\0'))
		{
			return uiLine;
		}
	}
	return IR816_NO_LINE;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the run of conditional branches from the instruction at or after
// line uiLine (past labels and directives) has a branch on the carry: a 32
// bit unsigned compare branches on Z then C ("L: beq L2 / bcs L3").
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CarryReadAt(CIRW65C816Code* pcCode, size uiLine)
{
	SIRW65C816Line	sLine;

	for (uiLine = IR816NextInstruction(pcCode, uiLine); uiLine != IR816_NO_LINE; uiLine = IR816NextInstruction(pcCode, uiLine + 1))
	{
		IR816Decode(pcCode->Text(uiLine), &sLine);
		if (sLine.uiFlags & IR816MF_BRANCH_C)
		{
			return true;
		}
		if (!(sLine.uiFlags & IR816MF_BRANCH) || (sLine.uiFlags & IR816MF_ENDS_FLOW) || (sLine.uiMnemonic == IR816M_Unknown))
		{
			//Not a conditional branch.
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the carry may be read after the branch after line uiLine (past
// labels): after it, or where it branches to (IR816CarryReadAt; branching on
// the carry is the only way the code reads a carry it didn't set).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CarryReadAfterBranch(CIRW65C816Code* pcCode, size uiLine)
{
	size		uiBranch;
	size		uiLabel;
	size		ui;
	const char*	szLabel;
	CChars*		psz;

	uiBranch = IR816SkipLabels(pcCode, uiLine) + 1;
	if ((uiBranch >= pcCode->NumLines()) || IR816CarryReadAt(pcCode, uiBranch + 1))
	{
		return true;
	}

	//"\t\tbne L63": the label's line is "L63:".
	psz = pcCode->GetText(uiBranch);
	if (psz->Length() <= 6)
	{
		return true;
	}
	szLabel = psz->Text() + 6;
	uiLabel = psz->Length() - 6;
	if (uiLabel + 3 > 40)
	{
		return true;
	}
	for (ui = 1; ui < pcCode->NumLines(); ui++)
	{
		psz = pcCode->GetText(ui);
		if ((psz->Length() == uiLabel + 1) && (strncmp(psz->Text(), szLabel, uiLabel) == 0) && (psz->Text()[uiLabel] == ':'))
		{
			return IR816CarryReadAt(pcCode, ui + 1);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction sets N and Z from all of A (as wide as it is).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SetsZNFromA(SIRW65C816Line* psLine)
{
	if (psLine->uiFlags & IR816MF_ZN_FROM_A)
	{
		return true;
	}
	return (psLine->uiFlags & IR816MF_SHIFT_STEP) && IR816OperandIsA(psLine);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the carry may be read after line uiLine before an instruction sets
// it again without reading it (a label, a directive, a branch, a jump, a
// return, a width change or the end might lead to a read).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CarryReadLater(CIRW65C816Code* pcCode, size uiLine)
{
	SIRW65C816Line	sLine;
	size		ui;

	for (ui = uiLine + 1; ui < pcCode->NumLines(); ui++)
	{
		if (!pcCode->Instruction(ui, &sLine))
		{
			return true;
		}
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
// Whether the "and 0xFF" before line uiFrom is done for nothing: A's low
// byte is all that is used before the next "and 0xFF" masks it again, as only
// clc, sec, adc, sbc, ora, eor, and and asl A come between (their low bytes
// don't depend on the high bytes), and the carry those leave isn't read after
// it (IR816CarryReadLater).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816MaskRedone(CIRW65C816Code* pcCode, size uiFrom)
{
	SIRW65C816Line	sLine;
	bool		bCarry;
	size		ui;

	bCarry = false;
	for (ui = uiFrom; ui < pcCode->NumLines(); ui++)
	{
		if (!pcCode->Instruction(ui, &sLine))
		{
			return false;
		}
		if ((sLine.uiMnemonic == IR816M_And) && IR816OperandIs(&sLine, "0xFF", 4))
		{
			return !bCarry || !IR816CarryReadLater(pcCode, ui);
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
// Whether line uiLine is "uiMnemonic szOperand" (szOperand as long as
// uiOperand).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LineIs(CIRW65C816Code* pcCode, size uiLine, uint16 uiMnemonic, const char* szOperand, size uiOperand)
{
	SIRW65C816Line	sLine;

	if ((uiLine >= pcCode->NumLines()) || !pcCode->Instruction(uiLine, &sLine))
	{
		return false;
	}
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
// The line "ldy w" after line uiFrom (not uiFrom itself), when the lines from
// uiFrom to it are straight line code that neither uses Y, changes a width
// (the sta before may have stored one byte, and tay copies two) nor names a
// byte near w; else IR816_NO_LINE.
//
//////////////////////////////////////////////////////////////////////////
static size IR816LoadYLater(CIRW65C816Code* pcCode, size uiFrom, const char* szWord, size uiWord)
{
	SIRW65C816Line	sLine;
	size		ui;

	for (ui = uiFrom; ui < pcCode->NumLines(); ui++)
	{
		if (!pcCode->Instruction(ui, &sLine))
		{
			return IR816_NO_LINE;
		}
		if ((sLine.uiMnemonic == IR816M_Ldy) && IR816OperandIs(&sLine, szWord, uiWord))
		{
			return (ui == uiFrom) ? IR816_NO_LINE : ui;
		}
		if ((sLine.uiFlags & (IR816MF_LEAVES | IR816MF_INTERRUPT | IR816MF_SETS_D | IR816MF_USES_Y | IR816MF_WIDTH)) || IR816IndexedBy(sLine.szOperand, sLine.uiOperand, 'Y') ||
			IR816MentionsNear(sLine.szOperand, sLine.uiOperand, atoi(szWord + 5)))
		{
			return IR816_NO_LINE;
		}
	}
	return IR816_NO_LINE;
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
void CIRW65C816Backend::PeepholeStores(CChars* pszDest, size uiStart)
{
	CIRW65C816Code	cCode;

	cCode.Init();
	cCode.Load(pszDest, uiStart);
	PeepholeStores(&cCode);
	cCode.Write(pszDest, uiStart);
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::PeepholeStores(CIRW65C816Code* pcCode)
{
	CIRW65C816Code	cOut;
	SIRW65C816Line	sLine;
	size		ui;
	size		uiNext;
	size		uiAfter;
	size		uj;
	bool		bDirect;

	cOut.Init();
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		bDirect = false;
		if (pcCode->Instruction(ui, &sLine))
		{
			bDirect = (sLine.uiMnemonic == IR816M_Sta) && sLine.bDirect;
		}
		uiNext = (bDirect && (ui + 1 < pcCode->NumLines())) ? ui + 1 : IR816_NO_LINE;
		if ((uiNext != IR816_NO_LINE) && (IR816LineIs(pcCode, uiNext, IR816M_Dec, sLine.szOperand, sLine.uiOperand) || IR816LineIs(pcCode, uiNext, IR816M_Inc, sLine.szOperand, sLine.uiOperand)) &&
			IR816LineIs(pcCode, uiNext + 1, IR816M_Lda, sLine.szOperand, sLine.uiOperand))
		{
			cOut.Append((pcCode->Text(uiNext)[2] == 'd') ? "\t\tdec A" : "\t\tinc A");
			if ((uiNext + 2 >= pcCode->NumLines()) || !IR816DeadStore(pcCode, uiNext + 2, sLine.szOperand, sLine.uiOperand))
			{
				cOut.Append(pcCode->Text(ui));
			}
			ui = uiNext + 1;
			continue;
		}
		uiAfter = (uiNext != IR816_NO_LINE) ? IR816LoadYLater(pcCode, uiNext, sLine.szOperand, sLine.uiOperand) : IR816_NO_LINE;
		if ((uiAfter != IR816_NO_LINE) && !IR816ZNReadAfter(pcCode, ui) && !IR816ZNReadAfter(pcCode, uiAfter))
		{
			//sta w ... ldy w (Y and w untouched between): tay at once.
			if ((uiAfter + 1 >= pcCode->NumLines()) || !IR816DeadStore(pcCode, uiAfter + 1, sLine.szOperand, sLine.uiOperand))
			{
				cOut.Append(pcCode->Text(ui));
			}
			cOut.Append("\t\ttay");
			for (uj = ui + 1; uj != uiAfter; uj++)
			{
				cOut.Append(pcCode->Text(uj));
			}
			ui = uiAfter;
			continue;
		}
		if ((uiNext != IR816_NO_LINE) && (IR816LineIs(pcCode, uiNext, IR816M_Ldy, sLine.szOperand, sLine.uiOperand) || IR816LineIs(pcCode, uiNext, IR816M_Ldx, sLine.szOperand, sLine.uiOperand)))
		{
			if ((uiNext + 1 >= pcCode->NumLines()) || !IR816DeadStore(pcCode, uiNext + 1, sLine.szOperand, sLine.uiOperand))
			{
				cOut.Append(pcCode->Text(ui));
			}
			cOut.Append((pcCode->Text(uiNext)[4] == 'y') ? "\t\ttay" : "\t\ttax");
			ui = uiNext;
			continue;
		}
		cOut.Append(pcCode->Text(ui));
	}
	pcCode->Take(&cOut);
	cOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction reads X: a transfer, store, step, compare or push
// of it (IR816MF_READS_X), or an X indexed operand.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ReadsX(SIRW65C816Line* psLine)
{
	return (psLine->uiFlags & IR816MF_READS_X) || IR816IndexedBy(psLine->szOperand, psLine->uiOperand, 'X');
}


//////////////////////////////////////////////////////////////////////////
//
// Whether X is written before it is read after line uiLine (tax, ldx, plx
// or tyx first, in straight line code).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816XDeadAfter(CIRW65C816Code* pcCode, size uiLine)
{
	SIRW65C816Line	sLine;
	size		ui;

	for (ui = uiLine + 1; ui < pcCode->NumLines(); ui++)
	{
		if (!pcCode->Instruction(ui, &sLine))
		{
			return false;
		}
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
void CIRW65C816Backend::PeepholeTransfers(CChars* pszDest, size uiStart)
{
	CIRW65C816Code	cCode;

	cCode.Init();
	cCode.Load(pszDest, uiStart);
	PeepholeTransfers(&cCode);
	cCode.Write(pszDest, uiStart);
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::PeepholeTransfers(CIRW65C816Code* pcCode)
{
	CArrayTemplate<uint8>	abRemove;
	SIRW65C816Line				sLine;
	size					ui;
	size					uiScan;
	size					uiTransfer;

	abRemove.Init(CIRW65C816Backend::GetScratch());
	abRemove.GrowToAtLeastNumElements(pcCode->NumLines(), true, 0);
	uiTransfer = IR816_NO_LINE;
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		if (ui == uiTransfer)
		{
			//The txy that the tay before made.
			*abRemove.Get(ui) = 1;
			uiTransfer = IR816_NO_LINE;
			continue;
		}
		if ((uiTransfer == IR816_NO_LINE) && pcCode->GetText(ui)->Equals("\t\ttax"))
		{
			for (uiScan = ui + 1; uiScan < pcCode->NumLines(); uiScan++)
			{
				if (!pcCode->Instruction(uiScan, &sLine))
				{
					break;
				}
				if (sLine.uiMnemonic == IR816M_Txy)
				{
					if (IR816XDeadAfter(pcCode, uiScan) && !IR816ZNReadAfter(pcCode, uiScan))
					{
						uiTransfer = uiScan;
					}
					break;
				}
				if ((sLine.uiFlags & (IR816MF_LEAVES | IR816MF_READS_X | IR816MF_WRITES_X | IR816MF_USES_Y)) || (sLine.uiMnemonic == IR816M_Tsx) ||
					IR816IndexedBy(sLine.szOperand, sLine.uiOperand, 'X') || IR816IndexedBy(sLine.szOperand, sLine.uiOperand, 'Y'))
				{
					break;
				}
			}
			if (uiTransfer != IR816_NO_LINE)
			{
				//The lines looked at after this one are all after it: changing it changes nothing they see.
				pcCode->Set(ui, "\t\ttay");
			}
		}
	}
	pcCode->RemoveMarked(&abRemove);
	abRemove.Kill();
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
void CIRW65C816Backend::PeepholeRegisters(CChars* pszDest, size uiStart)
{
	CIRW65C816Code	cCode;

	cCode.Init();
	cCode.Load(pszDest, uiStart);
	PeepholeRegisters(&cCode);
	cCode.Write(pszDest, uiStart);
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::PeepholeRegisters(CIRW65C816Code* pcCode)
{
	CArrayTemplate<uint8>	abRemove;
	SIRW65C816Line				sLine;
	SIRW65C816Held				asHeld[3];
	int						iRegister;
	size					ui;

	abRemove.Init(CIRW65C816Backend::GetScratch());
	abRemove.GrowToAtLeastNumElements(pcCode->NumLines(), true, 0);
	IR816ForgetHeld(asHeld, 0, 2);
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		if (!pcCode->Instruction(ui, &sLine))
		{
			IR816ForgetHeld(asHeld, 0, 2);
		}
		else if (pcCode->Text(ui)[2] != ';')
		{
			iRegister = (sLine.uiMnemonic == IR816M_Lda) ? 0 : (sLine.uiMnemonic == IR816M_Ldx) ? 1 : (sLine.uiMnemonic == IR816M_Ldy) ? 2 : -1;
			if ((iRegister >= 0) && IR816Holds(asHeld, iRegister, &sLine) && !IR816ZNReadAfter(pcCode, ui))
			{
				*abRemove.Get(ui) = 1;
				continue;
			}
			IR816UpdateHeld(asHeld, &sLine);
		}
	}
	pcCode->RemoveMarked(&abRemove);
	abRemove.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The "and 0xFF"s done for nothing (IR816MaskRedone) taken out of the code
// from uiStart, after Peephole has kept values in A (so only the masking
// is left between two masks).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::PeepholeMasks(CChars* pszDest, size uiStart)
{
	CIRW65C816Code	cCode;

	cCode.Init();
	cCode.Load(pszDest, uiStart);
	PeepholeMasks(&cCode);
	cCode.Write(pszDest, uiStart);
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::PeepholeMasks(CIRW65C816Code* pcCode)
{
	CArrayTemplate<uint8>	abRemove;
	size					ui;

	abRemove.Init(CIRW65C816Backend::GetScratch());
	abRemove.GrowToAtLeastNumElements(pcCode->NumLines(), true, 0);
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		if (pcCode->GetText(ui)->Equals("\t\tand 0xFF") && IR816MaskRedone(pcCode, ui + 1))
		{
			*abRemove.Get(ui) = 1;
		}
	}
	pcCode->RemoveMarked(&abRemove);
	abRemove.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the operand names szWord anywhere in it.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816OperandMentions(SIRW65C816Line* psLine, const char* szWord, size uiWord)
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
// before line uiFrom can stay in A instead: the next line that mentions the
// word loads it (and nothing after the load reads the N and Z it sets) or
// pushes it (pei, which becomes pha), and nothing before that changes A or
// goes anywhere (only clc, sec, ldx, ldy, stores, tax, tay, pha, pea, pei, phk
// and cmp come between).  Its one use (Store) is that load or push.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816KeepsInA(CIRW65C816Code* pcCode, size uiFrom, const char* szWord, size uiWord)
{
	SIRW65C816Line	sLine;
	size		ui;

	for (ui = uiFrom; ui < pcCode->NumLines(); ui++)
	{
		if (!pcCode->Instruction(ui, &sLine))
		{
			return false;
		}
		if (IR816OperandMentions(&sLine, szWord, uiWord))
		{
			return (sLine.uiOperand == uiWord) && (((sLine.uiMnemonic == IR816M_Lda) && !IR816ZNReadAfter(pcCode, ui)) || (sLine.uiMnemonic == IR816M_Pei));
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
// Whether a value stored to the word szWord by the line before line uiFrom
// can stay in A as the first operand of a commutative instruction instead
// (adc, and, ora or eor of the word straight after an lda of something else,
// with only clc or sec before it): *puiLoad is that lda's line and *puiUse
// the instruction's, which becomes the instruction on the lda's operand.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SwapsInA(CIRW65C816Code* pcCode, size uiFrom, const char* szWord, size uiWord, size* puiLoad, size* puiUse)
{
	SIRW65C816Line	sLine;
	size		ui;
	size		uiLoad;
	const char*	szSemicolon;

	for (ui = uiFrom; (ui < pcCode->NumLines()) && (pcCode->GetText(ui)->Equals("\t\tclc") || pcCode->GetText(ui)->Equals("\t\tsec")); ui++)
	{
	}
	if ((ui >= pcCode->NumLines()) || (strncmp(pcCode->Text(ui), "\t\tlda ", 6) != 0))
	{
		return false;
	}
	uiLoad = ui;
	IR816Decode(pcCode->Text(ui), &sLine);
	if (IR816OperandMentions(&sLine, szWord, uiWord))
	{
		return false;
	}
	szSemicolon = strchr(sLine.szOperand, ';');
	if ((sLine.uiOperand == 0) || (sLine.uiOperand > 40) || (sLine.szOperand[sLine.uiOperand - 1] == 'A') || (szSemicolon && (szSemicolon < sLine.szOperand + sLine.uiOperand)))
	{
		return false;
	}
	ui++;
	if ((ui >= pcCode->NumLines()) || (pcCode->Text(ui)[0] != '\t') || (pcCode->Text(ui)[1] != '\t'))
	{
		return false;
	}
	IR816Decode(pcCode->Text(ui), &sLine);
	if ((!(sLine.uiFlags & IR816MF_LOGIC) && (sLine.uiMnemonic != IR816M_Adc)) || !IR816OperandIs(&sLine, szWord, uiWord))
	{
		return false;
	}
	*puiLoad = uiLoad;
	*puiUse = ui;
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
static bool IR816KeepsAWord(SIRW65C816Line* psLine, const char* szA)
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
static bool IR816ChangesYConstant(SIRW65C816Line* psLine)
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
void CIRW65C816Backend::Peephole(CChars* pszDest, size uiStart)
{
	CIRW65C816Code	cCode;

	cCode.Init();
	cCode.Load(pszDest, uiStart);
	Peephole(&cCode);
	cCode.Write(pszDest, uiStart);
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Peephole(CIRW65C816Code* pcCode)
{
	CIRW65C816Code	cOut;
	CChars*		psz;
	SIRW65C816Line	sLine;
	char		szA[32];		//The direct page operand A holds ("[D + 12]"), or empty.
	char		szKept[32];		//The word whose store was left out, its value kept in A for its next lda, or empty.
	size		uiSwapLoad;		//An lda left out: the value kept in A is the first operand instead (IR816SwapsInA)...
	size		uiSwapUse;		//...of this instruction, which takes the lda's operand.
	char		szSwapped[64];
	char		szY[32];		//The constant Y holds, or empty.
	size		ui;
	size		uiLength;
	size		uiDeadLoad;		//cOut's last line if it is an lda of a direct page word or a constant, else IR816_NO_LINE.
	bool		bClearCarry;
	bool		bZNFromA;		//The last instruction kept set N and Z from A.
	bool		bKeep;
	CChars		sz;

	cOut.Init();
	szA[0] = '\0';
	szY[0] = '\0';
	szKept[0] = '\0';
	uiSwapLoad = IR816_NO_LINE;
	uiSwapUse = IR816_NO_LINE;
	bClearCarry = false;
	bZNFromA = false;
	uiDeadLoad = IR816_NO_LINE;
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		psz = pcCode->GetText(ui);
		uiLength = psz->Length();
		bKeep = true;

		//A label, or a directive: nothing is known after it.
		if ((uiLength < 3) || !pcCode->Instruction(ui, &sLine))
		{
			szA[0] = '\0';
			szY[0] = '\0';
			bClearCarry = false;
			bZNFromA = false;
			uiDeadLoad = IR816_NO_LINE;
			cOut.Append(psz->Text(), uiLength);
			continue;
		}

		//The mnemonic and the operand.
		if (sLine.bMarked)
		{
			uiLength -= StrLen(IR816_KEEP_IN_A);
		}

		//A value kept in A instead of its store and load, or as the first operand of a commutative instruction.
		if (sLine.bMarked && sLine.bDirect && (sLine.uiMnemonic == IR816M_Sta) && IR816KeepsInA(pcCode, ui + 1, sLine.szOperand, sLine.uiOperand))
		{
			memcpy(szKept, sLine.szOperand, sLine.uiOperand);
			szKept[sLine.uiOperand] = '\0';
			continue;
		}
		if (sLine.bMarked && sLine.bDirect && (szKept[0] == '\0') && (sLine.uiMnemonic == IR816M_Sta) &&
			IR816SwapsInA(pcCode, ui + 1, sLine.szOperand, sLine.uiOperand, &uiSwapLoad, &uiSwapUse))
		{
			continue;
		}
		if (ui == uiSwapLoad)
		{
			//The lda's operand goes to the instruction it was loaded for.
			snprintf(szSwapped, sizeof(szSwapped), "%.*s", (int)sLine.uiOperand, sLine.szOperand);
			uiSwapLoad = IR816_NO_LINE;
			continue;
		}
		if (ui == uiSwapUse)
		{
			sz.Init(CIRW65C816Backend::GetScratch());
			sz.Append("\t\t");
			sz.Append(sLine.szMnemonic);
			sz.Append(" ");
			sz.Append(szSwapped);
			cOut.Append(sz.Text());
			sz.Kill();
			uiSwapUse = IR816_NO_LINE;
			szA[0] = '\0';
			bClearCarry = false;
			bZNFromA = true;
			uiDeadLoad = IR816_NO_LINE;
			continue;
		}
		if (((sLine.uiMnemonic == IR816M_Lda) || (sLine.uiMnemonic == IR816M_Pei)) && IR816OperandIs(&sLine, szKept, StrLen(szKept)))
		{
			szKept[0] = '\0';
			if (sLine.uiMnemonic == IR816M_Pei)
			{
				cOut.Append("\t\tpha");
				uiDeadLoad = IR816_NO_LINE;
			}
			continue;
		}
		if ((sLine.uiMnemonic == IR816M_Pei) && IR816OperandIs(&sLine, szA, StrLen(szA)))
		{
			//A holds the word: pha pushes it for less.
			cOut.Append("\t\tpha");
			uiDeadLoad = IR816_NO_LINE;
			continue;
		}

		if ((sLine.uiMnemonic == IR816M_Lda) && sLine.bDirect && IR816OperandIs(&sLine, szA, StrLen(szA)) && !IR816BranchesOnZN(pcCode, ui))
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
		else if ((sLine.uiMnemonic == IR816M_Cmp) && IR816OperandIs(&sLine, "0", 1) && bZNFromA && IR816BranchesOnZN(pcCode, ui) &&
				 !IR816CarryReadAfterBranch(pcCode, ui))
		{
			bKeep = false;
		}
		if (!bKeep)
		{
			continue;
		}
		if ((sLine.uiMnemonic == IR816M_Lda) && (uiDeadLoad != IR816_NO_LINE))
		{
			//The lda before it loaded A for nothing.
			cOut.Truncate(uiDeadLoad);
		}
		uiDeadLoad = ((sLine.uiMnemonic == IR816M_Lda) && (sLine.bDirect || IR816IsDecimal(sLine.szOperand))) ? cOut.NumLines() : IR816_NO_LINE;
		cOut.Append(psz->Text(), uiLength);
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
	pcCode->Take(&cOut);
	cOut.Kill();
	RemoveDeadStores(pcCode);
}
