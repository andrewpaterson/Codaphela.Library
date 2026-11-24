/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "CTInt.h"
#include "CTShort.h"
#include "CTLongLong.h"
#include "CTChar.h"
#include "CTBoolean.h"
#include "CTString.h"
#include "CTIdentifier.h"
#include "CTUserLiteral.h"
#include "CppTokeniser.h"
#include "CPP65IRFrontEnd.h"
#include "CPPSyntaxTree.h"
#include "CPPConstants.h"
#include "CPPSemanticPass.h"
#include "CPPTypeWriter.h"
#include "LiteralHelper.h"
#include "CPPUnit.h"
#include "CPPStrings.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPStrings::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPStrings::Kill(void)
{
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends a string's element, the low byte first.
//
//////////////////////////////////////////////////////////////////////////
static void AppendStringElement(CArrayUint8* pauiBytes, uint32 uiElement, int iElementBytes)
{
	int		i;

	for (i = 0; i < iElementBytes; i++)
	{
		pauiBytes->Add((uint8)(uiElement >> (8 * i)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A character of a wide string: UTF-32 in 4 byte elements, otherwise UTF-16,
// where one past 0xFFFF is a surrogate pair.
//
//////////////////////////////////////////////////////////////////////////
static void AppendWideCharacter(CArrayUint8* pauiBytes, uint32 uiCharacter, int iElementBytes)
{
	if ((iElementBytes == 2) && (uiCharacter > 0xFFFF))
	{
		AppendStringElement(pauiBytes, 0xD800 + ((uiCharacter - 0x10000) >> 10), 2);
		AppendStringElement(pauiBytes, 0xDC00 + ((uiCharacter - 0x10000) & 0x3FF), 2);
		return;
	}
	AppendStringElement(pauiBytes, uiCharacter, iElementBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the UTF-8 bytes of the character uiCharacter.
//
//////////////////////////////////////////////////////////////////////////
static void AppendUTF8(CArrayUint8* pauiBytes, uint32 uiCharacter)
{
	char	szUTF8[UTF8_MAX_BYTES];
	size	uiBytes;
	size	i;

	uiBytes = EncodeUTF8(uiCharacter, szUTF8);
	for (i = 0; i < uiBytes; i++)
	{
		pauiBytes->Add((uint8)szUTF8[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes of a narrow string literal, without the zero at the end.  Adjacent
// literals are joined.  MSVC's __FUNCTION__, __FUNCSIG__ and __FUNCDNAME__ can be
// parts; they are the function's name, its class and name, and its label.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPStrings::DecodeString(CCPPSyntax* pcString, CArrayUint8* pauiBytes)
{
	CCPPToken*	pcToken;
	int			iToken;
	int			iElementBytes;
	CChars		szName;
	char*		szIdentifier;
	int			iLength;
	size		i;

	iElementBytes = StringElementBytes(pcString);
	for (iToken = pcString->miFirstToken; iToken <= pcString->miLastToken; iToken++)
	{
		pcToken = mpcUnit->mpcTokeniser->Get(iToken);
		if (pcToken->IsString())
		{
			if (!DecodeStringToken(pcString, (CCTString*)pcToken, iElementBytes, pauiBytes))
			{
				return false;
			}
			continue;
		}

		if (pcToken->IsUserString())
		{
			//"text"_s's characters, given to its literal operator (GetLiteralCall).
			if (!DecodeStringToken(pcString, (CCTString*)((CCTUserLiteral*)pcToken)->mpcLiteral, iElementBytes, pauiBytes))
			{
				return false;
			}
			continue;
		}
		if (iElementBytes != 1)
		{
			return mpcUnit->mcDiagnostics.Error(pcString, "The function's name can't be part of a wide string literal");
		}
		szIdentifier = pcToken->IsIdentifier() ? mpcUnit->mpcTokeniser->GetIdentifier(((CCTIdentifier*)pcToken)->miIdentifier, &iLength) : NULL;
		szName.Init();
		if (szIdentifier && (((CCTIdentifier*)pcToken)->miIdentifier == mpcUnit->mpcTokeniser->mlliFuncDName))
		{
			if (mpcUnit->mcContext.GetFunction())
			{
				mpcUnit->mcMangler.AppendFunctionLabel(&szName, mpcUnit->mcContext.GetFunction());
			}
		}
		else if (!szIdentifier || !FunctionNameText(((CCTIdentifier*)pcToken)->miIdentifier, &szName))
		{
			szName.Kill();
			return mpcUnit->mcDiagnostics.Error(pcString, "This part of a string literal is not supported by the compiler yet");
		}
		for (i = 0; i < szName.Length(); i++)
		{
			pauiBytes->Add((uint8)szName.GetChar(i));
		}
		szName.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes of an element of a string expression: its widest token's, as
// adjacent literals take the prefix any of them has.  [lex.string]
//
//////////////////////////////////////////////////////////////////////////
int CCPPStrings::StringElementBytes(CCPPSyntax* pcString)
{
	CCPPToken*	pcToken;
	int			iToken;
	int			iBytes;

	iBytes = 1;
	for (iToken = pcString->miFirstToken; iToken <= pcString->miLastToken; iToken++)
	{
		pcToken = mpcUnit->mpcTokeniser->Get(iToken);
		pcToken = pcToken->IsUserString() ? ((CCTUserLiteral*)pcToken)->mpcLiteral : pcToken;
		if (pcToken->IsString() && (((CCTString*)pcToken)->meEncoding == CTE_UTF32))
		{
			return 4;
		}
		if (pcToken->IsString() && ((((CCTString*)pcToken)->meEncoding == CTE_Wide) || (((CCTString*)pcToken)->meEncoding == CTE_UTF16)))
		{
			iBytes = 2;
		}
	}
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the array a string literal is: its elements and the zero
// after them (NULL if it can't be decoded).  It is an lvalue: sizeof and
// & are of the array, and its value is a pointer to its first element.
// [lex.string] 8, [expr.prim.general] 1
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPStrings::StringArrayType(CCPPSyntax* pcString)
{
	CArrayUint8		auiBytes;
	SCPP65Type*		psArray;

	auiBytes.Init();
	psArray = DecodeString(pcString, &auiBytes) ? mpcUnit->mcTypes.Array(mpcUnit->mcTypes.Const(StringElementType(pcString)), (int)(auiBytes.NumElements() / StringElementBytes(pcString)) + 1) : NULL;
	auiBytes.Kill();
	return psArray;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of a string expression's elements: char, or wchar_t for L"",
// char16_t for u"" and char32_t for U"".  [lex.string] 8-11
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPStrings::StringElementType(CCPPSyntax* pcString)
{
	CCPPToken*		pcToken;
	SCPP65Type*		psElement;
	int				iToken;

	psElement = &mpcUnit->mcTypes.msChar;
	for (iToken = pcString->miFirstToken; iToken <= pcString->miLastToken; iToken++)
	{
		pcToken = mpcUnit->mpcTokeniser->Get(iToken);
		pcToken = pcToken->IsUserString() ? ((CCTUserLiteral*)pcToken)->mpcLiteral : pcToken;
		if (pcToken->IsString())
		{
			switch (((CCTString*)pcToken)->meEncoding)
			{
			case CTE_UTF32:	return &mpcUnit->mcTypes.msChar32;
			case CTE_UTF16:	psElement = &mpcUnit->mcTypes.msChar16;	break;
			case CTE_Wide:	psElement = &mpcUnit->mcTypes.msWChar;		break;
			default:		break;
			}
		}
	}
	return psElement;
}


//////////////////////////////////////////////////////////////////////////
//
// A decoded string's element uiElement.
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPPStrings::StringElement(CArrayUint8* pauiBytes, int iElementBytes, size uiElement)
{
	uint32	uiValue;
	int		i;

	uiValue = 0;
	for (i = 0; i < iElementBytes; i++)
	{
		uiValue |= (uint32)pauiBytes->GetValue(uiElement * iElementBytes + i) << (8 * i);
	}
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The elements of one raw string literal token: its characters as they
// are written, with nothing escaped.  A new line is one '\n' however the
// source ends its lines.  [lex.string], [lex.phases] 1
//
//////////////////////////////////////////////////////////////////////////
void CCPPStrings::DecodeRawStringToken(CCTString* pcToken, int iElementBytes, CArrayUint8* pauiBytes)
{
	const char*		sz;
	const char*		szEnd;

	RawStringCharacters(pcToken->OpeningQuote(), pcToken->mszEnd, &sz, &szEnd);
	while (sz < szEnd)
	{
		if ((sz[0] == '\r') && (sz + 1 < szEnd) && (sz[1] == '\n'))
		{
			sz++;
		}
		else if (iElementBytes == 1)
		{
			pauiBytes->Add((uint8)*sz);
			sz++;
		}
		else
		{
			AppendWideCharacter(pauiBytes, DecodeUTF8(&sz, szEnd), iElementBytes);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes of one narrow string literal token (its escapes decoded as
// the tokeniser decodes them, DecodeEscape).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPStrings::DecodeStringToken(CCPPSyntax* pcString, CCTString* pcToken, int iElementBytes, CArrayUint8* pauiBytes)
{
	const char*		sz;
	const char*		szEnd;
	uint32			uiValue;
	char			cEscape;

	if (pcToken->IsRaw())
	{
		DecodeRawStringToken(pcToken, iElementBytes, pauiBytes);
		return true;
	}

	sz = pcToken->OpeningQuote() + 1;
	szEnd = pcToken->mszEnd;

	while (sz < szEnd)
	{
		if (*sz != '\\')
		{
			if (iElementBytes == 1)
			{
				pauiBytes->Add((uint8)*sz);
				sz++;
			}
			else
			{
				AppendWideCharacter(pauiBytes, DecodeUTF8(&sz, szEnd), iElementBytes);
			}
			continue;
		}

		//An escape, as the tokeniser decodes it: \u and \U are the character's UTF-8, the others one byte.
		sz++;
		cEscape = *sz;
		if (!DecodeEscape(&sz, szEnd, &uiValue))
		{
			return mpcUnit->mcDiagnostics.Error(pcString, "Invalid escape sequence in a string literal");
		}
		if (((cEscape == 'u') || (cEscape == 'U')) && !IsCharacterCodePoint(uiValue))
		{
			return mpcUnit->mcDiagnostics.Error(pcString, "The universal character name names no character: a surrogate (D800 to DFFF) or a value past 10FFFF");
		}
		if ((cEscape == 'u') || (cEscape == 'U'))
		{
			if (iElementBytes == 1)
			{
				AppendUTF8(pauiBytes, uiValue);
			}
			else
			{
				AppendWideCharacter(pauiBytes, uiValue, iElementBytes);
			}
		}
		else if ((iElementBytes < 4) && (uiValue >= ((uint32)1 << (8 * iElementBytes))))
		{
			return mpcUnit->mcDiagnostics.Error(pcString, (iElementBytes == 1) ? "The escape sequence's value is too large for a char" : "The escape sequence's value is too large for a wide character");
		}
		else
		{
			AppendStringElement(pauiBytes, uiValue, iElementBytes);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// __func__ and __FUNCTION__ are the function's name.  __PRETTY_FUNCTION__ and
// __FUNCSIG__ are its class and name ("CClass::Name"), not the whole signature.
// Returns false if lliName is not one of them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPStrings::FunctionNameText(int64 lliName, CChars* pszText)
{
	char*	szName;
	int		iLength;
	bool	bPretty;

	bPretty = (lliName == mpcUnit->GetTokeniser()->mlliPrettyFunction) || (lliName == mpcUnit->GetTokeniser()->mlliFuncSig);
	if (!bPretty && (lliName != mpcUnit->GetTokeniser()->mlliFunc) && (lliName != mpcUnit->GetTokeniser()->mlliFunction))
	{
		return false;
	}

	if (mpcUnit->GetContext()->GetFunction())
	{
		if (bPretty && mpcUnit->GetContext()->GetFunction()->mpcClass)
		{
			szName = mpcUnit->GetTokeniser()->GetIdentifier(mpcUnit->GetContext()->GetFunction()->mpcClass->mlliName, &iLength);
			pszText->Append(szName, iLength);
			pszText->Append("::");
		}
		if (!mpcUnit->GetContext()->GetFunction()->IsOperator())
		{
			szName = mpcUnit->GetTokeniser()->GetIdentifier(mpcUnit->GetContext()->GetFunction()->mlliName, &iLength);
			pszText->Append(szName, iLength);
		}
		else
		{
			pszText->Append("operator");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True for __func__ and the other names of the enclosing function, which the parser
// declares as globals.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPStrings::IsFunctionName(CCPPSyntaxIdExpression* pcId)
{
	CCPPSymbol*	pcSymbol;
	CChars		szName;
	bool		bName;

	pcSymbol = pcId->mpcName->mpcSymbol;
	if ((pcId->mpcName->macpComponents.NumElements() != 1) || (pcSymbol && ((pcSymbol->meKind != CPPSK_Variable) || mpcUnit->GetVariable(pcSymbol))))
	{
		return false;
	}
	szName.Init();
	bName = FunctionNameText(mpcUnit->GetComponentName(pcId->mpcName->GetLast()), &szName);
	szName.Kill();
	return bName;
}
