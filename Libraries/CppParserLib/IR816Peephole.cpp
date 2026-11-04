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


//The W65C816 back end's assembly text: its lines decoded (IR816Decode) and the passes over straight line code
//that take out what isn't needed (Peephole and the rest, RemoveUnreadStores, RemoveDeadStores).


//A mnemonic and its IR816MF_ bits.
struct SIR816Mnemonic
{
	const char*	szName;
	uint32		uiFlags;
};


//Per EIR816Mnemonic.
static const SIR816Mnemonic gasIR816Mnemonics[] =
{
	{ "adc", IR816MF_ZN_FROM_A | IR816MF_READS_A | IR816MF_A_ALU | IR816MF_SETS_CARRY | IR816MF_READS_CARRY },
	{ "and", IR816MF_LOGIC | IR816MF_ZN_FROM_A | IR816MF_READS_A | IR816MF_A_ALU },
	{ "asl", IR816MF_SHIFT_STEP | IR816MF_SETS_CARRY },
	{ "bcc", IR816MF_BRANCH | IR816MF_BRANCH_C | IR816MF_LEAVES | IR816MF_READS_CARRY },
	{ "bcs", IR816MF_BRANCH | IR816MF_BRANCH_C | IR816MF_LEAVES | IR816MF_READS_CARRY },
	{ "beq", IR816MF_BRANCH | IR816MF_BRANCH_ZN | IR816MF_LEAVES },
	{ "bit", IR816MF_READS_A | IR816MF_A_ALU | IR816MF_LEAVES },
	{ "bmi", IR816MF_BRANCH | IR816MF_BRANCH_ZN | IR816MF_LEAVES },
	{ "bne", IR816MF_BRANCH | IR816MF_BRANCH_ZN | IR816MF_LEAVES },
	{ "bpl", IR816MF_BRANCH | IR816MF_BRANCH_ZN | IR816MF_LEAVES },
	{ "bra", IR816MF_BRANCH | IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "brk", IR816MF_LEAVES | IR816MF_INTERRUPT },
	{ "brl", IR816MF_BRANCH | IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "bvc", IR816MF_BRANCH | IR816MF_LEAVES },
	{ "bvs", IR816MF_BRANCH | IR816MF_LEAVES },
	{ "clc", IR816MF_CARRY | IR816MF_SETS_CARRY },
	{ "cld", 0 },
	{ "cli", 0 },
	{ "clv", 0 },
	{ "cmp", IR816MF_COMPARE | IR816MF_READS_A | IR816MF_A_ALU | IR816MF_SETS_CARRY },
	{ "cop", IR816MF_INTERRUPT },
	{ "cpx", IR816MF_COMPARE | IR816MF_READS_X | IR816MF_SETS_CARRY },
	{ "cpy", IR816MF_COMPARE | IR816MF_Y_ALU | IR816MF_USES_Y | IR816MF_SETS_CARRY },
	{ "dec", IR816MF_SHIFT_STEP },
	{ "dex", IR816MF_STEP_INDEX | IR816MF_READS_X },
	{ "dey", IR816MF_STEP_INDEX | IR816MF_Y_ALU | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "eor", IR816MF_LOGIC | IR816MF_ZN_FROM_A | IR816MF_READS_A | IR816MF_A_ALU },
	{ "inc", IR816MF_SHIFT_STEP },
	{ "inx", IR816MF_STEP_INDEX | IR816MF_READS_X },
	{ "iny", IR816MF_STEP_INDEX | IR816MF_Y_ALU | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "jml", IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "jmp", IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "jsl", IR816MF_LEAVES | IR816MF_CALL | IR816MF_USES_Y },
	{ "jsr", IR816MF_LEAVES | IR816MF_CALL | IR816MF_USES_Y },
	{ "lda", IR816MF_LOADS_A | IR816MF_ZN_FROM_A | IR816MF_A_ALU },
	{ "ldx", IR816MF_LOAD_INDEX | IR816MF_WRITES_X },
	{ "ldy", IR816MF_LOAD_INDEX | IR816MF_Y_ALU | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "lsr", IR816MF_SHIFT_STEP | IR816MF_SETS_CARRY },
	{ "mvn", IR816MF_READS_A | IR816MF_BLOCK_MOVE | IR816MF_READS_X | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "mvp", IR816MF_READS_A | IR816MF_BLOCK_MOVE | IR816MF_READS_X | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "nop", 0 },
	{ "ora", IR816MF_LOGIC | IR816MF_ZN_FROM_A | IR816MF_READS_A | IR816MF_A_ALU },
	{ "pea", IR816MF_PUSH_WORD },
	{ "pei", IR816MF_PUSH_WORD },
	{ "per", 0 },
	{ "pha", IR816MF_READS_A | IR816MF_COPIES_A | IR816MF_PUSH_REGISTER },
	{ "phb", 0 },
	{ "phd", 0 },
	{ "phk", IR816MF_PUSH_WORD },
	{ "php", IR816MF_READS_CARRY },
	{ "phx", IR816MF_READS_X | IR816MF_PUSH_REGISTER },
	{ "phy", IR816MF_USES_Y | IR816MF_PUSH_REGISTER },
	{ "pla", IR816MF_LOADS_A | IR816MF_ZN_FROM_A },
	{ "plb", 0 },
	{ "pld", IR816MF_SETS_D },
	{ "plp", 0 },
	{ "plx", IR816MF_WRITES_X },
	{ "ply", IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "rep", IR816MF_WIDTH },
	{ "rol", IR816MF_SHIFT_STEP | IR816MF_SETS_CARRY | IR816MF_READS_CARRY },
	{ "ror", IR816MF_SHIFT_STEP | IR816MF_SETS_CARRY | IR816MF_READS_CARRY },
	{ "rti", IR816MF_LEAVES },
	{ "rtl", IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "rts", IR816MF_ENDS_FLOW | IR816MF_LEAVES },
	{ "sbc", IR816MF_ZN_FROM_A | IR816MF_READS_A | IR816MF_A_ALU | IR816MF_SETS_CARRY | IR816MF_READS_CARRY },
	{ "sec", IR816MF_CARRY | IR816MF_SETS_CARRY },
	{ "sed", 0 },
	{ "sei", 0 },
	{ "sep", IR816MF_WIDTH },
	{ "sta", IR816MF_STORE | IR816MF_READS_A | IR816MF_A_ALU },
	{ "stp", IR816MF_ENDS_FLOW },
	{ "stx", IR816MF_STORE | IR816MF_READS_X },
	{ "sty", IR816MF_STORE | IR816MF_Y_ALU | IR816MF_USES_Y },
	{ "stz", IR816MF_STORE },
	{ "tax", IR816MF_READS_A | IR816MF_COPIES_A | IR816MF_WRITES_X },
	{ "tay", IR816MF_READS_A | IR816MF_Y_ALU | IR816MF_COPIES_A | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "tcd", IR816MF_READS_A | IR816MF_SETS_D },
	{ "tcs", IR816MF_READS_A },
	{ "tdc", IR816MF_LOADS_A | IR816MF_ZN_FROM_A },
	{ "trb", 0 },
	{ "tsb", 0 },
	{ "tsc", IR816MF_LOADS_A },
	{ "tsx", 0 },
	{ "txa", IR816MF_LOADS_A | IR816MF_ZN_FROM_A | IR816MF_READS_X },
	{ "txs", IR816MF_READS_X },
	{ "txy", IR816MF_READS_X | IR816MF_USES_Y | IR816MF_WRITES_Y },
	{ "tya", IR816MF_LOADS_A | IR816MF_ZN_FROM_A | IR816MF_Y_ALU | IR816MF_USES_Y },
	{ "tyx", IR816MF_WRITES_X | IR816MF_USES_Y },
	{ "wai", 0 },
	{ "wdm", 0 },
	{ "xba", IR816MF_READS_A },
	{ "xce", IR816MF_SETS_CARRY | IR816MF_READS_CARRY },
};


