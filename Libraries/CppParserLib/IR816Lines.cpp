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


//The W65C816 back end's assembly text as its passes read it: the lines decoded (IR816Decode), and what A, X and Y
//are known to hold along straight line code (IR816UpdateHeld).


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
// Whether the instruction's operand is exactly szOperand (uiOperand long).
//
//////////////////////////////////////////////////////////////////////////
bool IR816OperandIs(SIR816Line* psLine, const char* szOperand, size uiOperand)
{
	return (psLine->uiOperand == uiOperand) && (strncmp(psLine->szOperand, szOperand, uiOperand) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction reads neither A nor the N and Z an lda before it
// set, and leaves A as it is: clc, sec, ldx, ldy, inx, iny, dex, dey, stx,
// sty, stz, pea, pei, phk, phx, phy, cpx, cpy, and shifts and steps of
// memory.  An lda followed only by these and then a load of A is dead.
//
//////////////////////////////////////////////////////////////////////////
bool IR816LeavesA(SIR816Line* psLine)
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
// The branch that goes when uiMnemonic's doesn't, or NULL.
//
//////////////////////////////////////////////////////////////////////////
const char* IR816InvertedBranch(uint16 uiMnemonic)
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
// A decimal constant operand.  The 0x and 0b constants the backend writes
// for masks and sign flips are not taken as plain constants.
//
//////////////////////////////////////////////////////////////////////////
bool IR816IsDecimal(const char* szOperand)
{
	if ((szOperand[0] < '0') || (szOperand[0] > '9'))
	{
		return false;
	}
	return !((szOperand[0] == '0') && ((szOperand[1] == 'x') || (szOperand[1] == 'b')));
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
