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
#include "UTF8.h"
#include "TextNavigation.h"


//////////////////////////////////////////////////////////////////////////
//
// Positions are bytes of pszText's UTF8 text, kept: the text can change
// between calls.
//
//////////////////////////////////////////////////////////////////////////
void CTextNavigation::Init(CChars* pszText)
{
	mpszText = pszText;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes in the character at uiPosition: a code point, or a sequence
// of them joined by ZWJs.  A byte that doesn't start valid UTF8 is a
// character by itself.
//
//////////////////////////////////////////////////////////////////////////
size CTextNavigation::CharacterLength(size uiPosition)
{
	CUTF8	cUTF8;
	size	uiRemaining;
	size	uiCharLength;

	if (uiPosition >= mpszText->Length())
	{
		return 0;
	}

	uiRemaining = mpszText->Length() - uiPosition;
	cUTF8.Init((uint8*)mpszText->Text() + uiPosition, uiRemaining);
	uiCharLength = cUTF8.PeekUTFBytes();
	cUTF8.Kill();
	if ((uiCharLength == 0) || (uiCharLength > uiRemaining))
	{
		return 1;
	}
	return uiCharLength;
}


//////////////////////////////////////////////////////////////////////////
//
// The start of the character after the one at uiPosition.  A carriage
// return and line feed are one character.
//
//////////////////////////////////////////////////////////////////////////
size CTextNavigation::NextPosition(size uiPosition)
{
	char*	szText;

	if (uiPosition >= mpszText->Length())
	{
		return mpszText->Length();
	}

	szText = mpszText->Text();
	if ((szText[uiPosition] == '\r') && (szText[uiPosition + 1] == '\n'))
	{
		return uiPosition + 2;
	}
	return uiPosition + CharacterLength(uiPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// The start of the character before uiPosition.  Characters can only be
// found going forward, so they are from the start of the line.
//
//////////////////////////////////////////////////////////////////////////
size CTextNavigation::PreviousPosition(size uiPosition)
{
	size	uiCurrent;
	size	uiNext;
	char*	szText;

	if (uiPosition == 0)
	{
		return 0;
	}

	szText = mpszText->Text();
	uiCurrent = FindLineStart(uiPosition);
	if (uiCurrent == uiPosition)
	{
		if ((uiPosition >= 2) && (szText[uiPosition - 2] == '\r'))
		{
			return uiPosition - 2;
		}
		return uiPosition - 1;
	}

	for (;;)
	{
		uiNext = NextPosition(uiCurrent);
		if (uiNext >= uiPosition)
		{
			return uiCurrent;
		}
		uiCurrent = uiNext;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The start of the line (ended by line feeds) uiPosition is on.
//
//////////////////////////////////////////////////////////////////////////
size CTextNavigation::FindLineStart(size uiPosition)
{
	char*	szText;

	szText = mpszText->Text();
	while ((uiPosition > 0) && (szText[uiPosition - 1] != '\n'))
	{
		uiPosition--;
	}
	return uiPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// The end of the line uiPosition is on, before its carriage return and
// line feed, or the end of the text.
//
//////////////////////////////////////////////////////////////////////////
size CTextNavigation::FindLineEnd(size uiPosition)
{
	char*	szText;
	size	uiLength;
	size	uiStart;

	szText = mpszText->Text();
	uiLength = mpszText->Length();
	uiStart = uiPosition;
	while ((uiPosition < uiLength) && (szText[uiPosition] != '\n'))
	{
		uiPosition++;
	}
	if ((uiPosition < uiLength) && (uiPosition > uiStart) && (szText[uiPosition - 1] == '\r'))
	{
		uiPosition--;
	}
	return uiPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// The first character of the line that isn't a space or tab, or the end
// of the line.
//
//////////////////////////////////////////////////////////////////////////
size CTextNavigation::FindLineIndent(size uiLineStart)
{
	size	uiLineEnd;
	char*	szText;

	szText = mpszText->Text();
	uiLineEnd = FindLineEnd(uiLineStart);
	while ((uiLineStart < uiLineEnd) && IsSpaceOrTab(szText[uiLineStart]))
	{
		uiLineStart++;
	}
	return uiLineStart;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of characters from the start of the line to uiPosition.
//
//////////////////////////////////////////////////////////////////////////
size CTextNavigation::CountColumns(size uiLineStart, size uiPosition)
{
	size	uiColumns;

	uiColumns = 0;
	while (uiLineStart < uiPosition)
	{
		uiLineStart = NextPosition(uiLineStart);
		uiColumns++;
	}
	return uiColumns;
}


//////////////////////////////////////////////////////////////////////////
//
// The position uiColumn characters along the line, or its end if it is
// shorter.
//
//////////////////////////////////////////////////////////////////////////
size CTextNavigation::FindColumn(size uiLineStart, size uiColumn)
{
	size	uiLineEnd;

	uiLineEnd = FindLineEnd(uiLineStart);
	while ((uiColumn > 0) && (uiLineStart < uiLineEnd))
	{
		uiLineStart = NextPosition(uiLineStart);
		uiColumn--;
	}
	return uiLineStart;
}


//////////////////////////////////////////////////////////////////////////
//
// The start of the word, or spaces, before uiPosition.  Punctuation is a
// word of one character.
//
//////////////////////////////////////////////////////////////////////////
size CTextNavigation::FindWordLeft(size uiPosition)
{
	size	uiPrevious;
	char*	szText;
	bool	bWhitespace;

	if (uiPosition == 0)
	{
		return 0;
	}

	szText = mpszText->Text();
	uiPosition = PreviousPosition(uiPosition);
	bWhitespace = IsWhitespace(szText[uiPosition]);
	if (!bWhitespace && !IsWordChar(szText[uiPosition]))
	{
		return uiPosition;
	}

	while (uiPosition > 0)
	{
		uiPrevious = PreviousPosition(uiPosition);
		if (bWhitespace ? !IsWhitespace(szText[uiPrevious]) : !IsWordChar(szText[uiPrevious]))
		{
			break;
		}
		uiPosition = uiPrevious;
	}
	return uiPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// The end of the word, or spaces, after uiPosition.  Punctuation is a
// word of one character.
//
//////////////////////////////////////////////////////////////////////////
size CTextNavigation::FindWordRight(size uiPosition)
{
	size	uiLength;
	char*	szText;
	bool	bWhitespace;

	uiLength = mpszText->Length();
	if (uiPosition >= uiLength)
	{
		return uiLength;
	}

	szText = mpszText->Text();
	bWhitespace = IsWhitespace(szText[uiPosition]);
	if (!bWhitespace && !IsWordChar(szText[uiPosition]))
	{
		return NextPosition(uiPosition);
	}

	while ((uiPosition < uiLength) && (bWhitespace ? IsWhitespace(szText[uiPosition]) : IsWordChar(szText[uiPosition])))
	{
		uiPosition = NextPosition(uiPosition);
	}
	return uiPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// The text from uiStart to uiEnd is worth finding the occurrences of, as
// a selection's are shown: it is on one line and isn't only spaces and
// tabs.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::IsOccurrenceText(size uiStart, size uiEnd)
{
	char*	szText;
	size	i;
	bool	bSpaces;

	if ((uiEnd <= uiStart) || (uiEnd > mpszText->Length()))
	{
		return false;
	}
	szText = mpszText->Text();
	bSpaces = true;
	for (i = uiStart; i < uiEnd; i++)
	{
		if ((szText[i] == '\n') || (szText[i] == '\r'))
		{
			return false;
		}
		if (!IsSpaceOrTab(szText[i]))
		{
			bSpaces = false;
		}
	}
	return !bSpaces;
}


//////////////////////////////////////////////////////////////////////////
//
// The starts of the other places the text from uiStart to uiEnd is in the
// text, exactly, case and all, are added to pauiStarts in order, up to
// uiMaxOccurrences of them.  They are found from the start and don't
// overlap each other; uiStart itself isn't one.
//
//////////////////////////////////////////////////////////////////////////
void CTextNavigation::FindOccurrences(size uiStart, size uiEnd, size uiMaxOccurrences, CArrayTextOccurrence* pauiStarts)
{
	char*	szText;
	size	uiLength;
	size	uiTextLength;
	size	i;
	size	uiFound;

	uiTextLength = mpszText->Length();
	if ((uiEnd <= uiStart) || (uiEnd > uiTextLength))
	{
		return;
	}
	szText = mpszText->Text();
	uiLength = uiEnd - uiStart;
	uiFound = 0;
	i = 0;
	while ((i + uiLength <= uiTextLength) && (uiFound < uiMaxOccurrences))
	{
		if ((szText[i] == szText[uiStart]) && (memcmp(&szText[i], &szText[uiStart], uiLength) == 0))
		{
			if (i != uiStart)
			{
				*pauiStarts->Add() = i;
				uiFound++;
			}
			i += uiLength;
		}
		else
		{
			i++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// uiStart to uiEnd is a range of the text: in order and not past its end.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::IsRange(size uiStart, size uiEnd)
{
	return (uiStart <= uiEnd) && (uiEnd <= mpszText->Length());
}


//////////////////////////////////////////////////////////////////////////
//
// *puiStart to *puiEnd grows over the characters before and after it that
// fIsInRun is true of.  By bytes: a character's bytes past 0x7F are all
// in a run or all out of it.
//
//////////////////////////////////////////////////////////////////////////
void CTextNavigation::ExpandRun(bool (*fIsInRun)(char), size* puiStart, size* puiEnd)
{
	char*	szText;
	size	uiLength;

	szText = mpszText->Text();
	uiLength = mpszText->Length();
	while ((*puiStart > 0) && fIsInRun(szText[*puiStart - 1]))
	{
		(*puiStart)--;
	}
	while ((*puiEnd < uiLength) && fIsInRun(szText[*puiEnd]))
	{
		(*puiEnd)++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The run of word characters that uiStart to uiEnd is in: uiStart to
// uiEnd are all word characters, or with nothing between them they are
// at or next to one.  False if they aren't in a word.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::FindWordOver(size uiStart, size uiEnd, size* puiStart, size* puiEnd)
{
	char*	szText;
	size	i;

	if (!IsRange(uiStart, uiEnd))
	{
		return false;
	}
	szText = mpszText->Text();
	for (i = uiStart; i < uiEnd; i++)
	{
		if (!IsWordChar(szText[i]))
		{
			return false;
		}
	}
	ExpandRun(&IsWordChar, &uiStart, &uiEnd);
	*puiStart = uiStart;
	*puiEnd = uiEnd;
	return uiStart < uiEnd;
}


//////////////////////////////////////////////////////////////////////////
//
// The quotes, ' or ", on uiStart's line whose inside has uiStart to uiEnd
// in it: *puiOpen is the opening quote and *puiClose the closing one.
// The line's quotes are paired from its start, a backslash escaping the
// character after it.  False if there are none around them, or they go
// past the line.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::FindQuotesAround(size uiStart, size uiEnd, size* puiOpen, size* puiClose)
{
	char*	szText;
	size	uiLineEnd;
	size	i;
	size	uiClose;

	if (!IsRange(uiStart, uiEnd))
	{
		return false;
	}
	szText = mpszText->Text();
	uiLineEnd = FindLineEnd(uiStart);
	for (i = FindLineStart(uiStart); i < uiLineEnd; i++)
	{
		if ((szText[i] != '"') && (szText[i] != '\''))
		{
			continue;
		}
		for (uiClose = i + 1; (uiClose < uiLineEnd) && (szText[uiClose] != szText[i]); uiClose++)
		{
			if ((szText[uiClose] == '\\') && (uiClose + 1 < uiLineEnd))
			{
				uiClose++;
			}
		}
		if (uiClose >= uiLineEnd)
		{
			return false;
		}
		if ((i < uiStart) && (uiEnd <= uiClose))
		{
			*puiOpen = i;
			*puiClose = uiClose;
			return true;
		}
		i = uiClose;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The brackets, ( [ or {, nearest around uiStart to uiEnd: *puiOpen the
// opening bracket at or before uiStart, *puiClose the bracket that closes
// it, at or after uiEnd - 1, the brackets and what is in them more than
// uiStart to uiEnd.  Brackets between are paired as they nest; strings
// and comments aren't looked past.  False if there are none around them.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::FindBracketsAround(size uiStart, size uiEnd, size* puiOpen, size* puiClose)
{
	char*	szText;
	size	i;
	size	uiDepth;
	size	uiClose;
	char	c;

	szText = mpszText->Text();
	if (!IsRange(uiStart, uiEnd))
	{
		return false;
	}

	//Back from the selection's first character (none for a caret) to an opening bracket not closed before it.
	uiDepth = 0;
	for (i = (uiStart < uiEnd) ? uiStart + 1 : uiStart; i > 0; i--)
	{
		c = szText[i - 1];
		if (IsClosingBracket(c))
		{
			uiDepth++;
		}
		else if (IsOpeningBracket(c))
		{
			if (uiDepth > 0)
			{
				uiDepth--;
			}
			else if (FindClosingBracket(i - 1, &uiClose) && (uiClose + 1 >= uiEnd) && ((i - 1 < uiStart) || (uiClose + 1 > uiEnd)))
			{
				*puiOpen = i - 1;
				*puiClose = uiClose;
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The bracket that closes the opening bracket at uiOpen, the brackets
// after it paired as they nest.  False if it isn't closed, or is closed
// by another kind.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::FindClosingBracket(size uiOpen, size* puiClose)
{
	char*	szText;
	size	uiLength;
	size	i;
	size	uiDepth;
	char	cClose;

	szText = mpszText->Text();
	uiLength = mpszText->Length();
	cClose = (szText[uiOpen] == '(') ? ')' : ((szText[uiOpen] == '[') ? ']' : '}');
	uiDepth = 0;
	for (i = uiOpen + 1; i < uiLength; i++)
	{
		if (IsOpeningBracket(szText[i]))
		{
			uiDepth++;
		}
		else if (IsClosingBracket(szText[i]))
		{
			if (uiDepth == 0)
			{
				*puiClose = i;
				return szText[i] == cClose;
			}
			uiDepth--;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The comment uiStart to uiEnd is in: a // comment to its line's end
// (not one in quotes), or a /* */ comment with its ends.  False if they
// aren't in one.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::FindCommentAround(size uiStart, size uiEnd, size* puiStart, size* puiEnd)
{
	char*	szText;
	size	uiLength;
	size	uiLineEnd;
	size	uiOpen;
	size	uiClose;
	size	i;

	szText = mpszText->Text();
	uiLength = mpszText->Length();
	if (!IsRange(uiStart, uiEnd) || (uiLength < 2))
	{
		return false;
	}

	//A line comment starts before uiStart on its line, outside quotes.
	uiLineEnd = FindLineEnd(uiStart);
	if (uiEnd <= uiLineEnd)
	{
		for (i = FindLineStart(uiStart); (i + 1 < uiLineEnd) && (i <= uiStart); i++)
		{
			if ((szText[i] == '/') && (szText[i + 1] == '/'))
			{
				if (!FindQuotesAround(i, i + 2, &uiOpen, &uiClose))
				{
					*puiStart = i;
					*puiEnd = uiLineEnd;
					return true;
				}
			}
		}
	}

	//A block comment opens at or before uiStart and first closes at or after uiEnd.
	uiOpen = (uiStart < uiLength - 1) ? uiStart : uiLength - 2;
	while ((szText[uiOpen] != '/') || (szText[uiOpen + 1] != '*'))
	{
		if (uiOpen == 0)
		{
			return false;
		}
		uiOpen--;
	}
	for (i = uiOpen + 3; i < uiLength; i++)
	{
		if ((szText[i - 1] == '*') && (szText[i] == '/'))
		{
			if (i + 1 < uiEnd)
			{
				return false;
			}
			*puiStart = uiOpen;
			*puiEnd = i + 1;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The word the character at uiPosition is in, as a double click selects
// it: a run of letters, digits and underscores, a run of spaces and tabs,
// or one other character.  Words don't go past the line.  Returns false,
// with the start and end at uiPosition, at a line's end or the text's.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::FindWordAround(size uiPosition, size* puiStart, size* puiEnd)
{
	size	uiLength;
	size	uiStart;
	size	uiEnd;
	char*	szText;
	bool	bSpaces;

	*puiStart = uiPosition;
	*puiEnd = uiPosition;
	uiLength = mpszText->Length();
	if (uiPosition >= uiLength)
	{
		return false;
	}

	szText = mpszText->Text();
	if ((szText[uiPosition] == '\n') || (szText[uiPosition] == '\r'))
	{
		return false;
	}

	bSpaces = IsSpaceOrTab(szText[uiPosition]);
	if (!bSpaces && !IsWordChar(szText[uiPosition]))
	{
		*puiEnd = NextPosition(uiPosition);
		return true;
	}

	uiStart = uiPosition;
	uiEnd = uiPosition;
	ExpandRun(bSpaces ? &IsSpaceOrTab : &IsWordChar, &uiStart, &uiEnd);
	*puiStart = uiStart;
	*puiEnd = uiEnd;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The line uiPosition is on with its line feed (and carriage return), as
// a triple click selects it.  The last line ends at the text's end.
//
//////////////////////////////////////////////////////////////////////////
void CTextNavigation::FindLineAround(size uiPosition, size* puiStart, size* puiEnd)
{
	*puiStart = FindLineStart(uiPosition);
	*puiEnd = NextPosition(FindLineEnd(uiPosition));
}


//////////////////////////////////////////////////////////////////////////
//
// Spaces and tabs: the spaces a line has, not the ones that end it.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::IsSpaceOrTab(char c)
{
	return (c == ' ') || (c == '\t');
}


//////////////////////////////////////////////////////////////////////////
//
// The brackets that pair: ( [ and {, and what closes them.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::IsOpeningBracket(char c) { return (c == '(') || (c == '[') || (c == '{'); }
bool CTextNavigation::IsClosingBracket(char c) { return (c == ')') || (c == ']') || (c == '}'); }


//////////////////////////////////////////////////////////////////////////
//
// Spaces, tabs, carriage returns and line feeds.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::IsWhitespace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}


//////////////////////////////////////////////////////////////////////////
//
// Letters, digits and underscores.  Any character that isn't ASCII is
// treated as a letter.
//
//////////////////////////////////////////////////////////////////////////
bool CTextNavigation::IsWordChar(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_') || ((uint8)c >= 0x80);
}