//The conditional branches in pairs: each goes when the other doesn't (IR816InvertedBranch).
static const uint16 gauiIR816InvertedBranches[] = { IR816M_Bcc, IR816M_Bcs, IR816M_Beq, IR816M_Bne, IR816M_Bmi, IR816M_Bpl, IR816M_Bvc, IR816M_Bvs };


//////////////////////////////////////////////////////////////////////////
//
// The IR816MF_ bits of a mnemonic the table doesn't have, as the rules
// took any b, j or rt mnemonic to leave straight line code, and any other
// three letter b one (but brk and bit) to branch to a label.
//
//////////////////////////////////////////////////////////////////////////
static uint32 IR816UnknownFlags(const char* szMnemonic)
{
	uint32	uiFlags;

	uiFlags = 0;
	if ((szMnemonic[0] == 'b') || (szMnemonic[0] == 'j') || ((szMnemonic[0] == 'r') && (szMnemonic[1] == 't')))
	{
		uiFlags |= IR816MF_LEAVES;
	}
	if ((szMnemonic[0] == 'b') && (StrLen(szMnemonic) == 3))
	{
		uiFlags |= IR816MF_BRANCH;
	}
	return uiFlags;
}


//Every mnemonic is three lower case letters: as a key, five bits a letter (see IR816MnemonicKey).
#define IR816_MNEMONIC_KEYS		(32 * 32 * 32)


//Each EIR816Mnemonic by its key (IR816M_Unknown for the keys of no mnemonic), made when first needed.
static uint8	gauiIR816MnemonicByKey[IR816_MNEMONIC_KEYS];
static bool		gbIR816MnemonicsIndexed = false;


