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
#ifndef __TEXT_FIELD_H__
#define __TEXT_FIELD_H__
#include "PlainGraphicalTextEditor.h"
#include "Label.h"
#include "GraphicalTextEditor.h"
#include "ComponentInput.h"


//CMotionListener listener should become pointer listener and CKeyListener should become input listener
class CTextField : public CLabel
{
public:
	BASE_FUNCTIONS(CTextField);

	CPlainGraphicalTextEditor	mcEditor;

	void 	Init(CViewport* pcViewport, char* szString, BOOL bDefaultBorder = TRUE, BOOL bDefaultBackground = TRUE);
	void 	Kill(void);

	BOOL	Draw(void);
	void	Layout(SInt2 sPosition, SInt2 sAreaSize);

	void	FocusGained(void);
	void	FocusLost(void);

	void	TimerAlarm(float fOversleep);

	void	MovedInto(void);
	void	MovedOutof(void);

	void 	Up(void) {mcEditor.Up();};
	void 	Down(void) {mcEditor.Down();};
	void 	Left(void) {mcEditor.Left();};
	void 	Right(void) {mcEditor.Right();};
	void 	WordLeft(void) {mcEditor.WordLeft();};
	void 	WordRight(void) {mcEditor.WordRight();};
	void 	Home(void) {mcEditor.Home();};
	void 	End(void) {mcEditor.End();};
	void 	DocumentHome(void) {mcEditor.DocumentHome();};
	void 	DocumentEnd(void) {mcEditor.DocumentEnd();};
	void 	PageUp(void) {mcEditor.PageUp();};
	void 	PageDown(void) {mcEditor.PageDown();};
	void 	LineUp(void);
	void 	LineDown(void);

	void 	SelectUp(void) {mcEditor.SelectUp();};
	void 	SelectDown(void) {mcEditor.SelectDown();};
	void 	SelectLeft(void) {mcEditor.SelectLeft();};
	void 	SelectRight(void) {mcEditor.SelectRight();};
	void 	SelectWordLeft(void) {mcEditor.SelectWordLeft();};
	void 	SelectWordRight(void) {mcEditor.SelectWordRight();};
	void 	SelectHome(void) {mcEditor.SelectHome();};
	void 	SelectEnd(void) {mcEditor.SelectEnd();};
	void 	SelectDocumentHome(void) {mcEditor.SelectDocumentHome();};
	void 	SelectDocumentEnd(void) {mcEditor.SelectDocumentEnd();};
	void 	SelectPageUp(void) {mcEditor.SelectPageUp();};
	void 	SelectPageDown(void) {mcEditor.SelectPageDown();};
	void 	SelectWholeWord(void) {mcEditor.SelectWholeWord();};
	void 	SelectAll(void) {mcEditor.SelectAll();};
	void 	ClearSelection(void) {mcEditor.ClearSelection();};

	void 	Printable(char c, BOOL bInsert) {mcEditor.Printable(c, bInsert);};
	void 	Space(BOOL bInsert) {mcEditor.Space(bInsert);};
	void 	Tab(BOOL bInsert) {mcEditor.Tab(bInsert);};
	void 	BackTab(void) {mcEditor.BackTab();};
	void 	Enter(void) {mcEditor.Enter();};
	void 	HomeEnter(void) {mcEditor.HomeEnter();};
	void 	EndEnter(void) {mcEditor.EndEnter();};

	void 	Delete(void) {mcEditor.Delete();};
	void 	Backspace(void) {mcEditor.Backspace();};
	void 	Cut(CChars* pszDest) {mcEditor.Cut(pszDest);};
	void 	Copy(CChars* pszDest) {mcEditor.Copy(pszDest);};
	void 	Paste(char* szSource) {mcEditor.Paste(szSource);};
	void 	CutLine(CChars* pszDest) {mcEditor.CutLine(pszDest);};
	void 	Duplicate(void) {mcEditor.Duplicate();};
	void 	DeleteWordRemainingRight(void) {mcEditor.DeleteWordRemainingRight();};
	void 	DeleteWordRemainingLeft(void) {mcEditor.DeleteWordRemainingLeft();};
	void 	DeleteLine(void) {mcEditor.DeleteLine();};

	void 	ActivateComponent(int x, int y);

	void 	ExecuteComponent(int x, int y) {};
	void 	ContextMenu(int x, int y) {};
	void 	StartDragScroll(int x, int y) {};
	void 	ContDragScroll(int x, int y) {};
	void 	StopDragScroll(int x, int y) {};
	void 	StartDragSelect(int x, int y) {};
	void 	ContDragSelect(int x, int y) {};
	void 	StopDragSelect(int x, int y) {};
	void 	Hover(int x, int y) {};
};


#endif // __TEXT_FIELD_H__

