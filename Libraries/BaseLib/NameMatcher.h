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
#ifndef __NAME_MATCHER_H__
#define __NAME_MATCHER_H__
#include "Chars.h"


//The most pieces of a name a match highlights, and the longest pattern and name matched.
#define NAME_MATCHER_RANGES		16
#define NAME_MATCHER_PATTERN	64
#define NAME_MATCHER_NAME		512


//A run of a name's characters a pattern matched, to be highlighted.
struct SNameMatchRange
{
	uint16	uiStart;
	uint16	uiLength;
};


//How well a name matched: higher is better.  The ranges are in the name, or in a path's file name.
struct SNameMatch
{
	int32				iScore;
	size				uiNumRanges;
	SNameMatchRange		asRanges[NAME_MATCHER_RANGES];
};


//Matches names against what is typed to find them, like IntelliJ IDEA's Go to Class and Go to File: the pattern's
//characters are found in the name in order, ignoring case, each either straight after the one before or at the start
//of a word of the name ("CTA" and "TeArea" find CTextArea, "tarea" finds TextArea).  The first must start a word.
//Words start at the name's start, at an upper case letter after a lower case one or a digit, at the last upper case
//letter of a run of them followed by a lower case one (the P of HTMLParser), at a digit after a letter, and at a
//letter or digit after anything else (an underscore, a dot).  A '*' or a space in the pattern lets the next character
//be anywhere further on.
//
//A path matches (MatchPath) when the pattern's last piece, after its last '/', matches the file name and each piece
//before it matches one of the path's folders, in order ("wl/tf" finds WindowLib/TextField.h).
//
//The score puts exact names first, then names it is the start of, then fewer, longer runs nearer the start and in the
//same case, then shorter names.
class CNameMatcher
{
protected:
	CChars	mszPattern;

public:
	void	Init(const char* szPattern);
	void	Kill(void);

	bool	IsEmpty(void);
	char*	GetPattern(void);
	bool	Match(const char* szName, SNameMatch* psDest);
	bool	Match(const char* szName, size uiLength, const char* szPattern, size uiPatternLength, SNameMatch* psDest);
	bool	MatchPath(const char* szPath, SNameMatch* psDest);

protected:
	bool	Find(const char* szName, size uiLength, const char* szPattern, size uiPatternLength, size uiPattern, size uiName, bool bAnywhere, size* auiMatched, uint8* auiFailed);
	int32	Score(const char* szName, size uiLength, const char* szPattern, size uiPatternLength, size* auiMatched, SNameMatch* psDest);
};


bool IsNameWordStart(const char* szName, size uiPosition);


#endif // __NAME_MATCHER_H__

