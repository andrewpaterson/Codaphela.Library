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
#ifndef __TEXT_SEARCH_H__
#define __TEXT_SEARCH_H__
#include "Chars.h"
#include "Regex.h"


#define TEXT_SEARCH_NOT_FOUND	MAX_SIZE


//Finds a piece of text in a longer one, as an editor's Find does: every place it is, whatever the case of its letters
//unless the case must match, and with whole words only the places where it isn't part of a longer word (the character
//before it and the one after it aren't letters, digits or underscores, unless its own first or last isn't either).
//The text searched needn't end with a zero and may have zeros in it.  An empty pattern is never found.
//
//The pattern can be a regular expression instead (CRegex): then what is found is as long as what it matched
//(GetFoundLength, after each Find), never nothing, and with whole words only where what it matched isn't part of a
//longer word.  One that isn't a regular expression finds nothing: IsValid and GetError say so.
//
//What a find is to be replaced with (AppendReplacement, for what Find last found) is the replacement as it is, or for
//a regular expression with its $1 for a group and its \n and \t (CRegex::AppendReplacement).
class CTextSearch
{
protected:
	CChars	mszPattern;  //In lower case unless the case must match.
	bool	mbMatchCase;
	bool	mbWholeWords;
	char	mcFirst;  //The pattern's first character and, unless the case must match, its upper case.
	char	mcFirstUpper;
	bool	mbRegex;
	bool	mbValid;  //Not a regular expression that didn't compile.
	CRegex	mcRegex;
	SRegexMatch	msFound;  //What Find last found: all of it, and a regular expression's groups.

public:
	void		Init(const char* szPattern, bool bMatchCase, bool bWholeWords, bool bRegex = false);
	void		Kill(void);

	size		Find(const char* szText, size uiLength, size uiFrom = 0);
	size		Count(const char* szText, size uiLength);
	bool		IsEmpty(void) const;
	size		Length(void) const;
	const char*	GetPattern(void) const;
	bool		IsMatchCase(void) const;
	bool		IsWholeWords(void) const;
	bool		IsRegex(void) const;
	bool		IsValid(void) const;
	const char*	GetError(void);
	size		GetErrorPosition(void) const;
	bool		GaveUp(void) const;
	size		GetFoundLength(void) const;
	bool		CheckReplacement(const char* szReplacement);
	void		AppendReplacement(const char* szText, const char* szReplacement, CChars* pszDest);

protected:
	bool		IsAt(const char* szText, size uiLength, size uiPosition);
	size		FindRegex(const char* szText, size uiLength, size uiFrom);
	size		FindText(const char* szText, size uiLength, size uiFrom);
};


#endif // __TEXT_SEARCH_H__