//////////////////////////////////////////////////////////////////////////
//
// The key of a mnemonic of three lower case letters, or -1.
//
//////////////////////////////////////////////////////////////////////////
static int IR816MnemonicKey(const char* szMnemonic)
{
	int		i;
	int		iKey;

	iKey = 0;
	for (i = 0; i < 3; i++)
	{
		if ((szMnemonic[i] < 'a') || (szMnemonic[i] > 'z'))
		{
			return -1;
		}
		iKey = (iKey << 5) | (szMnemonic[i] - 'a');
	}
	return (szMnemonic[3] == '\0') ? iKey : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes gauiIR816MnemonicByKey, if it isn't made already.  Before units are compiled at the same time (it is
// made when first needed otherwise).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::IndexMnemonics(void)
{
	int		i;

	if (gbIR816MnemonicsIndexed)
	{
		return;
	}
	memset(gauiIR816MnemonicByKey, IR816M_Unknown, sizeof(gauiIR816MnemonicByKey));
	for (i = 0; i < IR816M_Unknown; i++)
	{
		gauiIR816MnemonicByKey[IR816MnemonicKey(gasIR816Mnemonics[i].szName)] = (uint8)i;
	}
	gbIR816MnemonicsIndexed = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The EIR816Mnemonic of szMnemonic, IR816M_Unknown for one it doesn't have.
//
//////////////////////////////////////////////////////////////////////////
static uint16 IR816FindMnemonic(const char* szMnemonic)
{
	int		iKey;

	CIR816Backend::IndexMnemonics();
	iKey = IR816MnemonicKey(szMnemonic);
	return (iKey < 0) ? (uint16)IR816M_Unknown : gauiIR816MnemonicByKey[iKey];
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operand is a direct page word, [D + n] (n decimal): n into
// *piOffset.  [D + X], a pointer's [[D + n]:24 ...] and a symbol aren't.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsDirect(const char* szOperand, size uiOperand, int32* piOffset)
{
	if ((uiOperand >= 30) || (strncmp(szOperand, "[D + ", 5) != 0) || (szOperand[5] < '0') || (szOperand[5] > '9') || (szOperand[uiOperand - 1] != ']'))
	{
		return false;
	}
	*piOffset = atoi(&szOperand[5]);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is an instruction: it starts with two tabs and isn't a
// directive (".width").  Labels start in the first column.
//
//////////////////////////////////////////////////////////////////////////
bool IR816IsInstruction(const char* szLine)
{
	return (szLine[0] == '\t') && (szLine[1] == '\t') && (szLine[2] != '.');
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction at szLine (a line starting with two tabs, ending at a new
// line or the end) decoded: its mnemonic, what is known of it and its
// operand (less a IR816_KEEP_IN_A mark).
//
//////////////////////////////////////////////////////////////////////////
void IR816Decode(const char* szLine, SIR816Line* psLine)
{
	const char*	szEnd;
	size		uiLength;
	size		uiMark;
	int			i;

	szEnd = strchr(szLine, '\n');
	uiLength = szEnd ? (size)(szEnd - szLine) : StrLen(szLine);
	uiMark = StrLen(IR816_KEEP_IN_A);
	psLine->bMarked = (uiLength >= uiMark) && (strncmp(szLine + uiLength - uiMark, IR816_KEEP_IN_A, uiMark) == 0);
	if (psLine->bMarked)
	{
		uiLength -= uiMark;
	}
	for (i = 0; (i < 7) && (2 + (size)i < uiLength) && (szLine[2 + i] != ' '); i++)
	{
		psLine->szMnemonic[i] = szLine[2 + i];
	}
	psLine->szMnemonic[i] = '\0';
	psLine->szOperand = szLine + 2 + i + ((2 + (size)i < uiLength) ? 1 : 0);
	psLine->uiOperand = (size)(szLine + uiLength - psLine->szOperand);
	psLine->uiMnemonic = IR816FindMnemonic(psLine->szMnemonic);
	psLine->uiFlags = (psLine->uiMnemonic != IR816M_Unknown) ? gasIR816Mnemonics[psLine->uiMnemonic].uiFlags : IR816UnknownFlags(psLine->szMnemonic);
	psLine->iOffset = 0;
	psLine->bDirect = IR816IsDirect(psLine->szOperand, psLine->uiOperand, &psLine->iOffset);
}


//////////////////////////////////////////////////////////////////////////
//
// A line of a split function (PeepholeBranches) decoded, when it may be an
// instruction (it starts with a tab and has a mnemonic's room).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816DecodeLine(CChars* psz, SIR816Line* psLine)
{
	if ((psz->Length() < 3) || (psz->Text()[0] != '\t'))
	{
		return false;
	}
	IR816Decode(psz->Text(), psLine);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line of a split function is an instruction with any of the
// IR816MF_ bits in uiFlags.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LineHas(CChars* psz, uint32 uiFlags)
{
	SIR816Line	sLine;

	return IR816DecodeLine(psz, &sLine) && ((sLine.uiFlags & uiFlags) != 0);
}


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
// Whether the instruction's operand is exactly szOperand (uiOperand long).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816OperandIs(SIR816Line* psLine, const char* szOperand, size uiOperand)
{
	return (psLine->uiOperand == uiOperand) && (strncmp(psLine->szOperand, szOperand, uiOperand) == 0);
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
// Whether the instruction reads neither A nor the N and Z an lda before it
// set, and leaves A as it is: clc, sec, ldx, ldy, inx, iny, dex, dey, stx,
// sty, stz, pea, pei, phk, phx, phy, cpx, cpy, and shifts and steps of
// memory.  An lda followed only by these and then a load of A is dead.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LeavesA(SIR816Line* psLine)
{
	bool	bMemory;

	if (psLine->uiFlags & (IR816MF_CARRY | IR816MF_LOAD_INDEX | IR816MF_STEP_INDEX | IR816MF_PUSH_WORD))
	{
		return true;
	}
	if ((psLine->uiFlags & (IR816MF_STORE | IR816MF_PUSH_REGISTER | IR816MF_COMPARE)) && !(psLine->uiFlags & IR816MF_READS_A))
	{
		return true;
	}
	bMemory = (psLine->uiOperand > 0) && (psLine->szOperand[0] == '[');
	return bMemory && (psLine->uiFlags & IR816MF_SHIFT_STEP);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is a label ("L12:" or a function's).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsLabel(CChars* psz)
{
	return (psz->Length() > 1) && (psz->Text()[0] != '\t') && (psz->Text()[psz->Length() - 1] == ':');
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is a branch (bra or a conditional one) to a label: the
// line decoded and its label (without the colon).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816BranchLine(CChars* psz, SIR816Line* psLine, char* szLabel)
{
	size	ui;

	if (!IR816DecodeLine(psz, psLine))
	{
		return false;
	}
	if (!(psLine->uiFlags & IR816MF_BRANCH) || (psLine->uiOperand < 2) || (psLine->uiOperand > 30) || (psLine->szOperand[0] != 'L'))
	{
		return false;
	}
	for (ui = 0; (ui < psLine->uiOperand) && (psLine->szOperand[ui] != ' '); ui++)
	{
		szLabel[ui] = psLine->szOperand[ui];
	}
	szLabel[ui] = '\0';
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The branch that goes when uiMnemonic's doesn't, or NULL.
//
//////////////////////////////////////////////////////////////////////////
static const char* IR816InvertedBranch(uint16 uiMnemonic)
{
	int	i;

	for (i = 0; i < 8; i++)
	{
		if (gauiIR816InvertedBranches[i] == uiMnemonic)
		{
			return gasIR816Mnemonics[gauiIR816InvertedBranches[i ^ 1]].szName;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the lines from uiFrom up to the first instruction are labels
// with szLabel among them.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LabelFollows(CArrayChars* paszLines, size uiFrom, const char* szLabel)
{
	size	ui;
	size	uiLength;
	CChars*	psz;

	uiLength = StrLen(szLabel);
	for (ui = uiFrom; (ui < paszLines->NumElements()) && IR816IsLabel(paszLines->Get(ui)); ui++)
	{
		psz = paszLines->Get(ui);
		if ((psz->Length() == uiLength + 1) && (strncmp(psz->Text(), szLabel, uiLength) == 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of a local label's name ("L12", or "L12:" with bColon), else -1.
//
//////////////////////////////////////////////////////////////////////////
static int64 IR816LocalLabelNumber(const char* szLabel, bool bColon)
{
	size	ui;

	if ((szLabel[0] != 'L') || (szLabel[1] < '0') || (szLabel[1] > '9') || ((szLabel[1] == '0') && (szLabel[2] >= '0') && (szLabel[2] <= '9')))
	{
		return -1;
	}
	for (ui = 1; (szLabel[ui] >= '0') && (szLabel[ui] <= '9'); ui++)
	{
	}
	if (bColon ? ((szLabel[ui] != ':') || (szLabel[ui + 1] != '\0')) : (szLabel[ui] != '\0'))
	{
		return -1;
	}
	return (ui <= 10) ? atoll(&szLabel[1]) : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The line local label uiNumber is defined on (its index + 1 in pauiLines, 0 for none), made again from paszLines
// when it is out of date: lines move as others are removed, so the line found is checked to be the label's.
// Labels are defined once.  Returns the line, or -1.
//
//////////////////////////////////////////////////////////////////////////
static size IR816LocalLabelLine(CArrayChars* paszLines, CArrayTemplate<size>* pauiLines, int64 lliNumber, const char* szLabel)
{
	size	ui;
	size	uiLine;
	size	uiZero;
	int64	lliDefined;
	CChars*	psz;

	uiLine = ((size)lliNumber < pauiLines->NumElements()) ? *pauiLines->Get((size)lliNumber) : 0;
	if ((uiLine != 0) && (uiLine <= paszLines->NumElements()))
	{
		psz = paszLines->Get(uiLine - 1);
		if ((psz->Length() == StrLen(szLabel) + 1) && (strncmp(psz->Text(), szLabel, psz->Length() - 1) == 0) && (psz->Text()[psz->Length() - 1] == ':'))
		{
			return uiLine - 1;
		}
	}

	uiZero = 0;
	pauiLines->FakeSetUsedElements(0);
	for (ui = 0; ui < paszLines->NumElements(); ui++)
	{
		psz = paszLines->Get(ui);
		lliDefined = IR816IsLabel(psz) ? IR816LocalLabelNumber(psz->Text(), true) : -1;
		if (lliDefined < 0)
		{
			continue;
		}
		while (pauiLines->NumElements() <= (size)lliDefined)
		{
			pauiLines->Add(&uiZero);
		}
		if (*pauiLines->Get((size)lliDefined) == 0)
		{
			*pauiLines->Get((size)lliDefined) = ui + 1;
		}
	}
	uiLine = ((size)lliNumber < pauiLines->NumElements()) ? *pauiLines->Get((size)lliNumber) : 0;
	return (uiLine != 0) ? uiLine - 1 : (size)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the first instruction after label szLabel, else -1.  pauiLabelLines is where local labels are
// defined (IR816LocalLabelLine), so they aren't searched for.
//
//////////////////////////////////////////////////////////////////////////
static size IR816LabelledInstruction(CArrayChars* paszLines, const char* szLabel, CArrayTemplate<size>* pauiLabelLines)
{
	size	ui;
	size	uiLength;
	int64	lliNumber;
	CChars*	psz;

	lliNumber = IR816LocalLabelNumber(szLabel, false);
	if (lliNumber >= 0)
	{
		ui = IR816LocalLabelLine(paszLines, pauiLabelLines, lliNumber, szLabel);
	}
	else
	{
		uiLength = StrLen(szLabel);
		for (ui = 0; ui < paszLines->NumElements(); ui++)
		{
			psz = paszLines->Get(ui);
			if (IR816IsLabel(psz) && (psz->Length() == uiLength + 1) && (strncmp(psz->Text(), szLabel, uiLength) == 0))
			{
				break;
			}
		}
		if (ui == paszLines->NumElements())
		{
			ui = (size)-1;
		}
	}
	if (ui == (size)-1)
	{
		return (size)-1;
	}
	for (ui++; (ui < paszLines->NumElements()) && IR816IsLabel(paszLines->Get(ui)); ui++)
	{
	}
	return (ui < paszLines->NumElements()) ? ui : (size)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether c may be part of a label's name.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsNameCharacter(char c)
{
	return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '_');
}


//////////////////////////////////////////////////////////////////////////
//
// A decimal constant operand.  The 0x and 0b constants the backend writes
// for masks and sign flips are not taken as plain constants.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsDecimal(const char* szOperand)
{
	if ((szOperand[0] < '0') || (szOperand[0] > '9'))
	{
		return false;
	}
	return !((szOperand[0] == '0') && ((szOperand[1] == 'x') || (szOperand[1] == 'b')));
}


//////////////////////////////////////////////////////////////////////////
//
// Adds iChange to the count of each local label (L and a number) an instruction line names, in paiMentions by
// number.  A name is bounded by characters that can't be in a name, as IR816LabelUsed matches it.
//
//////////////////////////////////////////////////////////////////////////
static void IR816CountMentions(const char* szLine, CArrayTemplate<int32>* paiMentions, int32 iChange)
{
	const char*	szAt;
	const char*	szEnd;
	int64		lliNumber;
	int32		iZero;

	iZero = 0;
	for (szAt = szLine; (szAt = strchr(szAt, 'L')) != NULL; szAt++)
	{
		if ((szAt == szLine) || IR816IsNameCharacter(szAt[-1]) || (szAt[1] < '0') || (szAt[1] > '9'))
		{
			continue;
		}
		for (szEnd = szAt + 1; (*szEnd >= '0') && (*szEnd <= '9'); szEnd++)
		{
		}
		if (IR816IsNameCharacter(*szEnd) || ((szAt[1] == '0') && (szEnd - szAt > 2)) || (szEnd - szAt > 11))
		{
			continue;
		}
		lliNumber = atoll(szAt + 1);
		while (paiMentions->NumElements() <= (size)lliNumber)
		{
			paiMentions->Add(&iZero);
		}
		*paiMentions->Get((size)lliNumber) += iChange;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a label ("L12:") is named by an instruction's operand.  A local label's mentions are counted
// (IR816CountMentions, made when first needed and kept up to date as lines are removed: *pbCounted); another's
// are searched for.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LabelUsed(CArrayChars* paszLines, CChars* pszLabel, CArrayTemplate<int32>* paiMentions, bool* pbCounted)
{
	size		ui;
	size		uiLength;
	int64		lliNumber;
	const char*	szAt;
	const char*	szText;

	lliNumber = IR816LocalLabelNumber(pszLabel->Text(), true);
	if (lliNumber >= 0)
	{
		if (!*pbCounted)
		{
			paiMentions->FakeSetUsedElements(0);
			for (ui = 0; ui < paszLines->NumElements(); ui++)
			{
				szText = paszLines->GetText(ui);
				if (szText[0] == '\t')
				{
					IR816CountMentions(szText, paiMentions, 1);
				}
			}
			*pbCounted = true;
		}
		return ((size)lliNumber < paiMentions->NumElements()) && (*paiMentions->Get((size)lliNumber) > 0);
	}

	uiLength = pszLabel->Length() - 1;
	for (ui = 0; ui < paszLines->NumElements(); ui++)
	{
		szText = paszLines->GetText(ui);
		if (szText[0] != '\t')
		{
			continue;
		}
		for (szAt = szText; (szAt = strchr(szAt, 'L')) != NULL; szAt++)
		{
			if ((strncmp(szAt, pszLabel->Text(), uiLength) == 0) && !IR816IsNameCharacter(szAt[-1]) && !IR816IsNameCharacter(szAt[uiLength]))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the instructions after a bra, brl, jml, jmp, rtl, rts or stp
// (IR816MF_ENDS_FLOW) up to a label other than an unused local one
// (IR816LabelUsed), which go too.  True if any did.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816Unreachable(CArrayChars* paszLines)
{
	size					ui;
	CChars*					psz;
	bool					bChanged;
	CArrayTemplate<int32>	aiMentions;
	bool					bCounted;

	bChanged = false;
	aiMentions.Init();
	bCounted = false;
	for (ui = 0; ui + 1 < paszLines->NumElements(); ui++)
	{
		if (!IR816LineHas(paszLines->Get(ui), IR816MF_ENDS_FLOW))
		{
			continue;
		}
		while (ui + 1 < paszLines->NumElements())
		{
			psz = paszLines->Get(ui + 1);
			if (psz->Empty() || ((psz->Text()[0] != '\t') && ((psz->Text()[0] != 'L') || !IR816IsLabel(psz) || IR816LabelUsed(paszLines, psz, &aiMentions, &bCounted))))
			{
				break;
			}
			if ((psz->Text()[0] == '\t') && (psz->Text()[2] == '.'))
			{
				break;
			}
			if (bCounted && (psz->Text()[0] == '\t'))
			{
				IR816CountMentions(psz->Text(), &aiMentions, -1);
			}
			paszLines->Remove(ui + 1);
			bChanged = true;
		}
	}
	aiMentions.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what runs from line uiFrom on (past labels, following bras, past
// instructions that leave A and don't read it or N and Z: IR816LeavesA)
// starts with an instruction that sets all of A (IR816MF_LOADS_A) and N and
// Z, so a value in A and its flags aren't read.  pauiLabelLines is as for
// IR816LabelledInstruction.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SetsAFirst(CArrayChars* paszLines, size uiFrom, CArrayTemplate<size>* pauiLabelLines)
{
	size		ui;
	size		uiTarget;
	int			iSteps;
	SIR816Line	sLine;
	char		szLabel[32];

	ui = uiFrom;
	for (iSteps = 0; (iSteps < 32) && (ui < paszLines->NumElements()); iSteps++)
	{
		if (IR816IsLabel(paszLines->Get(ui)))
		{
			ui++;
			continue;
		}
		if (IR816DecodeLine(paszLines->Get(ui), &sLine) && IR816LeavesA(&sLine))
		{
			ui++;
			continue;
		}
		if (IR816BranchLine(paszLines->Get(ui), &sLine, szLabel) && (sLine.uiMnemonic == IR816M_Bra))
		{
			uiTarget = IR816LabelledInstruction(paszLines, szLabel, pauiLabelLines);
			if (uiTarget == (size)-1)
			{
				return false;
			}
			ui = uiTarget;
			continue;
		}
		return IR816LineHas(paszLines->Get(ui), IR816MF_LOADS_A);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the straight line code from line uiFrom on writes X (tax, ldx,
// plx) before anything that might read it: only instructions that don't
// (on A, Y, the flags or the direct page, not "[D + X]") come between, and
// no label, branch, call or return.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SetsXFirst(CArrayChars* paszLines, size uiFrom)
{
	size		ui;
	CChars*		psz;
	SIR816Line	sLine;

	for (ui = uiFrom; ui < paszLines->NumElements(); ui++)
	{
		psz = paszLines->Get(ui);
		if (!IR816DecodeLine(psz, &sLine) || (strstr(psz->Text(), "X]") != NULL) || (strstr(psz->Text(), ", X") != NULL))
		{
			return false;
		}
		if ((sLine.uiFlags & IR816MF_WRITES_X) && (sLine.uiMnemonic != IR816M_Tyx))
		{
			return true;
		}
		if (!(sLine.uiFlags & (IR816MF_A_ALU | IR816MF_Y_ALU | IR816MF_CARRY | IR816MF_WIDTH | IR816MF_SHIFT_STEP)) && (sLine.uiMnemonic != IR816M_Stz))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The local labels (L and a number) the lines name anywhere but in their
// own definitions (branches, jumps and jump tables' .words), marked by
// number in pabUsed.
//
//////////////////////////////////////////////////////////////////////////
static void IR816FindUsedLabels(CArrayChars* paszLines, CArrayTemplate<uint8>* pabUsed)
{
	const char*	szText;
	size		ui;
	size		uiNumber;
	uint8		uiZero;
	uint8		uiOne;

	uiZero = 0;
	uiOne = 1;
	pabUsed->FakeSetUsedElements(0);
	for (ui = 0; ui < paszLines->NumElements(); ui++)
	{
		szText = paszLines->GetText(ui);
		if (szText[0] == 'L')
		{
			//A label's definition.
			continue;
		}
		for (; *szText; szText++)
		{
			if ((szText[0] == 'L') && (szText[1] >= '0') && (szText[1] <= '9') && ((szText == paszLines->GetText(ui)) || !IR816IsNameCharacter(szText[-1])))
			{
				uiNumber = (size)atoi(&szText[1]);
				while (pabUsed->NumElements() <= uiNumber)
				{
					pabUsed->Add(&uiZero);
				}
				pabUsed->Set(uiNumber, &uiOne);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False if psz is a local label's definition ("L12:") that nothing names
// (IR816FindUsedLabels); true for any other line.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LabelUsed(CChars* psz, CArrayTemplate<uint8>* pabUsed)
{
	const char*	szText;
	size		ui;
	size		uiNumber;

	szText = psz->Text();
	if ((szText[0] != 'L') || (szText[1] < '0') || (szText[1] > '9'))
	{
		return true;
	}
	for (ui = 1; (szText[ui] >= '0') && (szText[ui] <= '9'); ui++)
	{
	}
	if ((szText[ui] != ':') || (szText[ui + 1] != '\0'))
	{
		return true;
	}
	uiNumber = (size)atoi(&szText[1]);
	return (uiNumber < pabUsed->NumElements()) && (*pabUsed->Get(uiNumber) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a branch line, "\t\tszMnemonic szLabel", over psz.
//
//////////////////////////////////////////////////////////////////////////
static void IR816SetBranch(CChars* psz, const char* szMnemonic, const char* szLabel)
{
	psz->Clear();
	psz->Append("\t\t");
	psz->Append(szMnemonic);
	psz->Append(" ");
	psz->Append(szLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// Branches tidied in the code written from uiStart on, until none changes:
// a bra to the label that follows goes; a branch to a label whose first
// instruction is a bra goes where that does; Bcc X, bra Y, X: is B!cc Y,
// X: (and bcc X, beq X, bra Y, X: is beq X, bcs Y, X:); Bcc X, bra Y
// round a loop (marked IR816_ROUND by JumpTo) is B!cc Y, bra X; an lda of a direct
// page word or a constant goes when what runs next (falling through or
// after a bra) sets A and N and Z before reading A or the flags; a tax goes
// when X is written before it is read (IR816SetsXFirst); txa after tax (or tax after txa),
// only clc or sec between, goes; a txy goes after a txy and an instruction that changes neither X
// nor Y; and code after a bra, rtl or stp up to a label that is used goes
// (IR816Unreachable).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeBranches(CChars* pszDest, size uiStart)
{
	CArrayTemplate<uint8>	abUsed;
	CArrayTemplate<size>	auiLabelLines;
	CArrayChars				aszLines;
	CChars					szOut;
	CChars*					psz;
	SIR816Line				sLine;
	SIR816Line				sLine2;
	SIR816Line				sLine3;
	char					szLabel[32];
	char					szLabel2[32];
	char					szLabel3[32];
	size					ui;
	size					uiTarget;
	int						iPass;
	bool					bChanged;

	aszLines.Init();
	aszLines.Split(pszDest->Text() + uiStart, '\n');
	if ((aszLines.NumElements() > 0) && aszLines.Tail()->Empty())
	{
		aszLines.RemoveTail();
	}
	abUsed.Init();
	auiLabelLines.Init();
	for (iPass = 0; iPass < 8; iPass++)
	{
		bChanged = false;
		IR816FindUsedLabels(&aszLines, &abUsed);
		for (ui = 0; ui < aszLines.NumElements(); ui++)
		{
			psz = aszLines.Get(ui);
			if (!IR816LabelUsed(psz, &abUsed))
			{
				//A label nothing goes to (its branches were sent elsewhere): it would keep the rules below apart.
				aszLines.Remove(ui);
				ui--;
				bChanged = true;
				continue;
			}
			if (IR816BranchLine(psz, &sLine, szLabel))
			{
				//To a bra: straight to where it goes.
				uiTarget = IR816LabelledInstruction(&aszLines, szLabel, &auiLabelLines);
				if ((uiTarget != (size)-1) && IR816BranchLine(aszLines.Get(uiTarget), &sLine2, szLabel2) && (sLine2.uiMnemonic == IR816M_Bra) &&
					(StringCompare(szLabel2, szLabel) != 0))
				{
					IR816SetBranch(psz, sLine.szMnemonic, szLabel2);
					bChanged = true;
					continue;
				}

				//A bra to the next label.
				if ((sLine.uiMnemonic == IR816M_Bra) && IR816LabelFollows(&aszLines, ui + 1, szLabel))
				{
					aszLines.Remove(ui);
					ui--;
					bChanged = true;
					continue;
				}

				//Bcc X, bra Y, X:
				if ((ui + 1 < aszLines.NumElements()) && IR816InvertedBranch(sLine.uiMnemonic) && IR816BranchLine(aszLines.Get(ui + 1), &sLine2, szLabel2) &&
					(sLine2.uiMnemonic == IR816M_Bra) && IR816LabelFollows(&aszLines, ui + 2, szLabel))
				{
					IR816SetBranch(psz, IR816InvertedBranch(sLine.uiMnemonic), szLabel2);
					aszLines.Remove(ui + 1);
					bChanged = true;
					continue;
				}

				//Bcc X, bra Y round a loop (JumpTo marked it) is B!cc Y, bra X: going round, the usual way, is one branch taken.
				if ((ui + 1 < aszLines.NumElements()) && IR816InvertedBranch(sLine.uiMnemonic) && IR816BranchLine(aszLines.Get(ui + 1), &sLine2, szLabel2) &&
					(sLine2.uiMnemonic == IR816M_Bra) && aszLines.Get(ui + 1)->EndsWith(IR816_ROUND) && (StringCompare(szLabel, szLabel2) != 0))
				{
					IR816SetBranch(psz, IR816InvertedBranch(sLine.uiMnemonic), szLabel2);
					IR816SetBranch(aszLines.Get(ui + 1), "bra", szLabel);
					bChanged = true;
					continue;
				}

				//bcc X, beq X, bra Y, X: (either order of the first two) is beq X, bcs Y, X:.
				if ((ui + 2 < aszLines.NumElements()) && IR816BranchLine(aszLines.Get(ui + 1), &sLine2, szLabel2) &&
					IR816BranchLine(aszLines.Get(ui + 2), &sLine3, szLabel3) && (sLine3.uiMnemonic == IR816M_Bra) && (StringCompare(szLabel, szLabel2) == 0) &&
					(((sLine.uiMnemonic == IR816M_Bcc) && (sLine2.uiMnemonic == IR816M_Beq)) || ((sLine.uiMnemonic == IR816M_Beq) && (sLine2.uiMnemonic == IR816M_Bcc))) &&
					IR816LabelFollows(&aszLines, ui + 3, szLabel))
				{
					IR816SetBranch(psz, "beq", szLabel);
					IR816SetBranch(aszLines.Get(ui + 1), "bcs", szLabel3);
					aszLines.Remove(ui + 2);
					bChanged = true;
					continue;
				}
				continue;
			}

			//An lda (of a direct page word or a constant) for nothing: what runs next sets A before reading A or N and Z.
			if ((psz->Length() > 3) && IR816DecodeLine(psz, &sLine))
			{
				if ((sLine.uiMnemonic == IR816M_Lda) && ((strncmp(sLine.szOperand, "[D + ", 5) == 0) || IR816IsDecimal(sLine.szOperand)) &&
					IR816SetsAFirst(&aszLines, ui + 1, &auiLabelLines))
				{
					aszLines.Remove(ui);
					ui--;
					bChanged = true;
					continue;
				}
			}

			//A tax for nothing: X is written again before anything reads it.
			if (psz->Equals("\t\ttax") && IR816SetsXFirst(&aszLines, ui + 1))
			{
				aszLines.Remove(ui);
				ui--;
				bChanged = true;
				continue;
			}

			//tax then txa, or txa then tax, with only clc or sec between: A and X hold the same already (and N and Z
			//are as the first set them).
			if (psz->Equals("\t\ttax") || psz->Equals("\t\ttxa"))
			{
				for (uiTarget = ui + 1; (uiTarget < aszLines.NumElements()) && (aszLines.Get(uiTarget)->Equals("\t\tclc") || aszLines.Get(uiTarget)->Equals("\t\tsec")); uiTarget++)
				{
				}
				if ((uiTarget < aszLines.NumElements()) && aszLines.Get(uiTarget)->Equals((psz->Text()[3] == 'a') ? "\t\ttxa" : "\t\ttax"))
				{
					aszLines.Remove(uiTarget);
					bChanged = true;
					continue;
				}
			}

			//txy, an instruction that keeps X and Y (lda, sta, and, ora, eor, adc, sbc, cmp, sep, rep, clc or sec), txy.
			if ((ui + 2 < aszLines.NumElements()) && psz->Equals("\t\ttxy") && IR816DecodeLine(aszLines.Get(ui + 1), &sLine) &&
				(((sLine.uiFlags & IR816MF_A_ALU) && (sLine.uiMnemonic != IR816M_Bit)) || (sLine.uiFlags & (IR816MF_WIDTH | IR816MF_CARRY))) &&
				aszLines.Get(ui + 2)->Equals("\t\ttxy"))
			{
				aszLines.Remove(ui + 2);
				bChanged = true;
				continue;
			}
		}
		bChanged |= IR816Unreachable(&aszLines);
		if (!bChanged)
		{
			break;
		}
	}

	szOut.Init();
	for (ui = 0; ui < aszLines.NumElements(); ui++)
	{
		psz = aszLines.Get(ui);
		if (psz->EndsWith(IR816_ROUND))
		{
			psz->RemoveFromEnd(StrLen(IR816_ROUND));
		}
		szOut.Append(psz);
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut);
	szOut.Kill();
	aszLines.Kill();
	auiLabelLines.Kill();
	abUsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The line after the one at szLine; NULL at the end.
//
//////////////////////////////////////////////////////////////////////////
const char* IR816NextLine(const char* szLine)
{
	const char*	szEnd;

	szEnd = strchr(szLine, '\n');
	return (szEnd && szEnd[1]) ? szEnd + 1 : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The end of the line at szLine: its new line, or the end of the text.
//
//////////////////////////////////////////////////////////////////////////
const char* IR816LineEnd(const char* szLine)
{
	const char*	szEnd;

	szEnd = strchr(szLine, '\n');
	return szEnd ? szEnd : szLine + StrLen(szLine);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether N or Z may be read after the line at szLine before an
// instruction sets them again (stores, register pushes, clc and sec don't;
// a label or the end might lead to a read).
//
//////////////////////////////////////////////////////////////////////////
bool IR816ZNReadAfter(const char* szLine)
{
	SIR816Line	sLine;

	for (szLine = IR816NextLine(szLine); szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return true;
		}
		IR816Decode(szLine, &sLine);
		if (sLine.uiFlags & IR816MF_BRANCH_ZN)
		{
			return true;
		}
		if (!(sLine.uiFlags & (IR816MF_STORE | IR816MF_PUSH_REGISTER | IR816MF_CARRY)))
		{
			return false;
		}
	}
	return true;
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
// Whether an operand may read or write the direct page bytes near
// iOffset: it names [D + n] with n within 2 of it (a pointer is 3 bytes;
// words may start at either byte), or [D + X].  A whole operand [D + n] is
// a word, bytes n and n + 1: near only when it shares one with iOffset's.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816MentionsNear(const char* szOperand, size uiOperand, int32 iOffset)
{
	size	ui;
	int32	iValue;
	size	uiDigit;
	bool	bWord;

	for (ui = 0; ui + 5 <= uiOperand; ui++)
	{
		if (strncmp(szOperand + ui, "D + ", 4) != 0)
		{
			continue;
		}
		if (szOperand[ui + 4] == 'X')
		{
			return true;
		}
		iValue = 0;
		for (uiDigit = ui + 4; (uiDigit < uiOperand) && (szOperand[uiDigit] >= '0') && (szOperand[uiDigit] <= '9'); uiDigit++)
		{
			iValue = iValue * 10 + (szOperand[uiDigit] - '0');
		}
		bWord = (ui == 1) && (szOperand[0] == '[') && (uiDigit + 1 == uiOperand) && (szOperand[uiDigit] == ']');
		if (bWord ? ((iValue >= iOffset - 1) && (iValue <= iOffset + 1)) : ((iValue >= iOffset - 2) && (iValue <= iOffset + 2)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a store to the direct page word szWord (as "[D + 12]") by the line
// before szLine is never read: in straight line code (no label, jump, call
// or width change) the next instruction that names a byte near it, other
// than a store to another direct page word, stores a word to it.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816DeadStore(const char* szLine, const char* szWord, size uiWord)
{
	SIR816Line	sLine;
	int32		iOffset;
	bool		bStore;

	iOffset = atoi(szWord + 5);
	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
		if (sLine.uiFlags & (IR816MF_LEAVES | IR816MF_WIDTH | IR816MF_BLOCK_MOVE | IR816MF_SETS_D | IR816MF_INTERRUPT))
		{
			return false;
		}
		if (IR816MentionsNear(sLine.szOperand, sLine.uiOperand, iOffset))
		{
			bStore = (sLine.uiFlags & IR816MF_STORE) != 0;
			if (bStore && IR816OperandIs(&sLine, szWord, uiWord))
			{
				return true;
			}
			if (bStore && (strncmp(sLine.szOperand, "[D + ", 5) == 0) && (sLine.szOperand[sLine.uiOperand - 1] == ']') && !memchr(sLine.szOperand, ':', sLine.uiOperand))
			{
				//A store to another direct page word reads nothing.
				continue;
			}
			return false;
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

	szOut.Init();
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

	szOut.Init();
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
// Nothing known of what registers iFirst to iLast (0 A, 1 X, 2 Y) hold.
//
//////////////////////////////////////////////////////////////////////////
void IR816ForgetHeld(SIR816Held* asHeld, int iFirst, int iLast)
{
	int	i;
	int	j;

	for (i = iFirst; i <= iLast; i++)
	{
		for (j = 0; j < IR816_HELD; j++)
		{
			asHeld[i].aszOperand[j][0] = '\0';
			asHeld[i].aiOffset[j] = -1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What registers hold of a direct page word a 16 bit write at [D + iOffset]
// may change (any word overlapping it), or, for an iOffset of -1 (a write
// through a pointer or an index, which may reach the frame), of any word.
//
//////////////////////////////////////////////////////////////////////////
static void IR816ForgetWritten(SIR816Held* asHeld, int32 iOffset)
{
	int	i;
	int	j;

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < IR816_HELD; j++)
		{
			if ((asHeld[i].aiOffset[j] >= 0) && ((iOffset < 0) || ((asHeld[i].aiOffset[j] >= iOffset - 1) && (asHeld[i].aiOffset[j] <= iOffset + 1))))
			{
				asHeld[i].aszOperand[j][0] = '\0';
				asHeld[i].aiOffset[j] = -1;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction's operand is one a register's contents are known
// as: a direct page word or a constant.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsHeldOperand(SIR816Line* psLine)
{
	size	ui;

	if (psLine->bDirect)
	{
		return true;
	}
	if ((psLine->uiOperand == 0) || (psLine->uiOperand >= IR816_HELD_LENGTH) || (psLine->szOperand[0] < '0') || (psLine->szOperand[0] > '9'))
	{
		return false;
	}
	for (ui = 0; ui < psLine->uiOperand; ui++)
	{
		if (psLine->szOperand[ui] == ' ')
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether register iRegister holds the instruction's operand.
//
//////////////////////////////////////////////////////////////////////////
bool IR816Holds(SIR816Held* asHeld, int iRegister, SIR816Line* psLine)
{
	int	j;

	for (j = 0; j < IR816_HELD; j++)
	{
		if ((asHeld[iRegister].aszOperand[j][0] != '\0') && (StrLen(asHeld[iRegister].aszOperand[j]) == psLine->uiOperand) &&
			(strncmp(asHeld[iRegister].aszOperand[j], psLine->szOperand, psLine->uiOperand) == 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Register iRegister holds the instruction's operand: only it after a load
// (bAlso false), as well as what it held after a store (bAlso).  Nothing
// is known after a load of anything but a direct page word or a constant.
//
//////////////////////////////////////////////////////////////////////////
static void IR816Hold(SIR816Held* asHeld, int iRegister, SIR816Line* psLine, bool bAlso)
{
	SIR816Held*	psHeld;

	psHeld = &asHeld[iRegister];
	if (!bAlso)
	{
		IR816ForgetHeld(asHeld, iRegister, iRegister);
	}
	if (!IR816IsHeldOperand(psLine) || IR816Holds(asHeld, iRegister, psLine))
	{
		return;
	}
	//The operand first, what it held second (the oldest is forgotten).
	memcpy(psHeld->aszOperand[1], psHeld->aszOperand[0], IR816_HELD_LENGTH);
	psHeld->aiOffset[1] = psHeld->aiOffset[0];
	memcpy(psHeld->aszOperand[0], psLine->szOperand, psLine->uiOperand);
	psHeld->aszOperand[0][psLine->uiOperand] = '\0';
	psHeld->aiOffset[0] = psLine->bDirect ? psLine->iOffset : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// What A, X and Y hold after the instruction (PeepholeRegisters).
//
//////////////////////////////////////////////////////////////////////////
void IR816UpdateHeld(SIR816Held* asHeld, SIR816Line* psLine)
{
	bool	bOfA;

	switch (psLine->uiMnemonic)
	{
	case IR816M_Lda:	IR816Hold(asHeld, 0, psLine, false);	return;
	case IR816M_Ldx:	IR816Hold(asHeld, 1, psLine, false);	return;
	case IR816M_Ldy:	IR816Hold(asHeld, 2, psLine, false);	return;
	case IR816M_Sta:
	case IR816M_Stx:
	case IR816M_Sty:
	case IR816M_Stz:
		IR816ForgetWritten(asHeld, psLine->bDirect ? psLine->iOffset : -1);
		if (psLine->bDirect && (psLine->uiMnemonic != IR816M_Stz))
		{
			IR816Hold(asHeld, (psLine->uiMnemonic == IR816M_Sta) ? 0 : (psLine->uiMnemonic == IR816M_Stx) ? 1 : 2, psLine, true);
		}
		return;
	case IR816M_Tax:	asHeld[1] = asHeld[0];	return;
	case IR816M_Tay:	asHeld[2] = asHeld[0];	return;
	case IR816M_Txa:	asHeld[0] = asHeld[1];	return;
	case IR816M_Tya:	asHeld[0] = asHeld[2];	return;
	case IR816M_Txy:	asHeld[2] = asHeld[1];	return;
	case IR816M_Tyx:	asHeld[1] = asHeld[2];	return;
	case IR816M_Inc:
	case IR816M_Dec:
	case IR816M_Asl:
	case IR816M_Lsr:
	case IR816M_Rol:
	case IR816M_Ror:
	case IR816M_Tsb:
	case IR816M_Trb:
		bOfA = (psLine->uiOperand == 0) || ((psLine->uiOperand == 1) && (psLine->szOperand[0] == 'A'));
		if (bOfA)
		{
			IR816ForgetHeld(asHeld, 0, 0);
		}
		else
		{
			IR816ForgetWritten(asHeld, psLine->bDirect ? psLine->iOffset : -1);
		}
		return;
	case IR816M_Inx:
	case IR816M_Dex:
	case IR816M_Plx:
	case IR816M_Tsx:
		IR816ForgetHeld(asHeld, 1, 1);
		return;
	case IR816M_Iny:
	case IR816M_Dey:
	case IR816M_Ply:
		IR816ForgetHeld(asHeld, 2, 2);
		return;
	case IR816M_Bcc:
	case IR816M_Bcs:
	case IR816M_Beq:
	case IR816M_Bmi:
	case IR816M_Bne:
	case IR816M_Bpl:
	case IR816M_Bvc:
	case IR816M_Bvs:
	case IR816M_Cmp:
	case IR816M_Cpx:
	case IR816M_Cpy:
	case IR816M_Bit:
	case IR816M_Clc:
	case IR816M_Sec:
	case IR816M_Clv:
	case IR816M_Nop:
	case IR816M_Pha:
	case IR816M_Phx:
	case IR816M_Phy:
	case IR816M_Php:
	case IR816M_Phb:
	case IR816M_Phd:
	case IR816M_Phk:
	case IR816M_Pea:
	case IR816M_Pei:
	case IR816M_Per:
		return;
	case IR816M_Adc:
	case IR816M_Sbc:
	case IR816M_And:
	case IR816M_Ora:
	case IR816M_Eor:
	case IR816M_Pla:
	case IR816M_Xba:
	case IR816M_Tsc:
	case IR816M_Tdc:
		IR816ForgetHeld(asHeld, 0, 0);
		return;
	default:
		//Calls, jumps, returns, width and mode changes, moves and D or the stack changing: nothing is known after.
		IR816ForgetHeld(asHeld, 0, 2);
		return;
	}
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

	szOut.Init();
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

	szOut.Init();
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
// Whether an lda before the line at szLine is never read: in straight line
// code, before anything reads A or the N and Z the lda set, an instruction
// sets A again (only IR816LeavesA instructions come between).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816DeadLoad(const char* szLine)
{
	SIR816Line	sLine;

	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
		if (sLine.uiFlags & IR816MF_LOADS_A)
		{
			return true;
		}
		if (!IR816LeavesA(&sLine))
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
	szOut.Init();
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


//////////////////////////////////////////////////////////////////////////
//
// Whether the tdc at szLine only moves S: a tcs follows it, after nothing
// but a clc and an adc of a constant (a frame's exit).
//
//////////////////////////////////////////////////////////////////////////
bool IR816MovesS(const char* szLine)
{
	SIR816Line	sLine;
	int			i;

	for (i = 0, szLine = IR816NextLine(szLine); (i < 3) && szLine && IR816IsInstruction(szLine); i++, szLine = IR816NextLine(szLine))
	{
		IR816Decode(szLine, &sLine);
		if (sLine.uiMnemonic == IR816M_Tcs)
		{
			return true;
		}
		if ((sLine.uiMnemonic != IR816M_Clc) && !((sLine.uiMnemonic == IR816M_Adc) && (sLine.uiOperand > 0) && (sLine.szOperand[0] >= '0') && (sLine.szOperand[0] <= '9')))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The direct page bytes the function's code at szLine may read: for each
// instruction mentioning [D + n] other than a store to it, the word's two
// bytes ([D + n]), or a pointer's three ([[D + n]:24 ...]), and the
// scratch bytes when it calls a runtime routine (they work in the caller's
// frame, IR816ScratchRoutine).  True (all of them may be) when the code
// indexes the direct page ([D + X]) or takes D's value (tdc: the frame's
// address may be passed on), unless only to move S (IR816MovesS).
//
//////////////////////////////////////////////////////////////////////////
bool IR816FindReads(const char* szLine, uint8* abRead)
{
	SIR816Line	sLine;
	const char*	szOperand;
	int32		iOffset;
	int			iBytes;
	int			i;

	memset(abRead, 0, IR816_DIRECT_PAGE);
	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine) || (szLine[2] == ';'))
		{
			continue;
		}
		IR816Decode(szLine, &sLine);
		if ((sLine.uiMnemonic == IR816M_Tdc) && !IR816MovesS(szLine))
		{
			return true;
		}
		if (sLine.uiMnemonic == IR816M_Unknown)
		{
			return true;
		}
		if ((sLine.uiMnemonic == IR816M_Jsl) && (sLine.uiOperand > 3) && (strncmp(&sLine.szOperand[sLine.uiOperand - 3], IR816_FRAME_OPERANDS, 3) == 0))
		{
			return true;
		}
		if ((sLine.uiMnemonic == IR816M_Jsl) && (strncmp(sLine.szOperand, IR816_RUNTIME_PREFIX, IR816_RUNTIME_PREFIX_LENGTH) == 0))
		{
			if (IR816IndexedRoutine(sLine.szOperand, sLine.uiOperand))
			{
				return true;
			}
			memset(&abRead[IR816_SCRATCH_FIRST], 1, IR816_SCRATCH_END - IR816_SCRATCH_FIRST);
			continue;
		}
		szOperand = sLine.szOperand;
		if ((sLine.uiOperand >= 7) && (strncmp(szOperand, "[D + X]", 7) == 0))
		{
			return true;
		}
		if (sLine.bDirect)
		{
			if (sLine.uiFlags & IR816MF_STORE)
			{
				continue;
			}
			iOffset = sLine.iOffset;
			iBytes = 2;
		}
		else if ((sLine.uiOperand >= 7) && (strncmp(szOperand, "[[D + ", 6) == 0) && (szOperand[6] >= '0') && (szOperand[6] <= '9'))
		{
			iOffset = atoi(&szOperand[6]);
			iBytes = 3;
		}
		else
		{
			continue;
		}
		for (i = 0; i < iBytes; i++)
		{
			if (iOffset + i >= IR816_DIRECT_PAGE)
			{
				return true;
			}
			abRead[iOffset + i] = 1;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Stores to a direct page word whose bytes nothing in the function's code
// from uiStart reads (IR816FindReads) taken out, wherever they are: the
// high words of values only their low words of are used (a shift's
// count), and values worked out for nothing.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::RemoveUnreadStores(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	SIR816Line	sLine;
	uint8		abRead[IR816_DIRECT_PAGE];

	if (IR816FindReads(pszDest->Text() + uiStart, abRead))
	{
		return;
	}
	szOut.Init();
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		if (IR816IsInstruction(szLine) && (szLine[2] != ';'))
		{
			IR816Decode(szLine, &sLine);
			if (sLine.bDirect && (sLine.uiFlags & IR816MF_STORE) && (sLine.iOffset < IR816_DIRECT_PAGE - 1) && !abRead[sLine.iOffset] && !abRead[sLine.iOffset + 1])
			{
				continue;
			}
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut.Text(), szOut.Length());
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// After Peephole: stores to a direct page word that the next store to it
// overwrites unread (IR816DeadStore) go, and loads of a direct page word or
// a constant that A is loaded again over before it is read (IR816DeadLoad),
// until none are left.
// Places are shared, so a value worked out in steps is often stored after
// each.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::RemoveDeadStores(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	SIR816Line	sLine;
	bool		bRemoved;

	//Again while any went: the lda that fed a store that went is loaded over unread now.
	do
	{
		bRemoved = false;
		szOut.Init();
		for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
		{
			szEnd = IR816LineEnd(szLine);
			if ((szEnd - szLine >= 3) && IR816IsInstruction(szLine) && (*szEnd == '\n'))
			{
				IR816Decode(szLine, &sLine);
				if (sLine.bDirect && (sLine.uiFlags & IR816MF_STORE) && IR816DeadStore(szEnd + 1, sLine.szOperand, sLine.uiOperand))
				{
					bRemoved = true;
					continue;
				}
				if ((sLine.uiMnemonic == IR816M_Lda) && (sLine.bDirect || ((sLine.uiOperand > 0) && (sLine.szOperand[0] != '['))) && IR816DeadLoad(szEnd + 1))
				{
					bRemoved = true;
					continue;
				}
			}
			szOut.Append(szLine, (size)(szEnd - szLine));
			szOut.AppendNewLine();
		}
		pszDest->SetLength(uiStart);
		pszDest->Append(szOut.Text(), szOut.Length());
		szOut.Kill();
	}
	while (bRemoved);
}

