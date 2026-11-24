/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "Logger.h"
#include "Number.h"
#include "StringHelper.h"
#include "CPPString.h"
#include "CPPIdentifier.h"
#include "YAMLWhitespace.h"
#include "YAMLParser.h"


//Plain scalars that YAML 1.2's core schema reads as null, true and false.
const char*	gaszYAMLNulls[] = { "~", "null", "Null", "NULL" };
const char*	gaszYAMLTrues[] = { "true", "True", "TRUE" };
const char*	gaszYAMLFalses[] = { "false", "False", "FALSE" };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CYAMLParser::Init(const char* szText, const char* szFileName, CMarkup* pcMarkup, CLogger* pcLogger)
{
	Init(szText, szFileName, pcMarkup->mpcDoc, pcLogger);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CYAMLParser::Init(const char* szText, const char* szFileName, CMarkupDoc* pcDoc, CLogger* pcLogger)
{
	CBaseParser::Init(szText, szFileName, pcLogger, SkipYAMLWhitespace, ParseCPPString, ParseCPPExactIdentifier, ParseCPPIdentifier);

	mpcDoc = pcDoc;
	mpcCurrent = NULL;
	mpcLogger = pcLogger;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CYAMLParser::Init(char* szText, lsize iTextLen, const char* szFileName, CMarkup* pcMarkup, CLogger* pcLogger)
{
	Init(szText, iTextLen, szFileName, pcMarkup->mpcDoc, pcLogger);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CYAMLParser::Init(char* szText, lsize iTextLen, const char* szFileName, CMarkupDoc* pcDoc, CLogger* pcLogger)
{
	CBaseParser::Init(szText, iTextLen, szFileName, pcLogger, SkipYAMLWhitespace, ParseCPPString, ParseCPPExactIdentifier, ParseCPPIdentifier);

	mpcDoc = pcDoc;
	mpcCurrent = NULL;
	mpcLogger = pcLogger;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CYAMLParser::Kill(void)
{
	CBaseParser::Kill();

	mpcDoc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::Parse(void)
{
	TRISTATE		tResult;
	STextPosition	sPosition;

	//Not CTextParser::SkipUTF8BOM: it skips white space first, and fails on empty text.  The BOM isn't in a column:
	//indentation is counted from after it.
	if ((Peek(0) == '\xEF') && (Peek(1) == '\xBB') && (Peek(2) == '\xBF'))
	{
		Step(3);
		sPosition.Init(mcParser.Position(), mcParser.Line(), (lsize)0);
		mcParser.SetPosition(&sPosition);
	}

	tResult = ParseRoot();
	if (tResult != TRITRUE)
	{
		return TRIERROR;
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// One document: an optional --- line, the root node, an optional ...
// line.  The root of a mapping or a sequence is its tag, named ""; a
// scalar root is an unnamed attribute of a root tag named "".
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseRoot(void)
{
	TRISTATE	tResult;
	lsize		uiIndent;
	char		c;

	tResult = SkipToContent();
	ReturnOnError(tResult);

	if ((Peek(0) == '%') && (mcParser.Column() == 0))
	{
		return Error("Directives (%) are not supported");
	}

	if (IsDocumentMarker("---"))
	{
		Step(3);
		mcParser.SkipWhitespace();
		if (!IsLineEnd())
		{
			return Error("A value on the --- line is not supported");
		}
		tResult = EndLine();
		ReturnOnError(tResult);
		tResult = SkipToContent();
		ReturnOnError(tResult);
	}

	if (mcParser.IsOutside())
	{
		return TRITRUE;
	}

	if (!IsDocumentMarker("...") && !IsDocumentMarker("---"))
	{
		uiIndent = mcParser.Column();
		if (IsSequenceEntry())
		{
			tResult = ParseBlockSequence("", uiIndent);
		}
		else if (IsKeyAhead())
		{
			tResult = ParseBlockMapping("", uiIndent);
		}
		else
		{
			c = Peek(0);
			if ((c == '[') || (c == '{'))
			{
				tResult = ParseInlineValue("");
			}
			else
			{
				AddTag("");
				tResult = ParseInlineValue("");
				EndTag();
			}

			if (tResult == TRITRUE)
			{
				tResult = EndLine();
			}
			if (tResult == TRITRUE)
			{
				tResult = SkipToContent();
			}
		}
		ReturnOnError(tResult);
	}

	if (IsDocumentMarker("..."))
	{
		Step(3);
		mcParser.SkipWhitespace();
		if (!IsLineEnd())
		{
			return Error("A value on the ... line is not supported");
		}
		tResult = EndLine();
		ReturnOnError(tResult);
		tResult = SkipToContent();
		ReturnOnError(tResult);
	}

	if (!mcParser.IsOutside())
	{
		if (IsDocumentMarker("---"))
		{
			return Error("Only one document per file is supported");
		}
		return Error("Expected the end of the document (is this line indented wrongly?)");
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a key or a "- " entry that has nothing after it on its
// line: a block mapping or sequence indented more on the lines below, a
// sequence at the key's own indent (bSameIndentSequence), a scalar alone
// on the next line, or else null.  Returns at the next line's content.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseBlockNode(const char* szName, lsize uiParentIndent, bool bSameIndentSequence)
{
	TRISTATE	tResult;
	lsize		uiIndent;

	tResult = EndLine();
	ReturnOnError(tResult);
	tResult = SkipToContent();
	ReturnOnError(tResult);

	if (mcParser.IsOutside() || IsDocumentMarker("---") || IsDocumentMarker("..."))
	{
		return AddNullAttribute(szName, true);
	}

	uiIndent = mcParser.Column();
	if (uiIndent > uiParentIndent)
	{
		if (IsSequenceEntry())
		{
			return ParseBlockSequence(szName, uiIndent);
		}
		if (IsKeyAhead())
		{
			return ParseBlockMapping(szName, uiIndent);
		}

		tResult = ParseInlineValue(szName);
		ReturnOnError(tResult);
		tResult = EndLine();
		ReturnOnError(tResult);
		tResult = SkipToContent();
		ReturnOnError(tResult);

		if (!mcParser.IsOutside() && (mcParser.Column() > uiParentIndent))
		{
			return Error("Unexpected indentation (values can't continue on the next line)");
		}
		return TRITRUE;
	}

	if (bSameIndentSequence && (uiIndent == uiParentIndent) && IsSequenceEntry())
	{
		return ParseBlockSequence(szName, uiIndent);
	}

	return AddNullAttribute(szName, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Keys at uiIndent, each with its value, until a line is indented less.
// Starts at the first key; returns at the next line's content.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseBlockMapping(const char* szName, lsize uiIndent)
{
	TRISTATE	tResult;
	CChars		szKey;

	AddTag(szName);
	for (;;)
	{
		if (IsSequenceEntry())
		{
			return Error("Expected a key, not a '- ' entry");
		}

		szKey.Init();
		tResult = ParseBlockKey(&szKey);
		if (tResult == TRITRUE)
		{
			tResult = CheckKey(szKey.Text());
		}

		if (tResult == TRITRUE)
		{
			mcParser.SkipWhitespace();
			if (IsLineEnd())
			{
				tResult = ParseBlockNode(szKey.Text(), uiIndent, true);
			}
			else
			{
				tResult = ParseInlineValue(szKey.Text());
				if (tResult == TRITRUE)
				{
					tResult = EndLine();
				}
				if (tResult == TRITRUE)
				{
					tResult = SkipToContent();
				}
			}
		}
		szKey.Kill();
		ReturnOnError(tResult);

		if (mcParser.IsOutside() || IsDocumentMarker("---") || IsDocumentMarker("...") || (mcParser.Column() < uiIndent))
		{
			break;
		}
		if (mcParser.Column() > uiIndent)
		{
			return Error("Unexpected indentation (values can't continue on the next line)");
		}
	}

	EndTag();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// "- " entries at uiIndent until a line is indented less or isn't an
// entry.  An entry's value is on its line (a scalar, a flow collection,
// the first key of a mapping or a nested "- "), or on the lines below.
// Starts at the first '-'; returns at the next line's content.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseBlockSequence(const char* szName, lsize uiIndent)
{
	TRISTATE	tResult;
	lsize		uiEntryIndent;

	AddTag(szName);
	for (;;)
	{
		mcParser.StepRight();
		mcParser.SkipWhitespace();
		if (IsLineEnd())
		{
			tResult = ParseBlockNode("", uiIndent, false);
		}
		else
		{
			uiEntryIndent = mcParser.Column();
			if (IsSequenceEntry())
			{
				tResult = ParseBlockSequence("", uiEntryIndent);
			}
			else if (IsKeyAhead())
			{
				tResult = ParseBlockMapping("", uiEntryIndent);
			}
			else
			{
				tResult = ParseInlineValue("");
				if (tResult == TRITRUE)
				{
					tResult = EndLine();
				}
				if (tResult == TRITRUE)
				{
					tResult = SkipToContent();
				}
			}
		}
		ReturnOnError(tResult);

		if (mcParser.IsOutside() || IsDocumentMarker("---") || IsDocumentMarker("...") || (mcParser.Column() < uiIndent))
		{
			break;
		}
		if (mcParser.Column() > uiIndent)
		{
			return Error("Unexpected indentation (values can't continue on the next line)");
		}
		if (!IsSequenceEntry())
		{
			break;
		}
	}

	EndTag();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// A value that starts on the current line: a tagged, quoted or plain
// scalar, or a flow collection.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseInlineValue(const char* szName)
{
	TRISTATE	tResult;
	CChars		sz;
	char		c;

	tResult = CheckIndicator();
	ReturnOnError(tResult);

	c = Peek(0);
	if (c == '!')
	{
		return ParseTagged(szName);
	}
	if (c == '[')
	{
		return ParseFlowSequence(szName);
	}
	if (c == '{')
	{
		return ParseFlowMapping(szName);
	}

	sz.Init();
	if ((c == '"') || (c == '\''))
	{
		tResult = ParseQuoted(&sz);
		if (tResult == TRITRUE)
		{
			tResult = AddStringAttribute(szName, sz.Text(), true);
		}
	}
	else
	{
		tResult = ParsePlain(&sz, false);
		if (tResult == TRITRUE)
		{
			tResult = AddScalar(szName, sz.Text());
		}
	}
	sz.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A tag and the scalar after it, kept as text: "!!python/name:a.b" or
// "!!str 12".
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseTagged(const char* szName)
{
	TRISTATE	tResult;
	CChars		szTag;
	CChars		szValue;
	char		c;

	szTag.Init();
	while (!IsYAMLSeparator(Peek(0)))
	{
		szTag.Append(Peek(0));
		mcParser.StepRight();
	}

	mcParser.SkipWhitespace();
	if (IsLineEnd())
	{
		tResult = AddStringAttribute(szName, szTag.Text(), true);
		szTag.Kill();
		return tResult;
	}

	c = Peek(0);
	if ((c == '[') || (c == '{'))
	{
		szTag.Kill();
		return Error("Tags on [ ] and { } collections are not supported");
	}

	tResult = CheckIndicator();
	if (tResult != TRITRUE)
	{
		szTag.Kill();
		return tResult;
	}

	szValue.Init();
	if ((c == '"') || (c == '\''))
	{
		tResult = ParseQuoted(&szValue);
	}
	else
	{
		tResult = ParsePlain(&szValue, false);
	}

	if (tResult == TRITRUE)
	{
		szTag.Append(' ');
		szTag.Append(&szValue);
		tResult = AddStringAttribute(szName, szTag.Text(), true);
	}
	szValue.Kill();
	szTag.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
// [ value, value, ... ], over several lines if need be.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseFlowSequence(const char* szName)
{
	TRISTATE	tResult;
	char		c;

	AddTag(szName);
	mcParser.StepRight();
	for (;;)
	{
		tResult = SkipFlowWhitespace();
		ReturnOnError(tResult);
		if (Peek(0) == ']')
		{
			mcParser.StepRight();
			EndTag();
			return TRITRUE;
		}

		tResult = ParseFlowNode("");
		ReturnOnError(tResult);

		tResult = SkipFlowWhitespace();
		ReturnOnError(tResult);
		c = Peek(0);
		if (c == ',')
		{
			mcParser.StepRight();
		}
		else if (c == ']')
		{
			mcParser.StepRight();
			EndTag();
			return TRITRUE;
		}
		else
		{
			return Error("Expected ',' or ']'");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// { key: value, key: value, ... }, over several lines if need be.  A key
// without a value is null.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseFlowMapping(const char* szName)
{
	TRISTATE	tResult;
	CChars		szKey;
	char		c;

	AddTag(szName);
	mcParser.StepRight();
	for (;;)
	{
		tResult = SkipFlowWhitespace();
		ReturnOnError(tResult);
		if (Peek(0) == '}')
		{
			mcParser.StepRight();
			EndTag();
			return TRITRUE;
		}

		szKey.Init();
		tResult = ParseFlowKey(&szKey);
		if (tResult == TRITRUE)
		{
			tResult = CheckKey(szKey.Text());
		}
		if (tResult == TRITRUE)
		{
			tResult = SkipFlowWhitespace();
		}
		if (tResult == TRITRUE)
		{
			c = Peek(0);
			if ((c == ',') || (c == '}'))
			{
				tResult = AddNullAttribute(szKey.Text(), true);
			}
			else
			{
				tResult = ParseFlowNode(szKey.Text());
			}
		}
		szKey.Kill();
		ReturnOnError(tResult);

		tResult = SkipFlowWhitespace();
		ReturnOnError(tResult);
		c = Peek(0);
		if (c == ',')
		{
			mcParser.StepRight();
		}
		else if (c == '}')
		{
			mcParser.StepRight();
			EndTag();
			return TRITRUE;
		}
		else
		{
			return Error("Expected ',' or '}'");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A value inside [ ] or { }.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseFlowNode(const char* szName)
{
	TRISTATE	tResult;
	CChars		sz;
	char		c;

	c = Peek(0);
	if (c == '[')
	{
		return ParseFlowSequence(szName);
	}
	if (c == '{')
	{
		return ParseFlowMapping(szName);
	}
	if (c == '!')
	{
		return Error("Tags inside [ ] and { } are not supported");
	}
	if (IsYAMLFlowIndicator(c))
	{
		return Error("Expected a value");
	}

	tResult = CheckIndicator();
	ReturnOnError(tResult);

	sz.Init();
	if ((c == '"') || (c == '\''))
	{
		tResult = ParseQuoted(&sz);
		if (tResult == TRITRUE)
		{
			tResult = AddStringAttribute(szName, sz.Text(), true);
		}
	}
	else
	{
		tResult = ParsePlain(&sz, true);
		if ((tResult == TRITRUE) && sz.Empty())
		{
			tResult = Error("Expected a value");
		}
		if (tResult == TRITRUE)
		{
			tResult = AddScalar(szName, sz.Text());
		}
	}
	sz.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A block mapping's key and its ':'.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseBlockKey(CChars* pszKey)
{
	TRISTATE	tResult;
	char		c;

	c = Peek(0);
	if ((c == '"') || (c == '\''))
	{
		tResult = ParseQuoted(pszKey);
		ReturnOnError(tResult);
		mcParser.SkipWhitespace();
	}
	else
	{
		for (;;)
		{
			c = Peek(0);
			if ((c == ':') && IsYAMLSeparator(Peek(1)))
			{
				break;
			}
			if ((c == '\n') || (c == '\0'))
			{
				return Error("Expected ':' after a key");
			}
			pszKey->Append(c);
			mcParser.StepRight();
		}
		while (!pszKey->Empty() && IsYAMLWhitespace(pszKey->Text()[pszKey->Length() - 1]))
		{
			pszKey->RemoveLastCharacter();
		}
	}

	if (pszKey->Empty())
	{
		return Error("Empty keys are not supported");
	}
	if (Peek(0) != ':')
	{
		return Error("Expected ':' after a key");
	}
	mcParser.StepRight();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// A flow mapping's key and its ':'.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseFlowKey(CChars* pszKey)
{
	TRISTATE	tResult;
	char		c;

	c = Peek(0);
	if ((c == '"') || (c == '\''))
	{
		tResult = ParseQuoted(pszKey);
	}
	else
	{
		tResult = ParsePlain(pszKey, true);
	}
	ReturnOnError(tResult);

	if (pszKey->Empty())
	{
		return Error("Empty keys are not supported");
	}

	tResult = SkipFlowWhitespace();
	ReturnOnError(tResult);
	if (Peek(0) != ':')
	{
		return Error("Expected ':' after a key");
	}
	mcParser.StepRight();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseQuoted(CChars* pszValue)
{
	if (Peek(0) == '"')
	{
		return ParseDoubleQuoted(pszValue);
	}
	return ParseSingleQuoted(pszValue);
}


//////////////////////////////////////////////////////////////////////////
//
// "..." with \ escapes, on one line.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseDoubleQuoted(CChars* pszValue)
{
	TRISTATE	tResult;
	char		c;

	mcParser.StepRight();
	for (;;)
	{
		c = Peek(0);
		if ((c == '\n') || (c == '\0'))
		{
			return Error("A quoted value must end on its own line (multi-line quoted values are not supported)");
		}
		if (c == '"')
		{
			mcParser.StepRight();
			return TRITRUE;
		}
		if (c == '\\')
		{
			tResult = ParseEscape(pszValue);
			ReturnOnError(tResult);
		}
		else
		{
			pszValue->Append(c);
			mcParser.StepRight();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// '...' where '' is a ', on one line.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseSingleQuoted(CChars* pszValue)
{
	char	c;

	mcParser.StepRight();
	for (;;)
	{
		c = Peek(0);
		if ((c == '\n') || (c == '\0'))
		{
			return Error("A quoted value must end on its own line (multi-line quoted values are not supported)");
		}
		if (c == '\'')
		{
			if (Peek(1) == '\'')
			{
				pszValue->Append('\'');
				Step(2);
			}
			else
			{
				mcParser.StepRight();
				return TRITRUE;
			}
		}
		else
		{
			pszValue->Append(c);
			mcParser.StepRight();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A double quoted value's escape (YAML 1.2 5.7), from its \.  Code
// points are appended as UTF-8.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParseEscape(CChars* pszValue)
{
	char		c;
	uint32		uiCode;
	size		uiDigits;
	size		i;
	uint32		uiDigit;

	mcParser.StepRight();
	c = Peek(0);
	uiDigits = 0;
	uiCode = 0;
	switch (c)
	{
	case 'a':	uiCode = 0x07;	break;
	case 'b':	uiCode = 0x08;	break;
	case 't':
	case '\t':	uiCode = 0x09;	break;
	case 'n':	uiCode = 0x0A;	break;
	case 'v':	uiCode = 0x0B;	break;
	case 'f':	uiCode = 0x0C;	break;
	case 'r':	uiCode = 0x0D;	break;
	case 'e':	uiCode = 0x1B;	break;
	case ' ':	uiCode = ' ';	break;
	case '"':	uiCode = '"';	break;
	case '/':	uiCode = '/';	break;
	case '\\':	uiCode = '\\';	break;
	case 'N':	uiCode = 0x85;	break;
	case '_':	uiCode = 0xA0;	break;
	case 'L':	uiCode = 0x2028;	break;
	case 'P':	uiCode = 0x2029;	break;
	case 'x':	uiDigits = 2;	break;
	case 'u':	uiDigits = 4;	break;
	case 'U':	uiDigits = 8;	break;
	case '0':	return Error("\\0 can't be held in a value");
	case '\n':
	case '\0':	return Error("A quoted value must end on its own line (multi-line quoted values are not supported)");
	default:	return Error("Unknown escape in a double quoted value");
	}
	mcParser.StepRight();

	for (i = 0; i < uiDigits; i++)
	{
		c = Peek(0);
		if ((c >= '0') && (c <= '9'))
		{
			uiDigit = c - '0';
		}
		else if ((c >= 'a') && (c <= 'f'))
		{
			uiDigit = c - 'a' + 10;
		}
		else if ((c >= 'A') && (c <= 'F'))
		{
			uiDigit = c - 'A' + 10;
		}
		else
		{
			return Error("Expected a hexadecimal digit in an escape");
		}
		uiCode = (uiCode << 4) | uiDigit;
		mcParser.StepRight();
	}

	if (uiCode == 0)
	{
		return Error("\\0 can't be held in a value");
	}

	if (uiCode < 0x80)
	{
		pszValue->Append((char)uiCode);
	}
	else if (uiCode < 0x800)
	{
		pszValue->Append((char)(0xC0 | (uiCode >> 6)));
		pszValue->Append((char)(0x80 | (uiCode & 0x3F)));
	}
	else if (uiCode < 0x10000)
	{
		pszValue->Append((char)(0xE0 | (uiCode >> 12)));
		pszValue->Append((char)(0x80 | ((uiCode >> 6) & 0x3F)));
		pszValue->Append((char)(0x80 | (uiCode & 0x3F)));
	}
	else if (uiCode < 0x110000)
	{
		pszValue->Append((char)(0xF0 | (uiCode >> 18)));
		pszValue->Append((char)(0x80 | ((uiCode >> 12) & 0x3F)));
		pszValue->Append((char)(0x80 | ((uiCode >> 6) & 0x3F)));
		pszValue->Append((char)(0x80 | (uiCode & 0x3F)));
	}
	else
	{
		return Error("An escape's code point is past U+10FFFF");
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// A plain (unquoted) scalar to the end of its line, a comment, or in a
// flow collection a , [ ] { } or ': '.  Trailing spaces are dropped.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::ParsePlain(CChars* pszValue, bool bFlow)
{
	char	c;
	char	cNext;

	for (;;)
	{
		c = Peek(0);
		if ((c == '\n') || (c == '\0'))
		{
			break;
		}
		cNext = Peek(1);
		if (IsYAMLWhitespace(c) && (cNext == '#'))
		{
			break;
		}
		if ((c == ':') && (IsYAMLSeparator(cNext) || (bFlow && IsYAMLFlowIndicator(cNext))))
		{
			if (!bFlow)
			{
				return Error("A value can't contain ': ' (quote it)");
			}
			break;
		}
		if (bFlow && IsYAMLFlowIndicator(c))
		{
			break;
		}
		pszValue->Append(c);
		mcParser.StepRight();
	}

	while (!pszValue->Empty() && IsYAMLWhitespace(pszValue->Text()[pszValue->Length() - 1]))
	{
		pszValue->RemoveLastCharacter();
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// A plain scalar typed by YAML 1.2's core schema: null, a boolean, a
// number or else a string.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::AddScalar(const char* szName, const char* szPlain)
{
	size		i;
	CNumber		cNumber;

	if (StrEmpty(szPlain))
	{
		return AddNullAttribute(szName, true);
	}
	for (i = 0; i < 4; i++)
	{
		if (StringCompare(szPlain, gaszYAMLNulls[i]) == 0)
		{
			return AddNullAttribute(szName, true);
		}
	}
	for (i = 0; i < 3; i++)
	{
		if (StringCompare(szPlain, gaszYAMLTrues[i]) == 0)
		{
			return AddBooleanAttribute(szName, true, true);
		}
		if (StringCompare(szPlain, gaszYAMLFalses[i]) == 0)
		{
			return AddBooleanAttribute(szName, false, true);
		}
	}

	if (IsYAMLNumber(szPlain))
	{
		cNumber.Init(szPlain);
		if (!cNumber.IsNAN())
		{
			return AddNumberAttribute(szName, &cNumber, true);
		}
	}
	return AddStringAttribute(szName, szPlain, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::AddTag(const char* szName)
{
	mpcCurrent = mpcDoc->AppendTag(mpcCurrent, szName, mcParser.Line(), mcParser.Column());
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CYAMLParser::EndTag(void)
{
	mpcCurrent = mpcCurrent->mpcParent;
}


//////////////////////////////////////////////////////////////////////////
//
// A mapping's keys are unique: neither an attribute nor a tag of the
// mapping may have the name already.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::CheckKey(const char* szKey)
{
	CChars		sz;
	TRISTATE	tResult;

	if (mpcCurrent->GetAttribute(szKey, NULL, NULL) || (mpcCurrent->GetTag(szKey) != NULL))
	{
		sz.Init("Duplicate key '");
		sz.Append(szKey);
		sz.Append("'");
		tResult = Error(sz.Text());
		sz.Kill();
		return tResult;
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// The characters that can't start a supported value.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::CheckIndicator(void)
{
	char	c;

	c = Peek(0);
	if (c == '&')
	{
		return Error("Anchors (&) are not supported");
	}
	if (c == '*')
	{
		return Error("Aliases (*) are not supported");
	}
	if ((c == '|') || (c == '>'))
	{
		return Error("Block scalars (| and >) are not supported");
	}
	if ((c == '?') && IsYAMLSeparator(Peek(1)))
	{
		return Error("Complex keys (?) are not supported");
	}
	if ((c == '-') && IsYAMLSeparator(Peek(1)))
	{
		return Error("A '- ' entry can't start on its key's line");
	}
	if ((c == '%') || (c == '@') || (c == '`'))
	{
		return Error("A value can't start with %, @ or ` (quote it)");
	}
	if ((c == ',') || (c == ']') || (c == '}'))
	{
		return Error("A value can't start with ',', ']' or '}' (quote it)");
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// Over blank lines and comment lines to the first character of the next
// line with content, or the end.  Tabs can't indent.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::SkipToContent(void)
{
	bool	bTab;
	char	c;

	for (;;)
	{
		if (mcParser.IsOutside())
		{
			return TRITRUE;
		}

		bTab = false;
		for (;;)
		{
			c = Peek(0);
			if (c == '\t')
			{
				bTab = true;
			}
			else if (c != ' ')
			{
				break;
			}
			mcParser.StepRight();
		}

		if (c == '\n')
		{
			mcParser.StepRight();
		}
		else if (c == '#')
		{
			while (!mcParser.IsOutside() && (Peek(0) != '\n'))
			{
				mcParser.StepRight();
			}
		}
		else if (c == '\0')
		{
			return TRITRUE;
		}
		else if (bTab)
		{
			return Error("Tabs can't be used to indent");
		}
		else
		{
			return TRITRUE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// After a value: white space, an optional comment and the line's end.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::EndLine(void)
{
	char	c;

	mcParser.SkipWhitespace();
	c = Peek(0);
	if (c == '#')
	{
		if ((mcParser.Column() != 0) && !IsYAMLWhitespace(mcParser.Current(-1)))
		{
			return Error("A comment needs a space before its #");
		}
		while (!mcParser.IsOutside() && (Peek(0) != '\n'))
		{
			mcParser.StepRight();
		}
		c = Peek(0);
	}

	if (c == '\n')
	{
		mcParser.StepRight();
		return TRITRUE;
	}
	if (mcParser.IsOutside())
	{
		return TRITRUE;
	}
	return Error("Expected the end of the line");
}


//////////////////////////////////////////////////////////////////////////
//
// Inside [ ] and { }: spaces, tabs, line ends and comments.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CYAMLParser::SkipFlowWhitespace(void)
{
	bool	bAfterSpace;
	char	c;

	bAfterSpace = false;
	for (;;)
	{
		c = Peek(0);
		if (IsYAMLWhitespace(c) || (c == '\n'))
		{
			bAfterSpace = true;
			mcParser.StepRight();
		}
		else if ((c == '#') && bAfterSpace)
		{
			while (!mcParser.IsOutside() && (Peek(0) != '\n'))
			{
				mcParser.StepRight();
			}
		}
		else
		{
			break;
		}
	}

	if (mcParser.IsOutside())
	{
		return Error("Expected the end of a [ ] or { } collection");
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// The rest of the line is white space, a comment or nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CYAMLParser::IsLineEnd(void)
{
	char	c;

	c = Peek(0);
	return (c == '\0') || (c == '\n') || (c == '#');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CYAMLParser::IsSequenceEntry(void)
{
	return (Peek(0) == '-') && IsYAMLSeparator(Peek(1));
}


//////////////////////////////////////////////////////////////////////////
//
// The current line starts a block mapping entry: a key (plain or quoted)
// then ':' and white space or the line's end.
//
//////////////////////////////////////////////////////////////////////////
bool CYAMLParser::IsKeyAhead(void)
{
	lsize	i;
	char	c;
	char	cQuote;

	c = Peek(0);
	if ((c == '"') || (c == '\''))
	{
		cQuote = c;
		i = 1;
		for (;;)
		{
			c = Peek(i);
			if ((c == '\0') || (c == '\n'))
			{
				return false;
			}
			if ((cQuote == '"') && (c == '\\'))
			{
				if (Peek(i + 1) == '\0')
				{
					return false;
				}
				i += 2;
			}
			else if (c == cQuote)
			{
				if ((cQuote == '\'') && (Peek(i + 1) == '\''))
				{
					i += 2;
				}
				else
				{
					i++;
					break;
				}
			}
			else
			{
				i++;
			}
		}

		while (IsYAMLWhitespace(Peek(i)))
		{
			i++;
		}
		return (Peek(i) == ':') && IsYAMLSeparator(Peek(i + 1));
	}

	if ((c == '[') || (c == '{') || (c == '#') || (c == '&') || (c == '*') || (c == '!') || (c == '|') || (c == '>') || (c == '%') || (c == '@') || (c == '`') || (c == '\0') || (c == '\n'))
	{
		return false;
	}

	for (i = 0;; i++)
	{
		c = Peek(i);
		if ((c == '\0') || (c == '\n'))
		{
			return false;
		}
		if ((c == ':') && IsYAMLSeparator(Peek(i + 1)))
		{
			return true;
		}
		if (IsYAMLWhitespace(c) && (Peek(i + 1) == '#'))
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// --- or ... at the start of a line, alone or before white space.
//
//////////////////////////////////////////////////////////////////////////
bool CYAMLParser::IsDocumentMarker(const char* szMarker)
{
	if (mcParser.IsOutside() || (mcParser.Column() != 0))
	{
		return false;
	}
	return (Peek(0) == szMarker[0]) && (Peek(1) == szMarker[1]) && (Peek(2) == szMarker[2]) && IsYAMLSeparator(Peek(3));
}


//////////////////////////////////////////////////////////////////////////
//
// The character uiOffset after the current one, or 0 past the end.
//
//////////////////////////////////////////////////////////////////////////
char CYAMLParser::Peek(lsize uiOffset)
{
	if (mcParser.IsOutside() || (uiOffset >= mcParser.RemainingLength()))
	{
		return '\0';
	}
	return mcParser.Current((int)uiOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CYAMLParser::Step(lsize uiCount)
{
	lsize	i;

	for (i = 0; i < uiCount; i++)
	{
		mcParser.StepRight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// White space, a line's end or the text's end: what must follow a
// mapping's ':' and a sequence's '-'.
//
//////////////////////////////////////////////////////////////////////////
bool IsYAMLSeparator(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\0');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsYAMLFlowIndicator(char c)
{
	return (c == ',') || (c == '[') || (c == ']') || (c == '{') || (c == '}');
}


//////////////////////////////////////////////////////////////////////////
//
// Decimal digits with an optional sign and an optional fraction: the
// numbers a plain scalar can be (hexadecimal, octal, exponents, .inf
// and .nan stay strings).
//
//////////////////////////////////////////////////////////////////////////
bool IsYAMLNumber(const char* szText)
{
	size	i;
	size	uiDigits;

	i = 0;
	if ((szText[i] == '-') || (szText[i] == '+'))
	{
		i++;
	}

	uiDigits = 0;
	while ((szText[i] >= '0') && (szText[i] <= '9'))
	{
		i++;
		uiDigits++;
	}
	if (uiDigits == 0)
	{
		return false;
	}

	if (szText[i] == '.')
	{
		i++;
		uiDigits = 0;
		while ((szText[i] >= '0') && (szText[i] <= '9'))
		{
			i++;
			uiDigits++;
		}
		if (uiDigits == 0)
		{
			return false;
		}
	}
	return szText[i] == '\0';
}

