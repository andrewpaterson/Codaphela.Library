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
#ifndef __TEXT_NAVIGATION_H__
#define __TEXT_NAVIGATION_H__
#include "Chars.h"


//Moving through a UTF8 text by characters, lines and words, as text editors do (CTextEditing).
//Positions are bytes, always at the start of a character: a code point, or code points joined by ZWJs.  A carriage
//return and line feed are one character.  Words are runs of letters, digits and underscores (any character that isn't
//ASCII counts as a letter), or of spaces; any other character is a word by itself.
class CTextNavigation
{
protected:
	CChars*		mpszText;  //Not held: the editor's text, read as it is at each call.

public:
	void		Init(CChars* pszText);

	size		CharacterLength(size uiPosition);
	size		NextPosition(size uiPosition);
	size		PreviousPosition(size uiPosition);
	size		FindLineStart(size uiPosition);
	size		FindLineEnd(size uiPosition);
	size		FindLineIndent(size uiLineStart);
	size		CountColumns(size uiLineStart, size uiPosition);
	size		FindColumn(size uiLineStart, size uiColumn);
	size		FindWordLeft(size uiPosition);
	size		FindWordRight(size uiPosition);

	static bool	IsWhitespace(char c);
	static bool	IsWordChar(char c);
};


#endif // __TEXT_NAVIGATION_H__

