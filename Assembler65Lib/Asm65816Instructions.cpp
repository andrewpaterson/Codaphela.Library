/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "Asm65816Assembler.h"


#define ASM65816_MAX_MNEMONICS	96


struct SAsm65816Opcode
{
	const char*	szMnemonic;
	uint8		uiMode;
};


//Each opcode's mnemonic and addressing mode (W65C816S datasheet table 5-4).  The
//test TestAsm65816Opcodes checks it against MicroprocessorLib's instruction factory.
static const SAsm65816Opcode	gasAsm65816Opcodes[256] = {
	{ "brk", A65816M_Immediate8 },  { "ora", A65816M_DirectIndirectX },  { "cop", A65816M_Immediate8 },  { "ora", A65816M_Stack },
	{ "tsb", A65816M_Direct },  { "ora", A65816M_Direct },  { "asl", A65816M_Direct },  { "ora", A65816M_DirectIndirectLong },
	{ "php", A65816M_Implied },  { "ora", A65816M_ImmediateM },  { "asl", A65816M_Accumulator },  { "phd", A65816M_Implied },
	{ "tsb", A65816M_Absolute },  { "ora", A65816M_Absolute },  { "asl", A65816M_Absolute },  { "ora", A65816M_Long },
	{ "bpl", A65816M_Relative },  { "ora", A65816M_DirectIndirectY },  { "ora", A65816M_DirectIndirect },  { "ora", A65816M_StackIndirectY },
	{ "trb", A65816M_Direct },  { "ora", A65816M_DirectX },  { "asl", A65816M_DirectX },  { "ora", A65816M_DirectIndirectLongY },
	{ "clc", A65816M_Implied },  { "ora", A65816M_AbsoluteY },  { "inc", A65816M_Accumulator },  { "tcs", A65816M_Implied },
	{ "trb", A65816M_Absolute },  { "ora", A65816M_AbsoluteX },  { "asl", A65816M_AbsoluteX },  { "ora", A65816M_LongX },
	{ "jsr", A65816M_JumpAbsolute },  { "and", A65816M_DirectIndirectX },  { "jsl", A65816M_JumpLong },  { "and", A65816M_Stack },
	{ "bit", A65816M_Direct },  { "and", A65816M_Direct },  { "rol", A65816M_Direct },  { "and", A65816M_DirectIndirectLong },
	{ "plp", A65816M_Implied },  { "and", A65816M_ImmediateM },  { "rol", A65816M_Accumulator },  { "pld", A65816M_Implied },
	{ "bit", A65816M_Absolute },  { "and", A65816M_Absolute },  { "rol", A65816M_Absolute },  { "and", A65816M_Long },
	{ "bmi", A65816M_Relative },  { "and", A65816M_DirectIndirectY },  { "and", A65816M_DirectIndirect },  { "and", A65816M_StackIndirectY },
	{ "bit", A65816M_DirectX },  { "and", A65816M_DirectX },  { "rol", A65816M_DirectX },  { "and", A65816M_DirectIndirectLongY },
	{ "sec", A65816M_Implied },  { "and", A65816M_AbsoluteY },  { "dec", A65816M_Accumulator },  { "tsc", A65816M_Implied },
	{ "bit", A65816M_AbsoluteX },  { "and", A65816M_AbsoluteX },  { "rol", A65816M_AbsoluteX },  { "and", A65816M_LongX },
	{ "rti", A65816M_Implied },  { "eor", A65816M_DirectIndirectX },  { "wdm", A65816M_Immediate8 },  { "eor", A65816M_Stack },
	{ "mvp", A65816M_BlockMove },  { "eor", A65816M_Direct },  { "lsr", A65816M_Direct },  { "eor", A65816M_DirectIndirectLong },
	{ "pha", A65816M_Implied },  { "eor", A65816M_ImmediateM },  { "lsr", A65816M_Accumulator },  { "phk", A65816M_Implied },
	{ "jmp", A65816M_JumpAbsolute },  { "eor", A65816M_Absolute },  { "lsr", A65816M_Absolute },  { "eor", A65816M_Long },
	{ "bvc", A65816M_Relative },  { "eor", A65816M_DirectIndirectY },  { "eor", A65816M_DirectIndirect },  { "eor", A65816M_StackIndirectY },
	{ "mvn", A65816M_BlockMove },  { "eor", A65816M_DirectX },  { "lsr", A65816M_DirectX },  { "eor", A65816M_DirectIndirectLongY },
	{ "cli", A65816M_Implied },  { "eor", A65816M_AbsoluteY },  { "phy", A65816M_Implied },  { "tcd", A65816M_Implied },
	{ "jml", A65816M_JumpLong },  { "eor", A65816M_AbsoluteX },  { "lsr", A65816M_AbsoluteX },  { "eor", A65816M_LongX },
	{ "rts", A65816M_Implied },  { "adc", A65816M_DirectIndirectX },  { "per", A65816M_RelativeLong },  { "adc", A65816M_Stack },
	{ "stz", A65816M_Direct },  { "adc", A65816M_Direct },  { "ror", A65816M_Direct },  { "adc", A65816M_DirectIndirectLong },
	{ "pla", A65816M_Implied },  { "adc", A65816M_ImmediateM },  { "ror", A65816M_Accumulator },  { "rtl", A65816M_Implied },
	{ "jmp", A65816M_JumpIndirect },  { "adc", A65816M_Absolute },  { "ror", A65816M_Absolute },  { "adc", A65816M_Long },
	{ "bvs", A65816M_Relative },  { "adc", A65816M_DirectIndirectY },  { "adc", A65816M_DirectIndirect },  { "adc", A65816M_StackIndirectY },
	{ "stz", A65816M_DirectX },  { "adc", A65816M_DirectX },  { "ror", A65816M_DirectX },  { "adc", A65816M_DirectIndirectLongY },
	{ "sei", A65816M_Implied },  { "adc", A65816M_AbsoluteY },  { "ply", A65816M_Implied },  { "tdc", A65816M_Implied },
	{ "jmp", A65816M_JumpIndirectX },  { "adc", A65816M_AbsoluteX },  { "ror", A65816M_AbsoluteX },  { "adc", A65816M_LongX },
	{ "bra", A65816M_Relative },  { "sta", A65816M_DirectIndirectX },  { "brl", A65816M_RelativeLong },  { "sta", A65816M_Stack },
	{ "sty", A65816M_Direct },  { "sta", A65816M_Direct },  { "stx", A65816M_Direct },  { "sta", A65816M_DirectIndirectLong },
	{ "dey", A65816M_Implied },  { "bit", A65816M_ImmediateM },  { "txa", A65816M_Implied },  { "phb", A65816M_Implied },
	{ "sty", A65816M_Absolute },  { "sta", A65816M_Absolute },  { "stx", A65816M_Absolute },  { "sta", A65816M_Long },
	{ "bcc", A65816M_Relative },  { "sta", A65816M_DirectIndirectY },  { "sta", A65816M_DirectIndirect },  { "sta", A65816M_StackIndirectY },
	{ "sty", A65816M_DirectX },  { "sta", A65816M_DirectX },  { "stx", A65816M_DirectY },  { "sta", A65816M_DirectIndirectLongY },
	{ "tya", A65816M_Implied },  { "sta", A65816M_AbsoluteY },  { "txs", A65816M_Implied },  { "txy", A65816M_Implied },
	{ "stz", A65816M_Absolute },  { "sta", A65816M_AbsoluteX },  { "stz", A65816M_AbsoluteX },  { "sta", A65816M_LongX },
	{ "ldy", A65816M_ImmediateX },  { "lda", A65816M_DirectIndirectX },  { "ldx", A65816M_ImmediateX },  { "lda", A65816M_Stack },
	{ "ldy", A65816M_Direct },  { "lda", A65816M_Direct },  { "ldx", A65816M_Direct },  { "lda", A65816M_DirectIndirectLong },
	{ "tay", A65816M_Implied },  { "lda", A65816M_ImmediateM },  { "tax", A65816M_Implied },  { "plb", A65816M_Implied },
	{ "ldy", A65816M_Absolute },  { "lda", A65816M_Absolute },  { "ldx", A65816M_Absolute },  { "lda", A65816M_Long },
	{ "bcs", A65816M_Relative },  { "lda", A65816M_DirectIndirectY },  { "lda", A65816M_DirectIndirect },  { "lda", A65816M_StackIndirectY },
	{ "ldy", A65816M_DirectX },  { "lda", A65816M_DirectX },  { "ldx", A65816M_DirectY },  { "lda", A65816M_DirectIndirectLongY },
	{ "clv", A65816M_Implied },  { "lda", A65816M_AbsoluteY },  { "tsx", A65816M_Implied },  { "tyx", A65816M_Implied },
	{ "ldy", A65816M_AbsoluteX },  { "lda", A65816M_AbsoluteX },  { "ldx", A65816M_AbsoluteY },  { "lda", A65816M_LongX },
	{ "cpy", A65816M_ImmediateX },  { "cmp", A65816M_DirectIndirectX },  { "rep", A65816M_Immediate8 },  { "cmp", A65816M_Stack },
	{ "cpy", A65816M_Direct },  { "cmp", A65816M_Direct },  { "dec", A65816M_Direct },  { "cmp", A65816M_DirectIndirectLong },
	{ "iny", A65816M_Implied },  { "cmp", A65816M_ImmediateM },  { "dex", A65816M_Implied },  { "wai", A65816M_Implied },
	{ "cpy", A65816M_Absolute },  { "cmp", A65816M_Absolute },  { "dec", A65816M_Absolute },  { "cmp", A65816M_Long },
	{ "bne", A65816M_Relative },  { "cmp", A65816M_DirectIndirectY },  { "cmp", A65816M_DirectIndirect },  { "cmp", A65816M_StackIndirectY },
	{ "pei", A65816M_Direct },  { "cmp", A65816M_DirectX },  { "dec", A65816M_DirectX },  { "cmp", A65816M_DirectIndirectLongY },
	{ "cld", A65816M_Implied },  { "cmp", A65816M_AbsoluteY },  { "phx", A65816M_Implied },  { "stp", A65816M_Implied },
	{ "jml", A65816M_JumpIndirectLong },  { "cmp", A65816M_AbsoluteX },  { "dec", A65816M_AbsoluteX },  { "cmp", A65816M_LongX },
	{ "cpx", A65816M_ImmediateX },  { "sbc", A65816M_DirectIndirectX },  { "sep", A65816M_Immediate8 },  { "sbc", A65816M_Stack },
	{ "cpx", A65816M_Direct },  { "sbc", A65816M_Direct },  { "inc", A65816M_Direct },  { "sbc", A65816M_DirectIndirectLong },
	{ "inx", A65816M_Implied },  { "sbc", A65816M_ImmediateM },  { "nop", A65816M_Implied },  { "xba", A65816M_Implied },
	{ "cpx", A65816M_Absolute },  { "sbc", A65816M_Absolute },  { "inc", A65816M_Absolute },  { "sbc", A65816M_Long },
	{ "beq", A65816M_Relative },  { "sbc", A65816M_DirectIndirectY },  { "sbc", A65816M_DirectIndirect },  { "sbc", A65816M_StackIndirectY },
	{ "pea", A65816M_Immediate16 },  { "sbc", A65816M_DirectX },  { "inc", A65816M_DirectX },  { "sbc", A65816M_DirectIndirectLongY },
	{ "sed", A65816M_Implied },  { "sbc", A65816M_AbsoluteY },  { "plx", A65816M_Implied },  { "xce", A65816M_Implied },
	{ "jsr", A65816M_JumpIndirectX },  { "sbc", A65816M_AbsoluteX },  { "inc", A65816M_AbsoluteX },  { "sbc", A65816M_LongX },
};


