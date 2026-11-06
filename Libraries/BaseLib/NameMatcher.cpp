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
#include <stdlib.h>
#include <string.h>
#include "StringHelper.h"
#include "NameMatcher.h"


//What the pattern's character is matched against: a character of the name that may start a word.
#define NAME_MATCHER_NOT_MATCHED	((size)-1)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsNameUpper(char c) { return (c >= 'A') && (c <= 'Z'); }
static bool IsNameLower(char c) { return (c >= 'a') && (c <= 'z'); }
static bool IsNameDigit(char c) { return (c >= '0') && (c <= '9'); }
static bool IsNameAlphanumeric(char c) { return IsNameUpper(c) || IsNameLower(c) || IsNameDigit(c) || ((unsigned char)c >= 0x80); }
static bool IsNameWildcard(char c) { return (c == '*') || (c == ' '); }


//////////////////////////////////////////////////////////////////////////
//
// Whether a word of the name starts at uiPosition (see CNameMatcher).
//
//////////////////////////////////////////////////////////////////////////
bool IsNameWordStart(const char* szName, size uiPosition)
{
	char	c;
	char	cBefore;

	c = szName[uiPosition];
	if (!IsNameAlphanumeric(c))
	{
		return false;
	}
	if (uiPosition == 0)
	{
		return true;
	}
	cBefore = szName[uiPosition - 1];
	if (!IsNameAlphanumeric(cBefore))
	{
		return true;
	}
	if (IsNameUpper(c))
	{
		if (IsNameLower(cBefore) || IsNameDigit(cBefore))
		{
			return true;
		}
		//The last capital of a run of them before a lower case letter: HTMLParser's P.
		return IsNameUpper(cBefore) && IsNameLower(szName[uiPosition + 1]);
	}
	if (IsNameDigit(c))
	{
		return !IsNameDigit(cBefore);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The pattern is kept as it was typed; leading and trailing spaces are
// dropped.
//
//////////////////////////////////////////////////////////////////////////
void CNameMatcher::Init(const char* szPattern)
{
	mszPattern.Init(szPattern == NULL ? "" : szPattern);
	mszPattern.StripWhitespace();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNameMatcher::Kill(void)
{
	mszPattern.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An empty pattern matches every name, all with the same score but for
// their lengths.
//
//////////////////////////////////////////////////////////////////////////
bool CNameMatcher::IsEmpty(void)
{
	return mszPattern.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CNameMatcher::GetPattern(void)
{
	return mszPattern.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNameMatcher::Match(const char* szName, SNameMatch* psDest)
{
	return Match(szName, StrLen(szName), mszPattern.Text(), mszPattern.Length(), psDest);
}


//////////////////////////////////////////////////////////////////////////
//
// szName's first uiLength characters against the pattern's first
// uiPatternLength.  Returns false if they don't match.
//
//////////////////////////////////////////////////////////////////////////
bool CNameMatcher::Match(const char* szName, size uiLength, const char* szPattern, size uiPatternLength, SNameMatch* psDest)
{
	size	auiMatched[NAME_MATCHER_PATTERN];
	uint8*	auiFailed;
	size	uiFailedSize;
	bool	bFound;

	psDest->iScore = 0;
	psDest->uiNumRanges = 0;
	if (uiPatternLength == 0)
	{
		psDest->iScore = -(int32)uiLength;
		return true;
	}
	if ((uiPatternLength > NAME_MATCHER_PATTERN) || (uiLength > NAME_MATCHER_NAME))
	{
		return false;
	}

	//Positions already known not to match, so a long name can't take long: two bits (anywhere or not) for each
	//pattern character at each name position.
	uiFailedSize = (uiPatternLength * (uiLength + 1) * 2 + 7) / 8;
	auiFailed = (uint8*)malloc(uiFailedSize);
	memset(auiFailed, 0, uiFailedSize);
	bFound = Find(szName, uiLength, szPattern, uiPatternLength, 0, 0, false, auiMatched, auiFailed);
	free(auiFailed);
	if (!bFound)
	{
		return false;
	}
	psDest->iScore = Score(szName, uiLength, szPattern, uiPatternLength, auiMatched, psDest);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Finds where the pattern's characters from uiPattern on match the name
// from uiName on.  bAnywhere lets the next character match anywhere (it
// came after a wildcard); without it, it must follow the last matched
// character or start a word.  The first way found that keeps the most
// characters together is kept in auiMatched.
//
//////////////////////////////////////////////////////////////////////////
bool CNameMatcher::Find(const char* szName, size uiLength, const char* szPattern, size uiPatternLength, size uiPattern, size uiName, bool bAnywhere, size* auiMatched, uint8* auiFailed)
{
	char	c;
	size	i;
	size	uiBit;
	bool	bContinues;

	if (uiPattern == uiPatternLength)
	{
		return true;
	}
	c = szPattern[uiPattern];
	if (IsNameWildcard(c))
	{
		auiMatched[uiPattern] = NAME_MATCHER_NOT_MATCHED;
		return Find(szName, uiLength, szPattern, uiPatternLength, uiPattern + 1, uiName, true, auiMatched, auiFailed);
	}

	uiBit = ((uiPattern * (uiLength + 1)) + uiName) * 2 + (bAnywhere ? 1 : 0);
	if (auiFailed[uiBit / 8] & (1 << (uiBit % 8)))
	{
		return false;
	}

	c = ToLower(c);

	//Straight after the character before.
	bContinues = (uiPattern > 0) && (auiMatched[uiPattern - 1] != NAME_MATCHER_NOT_MATCHED) && (auiMatched[uiPattern - 1] + 1 == uiName);
	if (bContinues && (uiName < uiLength) && (ToLower(szName[uiName]) == c))
	{
		auiMatched[uiPattern] = uiName;
		if (Find(szName, uiLength, szPattern, uiPatternLength, uiPattern + 1, uiName + 1, false, auiMatched, auiFailed))
		{
			return true;
		}
	}

	//At a word start further on, or anywhere after a wildcard.
	for (i = bContinues ? uiName + 1 : uiName; i < uiLength; i++)
	{
		if ((ToLower(szName[i]) == c) && (bAnywhere || IsNameWordStart(szName, i)))
		{
			auiMatched[uiPattern] = i;
			if (Find(szName, uiLength, szPattern, uiPatternLength, uiPattern + 1, i + 1, false, auiMatched, auiFailed))
			{
				return true;
			}
		}
	}

	auiFailed[uiBit / 8] |= (uint8)(1 << (uiBit % 8));
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Fills the match's ranges from the matched positions and scores it.
//
//////////////////////////////////////////////////////////////////////////
int32 CNameMatcher::Score(const char* szName, size uiLength, const char* szPattern, size uiPatternLength, size* auiMatched, SNameMatch* psDest)
{
	int32				iScore;
	size				i;
	size				uiPosition;
	size				uiMatched;
	SNameMatchRange*	psRange;
	size				uiFirst;

	iScore = 0;
	uiMatched = 0;
	uiFirst = NAME_MATCHER_NOT_MATCHED;
	psRange = NULL;
	for (i = 0; i < uiPatternLength; i++)
	{
		uiPosition = auiMatched[i];
		if (uiPosition == NAME_MATCHER_NOT_MATCHED)
		{
			continue;
		}
		if (uiFirst == NAME_MATCHER_NOT_MATCHED)
		{
			uiFirst = uiPosition;
		}
		uiMatched++;
		iScore += 10;
		if (szName[uiPosition] == szPattern[i])
		{
			iScore += 2;
		}
		if (psRange && (psRange->uiStart + psRange->uiLength == uiPosition))
		{
			psRange->uiLength++;
		}
		else
		{
			iScore -= 15;
			if (psDest->uiNumRanges < NAME_MATCHER_RANGES)
			{
				psRange = &psDest->asRanges[psDest->uiNumRanges];
				psDest->uiNumRanges++;
				psRange->uiStart = (uint16)uiPosition;
				psRange->uiLength = 1;
			}
			else
			{
				psRange = NULL;
			}
		}
	}

	if (uiFirst == 0)
	{
		iScore += 500;
		if (uiMatched == uiLength)
		{
			iScore += 1000;
			if (memcmp(szName, szPattern, uiLength) == 0)
			{
				iScore += 1000;
			}
		}
	}
	else if (uiFirst != NAME_MATCHER_NOT_MATCHED)
	{
		iScore -= (int32)uiFirst;
	}
	iScore -= (int32)uiLength;
	return iScore;
}


//////////////////////////////////////////////////////////////////////////
//
// The pattern's last piece matches the file name, and each piece before
// it one of the path's folders, in order.  '\' is a '/'.  The ranges are
// in the file name.  A pattern without a '/' only matches the file name.
//
//////////////////////////////////////////////////////////////////////////
bool CNameMatcher::MatchPath(const char* szPath, SNameMatch* psDest)
{
	const char*	szPattern;
	size		uiPatternLength;
	size		uiPathLength;
	size		uiFileStart;
	size		uiPieceStart;
	size		uiPieceEnd;
	size		uiFolderStart;
	size		uiFolderEnd;
	size		i;
	SNameMatch	sFolder;
	int32		iFolders;
	bool		bFound;

	szPattern = mszPattern.Text();
	uiPatternLength = mszPattern.Length();
	uiPathLength = StrLen(szPath);

	uiFileStart = uiPathLength;
	while ((uiFileStart > 0) && (szPath[uiFileStart - 1] != '/') && (szPath[uiFileStart - 1] != '\\'))
	{
		uiFileStart--;
	}
	uiPieceStart = uiPatternLength;
	while ((uiPieceStart > 0) && (szPattern[uiPieceStart - 1] != '/') && (szPattern[uiPieceStart - 1] != '\\'))
	{
		uiPieceStart--;
	}
	if (!Match(&szPath[uiFileStart], uiPathLength - uiFileStart, &szPattern[uiPieceStart], uiPatternLength - uiPieceStart, psDest))
	{
		return false;
	}

	//Each folder piece, first to last, matches a folder after the one the piece before matched.
	iFolders = 0;
	uiFolderStart = 0;
	uiPieceEnd = 0;
	for (i = 0; i < uiPieceStart; i = uiPieceEnd + 1)
	{
		uiPieceEnd = i;
		while ((uiPieceEnd < uiPieceStart) && (szPattern[uiPieceEnd] != '/') && (szPattern[uiPieceEnd] != '\\'))
		{
			uiPieceEnd++;
		}
		if (uiPieceEnd == i)
		{
			continue;
		}
		bFound = false;
		while (uiFolderStart < uiFileStart)
		{
			uiFolderEnd = uiFolderStart;
			while ((uiFolderEnd < uiFileStart) && (szPath[uiFolderEnd] != '/') && (szPath[uiFolderEnd] != '\\'))
			{
				uiFolderEnd++;
			}
			sFolder.uiNumRanges = 0;
			bFound = (uiFolderEnd > uiFolderStart) && Match(&szPath[uiFolderStart], uiFolderEnd - uiFolderStart, &szPattern[i], uiPieceEnd - i, &sFolder);
			uiFolderStart = uiFolderEnd + 1;
			if (bFound)
			{
				iFolders += sFolder.iScore / 4;
				break;
			}
		}
		if (!bFound)
		{
			return false;
		}
	}
	psDest->iScore += iFolders;
	return true;
}

