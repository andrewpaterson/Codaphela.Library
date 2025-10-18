/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "PlainGraphicalTextEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainGraphicalTextEditor::Init(CTextData* pcText, char* szText)
{
	mpcText = pcText;
	mcEditor.Init(szText);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainGraphicalTextEditor::Kill(void)
{
	mcEditor.Kill();
	mpcText = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainGraphicalTextEditor::Sync(void)
{
	mpcText->Clear();
	mpcText->Insert(0, 0, mpcText->mpcDefaultFont, mcEditor.Text(), mcEditor.Length());
}


void CPlainGraphicalTextEditor::Up(void)
{
	mcEditor.Up();
	Sync();
}


void CPlainGraphicalTextEditor::Down(void)
{
	mcEditor.Down();
	Sync();
}


void CPlainGraphicalTextEditor::Left(void)
{
	mcEditor.Left();
	Sync();
}


void CPlainGraphicalTextEditor::Right(void)
{
	mcEditor.Right();
	Sync();
}


void CPlainGraphicalTextEditor::WordLeft(void)
{
	mcEditor.WordLeft();
	Sync();
}


void CPlainGraphicalTextEditor::WordRight(void)
{
	mcEditor.WordRight();
	Sync();
}


void CPlainGraphicalTextEditor::Delete(void)
{
	mcEditor.Delete();
	Sync();
}


void CPlainGraphicalTextEditor::Backspace(void)
{
	mcEditor.Backspace();
	Sync();
}


void CPlainGraphicalTextEditor::Enter(void)
{
	mcEditor.Enter();
	Sync();
}


void CPlainGraphicalTextEditor::Printable(char c, bool bInsert)
{
	mcEditor.Printable(c, bInsert);
	Sync();
}


void CPlainGraphicalTextEditor::Home(void)
{
	mcEditor.Home();
	Sync();
}


void CPlainGraphicalTextEditor::End(void)
{
	mcEditor.End();
	Sync();
}


void CPlainGraphicalTextEditor::DocumentHome(void)
{
	mcEditor.DocumentHome();
	Sync();
}


void CPlainGraphicalTextEditor::DocumentEnd(void)
{
	mcEditor.DocumentEnd();
	Sync();
}


void CPlainGraphicalTextEditor::PageUp(void)
{
	mcEditor.PageUp();
	Sync();
}


void CPlainGraphicalTextEditor::PageDown(void)
{
	mcEditor.PageDown();
	Sync();
}


void CPlainGraphicalTextEditor::SelectUp(void)
{
	mcEditor.SelectUp();
	Sync();
}


void CPlainGraphicalTextEditor::SelectDown(void)
{
	mcEditor.SelectDown();
	Sync();
}


void CPlainGraphicalTextEditor::SelectLeft(void)
{
	mcEditor.SelectLeft();
	Sync();
}


void CPlainGraphicalTextEditor::SelectRight(void)
{
	mcEditor.SelectRight();
	Sync();
}


void CPlainGraphicalTextEditor::SelectWordLeft(void)
{
	mcEditor.SelectWordLeft();
	Sync();
}


void CPlainGraphicalTextEditor::SelectWordRight(void)
{
	mcEditor.SelectWordRight();
	Sync();
}


void CPlainGraphicalTextEditor::SelectHome(void)
{
	mcEditor.SelectHome();
	Sync();
}


void CPlainGraphicalTextEditor::SelectEnd(void)
{
	mcEditor.SelectEnd();
	Sync();
}


void CPlainGraphicalTextEditor::SelectDocumentHome(void)
{
	mcEditor.SelectDocumentHome();
	Sync();
}


void CPlainGraphicalTextEditor::SelectDocumentEnd(void)
{
	mcEditor.SelectDocumentEnd();
	Sync();
}


void CPlainGraphicalTextEditor::SelectPageUp(void)
{
	mcEditor.SelectPageUp();
	Sync();
}


void CPlainGraphicalTextEditor::SelectPageDown(void)
{
	mcEditor.SelectPageDown();
	Sync();
}


void CPlainGraphicalTextEditor::SelectWholeWord(void)
{
	mcEditor.SelectWholeWord();
	Sync();
}


void CPlainGraphicalTextEditor::SelectAll(void)
{
	mcEditor.SelectAll();
	Sync();
}


void CPlainGraphicalTextEditor::ClearSelection(void)
{
	mcEditor.ClearSelection();
	Sync();
}


void CPlainGraphicalTextEditor::Cut(CChars* pszDest)
{
	mcEditor.Cut(pszDest);
	Sync();
}


void CPlainGraphicalTextEditor::Copy(CChars* pszDest)
{
	mcEditor.Copy(pszDest);
	Sync();
}


void CPlainGraphicalTextEditor::Paste(char* szSource)
{
	mcEditor.Paste(szSource);
	Sync();
}


void CPlainGraphicalTextEditor::CutLine(CChars* pszDest)
{
	mcEditor.CutLine(pszDest);
	Sync();
}


void CPlainGraphicalTextEditor::Duplicate(void)
{
	mcEditor.Duplicate();
	Sync();
}


void CPlainGraphicalTextEditor::DeleteWordRemainingRight(void)
{
	mcEditor.DeleteWordRemainingRight();
	Sync();
}


void CPlainGraphicalTextEditor::DeleteWordRemainingLeft(void)
{
	mcEditor.DeleteWordRemainingLeft();
	Sync();
}


void CPlainGraphicalTextEditor::DeleteLine(void)
{
	mcEditor.DeleteLine();
	Sync();
}


void CPlainGraphicalTextEditor::EndEnter(void)
{
	mcEditor.EndEnter();
	Sync();
}


void CPlainGraphicalTextEditor::HomeEnter(void)
{
	mcEditor.HomeEnter();
	Sync();
}


void CPlainGraphicalTextEditor::Space(bool bInsert)
{
	mcEditor.Space(bInsert);
	Sync();
}


void CPlainGraphicalTextEditor::Tab(bool bInsert)
{
	mcEditor.Tab(bInsert);
	Sync();
}


void CPlainGraphicalTextEditor::BackTab(void)
{
	mcEditor.BackTab();
	Sync();
}

