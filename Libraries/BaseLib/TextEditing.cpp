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
#include "TextEditing.h"


//////////////////////////////////////////////////////////////////////////
//
// The editing listens to pcDocument so its caret and selection follow
// the document's edits.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Init(CTextDocument* pcDocument, CTextEditingView* pcView)
{
	mpcDocument = pcDocument;
	mpcNavigation = pcDocument->GetNavigation();
	mpcView = pcView;
	muiCaret = 0;
	muiAnchor = MAX_SIZE;
	mbCaretAtRowEnd = false;
	miUpDownX = -1;
	muiTabWidth = 4;
	mbEditable = true;
	mbSingleLine = false;
	mbEdited = false;
	mpcDocument->AddListener(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Kill(void)
{
	mpcDocument->RemoveListener(this);
	mpcDocument = NULL;
	mpcNavigation = NULL;
	mpcView = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The caret goes to the start with nothing selected, as for a text just
// set.  The view isn't told.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Reset(void)
{
	muiCaret = 0;
	muiAnchor = MAX_SIZE;
	mbCaretAtRowEnd = false;
	miUpDownX = -1;
}


//////////////////////////////////////////////////////////////////////////
//
// A text that isn't editable can still be navigated, selected and copied
// from.  Its document keeps no history.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::SetEditable(bool bEditable)
{
	mbEditable = bEditable;
	mpcDocument->SetUndoable(bEditable);
	ClearUndo();
}


//////////////////////////////////////////////////////////////////////////
//
// How many spaces apart the tab stops are, at least 1.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::SetTabWidth(size uiTabWidth)
{
	muiTabWidth = (uiTabWidth < 1) ? 1 : uiTabWidth;
}


//////////////////////////////////////////////////////////////////////////
//
// A single line text gets no line feeds (see MakeInsertable).
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::SetSingleLine(bool bSingleLine)
{
	mbSingleLine = bSingleLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::IsEditable(void) { return mbEditable; }
bool CTextEditing::IsSingleLine(void) { return mbSingleLine; }
size CTextEditing::GetTabWidth(void) { return muiTabWidth; }
size CTextEditing::GetCaretPosition(void) { return muiCaret; }
bool CTextEditing::IsCaretAtRowEnd(void) { return mbCaretAtRowEnd; }
size CTextEditing::GetCaretRow(void) { return mpcView->FindRow(muiCaret, mbCaretAtRowEnd); }  //Only when the view has rows.
int32 CTextEditing::GetCaretX(void) { return mpcView->GetPositionX(muiCaret, mbCaretAtRowEnd); }  //Only when the view has rows.
bool CTextEditing::IsSelected(void) { return (muiAnchor != MAX_SIZE) && (muiAnchor != muiCaret); }


//////////////////////////////////////////////////////////////////////////
//
// uiPosition should be the start of a character; past the end it is the
// end.  Nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::SetCaretPosition(size uiPosition)
{
	if (uiPosition > mpcDocument->Length())
	{
		uiPosition = mpcDocument->Length();
	}
	MoveCaret(uiPosition, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Moving without selecting clears the selection.  Selecting starts one at
// the caret if there isn't one.  Up and down keep the x they started at.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::MoveCaret(size uiPosition, bool bSelect, bool bRowEnd, bool bKeepUpDown)
{
	if (bSelect)
	{
		AnchorSelection();
	}
	else
	{
		muiAnchor = MAX_SIZE;
	}

	muiCaret = uiPosition;
	mbCaretAtRowEnd = bRowEnd;
	if (!bKeepUpDown)
	{
		miUpDownX = -1;
	}
	mpcView->MarksChanged();
	mpcView->ShowCaret();
}


//////////////////////////////////////////////////////////////////////////
//
// A selection starts at the caret, unless there is one, as for a shift
// press of the pointer.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::AnchorSelection(void)
{
	if (muiAnchor == MAX_SIZE)
	{
		muiAnchor = muiCaret;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if nothing is selected, then the start and end are the
// caret.  The start is before the end.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::GetSelection(size* puiStart, size* puiEnd)
{
	if (!IsSelected())
	{
		*puiStart = muiCaret;
		*puiEnd = muiCaret;
		return false;
	}
	if (muiAnchor < muiCaret)
	{
		*puiStart = muiAnchor;
		*puiEnd = muiCaret;
	}
	else
	{
		*puiStart = muiCaret;
		*puiEnd = muiAnchor;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Selects from uiStart to uiEnd with the caret at uiEnd.  The same start
// and end selects nothing.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Select(size uiStart, size uiEnd)
{
	if (uiStart > mpcDocument->Length())
	{
		uiStart = mpcDocument->Length();
	}
	if (uiEnd > mpcDocument->Length())
	{
		uiEnd = mpcDocument->Length();
	}
	muiAnchor = uiStart;
	MoveCaret(uiEnd, true);
}


//////////////////////////////////////////////////////////////////////////
//
// The position nearest x, y in the view, as a pointer puts the caret:
// on the row y is in, or the nearest row, the gap between characters
// nearest x.  False, and the start of the text, if the view has no rows.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::FindPositionAt(int32 x, int32 y, size* puiPosition, bool* pbRowEnd)
{
	*pbRowEnd = false;
	if (!mpcView->HasRows())
	{
		*puiPosition = 0;
		return false;
	}
	*puiPosition = mpcView->FindPositionInRow(mpcView->FindRowAt(y), x, pbRowEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Moving the caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Up(void) { MoveUpDown(TEM_RowUp, false); }
void CTextEditing::Down(void) { MoveUpDown(TEM_RowDown, false); }
void CTextEditing::Left(void) { MoveCaret(mpcNavigation->PreviousPosition(muiCaret), false); }
void CTextEditing::Right(void) { MoveCaret(mpcNavigation->NextPosition(muiCaret), false); }
void CTextEditing::WordLeft(void) { MoveCaret(mpcNavigation->FindWordLeft(muiCaret), false); }
void CTextEditing::WordRight(void) { MoveCaret(mpcNavigation->FindWordRight(muiCaret), false); }
void CTextEditing::Home(void) { MoveHome(false); }
void CTextEditing::End(void) { MoveEnd(false); }
void CTextEditing::DocumentHome(void) { MoveCaret(0, false); }
void CTextEditing::DocumentEnd(void) { MoveCaret(mpcDocument->Length(), false); }
void CTextEditing::PageUp(void) { MoveUpDown(TEM_PageUp, false); }
void CTextEditing::PageDown(void) { MoveUpDown(TEM_PageDown, false); }


//////////////////////////////////////////////////////////////////////////
//
// Selecting.  The selection is from where it started to the caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::SelectUp(void) { MoveUpDown(TEM_RowUp, true); }
void CTextEditing::SelectDown(void) { MoveUpDown(TEM_RowDown, true); }
void CTextEditing::SelectLeft(void) { MoveCaret(mpcNavigation->PreviousPosition(muiCaret), true); }
void CTextEditing::SelectRight(void) { MoveCaret(mpcNavigation->NextPosition(muiCaret), true); }
void CTextEditing::SelectWordLeft(void) { MoveCaret(mpcNavigation->FindWordLeft(muiCaret), true); }
void CTextEditing::SelectWordRight(void) { MoveCaret(mpcNavigation->FindWordRight(muiCaret), true); }
void CTextEditing::SelectHome(void) { MoveHome(true); }
void CTextEditing::SelectEnd(void) { MoveEnd(true); }
void CTextEditing::SelectDocumentHome(void) { MoveCaret(0, true); }
void CTextEditing::SelectDocumentEnd(void) { MoveCaret(mpcDocument->Length(), true); }
void CTextEditing::SelectPageUp(void) { MoveUpDown(TEM_PageUp, true); }
void CTextEditing::SelectPageDown(void) { MoveUpDown(TEM_PageDown, true); }
void CTextEditing::SelectAll(void) { Select(0, mpcDocument->Length()); }


//////////////////////////////////////////////////////////////////////////
//
// Selects the word the caret is in or next to.  Not next to a word,
// nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::SelectWholeWord(void)
{
	size	uiStart;
	size	uiEnd;
	char*	szText;

	szText = mpcDocument->Text();
	uiStart = muiCaret;
	if ((muiCaret > 0) && CTextNavigation::IsWordChar(szText[mpcNavigation->PreviousPosition(muiCaret)]))
	{
		uiStart = mpcNavigation->FindWordLeft(muiCaret);
	}
	uiEnd = muiCaret;
	if ((muiCaret < mpcDocument->Length()) && CTextNavigation::IsWordChar(szText[muiCaret]))
	{
		uiEnd = mpcNavigation->FindWordRight(muiCaret);
	}
	Select(uiStart, uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::ClearSelection(void)
{
	muiAnchor = MAX_SIZE;
	mpcView->MarksChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// To the row above or below, or a page of rows, as near the x up and
// down started at as the row goes.  Up from the first row goes to the
// start of the text and down from the last to its end; a page up or down
// from them doesn't move.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::MoveUpDown(ETextEditingMove eMove, bool bSelect)
{
	size	uiRow;
	size	uiPosition;
	bool	bRowEnd;
	bool	bDown;
	bool	bPage;
	bool	bAtEnd;

	if (!mpcView->HasRows())
	{
		return;
	}

	bDown = (eMove == TEM_RowDown) || (eMove == TEM_PageDown);
	bPage = (eMove == TEM_PageUp) || (eMove == TEM_PageDown);
	if (miUpDownX < 0)
	{
		miUpDownX = GetCaretX();
	}

	uiRow = GetCaretRow();
	bAtEnd = bDown ? (uiRow + 1 == mpcView->NumRows()) : (uiRow == 0);
	if (bAtEnd)
	{
		if (!bPage)
		{
			MoveCaret(bDown ? mpcDocument->Length() : 0, bSelect, false, true);
		}
		return;
	}

	if (bPage)
	{
		uiRow = mpcView->ScrollPage(uiRow, bDown);
	}
	else if (bDown)
	{
		uiRow++;
	}
	else
	{
		uiRow--;
	}

	uiPosition = mpcView->FindPositionInRow(uiRow, miUpDownX, &bRowEnd);
	MoveCaret(uiPosition, bSelect, bRowEnd, true);
}


//////////////////////////////////////////////////////////////////////////
//
// To the start of a wrapped row, then to the first character on the line
// that isn't a space, then to the start of the line, and back.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::MoveHome(bool bSelect)
{
	size	uiLineStart;
	size	uiIndent;
	size	uiRowStart;

	uiLineStart = mpcNavigation->FindLineStart(muiCaret);
	if (mpcView->HasRows())
	{
		uiRowStart = mpcView->GetRowStart(GetCaretRow());
		if ((uiRowStart > uiLineStart) && ((uiRowStart != muiCaret) || mbCaretAtRowEnd))
		{
			MoveCaret(uiRowStart, bSelect);
			return;
		}
	}

	uiIndent = mpcNavigation->FindLineIndent(uiLineStart);
	MoveCaret(muiCaret == uiIndent ? uiLineStart : uiIndent, bSelect);
}


//////////////////////////////////////////////////////////////////////////
//
// To the end of a wrapped row, then to the end of the line.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::MoveEnd(bool bSelect)
{
	size	uiRowEnd;
	bool	bRowEnd;

	if (mpcView->HasRows())
	{
		uiRowEnd = mpcView->GetRowEnd(GetCaretRow(), &bRowEnd);
		if (bRowEnd && !((muiCaret == uiRowEnd) && mbCaretAtRowEnd))
		{
			MoveCaret(uiRowEnd, bSelect, true);
			return;
		}
	}
	MoveCaret(mpcNavigation->FindLineEnd(muiCaret), bSelect);
}


//////////////////////////////////////////////////////////////////////////
//
// c replaces the selection.  Without one it is inserted, or with bInsert
// false it overwrites the character after the caret unless that ends the
// line.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Printable(char c, bool bInsert)
{
	char	sz[2];
	char*	szText;

	if (!BeginEditIfEditable())
	{
		return;
	}

	sz[0] = c;
	sz[1] = '\0';
	mpcDocument->GetUndo()->Typed(c, IsSelected() ? MAX_SIZE : muiCaret);
	szText = mpcDocument->Text();
	if (!IsSelected() && !bInsert && (muiCaret < mpcDocument->Length()) && (szText[muiCaret] != '\n') && (szText[muiCaret] != '\r'))
	{
		RemoveText(muiCaret, mpcNavigation->NextPosition(muiCaret));
	}
	ReplaceSelection(sz);
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// Spaces to the next tab stop replace the selection, or are inserted.  A
// selection over more than one line has its lines indented instead.
// bInsert is ignored, tabs never overwrite.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Tab(bool bInsert)
{
	size	uiStart;
	size	uiEnd;
	size	uiColumns;
	CChars	szSpaces;

	if (!BeginEditIfEditable())
	{
		return;
	}

	GetSelection(&uiStart, &uiEnd);
	if (mpcNavigation->FindLineStart(uiStart) != mpcNavigation->FindLineStart(uiEnd))
	{
		IndentLines(uiStart, uiEnd);
	}
	else
	{
		uiColumns = mpcNavigation->CountColumns(mpcNavigation->FindLineStart(uiStart), uiStart);
		szSpaces.Init();
		szSpaces.Append(' ', muiTabWidth - (uiColumns % muiTabWidth));
		ReplaceSelection(szSpaces.Text());
		szSpaces.Kill();
	}
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// Removes a tab's worth of spaces, or a tab, from the start of the lines
// the selection is over, or the caret's line.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::BackTab(void)
{
	size	uiStart;
	size	uiEnd;

	if (!BeginEditIfEditable())
	{
		return;
	}

	GetSelection(&uiStart, &uiEnd);
	OutdentLines(uiStart, uiEnd);
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// A line feed replaces the selection, or is inserted.  Not in a single
// line text, nor are the other enters.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Enter(void)
{
	if (mbSingleLine || !BeginEditIfEditable())
	{
		return;
	}

	ReplaceSelection("\n");
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// Starts a new line before the caret's line, and the caret goes to it.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::HomeEnter(void)
{
	size	uiLineStart;

	if (mbSingleLine || !BeginEditIfEditable())
	{
		return;
	}

	muiAnchor = MAX_SIZE;
	uiLineStart = mpcNavigation->FindLineStart(muiCaret);
	InsertText(uiLineStart, "\n");
	muiCaret = uiLineStart;
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// Starts a new line after the caret's line, and the caret goes to it.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::EndEnter(void)
{
	if (mbSingleLine || !BeginEditIfEditable())
	{
		return;
	}

	muiAnchor = MAX_SIZE;
	muiCaret = mpcNavigation->FindLineEnd(muiCaret);
	InsertText(muiCaret, "\n");
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes the selection, or the character after the caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Delete(void)
{
	if (!BeginEditIfEditable())
	{
		return;
	}

	if (!DeleteSelection())
	{
		RemoveText(muiCaret, mpcNavigation->NextPosition(muiCaret));
	}
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes the selection, or the character before the caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Backspace(void)
{
	if (!BeginEditIfEditable())
	{
		return;
	}

	if (!DeleteSelection())
	{
		RemoveText(mpcNavigation->PreviousPosition(muiCaret), muiCaret);
	}
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes from the caret to the end of the word, or the spaces, it is
// in.  A selection is deleted instead.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::DeleteWordRemainingRight(void)
{
	if (!BeginEditIfEditable())
	{
		return;
	}

	if (!DeleteSelection())
	{
		RemoveText(muiCaret, mpcNavigation->FindWordRight(muiCaret));
	}
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes from the start of the word, or the spaces, the caret is in to
// the caret.  A selection is deleted instead.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::DeleteWordRemainingLeft(void)
{
	if (!BeginEditIfEditable())
	{
		return;
	}

	if (!DeleteSelection())
	{
		RemoveText(mpcNavigation->FindWordLeft(muiCaret), muiCaret);
	}
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes the lines the selection is over, or the caret's line, with
// their line feed.  The last line takes the line feed before it.  The
// caret stays as far along the line it is left on.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::DeleteLine(void)
{
	size	uiStart;
	size	uiEnd;
	size	uiColumn;
	bool	bSelected;

	if (!BeginEditIfEditable())
	{
		return;
	}

	uiColumn = mpcNavigation->CountColumns(mpcNavigation->FindLineStart(muiCaret), muiCaret);
	bSelected = GetSelection(&uiStart, &uiEnd);
	uiStart = mpcNavigation->FindLineStart(uiStart);
	if (bSelected && (uiEnd > uiStart) && (mpcNavigation->FindLineStart(uiEnd) == uiEnd))
	{
		//A selection that ends at the start of a line doesn't take that line.
		uiEnd = mpcNavigation->PreviousPosition(uiEnd);
	}
	uiEnd = mpcNavigation->FindLineEnd(uiEnd);
	if (uiEnd < mpcDocument->Length())
	{
		uiEnd = mpcNavigation->NextPosition(uiEnd);
	}
	else if (uiStart > 0)
	{
		uiStart = mpcNavigation->PreviousPosition(uiStart);
	}

	muiAnchor = MAX_SIZE;
	RemoveText(uiStart, uiEnd);
	muiCaret = mpcNavigation->FindColumn(mpcNavigation->FindLineStart(uiStart), uiColumn);
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is copied in front of itself and the copy after it stays
// selected.  Without one the caret's line is copied above itself and the
// caret goes with the line below; a single line text's isn't.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Duplicate(void)
{
	size	uiStart;
	size	uiEnd;
	CChars	szCopy;

	if (!BeginEditIfEditable())
	{
		return;
	}

	szCopy.Init();
	if (GetSelection(&uiStart, &uiEnd))
	{
		szCopy.AppendSubString(mpcDocument->Text(), uiStart, uiEnd);
	}
	else if (mbSingleLine)
	{
		szCopy.Kill();
		EditDone();
		return;
	}
	else
	{
		uiStart = mpcNavigation->FindLineStart(muiCaret);
		uiEnd = mpcNavigation->FindLineEnd(muiCaret);
		szCopy.AppendSubString(mpcDocument->Text(), uiStart, uiEnd);
		szCopy.Append('\n');
	}
	InsertText(uiStart, szCopy.Text());
	szCopy.Kill();
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is appended to pszDest and deleted.  Without one the
// caret's line is cut, but nothing of a single line text: all of it would
// go.  A text that isn't editable only copies what would be cut.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Cut(CChars* pszDest)
{
	BeginEdit();
	if (!IsSelected())
	{
		if (!mbSingleLine)
		{
			CutLine(pszDest);
		}
		return;
	}

	Copy(pszDest);
	if (mbEditable)
	{
		DeleteSelection();
		EditDone();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The caret's line, with its line feed, is appended to pszDest and
// deleted.  The caret stays as far along the line that moves up into its
// place.  With a selection only the selection is cut.  A text that isn't
// editable only copies what would be cut.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::CutLine(CChars* pszDest)
{
	size	uiLineStart;
	size	uiEnd;
	size	uiColumn;

	BeginEdit();
	if (IsSelected())
	{
		Cut(pszDest);
		return;
	}

	uiLineStart = mpcNavigation->FindLineStart(muiCaret);
	uiColumn = mpcNavigation->CountColumns(uiLineStart, muiCaret);
	uiEnd = mpcNavigation->NextPosition(mpcNavigation->FindLineEnd(muiCaret));
	pszDest->AppendSubString(mpcDocument->Text(), uiLineStart, uiEnd);
	if (!mbEditable)
	{
		return;
	}
	RemoveText(uiLineStart, uiEnd);
	muiCaret = mpcNavigation->FindColumn(uiLineStart, uiColumn);
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is appended to pszDest.  Without one nothing is copied.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Copy(CChars* pszDest)
{
	size	uiStart;
	size	uiEnd;

	if (GetSelection(&uiStart, &uiEnd))
	{
		pszDest->AppendSubString(mpcDocument->Text(), uiStart, uiEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// szSource replaces the selection, or is inserted, its line breaks made
// insertable (MakeInsertable).  Pasting nothing leaves a selection where
// it is.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Paste(char* szSource)
{
	CChars	szText;

	if (!BeginEditIfEditable() || (szSource == NULL) || (szSource[0] == '\0'))
	{
		return;
	}

	szText.Init(szSource);
	MakeInsertable(&szText);
	ReplaceSelection(szText.Text());
	szText.Kill();
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// Each of the ranges, uiStart to uiEnd (exclusive), in order and not
// overlapping, becomes szText, as one edit: one undo puts them all back.
// A rename's places in a file, say.  False if the text can't be edited or
// a range is out of order or past the end.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::ReplaceRanges(size* auiStarts, size* auiEnds, size uiNumRanges, const char* szText)
{
	size	i;

	if (!mbEditable || (uiNumRanges == 0))
	{
		return false;
	}
	for (i = 0; i < uiNumRanges; i++)
	{
		if ((auiStarts[i] > auiEnds[i]) || (auiEnds[i] > mpcDocument->Length()) || ((i > 0) && (auiStarts[i] < auiEnds[i - 1])))
		{
			return false;
		}
	}

	//From the last so each range's place is still where it was.
	BeginEdit();
	for (i = uiNumRanges; i > 0; i--)
	{
		RemoveText(auiStarts[i - 1], auiEnds[i - 1]);
		InsertText(auiStarts[i - 1], szText);
	}
	EditDone();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// All of the text becomes szText, its line breaks made insertable
// (MakeInsertable), as one edit.  The caret goes to the end.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::ReplaceText(const char* szText)
{
	CChars	szNew;

	if (!BeginEditIfEditable())
	{
		return;
	}

	szNew.Init(szText == NULL ? "" : szText);
	MakeInsertable(&szNew);
	if (!szNew.Equals(mpcDocument->Text()))
	{
		muiAnchor = MAX_SIZE;
		RemoveText(0, mpcDocument->Length());
		InsertText(0, szNew.Text());
	}
	szNew.Kill();
	MoveCaret(mpcDocument->Length(), false);
	EditDone();
}


//////////////////////////////////////////////////////////////////////////
//
// The caret, and the selection's start, at or after text put in move
// with it.  Inside text taken out they go to where it was.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::DocumentChanged(STextDocumentChange* psChange)
{
	size	uiLength;

	mbEdited = true;
	uiLength = psChange->Length();
	if (psChange->eChange == TDC_Inserted)
	{
		if (muiCaret >= psChange->uiStart)
		{
			muiCaret += uiLength;
		}
		if ((muiAnchor != MAX_SIZE) && (muiAnchor >= psChange->uiStart))
		{
			muiAnchor += uiLength;
		}
		return;
	}

	if (muiCaret >= psChange->uiEnd)
	{
		muiCaret -= uiLength;
	}
	else if (muiCaret > psChange->uiStart)
	{
		muiCaret = psChange->uiStart;
	}
	if (muiAnchor != MAX_SIZE)
	{
		if (muiAnchor >= psChange->uiEnd)
		{
			muiAnchor -= uiLength;
		}
		else if (muiAnchor > psChange->uiStart)
		{
			muiAnchor = psChange->uiStart;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// All the edits' changes are made by InsertText and RemoveText.  The
// first change of an edit notes the caret and selection before it,
// unless the edit noted them as it began (BeginEdit).
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::InsertText(size uiPosition, const char* szText)
{
	if (szText[0] == '\0')
	{
		return;
	}
	mpcDocument->StartEdit(muiCaret, muiAnchor);
	mpcDocument->Insert(uiPosition, szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::RemoveText(size uiStart, size uiEnd)
{
	if (uiEnd <= uiStart)
	{
		return;
	}
	mpcDocument->StartEdit(muiCaret, muiAnchor);
	mpcDocument->Remove(uiStart, uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// szText replaces the selection, or is inserted at the caret.  The caret
// is after it and nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::ReplaceSelection(const char* szText)
{
	DeleteSelection();
	muiAnchor = MAX_SIZE;
	InsertText(muiCaret, szText);
}


//////////////////////////////////////////////////////////////////////////
//
// Text put in has line feeds for line breaks: carriage returns become
// line feeds, or go where they come before one.  A single line text has
// spaces for them instead.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::MakeInsertable(CChars* pszText)
{
	if (mbSingleLine)
	{
		pszText->Replace("\r\n", " ");
		pszText->Replace('\r', ' ');
		pszText->Replace('\n', ' ');
	}
	else
	{
		pszText->Replace("\r\n", "\n");
		pszText->Replace('\r', '\n');
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if nothing was selected.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::DeleteSelection(void)
{
	size	uiStart;
	size	uiEnd;

	if (!GetSelection(&uiStart, &uiEnd))
	{
		return false;
	}
	muiAnchor = MAX_SIZE;
	RemoveText(uiStart, uiEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The starts of the lines from uiStart's to uiEnd's, but not uiEnd's if
// it is at its start and after uiStart.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::FindLines(size uiStart, size uiEnd, CArrayTextLineStart* pauiLineStarts)
{
	size	uiLine;

	uiLine = mpcNavigation->FindLineStart(uiStart);
	for (;;)
	{
		*pauiLineStarts->Add() = uiLine;
		uiLine = mpcNavigation->FindLineEnd(uiLine);
		if (uiLine >= mpcDocument->Length())
		{
			break;
		}
		uiLine = mpcNavigation->NextPosition(uiLine);
		if (uiLine >= uiEnd)
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Inserts a tab's worth of spaces at the start of each line from
// uiStart's to uiEnd's (FindLines).  A selection that started at the
// start of a line still does.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::IndentLines(size uiStart, size uiEnd)
{
	CArrayTextLineStart	auiLineStarts;
	size				i;
	CChars				szSpaces;
	bool				bAnchorFirst;
	bool				bAtLineStart;

	auiLineStarts.Init();
	FindLines(uiStart, uiEnd, &auiLineStarts);
	bAtLineStart = (*auiLineStarts.Get(0) == uiStart);

	szSpaces.Init();
	szSpaces.Append(' ', muiTabWidth);
	bAnchorFirst = (muiAnchor != MAX_SIZE) && (muiAnchor < muiCaret);
	i = auiLineStarts.NumElements();
	while (i > 0)
	{
		i--;
		InsertText(*auiLineStarts.Get(i), szSpaces.Text());
	}
	szSpaces.Kill();
	auiLineStarts.Kill();

	if (bAtLineStart)
	{
		if (bAnchorFirst)
		{
			muiAnchor = uiStart;
		}
		else if (muiAnchor != MAX_SIZE)
		{
			muiCaret = uiStart;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Removes a tab, or up to a tab's worth of spaces, from the start of each
// line from uiStart's to uiEnd's (FindLines).
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::OutdentLines(size uiStart, size uiEnd)
{
	CArrayTextLineStart	auiLineStarts;
	size				uiLine;
	size				i;
	size				uiRemove;
	char*				szText;

	auiLineStarts.Init();
	FindLines(uiStart, uiEnd, &auiLineStarts);
	i = auiLineStarts.NumElements();
	while (i > 0)
	{
		i--;
		uiLine = *auiLineStarts.Get(i);
		szText = mpcDocument->Text();
		uiRemove = 0;
		if (szText[uiLine] == '\t')
		{
			uiRemove = 1;
		}
		else
		{
			while ((uiRemove < muiTabWidth) && (szText[uiLine + uiRemove] == ' '))
			{
				uiRemove++;
			}
		}
		RemoveText(uiLine, uiLine + uiRemove);
	}
	auiLineStarts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An edit action begins (typing, deleting, pasting ...): the caret and
// selection are noted as they are, before it moves them or changes the
// text, for undo to put back.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::BeginEdit(void)
{
	mbEdited = false;
	mpcDocument->BeginEdit(muiCaret, muiAnchor);
}


//////////////////////////////////////////////////////////////////////////
//
// An edit begins (BeginEdit), and true if the text is editable; false if
// it isn't, and the edit does nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::BeginEditIfEditable(void)
{
	BeginEdit();
	return mbEditable;
}


//////////////////////////////////////////////////////////////////////////
//
// An edit, or an undo or redo, has changed the text: it is kept and the
// view told the text was edited.  Up and down start again from the caret.
// An edit that changed nothing is dropped; only the marks may have moved.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::EditDone(void)
{
	if (!mbEdited)
	{
		mpcDocument->GetUndo()->DropEdit();
		mpcView->MarksChanged();
		return;
	}

	mbEdited = false;
	KeepEdit();
	mbCaretAtRowEnd = false;
	miUpDownX = -1;
	mpcView->TextEdited();
}


//////////////////////////////////////////////////////////////////////////
//
// The edit is kept in the document's history: its text with the caret
// and selection from before it and after it.  Typing goes on the state
// of the typing before it, as one edit (see CTextUndo).  An undo or redo
// isn't kept.  The view is told what was kept.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::KeepEdit(void)
{
	ETextUndoKeep	eKeep;
	size			uiIdBefore;

	uiIdBefore = mpcDocument->GetUndo()->GetCurrentId();
	eKeep = mpcDocument->EndEdit(muiCaret, muiAnchor);
	if (eKeep == TUK_Merged)
	{
		UndoChanged(TUC_Merged, uiIdBefore, mpcDocument->GetUndo()->GetCurrentId());
	}
	else if (eKeep == TUK_Pushed)
	{
		UndoChanged(TUC_Kept, uiIdBefore, mpcDocument->GetUndo()->GetCurrentId());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::Undo(void) { return LoadUndo(mpcDocument->GetUndo()->FindUndo()); }
bool CTextEditing::Redo(void) { return LoadUndo(mpcDocument->GetUndo()->FindRedo()); }


//////////////////////////////////////////////////////////////////////////
//
// Undoes or redoes to the state with id uiId (STextUndoChange's
// uiIdAfter), as an undo history across texts does.  False if it isn't
// in the history or is the current state.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::UndoTo(size uiId) { return LoadUndo(mpcDocument->GetUndo()->FindId(uiId)); }


//////////////////////////////////////////////////////////////////////////
//
// The history starts again with the text, caret and selection as they
// are.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::ClearUndo(void)
{
	mpcDocument->ClearUndo(muiCaret, muiAnchor);
}


//////////////////////////////////////////////////////////////////////////
//
// Changes the text to state uiIndex of the history, the caret and
// selection as they were: undone, as before the first edit undone;
// redone, as after the last edit redone.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::LoadUndo(size uiIndex)
{
	size	uiIdBefore;
	size	uiCaret;
	size	uiAnchor;
	bool	bUndo;

	if (!mpcDocument->IsUndoable() || !mpcDocument->GetUndo()->CanLoad(uiIndex))
	{
		return false;
	}
	bUndo = mpcDocument->GetUndo()->IsUndo(uiIndex);
	uiIdBefore = mpcDocument->GetUndo()->GetCurrentId();
	mpcDocument->Load(uiIndex, &uiCaret, &uiAnchor);

	muiCaret = uiCaret;
	muiAnchor = uiAnchor;
	EditDone();
	mpcDocument->GetUndo()->Loaded(muiCaret);
	UndoChanged(bUndo ? TUC_Undone : TUC_Redone, uiIdBefore, mpcDocument->GetUndo()->GetCurrentId());
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::UndoChanged(ETextUndoChange eChange, size uiIdBefore, size uiIdAfter)
{
	STextUndoChange	sChange;

	sChange.eChange = eChange;
	sChange.uiIdBefore = uiIdBefore;
	sChange.uiIdAfter = uiIdAfter;
	mpcView->UndoChanged(&sChange);
}

