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
#include "Asm816Assembler.h"


//The status bits rep and sep name.
static const char	gacAsm816Flags[8] = { 'c', 'z', 'i', 'd', 'x', 'm', 'v', 'n' };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm816Assembler::Init(void)
{
	CAsm65Assembler::Init();
	muiWidthA = ASM816_WIDTH_UNKNOWN;
	muiWidthX = ASM816_WIDTH_UNKNOWN;
	mbRelax = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A, X, Y, S and D.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Assembler::IsRegisterName(SAsm65Token* psToken)
{
	if ((psToken->eType != AT65_Identifier) || (psToken->uiLength != 1))
	{
		return false;
	}
	return strchr("AXYSD", psToken->szText[0]) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// W65C816S instructions are bytes.
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm816Assembler::InstructionAlignment(void)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
// .width and .word24.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Assembler::ProcessorDirective(SAsm65Token* psDirective, bool* pbResult)
{
	if (psDirective->IsIdentifierIgnoreCase(".width"))
	{
		*pbResult = Width();
		return true;
	}
	else if (psDirective->IsIdentifierIgnoreCase(".word24"))
	{
		*pbResult = Data(3);
		return true;
	}
	else if (psDirective->IsIdentifierIgnoreCase(".relax"))
	{
		mbRelax = true;
		*pbResult = true;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The widths are unknown at the start of each section until rep, sep or .width, and
// branches aren't relaxed until .relax.
//
//////////////////////////////////////////////////////////////////////////
void CAsm816Assembler::SectionStarted(void)
{
	muiWidthA = ASM816_WIDTH_UNKNOWN;
	muiWidthX = ASM816_WIDTH_UNKNOWN;
	mbRelax = false;
}


//////////////////////////////////////////////////////////////////////////
//
// .width A, 16  .width X, 8  .width (A, X), 16
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Assembler::Width(void)
{
	SAsm65Token*	psOpen;
	SAsm65Token*	psToken;
	bool			bA;
	bool			bX;
	int64			iBits;
	int				iCount;

	bA = false;
	bX = false;
	psOpen = Peek();
	iCount = 0;
	if (Accept(AP65_LeftRound))
	{
		do
		{
			psToken = Next();
			if (psToken->IsIdentifier("A"))
			{
				bA = true;
			}
			else if (psToken->IsIdentifier("X") || psToken->IsIdentifier("Y"))
			{
				bX = true;
			}
			else
			{
				return ErrorExpected(psToken, "A or X");
			}
			iCount++;
		}
		while (Accept(AP65_Comma));
		if (!Expect(AP65_RightRound, "')'"))
		{
			return false;
		}
		if (iCount < 2)
		{
			return Error(psOpen, "Round brackets group two or more registers");
		}
	}
	else
	{
		psToken = Next();
		if (psToken->IsIdentifier("A"))
		{
			bA = true;
		}
		else if (psToken->IsIdentifier("X") || psToken->IsIdentifier("Y"))
		{
			bX = true;
		}
		else
		{
			return ErrorExpected(psToken, "A, X or (A, X)");
		}
	}

	if (!Expect(AP65_Comma, "','"))
	{
		return false;
	}
	psToken = Peek();
	if (!ExpressionValue(&iBits))
	{
		return false;
	}
	if ((iBits != 8) && (iBits != 16))
	{
		return Error(psToken, "A width is 8 or 16 bits");
	}
	if (bA)
	{
		muiWidthA = (uint8)iBits;
	}
	if (bX)
	{
		muiWidthX = (uint8)iBits;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// rep and sep's operand: flags (rep m, rep (m, x)) or a value (rep $30).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Assembler::StatusBits(uint8* puiBits)
{
	SAsm65Token*	psToken;
	SAsm65Value		sValue;
	const char*		pcFlag;
	int				iCount;

	*puiBits = 0;
	psToken = Peek();
	if (Accept(AP65_LeftRound))
	{
		iCount = 0;
		do
		{
			psToken = Next();
			pcFlag = ((psToken->eType == AT65_Identifier) && (psToken->uiLength == 1)) ? (const char*)memchr(gacAsm816Flags, psToken->szText[0], 8) : NULL;
			if (pcFlag == NULL)
			{
				return ErrorExpected(psToken, "a flag (c, z, i, d, x, m, v or n)");
			}
			*puiBits |= (uint8)(1 << (pcFlag - gacAsm816Flags));
			iCount++;
		}
		while (Accept(AP65_Comma));
		if (!Expect(AP65_RightRound, "')'"))
		{
			return false;
		}
		if (iCount < 2)
		{
			return Error(psToken, "Round brackets group two or more flags");
		}
		return true;
	}

	if ((psToken->eType == AT65_Identifier) && (psToken->uiLength == 1) && memchr(gacAsm816Flags, psToken->szText[0], 8))
	{
		Next();
		*puiBits = (uint8)(1 << ((const char*)memchr(gacAsm816Flags, psToken->szText[0], 8) - gacAsm816Flags));
		return true;
	}

	if (!Expression(&sValue))
	{
		return false;
	}
	if (IsRelocatable(&sValue))
	{
		return Error(psToken, "rep and sep take flags or a constant");
	}
	if (!sValue.bResolved)
	{
		//A constant defined later: the widths are unknown until it is.
		muiWidthA = ASM816_WIDTH_UNKNOWN;
		muiWidthX = ASM816_WIDTH_UNKNOWN;
		*puiBits = 0;
		return true;
	}
	if ((sValue.iValue < 0) || (sValue.iValue > 0xFF))
	{
		return Error(psToken, "Value does not fit 8 bits");
	}
	*puiBits = (uint8)sValue.iValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// rep (bSet false) or sep (bSet true) of uiBits changes m and x.
//
//////////////////////////////////////////////////////////////////////////
void CAsm816Assembler::ChangeWidths(bool bSet, uint8 uiBits)
{
	if (uiBits & 0x20)
	{
		muiWidthA = bSet ? 8 : 16;
	}
	if (uiBits & 0x10)
	{
		muiWidthX = bSet ? 8 : 16;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A :8, :16 or :24 suffix (bAllow24 for addresses).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Assembler::Bits816(uint8* puiBits, bool bAllow24)
{
	SAsm65Token*	psToken;

	*puiBits = 0;
	if (!Accept(AP65_Colon))
	{
		return true;
	}
	psToken = Next();
	if ((psToken->eType != AT65_Number) || ((psToken->iValue != 8) && (psToken->iValue != 16) && (psToken->iValue != 24)))
	{
		return ErrorExpected(psToken, bAllow24 ? "24" : ":8 or :16");
	}
	*puiBits = (uint8)psToken->iValue;
	if (Peek()->Is(AP65_Colon))
	{
		return Error(Peek(), "Only one suffix is allowed");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// + X or + Y after an address.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Assembler::Index816(char* pcIndex, bool bAllowY)
{
	SAsm65Token*	psToken;

	*pcIndex = 0;
	if (!Peek()->Is(AP65_Plus) || !IsRegisterName(Peek(1)))
	{
		return true;
	}
	Next();
	psToken = Next();
	if (psToken->IsIdentifier("X") || (bAllowY && psToken->IsIdentifier("Y")))
	{
		*pcIndex = psToken->szText[0];
		return true;
	}
	return ErrorExpected(psToken, bAllowY ? "X or Y" : "X");
}


//////////////////////////////////////////////////////////////////////////
//
// What is inside the brackets: D + value, S + value or value, then + X or + Y.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Assembler::Inner816(SAsm816Operand* psOperand)
{
	SAsm65Token*	psToken;

	psToken = Peek();
	if (psToken->IsIdentifier("D") || psToken->IsIdentifier("S"))
	{
		Next();
		psOperand->eBase = psToken->IsIdentifier("D") ? A816B_Direct : A816B_Stack;
		if (Peek()->Is(AP65_Plus) && !IsRegisterName(Peek(1)))
		{
			Next();
			if (!Expression(&psOperand->sValue))
			{
				return false;
			}
		}
		else
		{
			psOperand->sValue.iValue = 0;
			psOperand->sValue.bResolved = true;
			psOperand->sValue.iTerms = 0;
			psOperand->sValue.iProgramCounter = 0;
			psOperand->sValue.uiForm = ASM65RF_Plain;
		}
	}
	else if (IsRegisterName(psToken))
	{
		return Error(psToken, "An address is D + value, S + value or a value, then + X or + Y");
	}
	else
	{
		psOperand->eBase = A816B_Value;
		if (!Expression(&psOperand->sValue))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A, value{:bits}, [inner]{:24} or [[inner]{:24} {+ Y}].
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Assembler::Operand816(SAsm816Operand* psOperand)
{
	SAsm65Token*	psToken;
	char			cIndex;
	uint8			uiBits;

	memset(psOperand, 0, sizeof(SAsm816Operand));
	psToken = Peek();
	psOperand->psToken = psToken;

	if (psToken->IsIdentifier("A") && Peek(1)->IsEndOfStatement())
	{
		Next();
		psOperand->bAccumulator = true;
		return true;
	}

	if (!Accept(AP65_LeftSquare))
	{
		if (!Expression(&psOperand->sValue))
		{
			return false;
		}
		return Bits816(&psOperand->uiValueBits, true);
	}

	psOperand->bMemory = true;
	if (Accept(AP65_LeftSquare))
	{
		psOperand->bPointer = true;
		if (!Inner816(psOperand) || !Index816(&psOperand->cInnerIndex, false))
		{
			return false;
		}
		if (!Expect(AP65_RightSquare, "']'"))
		{
			return false;
		}
		if (!Bits816(&uiBits, true))
		{
			return false;
		}
		if ((uiBits != 0) && (uiBits != 24))
		{
			return Error(psToken, "A pointer is 16 bits, or 24 with :24");
		}
		psOperand->bLongPointer = uiBits == 24;
		if (!Index816(&psOperand->cIndex, true))
		{
			return false;
		}
		if (psOperand->cIndex == 'X')
		{
			return Error(psToken, "X indexes inside the pointer's brackets: [[D + value + X]]");
		}
	}
	else
	{
		if (!Inner816(psOperand) || !Index816(&cIndex, true))
		{
			return false;
		}
		psOperand->cIndex = cIndex;
	}
	if (!Expect(AP65_RightSquare, "']'"))
	{
		return false;
	}
	if (!Bits816(&uiBits, true))
	{
		return false;
	}
	if ((uiBits != 0) && (uiBits != 24))
	{
		return Error(psToken, "An address is 16 bits, or 24 with :24");
	}
	psOperand->bLong = uiBits == 24;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm816Assembler::AddByte(SAsm816Instruction* psInstruction, uint8 uiByte)
{
	psInstruction->auiBytes[psInstruction->uiBytes] = uiByte;
	psInstruction->uiBytes++;
}


//////////////////////////////////////////////////////////////////////////
//
// A value of uiBytes, or a relocation of uiForm for the linker (a plain value takes
// uiForm, a value written >> 16 or & $FFFF keeps its own).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Assembler::AddField(SAsm816Instruction* psInstruction, SAsm65Value* psValue, uint8 uiBytes, bool bSigned, uint8 uiForm, SAsm65Token* psToken)
{
	SAsm816Relocation*	psRelocation;
	uint8				i;

	if (IsRelocatable(psValue))
	{
		UseTerm();
		psRelocation = &psInstruction->asRelocations[psInstruction->uiRelocations];
		psInstruction->uiRelocations++;
		psRelocation->sValue = *psValue;
		if (psValue->uiForm == ASM65RF_Plain)
		{
			psRelocation->sValue.uiForm = uiForm;
		}
		psRelocation->uiOffset = psInstruction->uiBytes;
		psRelocation->uiBytes = uiBytes;
		psRelocation->bSigned = bSigned;
		psRelocation->bRelative = false;
		psRelocation->psToken = psToken;
		for (i = 0; i < uiBytes; i++)
		{
			AddByte(psInstruction, 0);
		}
		return true;
	}

	if (psValue->bResolved && !Asm65Fits(psValue->iValue, uiBytes, false) && !Asm65Fits(psValue->iValue, uiBytes, true))
	{
		return Error(psToken, "Value does not fit");
	}
	for (i = 0; i < uiBytes; i++)
	{
		AddByte(psInstruction, (uint8)(psValue->iValue >> (8 * i)));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A branch offset of uiBytes from the end of an instruction uiLength bytes long: to a
// label, C + offset, or a raw offset written without a label.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Assembler::AddRelative(SAsm816Instruction* psInstruction, SAsm65Value* psValue, uint8 uiBytes, uint8 uiLength, SAsm65Token* psToken)
{
	SAsm816Relocation*	psRelocation;
	SAsm65Value			sOffset;
	uint8				i;

	if (!RelativeTarget(psValue, psToken))
	{
		return false;
	}
	if (TargetIsRelocatable(psValue))
	{
		if (psValue->uiForm != ASM65RF_Plain)
		{
			return Error(psToken, "A branch target is an address");
		}
		UseTerm();
		psRelocation = &psInstruction->asRelocations[psInstruction->uiRelocations];
		psInstruction->uiRelocations++;
		psRelocation->sValue = *psValue;
		psRelocation->uiOffset = psInstruction->uiBytes;
		psRelocation->uiBytes = uiBytes;
		psRelocation->bSigned = true;
		psRelocation->bRelative = true;
		psRelocation->psToken = psToken;
		for (i = 0; i < uiBytes; i++)
		{
			AddByte(psInstruction, 0);
		}
		return true;
	}

	sOffset = *psValue;
	if ((psValue->iProgramCounter == 0) && psValue->bLabel)
	{
		sOffset.iValue = psValue->iValue - (int64)(muiStatementAddress + uiLength);
	}
	if (sOffset.bResolved && !Asm65Fits(sOffset.iValue, uiBytes, true))
	{
		if (uiBytes == 2)
		{
			//brl and per wrap around within the bank.
			sOffset.iValue = (int16)(uint16)sOffset.iValue;
		}
		else
		{
			return Error(psToken, "Target is out of range");
		}
	}
	for (i = 0; i < uiBytes; i++)
	{
		AddByte(psInstruction, (uint8)(sOffset.iValue >> (8 * i)));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An instruction's length can only grow from pass to pass (see AbsoluteOrLong).
//
//////////////////////////////////////////////////////////////////////////
void CAsm816Assembler::Emit816(SAsm816Instruction* psInstruction)
{
	uint8				i;
	SAsm816Relocation*	psRelocation;

	if (muiInstruction >= mcMinimumWords.NumElements())
	{
		mcMinimumWords.Add(psInstruction->uiBytes);
		mbChanged = true;
	}
	else if (psInstruction->uiBytes > mcMinimumWords.GetValue(muiInstruction))
	{
		mcMinimumWords.SetValue(muiInstruction, psInstruction->uiBytes);
		mbChanged = true;
	}

	for (i = 0; i < psInstruction->uiRelocations; i++)
	{
		psRelocation = &psInstruction->asRelocations[i];
		RecordRelocation(&psRelocation->sValue, muiAddress + psRelocation->uiOffset, psRelocation->uiBytes, psRelocation->bSigned, psRelocation->bRelative, psInstruction->uiBytes - psRelocation->uiOffset, psRelocation->psToken);
	}

	for (i = 0; i < psInstruction->uiBytes; i++)
	{
		WriteByte(psInstruction->auiBytes[i]);
	}
	muiInstruction++;
}

