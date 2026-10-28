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
#include "Asm65Assembler.h"


//Immediate widths in bytes, from the decoder's table without the unused code 0.  The
//ALU and push take all 7, jumps and branches the first 4 (8 to 32 bits), shifts the
//first 2 (8 and 16 bits).
static const uint8* const gauiWidthBytes = &gaui65ImmediateBytes[1];


enum EAsm65Kind
{
	AK65_ArithmeticLogic,
	AK65_Shift,
	AK65_Jump,
	AK65_Return,
	AK65_Push,
	AK65_Pull,
	AK65_StateBits,
	AK65_Width,
	AK65_Break,
	AK65_Stop,
	AK65_GetWidth,
	AK65_ExtendWidth,
	AK65_Branch,
	AK65_BranchEqual,
	AK65_NoOperation,
	AK65_CopyExtend,
	AK65_MultiplyDivide,
	AK65_NegateAbsolute,
	AK65_Find,
	AK65_Subcopy,
	AK65_BitToStatus,
	AK65_StatusToBit,
	AK65_StatusToStatus,
	AK65_Block,
	AK65_Vector,
	AK65_Hardware,
};


struct SAsm65Mnemonic
{
	const char*	szName;
	EAsm65Kind	eKind;
	uint16		uiParameter;
};


//Jump parameters: bit 0 push C, bit 1 relative.  Find parameters: bit 2 set, bits 1-0 the unit.
static const SAsm65Mnemonic gasMnemonics[] =
{
	{ "cpy",	AK65_ArithmeticLogic,	0 },
	{ "and",	AK65_ArithmeticLogic,	1 },
	{ "or",		AK65_ArithmeticLogic,	2 },
	{ "xor",	AK65_ArithmeticLogic,	3 },
	{ "tst",	AK65_ArithmeticLogic,	4 },
	{ "add",	AK65_ArithmeticLogic,	5 },
	{ "sub",	AK65_ArithmeticLogic,	6 },
	{ "cmp",	AK65_ArithmeticLogic,	7 },
	{ "shl",	AK65_Shift,				1 },
	{ "shr",	AK65_Shift,				0 },
	{ "jmp",	AK65_Jump,				0 },
	{ "jsr",	AK65_Jump,				1 },
	{ "bra",	AK65_Jump,				2 },
	{ "brsr",	AK65_Jump,				3 },
	{ "ret",	AK65_Return,			0 },
	{ "rti",	AK65_Return,			1 },
	{ "psh",	AK65_Push,				0 },
	{ "pul",	AK65_Pull,				0 },
	{ "sep",	AK65_StateBits,			0 },
	{ "cep",	AK65_StateBits,			1 },
	{ "wth",	AK65_Width,				0 },
	{ "wtth",	AK65_Width,				1 },
	{ "brk",	AK65_Break,				0 },
	{ "stp",	AK65_Stop,				0 },
	{ "gwth",	AK65_GetWidth,			0 },
	{ "wthx",	AK65_ExtendWidth,		0 },
	{ "brac",	AK65_Branch,			0 },
	{ "bras",	AK65_Branch,			1 },
	{ "brasa",	AK65_Branch,			2 },
	{ "braso",	AK65_Branch,			3 },
	{ "brae",	AK65_BranchEqual,		0 },
	{ "brane",	AK65_BranchEqual,		1 },
	{ "nop",	AK65_NoOperation,		0 },
	{ "cpyx",	AK65_CopyExtend,		0 },
	{ "mul",	AK65_MultiplyDivide,	0 },
	{ "div",	AK65_MultiplyDivide,	1 },
	{ "neg",	AK65_NegateAbsolute,	2 },
	{ "abs",	AK65_NegateAbsolute,	3 },
	{ "fsbi",	AK65_Find,				4 },
	{ "fcbi",	AK65_Find,				0 },
	{ "fscr",	AK65_Find,				5 },
	{ "fccr",	AK65_Find,				1 },
	{ "fsny",	AK65_Find,				6 },
	{ "fcny",	AK65_Find,				2 },
	{ "fsby",	AK65_Find,				7 },
	{ "fcby",	AK65_Find,				3 },
	{ "scp",	AK65_Subcopy,			0 },
	{ "scb",	AK65_Subcopy,			1 },
	{ "bist",	AK65_BitToStatus,		0 },
	{ "stab",	AK65_StatusToBit,		0 },
	{ "stat",	AK65_StatusToStatus,	0 },
	{ "bcpy",	AK65_Block,				0 },
	{ "band",	AK65_Block,				2 },
	{ "btst",	AK65_Block,				3 },
	{ "bor",	AK65_Block,				4 },
	{ "bxor",	AK65_Block,				5 },
	{ "badd",	AK65_Block,				6 },
	{ "bsub",	AK65_Block,				8 },
	{ "bcmp",	AK65_Block,				10 },
	{ "bpcpy",	AK65_Block,				12 },
	{ "bpand",	AK65_Block,				13 },
	{ "bpor",	AK65_Block,				14 },
	{ "bpxor",	AK65_Block,				15 },
	{ "vcpy",	AK65_Vector,			0 },
	{ "vnot",	AK65_Vector,			1 },
	{ "vand",	AK65_Vector,			2 },
	{ "vtst",	AK65_Vector,			3 },
	{ "vor",	AK65_Vector,			4 },
	{ "vxor",	AK65_Vector,			5 },
	{ "vadd",	AK65_Vector,			6 },
	{ "vsub",	AK65_Vector,			8 },
	{ "vcmp",	AK65_Vector,			10 },
	{ "vpcpy",	AK65_Vector,			12 },
	{ "vpand",	AK65_Vector,			13 },
	{ "vpor",	AK65_Vector,			14 },
	{ "vpxor",	AK65_Vector,			15 },
	{ "hws",	AK65_Hardware,			0 },
	{ "hwg",	AK65_Hardware,			1 },
	{ "hwsp",	AK65_Hardware,			2 },
	{ "hwcp",	AK65_Hardware,			3 },
};


//Where the logical (codes 0 to 4) and arithmetic (5 to 7) instructions put their fields.
//The field is the second register, the immediate width, the embedded value or the
//addressing register.  Logical memory forms have no sign bit.
struct SAsm65AluLayout
{
	int					iFieldShift;
	int					iModeShift;
	int					iSignedShift;
	int					iSecondRegisterShift;
	int					iEmbeddedBits;
	const E65Register*	aeAddress;
	int					iAddressSize;
	const E65Register*	aeSecond;
	int					iSecondSize;
};

static const SAsm65AluLayout gsLogicalLayout = { 8, 4, 11, 10, 4, gae65Registers, 8, gae65VWXY, 4 };
static const SAsm65AluLayout gsArithmeticLayout = { 9, 5, 4, 11, 3, gae65NoneVXY, 4, gae65XY, 2 };


//////////////////////////////////////////////////////////////////////////
//
// uiTop holds the fields above the layout's field (the first register, swap and sign).
//
//////////////////////////////////////////////////////////////////////////
static uint16 AluOpcode(const SAsm65AluLayout* psLayout, uint16 uiCode, uint16 uiTop, uint16 uiField, uint16 uiMode, bool bSigned)
{
	return (uint16)(uiTop | (uiField << psLayout->iFieldShift) | (uiMode << psLayout->iModeShift) | (bSigned << psLayout->iSignedShift) | uiCode);
}


