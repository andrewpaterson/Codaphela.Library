/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __PLAIN_TEXT_EDITOR_H__
#define __PLAIN_TEXT_EDITOR_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Int2.h"
#include "TextEditor.h"


class CPlainTextEditor : public CTextEditor
{
public:
	BOOL		mbMultiLine;
	int			miTabWidth;  //In Spaces

	CChars		mszText;

	int		miEditPos;
	int		miPageLength;  //Characters to step up or down on page up or page down.

	int		miSelectionAnchor;  // -1 for no selection.
	int		miSelectionFloating;  // -1 for no selection.
	int		miUpDownCharCount;

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
	void 	Printable(char c, BOOL bInsert);
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
	void 	Space(BOOL bInsert);
	void 	Space(void);
	void 	Tab(BOOL bInsert);
	void 	Tab(void);
	void 	BackTab(void);

	char*	Text(void);
	int		Length(void);
	void	SetPageLength(int iPageSize);
	void	SetPosition(int iPosition);

	int		GetCharCountAlongLine(int iPos);
	SInt2	GetEditPosition(void);
	SInt2	GetPosition(int iPosition);

private:
	int		FindUp(int iPos);
	int		FindDown(int iPos);
	int		FindLeft(int iPos);
	int		FindRight(int iPos);
	int		FindHome(int iPos);
	int		FindEnd(int iPos);
	int		FindDocumentHome(void);
	int		FindDocumentEnd(void);
	int		FindPageUp(int iPos);
	int		FindPageDown(int iPos);

	int		FindWordLeft(int iPos);
	int		FindWordRight(int iPos);
	BOOL 	CanGoLeft(int iPos);
	BOOL 	CanGoRight(int iPos);
	BOOL	IsWhiteSpace(char cCurrent);
	BOOL	IsNormalChar(char cCurrent);
	BOOL	IsSelected(void);
	int		FindStartOfLine(int iPos);
	int		FindEndOfLine(int iPos);
	int		FindPositionAlongLine(int iStartOfLine, int iCharCount);
	int		CountChar(int iCharCount, char c);
	void	ResetUpDown(void);
	void	UpdateUpDown(int iPos);
	void	FixSelection(void);
	void	BeginSelection(void);
	void	EndSelection(void);
	int		DeleteSelection(void);
	void	OverwriteSelection(char* szText);
};


#endif // !__PLAIN_TEXT_EDITOR_H__

