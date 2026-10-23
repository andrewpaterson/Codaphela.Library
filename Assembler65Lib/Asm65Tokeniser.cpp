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
#include "Asm65Tokeniser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsIdentifierStart(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_') || (c == '.');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsIdentifierPart(char c)
{
	return IsIdentifierStart(c) || ((c >= '0') && (c <= '9'));
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the digit's value or -1 if it is not a digit in the base.
//
//////////////////////////////////////////////////////////////////////////
static int DigitValue(char c, int iBase)
{
	int	iValue;

	if ((c >= '0') && (c <= '9'))
	{
		iValue = c - '0';
	}
	else if ((c >= 'a') && (c <= 'f'))
	{
		iValue = c - 'a' + 10;
	}
	else if ((c >= 'A') && (c <= 'F'))
	{
		iValue = c - 'A' + 10;
	}
	else
	{
		return -1;
	}
	return (iValue < iBase) ? iValue : -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Tokeniser::Init(CAsm65Errors* pcErrors)
{
	mcTokens.Init();
	mszStrings.Init();
	mpcErrors = pcErrors;
	mszPos = NULL;
	muiLine = 1;
	mszLineStart = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Tokeniser::Kill(void)
{
	mszStrings.Kill();
	mcTokens.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Tokeniser::Tokenise(const char* szSource)
{
	bool			bResult;
	char			c;
	SAsm65Token*	psToken;

	mszPos = szSource;
	mszLineStart = szSource;
	muiLine = 1;
	bResult = true;

	for (;;)
	{
		c = *mszPos;
		if (c == '\0')
		{
			break;
		}
		else if ((c == ' ') || (c == '\t') || (c == '\r'))
		{
			mszPos++;
		}
		else if (c == ';')
		{
			while ((*mszPos != '\0') && (*mszPos != '\n'))
			{
				mszPos++;
			}
		}
		else if (c == '\n')
		{
			AddToken(AT65_NewLine, mszPos);
			mszPos++;
			muiLine++;
			mszLineStart = mszPos;
		}
		else if (IsIdentifierStart(c))
		{
			psToken = AddToken(AT65_Identifier, mszPos);
			while (IsIdentifierPart(*mszPos))
			{
				mszPos++;
			}
			psToken->uiLength = (uint32)(mszPos - psToken->szText);
		}
		else if (((c >= '0') && (c <= '9')) || ((c == '$') && (DigitValue(mszPos[1], 16) >= 0)))
		{
			bResult &= Number();
		}
		else if (c == '\'')
		{
			bResult &= Character();
		}
		else if (c == '"')
		{
			bResult &= String();
		}
		else
		{
			bResult &= Punctuator();
		}
	}

	AddToken(AT65_End, mszPos);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Token* CAsm65Tokeniser::AddToken(EAsm65TokenType eType, const char* szStart)
{
	SAsm65Token*	psToken;

	psToken = mcTokens.Add();
	psToken->eType = eType;
	psToken->ePunctuator = AP65_None;
	psToken->uiLine = muiLine;
	psToken->uiColumn = (uint32)(szStart - mszLineStart) + 1;
	psToken->szText = szStart;
	psToken->uiLength = 0;
	psToken->iValue = 0;
	psToken->uiStringLength = 0;
	return psToken;
}


//////////////////////////////////////////////////////////////////////////
//
// Decimal, $hex, 0xhex and 0bbinary.  Values are 64 bits and wrap.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Tokeniser::Number(void)
{
	SAsm65Token*	psToken;
	int				iBase;
	int				iDigit;
	uint64			uiValue;
	bool			bDigits;

	psToken = AddToken(AT65_Number, mszPos);
	iBase = 10;
	if (*mszPos == '$')
	{
		iBase = 16;
		mszPos++;
	}
	else if ((mszPos[0] == '0') && ((mszPos[1] == 'x') || (mszPos[1] == 'X')))
	{
		iBase = 16;
		mszPos += 2;
	}
	else if ((mszPos[0] == '0') && ((mszPos[1] == 'b') || (mszPos[1] == 'B')) && (DigitValue(mszPos[2], 2) >= 0))
	{
		iBase = 2;
		mszPos += 2;
	}

	uiValue = 0;
	bDigits = false;
	for (;;)
	{
		if (*mszPos == '_')
		{
			mszPos++;
			continue;
		}
		iDigit = DigitValue(*mszPos, iBase);
		if (iDigit < 0)
		{
			break;
		}
		uiValue = uiValue * iBase + iDigit;
		bDigits = true;
		mszPos++;
	}

	psToken->iValue = (int64)uiValue;
	psToken->uiLength = (uint32)(mszPos - psToken->szText);
	if (!bDigits || IsIdentifierPart(*mszPos))
	{
		Error(psToken->szText, "Malformed number");
		while (IsIdentifierPart(*mszPos))
		{
			mszPos++;
		}
		psToken->uiLength = (uint32)(mszPos - psToken->szText);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Tokeniser::Escape(char* pc)
{
	char	c;

	c = *mszPos;
	if (c != '\\')
	{
		if ((c == '\0') || (c == '\n'))
		{
			return false;
		}
		*pc = c;
		mszPos++;
		return true;
	}

	mszPos++;
	switch (*mszPos)
	{
	case 'n':
		*pc = '\n';
		break;
	case 'r':
		*pc = '\r';
		break;
	case 't':
		*pc = '\t';
		break;
	case '0':
		*pc = '\0';
		break;
	case '\\':
	case '\'':
	case '"':
		*pc = *mszPos;
		break;
	default:
		return false;
	}
	mszPos++;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A character is a number.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Tokeniser::Character(void)
{
	SAsm65Token*	psToken;
	char			c;

	psToken = AddToken(AT65_Number, mszPos);
	mszPos++;
	if (!Escape(&c) || (*mszPos != '\''))
	{
		Error(psToken->szText, "Malformed character");
		while ((*mszPos != '\0') && (*mszPos != '\n') && (*mszPos != '\''))
		{
			mszPos++;
		}
		if (*mszPos == '\'')
		{
			mszPos++;
		}
		psToken->uiLength = (uint32)(mszPos - psToken->szText);
		return false;
	}
	mszPos++;
	psToken->iValue = (uint8)c;
	psToken->uiLength = (uint32)(mszPos - psToken->szText);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Tokeniser::String(void)
{
	SAsm65Token*	psToken;
	char			c;

	psToken = AddToken(AT65_String, mszPos);
	psToken->iValue = (int64)mszStrings.Length();
	mszPos++;
	while (*mszPos != '"')
	{
		if (!Escape(&c))
		{
			Error(psToken->szText, "Unterminated string");
			while ((*mszPos != '\0') && (*mszPos != '\n'))
			{
				mszPos++;
			}
			psToken->uiLength = (uint32)(mszPos - psToken->szText);
			return false;
		}
		mszStrings.Append(c);
		psToken->uiStringLength++;
	}
	mszPos++;
	mszStrings.Append('\0');
	psToken->uiLength = (uint32)(mszPos - psToken->szText);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Tokeniser::Punctuator(void)
{
	SAsm65Token*		psToken;
	EAsm65Punctuator	ePunctuator;
	uint32				uiLength;

	uiLength = 1;
	switch (*mszPos)
	{
	case ',':
		ePunctuator = AP65_Comma;
		break;
	case ':':
		ePunctuator = AP65_Colon;
		break;
	case '[':
		ePunctuator = AP65_LeftSquare;
		break;
	case ']':
		ePunctuator = AP65_RightSquare;
		break;
	case '(':
		ePunctuator = AP65_LeftRound;
		break;
	case ')':
		ePunctuator = AP65_RightRound;
		break;
	case '+':
		ePunctuator = AP65_Plus;
		break;
	case '-':
		ePunctuator = AP65_Minus;
		break;
	case '*':
		ePunctuator = AP65_Multiply;
		break;
	case '/':
		ePunctuator = AP65_Divide;
		break;
	case '%':
		ePunctuator = AP65_Modulus;
		break;
	case '&':
		ePunctuator = AP65_And;
		break;
	case '|':
		ePunctuator = AP65_Or;
		break;
	case '^':
		ePunctuator = AP65_Xor;
		break;
	case '~':
		ePunctuator = AP65_Not;
		break;
	case '!':
		ePunctuator = AP65_LogicalNot;
		break;
	case '=':
		ePunctuator = AP65_Equals;
		break;
	case '<':
	case '>':
		if (mszPos[1] != mszPos[0])
		{
			Error(mszPos, "Unknown character");
			mszPos++;
			return false;
		}
		ePunctuator = (*mszPos == '<') ? AP65_ShiftLeft : AP65_ShiftRight;
		uiLength = 2;
		break;
	default:
		Error(mszPos, "Unknown character");
		mszPos++;
		return false;
	}

	psToken = AddToken(AT65_Punctuator, mszPos);
	psToken->ePunctuator = ePunctuator;
	psToken->uiLength = uiLength;
	mszPos += uiLength;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Tokeniser::Error(const char* szStart, const char* szMessage)
{
	if (mpcErrors)
	{
		mpcErrors->Add(muiLine, (uint32)(szStart - mszLineStart) + 1, szMessage);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Tokeniser::NumTokens(void)
{
	return mcTokens.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Token* CAsm65Tokeniser::Get(size uiIndex)
{
	return mcTokens.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Tokeniser::GetString(SAsm65Token* psToken)
{
	return mszStrings.Text((size)psToken->iValue);
}


//////////////////////////////////////////////////////////////////////////
//
// One token per line: its type and text.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Tokeniser::Print(CChars* psz)
{
	static const char*	aszTypes[] = { "End", "NewLine", "Identifier", "Number", "String", "Punctuator" };
	size				i;
	SAsm65Token*		psToken;

	for (i = 0; i < mcTokens.NumElements(); i++)
	{
		psToken = mcTokens.Get(i);
		psz->Append(aszTypes[psToken->eType]);
		if (psToken->eType == AT65_Number)
		{
			psz->Append(' ');
			psz->Append(psToken->iValue);
		}
		else if (psToken->eType == AT65_String)
		{
			psz->Append(" \"");
			psz->Append(GetString(psToken));
			psz->Append('"');
		}
		else if ((psToken->eType == AT65_Identifier) || (psToken->eType == AT65_Punctuator))
		{
			psz->Append(' ');
			psz->Append(psToken->szText, psToken->uiLength);
		}
		psz->AppendNewLine();
	}
}