//////////////////////////////////////////////////////////////////////////
//
// The width code of the 3 bit immediate width fields: unused, 8, 16, 24, 32, 40, 48, 64.
//
//////////////////////////////////////////////////////////////////////////
static uint16 ImmediateCode(uint8 uiBytes)
{
	return (uiBytes == 8) ? 7 : uiBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// Embedded fields: positive values are 1 to 2^bits (field + 1), negative values are
// -2^bits to -1 (field - 2^bits).
//
//////////////////////////////////////////////////////////////////////////
static bool EmbeddedField(int64 iValue, int iBits, uint16* puiSign, uint16* puiField)
{
	int64	iLimit;

	iLimit = (int64)1 << iBits;
	if ((iValue >= 1) && (iValue <= iLimit))
	{
		*puiSign = 0;
		*puiField = (uint16)(iValue - 1);
		return true;
	}
	if ((iValue < 0) && (iValue >= -iLimit))
	{
		*puiSign = 1;
		*puiField = (uint16)(iValue + iLimit);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Relative offsets are from the next instruction (C).  A value that uses a label is
// the target address; C + offset and a raw value are the offset itself.
//
//////////////////////////////////////////////////////////////////////////
static int64 RelativeOffset(SAsm65Value* psValue, uint32 uiAddress, uint8 uiWords)
{
	if ((psValue->iProgramCounter != 0) || !psValue->bLabel)
	{
		return psValue->iValue;
	}
	return psValue->iValue - (int64)(uiAddress + 2 * uiWords);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void StartInstruction(SAsm65Instruction* psInstruction, uint16 uiOpcode)
{
	psInstruction->auiWords[0] = uiOpcode;
	psInstruction->uiWords = 1;
	psInstruction->uiRelocations = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Instruction(SAsm65Token* psMnemonic)
{
	size					i;
	const SAsm65Mnemonic*	psEntry;

	for (i = 0; i < sizeof(gasMnemonics) / sizeof(SAsm65Mnemonic); i++)
	{
		psEntry = &gasMnemonics[i];
		if (!psMnemonic->IsIdentifierIgnoreCase(psEntry->szName))
		{
			continue;
		}

		switch (psEntry->eKind)
		{
		case AK65_ArithmeticLogic:
			return ArithmeticLogic(psEntry->uiParameter);
		case AK65_Shift:
			return Shift(psEntry->uiParameter != 0);
		case AK65_Jump:
			return Jump((psEntry->uiParameter & 1) != 0, (psEntry->uiParameter & 2) != 0);
		case AK65_Return:
			return Return(psEntry->uiParameter != 0);
		case AK65_Push:
			return Push();
		case AK65_Pull:
			return Pull();
		case AK65_StateBits:
			return StateBits(psEntry->uiParameter);
		case AK65_Width:
			return Width(psEntry->uiParameter != 0);
		case AK65_Break:
			return Break();
		case AK65_Stop:
			return Stop();
		case AK65_GetWidth:
			return GetWidth();
		case AK65_ExtendWidth:
			return ExtendWidth();
		case AK65_Branch:
			return Branch(psEntry->uiParameter);
		case AK65_BranchEqual:
			return BranchEqual(psEntry->uiParameter);
		case AK65_NoOperation:
			return NoOperation();
		case AK65_CopyExtend:
			return CopyExtend();
		case AK65_MultiplyDivide:
			return MultiplyDivide(psEntry->uiParameter);
		case AK65_NegateAbsolute:
			return NegateAbsolute(psEntry->uiParameter);
		case AK65_Find:
			return Find((psEntry->uiParameter & 4) != 0, psEntry->uiParameter & 3);
		case AK65_Subcopy:
			return Subcopy(psEntry->uiParameter != 0);
		case AK65_BitToStatus:
			return BitToStatus();
		case AK65_StatusToBit:
			return StatusToBit();
		case AK65_StatusToStatus:
			return StatusToStatus();
		case AK65_Block:
			return Block(psEntry->uiParameter);
		case AK65_Vector:
			return Vector(psEntry->uiParameter);
		case AK65_Hardware:
			return Error(psMnemonic, "Hardware register instructions are not supported");
		}
	}
	return Error(psMnemonic, "Unknown instruction");
}


//////////////////////////////////////////////////////////////////////////
//
// Picks the smallest width (from auiBytes, smallest first) that holds the value and
// keeps the instruction at least as long as it was in the last pass.  uiFixedWords is
// the instruction's length without the value.  A section address is 32 bits unless
// its width is given (the linker checks it fits).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ChooseBytes(SAsm65Value* psValue, bool bSigned, uint8 uiExplicitBytes, const uint8* auiBytes, int iNumBytes, uint8 uiFixedWords, SAsm65Token* psToken, uint8* puiBytes)
{
	int		i;
	uint8	uiMinimumWords;
	uint8	uiBytes;
	bool	bRelocatable;

	*puiBytes = 0;
	bRelocatable = IsRelocatable(psValue);
	if (uiExplicitBytes != 0)
	{
		for (i = 0; i < iNumBytes; i++)
		{
			if (auiBytes[i] == uiExplicitBytes)
			{
				break;
			}
		}
		if (i == iNumBytes)
		{
			return Error(psToken, "That width is not allowed here");
		}
		if (!bRelocatable && psValue->bResolved && !Asm65Fits(psValue->iValue, uiExplicitBytes, bSigned))
		{
			return Error(psToken, "Value does not fit");
		}
		*puiBytes = uiExplicitBytes;
		return true;
	}

	uiMinimumWords = GetMinimumWords();
	for (i = 0; i < iNumBytes; i++)
	{
		uiBytes = auiBytes[i];
		if (uiFixedWords + (uiBytes + 1) / 2 < uiMinimumWords)
		{
			continue;
		}
		if (bRelocatable)
		{
			if (uiBytes >= 4)
			{
				*puiBytes = uiBytes;
				return true;
			}
		}
		else if (!psValue->bResolved || Asm65Fits(psValue->iValue, uiBytes, bSigned))
		{
			*puiBytes = uiBytes;
			return true;
		}
	}
	*puiBytes = auiBytes[iNumBytes - 1];
	if (bRelocatable)
	{
		return Error(psToken, "A section address cannot be used here");
	}
	return Error(psToken, "Value does not fit");
}


//////////////////////////////////////////////////////////////////////////
//
// The narrowest signed immediate (8 to 32 bits) for a relative target that keeps the
// instruction at least as long as in the last pass.  The offset depends on the width
// when the target is a label.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::RelativeBytes(SAsm65Value* psTarget, SAsm65Token* psToken, uint8* puiBytes, int64* piOffset)
{
	int		i;
	uint8	uiWords;

	*puiBytes = 0;
	*piOffset = 0;
	if (IsRelocatable(psTarget))
	{
		*puiBytes = gauiWidthBytes[3];
		return true;
	}
	for (i = 0; i < 4; i++)
	{
		uiWords = 1 + (gauiWidthBytes[i] + 1) / 2;
		*piOffset = RelativeOffset(psTarget, muiStatementAddress, uiWords);
		if ((uiWords >= GetMinimumWords()) && (!psTarget->bResolved || Asm65Fits(*piOffset, gauiWidthBytes[i], true)))
		{
			*puiBytes = gauiWidthBytes[i];
			return true;
		}
	}
	return Error(psToken, "Target is out of range");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::AddValueWords(SAsm65Instruction* psInstruction, uint64 uiValue, uint8 uiBytes)
{
	uint8	i;

	if (uiBytes < 8)
	{
		uiValue &= ((uint64)1 << (8 * uiBytes)) - 1;
	}
	for (i = 0; i < (uiBytes + 1) / 2; i++)
	{
		psInstruction->auiWords[psInstruction->uiWords] = (uint16)(uiValue >> (16 * i));
		psInstruction->uiWords++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::AddAddressWords(SAsm65Instruction* psInstruction, SAsm65Memory* psMemory)
{
	if (psMemory->uiAddressWords != 0)
	{
		AddRelocatableWords(psInstruction, psMemory->psValue, psMemory->uiAddress, 2 * psMemory->uiAddressWords, false, false, psMemory->psToken);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The value's words, or zeros and a relocation when the value is a section address.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::AddRelocatableWords(SAsm65Instruction* psInstruction, SAsm65Value* psValue, uint64 uiValue, uint8 uiBytes, bool bSigned, bool bRelative, SAsm65Token* psToken)
{
	SAsm65InstructionRelocation*	psRelocation;

	if (!IsRelocatable(psValue))
	{
		AddValueWords(psInstruction, uiValue, uiBytes);
		return true;
	}
	if (psInstruction->uiRelocations == ASM65_MAX_INSTRUCTION_RELOCATIONS)
	{
		return Error(psToken, "Too many section addresses in one instruction");
	}

	UseTerm();
	psRelocation = &psInstruction->asRelocations[psInstruction->uiRelocations];
	psInstruction->uiRelocations++;
	psRelocation->sValue = *psValue;
	psRelocation->uiWord = psInstruction->uiWords;
	psRelocation->uiBytes = uiBytes;
	psRelocation->bSigned = bSigned;
	psRelocation->bRelative = bRelative;
	psRelocation->psToken = psToken;
	AddValueWords(psInstruction, 0, uiBytes);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Classifies a memory operand and picks offset (16 bit) or absolute (32 bit) for the
// modes with an address: offset when the address is below 0001:0000.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::MemoryOperand(SAsm65Operand* psOperand, SAsm65Memory* psMemory, uint8 uiFixedWords)
{
	SAsm65Address*	psAddress;
	bool			bOffset;
	bool			bFits;
	bool			bRelocatable;

	psAddress = &psOperand->sAddress;
	if (!Memory(psAddress, psMemory))
	{
		return false;
	}
	if (psMemory->uiAddressWords == 0)
	{
		return true;
	}

	//Addresses that are too wide lose their high bits with a warning (the user, 2026-10-23).
	//A section address is absolute unless :16 says it is an offset (the user, 2026-10-23).
	bRelocatable = IsRelocatable(&psAddress->sValue);
	bFits = !bRelocatable && (!psAddress->sValue.bResolved || ((psAddress->sValue.iValue >= 0) && (psAddress->sValue.iValue <= 0xFFFF)));
	if (psAddress->uiAddressBits == 16)
	{
		if (!bFits && !bRelocatable)
		{
			Warning(psAddress->psToken, "Offset address is wider than 16 bits; the high bits are discarded");
		}
		bOffset = true;
	}
	else if (psAddress->uiAddressBits == 32)
	{
		bOffset = false;
	}
	else
	{
		bOffset = bFits && (uiFixedWords + 1 >= GetMinimumWords());
	}

	if (!bOffset && !bRelocatable && psAddress->sValue.bResolved && ((psAddress->sValue.iValue < 0) || (psAddress->sValue.iValue > 0xFFFFFFFFLL)))
	{
		Warning(psAddress->psToken, "Address is wider than 32 bits; the high bits are discarded");
	}

	if (bOffset)
	{
		psMemory->uiMode += OPERAND_MODE_OFFSET - OPERAND_MODE_ABSOLUTE;
		psMemory->uiAddressWords = 1;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::SingleRegisterField(SAsm65Memory* psMemory, const E65Register* aeList, int iListSize, SAsm65Token* psToken, uint16* puiField)
{
	int		iIndex;
	CChars	szMessage;
	bool	bResult;

	*puiField = 0;
	iIndex = Get65C64RegisterIndex(psMemory->eFirst, aeList, iListSize);
	if (iIndex >= 0)
	{
		*puiField = (uint16)iIndex;
		return true;
	}

	szMessage.Init();
	if (psMemory->eFirst == R65_Zero)
	{
		szMessage.Append("This address needs a register");
	}
	else
	{
		szMessage.Append("Register ");
		szMessage.Append(Get65C64RegisterName(psMemory->eFirst));
		szMessage.Append(" cannot address memory here");
	}
	bResult = Error(psToken, szMessage.Text());
	szMessage.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The first register must be A, B, S or V and the second from aeSecondList.  R1 + R2
// and [R1 + R2] may be written either way round.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::TwoRegisterFields(SAsm65Memory* psMemory, const E65Register* aeSecondList, int iSecondListSize, SAsm65Token* psToken, uint16* puiFirst, uint16* puiSecond)
{
	int		iFirst;
	int		iSecond;

	*puiFirst = 0;
	*puiSecond = 0;
	iFirst = Get65C64RegisterIndex(psMemory->eFirst, gae65ABSV, 4);
	iSecond = Get65C64RegisterIndex(psMemory->eSecond, aeSecondList, iSecondListSize);
	if (((iFirst < 0) || (iSecond < 0)) && (psMemory->uiMode != OPERAND_MODE_REGISTER_INDIRECT_REGISTER))
	{
		iFirst = Get65C64RegisterIndex(psMemory->eSecond, gae65ABSV, 4);
		iSecond = Get65C64RegisterIndex(psMemory->eFirst, aeSecondList, iSecondListSize);
	}
	if ((iFirst < 0) || (iSecond < 0))
	{
		return Error(psToken, "Those two registers cannot address memory here");
	}
	*puiFirst = (uint16)iFirst;
	*puiSecond = (uint16)iSecond;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// cpy, and, or, xor, tst (logical, codes 0 to 4) and add, sub, cmp (arithmetic, 5 to 7).
//   op R1, R2{:s}          register
//   op R, value{:width}{:s}  0 (the zero register), embedded or immediate
//   op R, [address]{:s}    memory into the register
//   op [address], R{:s}    the register into memory (swap)
//   op [address], value    embedded (-8 to 8)
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ArithmeticLogic(uint16 uiCode)
{
	SAsm65Operand			sFirst;
	SAsm65Operand			sSecond;
	SAsm65Instruction		sInstruction;
	const SAsm65AluLayout*	psLayout;
	bool					bSigned;
	uint16					uiFirst;
	uint16					uiSign;
	uint16					uiField;
	uint8					uiBytes;
	int64					iValue;
	bool					bRelocatable;

	if (!Operand(&sFirst) || !Expect(AP65_Comma, "','") || !Operand(&sSecond))
	{
		return false;
	}

	psLayout = (uiCode >= 5) ? &gsArithmeticLayout : &gsLogicalLayout;
	uiFirst = (uint16)(sFirst.eRegister << 13);
	if ((sFirst.eKind == AO65_Memory) || (sSecond.eKind == AO65_Memory))
	{
		if (!ArithmeticLogicMemory(uiCode, &sFirst, &sSecond, &sInstruction))
		{
			return false;
		}
	}
	else if (sFirst.eKind != AO65_Register)
	{
		return Error(sFirst.psToken, "The first operand must be a register or memory");
	}
	else if (sSecond.eKind == AO65_Register)
	{
		StartInstruction(&sInstruction, AluOpcode(psLayout, uiCode, uiFirst, sSecond.eRegister, OPERAND_MODE_REGISTER, sSecond.bSigned));
	}
	else
	{
		iValue = sSecond.sValue.iValue;
		bRelocatable = IsRelocatable(&sSecond.sValue);
		bSigned = sSecond.bSigned || (!bRelocatable && (iValue < 0));
		if (!bRelocatable && (sSecond.uiWidthBytes == 0) && (iValue == 0) && (GetMinimumWords() <= 1))
		{
			//The zero register.
			StartInstruction(&sInstruction, AluOpcode(psLayout, uiCode, uiFirst, R65_Zero, OPERAND_MODE_REGISTER, false));
		}
		else if (!bRelocatable && (sSecond.uiWidthBytes == 0) && (GetMinimumWords() <= 1) && EmbeddedField(iValue, psLayout->iEmbeddedBits, &uiSign, &uiField))
		{
			StartInstruction(&sInstruction, AluOpcode(psLayout, uiCode, (uint16)(uiFirst | (uiSign << 12)), uiField, OPERAND_MODE_EMBEDDED, false));
		}
		else
		{
			if (!ChooseBytes(&sSecond.sValue, bSigned, sSecond.uiWidthBytes, gauiWidthBytes, 7, 1, sSecond.psToken, &uiBytes))
			{
				return false;
			}
			StartInstruction(&sInstruction, AluOpcode(psLayout, uiCode, uiFirst, ImmediateCode(uiBytes), OPERAND_MODE_IMMEDIATE, bSigned));
			if (!AddRelocatableWords(&sInstruction, &sSecond.sValue, (uint64)iValue, uiBytes, bSigned, false, sSecond.psToken))
			{
				return false;
			}
		}
	}

	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// One operand is memory.  Logical instructions address with any register and have no
// sign bit; arithmetic instructions address with V, X or Y and have a sign bit.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ArithmeticLogicMemory(uint16 uiCode, SAsm65Operand* psFirst, SAsm65Operand* psSecond, SAsm65Instruction* psInstruction)
{
	SAsm65Operand*			psMemoryOperand;
	SAsm65Memory			sMemory;
	const SAsm65AluLayout*	psLayout;
	bool					bArithmetic;
	bool					bSwap;
	bool					bEmbedded;
	bool					bSigned;
	E65Register				eRegister;
	uint16					uiSign;
	uint16					uiField;
	uint16					uiFirst;
	uint16					uiSecond;
	uint16					uiOpcode;

	bArithmetic = uiCode >= 5;
	psLayout = bArithmetic ? &gsArithmeticLayout : &gsLogicalLayout;
	bSwap = psFirst->eKind == AO65_Memory;
	bEmbedded = false;
	uiSign = 0;
	uiField = 0;
	eRegister = R65_Zero;
	if (bSwap)
	{
		psMemoryOperand = psFirst;
		if (psSecond->eKind == AO65_Register)
		{
			eRegister = psSecond->eRegister;
		}
		else if (psSecond->eKind == AO65_Constant)
		{
			if (psSecond->uiWidthBytes != 0)
			{
				return Error(psSecond->psToken, "A value stored to memory has no width");
			}
			if (psSecond->sValue.iValue != 0)
			{
				if (!EmbeddedField(psSecond->sValue.iValue, 3, &uiSign, &uiField))
				{
					return Error(psSecond->psToken, "A value used with memory must be -8 to 8; use a register");
				}
				bEmbedded = true;
			}
		}
		else
		{
			return Error(psSecond->psToken, "Only one operand can be memory");
		}
		bSigned = psSecond->bSigned || psFirst->bSigned;
	}
	else
	{
		psMemoryOperand = psSecond;
		eRegister = psFirst->eRegister;
		bSigned = psSecond->bSigned;
	}

	if (bSigned && !bArithmetic)
	{
		return Error(psSecond->psToken, "Logical instructions with memory cannot sign extend");
	}
	if (!MemoryOperand(psMemoryOperand, &sMemory, 1))
	{
		return false;
	}

	if (Is65C64TwoRegisterMode(sMemory.uiMode))
	{
		if (bEmbedded)
		{
			return Error(psSecond->psToken, "A two register address cannot take a value");
		}
		if (!TwoRegisterFields(&sMemory, psLayout->aeSecond, psLayout->iSecondSize, psMemoryOperand->psToken, &uiFirst, &uiSecond))
		{
			return false;
		}
		uiOpcode = AluOpcode(psLayout, uiCode, (uint16)((bSwap << 15) | (eRegister << 12) | (uiSecond << psLayout->iSecondRegisterShift)), uiFirst, sMemory.uiMode, bSigned);
	}
	else
	{
		if (!SingleRegisterField(&sMemory, psLayout->aeAddress, psLayout->iAddressSize, psMemoryOperand->psToken, &uiFirst))
		{
			return false;
		}
		if (bEmbedded)
		{
			uiOpcode = AluOpcode(psLayout, uiCode, (uint16)((uiSign << 15) | (1 << 14) | (uiField << 11)), uiFirst, sMemory.uiMode, false);
		}
		else
		{
			uiOpcode = AluOpcode(psLayout, uiCode, (uint16)((bSwap << 15) | (eRegister << 11)), uiFirst, sMemory.uiMode, bSigned);
		}
	}

	StartInstruction(psInstruction, uiOpcode);
	AddAddressWords(psInstruction, &sMemory);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// shl and shr (instruction code 14, extend 2 and 3).
//   shl R1, R2
//   shl R, amount          0, 1 to 8 embedded, else an 8 or 16 bit immediate
//   shl R, [address]       shl [address], R       shl [address], 1 to 8
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Shift(bool bLeft)
{
	SAsm65Operand		sFirst;
	SAsm65Operand		sSecond;
	SAsm65Instruction	sInstruction;
	uint16				uiExtend;
	uint8				uiBytes;
	int64				iValue;

	if (!Operand(&sFirst) || !Expect(AP65_Comma, "','") || !Operand(&sSecond))
	{
		return false;
	}
	if (sFirst.bSigned || sSecond.bSigned)
	{
		return Error(sFirst.psToken, "Shifts cannot sign extend");
	}

	uiExtend = bLeft ? 2 : 3;
	if ((sFirst.eKind == AO65_Memory) || (sSecond.eKind == AO65_Memory))
	{
		if (!ShiftMemory(uiExtend, &sFirst, &sSecond, &sInstruction))
		{
			return false;
		}
	}
	else if (sFirst.eKind != AO65_Register)
	{
		return Error(sFirst.psToken, "The first operand must be a register or memory");
	}
	else if (sSecond.eKind == AO65_Register)
	{
		StartInstruction(&sInstruction, (uint16)((sFirst.eRegister << 13) | (sSecond.eRegister << 10) | (OPERAND_MODE_REGISTER << 6) | (uiExtend << 4) | 14));
	}
	else
	{
		iValue = sSecond.sValue.iValue;
		if (iValue < 0)
		{
			return Error(sSecond.psToken, "A shift amount cannot be negative");
		}
		if ((sSecond.uiWidthBytes == 0) && (iValue == 0) && (GetMinimumWords() <= 1))
		{
			StartInstruction(&sInstruction, (uint16)((sFirst.eRegister << 13) | (R65_Zero << 10) | (OPERAND_MODE_REGISTER << 6) | (uiExtend << 4) | 14));
		}
		else if ((sSecond.uiWidthBytes == 0) && (iValue >= 1) && (iValue <= 8) && (GetMinimumWords() <= 1))
		{
			StartInstruction(&sInstruction, (uint16)((sFirst.eRegister << 13) | ((iValue - 1) << 10) | (OPERAND_MODE_EMBEDDED << 6) | (uiExtend << 4) | 14));
		}
		else
		{
			if (!ChooseBytes(&sSecond.sValue, false, sSecond.uiWidthBytes, gauiWidthBytes, 2, 1, sSecond.psToken, &uiBytes))
			{
				return false;
			}
			StartInstruction(&sInstruction, (uint16)((sFirst.eRegister << 13) | ((uiBytes == 2) << 10) | (OPERAND_MODE_IMMEDIATE << 6) | (uiExtend << 4) | 14));
			AddValueWords(&sInstruction, (uint64)iValue, uiBytes);
		}
	}

	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The shift's register is A, B, S or V with a one register address and A, B, X or Y
// with a two register address.  Addressing registers are V, X or Y.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ShiftMemory(uint16 uiExtend, SAsm65Operand* psFirst, SAsm65Operand* psSecond, SAsm65Instruction* psInstruction)
{
	SAsm65Operand*	psMemoryOperand;
	SAsm65Operand*	psOther;
	SAsm65Memory	sMemory;
	bool			bSwap;
	int				iRegister;
	uint16			uiFirst;
	uint16			uiSecond;
	uint16			uiOpcode;
	int64			iValue;

	bSwap = psFirst->eKind == AO65_Memory;
	psMemoryOperand = bSwap ? psFirst : psSecond;
	psOther = bSwap ? psSecond : psFirst;
	if (psOther->eKind == AO65_Memory)
	{
		return Error(psOther->psToken, "Only one operand can be memory");
	}
	if (!bSwap && (psOther->eKind != AO65_Register))
	{
		return Error(psOther->psToken, "The first operand must be a register or memory");
	}
	if (!MemoryOperand(psMemoryOperand, &sMemory, 1))
	{
		return false;
	}

	if (Is65C64TwoRegisterMode(sMemory.uiMode))
	{
		if (psOther->eKind != AO65_Register)
		{
			return Error(psOther->psToken, "A two register address cannot take a value");
		}
		iRegister = Get65C64RegisterIndex(psOther->eRegister, gae65ABXY, 4);
		if (iRegister < 0)
		{
			return Error(psOther->psToken, "The register must be A, B, X or Y");
		}
		if (!TwoRegisterFields(&sMemory, gae65XY, 2, psMemoryOperand->psToken, &uiFirst, &uiSecond))
		{
			return false;
		}
		uiOpcode = (uint16)((bSwap << 15) | (iRegister << 13) | (uiSecond << 12) | (uiFirst << 10) | (sMemory.uiMode << 6) | (uiExtend << 4) | 14);
	}
	else
	{
		if (!SingleRegisterField(&sMemory, gae65NoneVXY, 4, psMemoryOperand->psToken, &uiFirst))
		{
			return false;
		}
		if (psOther->eKind == AO65_Register)
		{
			iRegister = Get65C64RegisterIndex(psOther->eRegister, gae65ABSV, 4);
			if (iRegister < 0)
			{
				return Error(psOther->psToken, "The register must be A, B, S or V");
			}
			uiOpcode = (uint16)((bSwap << 15) | (iRegister << 12) | (uiFirst << 10) | (sMemory.uiMode << 6) | (uiExtend << 4) | 14);
		}
		else
		{
			iValue = psOther->sValue.iValue;
			if (psOther->uiWidthBytes != 0)
			{
				return Error(psOther->psToken, "A shift of memory has no width");
			}
			if (psOther->sValue.bResolved && ((iValue < 1) || (iValue > 8)))
			{
				return Error(psOther->psToken, "A shift of memory must be 1 to 8");
			}
			if (!psOther->sValue.bResolved)
			{
				iValue = 1;
			}
			uiOpcode = (uint16)(((((iValue - 1) >> 2) & 1) << 15) | (1 << 14) | (((iValue - 1) & 3) << 12) | (uiFirst << 10) | (sMemory.uiMode << 6) | (uiExtend << 4) | 14);
		}
	}

	StartInstruction(psInstruction, uiOpcode);
	AddAddressWords(psInstruction, &sMemory);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// {, P} {, stack} in any order.  The stack is S, V, W or Y.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::JumpOptions(E65Register* peStack, bool* pbPushP)
{
	SAsm65Token*	psToken;
	E65Register		eRegister;

	*peStack = R65_S;
	*pbPushP = false;
	while (Accept(AP65_Comma))
	{
		psToken = Peek();
		if (psToken->IsIdentifier("P"))
		{
			Next();
			*pbPushP = true;
		}
		else if (IsRegister(psToken, &eRegister) && (Get65C64RegisterIndex(eRegister, gae65SVWY, 4) >= 0))
		{
			Next();
			*peStack = eRegister;
		}
		else
		{
			return Error(psToken, "Expected P or a stack register (S, V, W or Y)");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// jmp, jsr, bra and brsr (instruction code 10).  The operand is the target (the
// effective address), not memory:
//   jmp R                  jump to the register's value
//   jmp label              immediate (relative to the next instruction for bra, brsr;
//                          their C + offset or value without a label is the offset)
//   jmp label + X          jmp [pointer]          jmp V + X  ...  (memory modes)
// Options: {, P} pushes P after C, {, S|V|W|Y} is the stack.  bra to a label within
// -64 to 63 bytes is the one word branch always.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Jump(bool bPushC, bool bRelative)
{
	SAsm65Address		sAddress;
	SAsm65Operand		sOperand;
	SAsm65Memory		sMemory;
	SAsm65Instruction	sInstruction;
	E65Register			eStack;
	bool				bPushP;
	uint16				uiStack;
	uint16				uiMode;
	uint16				uiField;
	uint16				uiFirst;
	uint16				uiSecond;
	uint8				uiBytes;
	int64				iTarget;
	int64				iOffset;
	bool				bRelocatable;

	mbRelative = bRelative;
	if (!Address(&sAddress))
	{
		return false;
	}
	mbRelative = false;
	if (!JumpOptions(&eStack, &bPushP))
	{
		return false;
	}
	if ((sAddress.sValue.iProgramCounter != 0) && (sAddress.bIndirect || (sAddress.eBase != R65_Zero) || (sAddress.eInner != R65_Zero) || (sAddress.uiAddressBits != 0)))
	{
		return Error(sAddress.psToken, "C is only allowed in a target, not an address");
	}
	if (!RelativeTarget(&sAddress.sValue, sAddress.psToken))
	{
		return false;
	}
	uiStack = (uint16)Get65C64RegisterIndex(eStack, gae65SVWY, 4);
	iTarget = sAddress.sValue.iValue;

	if (!sAddress.bIndirect && (sAddress.eBase != R65_Zero) && (sAddress.eInner == R65_Zero))
	{
		StartInstruction(&sInstruction, (uint16)((uiStack << 14) | (sAddress.eBase << 11) | (OPERAND_MODE_REGISTER << 7) | (bPushP << 6) | (bPushC << 5) | (bRelative << 4) | 10));
	}
	else if (!sAddress.bIndirect && (sAddress.eBase == R65_Zero) && (sAddress.eInner == R65_Zero) && (sAddress.uiAddressBits == 0))
	{
		if (!bRelative)
		{
			if (!ChooseBytes(&sAddress.sValue, false, 0, gauiWidthBytes, 4, 1, sAddress.psToken, &uiBytes))
			{
				return false;
			}
			StartInstruction(&sInstruction, (uint16)((uiStack << 14) | (uiBytes << 11) | (OPERAND_MODE_IMMEDIATE << 7) | (bPushP << 6) | (bPushC << 5) | (0 << 4) | 10));
			if (!AddRelocatableWords(&sInstruction, &sAddress.sValue, (uint64)iTarget, uiBytes, false, false, sAddress.psToken))
			{
				return false;
			}
		}
		else
		{
			bRelocatable = TargetIsRelocatable(&sAddress.sValue);
			iOffset = RelativeOffset(&sAddress.sValue, muiStatementAddress, 1);
			if (!bRelocatable && !bPushC && !bPushP && (eStack == R65_S) && (GetMinimumWords() <= 1) && (!sAddress.sValue.bResolved || ((iOffset >= -64) && (iOffset <= 63))))
			{
				//Branch always.
				StartInstruction(&sInstruction, (uint16)(((iOffset & 0x7F) << 9) | (1 << 8) | (0 << 6) | (1 << 4) | 12));
			}
			else
			{
				if (!RelativeBytes(&sAddress.sValue, sAddress.psToken, &uiBytes, &iOffset))
				{
					return false;
				}
				StartInstruction(&sInstruction, (uint16)((uiStack << 14) | (uiBytes << 11) | (OPERAND_MODE_IMMEDIATE << 7) | (bPushP << 6) | (bPushC << 5) | (1 << 4) | 10));
				if (!AddRelocatableWords(&sInstruction, &sAddress.sValue, (uint64)iOffset, uiBytes, true, true, sAddress.psToken))
				{
					return false;
				}
			}
		}
	}
	else
	{
		sOperand.eKind = AO65_Memory;
		sOperand.sAddress = sAddress;
		sOperand.psToken = sAddress.psToken;
		if (!MemoryOperand(&sOperand, &sMemory, 1))
		{
			return false;
		}
		uiMode = sMemory.uiMode;
		if (Is65C64TwoRegisterMode(sMemory.uiMode))
		{
			if (!TwoRegisterFields(&sMemory, gae65XY, 2, sAddress.psToken, &uiFirst, &uiSecond))
			{
				return false;
			}
			uiField = (uint16)((uiSecond << 2) | uiFirst);
		}
		else if (!SingleRegisterField(&sMemory, gae65Registers, 8, sAddress.psToken, &uiField))
		{
			return false;
		}
		StartInstruction(&sInstruction, (uint16)((uiStack << 14) | (uiField << 11) | (uiMode << 7) | (bPushP << 6) | (bPushC << 5) | (bRelative << 4) | 10));
		AddAddressWords(&sInstruction, &sMemory);
	}

	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes C and/or P without jumping (instruction code 10 with no immediate) or pulls P
// (instruction code 11).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ProgramState(bool bPush, bool bC, bool bP, E65Register eStack)
{
	SAsm65Instruction	sInstruction;
	int					iStack;

	iStack = Get65C64RegisterIndex(eStack, gae65SVWY, 4);
	if (iStack < 0)
	{
		return Error(Peek(), "The stack for C and P must be S, V, W or Y");
	}
	if (bPush)
	{
		StartInstruction(&sInstruction, (uint16)((iStack << 14) | (0 << 11) | (OPERAND_MODE_IMMEDIATE << 7) | (bP << 6) | (bC << 5) | (0 << 4) | 10));
	}
	else
	{
		if (bC)
		{
			return Error(Peek(), "Use ret to pull C");
		}
		StartInstruction(&sInstruction, (uint16)((iStack << 14) | (1 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | 11));
	}
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// ret {P} {, stack} and rti {stack} (instruction code 11, extend 0).  ret pulls P
// (if asked) then C; rti pulls P then C.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Return(bool bRti)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	E65Register			eStack;
	E65Register			eRegister;
	bool				bPullP;
	bool				bFirst;

	eStack = R65_S;
	bPullP = bRti;
	bFirst = true;
	while (!Peek()->IsEndOfStatement())
	{
		if (!bFirst && !Expect(AP65_Comma, "','"))
		{
			return false;
		}
		bFirst = false;
		psToken = Peek();
		if (psToken->IsIdentifier("P") && !bRti)
		{
			Next();
			bPullP = true;
		}
		else if (IsRegister(psToken, &eRegister) && (Get65C64RegisterIndex(eRegister, gae65SVWY, 4) >= 0))
		{
			Next();
			eStack = eRegister;
		}
		else
		{
			return Error(psToken, bRti ? "Expected a stack register (S, V, W or Y)" : "Expected P or a stack register (S, V, W or Y)");
		}
	}

	StartInstruction(&sInstruction, (uint16)((Get65C64RegisterIndex(eStack, gae65SVWY, 4) << 14) | (bPullP << 9) | (1 << 8) | (bRti << 7) | (0 << 4) | 11));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A register list: R or (R, R, ...).  P and C are the status and the program counter.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::RegisterList(uint8* puiMask, bool* pbP, bool* pbC)
{
	bool			bList;
	SAsm65Token*	psStart;
	SAsm65Token*	psToken;
	E65Register		eRegister;
	uint8			uiBit;
	int				iCount;

	*puiMask = 0;
	*pbP = false;
	*pbC = false;
	psStart = Peek();
	bList = Accept(AP65_LeftRound);
	iCount = 0;
	do
	{
		iCount++;
		psToken = Peek();
		if (IsRegister(psToken, &eRegister))
		{
			uiBit = (uint8)(1 << (eRegister - R65_A));
			if (*puiMask & uiBit)
			{
				return Error(psToken, "Register is listed twice");
			}
			*puiMask |= uiBit;
		}
		else if (psToken->IsIdentifier("P"))
		{
			*pbP = true;
		}
		else if (psToken->IsIdentifier("C"))
		{
			*pbC = true;
		}
		else
		{
			return Error(psToken, "Expected a register, P or C");
		}
		Next();
	}
	while (bList && Accept(AP65_Comma));

	if (bList && !CloseGroup(psStart, iCount))
	{
		return false;
	}
	if ((*puiMask != 0) && (*pbP || *pbC))
	{
		return Error(psToken, "P and C cannot be listed with registers");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// psh registers {, stack}      instruction code 13, extend 0 (any stack register)
// psh P | C | (C, P) {, stack} instruction code 10 (stack S, V, W or Y)
// psh value{:width} {, stack}  push operand (an immediate; never C: push C then add)
// psh [address] {, stack}      push operand (the 32 bit value at the address)
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Push(void)
{
	SAsm65Instruction	sInstruction;
	SAsm65Operand		sOperand;
	SAsm65Token*		psToken;
	E65Register			eRegister;
	E65Register			eStack;
	uint8				uiBytes;
	bool				bSigned;

	psToken = Peek();
	//C is always the program counter register here, never a value.
	if (IsRegister(psToken, &eRegister) || psToken->IsIdentifier("P") || psToken->IsIdentifier("C") ||
		(psToken->Is(AP65_LeftRound) && (IsRegister(Peek(1), &eRegister) || Peek(1)->IsIdentifier("P") || Peek(1)->IsIdentifier("C"))))
	{
		return PushPullRegisters(true);
	}

	if (!Operand(&sOperand) || !StackRegister(&eStack))
	{
		return false;
	}
	if (sOperand.eKind == AO65_Memory)
	{
		return PushPullMemory(true, &sOperand, eStack);
	}

	if (sOperand.bSigned)
	{
		return Error(sOperand.psToken, "A pushed value cannot sign extend");
	}
	bSigned = !IsRelocatable(&sOperand.sValue) && (sOperand.sValue.iValue < 0);
	if (!ChooseBytes(&sOperand.sValue, bSigned, sOperand.uiWidthBytes, gauiWidthBytes, 7, 1, sOperand.psToken, &uiBytes))
	{
		return false;
	}
	StartInstruction(&sInstruction, (uint16)((eStack << 13) | (ImmediateCode(uiBytes) << 10) | (OPERAND_MODE_IMMEDIATE << 6) | (1 << 4) | 13));
	if (!AddRelocatableWords(&sInstruction, &sOperand.sValue, (uint64)sOperand.sValue.iValue, uiBytes, bSigned, false, sOperand.psToken))
	{
		return false;
	}
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// pul registers {, stack}      instruction code 13, extend 2
// pul P {, stack}              instruction code 11 (pull P only)
// pul [address] {, stack}      pull operand (32 bits into the address)
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Pull(void)
{
	SAsm65Operand	sOperand;
	E65Register		eStack;

	if (!Peek()->Is(AP65_LeftSquare))
	{
		return PushPullRegisters(false);
	}
	if (!Operand(&sOperand) || !StackRegister(&eStack))
	{
		return false;
	}
	return PushPullMemory(false, &sOperand, eStack);
}


//////////////////////////////////////////////////////////////////////////
//
// psh or pul a register list {, stack}: instruction code 13, extend 0 or 2.  A list of
// P and C is the program state instead.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::PushPullRegisters(bool bPush)
{
	SAsm65Instruction	sInstruction;
	E65Register			eStack;
	uint8				uiMask;
	bool				bP;
	bool				bC;

	if (!RegisterList(&uiMask, &bP, &bC))
	{
		return false;
	}
	if (bPush && bC && !Peek()->IsEndOfStatement() && !Peek()->Is(AP65_Comma))
	{
		return Error(Peek(), "C cannot be part of a pushed value; push C then add to it");
	}
	if (!StackRegister(&eStack))
	{
		return false;
	}
	if (bP || bC)
	{
		return ProgramState(bPush, bC, bP, eStack);
	}
	StartInstruction(&sInstruction, (uint16)((eStack << 13) | (uiMask << 6) | ((bPush ? 0 : 2) << 4) | 13));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// psh or pul the 32 bits at an address (instruction code 13, extend 1 or 3).  Only the
// one register addresses are allowed.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::PushPullMemory(bool bPush, SAsm65Operand* psOperand, E65Register eStack)
{
	SAsm65Instruction	sInstruction;
	SAsm65Memory		sMemory;
	uint16				uiField;

	if (!MemoryOperand(psOperand, &sMemory, 1))
	{
		return false;
	}
	if (Is65C64TwoRegisterMode(sMemory.uiMode))
	{
		return Error(psOperand->psToken, bPush ? "Push cannot use a two register address" : "Pull cannot use a two register address");
	}
	if (!SingleRegisterField(&sMemory, gae65Registers, 8, psOperand->psToken, &uiField))
	{
		return false;
	}
	StartInstruction(&sInstruction, (uint16)((eStack << 13) | (uiField << 10) | (sMemory.uiMode << 6) | ((bPush ? 1 : 3) << 4) | 13));
	AddAddressWords(&sInstruction, &sMemory);
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// {, stack}: any register, S when left out.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::StackRegister(E65Register* peStack)
{
	*peStack = R65_S;
	if (Accept(AP65_Comma))
	{
		return Register(peStack, "a stack register");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// sep flag or sep (flag, flag, ...), and cep: e, l, f, g, n, v, z, c.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::StateBits(uint16 uiSub)
{
	static const char*	aszFlags[8] = { "c", "z", "v", "n", "g", "f", "l", "e" };
	SAsm65Instruction	sInstruction;
	uint16				uiMask;

	if (!NameList(aszFlags, 8, "a status flag (e, l, f, g, n, v, z or c)", &uiMask))
	{
		return false;
	}
	StartInstruction(&sInstruction, (uint16)((uiMask << 8) | (uiSub << 6) | (1 << 4) | 11));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// name or (name, name, ...) as a mask: the i-th name is bit i.  Two or more names must
// be in brackets (the user, 2026-10-23).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::NameList(const char** aszNames, int iNumNames, const char* szWhat, uint16* puiMask)
{
	SAsm65Token*	psStart;
	SAsm65Token*	psToken;
	bool			bList;
	int				i;
	int				iCount;

	*puiMask = 0;
	psStart = Peek();
	bList = Accept(AP65_LeftRound);
	iCount = 0;
	do
	{
		iCount++;
		psToken = Peek();
		for (i = 0; i < iNumNames; i++)
		{
			if (psToken->IsIdentifier(aszNames[i]))
			{
				break;
			}
		}
		if (i == iNumNames)
		{
			return ErrorExpected(psToken, szWhat);
		}
		Next();
		*puiMask |= (uint16)(1 << i);
	}
	while (Accept(AP65_Comma));

	if (bList)
	{
		return CloseGroup(psStart, iCount);
	}
	if (iCount > 1)
	{
		return Error(psStart, "More than one operand must be grouped in brackets");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The ')' of a group.  Brackets group two or more operands; a group of one is an error.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::CloseGroup(SAsm65Token* psOpen, int iCount)
{
	if (!Expect(AP65_RightRound, "')'"))
	{
		return false;
	}
	if (iCount < 2)
	{
		return Error(psOpen, "Brackets only group more than one operand");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A width in bits (8, 16, 32 or 64) as the bytes the instruction encodes, or 0.
//
//////////////////////////////////////////////////////////////////////////
static int64 WidthBitsToBytes(int64 iBits)
{
	if ((iBits == 8) || (iBits == 16) || (iBits == 32) || (iBits == 64))
	{
		return iBits / 8;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// wth R, bits|register and wtth (R1, R2), bits|register.  R is A, B, X or Y; the
// width is 8, 16, 32 or 64 bits (encoded as bytes).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Width(bool bTwo)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	E65Register			eFirst;
	E65Register			eSecond;
	E65Register			eWidth;
	int					iFirst;
	int					iSecond;
	int64				iBits;
	int64				iBytes;
	uint16				uiOperand;

	psToken = Peek();
	if (bTwo)
	{
		if (!Expect(AP65_LeftRound, "'('") || !Register(&eFirst, "a register") || !Expect(AP65_Comma, "','") || !Register(&eSecond, "a register") || !Expect(AP65_RightRound, "')'"))
		{
			return false;
		}
	}
	else
	{
		if (!Register(&eFirst, "a register"))
		{
			return false;
		}
		eSecond = R65_A;
	}
	iFirst = Get65C64RegisterIndex(eFirst, gae65ABXY, 4);
	iSecond = Get65C64RegisterIndex(eSecond, gae65ABXY, 4);
	if ((iFirst < 0) || (iSecond < 0))
	{
		return Error(psToken, "Only A, B, X and Y have a width");
	}
	if (!Expect(AP65_Comma, "','"))
	{
		return false;
	}

	psToken = Peek();
	if (IsRegister(psToken, &eWidth))
	{
		Next();
		uiOperand = (uint16)((0 << 11) | (eWidth << 8));
	}
	else
	{
		if (!ExpressionValue(&iBits))
		{
			return false;
		}
		iBytes = WidthBitsToBytes(iBits);
		if (iBytes == 0)
		{
			return Error(psToken, "A width is 8, 16, 32 or 64 bits");
		}
		uiOperand = (uint16)((1 << 11) | ((iBytes - 1) << 8));
	}

	StartInstruction(&sInstruction, (uint16)((iFirst << 14) | ((bTwo ? iSecond : 0) << 12) | uiOperand | ((bTwo ? 3 : 2) << 6) | (1 << 4) | 11));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// brk vector (0 to 1023).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Break(void)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	int64				iVector;

	psToken = Peek();
	if (!ExpressionValue(&iVector))
	{
		return false;
	}
	if ((iVector < 0) || (iVector > 1023))
	{
		return Error(psToken, "A break vector is 0 to 1023");
	}
	StartInstruction(&sInstruction, (uint16)((iVector << 6) | (2 << 4) | 11));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// stp {wake list}: l (the level interrupt) and e0 to e3 (the edge interrupts).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Stop(void)
{
	static const char*	aszWake[5] = { "l", "e0", "e1", "e2", "e3" };
	SAsm65Instruction	sInstruction;
	uint16				uiMask;

	uiMask = 0;
	if (!Peek()->IsEndOfStatement() && !NameList(aszWake, 5, "a wake interrupt (l, e0, e1, e2 or e3)", &uiMask))
	{
		return false;
	}
	StartInstruction(&sInstruction, (uint16)((uiMask << 8) | (0 << 6) | (3 << 4) | 11));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// gwth R, Q or gwth R, (Q, -1): Q's width in bits (less 1) into R.  Q is A, B, X or Y.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::GetWidth(void)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	E65Register			eDestination;
	E65Register			eQuery;
	bool				bList;
	bool				bDecrement;
	int					iQuery;
	int64				iSubtract;
	SAsm65Token*		psSubtract;

	if (!Register(&eDestination, "a register") || !Expect(AP65_Comma, "','"))
	{
		return false;
	}
	psToken = Peek();
	bList = Accept(AP65_LeftRound);
	if (!Register(&eQuery, "a register"))
	{
		return false;
	}
	//(Q, -1) subtracts one from the width (the user: -1 is not a direction here).
	bDecrement = bList;
	if (bList)
	{
		if (!Expect(AP65_Comma, "','"))
		{
			return false;
		}
		psSubtract = Peek();
		if (!ExpressionValue(&iSubtract) || !Expect(AP65_RightRound, "')'"))
		{
			return false;
		}
		if (iSubtract != -1)
		{
			return Error(psSubtract, "gwth can only subtract 1 (-1)");
		}
	}
	iQuery = Get65C64RegisterIndex(eQuery, gae65ABXY, 4);
	if (iQuery < 0)
	{
		return Error(psToken, "Only A, B, X and Y have a width");
	}

	StartInstruction(&sInstruction, (uint16)((iQuery << 14) | (bDecrement << 13) | (eDestination << 10) | (2 << 6) | (3 << 4) | 11));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// wthx R, bits{:s}: extend R from its width to bits (zero or sign extend).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ExtendWidth(void)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	E65Register			eRegister;
	int					iRegister;
	int64				iBits;
	int64				iBytes;
	bool				bSigned;

	psToken = Peek();
	if (!Register(&eRegister, "a register") || !Expect(AP65_Comma, "','"))
	{
		return false;
	}
	iRegister = Get65C64RegisterIndex(eRegister, gae65ABXY, 4);
	if (iRegister < 0)
	{
		return Error(psToken, "Only A, B, X and Y have a width");
	}

	psToken = Peek();
	bSigned = false;
	if (!ExpressionValue(&iBits) || !Suffixes(NULL, &bSigned, NULL))
	{
		return false;
	}
	iBytes = WidthBitsToBytes(iBits);
	if (iBytes == 0)
	{
		return Error(psToken, "A width is 8, 16, 32 or 64 bits");
	}

	StartInstruction(&sInstruction, (uint16)((iRegister << 14) | (bSigned << 11) | ((iBytes - 1) << 8) | (3 << 6) | (3 << 4) | 11));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// brac flag, target       bras flag, target       (flag: 0, 1, c, z, n, v, f or g)
// brasa (flag, second), target    braso (flag, second), target    (second: c, z, n or v)
// The target is an address (a value using a label), C + offset or a raw byte offset.
// Offsets are from the next instruction (C).  Offsets of
// 2, 4, 6, 8, -4, -6, -8 and -10 are embedded, others take an 8 to 32 bit immediate.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Branch(uint16 uiCondition)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	SAsm65Value			sTarget;
	uint8				uiFirst;
	uint8				uiSecond;
	int64				iOffset;
	uint8				uiBytes;
	int					i;
	bool				bRelocatable;

	uiSecond = 2;
	if (uiCondition >= 2)
	{
		if (!Expect(AP65_LeftRound, "'('") || !Flag(&uiFirst, true, "a status flag") || !Expect(AP65_Comma, "','"))
		{
			return false;
		}
		psToken = Peek();
		if (!Flag(&uiSecond, false, "a status flag") || !Expect(AP65_RightRound, "')'"))
		{
			return false;
		}
		if (uiSecond > 5)
		{
			return Error(psToken, "The second flag must be c, z, n or v");
		}
	}
	else if (!Flag(&uiFirst, true, "a status flag"))
	{
		return false;
	}
	if (!Expect(AP65_Comma, "','"))
	{
		return false;
	}

	psToken = Peek();
	mbRelative = true;
	if (!Expression(&sTarget))
	{
		return false;
	}
	mbRelative = false;
	if (!RelativeTarget(&sTarget, psToken))
	{
		return false;
	}

	bRelocatable = TargetIsRelocatable(&sTarget);
	iOffset = RelativeOffset(&sTarget, muiStatementAddress, 1);
	if (!bRelocatable && (GetMinimumWords() <= 1))
	{
		for (i = 0; i < 8; i++)
		{
			if (!sTarget.bResolved || (gai65BranchOffsets[i] == iOffset))
			{
				StartInstruction(&sInstruction, (uint16)((uiFirst << 13) | ((uiSecond - 2) << 11) | (i << 8) | (uiCondition << 6) | (0 << 4) | 12));
				Emit(&sInstruction);
				return true;
			}
		}
	}

	if (!RelativeBytes(&sTarget, psToken, &uiBytes, &iOffset))
	{
		return false;
	}
	StartInstruction(&sInstruction, (uint16)((uiFirst << 13) | ((uiSecond - 2) << 11) | ((uiBytes - 1) << 9) | (0 << 8) | (uiCondition << 6) | (1 << 4) | 12));
	if (!AddRelocatableWords(&sInstruction, &sTarget, (uint64)iOffset, uiBytes, true, true, psToken))
	{
		return false;
	}
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// brae (flag, flag), target    brane (flag, flag), target    (flags: c, z, n, v, f or g)
// Branches when the two flags are equal (brae) or not equal (brane).  The target is as
// in the qualified branches.  Offsets of -8, -4, 4 and 8 are embedded, others take an
// 8 to 32 bit immediate.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::BranchEqual(uint16 uiSub)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	SAsm65Value			sTarget;
	uint8				uiFirst;
	uint8				uiSecond;
	uint16				uiFlags;
	int64				iOffset;
	uint8				uiBytes;
	int					i;
	bool				bRelocatable;

	if (!Expect(AP65_LeftRound, "'('") || !Flag(&uiFirst, false, "a status flag") || !Expect(AP65_Comma, "','") ||
		!Flag(&uiSecond, false, "a status flag") || !Expect(AP65_RightRound, "')'") || !Expect(AP65_Comma, "','"))
	{
		return false;
	}
	uiFlags = (uint16)((uiFirst << 13) | (uiSecond << 10) | (uiSub << 6) | (2 << 4) | 12);

	psToken = Peek();
	mbRelative = true;
	if (!Expression(&sTarget))
	{
		return false;
	}
	mbRelative = false;
	if (!RelativeTarget(&sTarget, psToken))
	{
		return false;
	}

	bRelocatable = TargetIsRelocatable(&sTarget);
	iOffset = RelativeOffset(&sTarget, muiStatementAddress, 1);
	if (!bRelocatable && (GetMinimumWords() <= 1))
	{
		for (i = 0; i < 4; i++)
		{
			if (!sTarget.bResolved || (gai65EqualBranchOffsets[i] == iOffset))
			{
				StartInstruction(&sInstruction, (uint16)(uiFlags | (i << 8) | (1 << 7)));
				Emit(&sInstruction);
				return true;
			}
		}
	}

	if (!RelativeBytes(&sTarget, psToken, &uiBytes, &iOffset))
	{
		return false;
	}
	StartInstruction(&sInstruction, (uint16)(uiFlags | ((uiBytes - 1) << 8) | (0 << 7)));
	if (!AddRelocatableWords(&sInstruction, &sTarget, (uint64)iOffset, uiBytes, true, true, psToken))
	{
		return false;
	}
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// cpyx R, [address]:bits      memory of bits (8, 16, 32 or 64) into R, zero extended
// cpyx R, [address]:sbits     memory of bits (8, 16 or 32) into R, sign extended (code 15)
// cpyx [address]:bits, R      R's low bits into memory (swap)
// R is A, B, X or Y.  The address is a minimal addressing mode: [value {+ R}],
// [[value]], [[value + R]], [R] or [[R]], with R V, X or Y.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::CopyExtend(void)
{
	SAsm65Operand		sFirst;
	SAsm65Operand		sSecond;
	SAsm65Operand*		psMemoryOperand;
	SAsm65Operand*		psRegisterOperand;
	SAsm65Memory		sMemory;
	SAsm65Instruction	sInstruction;
	bool				bSwap;
	bool				bSigned;
	bool				bResult;
	int					iRegister;
	int					iMinimal;
	int					iWidthCode;
	uint16				uiAddress;
	int					i;

	//Only cpyx's memory operand takes a width.
	mbMemoryWidth = true;
	bResult = Operand(&sFirst) && Expect(AP65_Comma, "','") && Operand(&sSecond);
	mbMemoryWidth = false;
	if (!bResult)
	{
		return false;
	}

	bSwap = sFirst.eKind == AO65_Memory;
	psMemoryOperand = bSwap ? &sFirst : &sSecond;
	psRegisterOperand = bSwap ? &sSecond : &sFirst;
	if (psMemoryOperand->eKind != AO65_Memory)
	{
		return Error(psMemoryOperand->psToken, "cpyx needs a memory operand");
	}
	for (iWidthCode = 0; iWidthCode < 4; iWidthCode++)
	{
		if (gaui65CopyExtendBytes[iWidthCode] == psMemoryOperand->uiWidthBytes)
		{
			break;
		}
	}
	if (iWidthCode == 4)
	{
		return Error(psMemoryOperand->psToken, "cpyx memory needs a width of :8, :16, :32 or :64");
	}
	if (psRegisterOperand->eKind != AO65_Register)
	{
		return Error(psRegisterOperand->psToken, "cpyx needs a register operand");
	}
	if (psRegisterOperand->bSigned)
	{
		return Error(psRegisterOperand->psToken, "cpyx sign extends memory: [address]:s8, :s16 or :s32");
	}
	bSigned = psMemoryOperand->bSigned;
	if (bSigned && bSwap)
	{
		return Error(psMemoryOperand->psToken, "A copy into memory cannot sign extend");
	}
	if (bSigned && (psMemoryOperand->uiWidthBytes == 8))
	{
		return Error(psMemoryOperand->psToken, "64 bits has nothing to sign extend; use :64");
	}
	iRegister = Get65C64RegisterIndex(psRegisterOperand->eRegister, gae65ABXY, 4);
	if (iRegister < 0)
	{
		return Error(psRegisterOperand->psToken, "The register must be A, B, X or Y");
	}

	if (!MemoryOperand(psMemoryOperand, &sMemory, 1))
	{
		return false;
	}
	iMinimal = -1;
	for (i = 0; i < 8; i++)
	{
		if (gaui65MinimalModes[i] == sMemory.uiMode)
		{
			iMinimal = i;
			break;
		}
	}
	if (iMinimal < 0)
	{
		return Error(psMemoryOperand->psToken, "cpyx cannot use that addressing mode");
	}
	if (!SingleRegisterField(&sMemory, gae65NoneVXY, 4, psMemoryOperand->psToken, &uiAddress))
	{
		return false;
	}

	if (bSigned)
	{
		//Instruction code 15 extend 6: there is no room in code 12.
		StartInstruction(&sInstruction, (uint16)((iRegister << 14) | (uiAddress << 12) | (iMinimal << 9) | (iWidthCode << 7) | (6 << 4) | 15));
	}
	else
	{
		StartInstruction(&sInstruction, (uint16)((bSwap << 15) | (iRegister << 13) | (uiAddress << 11) | (iMinimal << 8) | (iWidthCode << 6) | (3 << 4) | 12));
	}
	AddAddressWords(&sInstruction, &sMemory);
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// nop {cycles} (1 to 8 cycles of waiting, 1 when left out).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::NoOperation(void)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	int64				iWait;

	iWait = 1;
	psToken = Peek();
	if (!psToken->IsEndOfStatement())
	{
		if (!ExpressionValue(&iWait))
		{
			return false;
		}
		if ((iWait < 1) || (iWait > 8))
		{
			return Error(psToken, "A nop waits 1 to 8 cycles");
		}
	}
	StartInstruction(&sInstruction, (uint16)(((iWait - 1) << 9) | (1 << 8) | (1 << 6) | (1 << 4) | 12));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// mul and div (no operands; they use A, B, W and Y).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::MultiplyDivide(uint16 uiSub)
{
	SAsm65Instruction	sInstruction;

	StartInstruction(&sInstruction, (uint16)((uiSub << 6) | (0 << 4) | 14));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// neg R1 {, R2} and abs R1 {, R2}: R1 gets (the absolute of) R2 negated.  R2 is R1
// when it is left out.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::NegateAbsolute(uint16 uiSub)
{
	SAsm65Instruction	sInstruction;
	E65Register			eDestination;
	E65Register			eSource;

	if (!Register(&eDestination, "a register"))
	{
		return false;
	}
	eSource = eDestination;
	if (Accept(AP65_Comma) && !Register(&eSource, "a register"))
	{
		return false;
	}
	StartInstruction(&sInstruction, (uint16)((eDestination << 13) | (eSource << 8) | (uiSub << 6) | (0 << 4) | 14));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// fsbi R, I {, +1|-1} ...: scan R from the unit at index I up (or down).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Find(bool bSet, uint16 uiUnit)
{
	SAsm65Instruction	sInstruction;
	E65Register			eRegister;
	E65Register			eIndex;
	bool				bDecrement;

	if (!Register(&eRegister, "a register") || !Expect(AP65_Comma, "','") || !Register(&eIndex, "an index register"))
	{
		return false;
	}
	bDecrement = false;
	if (Accept(AP65_Comma) && !Direction(&bDecrement))
	{
		return false;
	}
	StartInstruction(&sInstruction, (uint16)((eRegister << 13) | (eIndex << 10) | (bDecrement << 9) | (bSet << 8) | (uiUnit << 6) | (1 << 4) | 14));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// (R, index) where the index is a register or a value.  The index field is a byte of
// the operand word: bit 7 set for a value (bits 6-4 a byte index, bits 6-1 a bit
// index), else the register in bits 6-4.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::SubcopyPair(E65Register* peRegister, uint16* puiIndexField, bool bBits)
{
	SAsm65Token*	psToken;
	E65Register		eIndex;
	int64			iIndex;

	*puiIndexField = 0;
	if (!Expect(AP65_LeftRound, "'('") || !Register(peRegister, "a register") || !Expect(AP65_Comma, "','"))
	{
		return false;
	}
	psToken = Peek();
	if (IsRegister(psToken, &eIndex))
	{
		Next();
		*puiIndexField = (uint16)(eIndex << 4);
	}
	else
	{
		if (!ExpressionValue(&iIndex))
		{
			return false;
		}
		if ((iIndex < 0) || (iIndex > (bBits ? 63 : 7)))
		{
			return Error(psToken, bBits ? "A bit index is 0 to 63" : "A byte index is 0 to 7");
		}
		*puiIndexField = (uint16)(0x80 | (bBits ? (iIndex << 1) : (iIndex << 4)));
	}
	return Expect(AP65_RightRound, "')'");
}


//////////////////////////////////////////////////////////////////////////
//
// scp (source, index), (destination, index), count    (count 1 to 8 bytes or a register)
// scb (source, index), (destination, index), count    (count 1 to 64 bits or a register)
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Subcopy(bool bBits)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	E65Register			eSource;
	E65Register			eDestination;
	E65Register			eCount;
	uint16				uiSourceIndex;
	uint16				uiDestinationIndex;
	uint16				uiCount;
	uint16				uiWord;
	int64				iCount;

	if (!SubcopyPair(&eSource, &uiSourceIndex, bBits) || !Expect(AP65_Comma, "','") || !SubcopyPair(&eDestination, &uiDestinationIndex, bBits) || !Expect(AP65_Comma, "','"))
	{
		return false;
	}

	uiWord = (uint16)((uiSourceIndex << 8) | uiDestinationIndex);
	psToken = Peek();
	if (IsRegister(psToken, &eCount))
	{
		Next();
		uiCount = (uint16)((0 << 9) | (eCount << 6));
	}
	else
	{
		if (!ExpressionValue(&iCount))
		{
			return false;
		}
		if ((iCount < 1) || (iCount > (bBits ? 64 : 8)))
		{
			return Error(psToken, bBits ? "A bit count is 1 to 64" : "A byte count is 1 to 8");
		}
		if (bBits)
		{
			uiCount = (uint16)((1 << 9) | (((iCount - 1) & 0xF) << 5));
			uiWord |= (uint16)(((((iCount - 1) >> 4) & 1) << 8) | (((iCount - 1) >> 5) & 1));
		}
		else
		{
			uiCount = (uint16)((1 << 9) | ((iCount - 1) << 6));
		}
	}

	StartInstruction(&sInstruction, (uint16)((eSource << 13) | (eDestination << 10) | uiCount | (bBits << 4) | 9));
	sInstruction.auiWords[1] = uiWord;
	sInstruction.uiWords = 2;
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A bit index: a register (style 7), 0 to 5 (the style is the index) or 6 to 63 (style
// 6, an immediate).  bForceWord makes 0 to 5 an immediate too.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::BitIndex(uint16* puiStyle, uint16* puiWord, bool bForceWord)
{
	SAsm65Token*	psToken;
	E65Register		eIndex;
	int64			iIndex;

	psToken = Peek();
	*puiStyle = 0;
	*puiWord = 0;
	if (IsRegister(psToken, &eIndex))
	{
		Next();
		*puiStyle = 7;
		*puiWord = (uint16)(eIndex << 13);
		return true;
	}
	if (!ExpressionValue(&iIndex))
	{
		return false;
	}
	if ((iIndex < 0) || (iIndex > 63))
	{
		return Error(psToken, "A bit index is 0 to 63");
	}
	if ((iIndex <= 5) && !bForceWord)
	{
		*puiStyle = (uint16)iIndex;
	}
	else
	{
		*puiStyle = 6;
		*puiWord = (uint16)(iIndex << 10);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// bist flag, (R, index): bit index of R into the flag (c, z, n, v, f or g).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::BitToStatus(void)
{
	SAsm65Instruction	sInstruction;
	E65Register			eRegister;
	uint8				uiFlag;
	uint16				uiStyle;
	uint16				uiWord;

	if (!Flag(&uiFlag, false, "a status flag") || !Expect(AP65_Comma, "','") || !Expect(AP65_LeftRound, "'('") ||
		!Register(&eRegister, "a register") || !Expect(AP65_Comma, "','") || !BitIndex(&uiStyle, &uiWord, false) || !Expect(AP65_RightRound, "')'"))
	{
		return false;
	}

	StartInstruction(&sInstruction, (uint16)((uiFlag << 13) | (eRegister << 10) | (uiStyle << 7) | (4 << 4) | 15));
	if (uiStyle >= 6)
	{
		sInstruction.auiWords[1] = uiWord;
		sInstruction.uiWords = 2;
	}
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// stab (R, index {, repeat {, +1|-1}}), flag: the flag (0, 1, c, z, n, v, f or g)
// into bit index of R, or into repeat bits from the index up (or down).  repeat is a
// register or 0 to 63.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::StatusToBit(void)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	E65Register			eRegister;
	E65Register			eRepeat;
	uint8				uiFlag;
	uint16				uiStyle;
	uint16				uiWord;
	uint16				uiRepeat;
	bool				bRepeat;
	bool				bDecrement;
	bool				bRepeatRegister;
	int64				iRepeat;
	size				uiIndexToken;
	size				uiEndToken;

	if (!Expect(AP65_LeftRound, "'('") || !Register(&eRegister, "a register") || !Expect(AP65_Comma, "','"))
	{
		return false;
	}

	//The index's style depends on whether a repeat follows, so read the index twice.
	uiIndexToken = muiToken;
	if (!BitIndex(&uiStyle, &uiWord, false))
	{
		return false;
	}
	bRepeat = Accept(AP65_Comma);
	bDecrement = false;
	uiRepeat = 0;
	if (bRepeat)
	{
		uiEndToken = muiToken;
		muiToken = uiIndexToken;
		BitIndex(&uiStyle, &uiWord, true);
		muiToken = uiEndToken;

		psToken = Peek();
		bRepeatRegister = IsRegister(psToken, &eRepeat);
		if (bRepeatRegister)
		{
			Next();
			uiRepeat = (uint16)(eRepeat << 5);
		}
		else
		{
			if (!ExpressionValue(&iRepeat))
			{
				return false;
			}
			if ((iRepeat < 0) || (iRepeat > 63))
			{
				return Error(psToken, "A repeat count is 0 to 63");
			}
			uiRepeat = (uint16)(iRepeat << 2);
		}
		if (Accept(AP65_Comma) && !Direction(&bDecrement))
		{
			return false;
		}
		uiWord |= (uint16)(((bDecrement ? 2 : 0) + (bRepeatRegister ? 0 : 1)) << 8) | uiRepeat;
	}
	else if (uiStyle >= 6)
	{
		//An operand word always has a repeat (the simulator writes as many bits as it says), so one bit is a repeat of 1.
		uiWord |= (uint16)((1 << 8) | (1 << 2));
	}

	if (!Expect(AP65_RightRound, "')'") || !Expect(AP65_Comma, "','") || !Flag(&uiFlag, true, "a status flag"))
	{
		return false;
	}

	StartInstruction(&sInstruction, (uint16)((uiFlag << 13) | (eRegister << 10) | (uiStyle << 7) | (5 << 4) | 15));
	if (uiStyle >= 6)
	{
		sInstruction.auiWords[1] = uiWord;
		sInstruction.uiWords = 2;
	}
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// stat destination, source: c, z, n, v, f or g from 0, 1, c, z, n, v, f or g.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::StatusToStatus(void)
{
	SAsm65Instruction	sInstruction;
	uint8				uiDestination;
	uint8				uiSource;

	if (!Flag(&uiDestination, false, "a status flag") || !Expect(AP65_Comma, "','") || !Flag(&uiSource, true, "a status flag"))
	{
		return false;
	}
	StartInstruction(&sInstruction, (uint16)((uiDestination << 13) | (uiSource << 10) | (7 << 4) | 15));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// bop R, V, +1|-1, A {, stop}   (instruction code 8, b4 = 0)
// R is B, S, V, W, X or Y (A counts).  The element width is R's width.  bcpy never
// stops early.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Block(uint16 uiSub)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	E65Register			eRegister;
	bool				bDecrement;
	bool				bAllowStop;
	uint8				uiStopStyle;

	psToken = Peek();
	if (!Register(&eRegister, "a register"))
	{
		return false;
	}
	if (eRegister == R65_A)
	{
		return Error(psToken, "A counts the elements; use B, S, V, W, X or Y");
	}
	if (!Expect(AP65_Comma, "','"))
	{
		return false;
	}
	if (!FixedRegister(R65_V) || !Expect(AP65_Comma, "','") || !Direction(&bDecrement) || !Expect(AP65_Comma, "','") || !FixedRegister(R65_A))
	{
		return false;
	}
	psToken = Peek(1);
	if (!StopStyle(&bAllowStop, &uiStopStyle))
	{
		return false;
	}
	if (bAllowStop && (uiSub == 0))
	{
		return Error(psToken, "bcpy cannot stop early");
	}

	StartInstruction(&sInstruction, (uint16)((bDecrement << 15) | (bAllowStop << 14) | (uiStopStyle << 12) | (eRegister << 9) | (uiSub << 5) | (0 << 4) | 8));
	Emit(&sInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// vop W, V, direction {, V direction}, A, width {, stop}   (instruction code 8, b4 = 1)
// One direction (+1 or -1) is for both W and V; two are W's then V's.  Block and vector
// operands are never grouped in brackets.  The element width is 8, 16, 32 or 64 bits.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Vector(uint16 uiSub)
{
	SAsm65Instruction	sInstruction;
	SAsm65Token*		psToken;
	bool				bDecrementV;
	bool				bDecrementW;
	bool				bAllowStop;
	uint8				uiStopStyle;
	int64				iBits;
	uint16				uiWidth;

	if (!FixedRegister(R65_W) || !Expect(AP65_Comma, "','") || !FixedRegister(R65_V) || !Expect(AP65_Comma, "','"))
	{
		return false;
	}

	//One direction for both, or W's then V's (never in brackets).
	if (!Direction(&bDecrementW) || !Expect(AP65_Comma, "','"))
	{
		return false;
	}
	bDecrementV = bDecrementW;
	if (Peek()->eType != AT65_Identifier)
	{
		if (!Direction(&bDecrementV) || !Expect(AP65_Comma, "','"))
		{
			return false;
		}
	}
	if (!FixedRegister(R65_A) || !Expect(AP65_Comma, "','"))
	{
		return false;
	}

	psToken = Peek();
	if (!ExpressionValue(&iBits))
	{
		return false;
	}
	switch (iBits)
	{
	case 8:
		uiWidth = 0;
		break;
	case 16:
		uiWidth = 1;
		break;
	case 32:
		uiWidth = 2;
		break;
	case 64:
		uiWidth = 3;
		break;
	default:
		return Error(psToken, "A vector element is 8, 16, 32 or 64 bits");
	}
	if (!StopStyle(&bAllowStop, &uiStopStyle))
	{
		return false;
	}

	StartInstruction(&sInstruction, (uint16)((bDecrementV << 15) | (bAllowStop << 14) | (uiStopStyle << 12) | (bDecrementW << 11) | (uiWidth << 9) | (uiSub << 5) | (1 << 4) | 8));
	Emit(&sInstruction);
	return true;
}
