/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __PLAIN_TEXT_EDITOR_H__
#define __PLAIN_TEXT_EDITOR_H__
#include "Chars.h"
#include "Int2.h"
#include "TextEditor.h"


class CPlainTextEditor : public CTextEditor
{
public:
	bool		mbMultiLine;
	size		miTabWidth;  //In Spaces

	CChars		mszText;

	size		miEditPos;
	size		miPageLength;  //Characters to step up or down on page up or page down.

	size		miSelectionAnchor;  // -1 for no selection.
	size		miSelectionFloating;  // -1 for no selection.
	size		miUpDownCharCount;

	void	Init(void);
	void 	Init(char* szText);
	void 	Kill(void);

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
	void 	Printable(char c);
	void 	Home(void);
	void 	End(void);
	void 	DocumentHome(void);
	void 	DocumentEnd(void);
	void 	PageUp(void);
	void 	PageDown(void);
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
	void 	Space(void);
	void 	Tab(bool bInsert);
	void 	Tab(void);
	void 	BackTab(void);

	char*	Text(void);
	size	Length(void);
	void	SetPageLength(size iPageSize);
	void	SetPosition(size iPosition);

	size	GetCharCountAlongLine(size iPos);
	SInt2	GetEditPosition(void);
	SInt2	GetPosition(size iPosition);

private:
	size	FindUp(size iPos);
	size	FindDown(size iPos);
	size	FindLeft(size iPos);
	size	FindRight(size iPos);
	size	FindHome(size iPos);
	size	FindEnd(size iPos);
	size	FindDocumentHome(void);
	size	FindDocumentEnd(void);
	size	FindPageUp(size iPos);
	size	FindPageDown(size iPos);

	size	FindWordLeft(size iPos);
	size	FindWordRight(size iPos);
	bool 	CanGoLeft(size iPos);
	bool 	CanGoRight(size iPos);
	bool	IsWhitespace(char cCurrent);
	bool	IsNormalChar(char cCurrent);
	bool	IsSelected(void);
	size	FindStartOfLine(size iPos);
	size	FindEndOfLine(size iPos);
	size	FindPositionAlongLine(size iStartOfLine, size iCharCount);
	size	CountChar(size iCharCount, char c);
	void	ResetUpDown(void);
	void	UpdateUpDown(size iPos);
	void	FixSelection(void);
	void	BeginSelection(void);
	void	EndSelection(void);
	size	DeleteSelection(void);
	void	OverwriteSelection(char* szText);
};


#endif // __PLAIN_TEXT_EDITOR_H__

