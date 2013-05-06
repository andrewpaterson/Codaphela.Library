/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __PLAIN_GRAPHICAL_TEXT_EDITOR_H__
#define __PLAIN_GRAPHICAL_TEXT_EDITOR_H__
#include "CoreLib/PlainTextEditor.h"
#include "TextData.h"


class CPlainGraphicalTextEditor
{
public:
	CPlainTextEditor	mcEditor;
	CTextData*			mpcText;

	void	Init(CTextData* pcText, char* szText);
	void	Kill(void);
	void	Sync(void);

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
	void 	Tab(BOOL bInsert);
	void 	BackTab(void);
};


#endif // __PLAIN_GRAPHICAL_TEXT_EDITOR_H__