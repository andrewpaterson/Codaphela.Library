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
#include <string.h>
#include "StringHelper.h"
#include "TextSearch.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsTextSearchWordCharacter(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static char ToTextSearchUpper(char c)
{
	return ((c >= 'a') && (c <= 'z')) ? (char)(c - 'a' + 'A') : c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextSearch::Init(const char* szPattern, bool bMatchCase, bool bWholeWords)
{
	mszPattern.Init(szPattern);
	mbMatchCase = bMatchCase;
	mbWholeWords = bWholeWords;
	if (!mbMatchCase)
	{
		ToLower(mszPattern.Text());
	}
	mcFirst = mszPattern.Empty() ? '\0' : mszPattern.Text()[0];
	mcFirstUpper = mbMatchCase ? mcFirst : ToTextSearchUpper(mcFirst);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextSearch::Kill(void)
{
	mszPattern.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the pattern is at uiPosition, its first character there
// already.
//
//////////////////////////////////////////////////////////////////////////
bool CTextSearch::IsAt(const char* szText, size uiLength, size uiPosition)
{
	const char*	szPattern;
	size		uiPattern;
	size		i;

	szPattern = mszPattern.Text();
	uiPattern = mszPattern.Length();
	if (mbMatchCase)
	{
		if (memcmp(&szText[uiPosition + 1], &szPattern[1], uiPattern - 1) != 0)
		{
			return false;
		}
	}
	else
	{
		for (i = 1; i < uiPattern; i++)
		{
			if (ToLower(szText[uiPosition + i]) != szPattern[i])
			{
				return false;
			}
		}
	}

	if (mbWholeWords)
	{
		if ((uiPosition > 0) && IsTextSearchWordCharacter(szPattern[0]) && IsTextSearchWordCharacter(szText[uiPosition - 1]))
		{
			return false;
		}
		if ((uiPosition + uiPattern < uiLength) && IsTextSearchWordCharacter(szPattern[uiPattern - 1]) && IsTextSearchWordCharacter(szText[uiPosition + uiPattern]))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the pattern first is in szText at or after uiFrom,
// TEXT_SEARCH_NOT_FOUND if it isn't.
//
//////////////////////////////////////////////////////////////////////////
size CTextSearch::Find(const char* szText, size uiLength, size uiFrom)
{
	const char*	pcFound;
	const char*	pcUpper;
	size		uiPattern;
	size		uiLast;
	size		uiPosition;
	size		uiLower;
	size		uiUpper;

	uiPattern = mszPattern.Length();
	if ((uiPattern == 0) || (uiLength < uiPattern) || (uiFrom > uiLength - uiPattern))
	{
		return TEXT_SEARCH_NOT_FOUND;
	}

	//The first character is found with memchr, which is much the quickest way through text it isn't in.  Without the
	//case each of its cases is looked for, the other only as far as the one found.
	uiLast = uiLength - uiPattern;
	uiPosition = uiFrom;
	uiLower = uiFrom;
	uiUpper = uiFrom;
	for (;;)
	{
		if (uiLower <= uiPosition)
		{
			pcFound = (const char*)memchr(&szText[uiPosition], mcFirst, uiLast - uiPosition + 1);
			uiLower = pcFound ? (size)(pcFound - szText) : TEXT_SEARCH_NOT_FOUND;
		}
		if (mcFirstUpper == mcFirst)
		{
			uiUpper = TEXT_SEARCH_NOT_FOUND;
		}
		else if (uiUpper <= uiPosition)
		{
			pcUpper = (const char*)memchr(&szText[uiPosition], mcFirstUpper, ((uiLower != TEXT_SEARCH_NOT_FOUND) ? uiLower : uiLast + 1) - uiPosition);
			uiUpper = pcUpper ? (size)(pcUpper - szText) : ((uiLower != TEXT_SEARCH_NOT_FOUND) ? uiLower + 1 : TEXT_SEARCH_NOT_FOUND);
		}

		uiPosition = (uiLower < uiUpper) ? uiLower : uiUpper;
		if (uiPosition > uiLast)
		{
			return TEXT_SEARCH_NOT_FOUND;
		}
		if (IsAt(szText, uiLength, uiPosition))
		{
			return uiPosition;
		}
		uiPosition++;
		if (uiPosition > uiLast)
		{
			return TEXT_SEARCH_NOT_FOUND;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many times the pattern is in szText, those that overlap not
// counted.
//
//////////////////////////////////////////////////////////////////////////
size CTextSearch::Count(const char* szText, size uiLength)
{
	size	uiCount;
	size	uiPosition;

	uiCount = 0;
	uiPosition = Find(szText, uiLength, 0);
	while (uiPosition != TEXT_SEARCH_NOT_FOUND)
	{
		uiCount++;
		uiPosition = Find(szText, uiLength, uiPosition + mszPattern.Length());
	}
	return uiCount;
}


bool CTextSearch::IsEmpty(void) { return mszPattern.Empty(); }
size CTextSearch::Length(void) { return mszPattern.Length(); }
char* CTextSearch::GetPattern(void) { return mszPattern.Text(); }
bool CTextSearch::IsMatchCase(void) { return mbMatchCase; }
bool CTextSearch::IsWholeWords(void) { return mbWholeWords; }

