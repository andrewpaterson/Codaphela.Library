/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __TEXT_MARKUP_PARSER_H__
#define __TEXT_MARKUP_PARSER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "TextStyle.h"


//A stretch of the plain text and the changes to the style it's drawn in.  uiStart and uiLength are bytes of the UTF8
//plain text.
struct STextStyleRun
{
	size				uiStart;
	size				uiLength;
	STextStyleOverride	sOverride;

	void	Init(size uiStart, size uiLength, STextStyleOverride* psOverride);
};


typedef CArrayTemplate<STextStyleRun>	CArrayTextStyleRun;


//A group of marked up text being read: the changes of every group it's in and its own, and which kind of mark opened
//it (the parser's own number) so the right mark closes it.
struct STextMarkupGroup
{
	STextStyleOverride	sOverride;
	int					iKind;
};


typedef CArrayTemplate<STextMarkupGroup>	CArrayTextMarkupGroup;


//Reads marked up text into plain text and the style runs it's drawn in.  Marks open and close groups; the text in a
//group is in the group's style changes on top of those of the groups it's in.  The runs cover the whole of the plain
//text, in order and without gaps, each in the changes of all the groups around it (a run outside any group changes
//nothing).  Empty runs aren't made.
//
//The marks are the markup's own: a markup is a subclass that recognises them.  At each position of the markup the
//innermost group's close is looked for first, then an escape, then an open; otherwise the character is plain text.
class CTextMarkupParser
{
protected:
	CChars*					mpszText;
	CArrayTextStyleRun*		mpasRuns;
	CArrayTextMarkupGroup	masGroups;
	size					muiRunStart;
	STextStyleOverride		msNone;

public:
			void	Init(void);
	virtual	void	Kill(void);

			bool	Parse(const char* szMarkup, CChars* pszText, CArrayTextStyleRun* pasRuns);

protected:
	//The length in bytes of the mark that opens a group at szMarkup, and its style changes and kind.  0 if there's none.
	virtual	size	ReadOpen(const char* szMarkup, STextStyleOverride* psOverride, int* piKind) = 0;

	//The length in bytes of the mark that closes a group of iKind at szMarkup.  0 if there's none.
	virtual	size	ReadClose(const char* szMarkup, int iKind) = 0;

	//The length in bytes of an escape at szMarkup, with the plain text it stands for appended to pszText.  0 if there's
	//none.  There are none unless the markup has them.
	virtual	size	ReadEscape(const char* szMarkup, CChars* pszText);

			void	EndRun(void);
			STextStyleOverride*	GetOverride(size uiNumGroups);
};


#endif // __TEXT_MARKUP_PARSER_H__

