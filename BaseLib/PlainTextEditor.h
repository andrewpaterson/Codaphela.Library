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
	bool	mbMultiLine;
	size	miTabWidth;  //In Spaces

	CChars	mszText;

	size	miEditPos;
	size	miPageLength;  //Characters to step up or down on page up or page down.

	size	miSelectionAnchor;  // -1 for no selection.
	size	miSelectionFloating;  // -1 for no selection.
	size	miUpDownCharCount;

	void	Init(void);
	void 	Init(char* szText);
	void 	Kill(void);

	void 	Up(void) override;
	void 	Down(void) override;
	void 	Left(void) override;
	void 	Right(void) override;
	void 	WordLeft(void) override;
	void 	WordRight(void) override;
	void 	Delete(void) override;
	void 	Backspace(void) override;
	void 	Enter(void) override;
	void 	Printable(char c, bool bInsert) override;
	void 	Home(void) override;
	void 	End(void) override;
	void 	DocumentHome(void) override;
	void 	DocumentEnd(void) override;
	void 	PageUp(void) override;
	void 	PageDown(void) override;
	void 	SelectUp(void) override;
	void 	SelectDown(void) override;
	void 	SelectLeft(void) override;
	void 	SelectRight(void) override;
	void 	SelectWordLeft(void) override;
	void 	SelectWordRight(void) override;
	void 	SelectHome(void) override;
	void 	SelectEnd(void) override;
	void 	SelectDocumentHome(void) override;
	void 	SelectDocumentEnd(void) override;
	void 	SelectPageUp(void) override;
	void 	SelectPageDown(void) override;
	void 	SelectWholeWord(void) override;
	void	SelectAll(void) override;
	void	ClearSelection(void) override;
	void 	Cut(CChars* pszDest) override;
	void 	Copy(CChars* pszDest) override;
	void 	Paste(char* szSource) override;
	void 	CutLine(CChars* pszDest) override;
	void 	Duplicate(void) override;
	void 	DeleteWordRemainingRight(void) override;
	void 	DeleteWordRemainingLeft(void) override;
	void 	DeleteLine(void) override;
	void 	EndEnter(void) override;
	void 	HomeEnter(void) override;
	void 	Space(bool bInsert) override;
	void 	Tab(bool bInsert) override;
	void 	BackTab(void) override;

	void 	Printable(char c);
	void 	Space(void);
	void 	Tab(void);

	char*	Text(void);
	char*	Line(CChars* pszDest, bool bFromEditPosition = true);
	size	Length(void);
	void	SetPageLength(size iPageSize);
	void	SetPosition(size iPosition);

	size	GetCharCountAlongLine(size iPos);
	SInt2	GetEditPosition(void);
	SInt2	GetPosition(size iPosition);

protected:
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

