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
#include <stdlib.h>
#include "BaseLib/StringHelper.h"
#include "IRParser.h"


#define IR_MAX_NUMBERED		(1 << 20)		//Numbered temporaries and blocks above this are not accepted.


//////////////////////////////////////////////////////////////////////////
//
// szFile is only used in error messages and may be NULL.
//
//////////////////////////////////////////////////////////////////////////
void CIRParser::Init(CIRModule* pcModule, CCPPErrors* pcErrors, const char* szFile)
{
	mpcModule = pcModule;
	mpcErrors = pcErrors;
	mszFile = szFile;
	mszText = NULL;
	muiPosition = 0;
	mbError = false;
	mszName.Init();
	mszNumber.Init();
	mszResult.Init();

	mpcFunction = NULL;
	mpcBlock = NULL;
	mauiNumberedTemporaries.Init();
	mapcNumberedBlocks.Init();
	mauiTemporaryUses.Init();
	mauiBlockUses.Init();
	macLabelled.Init();
	mapcLabelOrder.Init();
	muiResultPosition = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRParser::Kill(void)
{
	mapcLabelOrder.Kill();
	macLabelled.Kill();
	mauiBlockUses.Kill();
	mauiTemporaryUses.Kill();
	mapcNumberedBlocks.Kill();
	mauiNumberedTemporaries.Kill();
	mszResult.Kill();
	mszNumber.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds szText's spaces, globals and functions to the module.  False (with
// the error in the errors) if the text is not IR.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::Parse(const char* szText)
{
	size	uiStart;
	bool	bResult;

	mszText = szText;
	muiPosition = 0;
	mbError = false;

	for (;;)
	{
		SkipSpace();
		if (AtEnd())
		{
			return true;
		}

		uiStart = muiPosition;
		if (GetKeyword("space"))
		{
			bResult = ParseSpace();
		}
		else if (GetKeyword("global"))
		{
			bResult = ParseGlobal();
		}
		else if (GetKeyword("function"))
		{
			bResult = ParseFunction();
		}
		else if (GetKeyword("restrict"))
		{
			bResult = ParseRestrict();
		}
		else
		{
			bResult = ErrorAt(uiStart, "Expected space, global, function or restrict.");
		}
		if (!bResult)
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Reports the first error only, at the current position, with the name (and
// its % or @) or the text up to a space there.  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::Error(const char* szMessage)
{
	size	ui;
	int		iLine;
	int		iColumn;
	CChars	szDetail;

	if (mbError)
	{
		return false;
	}
	mbError = true;

	iLine = 0;
	iColumn = 0;
	for (ui = 0; ui < muiPosition; ui++)
	{
		if (mszText[ui] == '\n')
		{
			iLine++;
			iColumn = 0;
		}
		else
		{
			iColumn++;
		}
	}

	szDetail.Init();
	ui = muiPosition;
	if ((mszText[ui] == '%') || (mszText[ui] == '@') || IsNameCharacter(mszText[ui]))
	{
		do
		{
			szDetail.Append(mszText[ui]);
			ui++;
		} while (IsNameCharacter(mszText[ui]) && (ui < muiPosition + 32));
	}
	else
	{
		for (; mszText[ui] && (mszText[ui] != '\n') && (mszText[ui] != ' ') && (mszText[ui] != '\t') && (ui < muiPosition + 32); ui++)
		{
			szDetail.Append(mszText[ui]);
		}
	}
	mpcErrors->Add(mszFile, iLine, iColumn, -1, szMessage, szDetail.Empty() ? NULL : szDetail.Text());
	szDetail.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ErrorAt(size uiPosition, const char* szMessage)
{
	muiPosition = uiPosition;
	SkipSpace();
	return Error(szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// Spaces, new lines and ; comments.
//
//////////////////////////////////////////////////////////////////////////
void CIRParser::SkipSpace(void)
{
	char	c;

	for (;;)
	{
		c = mszText[muiPosition];
		if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
		{
			muiPosition++;
		}
		else if (c == ';')
		{
			while (mszText[muiPosition] && (mszText[muiPosition] != '\n'))
			{
				muiPosition++;
			}
		}
		else
		{
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::AtEnd(void)
{
	return mszText[muiPosition] == '\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CIRParser::Peek(void)
{
	SkipSpace();
	return mszText[muiPosition];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::GetChar(char c)
{
	if (Peek() == c)
	{
		muiPosition++;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ExpectChar(char c)
{
	CChars	szMessage;

	if (GetChar(c))
	{
		return true;
	}
	szMessage.Init("Expected '");
	szMessage.Append(c);
	szMessage.Append("'.");
	Error(szMessage.Text());
	szMessage.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::IsNameCharacter(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_') || (c == '.') || (c == '$');
}


//////////////////////////////////////////////////////////////////////////
//
// Letters, digits, _, . and $ into mszName.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::GetName(void)
{
	size	uiStart;

	SkipSpace();
	uiStart = muiPosition;
	while (IsNameCharacter(mszText[muiPosition]))
	{
		muiPosition++;
	}
	mszName.Clear();
	if (muiPosition == uiStart)
	{
		return false;
	}
	mszName.AppendSubString(&mszText[uiStart], &mszText[muiPosition]);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ExpectName(const char* szError)
{
	return GetName() || Error(szError);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves past szKeyword if it is the next name.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::GetKeyword(const char* szKeyword)
{
	size	uiLength;

	SkipSpace();
	for (uiLength = 0; szKeyword[uiLength]; uiLength++)
	{
		if (mszText[muiPosition + uiLength] != szKeyword[uiLength])
		{
			return false;
		}
	}
	if (IsNameCharacter(mszText[muiPosition + uiLength]))
	{
		return false;
	}
	muiPosition += uiLength;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// ... if it is next.  False only for a . that doesn't start ....
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::GetEllipsis(bool* pbFound)
{
	*pbFound = false;
	if (GetChar('.'))
	{
		if (!ExpectChar('.') || !ExpectChar('.'))
		{
			return false;
		}
		*pbFound = true;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A number's text into mszNumber: an optional - then digits, letters, . and
// + or - after an e that is not hexadecimal.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::GetNumber(void)
{
	size	uiStart;
	char	c;
	bool	bHex;

	SkipSpace();
	uiStart = muiPosition;
	if (mszText[muiPosition] == '-')
	{
		muiPosition++;
	}
	if ((mszText[muiPosition] < '0') || (mszText[muiPosition] > '9'))
	{
		muiPosition = uiStart;
		return false;
	}
	bHex = (mszText[muiPosition] == '0') && (ToLower(mszText[muiPosition + 1]) == 'x');
	for (;;)
	{
		c = mszText[muiPosition];
		if (IsNameCharacter(c))
		{
			muiPosition++;
		}
		else if (!bHex && ((c == '+') || (c == '-')) && (ToLower(mszText[muiPosition - 1]) == 'e'))
		{
			muiPosition++;
		}
		else
		{
			break;
		}
	}
	mszNumber.Clear();
	mszNumber.AppendSubString(&mszText[uiStart], &mszText[muiPosition]);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Decimal or 0x hexadecimal, with an optional -.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseInteger(const char* szNumber, uint64* pulliValue)
{
	bool	bNegative;
	uint64	ulliValue;
	uint64	ulliBase;
	size	uiDigit;

	bNegative = (*szNumber == '-');
	if (bNegative)
	{
		szNumber++;
	}
	ulliBase = 10;
	if ((szNumber[0] == '0') && (ToLower(szNumber[1]) == 'x'))
	{
		ulliBase = 16;
		szNumber += 2;
	}
	if (*szNumber == '\0')
	{
		return false;
	}

	ulliValue = 0;
	for (; *szNumber; szNumber++)
	{
		uiDigit = GetCharHex(ToLower(*szNumber));
		if (uiDigit >= ulliBase)
		{
			return false;
		}
		ulliValue = ulliValue * ulliBase + uiDigit;
	}
	*pulliValue = bNegative ? (uint64)(-(int64)ulliValue) : ulliValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::GetInteger(uint64* pulliValue, const char* szError)
{
	return (GetNumber() && ParseInteger(mszNumber.Text(), pulliValue)) || Error(szError);
}


//////////////////////////////////////////////////////////////////////////
//
// A constant of sType: an integer, or a float as its bits (0x...) or in
// decimal.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseBits(SIRType sType, uint64* pulliBits)
{
	double	f;
	float	f32;
	uint32	uiBits;
	char*	szNumber;

	if (!GetNumber())
	{
		return Error("Expected a number.");
	}
	szNumber = mszNumber.Text();
	if (sType.IsFloat() && !((szNumber[0] == '0') && (ToLower(szNumber[1]) == 'x')))
	{
		f = strtod(szNumber, NULL);
		if (sType.Is(IRTK_F32))
		{
			f32 = (float)f;
			memcpy(&uiBits, &f32, sizeof(uint32));
			*pulliBits = uiBits;
		}
		else
		{
			memcpy(pulliBits, &f, sizeof(uint64));
		}
		return true;
	}
	return ParseInteger(szNumber, pulliBits) || Error("Bad number.");
}


//////////////////////////////////////////////////////////////////////////
//
// "..." with \HH escapes.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::GetString(CArrayTemplate<uint8>* pacBytes)
{
	char	c;
	uint8	uc;
	size	uiHigh;
	size	uiLow;

	if (!ExpectChar('"'))
	{
		return false;
	}
	for (;;)
	{
		c = mszText[muiPosition];
		if ((c == '\0') || (c == '\n'))
		{
			return Error("Unterminated string.");
		}
		muiPosition++;
		if (c == '"')
		{
			return true;
		}
		if (c == '\\')
		{
			uiHigh = GetCharHex(ToLower(mszText[muiPosition]));
			uiLow = (uiHigh < 16) ? GetCharHex(ToLower(mszText[muiPosition + 1])) : 16;
			if ((uiHigh >= 16) || (uiLow >= 16))
			{
				muiPosition--;
				return Error("Expected two hexadecimal digits after \\.");
			}
			muiPosition += 2;
			uc = (uint8)((uiHigh << 4) | uiLow);
		}
		else
		{
			uc = (uint8)c;
		}
		pacBytes->Add(&uc);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// szName as a type: void (if allowed), i1 ... f64, ptr or ptr.space.  A
// module without spaces gets "default", 4 bytes, when ptr is first used.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::MatchType(const char* szName, SIRType* psType, bool bAllowVoid)
{
	EIRTypeKind	eKind;
	int			iSpace;

	if (bAllowVoid && (StringCompare(szName, "void") == 0))
	{
		*psType = IRType(IRTK_None);
		return true;
	}
	eKind = FindIRTypeKind(szName);
	if (eKind == IRTK_Pointer)
	{
		if (mpcModule->NumSpaces() == 0)
		{
			mpcModule->AddSpace("default", 4);
		}
		*psType = IRType(IRTK_Pointer);
		return true;
	}
	if (eKind != IRTK_None)
	{
		*psType = IRType(eKind);
		return true;
	}
	if ((StrLen(szName) > 4) && (memcmp(szName, "ptr.", 4) == 0))
	{
		iSpace = mpcModule->FindSpace(&szName[4]);
		if (iSpace >= 0)
		{
			*psType = IRType(IRTK_Pointer, (uint8)iSpace);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseType(SIRType* psType, bool bAllowVoid)
{
	size	uiStart;

	SkipSpace();
	uiStart = muiPosition;
	if (GetName() && MatchType(mszName.Text(), psType, bAllowVoid))
	{
		return true;
	}
	return ErrorAt(uiStart, mszName.StartsWith("ptr.") ? "Unknown address space." : "Expected a type.");
}


//////////////////////////////////////////////////////////////////////////
//
// {, align n}{, attribute} ... in any order.  puiAlign is NULL for
// functions (they have no align).
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseAttributes(uint32* puiAttributes, uint32* puiAlign)
{
	uint64	ulliAlign;
	size	ui;

	*puiAttributes = 0;
	while (GetChar(','))
	{
		if (puiAlign && GetKeyword("align"))
		{
			if (!GetInteger(&ulliAlign, "Expected the alignment."))
			{
				return false;
			}
			*puiAlign = (uint32)ulliAlign;
			continue;
		}
		for (ui = 0; ui < IRA_NumWordAttributes; ui++)
		{
			if (GetKeyword(gasIRAttributeNames[ui].szName))
			{
				*puiAttributes |= gasIRAttributeNames[ui].uiAttribute;
				break;
			}
		}
		if (ui == IRA_NumWordAttributes)
		{
			return Error("Expected an attribute.");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// @name of a function or global not yet defined.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseNewSymbol(uint32* puiSymbol)
{
	size	uiStart;

	SkipSpace();
	uiStart = muiPosition;
	if (!ExpectChar('@') || !ExpectName("Expected a symbol's name."))
	{
		return false;
	}
	*puiSymbol = mpcModule->AddSymbol(mszName.Text());
	if (mpcModule->IsDefined(*puiSymbol))
	{
		return ErrorAt(uiStart, "Symbol defined twice.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// space name bytes
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseSpace(void)
{
	uint64	ulliBytes;
	size	uiStart;

	SkipSpace();
	uiStart = muiPosition;
	if (!ExpectName("Expected the space's name."))
	{
		return false;
	}
	if (mpcModule->FindSpace(mszName.Text()) >= 0)
	{
		return ErrorAt(uiStart, "Address space defined twice.");
	}
	if (!GetInteger(&ulliBytes, "Expected the size of a pointer."))
	{
		return false;
	}
	mpcModule->AddSpace(mszName.Text(), (int)ulliBytes);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// restrict @name: the symbol is a restrict pointer variable.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseRestrict(void)
{
	SkipSpace();
	if (!ExpectChar('@') || !ExpectName("Expected a symbol's name."))
	{
		return false;
	}
	mpcModule->SetRestrict(mpcModule->AddSymbol(mszName.Text()));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// global @name{, attributes} { items }
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseGlobal(void)
{
	CIRGlobal*	pcGlobal;
	uint32		uiSymbol;
	uint32		uiAttributes;
	uint32		uiAlign;

	if (!ParseNewSymbol(&uiSymbol))
	{
		return false;
	}
	uiAlign = 1;
	if (!ParseAttributes(&uiAttributes, &uiAlign))
	{
		return false;
	}
	pcGlobal = mpcModule->AddGlobal(mpcModule->GetSymbolName(uiSymbol), uiAlign, uiAttributes);

	if (!ExpectChar('{'))
	{
		return false;
	}
	if (GetChar('}'))
	{
		return true;
	}
	do
	{
		if (!ParseGlobalItem(pcGlobal))
		{
			return false;
		}
	} while (GetChar(','));
	return ExpectChar('}');
}


//////////////////////////////////////////////////////////////////////////
//
// bytes "...", zero n, T value or ptr @symbol{+-offset}.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseGlobalItem(CIRGlobal* pcGlobal)
{
	CArrayTemplate<uint8>	acBytes;
	SIRType					sType;
	uint64					ulliBits;
	uint32					uiSymbol;
	bool					bResult;

	if (GetKeyword("bytes"))
	{
		acBytes.Init();
		bResult = GetString(&acBytes);
		if (bResult)
		{
			pcGlobal->AddBytes(acBytes.GetData(), acBytes.NumElements());
		}
		acBytes.Kill();
		return bResult;
	}
	if (GetKeyword("zero"))
	{
		if (!GetInteger(&ulliBits, "Expected the number of zero bytes."))
		{
			return false;
		}
		pcGlobal->AddZero((size)ulliBits);
		return true;
	}

	if (!ParseType(&sType, false))
	{
		return false;
	}
	if (sType.Is(IRTK_I1))
	{
		return Error("i1 can't be in memory.");
	}
	if (sType.IsPointer() && GetChar('@'))
	{
		if (!ExpectName("Expected a symbol."))
		{
			return false;
		}
		uiSymbol = mpcModule->AddSymbol(mszName.Text());
		ulliBits = 0;
		if (((Peek() == '-') || GetChar('+')) && !GetInteger(&ulliBits, "Expected an offset."))
		{
			return false;
		}
		pcGlobal->AddSymbol(sType, uiSymbol, (int64)ulliBits);
		return true;
	}
	if (!ParseBits(sType, &ulliBits))
	{
		return false;
	}
	pcGlobal->AddValue(sType, ulliBits);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// function T @name(T {restrict} %p, ...){, attributes} { blocks }
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseFunction(void)
{
	SIRType			sReturnType;
	SIRType			sType;
	uint32			uiSymbol;
	uint32			uiAttributes;
	bool			bVariadic;
	bool			bRestrict;
	SIRReference	sParameter;
	bool			bResult;

	if (!ParseType(&sReturnType, true) || !ParseNewSymbol(&uiSymbol))
	{
		return false;
	}
	mpcFunction = mpcModule->AddFunction(mpcModule->GetSymbolName(uiSymbol), sReturnType);
	mpcBlock = NULL;
	mauiNumberedTemporaries.FakeSetUsedElements(0);
	mapcNumberedBlocks.FakeSetUsedElements(0);
	mauiTemporaryUses.FakeSetUsedElements(0);
	mauiBlockUses.FakeSetUsedElements(0);
	macLabelled.FakeSetUsedElements(0);
	mapcLabelOrder.FakeSetUsedElements(0);

	bVariadic = false;
	if (!ExpectChar('('))
	{
		return false;
	}
	if (!GetChar(')'))
	{
		do
		{
			if (!GetEllipsis(&bVariadic))
			{
				return false;
			}
			if (bVariadic)
			{
				break;
			}
			if (!ParseType(&sType, false))
			{
				return false;
			}
			bRestrict = GetKeyword("restrict");
			SkipSpace();
			muiResultPosition = muiPosition;
			if (!ExpectChar('%') || !ExpectName("Expected a parameter."))
			{
				return false;
			}
			mszResult.Clear();
			mszResult.Append(mszName);
			if (!DefineTemporary(sType, &sParameter))
			{
				return false;
			}
			mpcFunction->masParameters.Add(&sParameter);
			if (bRestrict)
			{
				mpcFunction->SetRestrictParameter(mpcFunction->masParameters.NumElements() - 1);
			}
		} while (GetChar(','));
		if (!ExpectChar(')'))
		{
			return false;
		}
	}

	if (!ParseAttributes(&uiAttributes, NULL))
	{
		return false;
	}
	mpcFunction->muiAttributes = uiAttributes | (bVariadic ? IRA_Variadic : 0);

	bResult = ExpectChar('{') && ParseBody() && FinishFunction();
	mpcFunction = NULL;
	mpcBlock = NULL;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Labels and statements up to the closing }.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseBody(void)
{
	size		uiStart;
	CIRBlock*	pcBlock;
	uint8		uiOne;

	uiOne = 1;
	for (;;)
	{
		if (GetChar('}'))
		{
			return true;
		}
		if (AtEnd())
		{
			return Error("Expected '}'.");
		}

		SkipSpace();
		uiStart = muiPosition;
		if ((Peek() != '%') && GetName() && GetChar(':'))
		{
			pcBlock = Block(mszName.Text(), uiStart);
			if (*macLabelled.Get(pcBlock->muiIndex))
			{
				return ErrorAt(uiStart, "Block defined twice.");
			}
			macLabelled.Set(pcBlock->muiIndex, &uiOne);
			mapcLabelOrder.Add(pcBlock);
			mpcBlock = pcBlock;
			continue;
		}
		muiPosition = uiStart;

		if (mpcBlock == NULL)
		{
			return Error("Expected a block label.");
		}
		if (mpcBlock->HasJump())
		{
			return Error("Instruction after the block's jump.");
		}
		if (!ParseStatement())
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// {%r = }opcode ..., {%r = }call ..., %r = phi ... or a jump.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseStatement(void)
{
	bool		bResult;
	size		uiStart;
	uint16		uiOpcode;
	SIROpcode*	psOpcode;

	bResult = false;
	if (Peek() == '%')
	{
		muiResultPosition = muiPosition;
		GetChar('%');
		if (!ExpectName("Expected a temporary's name.") || !ExpectChar('='))
		{
			return false;
		}
		mszResult.Clear();
		mszResult.Append(mszName);
		bResult = true;
	}

	SkipSpace();
	uiStart = muiPosition;
	if (!GetName())
	{
		return Error("Expected an instruction.");
	}
	if (bResult && mszName.Equals("phi"))
	{
		return ParsePhi();
	}

	uiOpcode = mpcModule->FindOpcode(mszName.Text());
	psOpcode = mpcModule->GetOpcode(uiOpcode);
	if (psOpcode == NULL)
	{
		if (bResult)
		{
			return ErrorAt(uiStart, "Expected an opcode with a result.");
		}
		muiPosition = uiStart;
		return ParseJump();
	}
	if (psOpcode->uiShape == IRSH_Call)
	{
		return ParseCall(bResult);
	}
	if (bResult && (psOpcode->uiResult == IRRS_None))
	{
		return ErrorAt(uiStart, "Expected an opcode with a result.");
	}
	if (!bResult && (psOpcode->uiResult != IRRS_None))
	{
		return ErrorAt(uiStart, "The opcode needs a result.");
	}
	return ParseOperation(uiOpcode, psOpcode, bResult);
}


//////////////////////////////////////////////////////////////////////////
//
// The type a constant operand of kind uiKind has.
//
//////////////////////////////////////////////////////////////////////////
SIRType CIRParser::OperandType(uint8 uiKind, SIRType sType)
{
	switch (uiKind)
	{
	case IROK_Same:
		return sType;
	case IROK_Pointer:
		return IRType(IRTK_Pointer);
	case IROK_Offset:
		return IRIntegerType(mpcModule->GetPointerSize(sType.uiSpace));
	case IROK_Constant:
		return IRType(IRTK_I32);
	default:
		return IRType(IRTK_None);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// After the opcode: flags, the type and the operands the opcode's table
// entry says.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseOperation(uint16 uiOpcode, SIROpcode* psOpcode, bool bResult)
{
	uint8			uiFlags;
	SIRType			sType;
	SIRType			sSourceType;
	SIRReference	asArguments[2];
	SIRReference	sResult;
	SIRInstruction*	psInstruction;
	size			ui;
	int				iOperand;

	uiFlags = 0;
	for (ui = 0; ui < IRF_NumWordFlags; ui++)
	{
		if (GetKeyword(gasIRFlagNames[ui].szName))
		{
			uiFlags |= gasIRFlagNames[ui].uiFlag;
		}
	}

	asArguments[0] = IRNone();
	asArguments[1] = IRNone();
	if (!ParseType(&sType, false))
	{
		return false;
	}
	if (psOpcode->uiShape == IRSH_Conversion)
	{
		sSourceType = sType;
		if (!ParseValue(sSourceType, &asArguments[0]) || !(GetKeyword("to") || Error("Expected 'to'.")) || !ParseType(&sType, false))
		{
			return false;
		}
	}
	else
	{
		for (iOperand = 0; iOperand < psOpcode->NumOperands(); iOperand++)
		{
			if ((iOperand > 0) && !ExpectChar(','))
			{
				return false;
			}
			if (!ParseValue(OperandType(psOpcode->auiOperands[iOperand], sType), &asArguments[iOperand]))
			{
				return false;
			}
		}
	}

	sResult = IRNone();
	if (bResult && !DefineTemporary((psOpcode->uiResult == IRRS_I1) ? IRType(IRTK_I1) : sType, &sResult))
	{
		return false;
	}
	psInstruction = mpcBlock->AddInstruction(uiOpcode, sType, sResult, asArguments[0], asArguments[1]);
	psInstruction->uiFlags = uiFlags;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// %r = phi T [block, value], ...
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParsePhi(void)
{
	SIRType			sType;
	SIRReference	sResult;
	SIRReference	sValue;
	SIRPhi*			psPhi;
	CIRBlock*		pcBlock;

	if (mpcBlock->masInstructions.NumElements() != 0)
	{
		return ErrorAt(muiResultPosition, "Phi after the block's instructions.");
	}
	if (!ParseType(&sType, false) || !DefineTemporary(sType, &sResult))
	{
		return false;
	}
	psPhi = mpcBlock->AddPhi(sType, sResult);
	do
	{
		if (!ExpectChar('[') || !ParseBlockName(&pcBlock) || !ExpectChar(',') || !ParseValue(sType, &sValue) || !ExpectChar(']'))
		{
			return false;
		}
		mpcBlock->AddPhiArgument(psPhi, pcBlock, sValue);
	} while (GetChar(','));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// call T callee(T a, ..., T b) after "call".
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseCall(bool bResult)
{
	SIRType			sType;
	SIRType			sArgumentType;
	SIRReference	sCallee;
	SIRReference	sValue;
	SIRReference	sResult;
	int				iArguments;
	int				iNamed;
	bool			bEllipsis;
	bool			bThisCall;

	bThisCall = GetKeyword("thiscall");
	if (!ParseType(&sType, true) || !ParseValue(IRType(IRTK_Pointer), &sCallee) || !ExpectChar('('))
	{
		return false;
	}
	iArguments = 0;
	iNamed = -1;
	if (!GetChar(')'))
	{
		do
		{
			if (!GetEllipsis(&bEllipsis))
			{
				return false;
			}
			if (bEllipsis)
			{
				if (iNamed >= 0)
				{
					return Error("... twice.");
				}
				iNamed = iArguments;
				continue;
			}
			if (!ParseType(&sArgumentType, false) || !ParseValue(sArgumentType, &sValue))
			{
				return false;
			}
			mpcFunction->Argument(mpcBlock, sArgumentType, sValue);
			iArguments++;
		} while (GetChar(','));
		if (!ExpectChar(')'))
		{
			return false;
		}
	}

	sResult = IRNone();
	if (bResult)
	{
		if (sType.IsNone())
		{
			return ErrorAt(muiResultPosition, "A void call has no result.");
		}
		if (!DefineTemporary(sType, &sResult))
		{
			return false;
		}
	}
	mpcFunction->AddCall(mpcBlock, sType, sResult, sCallee, iNamed, bThisCall);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// jmp, brif, ret or unreachable.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseJump(void)
{
	CIRBlock*		pcThen;
	CIRBlock*		pcElse;
	SIRReference	sValue;
	SIRType			sType;
	size			uiStart;

	if (GetKeyword("jmp"))
	{
		if (!ParseBlockName(&pcThen))
		{
			return false;
		}
		mpcBlock->Jump(pcThen);
		return true;
	}
	if (GetKeyword("brif"))
	{
		if (!ParseValue(IRType(IRTK_I1), &sValue) || !ExpectChar(',') || !ParseBlockName(&pcThen) || !ExpectChar(',') || !ParseBlockName(&pcElse))
		{
			return false;
		}
		mpcBlock->Branch(sValue, pcThen, pcElse);
		return true;
	}
	if (GetKeyword("ret"))
	{
		SkipSpace();
		uiStart = muiPosition;
		if (GetName() && MatchType(mszName.Text(), &sType, false))
		{
			if (!ParseValue(sType, &sValue))
			{
				return false;
			}
			mpcBlock->Return(sType, sValue);
		}
		else
		{
			muiPosition = uiStart;
			mpcBlock->Return();
		}
		return true;
	}
	if (GetKeyword("unreachable"))
	{
		mpcBlock->Unreachable();
		return true;
	}
	return Error("Expected an instruction.");
}


//////////////////////////////////////////////////////////////////////////
//
// %temporary, @symbol or a constant of sType.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseValue(SIRType sType, SIRReference* psReference)
{
	uint64	ulliBits;
	size	uiStart;

	SkipSpace();
	uiStart = muiPosition;
	if (GetChar('%'))
	{
		if (!ExpectName("Expected a temporary's name."))
		{
			return false;
		}
		*psReference = Temporary(mszName.Text(), uiStart);
		return true;
	}
	if (GetChar('@'))
	{
		if (!ExpectName("Expected a symbol's name."))
		{
			return false;
		}
		*psReference = mpcModule->Symbol(mszName.Text());
		return true;
	}

	if (sType.IsNone())
	{
		return Error("Expected a value.");
	}
	if (!ParseBits(sType, &ulliBits))
	{
		return false;
	}
	*psReference = mpcFunction->AddConstant(sType, ulliBits);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A block named in a jump or a phi.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::ParseBlockName(CIRBlock** ppcBlock)
{
	size	uiStart;

	SkipSpace();
	uiStart = muiPosition;
	if (!ExpectName("Expected a block's name."))
	{
		return false;
	}
	*ppcBlock = Block(mszName.Text(), uiStart);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if szName is all digits (a numbered temporary or block).
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::IsNumber(const char* szName, uint32* puiNumber)
{
	uint64	ulliNumber;

	ulliNumber = 0;
	for (; *szName; szName++)
	{
		if ((*szName < '0') || (*szName > '9'))
		{
			return false;
		}
		ulliNumber = ulliNumber * 10 + (*szName - '0');
		if (ulliNumber >= IR_MAX_NUMBERED)
		{
			return false;
		}
	}
	*puiNumber = (uint32)ulliNumber;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRParser::SetPosition(CArrayTemplate<size>* pauiPositions, size uiIndex, size uiPosition)
{
	pauiPositions->GrowToAtLeastNumElements(uiIndex + 1, true, 0);
	pauiPositions->Set(uiIndex, &uiPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// The temporary szName (or number); added with no type if it is new, its
// first use at uiPosition.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRParser::Temporary(const char* szName, size uiPosition)
{
	SIRReference	sTemporary;
	uint32			uiNumber;
	uint32*			puiEntry;
	uint32			uiEntry;

	if (IsNumber(szName, &uiNumber))
	{
		mauiNumberedTemporaries.GrowToAtLeastNumElements(uiNumber + 1, true, 0);
		puiEntry = mauiNumberedTemporaries.Get(uiNumber);
		if (*puiEntry != 0)
		{
			return IRReference(IRRK_Temporary, *puiEntry - 1);
		}
		sTemporary = mpcFunction->AddTemporary(IRType(IRTK_None));
		uiEntry = sTemporary.Index() + 1;
		mauiNumberedTemporaries.Set(uiNumber, &uiEntry);
	}
	else
	{
		sTemporary = mpcFunction->FindTemporary(szName);
		if (!sTemporary.IsNone())
		{
			return sTemporary;
		}
		sTemporary = mpcFunction->AddTemporary(IRType(IRTK_None), szName);
	}
	SetPosition(&mauiTemporaryUses, sTemporary.Index(), uiPosition);
	return sTemporary;
}


//////////////////////////////////////////////////////////////////////////
//
// The block szName (or number); added if it is new, its first use at
// uiPosition.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRParser::Block(const char* szName, size uiPosition)
{
	CIRBlock*	pcBlock;
	uint32		uiNumber;
	CIRBlock**	ppcEntry;

	if (IsNumber(szName, &uiNumber))
	{
		mapcNumberedBlocks.GrowToAtLeastNumElements(uiNumber + 1, true, 0);
		ppcEntry = mapcNumberedBlocks.Get(uiNumber);
		if (*ppcEntry)
		{
			return *ppcEntry;
		}
		pcBlock = mpcFunction->AddBlock();
		*mapcNumberedBlocks.Get(uiNumber) = pcBlock;
	}
	else
	{
		pcBlock = mpcFunction->FindBlock(szName);
		if (pcBlock)
		{
			return pcBlock;
		}
		pcBlock = mpcFunction->AddBlock(szName);
	}
	SetPosition(&mauiBlockUses, pcBlock->muiIndex, uiPosition);
	macLabelled.GrowToAtLeastNumElements(pcBlock->muiIndex + 1, true, 0);
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
// The temporary mszResult (named at muiResultPosition), defined here with
// sType.  It may already have been used, but not defined.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::DefineTemporary(SIRType sType, SIRReference* psReference)
{
	*psReference = Temporary(mszResult.Text(), muiResultPosition);
	if (!mpcFunction->GetTemporaryType(*psReference).IsNone())
	{
		return ErrorAt(muiResultPosition, "Temporary defined twice.");
	}
	mpcFunction->SetTemporaryType(*psReference, sType);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Every temporary used is defined and every block named has a label.  The
// blocks are put in the order of their labels and the predecessors worked
// out.
//
//////////////////////////////////////////////////////////////////////////
bool CIRParser::FinishFunction(void)
{
	size	ui;

	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		if (mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)ui)).IsNone())
		{
			return ErrorAt(*mauiTemporaryUses.Get(ui), "Temporary is never defined.");
		}
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		if (!*macLabelled.Get(ui))
		{
			return ErrorAt(*mauiBlockUses.Get(ui), "Block has no label.");
		}
	}
	mpcFunction->SetBlockOrder(&mapcLabelOrder);
	mpcFunction->ComputePredecessors();
	return true;
}
