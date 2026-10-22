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
#include <string.h>
#include "65C64Decoded.h"


//The register lists of the opcode encoding sheet, in order.
static E65Register gaeRegisters[8] = { R65_Zero, R65_A, R65_B, R65_S, R65_V, R65_W, R65_X, R65_Y };
static E65Register gaeNoneWXY[4] = { R65_Zero, R65_W, R65_X, R65_Y };
static E65Register gaeVWXY[4] = { R65_V, R65_W, R65_X, R65_Y };
static E65Register gaeABSV[4] = { R65_A, R65_B, R65_S, R65_V };
static E65Register gaeABXY[4] = { R65_A, R65_B, R65_X, R65_Y };
static E65Register gaeSVWY[4] = { R65_S, R65_V, R65_W, R65_Y };
static E65Register gaeXY[2] = { R65_X, R65_Y };

//Immediate widths: unused, 8, 16, 24, 32, 40, 48, 64 bits.
static uint8 gauiImmediateBytes[8] = { 0, 1, 2, 3, 4, 5, 6, 8 };

//Embedded branch offsets.
static int32 gaiBranchOffsets[8] = { 2, 4, 6, 8, -4, -6, -8, -10 };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint16 Field(uint16 uiOpcode, int iLowBit, int iBits)
{
	return (uiOpcode >> iLowBit) & ((1 << iBits) - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int64 Embedded(bool bNegative, uint16 uiField, int iBits)
{
	//Positive values are 1 to 2^bits, negative values are -2^bits to -1.
	if (bNegative)
	{
		return (int64)uiField - (1 << iBits);
	}
	else
	{
		return (int64)uiField + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Is65C64MemoryMode(uint8 uiMode)
{
	return (uiMode >= OPERAND_MODE_ABSOLUTE) && (uiMode <= OPERAND_MODE_REGISTER_INDIRECT_REGISTER);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsTwoRegisterMode(uint8 uiMode)
{
	return (uiMode == OPERAND_MODE_REGISTER_REGISTER) || (uiMode == OPERAND_MODE_REGISTER_REGISTER_INDIRECT) || (uiMode == OPERAND_MODE_REGISTER_INDIRECT_REGISTER);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SetRegister(S65C64Operand* psOperand, E65Register eRegister)
{
	psOperand->eKind = OK65_Register;
	psOperand->eRegister = eRegister;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SetImmediate(S65C64Operand* psOperand, uint8 uiBytes)
{
	psOperand->eKind = OK65_Immediate;
	psOperand->uiImmediateBytes = uiBytes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SetEmbedded(S65C64Operand* psOperand, int64 iValue)
{
	psOperand->eKind = OK65_Embedded;
	psOperand->iEmbedded = iValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SetMemory(S65C64Decoded* psDecoded, S65C64Operand* psOperand, uint8 uiMode, E65Register eFirst, E65Register eSecond)
{
	//eFirst is the addressing register of the single register modes or the first register of the
	//two register modes.  eSecond is the second register of the two register modes.
	psOperand->eKind = OK65_Memory;
	psDecoded->uiMode = uiMode;

	switch (uiMode)
	{
	case OPERAND_MODE_ABSOLUTE:
	case OPERAND_MODE_OFFSET:
		psDecoded->ePre1 = eFirst;
		break;
	case OPERAND_MODE_ABSOLUTE_INDIRECT:
	case OPERAND_MODE_OFFSET_INDIRECT:
		psDecoded->bIndirect = true;
		break;
	case OPERAND_MODE_ABSOLUTE_REGISTER_INDIRECT:
	case OPERAND_MODE_OFFSET_REGISTER_INDIRECT:
		psDecoded->ePre1 = eFirst;
		psDecoded->bIndirect = true;
		break;
	case OPERAND_MODE_ABSOLUTE_INDIRECT_REGISTER:
	case OPERAND_MODE_OFFSET_INDIRECT_REGISTER:
		psDecoded->bIndirect = true;
		psDecoded->ePost = eFirst;
		break;
	case OPERAND_MODE_ADDRESS_REGISTER:
		psDecoded->ePre1 = eFirst;
		break;
	case OPERAND_MODE_REGISTER_REGISTER:
		psDecoded->ePre1 = eFirst;
		psDecoded->ePre2 = eSecond;
		break;
	case OPERAND_MODE_REGISTER_INDIRECT:
		psDecoded->ePre1 = eFirst;
		psDecoded->bIndirect = true;
		break;
	case OPERAND_MODE_REGISTER_REGISTER_INDIRECT:
		psDecoded->ePre1 = eFirst;
		psDecoded->ePre2 = eSecond;
		psDecoded->bIndirect = true;
		break;
	case OPERAND_MODE_REGISTER_INDIRECT_REGISTER:
		psDecoded->ePre1 = eFirst;
		psDecoded->bIndirect = true;
		psDecoded->ePost = eSecond;
		break;
	}

	if (uiMode <= OPERAND_MODE_ABSOLUTE_INDIRECT_REGISTER)
	{
		psDecoded->uiAddressWords = 2;
	}
	else if (uiMode <= OPERAND_MODE_OFFSET_INDIRECT_REGISTER)
	{
		psDecoded->uiAddressWords = 1;
	}
	else
	{
		psDecoded->uiAddressWords = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SetMemoryOperands(S65C64Decoded* psDecoded, bool bSwap, E65Register eRegister, uint8 uiMode, E65Register eFirst, E65Register eSecond)
{
	if (bSwap)
	{
		SetMemory(psDecoded, &psDecoded->sFirst, uiMode, eFirst, eSecond);
		SetRegister(&psDecoded->sSecond, eRegister);
	}
	else
	{
		SetRegister(&psDecoded->sFirst, eRegister);
		SetMemory(psDecoded, &psDecoded->sSecond, uiMode, eFirst, eSecond);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void DecodeLogical(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	uint8	uiMode;

	uiMode = (uint8)Field(uiOpcode, 4, 4);
	psDecoded->uiMode = uiMode;
	if (uiMode == OPERAND_MODE_REGISTER)
	{
		SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 13, 3)]);
		psDecoded->bSigned = Field(uiOpcode, 11, 1) != 0;
		SetRegister(&psDecoded->sSecond, gaeRegisters[Field(uiOpcode, 8, 3)]);
	}
	else if (uiMode == OPERAND_MODE_IMMEDIATE)
	{
		SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 13, 3)]);
		psDecoded->bSigned = Field(uiOpcode, 11, 1) != 0;
		SetImmediate(&psDecoded->sSecond, gauiImmediateBytes[Field(uiOpcode, 8, 3)]);
	}
	else if (uiMode == OPERAND_MODE_EMBEDDED)
	{
		SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 13, 3)]);
		SetEmbedded(&psDecoded->sSecond, Embedded(Field(uiOpcode, 12, 1) != 0, Field(uiOpcode, 8, 4), 4));
	}
	else if (IsTwoRegisterMode(uiMode))
	{
		SetMemoryOperands(psDecoded, Field(uiOpcode, 15, 1) != 0, gaeRegisters[Field(uiOpcode, 12, 3)], uiMode, gaeABSV[Field(uiOpcode, 8, 2)], gaeVWXY[Field(uiOpcode, 10, 2)]);
	}
	else if (Field(uiOpcode, 14, 1) == 0)
	{
		SetMemoryOperands(psDecoded, Field(uiOpcode, 15, 1) != 0, gaeRegisters[Field(uiOpcode, 11, 3)], uiMode, gaeRegisters[Field(uiOpcode, 8, 3)], R65_Zero);
	}
	else
	{
		//The embedded value implies swap: the memory operand is first.
		SetMemory(psDecoded, &psDecoded->sFirst, uiMode, gaeRegisters[Field(uiOpcode, 8, 3)], R65_Zero);
		SetEmbedded(&psDecoded->sSecond, Embedded(Field(uiOpcode, 15, 1) != 0, Field(uiOpcode, 11, 3), 3));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void DecodeArithmetic(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	uint8	uiMode;

	psDecoded->bSigned = Field(uiOpcode, 4, 1) != 0;
	uiMode = (uint8)Field(uiOpcode, 5, 4);
	psDecoded->uiMode = uiMode;
	if (uiMode == OPERAND_MODE_REGISTER)
	{
		SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 13, 3)]);
		SetRegister(&psDecoded->sSecond, gaeRegisters[Field(uiOpcode, 9, 3)]);
	}
	else if (uiMode == OPERAND_MODE_IMMEDIATE)
	{
		SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 13, 3)]);
		SetImmediate(&psDecoded->sSecond, gauiImmediateBytes[Field(uiOpcode, 9, 3)]);
	}
	else if (uiMode == OPERAND_MODE_EMBEDDED)
	{
		SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 13, 3)]);
		SetEmbedded(&psDecoded->sSecond, Embedded(Field(uiOpcode, 12, 1) != 0, Field(uiOpcode, 9, 3), 3));
	}
	else if (IsTwoRegisterMode(uiMode))
	{
		SetMemoryOperands(psDecoded, Field(uiOpcode, 15, 1) != 0, gaeRegisters[Field(uiOpcode, 12, 3)], uiMode, gaeABSV[Field(uiOpcode, 9, 2)], gaeXY[Field(uiOpcode, 11, 1)]);
	}
	else if (Field(uiOpcode, 14, 1) == 0)
	{
		SetMemoryOperands(psDecoded, Field(uiOpcode, 15, 1) != 0, gaeRegisters[Field(uiOpcode, 11, 3)], uiMode, gaeNoneWXY[Field(uiOpcode, 9, 2)], R65_Zero);
	}
	else
	{
		SetMemory(psDecoded, &psDecoded->sFirst, uiMode, gaeNoneWXY[Field(uiOpcode, 9, 2)], R65_Zero);
		SetEmbedded(&psDecoded->sSecond, Embedded(Field(uiOpcode, 15, 1) != 0, Field(uiOpcode, 11, 3), 3));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void DecodeShift(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	uint8	uiMode;

	uiMode = (uint8)Field(uiOpcode, 6, 4);
	psDecoded->uiMode = uiMode;
	if (uiMode == OPERAND_MODE_REGISTER)
	{
		SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 13, 3)]);
		SetRegister(&psDecoded->sSecond, gaeRegisters[Field(uiOpcode, 10, 3)]);
	}
	else if (uiMode == OPERAND_MODE_IMMEDIATE)
	{
		SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 13, 3)]);
		SetImmediate(&psDecoded->sSecond, Field(uiOpcode, 10, 1) ? 2 : 1);
	}
	else if (uiMode == OPERAND_MODE_EMBEDDED)
	{
		//1 to 4 and 5 to 8: the high bit is not a sign.
		SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 13, 3)]);
		SetEmbedded(&psDecoded->sSecond, Field(uiOpcode, 10, 3) + 1);
	}
	else if (IsTwoRegisterMode(uiMode))
	{
		SetMemoryOperands(psDecoded, Field(uiOpcode, 15, 1) != 0, gaeABXY[Field(uiOpcode, 13, 2)], uiMode, gaeABSV[Field(uiOpcode, 10, 2)], gaeXY[Field(uiOpcode, 12, 1)]);
	}
	else if (Field(uiOpcode, 14, 1) == 0)
	{
		SetMemoryOperands(psDecoded, Field(uiOpcode, 15, 1) != 0, gaeABSV[Field(uiOpcode, 12, 2)], uiMode, gaeNoneWXY[Field(uiOpcode, 10, 2)], R65_Zero);
	}
	else
	{
		SetMemory(psDecoded, &psDecoded->sFirst, uiMode, gaeNoneWXY[Field(uiOpcode, 10, 2)], R65_Zero);
		SetEmbedded(&psDecoded->sSecond, ((Field(uiOpcode, 15, 1) << 2) | Field(uiOpcode, 12, 2)) + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SetAluAccess(S65C64Decoded* psDecoded)
{
	if (psDecoded->sFirst.eKind == OK65_Memory)
	{
		psDecoded->bMemoryData = true;
		switch (psDecoded->eInstruction)
		{
		case I65_Cpy:
			psDecoded->eAccess = MA65_Write;
			break;
		case I65_Tst:
		case I65_Cmp:
			psDecoded->eAccess = MA65_Read;
			break;
		default:
			psDecoded->eAccess = MA65_ReadWrite;
			break;
		}
	}
	else if (psDecoded->sSecond.eKind == OK65_Memory)
	{
		psDecoded->bMemoryData = true;
		psDecoded->eAccess = MA65_Read;
	}
	psDecoded->eDataAccess = psDecoded->eAccess;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void DecodeJump(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	uint8	uiMode;
	uint16	uiWidth;

	psDecoded->eStack = gaeSVWY[Field(uiOpcode, 14, 2)];
	psDecoded->bPushP = Field(uiOpcode, 6, 1) != 0;
	psDecoded->bPushC = Field(uiOpcode, 5, 1) != 0;
	psDecoded->bRelative = Field(uiOpcode, 4, 1) != 0;
	psDecoded->bSigned = psDecoded->bRelative;

	uiMode = (uint8)Field(uiOpcode, 7, 4);
	psDecoded->uiMode = uiMode;
	if (uiMode == OPERAND_MODE_REGISTER)
	{
		SetRegister(&psDecoded->sSecond, gaeRegisters[Field(uiOpcode, 11, 3)]);
	}
	else if (uiMode == OPERAND_MODE_IMMEDIATE)
	{
		//No jump, 8, 16, 24, 32 bits.
		uiWidth = Field(uiOpcode, 11, 3);
		if (uiWidth > 4)
		{
			psDecoded->eInstruction = I65_Illegal;
			return;
		}
		if (uiWidth != 0)
		{
			SetImmediate(&psDecoded->sSecond, (uint8)uiWidth);
		}
	}
	else if (uiMode == OPERAND_MODE_EMBEDDED)
	{
		psDecoded->eInstruction = I65_Illegal;
		return;
	}
	else if (IsTwoRegisterMode(uiMode))
	{
		SetMemory(psDecoded, &psDecoded->sSecond, uiMode, gaeABSV[Field(uiOpcode, 11, 2)], gaeXY[Field(uiOpcode, 13, 1)]);
	}
	else
	{
		SetMemory(psDecoded, &psDecoded->sSecond, uiMode, gaeRegisters[Field(uiOpcode, 11, 3)], R65_Zero);
	}

	if (psDecoded->sSecond.eKind == OK65_None)
	{
		psDecoded->eInstruction = I65_Psh;
	}
	else if (psDecoded->bRelative)
	{
		psDecoded->eInstruction = psDecoded->bPushC ? I65_Brsr : I65_Bra;
	}
	else
	{
		psDecoded->eInstruction = psDecoded->bPushC ? I65_Jsr : I65_Jmp;
	}

	if (psDecoded->bPushC || psDecoded->bPushP)
	{
		psDecoded->eAccess = MA65_Write;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void DecodeReturnStateBreakStop(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	uint16	uiSub;

	switch (Field(uiOpcode, 4, 2))
	{
	case 0:
		psDecoded->eStack = gaeSVWY[Field(uiOpcode, 14, 2)];
		psDecoded->bPullP = Field(uiOpcode, 9, 1) != 0;
		psDecoded->bPullC = Field(uiOpcode, 8, 1) != 0;
		if (Field(uiOpcode, 7, 1))
		{
			psDecoded->eInstruction = I65_Rti;
		}
		else
		{
			psDecoded->eInstruction = psDecoded->bPullC ? I65_Ret : I65_Pul;
		}
		if (psDecoded->bPullC || psDecoded->bPullP)
		{
			psDecoded->eAccess = MA65_Read;
		}
		break;

	case 1:
		uiSub = Field(uiOpcode, 6, 2);
		if (uiSub <= 1)
		{
			psDecoded->eInstruction = (uiSub == 0) ? I65_Sep : I65_Cep;
			psDecoded->uiMask = (uint8)Field(uiOpcode, 8, 8);
		}
		else
		{
			psDecoded->eInstruction = (uiSub == 2) ? I65_Wth : I65_Wtth;
			psDecoded->eWidthFirst = gaeABXY[Field(uiOpcode, 14, 2)];
			psDecoded->eWidthSecond = gaeABXY[Field(uiOpcode, 12, 2)];
			if (Field(uiOpcode, 11, 1))
			{
				psDecoded->uiWidthBytes = (uint8)(Field(uiOpcode, 8, 3) + 1);
				SetEmbedded(&psDecoded->sSecond, psDecoded->uiWidthBytes);
			}
			else
			{
				SetRegister(&psDecoded->sSecond, gaeRegisters[Field(uiOpcode, 8, 3)]);
			}
		}
		break;

	case 2:
		psDecoded->eInstruction = I65_Brk;
		psDecoded->uiVector = Field(uiOpcode, 6, 10);
		psDecoded->eStack = R65_S;
		psDecoded->eAccess = MA65_Write;
		break;

	case 3:
		uiSub = Field(uiOpcode, 6, 2);
		if (uiSub == 0)
		{
			psDecoded->eInstruction = I65_Stp;
			psDecoded->uiMask = (uint8)Field(uiOpcode, 8, 5);
		}
		else if (uiSub == 2)
		{
			psDecoded->eInstruction = I65_Gwth;
			psDecoded->eWidthFirst = gaeABXY[Field(uiOpcode, 14, 2)];
			psDecoded->bDecrement = Field(uiOpcode, 13, 1) != 0;
			SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 10, 3)]);
		}
		else if (uiSub == 3)
		{
			psDecoded->eInstruction = I65_Wthx;
			psDecoded->eWidthFirst = gaeABXY[Field(uiOpcode, 14, 2)];
			psDecoded->bSigned = Field(uiOpcode, 11, 1) != 0;
			psDecoded->uiWidthBytes = (uint8)(Field(uiOpcode, 8, 3) + 1);
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void DecodeBranchNopShift(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	static E65Instruction	aeBranches[4] = { I65_Brac, I65_Bras, I65_Brasa, I65_Braso };
	uint16					uiExtend;

	uiExtend = Field(uiOpcode, 4, 2);
	if ((uiExtend == 0) || ((uiExtend == 1) && (Field(uiOpcode, 8, 1) == 0)))
	{
		psDecoded->uiFlagFirst = (uint8)Field(uiOpcode, 13, 3);
		psDecoded->uiFlagSecond = (uint8)Field(uiOpcode, 11, 2);
		psDecoded->uiCondition = (uint8)Field(uiOpcode, 6, 2);
		psDecoded->eInstruction = aeBranches[psDecoded->uiCondition];
		psDecoded->bSigned = true;
		if (uiExtend == 0)
		{
			psDecoded->iBranchOffset = gaiBranchOffsets[Field(uiOpcode, 8, 3)];
			SetEmbedded(&psDecoded->sSecond, psDecoded->iBranchOffset);
		}
		else
		{
			SetImmediate(&psDecoded->sSecond, (uint8)(Field(uiOpcode, 9, 2) + 1));
		}
	}
	else if (uiExtend == 1)
	{
		if (Field(uiOpcode, 6, 1) == 0)
		{
			psDecoded->eInstruction = I65_Bal;
			psDecoded->iBranchOffset = (int32)Field(uiOpcode, 9, 7);
			if (psDecoded->iBranchOffset >= 64)
			{
				psDecoded->iBranchOffset -= 128;
			}
			SetEmbedded(&psDecoded->sSecond, psDecoded->iBranchOffset);
		}
		else
		{
			psDecoded->eInstruction = I65_Nop;
			psDecoded->uiWait = (uint8)(Field(uiOpcode, 9, 3) + 1);
		}
	}
	else
	{
		psDecoded->eInstruction = (uiExtend == 2) ? I65_Shl : I65_Shr;
		DecodeShift(uiOpcode, psDecoded);
		SetAluAccess(psDecoded);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void DecodePushPullOperand(uint16 uiOpcode, S65C64Decoded* psDecoded, bool bPush)
{
	uint8	uiMode;

	//Push an immediate or the 32 bit value at an address, pull into an address.
	uiMode = (uint8)Field(uiOpcode, 6, 4);
	if (bPush && (uiMode == OPERAND_MODE_IMMEDIATE))
	{
		psDecoded->eInstruction = I65_PshOperand;
		SetImmediate(&psDecoded->sSecond, gauiImmediateBytes[Field(uiOpcode, 10, 3)]);
		psDecoded->eAccess = MA65_Write;
	}
	else if (Is65C64MemoryMode(uiMode) && !IsTwoRegisterMode(uiMode))
	{
		psDecoded->eInstruction = bPush ? I65_PshOperand : I65_PulOperand;
		SetMemory(psDecoded, &psDecoded->sSecond, uiMode, gaeRegisters[Field(uiOpcode, 10, 3)], R65_Zero);
		psDecoded->eAccess = MA65_ReadWrite;
		if (bPush)
		{
			psDecoded->bMemoryData = true;
			psDecoded->eDataAccess = MA65_Read;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void DecodePushPull(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	psDecoded->eStack = gaeRegisters[Field(uiOpcode, 13, 3)];
	if (psDecoded->eStack == R65_Zero)
	{
		return;
	}

	switch (Field(uiOpcode, 4, 2))
	{
	case 0:
		psDecoded->eInstruction = I65_PshRegisters;
		psDecoded->uiMask = (uint8)Field(uiOpcode, 6, 7);
		psDecoded->eAccess = MA65_Write;
		break;
	case 2:
		psDecoded->eInstruction = I65_PulRegisters;
		psDecoded->uiMask = (uint8)Field(uiOpcode, 6, 7);
		psDecoded->eAccess = MA65_Read;
		break;
	case 1:
	case 3:
		DecodePushPullOperand(uiOpcode, psDecoded, Field(uiOpcode, 4, 2) == 1);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Subcopy copies a count of bytes (or bits) from an index in the source register to an
// index in the destination register.  The count is a register or embedded; the
// indices are in the operand word.  An embedded bit count's top two bits are in the
// operand word too.
//
//////////////////////////////////////////////////////////////////////////
static void DecodeSubcopy(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	bool	bBits;

	bBits = Field(uiOpcode, 4, 1) != 0;
	psDecoded->eInstruction = bBits ? I65_Scb : I65_Scp;
	psDecoded->eSource = gaeRegisters[Field(uiOpcode, 13, 3)];
	psDecoded->eDestination = gaeRegisters[Field(uiOpcode, 10, 3)];
	if (Field(uiOpcode, 9, 1))
	{
		if (bBits)
		{
			psDecoded->uiCount = (uint8)(Field(uiOpcode, 5, 4) + 1);
		}
		else
		{
			psDecoded->uiCount = (uint8)(Field(uiOpcode, 6, 3) + 1);
		}
	}
	else
	{
		psDecoded->eCount = gaeRegisters[Field(uiOpcode, 6, 3)];
	}
	psDecoded->uiOperandWords = 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Multiply and divide use A, B, W and Y.  They have no operand fields.
//
//////////////////////////////////////////////////////////////////////////
static void DecodeMultiplyDivideFindBlockVector(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	static uint8	auiUnitBits[4] = { 1, 2, 4, 8 };
	uint16			uiUnit;

	switch (Field(uiOpcode, 4, 2))
	{
	case 0:
		switch (Field(uiOpcode, 6, 2))
		{
		case 0:
			psDecoded->eInstruction = I65_Mul;
			break;
		case 1:
			psDecoded->eInstruction = I65_Div;
			break;
		default:
			//Negate and absolute: the first register gets the second.
			psDecoded->eInstruction = (Field(uiOpcode, 6, 2) == 2) ? I65_Neg : I65_Abs;
			SetRegister(&psDecoded->sFirst, gaeRegisters[Field(uiOpcode, 13, 3)]);
			SetRegister(&psDecoded->sSecond, gaeRegisters[Field(uiOpcode, 8, 3)]);
			break;
		}
		break;

	case 1:
		//Find: set (not zero) before clear (zero), for bits, crumbs, nybbles and bytes.
		uiUnit = Field(uiOpcode, 6, 2);
		psDecoded->eSource = gaeRegisters[Field(uiOpcode, 13, 3)];
		psDecoded->eCount = gaeRegisters[Field(uiOpcode, 10, 3)];
		psDecoded->bDecrement = Field(uiOpcode, 9, 1) != 0;
		psDecoded->bNotZero = Field(uiOpcode, 8, 1) != 0;
		psDecoded->uiUnitBits = auiUnitBits[uiUnit];
		psDecoded->eInstruction = (E65Instruction)(I65_Fsbi + 2 * uiUnit + (psDecoded->bNotZero ? 0 : 1));
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Instruction code 8: block operations (b4 = 0) on [V] with R, and vector operations
// (b4 = 1).  The sub instruction is b8-5.  A counts so R cannot be A.
//
//////////////////////////////////////////////////////////////////////////
static void DecodeBlockVector(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	//Sub instructions 7, 9 and 11 (the signed add, subtract and compare) were dropped.
	static E65Instruction	aeVector[16] = { I65_Vcpy, I65_Vnot, I65_Vand, I65_Vtst, I65_Vor, I65_Vxor, I65_Vadd, I65_Illegal,
											 I65_Vsub, I65_Illegal, I65_Vcmp, I65_Illegal, I65_Vpcpy, I65_Vpand, I65_Vpor, I65_Vpxor };
	static uint8			auiWidthBytes[4] = { 1, 2, 4, 8 };
	uint16					uiSub;

	psDecoded->bDecrement = Field(uiOpcode, 15, 1) != 0;
	psDecoded->bAllowStop = Field(uiOpcode, 14, 1) != 0;
	psDecoded->uiStopStyle = (uint8)Field(uiOpcode, 12, 2);
	uiSub = Field(uiOpcode, 5, 4);

	if (Field(uiOpcode, 4, 1) != 0)
	{
		//Vector operations: [V] op [W] -> [V].  Test and compare do not write.
		psDecoded->bVector = true;
		psDecoded->bDecrementW = Field(uiOpcode, 11, 1) != 0;
		psDecoded->uiWidthBytes = auiWidthBytes[Field(uiOpcode, 9, 2)];
		psDecoded->eInstruction = aeVector[uiSub];
		if ((psDecoded->eInstruction == I65_Vtst) || (psDecoded->eInstruction == I65_Vcmp))
		{
			psDecoded->eAccess = MA65_Read;
		}
		else
		{
			psDecoded->eAccess = MA65_ReadWrite;
		}
		return;
	}

	psDecoded->eSource = gaeRegisters[Field(uiOpcode, 9, 3)];
	if ((psDecoded->eSource == R65_Zero) || (psDecoded->eSource == R65_A))
	{
		return;
	}

	switch (Field(uiOpcode, 5, 4))
	{
	case 0:
		psDecoded->eInstruction = I65_Bcpy;
		psDecoded->eAccess = MA65_Write;
		break;
	case 2:
		psDecoded->eInstruction = I65_Band;
		psDecoded->eAccess = MA65_ReadWrite;
		break;
	case 12:
		psDecoded->eInstruction = I65_Bpcpy;
		psDecoded->eAccess = MA65_ReadWrite;
		break;
	case 13:
		psDecoded->eInstruction = I65_Bpand;
		psDecoded->eAccess = MA65_ReadWrite;
		break;
	case 14:
		psDecoded->eInstruction = I65_Bpor;
		psDecoded->eAccess = MA65_ReadWrite;
		break;
	case 15:
		psDecoded->eInstruction = I65_Bpxor;
		psDecoded->eAccess = MA65_ReadWrite;
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Hardware register instructions (extend 0 to 3) are not simulated.  Status flags are
// numbered as in the branches: 0, 1, c, z, n, v, f, g.
//
//////////////////////////////////////////////////////////////////////////
static void DecodeHardwareState(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	switch (Field(uiOpcode, 4, 3))
	{
	case 4:
	case 5:
		psDecoded->eInstruction = (Field(uiOpcode, 4, 3) == 4) ? I65_Bist : I65_Stab;
		psDecoded->uiFlagFirst = (uint8)Field(uiOpcode, 13, 3);
		if (psDecoded->eInstruction == I65_Bist)
		{
			psDecoded->eSource = gaeRegisters[Field(uiOpcode, 10, 3)];
			if (psDecoded->uiFlagFirst < 2)
			{
				psDecoded->eInstruction = I65_Illegal;
			}
		}
		else
		{
			psDecoded->eDestination = gaeRegisters[Field(uiOpcode, 10, 3)];
		}

		//Index style 7 is a register, 6 is immediate (both in the operand word), 0 to 5 is the index.
		psDecoded->uiIndexStyle = (uint8)Field(uiOpcode, 7, 3);
		psDecoded->uiOperandWords = (psDecoded->uiIndexStyle >= 6) ? 1 : 0;
		break;

	case 7:
		psDecoded->eInstruction = I65_Stat;
		psDecoded->uiFlagFirst = (uint8)Field(uiOpcode, 13, 3);
		psDecoded->uiFlagSecond = (uint8)Field(uiOpcode, 10, 3);
		if (psDecoded->uiFlagFirst < 2)
		{
			psDecoded->eInstruction = I65_Illegal;
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Decode65C64(uint16 uiOpcode, S65C64Decoded* psDecoded)
{
	static E65Instruction	aeAlu[8] = { I65_Cpy, I65_And, I65_Or, I65_Xor, I65_Tst, I65_Add, I65_Sub, I65_Cmp };
	uint16					uiCode;
	uint8					uiImmediateBytes;

	memset(psDecoded, 0, sizeof(S65C64Decoded));
	psDecoded->uiOpcode = uiOpcode;
	psDecoded->eInstruction = I65_Illegal;

	uiCode = Field(uiOpcode, 0, 4);
	switch (uiCode)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		psDecoded->eInstruction = aeAlu[uiCode];
		DecodeLogical(uiOpcode, psDecoded);
		SetAluAccess(psDecoded);
		break;
	case 5:
	case 6:
	case 7:
		psDecoded->eInstruction = aeAlu[uiCode];
		DecodeArithmetic(uiOpcode, psDecoded);
		SetAluAccess(psDecoded);
		break;
	case 8:
		DecodeBlockVector(uiOpcode, psDecoded);
		break;
	case 9:
		DecodeSubcopy(uiOpcode, psDecoded);
		break;
	case 10:
		DecodeJump(uiOpcode, psDecoded);
		break;
	case 11:
		DecodeReturnStateBreakStop(uiOpcode, psDecoded);
		break;
	case 12:
		DecodeBranchNopShift(uiOpcode, psDecoded);
		break;
	case 13:
		DecodePushPull(uiOpcode, psDecoded);
		break;
	case 14:
		DecodeMultiplyDivideFindBlockVector(uiOpcode, psDecoded);
		break;
	case 15:
		DecodeHardwareState(uiOpcode, psDecoded);
		break;
	}

	if (psDecoded->sSecond.eKind == OK65_Immediate)
	{
		uiImmediateBytes = psDecoded->sSecond.uiImmediateBytes;
		if (uiImmediateBytes == 0)
		{
			psDecoded->eInstruction = I65_Illegal;
		}
		psDecoded->uiOperandWords = (uiImmediateBytes + 1) / 2;
	}
	else if (psDecoded->uiOperandWords == 0)
	{
		psDecoded->uiOperandWords = psDecoded->uiAddressWords;
	}

	if (psDecoded->eInstruction == I65_Illegal)
	{
		memset(psDecoded, 0, sizeof(S65C64Decoded));
		psDecoded->uiOpcode = uiOpcode;
		psDecoded->eInstruction = I65_Illegal;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* Get65C64RegisterName(E65Register eRegister)
{
	static char* aszNames[8] = { "0", "A", "B", "S", "V", "W", "X", "Y" };

	return aszNames[eRegister & 7];
}

