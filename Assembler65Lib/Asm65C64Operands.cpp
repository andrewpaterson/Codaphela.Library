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
#include "Asm65C64Assembler.h"


//////////////////////////////////////////////////////////////////////////
//
// Registers are upper case: A, B, S, V, W, X and Y.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::IsRegister(SAsm65Token* psToken, E65Register* peRegister)
{
	static const char		acNames[7] = { 'A', 'B', 'S', 'V', 'W', 'X', 'Y' };
	static const E65Register	aeRegisters[7] = { R65_A, R65_B, R65_S, R65_V, R65_W, R65_X, R65_Y };
	int						i;

	if ((psToken->eType != AT65_Identifier) || (psToken->uiLength != 1))
	{
		return false;
	}
	for (i = 0; i < 7; i++)
	{
		if (psToken->szText[0] == acNames[i])
		{
			*peRegister = aeRegisters[i];
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A register's name: not a symbol, and an expression stops before + register.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::IsRegisterName(SAsm65Token* psToken)
{
	E65Register	eRegister;

	return IsRegister(psToken, &eRegister);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::Register(E65Register* peRegister, const char* szWhat)
{
	if (IsRegister(Peek(), peRegister))
	{
		Next();
		return true;
	}
	return ErrorExpected(Peek(), szWhat);
}


//////////////////////////////////////////////////////////////////////////
//
// The one register allowed here (the V, W and A of the block and vector operations).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::FixedRegister(E65Register eRegister)
{
	E65Register	eFound;

	if (IsRegister(Peek(), &eFound) && (eFound == eRegister))
	{
		Next();
		return true;
	}
	return ErrorExpected(Peek(), Get65C64RegisterName(eRegister));
}


//////////////////////////////////////////////////////////////////////////
//
// Status flags as numbered by the branches: 0, 1, c, z, n, v, f, g.  0 and 1 are only
// allowed where a flag is read.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::Flag(uint8* puiFlag, bool bAllowConstant, const char* szWhat)
{
	static const char*	aszFlags[8] = { "0", "1", "c", "z", "n", "v", "f", "g" };
	SAsm65Token*		psToken;
	uint8				i;

	psToken = Peek();
	if (bAllowConstant && (psToken->eType == AT65_Number) && (psToken->uiLength == 1) && ((psToken->iValue == 0) || (psToken->iValue == 1)))
	{
		Next();
		*puiFlag = (uint8)psToken->iValue;
		return true;
	}
	for (i = 2; i < 8; i++)
	{
		if (psToken->IsIdentifier(aszFlags[i]))
		{
			Next();
			*puiFlag = i;
			return true;
		}
	}
	return ErrorExpected(psToken, szWhat);
}


//////////////////////////////////////////////////////////////////////////
//
// +1 or -1.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::Direction(bool* pbDecrement)
{
	SAsm65Token*	psToken;
	SAsm65Value		sValue;

	psToken = Peek();
	if (BracketedList())
	{
		return Error(psToken, "Directions cannot be grouped in brackets");
	}
	if (!Expression(&sValue))
	{
		return false;
	}
	if (sValue.iValue == 1)
	{
		*pbDecrement = false;
		return true;
	}
	else if (sValue.iValue == -1)
	{
		*pbDecrement = true;
		return true;
	}
	return Error(psToken, "Expected a direction of +1 or -1");
}


//////////////////////////////////////////////////////////////////////////
//
// True when the next tokens are a bracketed group: '(' with a comma inside its
// matching ')'.  Brackets in a single argument's arithmetic have no comma.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::BracketedList(void)
{
	size			uiOffset;
	int				iDepth;
	SAsm65Token*	psToken;

	if (!Peek()->Is(AP65_LeftRound))
	{
		return false;
	}
	iDepth = 0;
	for (uiOffset = 0;; uiOffset++)
	{
		psToken = Peek(uiOffset);
		if (psToken->IsEndOfStatement())
		{
			return false;
		}
		if (psToken->Is(AP65_LeftRound))
		{
			iDepth++;
		}
		else if (psToken->Is(AP65_RightRound))
		{
			iDepth--;
			if (iDepth == 0)
			{
				return false;
			}
		}
		else if (psToken->Is(AP65_Comma) && (iDepth == 1))
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An optional stop condition: z, !z, n or !n.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::StopStyle(bool* pbAllowStop, uint8* puiStopStyle)
{
	bool			bNot;
	SAsm65Token*	psToken;

	*pbAllowStop = false;
	*puiStopStyle = 0;
	if (!Accept(AP65_Comma))
	{
		return true;
	}

	bNot = Accept(AP65_LogicalNot);
	psToken = Peek();
	if (psToken->IsIdentifier("z"))
	{
		*puiStopStyle = bNot ? 1 : 0;
	}
	else if (psToken->IsIdentifier("n"))
	{
		*puiStopStyle = bNot ? 3 : 2;
	}
	else
	{
		return Error(psToken, "Expected a stop condition (z, !z, n or !n)");
	}
	Next();
	*pbAllowStop = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 8, 16, 24, 32, 40, 48 or 64 (the widths of the 3 bit immediate width fields).
//
//////////////////////////////////////////////////////////////////////////
static bool IsImmediateWidth(int64 iBits)
{
	int	i;

	for (i = 1; i < 8; i++)
	{
		if (iBits == 8 * gaui65ImmediateBytes[i])
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// At most one suffix:
//   :8 :16 :24 :32 :40 :48 :64		an immediate's width
//   :16 :32							an address's size (16 an offset, 32 absolute)
//   :s								sign extend
//   :s8 :s16 ... :s64				sign extend an immediate of that width
// :s:16, :16:s and :16s are errors (the user, 2026-10-23).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::Suffixes(uint8* puiWidthBytes, bool* pbSigned, uint8* puiAddressBits)
{
	SAsm65Token*	psToken;
	int64			iBits;
	uint32			i;

	if (!Accept(AP65_Colon))
	{
		return true;
	}

	psToken = Peek();
	iBits = 0;
	if ((psToken->eType == AT65_Identifier) && (psToken->szText[0] == 's') && pbSigned)
	{
		for (i = 1; i < psToken->uiLength; i++)
		{
			if ((psToken->szText[i] < '0') || (psToken->szText[i] > '9'))
			{
				return Error(psToken, "Expected :s or a signed width such as :s16");
			}
			iBits = iBits * 10 + (psToken->szText[i] - '0');
		}
		if ((psToken->uiLength > 1) && (iBits == 0))
		{
			return Error(psToken, "A width is :8, :16, :24, :32, :40, :48 or :64");
		}
		if ((psToken->uiLength > 1) && !puiWidthBytes)
		{
			return Error(psToken, "A width is not allowed here");
		}
		*pbSigned = true;
	}
	else if (psToken->eType == AT65_Number)
	{
		iBits = psToken->iValue;
	}
	else
	{
		return Error(psToken, "Expected a width or s");
	}
	Next();

	if (iBits != 0)
	{
		if (puiAddressBits)
		{
			if ((iBits != 16) && (iBits != 32))
			{
				return Error(psToken, "An address is :16 (offset) or :32 (absolute)");
			}
			*puiAddressBits = (uint8)iBits;
		}
		else if (puiWidthBytes)
		{
			if (!IsImmediateWidth(iBits))
			{
				return Error(psToken, "A width is :8, :16, :24, :32, :40, :48 or :64");
			}
			*puiWidthBytes = (uint8)(iBits / 8);
		}
		else
		{
			return Error(psToken, "A width is not allowed here");
		}
	}

	if (Peek()->Is(AP65_Colon))
	{
		return Error(Peek(), "Only one suffix is allowed; a signed width is written :s16");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// base {+ inner} where base is a register or a value (with an optional :16 or :32).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::AddressInner(SAsm65C64Address* psAddress)
{
	SAsm65Token*	psToken;

	psToken = Peek();
	if (IsRegister(psToken, &psAddress->eBase))
	{
		Next();
		if (Peek()->Is(AP65_Colon))
		{
			return Error(Peek(), "A register address has no size");
		}
	}
	else
	{
		if (!Expression(&psAddress->sValue))
		{
			return false;
		}
		if (!Suffixes(NULL, NULL, &psAddress->uiAddressBits))
		{
			return false;
		}
	}

	if (Peek()->Is(AP65_Plus) && IsRegister(Peek(1), &psAddress->eInner))
	{
		Next();
		Next();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// inner | [inner] {+ post}
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::Address(SAsm65C64Address* psAddress)
{
	psAddress->bIndirect = false;
	psAddress->eBase = R65_Zero;
	ClearValue(&psAddress->sValue);
	psAddress->uiAddressBits = 0;
	psAddress->eInner = R65_Zero;
	psAddress->ePost = R65_Zero;
	psAddress->psToken = Peek();

	//Messages about the address point at its value or first register, not at '['.
	if (Accept(AP65_LeftSquare))
	{
		psAddress->bIndirect = true;
		psAddress->psToken = Peek();
		if (!AddressInner(psAddress))
		{
			return false;
		}
		if (!Expect(AP65_RightSquare, "']'"))
		{
			return false;
		}
		if (Peek()->Is(AP65_Plus) && IsRegister(Peek(1), &psAddress->ePost))
		{
			Next();
			Next();
		}
		return true;
	}
	return AddressInner(psAddress);
}


//////////////////////////////////////////////////////////////////////////
//
// register{:s} | [address] | value{:width}{:s}
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::Operand(SAsm65C64Operand* psOperand)
{
	SAsm65Token*	psToken;

	psToken = Peek();
	psOperand->eKind = AO65C64_None;
	psOperand->eRegister = R65_Zero;
	ClearValue(&psOperand->sValue);
	psOperand->uiWidthBytes = 0;
	psOperand->bSigned = false;
	psOperand->psToken = psToken;

	if (IsRegister(psToken, &psOperand->eRegister))
	{
		Next();
		psOperand->eKind = AO65C64_Register;
		return Suffixes(NULL, &psOperand->bSigned, NULL);
	}
	else if (Accept(AP65_LeftSquare))
	{
		psOperand->eKind = AO65C64_Memory;
		if (!Address(&psOperand->sAddress))
		{
			return false;
		}
		psOperand->psToken = psOperand->sAddress.psToken;
		if (!Expect(AP65_RightSquare, "']'"))
		{
			return false;
		}
		return Suffixes(mbMemoryWidth ? &psOperand->uiWidthBytes : NULL, &psOperand->bSigned, NULL);
	}

	psOperand->eKind = AO65C64_Constant;
	if (!Expression(&psOperand->sValue))
	{
		return false;
	}
	return Suffixes(&psOperand->uiWidthBytes, &psOperand->bSigned, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// The operand access mode of an address (not its words, which depend on the size).
//   value {+ R}          2 absolute or 6 offset (with or without a register)
//   R                    10
//   R1 + R2              11
//   [value]              3 or 7
//   [value + R]          4 or 8
//   [value] + R          5 or 9
//   [R]                  12
//   [R1 + R2]            13
//   [R1] + R2            14
// uiMode is the absolute mode; the caller picks the offset mode (mode + 4).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Assembler::Memory(SAsm65C64Address* psAddress, SAsm65C64Memory* psMemory)
{
	psMemory->eFirst = R65_Zero;
	psMemory->eSecond = R65_Zero;
	psMemory->uiAddressWords = 0;
	psMemory->uiAddress = (uint32)psAddress->sValue.iValue;
	psMemory->psValue = &psAddress->sValue;
	psMemory->psToken = psAddress->psToken;

	if (psAddress->eBase != R65_Zero)
	{
		psMemory->eFirst = psAddress->eBase;
		if (!psAddress->bIndirect)
		{
			if (psAddress->eInner == R65_Zero)
			{
				psMemory->uiMode = OPERAND_MODE_ADDRESS_REGISTER;
			}
			else
			{
				psMemory->uiMode = OPERAND_MODE_REGISTER_REGISTER;
				psMemory->eSecond = psAddress->eInner;
			}
		}
		else if ((psAddress->eInner != R65_Zero) && (psAddress->ePost != R65_Zero))
		{
			return Error(psAddress->psToken, "An address has at most two registers");
		}
		else if (psAddress->eInner != R65_Zero)
		{
			psMemory->uiMode = OPERAND_MODE_REGISTER_REGISTER_INDIRECT;
			psMemory->eSecond = psAddress->eInner;
		}
		else if (psAddress->ePost != R65_Zero)
		{
			psMemory->uiMode = OPERAND_MODE_REGISTER_INDIRECT_REGISTER;
			psMemory->eSecond = psAddress->ePost;
		}
		else
		{
			psMemory->uiMode = OPERAND_MODE_REGISTER_INDIRECT;
		}
		return true;
	}

	if (!psAddress->bIndirect)
	{
		psMemory->uiMode = OPERAND_MODE_ABSOLUTE;
		psMemory->eFirst = psAddress->eInner;
	}
	else if ((psAddress->eInner != R65_Zero) && (psAddress->ePost != R65_Zero))
	{
		return Error(psAddress->psToken, "An address has at most one register with a value");
	}
	else if (psAddress->eInner != R65_Zero)
	{
		psMemory->uiMode = OPERAND_MODE_ABSOLUTE_REGISTER_INDIRECT;
		psMemory->eFirst = psAddress->eInner;
	}
	else if (psAddress->ePost != R65_Zero)
	{
		psMemory->uiMode = OPERAND_MODE_ABSOLUTE_INDIRECT_REGISTER;
		psMemory->eFirst = psAddress->ePost;
	}
	else
	{
		psMemory->uiMode = OPERAND_MODE_ABSOLUTE_INDIRECT;
	}
	psMemory->uiAddressWords = 2;
	return true;
}

