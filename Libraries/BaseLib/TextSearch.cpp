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
void CTextSearch::Init(const char* szPattern, bool bMatchCase, bool bWholeWords, bool bRegex)
{
	mszPattern.Init(szPattern);
	mbMatchCase = bMatchCase;
	mbWholeWords = bWholeWords;
	mbRegex = bRegex;
	mbValid = true;
	msFound.uiStart = 0;
	msFound.uiEnd = 0;
	mcRegex.Init();
	if (mbRegex)
	{
		//A regular expression keeps its case: \D isn't \d.  The expression matches without it.
		mbValid = mszPattern.Empty() || mcRegex.Compile(szPattern, bMatchCase);
		mcFirst = '\0';
		mcFirstUpper = '\0';
		return;
	}
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
	mcRegex.Kill();
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
// TEXT_SEARCH_NOT_FOUND if it isn't.  GetFoundLength is how much of the
// text it is.
//
//////////////////////////////////////////////////////////////////////////
size CTextSearch::Find(const char* szText, size uiLength, size uiFrom)
{
	size	uiPosition;

	uiPosition = mbRegex ? FindRegex(szText, uiLength, uiFrom) : FindText(szText, uiLength, uiFrom);
	if (!mbRegex && (uiPosition != TEXT_SEARCH_NOT_FOUND))
	{
		msFound.uiStart = uiPosition;
		msFound.uiEnd = uiPosition + mszPattern.Length();
	}
	return uiPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// The regular expression's first match at or after uiFrom that isn't of
// nothing and, with whole words, isn't part of a longer word.
//
//////////////////////////////////////////////////////////////////////////
size CTextSearch::FindRegex(const char* szText, size uiLength, size uiFrom)
{
	size	uiPosition;

	if (!mbValid || mszPattern.Empty())
	{
		return TEXT_SEARCH_NOT_FOUND;
	}
	for (uiPosition = uiFrom; (uiPosition <= uiLength) && mcRegex.Find(szText, uiLength, uiPosition, &msFound); uiPosition = msFound.uiStart + 1)
	{
		if (msFound.uiEnd == msFound.uiStart)
		{
			continue;
		}
		if (mbWholeWords)
		{
			if ((msFound.uiStart > 0) && IsTextSearchWordCharacter(szText[msFound.uiStart]) && IsTextSearchWordCharacter(szText[msFound.uiStart - 1]))
			{
				continue;
			}
			if ((msFound.uiEnd < uiLength) && IsTextSearchWordCharacter(szText[msFound.uiEnd - 1]) && IsTextSearchWordCharacter(szText[msFound.uiEnd]))
			{
				continue;
			}
		}
		return msFound.uiStart;
	}
	msFound.uiStart = 0;
	msFound.uiEnd = 0;
	return TEXT_SEARCH_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// The text's.
//
//////////////////////////////////////////////////////////////////////////
size CTextSearch::FindText(const char* szText, size uiLength, size uiFrom)
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
		uiPosition = Find(szText, uiLength, uiPosition + GetFoundLength());
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// What Find last found is replaced with this: szReplacement, or for a
// regular expression what CRegex::AppendReplacement makes of it.  szText
// is the text Find was given.
//
//////////////////////////////////////////////////////////////////////////
void CTextSearch::AppendReplacement(const char* szText, const char* szReplacement, CChars* pszDest)
{
	if (mbRegex)
	{
		mcRegex.AppendReplacement(szText, &msFound, szReplacement, pszDest);
	}
	else
	{
		pszDest->Append(szReplacement);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a replacement can be used: a regular expression's must name
// only the groups it has (GetError says which it doesn't).
//
//////////////////////////////////////////////////////////////////////////
bool CTextSearch::CheckReplacement(const char* szReplacement)
{
	return !mbRegex || !mbValid || mcRegex.CheckReplacement(szReplacement);
}


bool CTextSearch::IsEmpty(void) const { return mszPattern.Empty(); }
bool CTextSearch::IsRegex(void) const { return mbRegex; }
bool CTextSearch::IsValid(void) const { return mbValid; }
const char* CTextSearch::GetError(void) { return mcRegex.GetError(); }
size CTextSearch::GetErrorPosition(void) const { return mcRegex.GetErrorPosition(); }
bool CTextSearch::GaveUp(void) const { return mbRegex && mcRegex.GaveUp(); }
size CTextSearch::GetFoundLength(void) const { return msFound.uiEnd - msFound.uiStart; }
size CTextSearch::Length(void) const { return mszPattern.Length(); }
const char* CTextSearch::GetPattern(void) const { return mszPattern.Text(); }
bool CTextSearch::IsMatchCase(void) const { return mbMatchCase; }
bool CTextSearch::IsWholeWords(void) const { return mbWholeWords; }

