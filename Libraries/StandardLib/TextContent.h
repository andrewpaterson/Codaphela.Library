/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __TEXT_CONTENT_H__
#define __TEXT_CONTENT_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"


enum ETextLineSeparator : uint8
{
	TLS_None,  //The last line of text that doesn't end with a separator.
	TLS_LF,
	TLS_CRLF,
	TLS_CR
};


struct STextLine
{
	lsize				uiStart;  //Offset of the line's first character in the text.
	lsize				uiLength;  //Without the separator.
	ETextLineSeparator	eSeparator;
};


typedef CArrayLargeTemplate<STextLine>	CArrayTextLine;


//One side of a text diff (IntelliJ's DocumentContent): the text, split into lines, and a title to show above it (a file
//name, or a file name and revision).  Text from a file, a string or (later) a revision is all the same once it is here.
//
//Lines end at LF, CRLF or a lone CR and don't include the separator.  Text that ends with a separator has no empty line
//after it, so "a\n" and "a" are both one line (the second's separator is TLS_None); empty text has no lines.
//Separators are kept per line so a view can show them.
class CTextContent
{
protected:
	CChars			mszTitle;
	CLargeChars		mszText;
	CArrayTextLine	macLines;

public:
	void				Init(void);
	void				Init(const char* szTitle, const char* szText);
	void				Init(const char* szTitle, const char* szText, lsize uiLength);
	bool				InitFromFile(const char* szTitle, const char* szFilename);
	void				Kill(void);

	void				Set(const char* szTitle, const char* szText);
	void				Set(const char* szTitle, const char* szText, lsize uiLength);
	bool				SetFromFile(const char* szTitle, const char* szFilename);

	char*				GetTitle(void);
	char*				Text(void);
	lsize				Length(void);

	lsize				NumLines(void);
	STextLine*			GetLine(lsize uiLine);
	char*				GetLineText(lsize uiLine);
	lsize				GetLineLength(lsize uiLine);
	lsize				GetLineStart(lsize uiLine);
	lsize				GetLineEnd(lsize uiLine);
	ETextLineSeparator	GetLineSeparator(lsize uiLine);

protected:
	void				SplitLines(void);
};


#endif // __TEXT_CONTENT_H__
