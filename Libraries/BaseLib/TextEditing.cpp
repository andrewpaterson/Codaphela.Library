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
	mpcSelectionRanges = NULL;
	masShrinks.Init();
	msExtended.uiStart = MAX_SIZE;
	msExtended.uiEnd = MAX_SIZE;
	mbBox = false;
	muiBoxAnchorRow = 0;
	miBoxAnchorX = 0;
	muiBoxCaretRow = 0;
	miBoxCaretX = 0;
	mbBoxEditing = false;
	mpcDocument->AddListener(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::Kill(void)
{
	mpcDocument->RemoveListener(this);
	masShrinks.Kill();
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
	mbBox = false;
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
size CTextEditing::GetAnchorPosition(void) { return IsSelected() ? muiAnchor : muiCaret; }  //Where the selection started, the caret if nothing is selected.
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
	mbBox = false;
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
// The document's, for finding words and lines in its text.
//
//////////////////////////////////////////////////////////////////////////
CTextNavigation* CTextEditing::GetNavigation(void) { return mpcNavigation; }
CTextEditingView* CTextEditing::GetView(void) { return mpcView; }  //What shows the text, for an input to ask it to show its caret.


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
// The character under x, y in the view, as a double click finds it: on
// the row y is in, or the nearest row, the character x is over, not the
// gap nearest it.  Left of the row it is the row's first.  Returns false
// past the row's end, or with no rows, with the position the gap nearest
// x, y.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::FindCharacterAt(int32 x, int32 y, size* puiPosition)
{
	size	uiRow;
	size	uiRowStart;
	size	uiRowEnd;
	size	uiGap;
	bool	bRowEnd;
	bool	bEndOfRow;

	if (!FindPositionAt(x, y, &uiGap, &bRowEnd))
	{
		*puiPosition = uiGap;
		return false;
	}

	uiRow = mpcView->FindRowAt(y);
	uiRowStart = mpcView->GetRowStart(uiRow);
	uiRowEnd = mpcView->GetRowEnd(uiRow, &bEndOfRow);
	*puiPosition = uiGap;
	if ((uiGap > uiRowStart) && (mpcView->GetPositionX(uiGap, bRowEnd) > x))
	{
		*puiPosition = mpcNavigation->PreviousPosition(uiGap);
		return true;
	}
	return uiGap < uiRowEnd;
}


//////////////////////////////////////////////////////////////////////////
//
// The character under x, y in the view is selected: a press there is on
// the selection.  The gap after the selection's last character isn't.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::IsSelectedAt(int32 x, int32 y)
{
	size	uiStart;
	size	uiEnd;
	size	uiPosition;

	if (!GetSelection(&uiStart, &uiEnd) || !FindCharacterAt(x, y, &uiPosition))
	{
		return false;
	}
	return (uiPosition >= uiStart) && (uiPosition < uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// The caret goes to the position nearest x, y in the view, as a pointer
// puts it (FindPositionAt), selecting from where it was with bSelect.
// False if the view has no rows: it goes to the start of the text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::MoveCaretTo(int32 x, int32 y, bool bSelect)
{
	size	uiPosition;
	bool	bRowEnd;
	bool	bRows;

	bRows = FindPositionAt(x, y, &uiPosition, &bRowEnd);
	MoveCaret(uiPosition, bSelect, bRowEnd);
	return bRows;
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
// The selection (or the caret) grows to the smallest range around it that
// is bigger (FindExtension), as IntelliJ's Extend Selection: the first
// from a caret is the word it is in or next to.  What it grew from is kept
// for ShrinkSelection.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::ExtendSelection(void)
{
	size		uiStart;
	size		uiEnd;
	STextRange	sRange;
	STextRange*	psShrink;

	if (!IsExtended())
	{
		masShrinks.ReInit();
	}
	GetSelection(&uiStart, &uiEnd);
	if (!FindExtension(uiStart, uiEnd, &sRange))
	{
		return;
	}
	psShrink = masShrinks.Add();
	psShrink->uiStart = IsSelected() ? muiAnchor : MAX_SIZE;
	psShrink->uiEnd = muiCaret;
	Select(sRange.uiStart, sRange.uiEnd);
	SetExtended();
}


//////////////////////////////////////////////////////////////////////////
//
// The selection goes back to what ExtendSelection grew it from, as long
// as it hasn't been changed any other way since and the text hasn't been
// edited.  Otherwise nothing changes.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::ShrinkSelection(void)
{
	STextRange	sShrink;

	if (!IsExtended() || (masShrinks.NumElements() == 0))
	{
		masShrinks.ReInit();
		return;
	}
	masShrinks.Pop(&sShrink);
	if (sShrink.uiStart == MAX_SIZE)
	{
		MoveCaret(sShrink.uiEnd, false);
	}
	else
	{
		Select(sShrink.uiStart, sShrink.uiEnd);
	}
	SetExtended();
}


//////////////////////////////////////////////////////////////////////////
//
// pcRanges adds ranges Extend Selection can grow to, NULL for none.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::SetSelectionRanges(CTextSelectionRanges* pcRanges)
{
	mpcSelectionRanges = pcRanges;
}


//////////////////////////////////////////////////////////////////////////
//
// The smallest of the text's ranges, and the ones added, that is around
// uiStart to uiEnd and bigger.  False if there is none: all the text is
// selected.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::FindExtension(size uiStart, size uiEnd, STextRange* psDest)
{
	CArrayTextRange	asRanges;
	STextRange*		psRange;
	size			i;
	bool			bFound;

	asRanges.Init();
	AddTextRanges(uiStart, uiEnd, &asRanges);
	if (mpcSelectionRanges)
	{
		mpcSelectionRanges->AddSelectionRanges(uiStart, uiEnd, &asRanges);
	}

	bFound = false;
	for (i = 0; i < asRanges.NumElements(); i++)
	{
		psRange = asRanges.Get(i);
		if ((psRange->uiStart <= uiStart) && (psRange->uiEnd >= uiEnd) && (psRange->uiEnd <= mpcDocument->Length()) && (psRange->uiEnd - psRange->uiStart > uiEnd - uiStart))
		{
			if (!bFound || (psRange->uiEnd - psRange->uiStart < psDest->uiEnd - psDest->uiStart))
			{
				*psDest = *psRange;
				bFound = true;
			}
		}
	}
	asRanges.Kill();
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
// The text's own ranges around uiStart to uiEnd (FindExtension picks from
// them): its word, the inside of its quotes and the quotes with it, the
// inside of its brackets and the brackets with it, its line without the
// indent, its whole lines with their line feeds, its comment, and all the
// text.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::AddTextRanges(size uiStart, size uiEnd, CArrayTextRange* pasDest)
{
	STextRange	sRange;
	size		uiOpen;
	size		uiClose;
	size		uiLineStart;

	if (mpcNavigation->FindWordOver(uiStart, uiEnd, &sRange.uiStart, &sRange.uiEnd))
	{
		*pasDest->Add() = sRange;
	}
	if (mpcNavigation->FindQuotesAround(uiStart, uiEnd, &uiOpen, &uiClose))
	{
		sRange.uiStart = uiOpen + 1;
		sRange.uiEnd = uiClose;
		*pasDest->Add() = sRange;
		sRange.uiStart = uiOpen;
		sRange.uiEnd = uiClose + 1;
		*pasDest->Add() = sRange;
	}
	if (mpcNavigation->FindBracketsAround(uiStart, uiEnd, &uiOpen, &uiClose))
	{
		sRange.uiStart = uiOpen + 1;
		sRange.uiEnd = uiClose;
		*pasDest->Add() = sRange;
		sRange.uiStart = uiOpen;
		sRange.uiEnd = uiClose + 1;
		*pasDest->Add() = sRange;
	}

	uiLineStart = mpcNavigation->FindLineStart(uiStart);
	sRange.uiStart = mpcNavigation->FindLineIndent(uiLineStart);
	sRange.uiEnd = mpcNavigation->FindLineEnd(uiStart);
	*pasDest->Add() = sRange;
	sRange.uiStart = uiLineStart;
	sRange.uiEnd = mpcNavigation->NextPosition(mpcNavigation->FindLineEnd(((uiEnd > uiStart) && (mpcNavigation->FindLineStart(uiEnd) == uiEnd)) ? uiEnd - 1 : uiEnd));
	*pasDest->Add() = sRange;

	if (mpcNavigation->FindCommentAround(uiStart, uiEnd, &sRange.uiStart, &sRange.uiEnd))
	{
		*pasDest->Add() = sRange;
	}
	sRange.uiStart = 0;
	sRange.uiEnd = mpcDocument->Length();
	*pasDest->Add() = sRange;
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is as Extend or Shrink last left it, so the selections it
// grew from can still be gone back to.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::IsExtended(void)
{
	return (msExtended.uiStart == (IsSelected() ? muiAnchor : MAX_SIZE)) && (msExtended.uiEnd == muiCaret);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::SetExtended(void)
{
	msExtended.uiStart = IsSelected() ? muiAnchor : MAX_SIZE;
	msExtended.uiEnd = muiCaret;
}


//////////////////////////////////////////////////////////////////////////
//
// A box from row uiAnchorRow at x iAnchorX to row uiCaretRow at x iCaretX
// (the view's), the caret at the second.  Rows past the last are the
// last.  Nothing without rows.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::BoxSelect(size uiAnchorRow, int32 iAnchorX, size uiCaretRow, int32 iCaretX)
{
	size	uiLastRow;

	if (!mpcView->HasRows())
	{
		return;
	}
	uiLastRow = mpcView->NumRows() - 1;
	mbBox = true;
	muiBoxAnchorRow = (uiAnchorRow < uiLastRow) ? uiAnchorRow : uiLastRow;
	miBoxAnchorX = (iAnchorX > 0) ? iAnchorX : 0;
	muiBoxCaretRow = (uiCaretRow < uiLastRow) ? uiCaretRow : uiLastRow;
	miBoxCaretX = (iCaretX > 0) ? iCaretX : 0;
	PlaceBoxCaret();
}


//////////////////////////////////////////////////////////////////////////
//
// The box's caret corner moves a row up or down, a character left or
// right along its row, or to its row's start or end, as Visual Studio's
// Shift+Alt keys move it.  Without a box one starts at the caret, or at a
// selection's anchor.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::BoxSelectUp(void)
{
	if (StartBox())
	{
		if (muiBoxCaretRow > 0)
		{
			muiBoxCaretRow--;
		}
		PlaceBoxCaret();
	}
}


void CTextEditing::BoxSelectDown(void)
{
	if (StartBox())
	{
		if (muiBoxCaretRow + 1 < mpcView->NumRows())
		{
			muiBoxCaretRow++;
		}
		PlaceBoxCaret();
	}
}


void CTextEditing::BoxSelectLeft(void)
{
	size	uiPosition;
	bool	bRowEnd;

	if (StartBox())
	{
		uiPosition = mpcView->FindPositionInRow(muiBoxCaretRow, miBoxCaretX, &bRowEnd);
		if (uiPosition > mpcView->GetRowStart(muiBoxCaretRow))
		{
			miBoxCaretX = mpcView->GetPositionX(mpcNavigation->PreviousPosition(uiPosition), false);
		}
		PlaceBoxCaret();
	}
}


void CTextEditing::BoxSelectRight(void)
{
	size	uiPosition;
	bool	bRowEnd;

	if (StartBox())
	{
		uiPosition = mpcView->FindPositionInRow(muiBoxCaretRow, miBoxCaretX, &bRowEnd);
		if (uiPosition < mpcView->GetRowEnd(muiBoxCaretRow, &bRowEnd))
		{
			miBoxCaretX = mpcView->GetPositionX(mpcNavigation->NextPosition(uiPosition), false);
		}
		PlaceBoxCaret();
	}
}


void CTextEditing::BoxSelectHome(void)
{
	if (StartBox())
	{
		miBoxCaretX = mpcView->GetPositionX(mpcView->GetRowStart(muiBoxCaretRow), false);
		PlaceBoxCaret();
	}
}


void CTextEditing::BoxSelectEnd(void)
{
	size	uiEnd;
	bool	bRowEnd;

	if (StartBox())
	{
		uiEnd = mpcView->GetRowEnd(muiBoxCaretRow, &bRowEnd);
		miBoxCaretX = mpcView->GetPositionX(uiEnd, bRowEnd);
		PlaceBoxCaret();
	}
}


bool CTextEditing::IsBox(void) { return mbBox; }


//////////////////////////////////////////////////////////////////////////
//
// The box's rows, top and bottom, and its left and right x.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::GetBox(size* puiTopRow, size* puiBottomRow, int32* piLeft, int32* piRight)
{
	*puiTopRow = (muiBoxAnchorRow < muiBoxCaretRow) ? muiBoxAnchorRow : muiBoxCaretRow;
	*puiBottomRow = (muiBoxAnchorRow < muiBoxCaretRow) ? muiBoxCaretRow : muiBoxAnchorRow;
	*piLeft = (miBoxAnchorX < miBoxCaretX) ? miBoxAnchorX : miBoxCaretX;
	*piRight = (miBoxAnchorX < miBoxCaretX) ? miBoxCaretX : miBoxAnchorX;
}


//////////////////////////////////////////////////////////////////////////
//
// Each of the box's rows' ranges, top to bottom: its characters from the
// box's left x to its right x.  A row shorter than the left edge has an
// empty range at its end.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::GetBoxRanges(CArrayTextRange* pasDest)
{
	size		uiTop;
	size		uiBottom;
	int32		iLeft;
	int32		iRight;
	size		uiRow;
	bool		bRowEnd;
	STextRange*	psRange;

	if (!mbBox)
	{
		return;
	}
	GetBox(&uiTop, &uiBottom, &iLeft, &iRight);
	for (uiRow = uiTop; uiRow <= uiBottom; uiRow++)
	{
		psRange = pasDest->Add();
		psRange->uiStart = mpcView->FindPositionInRow(uiRow, iLeft, &bRowEnd);
		psRange->uiEnd = mpcView->FindPositionInRow(uiRow, iRight, &bRowEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A box to move the caret corner of: the one there is, or a new one with
// both corners at the caret, or the anchor at a selection's anchor.
// False if the view has no rows.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::StartBox(void)
{
	bool	bAnchorRowEnd;

	if (!mpcView->HasRows())
	{
		return false;
	}
	if (mbBox)
	{
		return true;
	}
	bAnchorRowEnd = IsSelected() ? false : mbCaretAtRowEnd;
	muiBoxAnchorRow = mpcView->FindRow(GetAnchorPosition(), bAnchorRowEnd);
	miBoxAnchorX = mpcView->GetPositionX(GetAnchorPosition(), bAnchorRowEnd);
	muiBoxCaretRow = mpcView->FindRow(muiCaret, mbCaretAtRowEnd);
	miBoxCaretX = mpcView->GetPositionX(muiCaret, mbCaretAtRowEnd);
	mbBox = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The caret goes to the box's caret corner, nothing else selected, and
// the view is told.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::PlaceBoxCaret(void)
{
	bool	bRowEnd;

	muiAnchor = MAX_SIZE;
	muiCaret = mpcView->FindPositionInRow(muiBoxCaretRow, miBoxCaretX, &bRowEnd);
	mbCaretAtRowEnd = bRowEnd;
	miUpDownX = -1;
	mpcView->MarksChanged();
	mpcView->ShowCaret();
}


//////////////////////////////////////////////////////////////////////////
//
// szText replaces each of the box's rows' ranges.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::TypeInBox(const char* szText)
{
	CArrayTextRange	asRanges;
	CArrayChars		aszTexts;
	size			i;

	asRanges.Init();
	aszTexts.Init();
	GetBoxRanges(&asRanges);
	for (i = 0; i < asRanges.NumElements(); i++)
	{
		aszTexts.Add(szText);
	}
	ReplaceBoxRanges(&asRanges, &aszTexts, FindBoxReferenceRow(&asRanges));
	aszTexts.Kill();
	asRanges.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Backspace or delete in the box: a box with width has its ranges
// deleted.  One without is a caret on every row: the character before it
// (bBackwards) or after it goes on each row that reaches the box, not
// joining lines.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::DeleteInBox(bool bBackwards)
{
	CArrayTextRange	asRanges;
	CArrayChars		aszTexts;
	STextRange*		psRange;
	size			uiTop;
	size			uiBottom;
	int32			iLeft;
	int32			iRight;
	size			uiReference;
	size			i;
	bool			bRowEnd;

	GetBox(&uiTop, &uiBottom, &iLeft, &iRight);
	if (iLeft != iRight)
	{
		TypeInBox("");
		return;
	}

	asRanges.Init();
	aszTexts.Init();
	GetBoxRanges(&asRanges);
	uiReference = FindBoxReferenceRow(&asRanges);
	for (i = 0; i < asRanges.NumElements(); i++)
	{
		psRange = asRanges.Get(i);
		aszTexts.Add("");
		if (mpcView->GetPositionX(psRange->uiStart, false) != iLeft)
		{
			continue;
		}
		if (bBackwards && (psRange->uiStart > mpcView->GetRowStart(uiTop + i)))
		{
			psRange->uiStart = mpcNavigation->PreviousPosition(psRange->uiStart);
		}
		else if (!bBackwards && (psRange->uiEnd < mpcView->GetRowEnd(uiTop + i, &bRowEnd)))
		{
			psRange->uiEnd = mpcNavigation->NextPosition(psRange->uiEnd);
		}
	}
	ReplaceBoxRanges(&asRanges, &aszTexts, uiReference);
	aszTexts.Kill();
	asRanges.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pasting in the box: as many lines as it has rows go one to a row,
// anything else goes on every row.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::PasteInBox(char* szSource)
{
	CArrayTextRange	asRanges;
	CArrayChars		aszTexts;
	CChars			szText;
	size			i;

	if ((szSource == NULL) || (szSource[0] == '\0'))
	{
		return;
	}
	szText.Init(szSource);
	MakeInsertable(&szText);
	asRanges.Init();
	aszTexts.Init();
	GetBoxRanges(&asRanges);
	if (!mbSingleLine)
	{
		if (szText.EndsWith("\n"))
		{
			szText.RemoveFromEnd(1);
		}
		szText.Split(&aszTexts, '\n');
	}
	if (aszTexts.NumElements() != asRanges.NumElements())
	{
		aszTexts.ReInit();
		szText.Set(szSource);
		MakeInsertable(&szText);
		for (i = 0; i < asRanges.NumElements(); i++)
		{
			aszTexts.Add(szText.Text());
		}
	}
	ReplaceBoxRanges(&asRanges, &aszTexts, FindBoxReferenceRow(&asRanges));
	aszTexts.Kill();
	asRanges.Kill();
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The box's rows' ranges appended to pszDest, joined by line feeds.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::CopyBox(CChars* pszDest)
{
	CArrayTextRange	asRanges;
	STextRange*		psRange;
	size			i;

	asRanges.Init();
	GetBoxRanges(&asRanges);
	for (i = 0; i < asRanges.NumElements(); i++)
	{
		psRange = asRanges.Get(i);
		if (i > 0)
		{
			pszDest->Append('\n');
		}
		pszDest->AppendSubString(mpcDocument->Text(), psRange->uiStart, psRange->uiEnd);
	}
	asRanges.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The row (from the box's top) whose edit says where the box goes after
// it: the caret's row if it reaches the box's left edge, else the first
// that does, else the caret's.
//
//////////////////////////////////////////////////////////////////////////
size CTextEditing::FindBoxReferenceRow(CArrayTextRange* pasRanges)
{
	size	uiTop;
	size	uiBottom;
	int32	iLeft;
	int32	iRight;
	size	i;

	GetBox(&uiTop, &uiBottom, &iLeft, &iRight);
	if (mpcView->GetPositionX(pasRanges->Get(muiBoxCaretRow - uiTop)->uiStart, false) == iLeft)
	{
		return muiBoxCaretRow - uiTop;
	}
	for (i = 0; i < pasRanges->NumElements(); i++)
	{
		if (mpcView->GetPositionX(pasRanges->Get(i)->uiStart, false) == iLeft)
		{
			return i;
		}
	}
	return muiBoxCaretRow - uiTop;
}


//////////////////////////////////////////////////////////////////////////
//
// Each of the rows' ranges replaced by its text, as one edit: one undo
// puts them all back (and ends the box).  The box is left with no width,
// after the reference row's text.  Text with line feeds in moves the rows:
// the box ends, the caret after the last row's text.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::ReplaceBoxRanges(CArrayTextRange* pasRanges, CArrayChars* paszTexts, size uiReferenceRow)
{
	STextRange*	psRange;
	size		i;
	size		uiShift;
	size		uiReference;
	size		uiLast;
	size		uiLength;
	bool		bLineFeeds;
	char*		szText;

	if (!BeginEditIfEditable())
	{
		return;
	}

	//Bottom up, so the ranges above don't move; where the reference row's and last row's texts end once all are in.
	bLineFeeds = false;
	uiShift = 0;
	uiReference = 0;
	uiLast = 0;
	mbBoxEditing = true;
	for (i = 0; i < pasRanges->NumElements(); i++)
	{
		psRange = pasRanges->Get(i);
		szText = paszTexts->GetText(i);
		uiLength = (size)strlen(szText);
		if (strchr(szText, '\n'))
		{
			bLineFeeds = true;
		}
		if (i == uiReferenceRow)
		{
			uiReference = psRange->uiStart + uiShift + uiLength;
		}
		uiLast = psRange->uiStart + uiShift + uiLength;
		uiShift = uiShift + uiLength - (psRange->uiEnd - psRange->uiStart);
	}
	for (i = pasRanges->NumElements(); i > 0; i--)
	{
		psRange = pasRanges->Get(i - 1);
		RemoveText(psRange->uiStart, psRange->uiEnd);
		InsertText(psRange->uiStart, paszTexts->GetText(i - 1));
	}
	mbBoxEditing = false;

	if (bLineFeeds)
	{
		mbBox = false;
		muiAnchor = MAX_SIZE;
		muiCaret = uiLast;
		EditDone();
		mpcView->ShowCaret();
		return;
	}
	muiCaret = uiReference;
	EditDone();
	miBoxAnchorX = mpcView->GetPositionX(uiReference, false);
	miBoxCaretX = miBoxAnchorX;
	PlaceBoxCaret();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextEditing::ClearSelection(void)
{
	muiAnchor = MAX_SIZE;
	mbBox = false;
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

	sz[0] = c;
	sz[1] = '\0';
	if (mbBox)
	{
		TypeInBox(sz);
		return;
	}
	if (!BeginEditIfEditable())
	{
		return;
	}

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
	bool	bRowEnd;
	size	uiColumns;
	CChars	szSpaces;

	if (mbBox)
	{
		//To the next tab stop from the box's left edge on its caret row.
		uiStart = mpcView->FindPositionInRow(muiBoxCaretRow, (miBoxAnchorX < miBoxCaretX) ? miBoxAnchorX : miBoxCaretX, &bRowEnd);
		uiColumns = mpcNavigation->CountColumns(mpcNavigation->FindLineStart(uiStart), uiStart);
		szSpaces.Init();
		szSpaces.Append(' ', muiTabWidth - (uiColumns % muiTabWidth));
		TypeInBox(szSpaces.Text());
		szSpaces.Kill();
		return;
	}
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
	if (mbBox)
	{
		DeleteInBox(false);
		return;
	}
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
	if (mbBox)
	{
		DeleteInBox(true);
		return;
	}
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
	if (mbBox)
	{
		CopyBox(pszDest);
		TypeInBox("");
		return;
	}
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

	if (mbBox)
	{
		CopyBox(pszDest);
	}
	else if (GetSelection(&uiStart, &uiEnd))
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

	if (mbBox)
	{
		PasteInBox(szSource);
		return;
	}
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
// The text from uiStart to uiEnd is moved to uiTo, or with bCopy copied
// there, as one edit: one undo puts it back, with the selection before
// it.  The text at uiTo is selected after, the caret at its end, as a
// dropped drag leaves it.  uiTo inside the text, or at either end of it,
// moves nothing.  False if nothing was moved: the text can't be edited, or
// the range is empty or past the end.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditing::MoveText(size uiStart, size uiEnd, size uiTo, bool bCopy)
{
	CChars	szText;
	size	uiLength;

	if (!mbEditable || (uiEnd <= uiStart) || (uiEnd > mpcDocument->Length()) || (uiTo > mpcDocument->Length()) || ((uiTo >= uiStart) && (uiTo <= uiEnd)))
	{
		return false;
	}

	BeginEdit();
	uiLength = uiEnd - uiStart;
	szText.Init();
	szText.AppendSubString(mpcDocument->Text(), uiStart, uiEnd);
	InsertText(uiTo, szText.Text());
	szText.Kill();
	if (!bCopy)
	{
		if (uiTo < uiStart)
		{
			uiStart += uiLength;
			uiEnd += uiLength;
		}
		RemoveText(uiStart, uiEnd);
		if (uiTo > uiStart)
		{
			uiTo -= uiLength;
		}
	}
	muiAnchor = uiTo;
	muiCaret = uiTo + uiLength;
	EditDone();
	return true;
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
	masShrinks.ReInit();
	if (!mbBoxEditing)
	{
		mbBox = false;
	}
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

