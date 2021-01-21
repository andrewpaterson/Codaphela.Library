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
#ifndef __COMPONENT_LISTENER_H__
#define __COMPONENT_LISTENER_H__
#include "BaseLib/ArrayTemplate.h"
#include "CoreLib/TextEditor.h"


class CComponentListener : public CTextEditor, public CListener
{
public:
	CONSTRUCTABLE(CComponentListener);

	virtual void Up(void) {};
	virtual void Down(void) {};
	virtual void Left(void) {};
	virtual void Right(void) {};
	virtual void WordLeft(void) {};
	virtual void WordRight(void) {};
	virtual void Home(void) {};
	virtual void End(void) {};
	virtual void DocumentHome(void) {};
	virtual void DocumentEnd(void) {};
	virtual void PageUp(void) {};
	virtual void PageDown(void) {};
	virtual void LineUp(void) {};
	virtual void LineDown(void) {};

	//Selection
	virtual void SelectUp(void) {};
	virtual void SelectDown(void) {};
	virtual void SelectLeft(void) {};
	virtual void SelectRight(void) {};
	virtual void SelectWordLeft(void) {};
	virtual void SelectWordRight(void) {};
	virtual void SelectHome(void) {};
	virtual void SelectEnd(void) {};
	virtual void SelectDocumentHome(void) {};
	virtual void SelectDocumentEnd(void) {};
	virtual void SelectPageUp(void) {};
	virtual void SelectPageDown(void) {};
	virtual void SelectWholeWord(void) {};
	virtual void SelectAll(void) {};
	virtual void ClearSelection(void) {};

	//Modification
	virtual void Printable(char c, BOOL bInsert) {};
	virtual void Space(BOOL bInsert) {};
	virtual void Tab(BOOL bInsert) {};
	virtual void BackTab(void) {};
	virtual void Enter(void) {};
	virtual void HomeEnter(void) {};
	virtual void EndEnter(void) {};

	//Modification
	virtual void Delete(void) {};
	virtual void Backspace(void) {};
	virtual void Cut(CChars* pszDest) {};
	virtual void Copy(CChars* pszDest) {};
	virtual void Paste(char* szSource) {};
	virtual void CutLine(CChars* pszDest) {};
	virtual void Duplicate(void) {};
	virtual void DeleteWordRemainingRight(void) {};
	virtual void DeleteWordRemainingLeft(void) {};
	virtual void DeleteLine(void) {};

	//Mouse/Miscellaneous
	virtual void ActivateComponent(int x, int y) {};
	virtual void ExecuteComponent(int x, int y) {};
	virtual void ContextMenu(int x, int y) {};
	virtual void StartDragScroll(int x, int y) {};
	virtual void ContDragScroll(int x, int y) {};
	virtual void StopDragScroll(int x, int y) {};
	virtual void StartDragSelect(int x, int y) {};
	virtual void ContDragSelect(int x, int y) {};
	virtual void StopDragSelect(int x, int y) {};
	virtual void Hover(int x, int y) {};
};


#endif // __COMPONENT_LISTENER_H__