//The mnemonics, and the opcode of each mnemonic in each mode (or -1), made from
//gasAsm65816Opcodes when first needed.
static const char*	gaszAsm65816Mnemonics[ASM65816_MAX_MNEMONICS];
static int16		gaaiAsm65816Opcodes[ASM65816_MAX_MNEMONICS][A65816M_NumModes];
static int			giAsm65816Mnemonics = 0;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void Asm65816BuildIndex(void)
{
	int		i;
	int		j;

	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < giAsm65816Mnemonics; j++)
		{
			if (strcmp(gaszAsm65816Mnemonics[j], gasAsm65816Opcodes[i].szMnemonic) == 0)
			{
				break;
			}
		}
		if (j == giAsm65816Mnemonics)
		{
			gaszAsm65816Mnemonics[j] = gasAsm65816Opcodes[i].szMnemonic;
			memset(gaaiAsm65816Opcodes[j], 0xFF, sizeof(gaaiAsm65816Opcodes[j]));
			giAsm65816Mnemonics++;
		}
		gaaiAsm65816Opcodes[j][gasAsm65816Opcodes[i].uiMode] = (int16)i;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The mnemonic's index, or -1.  Mnemonics are not case sensitive.
//
//////////////////////////////////////////////////////////////////////////
static int Asm65816FindMnemonic(SAsm65Token* psToken)
{
	int		i;

	if (giAsm65816Mnemonics == 0)
	{
		Asm65816BuildIndex();
	}
	for (i = 0; i < giAsm65816Mnemonics; i++)
	{
		if (psToken->IsIdentifierIgnoreCase(gaszAsm65816Mnemonics[i]))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool Asm65816Has(int iMnemonic, uint8 uiMode)
{
	return gaaiAsm65816Opcodes[iMnemonic][uiMode] >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool Asm65816IsJump(int iMnemonic)
{
	return Asm65816Has(iMnemonic, A65816M_JumpAbsolute) || Asm65816Has(iMnemonic, A65816M_JumpLong) || Asm65816Has(iMnemonic, A65816M_JumpIndirect) ||
		   Asm65816Has(iMnemonic, A65816M_JumpIndirectX) || Asm65816Has(iMnemonic, A65816M_JumpIndirectLong);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* Asm65816Mnemonic(uint8 uiOpcode, uint8* puiMode)
{
	*puiMode = gasAsm65816Opcodes[uiOpcode].uiMode;
	return gasAsm65816Opcodes[uiOpcode].szMnemonic;
}


//////////////////////////////////////////////////////////////////////////
//
// mnemonic {operand {, operand}}
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65816Assembler::Instruction(SAsm65Token* psMnemonic)
{
	int				iMnemonic;
	SAsm65816Operand	sOperand;
	SAsm65816Operand	sSecond;
	uint8			uiMode;
	uint8			uiBits;
	bool			bOperand;
	bool			bResult;

	iMnemonic = Asm65816FindMnemonic(psMnemonic);
	if (iMnemonic < 0)
	{
		return Error(psMnemonic, "Unknown instruction");
	}

	memset(&sOperand, 0, sizeof(SAsm65816Operand));
	memset(&sSecond, 0, sizeof(SAsm65816Operand));
	bOperand = !Peek()->IsEndOfStatement();
	if (Asm65816Has(iMnemonic, A65816M_Immediate8) && (psMnemonic->IsIdentifierIgnoreCase("rep") || psMnemonic->IsIdentifierIgnoreCase("sep")))
	{
		sOperand.psToken = Peek();
		if (!bOperand)
		{
			return ErrorExpected(Peek(), "flags or a value");
		}
		if (!StatusBits(&uiBits))
		{
			return false;
		}
		ChangeWidths(psMnemonic->IsIdentifierIgnoreCase("sep"), uiBits);
		sOperand.sValue.iValue = uiBits;
		sOperand.sValue.bResolved = true;
		uiMode = A65816M_Immediate8;
	}
	else
	{
		mbRelative = Asm65816Has(iMnemonic, A65816M_Relative) || Asm65816Has(iMnemonic, A65816M_RelativeLong);
		if (bOperand && !Operand65816(&sOperand))
		{
			return false;
		}
		if (Asm65816Has(iMnemonic, A65816M_BlockMove))
		{
			if (!bOperand || !Expect(AP65_Comma, "','") || !Operand65816(&sSecond))
			{
				return bOperand ? false : ErrorExpected(Peek(), "the source bank");
			}
		}
		if (!ChooseMode(iMnemonic, bOperand ? &sOperand : NULL, psMnemonic, &uiMode))
		{
			return false;
		}
	}

	bResult = Encode(iMnemonic, uiMode, &sOperand, &sSecond, psMnemonic);

	//The widths are not known after the status is pulled or emulation is swapped.
	if (psMnemonic->IsIdentifierIgnoreCase("plp") || psMnemonic->IsIdentifierIgnoreCase("rti") || psMnemonic->IsIdentifierIgnoreCase("xce"))
	{
		muiWidthA = ASM65816_WIDTH_UNKNOWN;
		muiWidthX = ASM65816_WIDTH_UNKNOWN;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The addressing mode an operand is written in, if the instruction has it.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65816Assembler::ChooseMode(int iMnemonic, SAsm65816Operand* psOperand, SAsm65Token* psMnemonic, uint8* puiMode)
{
	uint8			uiMode;
	SAsm65Token*	psToken;

	psToken = psOperand ? psOperand->psToken : Peek();
	if (psOperand == NULL)
	{
		if (!Asm65816Has(iMnemonic, A65816M_Implied))
		{
			return ErrorExpected(psToken, "an operand");
		}
		*puiMode = A65816M_Implied;
		return true;
	}

	if (psOperand->bAccumulator)
	{
		uiMode = A65816M_Accumulator;
	}
	else if (!psOperand->bMemory)
	{
		if (Asm65816Has(iMnemonic, A65816M_ImmediateM))
		{
			uiMode = A65816M_ImmediateM;
		}
		else if (Asm65816Has(iMnemonic, A65816M_ImmediateX))
		{
			uiMode = A65816M_ImmediateX;
		}
		else if (Asm65816Has(iMnemonic, A65816M_Immediate8))
		{
			uiMode = A65816M_Immediate8;
		}
		else if (Asm65816Has(iMnemonic, A65816M_Immediate16))
		{
			uiMode = A65816M_Immediate16;
		}
		else if (Asm65816Has(iMnemonic, A65816M_Relative))
		{
			uiMode = A65816M_Relative;
		}
		else if (Asm65816Has(iMnemonic, A65816M_RelativeLong))
		{
			uiMode = A65816M_RelativeLong;
		}
		else if (Asm65816Has(iMnemonic, A65816M_JumpAbsolute))
		{
			uiMode = A65816M_JumpAbsolute;
		}
		else if (Asm65816Has(iMnemonic, A65816M_JumpLong))
		{
			uiMode = A65816M_JumpLong;
		}
		else if (Asm65816Has(iMnemonic, A65816M_BlockMove))
		{
			uiMode = A65816M_BlockMove;
		}
		else
		{
			return ErrorExpected(psToken, "memory in square brackets");
		}
		if ((psOperand->uiValueBits != 0) && (uiMode != A65816M_ImmediateM) && (uiMode != A65816M_ImmediateX))
		{
			return Error(psToken, "A width suffix is only for an immediate whose width is m or x");
		}
	}
	else if (Asm65816IsJump(iMnemonic))
	{
		//A jump's target is the address; in brackets it is read from memory.
		if ((psOperand->eBase != A65816B_Value) || psOperand->bPointer || (psOperand->cIndex == 'Y') || (psOperand->bLong && (psOperand->cIndex != 0)))
		{
			return Error(psToken, "A jump reads its target from [address], [address + X] or [address]:24");
		}
		uiMode = (psOperand->cIndex == 'X') ? A65816M_JumpIndirectX : (psOperand->bLong ? A65816M_JumpIndirectLong : A65816M_JumpIndirect);
	}
	else if (psOperand->eBase == A65816B_Direct)
	{
		if (psOperand->bLong)
		{
			return Error(psToken, "The direct page is in bank 0: write [[D + value]:24] for a 24 bit pointer");
		}
		if (!psOperand->bPointer)
		{
			uiMode = (psOperand->cIndex == 'X') ? A65816M_DirectX : ((psOperand->cIndex == 'Y') ? A65816M_DirectY : A65816M_Direct);
		}
		else if (psOperand->cInnerIndex == 'X')
		{
			if (psOperand->bLongPointer || (psOperand->cIndex != 0))
			{
				return Error(psToken, "[[D + value + X]] reads a 16 bit pointer and takes no more index");
			}
			uiMode = A65816M_DirectIndirectX;
		}
		else if (psOperand->bLongPointer)
		{
			uiMode = (psOperand->cIndex == 'Y') ? A65816M_DirectIndirectLongY : A65816M_DirectIndirectLong;
		}
		else
		{
			uiMode = (psOperand->cIndex == 'Y') ? A65816M_DirectIndirectY : A65816M_DirectIndirect;
		}
	}
	else if (psOperand->eBase == A65816B_Stack)
	{
		if (!psOperand->bPointer && (psOperand->cIndex == 0) && !psOperand->bLong)
		{
			uiMode = A65816M_Stack;
		}
		else if (psOperand->bPointer && (psOperand->cInnerIndex == 0) && !psOperand->bLongPointer && (psOperand->cIndex == 'Y') && !psOperand->bLong)
		{
			uiMode = A65816M_StackIndirectY;
		}
		else
		{
			return Error(psToken, "The stack is [S + value] or [[S + value] + Y]");
		}
	}
	else if (psOperand->bPointer)
	{
		return Error(psToken, "A pointer is read from the direct page or the stack: [[D + value]] or [[S + value] + Y]");
	}
	else if (psOperand->cIndex == 'Y')
	{
		if (psOperand->bLong)
		{
			return Error(psToken, "There is no long address indexed with Y");
		}
		uiMode = A65816M_AbsoluteY;
	}
	else if (psOperand->cIndex == 'X')
	{
		return AbsoluteOrLong(iMnemonic, psOperand, A65816M_AbsoluteX, A65816M_LongX, puiMode);
	}
	else
	{
		return AbsoluteOrLong(iMnemonic, psOperand, A65816M_Absolute, A65816M_Long, puiMode);
	}

	if (!Asm65816Has(iMnemonic, uiMode))
	{
		return Error(psToken, "The instruction has no such addressing mode");
	}
	*puiMode = uiMode;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// After .relax, whether a branch to a label must reach further than a byte:
// it did in an earlier pass (lengths only grow), it is in another section,
// or it is known to be too far.  A branch to C + offset never is.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65816Assembler::IsLongBranch(SAsm65Value* psTarget)
{
	int64	lliOffset;

	if (!mbRelax || !psTarget->bLabel || (psTarget->iProgramCounter != 0))
	{
		return false;
	}
	if (GetMinimumLength() > 2)
	{
		return true;
	}
	if (!IsLocalTarget(psTarget) && IsRelocatable(psTarget))
	{
		return true;
	}
	if (!psTarget->bResolved)
	{
		return false;
	}
	lliOffset = psTarget->iValue - (int64)(muiStatementAddress + 2);
	return !Asm65Fits(lliOffset, 1, true);
}


//////////////////////////////////////////////////////////////////////////
//
// [value] is absolute (16 bits in the data bank) unless :24 is given or the value
// doesn't fit 16 bits, then long.  An instruction once long stays long in later passes.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65816Assembler::AbsoluteOrLong(int iMnemonic, SAsm65816Operand* psOperand, uint8 uiAbsolute, uint8 uiLong, uint8* puiMode)
{
	bool	bLong;

	bLong = psOperand->bLong;
	if (!bLong && psOperand->sValue.bResolved && !IsRelocatable(&psOperand->sValue) && ((psOperand->sValue.iValue < 0) || (psOperand->sValue.iValue > 0xFFFF)))
	{
		if (!Asm65816Has(iMnemonic, uiLong))
		{
			return Error(psOperand->psToken, "Address doesn't fit 16 bits and the instruction has no long mode");
		}
		bLong = true;
	}
	if (!bLong && Asm65816Has(iMnemonic, uiLong) && (GetMinimumLength() > 3))
	{
		bLong = true;
	}
	if (bLong && !Asm65816Has(iMnemonic, uiLong))
	{
		return Error(psOperand->psToken, "The instruction has no long mode");
	}
	if (!bLong && !Asm65816Has(iMnemonic, uiAbsolute))
	{
		return Error(psOperand->psToken, "The instruction has no such addressing mode");
	}
	*puiMode = bLong ? uiLong : uiAbsolute;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An immediate of the width m or x gives, or its :8 / :16 suffix.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65816Assembler::Immediate(SAsm65816Instruction* psInstruction, SAsm65816Operand* psOperand, uint8 uiWidth, const char* szRegister)
{
	CChars	sz;
	bool	bResult;

	if (psOperand->uiValueBits == 24)
	{
		return Error(psOperand->psToken, "An immediate is :8 or :16");
	}
	if (uiWidth == ASM65816_WIDTH_UNKNOWN)
	{
		if (psOperand->uiValueBits == 0)
		{
			sz.Init("The width of ");
			sz.Append(szRegister);
			sz.Append(" is not known here: use rep, sep, .width or a :8 or :16 suffix");
			bResult = Error(psOperand->psToken, sz.Text());
			sz.Kill();
			return bResult;
		}
		uiWidth = psOperand->uiValueBits;
	}
	else if ((psOperand->uiValueBits != 0) && (psOperand->uiValueBits != uiWidth))
	{
		sz.Init("The suffix is not the width of ");
		sz.Append(szRegister);
		sz.Append(" (");
		sz.Append((int)uiWidth);
		sz.Append(" bits)");
		bResult = Error(psOperand->psToken, sz.Text());
		sz.Kill();
		return bResult;
	}
	return AddField(psInstruction, &psOperand->sValue, uiWidth / 8, false, ASM65RF_Plain, psOperand->psToken);
}


//////////////////////////////////////////////////////////////////////////
//
// A direct page or stack offset: a constant from 0 to 255.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65816Assembler::DirectOffset(SAsm65816Instruction* psInstruction, SAsm65816Operand* psOperand)
{
	if (IsRelocatable(&psOperand->sValue) || (psOperand->sValue.uiForm != ASM65RF_Plain))
	{
		return Error(psOperand->psToken, "A direct page or stack offset must be a constant");
	}
	if (psOperand->sValue.bResolved && ((psOperand->sValue.iValue < 0) || (psOperand->sValue.iValue > 0xFF)))
	{
		return Error(psOperand->psToken, "A direct page or stack offset is 0 to 255");
	}
	AddByte(psInstruction, (uint8)psOperand->sValue.iValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The opcode and its operand.  A jmp, jsr or jmp [Table + X] target is in the program
// bank: assembled absolute it must be in the instruction's bank, in an object the
// linker checks it (in bank).  So is a data address's bank: the data bank is taken to
// be the program bank; write :24 otherwise.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65816Assembler::Encode(int iMnemonic, uint8 uiMode, SAsm65816Operand* psOperand, SAsm65816Operand* psSecond, SAsm65Token* psMnemonic)
{
	SAsm65816Instruction	sInstruction;
	SAsm65Value			sValue;
	bool				bResult;

	memset(&sInstruction, 0, sizeof(SAsm65816Instruction));
	AddByte(&sInstruction, (uint8)gaaiAsm65816Opcodes[iMnemonic][uiMode]);
	sValue = psOperand->sValue;

	switch (uiMode)
	{
	case A65816M_Implied:
	case A65816M_Accumulator:
		bResult = true;
		break;
	case A65816M_ImmediateM:
		bResult = Immediate(&sInstruction, psOperand, muiWidthA, "A");
		break;
	case A65816M_ImmediateX:
		bResult = Immediate(&sInstruction, psOperand, muiWidthX, "X");
		break;
	case A65816M_Immediate8:
		bResult = AddField(&sInstruction, &sValue, 1, false, ASM65RF_Plain, psOperand->psToken);
		break;
	case A65816M_Immediate16:
	case A65816M_JumpIndirect:
	case A65816M_JumpIndirectLong:
		bResult = AddField(&sInstruction, &sValue, 2, false, ASM65RF_Plain, psOperand->psToken);
		break;
	case A65816M_Absolute:
	case A65816M_AbsoluteX:
	case A65816M_AbsoluteY:
	case A65816M_JumpAbsolute:
	case A65816M_JumpIndirectX:
		if (!IsRelocatable(&sValue) && sValue.bResolved && (uiMode != A65816M_Absolute) && (uiMode != A65816M_AbsoluteX) && (uiMode != A65816M_AbsoluteY))
		{
			if ((sValue.iValue >> 16) != (muiStatementAddress >> 16))
			{
				return Error(psOperand->psToken, "Target is in another bank: use jml or jsl");
			}
			sValue.iValue &= 0xFFFF;
		}
		bResult = AddField(&sInstruction, &sValue, 2, false, ASM65RF_InBank, psOperand->psToken);
		break;
	case A65816M_Long:
	case A65816M_LongX:
	case A65816M_JumpLong:
		bResult = AddField(&sInstruction, &sValue, 3, false, ASM65RF_Plain, psOperand->psToken);
		break;
	case A65816M_Direct:
	case A65816M_DirectX:
	case A65816M_DirectY:
	case A65816M_DirectIndirect:
	case A65816M_DirectIndirectX:
	case A65816M_DirectIndirectY:
	case A65816M_DirectIndirectLong:
	case A65816M_DirectIndirectLongY:
	case A65816M_Stack:
	case A65816M_StackIndirectY:
		bResult = DirectOffset(&sInstruction, psOperand);
		break;
	case A65816M_Relative:
		if (IsLongBranch(&sValue))
		{
			//bra is brl; a conditional branch is the opposite branch over a brl (bit 5 of the opcode is the condition).
			if (sInstruction.auiBytes[0] == 0x80)
			{
				sInstruction.auiBytes[0] = 0x82;
				bResult = AddRelative(&sInstruction, &sValue, 2, 3, psOperand->psToken);
			}
			else
			{
				sInstruction.auiBytes[0] ^= 0x20;
				AddByte(&sInstruction, 3);
				AddByte(&sInstruction, 0x82);
				bResult = AddRelative(&sInstruction, &sValue, 2, 5, psOperand->psToken);
			}
		}
		else
		{
			bResult = AddRelative(&sInstruction, &sValue, 1, 2, psOperand->psToken);
		}
		break;
	case A65816M_RelativeLong:
		bResult = AddRelative(&sInstruction, &sValue, 2, 3, psOperand->psToken);
		break;
	case A65816M_BlockMove:
		//The destination bank's byte comes first.
		bResult = AddField(&sInstruction, &psSecond->sValue, 1, false, ASM65RF_Plain, psSecond->psToken) &&
				  AddField(&sInstruction, &sValue, 1, false, ASM65RF_Plain, psOperand->psToken);
		break;
	default:
		bResult = Error(psMnemonic, "Unknown addressing mode");
		break;
	}

	if (!bResult)
	{
		return false;
	}
	Emit65816(&sInstruction);
	return true;
}
