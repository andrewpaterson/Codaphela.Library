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
#include "Asm65Assembler.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Init(void)
{
	mcErrors.Init();
	mcWarnings.Init();
	mcTokeniser.Init(&mcErrors);
	mcSymbols.Init();
	mcOutput.Init();
	mcMinimumWords.Init();
	mcPendingLabels.Init();

	muiToken = 0;
	muiAddress = 0;
	muiStatementAddress = 0;
	muiPass = 0;
	mbFinal = false;
	mbChanged = false;
	mbStatementError = false;
	mbRelative = false;
	muiInstruction = 0;
	muiPasses = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Kill(void)
{
	mcPendingLabels.Kill();
	mcMinimumWords.Kill();
	mcOutput.Kill();
	mcSymbols.Kill();
	mcTokeniser.Kill();
	mcWarnings.Kill();
	mcErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Passes run until no label moves and no instruction grows.  The final pass writes
// the output and reports errors.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Assemble(const char* szSource)
{
	if (!mcTokeniser.Tokenise(szSource))
	{
		return false;
	}

	mbFinal = false;
	for (muiPass = 1; muiPass <= ASM65_MAX_PASSES; muiPass++)
	{
		mbChanged = false;
		Pass();
		if (!mbChanged)
		{
			break;
		}
	}

	muiPass++;
	mbFinal = true;
	mbChanged = false;
	mcOutput.Clear();
	Pass();
	muiPasses = muiPass;

	if (mbChanged && (mcErrors.NumErrors() == 0))
	{
		mcErrors.Add(0, 0, "Labels did not settle");
	}
	return mcErrors.NumErrors() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Pass(void)
{
	muiToken = 0;
	muiAddress = 0;
	muiInstruction = 0;
	mcPendingLabels.Kill();
	mcPendingLabels.Init();

	while (Peek()->eType != AT65_End)
	{
		Statement();
	}
	muiStatementAddress = muiAddress;
	BindLabels();
}


//////////////////////////////////////////////////////////////////////////
//
// [label:]... [instruction | directive | name = expression] new line
//
// Labels are bound when the next statement starts (after an instruction aligns the
// address, a .org moves it or a .align pads it).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Statement(void)
{
	SAsm65Token*	psToken;
	bool			bResult;

	mbStatementError = false;
	mbRelative = false;
	while ((Peek()->eType == AT65_Identifier) && Peek(1)->Is(AP65_Colon))
	{
		psToken = Next();
		Next();
		if (IsReservedName(psToken))
		{
			Error(psToken, "Reserved name cannot be a label");
		}
		else
		{
			mcPendingLabels.Add((int)(psToken - mcTokeniser.Get(0)));
		}
	}

	psToken = Peek();
	muiStatementAddress = muiAddress;
	bResult = true;
	if (psToken->IsEndOfStatement())
	{
	}
	else if (psToken->eType != AT65_Identifier)
	{
		bResult = Error(psToken, "Expected an instruction, directive or label");
	}
	else if (Peek(1)->Is(AP65_Equals))
	{
		BindLabels();
		Next();
		Next();
		bResult = Constant(psToken);
	}
	else if (psToken->szText[0] == '.')
	{
		Next();
		if (psToken->IsIdentifierIgnoreCase(".org"))
		{
			bResult = Origin();
			BindLabels();
		}
		else if (psToken->IsIdentifierIgnoreCase(".align"))
		{
			bResult = Align();
			BindLabels();
		}
		else
		{
			BindLabels();
			if (psToken->IsIdentifierIgnoreCase(".byte"))
			{
				bResult = Data(1);
			}
			else if (psToken->IsIdentifierIgnoreCase(".word"))
			{
				bResult = Data(2);
			}
			else if (psToken->IsIdentifierIgnoreCase(".long"))
			{
				bResult = Data(4);
			}
			else if (psToken->IsIdentifierIgnoreCase(".quad"))
			{
				bResult = Data(8);
			}
			else
			{
				bResult = Error(psToken, "Unknown directive");
			}
		}
	}
	else
	{
		AlignTo(2);
		muiStatementAddress = muiAddress;
		BindLabels();
		Next();
		bResult = Instruction(psToken);
	}

	if (bResult)
	{
		ExpectEnd();
	}

	//Skip the rest of the line after an error.
	while (!Peek()->IsEndOfStatement())
	{
		Next();
	}
	if (Peek()->eType == AT65_NewLine)
	{
		Next();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::BindLabels(void)
{
	size			i;
	SAsm65Token*	psLabel;

	for (i = 0; i < mcPendingLabels.NumElements(); i++)
	{
		psLabel = mcTokeniser.Get(mcPendingLabels.GetValue(i));
		DefineSymbol(psLabel, muiAddress, true);
	}
	mcPendingLabels.Kill();
	mcPendingLabels.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// A symbol defined twice in one pass is an error.  A symbol whose value changed since
// the last pass needs another pass.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::DefineSymbol(SAsm65Token* psName, int64 iValue, bool bResolved)
{
	SAsm65Symbol*	psSymbol;

	psSymbol = mcSymbols.Put(psName->szText, psName->uiLength);
	if (psSymbol->uiPass == muiPass)
	{
		Error(psName, "Symbol is already defined");
		return;
	}
	if ((psSymbol->iValue != iValue) || (psSymbol->bResolved != bResolved))
	{
		mbChanged = true;
	}
	psSymbol->iValue = iValue;
	psSymbol->bResolved = bResolved;
	psSymbol->uiPass = muiPass;
}


//////////////////////////////////////////////////////////////////////////
//
// name = expression
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Constant(SAsm65Token* psName)
{
	SAsm65Value	sValue;

	if (IsReservedName(psName))
	{
		return Error(psName, "Reserved name cannot be a constant");
	}
	if (!Expression(&sValue))
	{
		return false;
	}
	DefineSymbol(psName, sValue.iValue, sValue.bResolved);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// .org address
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Origin(void)
{
	int64	iValue;

	if (!ExpressionValue(&iValue))
	{
		return false;
	}
	muiAddress = (uint32)iValue;
	muiStatementAddress = muiAddress;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// .align bytes (a power of two)
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Align(void)
{
	SAsm65Token*	psToken;
	int64			iValue;

	psToken = Peek();
	if (!ExpressionValue(&iValue))
	{
		return false;
	}
	if ((iValue <= 0) || (iValue > 0x10000) || ((iValue & (iValue - 1)) != 0))
	{
		return Error(psToken, "Alignment must be a power of two");
	}
	AlignTo((uint32)iValue);
	muiStatementAddress = muiAddress;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// .byte, .word, .long and .quad: values, little endian.  .byte also takes strings.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Data(uint8 uiBytes)
{
	SAsm65Token*	psToken;
	SAsm65Value		sValue;
	char*			szString;
	uint32			i;
	uint8			j;
	int64			iLimit;

	do
	{
		psToken = Peek();
		if (psToken->eType == AT65_String)
		{
			if (uiBytes != 1)
			{
				return Error(psToken, "Strings are only allowed in .byte");
			}
			Next();
			szString = mcTokeniser.GetString(psToken);
			for (i = 0; i < psToken->uiStringLength; i++)
			{
				WriteByte((uint8)szString[i]);
			}
		}
		else
		{
			if (!Expression(&sValue))
			{
				return false;
			}
			if (uiBytes < 8)
			{
				iLimit = (int64)1 << (8 * uiBytes);
				if ((sValue.iValue >= iLimit) || (sValue.iValue < -(iLimit / 2)))
				{
					return Error(psToken, "Value does not fit");
				}
			}
			for (j = 0; j < uiBytes; j++)
			{
				WriteByte((uint8)(sValue.iValue >> (8 * j)));
			}
		}
	}
	while (Accept(AP65_Comma));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Pads with zeros.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::AlignTo(uint32 uiAlignment)
{
	while ((muiAddress & (uiAlignment - 1)) != 0)
	{
		WriteByte(0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::WriteByte(uint8 uiByte)
{
	if (mbFinal)
	{
		mcOutput.Write(muiAddress, uiByte);
	}
	muiAddress++;
}


//////////////////////////////////////////////////////////////////////////
//
// An instruction's length can only grow from pass to pass.
//
//////////////////////////////////////////////////////////////////////////
uint8 CAsm65Assembler::GetMinimumWords(void)
{
	if (muiInstruction < mcMinimumWords.NumElements())
	{
		return mcMinimumWords.GetValue(muiInstruction);
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Emit(SAsm65Instruction* psInstruction)
{
	uint8	i;

	if (muiInstruction >= mcMinimumWords.NumElements())
	{
		mcMinimumWords.Add(psInstruction->uiWords);
		mbChanged = true;
	}
	else if (psInstruction->uiWords > mcMinimumWords.GetValue(muiInstruction))
	{
		mcMinimumWords.SetValue(muiInstruction, psInstruction->uiWords);
		mbChanged = true;
	}

	for (i = 0; i < psInstruction->uiWords; i++)
	{
		WriteByte((uint8)psInstruction->auiWords[i]);
		WriteByte((uint8)(psInstruction->auiWords[i] >> 8));
	}
	muiInstruction++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Token* CAsm65Assembler::Peek(size uiOffset)
{
	size	uiIndex;

	uiIndex = muiToken + uiOffset;
	if (uiIndex >= mcTokeniser.NumTokens())
	{
		uiIndex = mcTokeniser.NumTokens() - 1;
	}
	return mcTokeniser.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Never moves past the end of a statement.
//
//////////////////////////////////////////////////////////////////////////
SAsm65Token* CAsm65Assembler::Next(void)
{
	SAsm65Token*	psToken;

	psToken = Peek();
	if (psToken->eType != AT65_End)
	{
		muiToken++;
	}
	return psToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Accept(EAsm65Punctuator ePunctuator)
{
	if (Peek()->Is(ePunctuator))
	{
		Next();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Expect(EAsm65Punctuator ePunctuator, const char* szWhat)
{
	if (Accept(ePunctuator))
	{
		return true;
	}
	return ErrorExpected(Peek(), szWhat);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ExpectEnd(void)
{
	if (Peek()->IsEndOfStatement())
	{
		return true;
	}
	return Error(Peek(), "Expected the end of the line");
}


//////////////////////////////////////////////////////////////////////////
//
// Only the final pass reports errors; earlier passes may not know every value yet.
// One error per statement.  Always returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Error(SAsm65Token* psToken, const char* szMessage)
{
	if (mbFinal && !mbStatementError)
	{
		Report(&mcErrors, psToken, szMessage);
	}
	mbStatementError = true;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// "Expected what".  Always returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ErrorExpected(SAsm65Token* psToken, const char* szWhat)
{
	CChars	szMessage;
	bool	bResult;

	szMessage.Init("Expected ");
	szMessage.Append(szWhat);
	bResult = Error(psToken, szMessage.Text());
	szMessage.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Warnings are reported by the final pass and do not stop assembly.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Warning(SAsm65Token* psToken, const char* szMessage)
{
	if (mbFinal)
	{
		Report(&mcWarnings, psToken, szMessage);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Report(CAsm65Errors* pcReport, SAsm65Token* psToken, const char* szMessage)
{
	CChars	szText;

	if (psToken->eType == AT65_NewLine)
	{
		pcReport->Add(psToken->uiLine, psToken->uiColumn, szMessage, "end of line");
	}
	else if (psToken->eType == AT65_End)
	{
		pcReport->Add(psToken->uiLine, psToken->uiColumn, szMessage, "end of file");
	}
	else
	{
		szText.Init();
		szText.Append(psToken->szText, psToken->uiLength);
		pcReport->Add(psToken->uiLine, psToken->uiColumn, szMessage, szText.Text());
		szText.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Output* CAsm65Assembler::GetOutput(void)
{
	return &mcOutput;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Assembler::GetErrors(void)
{
	return mcErrors.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Assembler::NumErrors(void)
{
	return mcErrors.NumErrors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Assembler::GetWarnings(void)
{
	return mcWarnings.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Assembler::NumWarnings(void)
{
	return mcWarnings.NumErrors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::GetSymbol(const char* szName, int64* piValue)
{
	SAsm65Symbol*	psSymbol;

	psSymbol = mcSymbols.Get(szName, strlen(szName));
	if ((psSymbol == NULL) || !psSymbol->bResolved)
	{
		return false;
	}
	*piValue = psSymbol->iValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65Assembler::NumPasses(void)
{
	return muiPasses;
}
