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
#ifndef __ABSTRACT_TEXT_EDITOR_H__
#define __ABSTRACT_TEXT_EDITOR_H__
#include "BaseLib/Chars.h"


class CTextEditor
{
public:
	//Navigation
	virtual void Up(void) =0;
	virtual void Down(void) =0;
	virtual void Left(void) =0;
	virtual void Right(void) =0;
	virtual void WordLeft(void) =0;
	virtual void WordRight(void) =0;
	virtual void Home(void) =0;
	virtual void End(void) =0;
	virtual void DocumentHome(void) =0;
	virtual void DocumentEnd(void) =0;
	virtual void PageUp(void) =0;
	virtual void PageDown(void) =0;

	//Selection
	virtual void SelectUp(void) =0;
	virtual void SelectDown(void) =0;
	virtual void SelectLeft(void) =0;
	virtual void SelectRight(void) =0;
	virtual void SelectWordLeft(void) =0;
	virtual void SelectWordRight(void) =0;
	virtual void SelectHome(void) =0;
	virtual void SelectEnd(void) =0;
	virtual void SelectDocumentHome(void) =0;
	virtual void SelectDocumentEnd(void) =0;
	virtual void SelectPageUp(void) =0;
	virtual void SelectPageDown(void) =0;
	virtual void SelectWholeWord(void) =0;
	virtual void SelectAll(void) =0;
	virtual void ClearSelection(void) =0;

	//Modification
	virtual void Printable(char c, BOOL bInsert) =0;
	virtual void Space(BOOL bInsert) =0;
	virtual void Tab(BOOL bInsert) =0;
	virtual void BackTab(void) =0;
	virtual void Enter(void) =0;
	virtual void HomeEnter(void) =0;
	virtual void EndEnter(void) =0;

	//Modification
	virtual void Delete(void) =0;
	virtual void Backspace(void) =0;
	virtual void Cut(CChars* pszDest) =0;
	virtual void Copy(CChars* pszDest) =0;
	virtual void Paste(char* szSource) =0;
	virtual void CutLine(CChars* pszDest) =0;
	virtual void Duplicate(void) =0;
	virtual void DeleteWordRemainingRight(void) =0;
	virtual void DeleteWordRemainingLeft(void) =0;
	virtual void DeleteLine(void) =0;
};


#endif // __ABSTRACT_TEXT_EDITOR_H__

