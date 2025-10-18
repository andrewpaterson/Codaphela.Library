/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __GRAPHICAL_TEXT_EDITOR_H__
#define __GRAPHICAL_TEXT_EDITOR_H__
#include "BaseLib/Chars.h"
#include "BaseLib/TextEditor.h"
#include "TextData.h"


class CGraphicalTextEditor : public CTextEditor
{
public:
	bool		mbMultiLine;
	int			miTabWidth;  //In Spaces

	CTextData*	mpcText;

	bool		mbInsert;

	SInt2		msEditPos;  //In pixels
	int			miCenterLineX;  //For moving up and down (including page up and page down).  Resets to edit pos on anything other than up or down.
	int			miPageLength;  //Pixels to step up or down on page up or page down.

	SInt2		msSelectionStart;  //Both -1 for no selection.
	SInt2		msSelectionEnd;  //Both -1 for no selection.

	void	Init(CTextData* pcText);
	void	Kill(void);

	void 	Up(void);
	void 	Down(void);
	void 	Left(void);
	void 	Right(void);
	void 	WordLeft(void);
	void 	WordRight(void);
	void 	Delete(void);
	void 	Backspace(void);
	void 	Enter(void);
	void 	Printable(char c, bool bInsert);
	void 	Home(void);
	void 	End(void);
	void 	DocumentHome(void);
	void 	DocumentEnd(void);
	void 	PageUp(void);
	void 	PageDown(void);
	void 	LineUp(void);
	void 	LineDown(void);
	void 	SelectUp(void);
	void 	SelectDown(void);
	void 	SelectLeft(void);
	void 	SelectRight(void);
	void 	SelectWordLeft(void);
	void 	SelectWordRight(void);
	void 	SelectHome(void);
	void 	SelectEnd(void);
	void 	SelectDocumentHome(void);
	void 	SelectDocumentEnd(void);
	void 	SelectPageUp(void);
	void 	SelectPageDown(void);
	void 	SelectWholeWord(void);
	void	SelectAll(void);
	void	ClearSelection(void);
	void 	Cut(CChars* pszDest);
	void 	Copy(CChars* pszDest);
	void 	Paste(char* szSource);
	void 	CutLine(CChars* pszDest);
	void 	Duplicate(void);
	void 	DeleteWordRemainingRight(void);
	void 	DeleteWordRemainingLeft(void);
	void 	DeleteLine(void);
	void 	EndEnter(void);
	void 	HomeEnter(void);
	void 	Space(bool bInsert);
	void 	Tab(bool bInsert);
	void 	BackTab(void);

	SInt2	FindLeft(SInt2 sPos);
	SInt2	FindRight(SInt2 sPos);
	SInt2	FindEndOfWord(void);
	SInt2	FindStartOfWord(void);
	SInt2	FindUp(SInt2 sPos);
	SInt2	FindDown(SInt2 sPos);
	SInt2	FindStartOfLine(SInt2 sPos, bool bSkipWhiteSpace);
	SInt2	FindEndOfLine(SInt2 sPos, bool bSkipWhiteSpace);
	void	TouchSelection(void);
	SInt2	DeleteSubstring(SInt2 sStartInclusive, SInt2 sEndExclusive);
};


#endif // __GRAPHICAL_TEXT_EDITOR_H__

