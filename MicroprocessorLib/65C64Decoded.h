/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __65C64_DECODED_H__
#define __65C64_DECODED_H__
#include "BaseLib/PrimitiveTypes.h"


//Register codes in the 3 bit register fields.
enum E65Register
{
	R65_Zero,
	R65_A,
	R65_B,
	R65_S,
	R65_V,
	R65_W,
	R65_X,
	R65_Y,
};


enum E65Instruction
{
	I65_Illegal,
	I65_Cpy,
	I65_And,
	I65_Or,
	I65_Xor,
	I65_Tst,
	I65_Add,
	I65_Sub,
	I65_Cmp,
	I65_Jmp,
	I65_Jsr,
	I65_Bra,
	I65_Brsr,
	I65_Psh,
	I65_Ret,
	I65_Pul,
	I65_Rti,
	I65_Sep,
	I65_Cep,
	I65_Wth,
	I65_Wtth,
	I65_Brk,
	I65_Stp,
	I65_Gwth,
	I65_Wthx,
	I65_Bras,
	I65_Brac,
	I65_Brasa,
	I65_Braso,
	I65_Bal,
	I65_Nop,
	I65_Shl,
	I65_Shr,
	I65_PshRegisters,
	I65_PulRegisters,
	I65_Mul,
	I65_Div,
	I65_Scp,
	I65_Scb,
	I65_Neg,
	I65_Abs,
	I65_Fsbi,
	I65_Fcbi,
	I65_Fscr,
	I65_Fccr,
	I65_Fsny,
	I65_Fcny,
	I65_Fsby,
	I65_Fcby,
	I65_Bist,
	I65_Stab,
	I65_Stat,
	I65_PshOperand,
	I65_PulOperand,
	I65_Band,
	I65_Btst,
	I65_Bor,
	I65_Bxor,
	I65_Badd,
	I65_Bsub,
	I65_Bcmp,
	I65_Bcpy,
	I65_Bpcpy,
	I65_Bpand,
	I65_Bpor,
	I65_Bpxor,
	I65_Vcpy,
	I65_Vnot,
	I65_Vand,
	I65_Vtst,
	I65_Vor,
	I65_Vxor,
	I65_Vadd,
	I65_Vsub,
	I65_Vcmp,
	I65_Vpcpy,
	I65_Vpand,
	I65_Vpor,
	I65_Vpxor,
	I65_Reset,
	I65_FetchNext,
	I65_Interrupt,
	I65_Wait,

	NUM_65_INSTRUCTIONS
};


enum E65OperandKind
{
	OK65_None,
	OK65_Register,
	OK65_Immediate,
	OK65_Embedded,
	OK65_Memory,
};


//What the instruction does to memory after the address is known.
enum E65Access
{
	MA65_None,
	MA65_Read,
	MA65_Write,
	MA65_ReadWrite,
};


//Operand access modes (4 bits).
#define OPERAND_MODE_REGISTER					0
#define OPERAND_MODE_IMMEDIATE					1
#define OPERAND_MODE_ABSOLUTE					2
#define OPERAND_MODE_ABSOLUTE_INDIRECT			3
#define OPERAND_MODE_ABSOLUTE_REGISTER_INDIRECT	4
#define OPERAND_MODE_ABSOLUTE_INDIRECT_REGISTER	5
#define OPERAND_MODE_OFFSET						6
#define OPERAND_MODE_OFFSET_INDIRECT			7
#define OPERAND_MODE_OFFSET_REGISTER_INDIRECT	8
#define OPERAND_MODE_OFFSET_INDIRECT_REGISTER	9
#define OPERAND_MODE_ADDRESS_REGISTER			10
#define OPERAND_MODE_REGISTER_REGISTER			11
#define OPERAND_MODE_REGISTER_INDIRECT			12
#define OPERAND_MODE_REGISTER_REGISTER_INDIRECT	13
#define OPERAND_MODE_REGISTER_INDIRECT_REGISTER	14
#define OPERAND_MODE_EMBEDDED					15


struct S65C64Operand
{
	E65OperandKind	eKind;
	E65Register		eRegister;
	int64			iEmbedded;
	uint8			uiImmediateBytes;
};


//An opcode split into its fields.  The fields an instruction does not use are zero.
struct S65C64Decoded
{
	uint16			uiOpcode;
	E65Instruction	eInstruction;
	uint8			uiMode;
	bool			bSigned;
	S65C64Operand	sFirst;
	S65C64Operand	sSecond;

	//The memory operand's address: [words + Pre1 + Pre2] + Post when indirect, else words + Pre1 + Pre2.
	uint8			uiAddressWords;
	E65Register		ePre1;
	E65Register		ePre2;
	bool			bIndirect;
	E65Register		ePost;
	bool			bMemoryData;
	E65Access		eDataAccess;
	E65Access		eAccess;
	uint8			uiOperandWords;

	//Flow, stack and state.
	E65Register		eStack;
	bool			bPushP;
	bool			bPushC;
	bool			bRelative;
	bool			bPullP;
	bool			bPullC;
	uint8			uiMask;
	uint8			uiFlagFirst;
	uint8			uiFlagSecond;
	uint8			uiCondition;
	int32			iBranchOffset;
	uint8			uiWait;
	uint16			uiVector;
	E65Register		eWidthFirst;
	E65Register		eWidthSecond;
	uint8			uiWidthBytes;
	bool			bDecrement;

	//Subcopy.  The indices are in the operand word.
	E65Register		eSource;
	E65Register		eDestination;
	E65Register		eCount;
	uint8			uiCount;

	//Find (eSource is scanned, eCount is the index), bit to status and status to bit.
	bool			bNotZero;
	uint8			uiUnitBits;
	uint8			uiIndexStyle;

	//Block and vector operations (eSource is R, bDecrement is V's direction, bDecrementW is
	//W's, uiWidthBytes is a vector's element width).
	bool			bAllowStop;
	uint8			uiStopStyle;
	bool			bVector;
	bool			bDecrementW;
};


void	Decode65C64(uint16 uiOpcode, S65C64Decoded* psDecoded);
bool	Is65C64MemoryMode(uint8 uiMode);
char*	Get65C64RegisterName(E65Register eRegister);


#endif // __65C64_DECODED_H__

