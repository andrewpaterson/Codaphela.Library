/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "Window.h"
#include "TextField.h"


//////////////////////////////////////////////////////////////////////////
//
// The theme's font is drawn as it is, so it should already be the text's
// colour.  The selection is drawn under the text and the caret beside it.
// The caret starts at the end of the text, the history with the text.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szText)
{
	CChars	szLine;

	PreInit();

	CImageBorderContainer::Init(pWindow, pTheme->GetButtonImages(BL_Normal), pTheme->GetMetric(TM_FieldPaddingX), pTheme->GetMetric(TM_FieldPaddingY));
	mpFont = pTheme->GetFont();
	mcChanged.Init();
	miColumns = 20;
	miScroll = 0;
	SetCanGetFocus(true);

	mcDocument.Init(NULL);
	mcEditing.Init(&mcDocument, this);
	mcEditing.SetSingleLine(true);
	szLine.Init(szText == NULL ? "" : szText);
	mcEditing.MakeInsertable(&szLine);
	mcDocument.SetText(szLine.Text());
	szLine.Kill();

	mpMarksDraw = OMalloc<CTextMarksCanvasDraw>(pTheme->GetColour(TC_TextSelection), pTheme->GetColour(TC_Caret));
	mpMarks = OMalloc<CCanvas>(pWindow, mpMarksDraw);
	mpMarks->SetTransparent(true);
	mpMarks->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpPadded->AddComponent(mpMarks);

	mpText = OMalloc<CLabel>(pWindow, mpFont, mcDocument.Text());
	mpText->SetAlignment(CSH_Left, CSV_Center);
	mpPadded->AddComponent(mpText);

	UpdateFaceSize();
	mcInput.Init(this, &mcEditing, false);
	mcEditing.SetCaretPosition(mcDocument.Length());
	mcEditing.ClearUndo();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Class(void)
{
	CImageBorderContainer::Class();

	U_Unknown(CTextDocument, mcDocument);
	U_Unknown(CTextEditing, mcEditing);
	U_Unknown(CTextEditingInput, mcInput);
	M_Pointer(mpFont);
	M_Pointer(mpMarksDraw);
	M_Pointer(mpMarks);
	M_Pointer(mpText);
	U_Unknown(CComponentEvent<CTextField>, mcChanged);
	U_Int32(miColumns);
	U_Int32(miScroll);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Free(void)
{
	mcChanged.Kill();
	mcInput.Kill();
	mcEditing.Kill();
	mcDocument.Kill();
	CImageBorderContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextField::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextField::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The caret goes to the end of the new text.  Line breaks become spaces,
// the field is one line.  It's an edit, so the old text can be had back
// with undo.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::SetText(const char* szText)
{
	mcEditing.ReplaceText(szText);
}


//////////////////////////////////////////////////////////////////////////
//
// The face is wide enough for the columns of average characters and a
// line high, plus the padding.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTextField::GetContentSize(void)
{
	return SInt32Vec2((int)(miColumns * mpFont->GetAverageWidth()), mpFont->GetLineHeight());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::SetColumns(int32 iColumns)
{
	miColumns = iColumns;
	UpdateFaceSize();

	//What it is laid out from has changed.
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// How far from the start of the text the gap before the character at
// uiPosition is, in pixels.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextField::GetCharacterX(size uiPosition)
{
	CChars	szPrefix;
	int32	iX;

	if (uiPosition > mcDocument.Length())
	{
		uiPosition = mcDocument.Length();
	}
	szPrefix.Init();
	szPrefix.AppendSubString(mcDocument.Text(), 0, uiPosition);
	iX = (int32)mpFont->CalculateWidth(szPrefix.Text());
	szPrefix.Kill();
	return iX;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the text starts, unscrolled, relative to the field's left.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextField::GetTextLeft(void)
{
	CInt32Rect	cText;

	mpPadded->GetContainerBounds(&cText);
	return cText.GetLeft() - GetPosition().x;
}


//////////////////////////////////////////////////////////////////////////
//
// x is relative to the field's left.  The gap between characters
// nearest to it.
//
//////////////////////////////////////////////////////////////////////////
size CTextField::FindPositionAt(int x)
{
	CTextNavigation*	pcNavigation;
	int32				iTextX;
	size				uiPosition;
	size				uiNext;
	size				uiLength;
	int32				iLeft;
	int32				iRight;

	pcNavigation = mcDocument.GetNavigation();
	iTextX = x - GetTextLeft() + miScroll;
	uiLength = mcDocument.Length();
	iLeft = 0;
	for (uiPosition = 0; uiPosition < uiLength; uiPosition = uiNext)
	{
		uiNext = pcNavigation->NextPosition(uiPosition);
		iRight = GetCharacterX(uiNext);
		if (iTextX < (iLeft + iRight) / 2)
		{
			return uiPosition;
		}
		iLeft = iRight;
	}
	return uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out the border and face, then puts the text where the caret can be
// seen and the marks under it.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	CImageBorderContainer::Layout(sPosition, sAreaSize);
	LayoutText();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::LayoutText(void)
{
	CInt32Rect	cText;
	SInt32Vec2	sLabelSize;
	int32		iWidth;
	int32		iCaretX;
	int32		iTextWidth;
	int32		iTop;
	int32		iBottom;
	size		uiStart;
	size		uiEnd;
	SInt32Vec2	sMarksPosition;
	bool		bSelectionChanged;

	mpPadded->GetContainerBounds(&cText);
	iWidth = cText.GetWidth();
	if (iWidth <= 0)
	{
		return;
	}

	//The caret is kept in view, and no more of the text than needed is scrolled out of it on the left.
	iCaretX = GetCharacterX(mcEditing.GetCaretPosition());
	iTextWidth = GetCharacterX(mcDocument.Length());
	if (iCaretX - miScroll > iWidth - 1)
	{
		miScroll = iCaretX - (iWidth - 1);
	}
	if (iCaretX - miScroll < 0)
	{
		miScroll = iCaretX;
	}
	if ((iTextWidth - miScroll < iWidth - 1) && (miScroll > 0))
	{
		miScroll = iTextWidth - (iWidth - 1);
		if (miScroll < 0)
		{
			miScroll = 0;
		}
	}

	sLabelSize = mpText->GetDesiredSize();
	iTop = cText.GetTop() + (cText.GetHeight() - sLabelSize.y) / 2;
	mpText->Layout(SInt32Vec2(cText.GetLeft() - miScroll, iTop), sLabelSize);

	//The marks are in the marks canvas' space, which covers the face less its padding.
	sMarksPosition = mpMarks->GetPosition();
	iTop = iTop - sMarksPosition.y;
	iBottom = iTop + sLabelSize.y - 1;
	//Like IntelliJ's, a field without focus doesn't show its selection.
	if (HasFocus() && GetSelection(&uiStart, &uiEnd))
	{
		bSelectionChanged = mpMarksDraw->SetSelection(GetCharacterX(uiStart) - miScroll, iTop, GetCharacterX(uiEnd) - miScroll - 1, iBottom);
	}
	else
	{
		bSelectionChanged = mpMarksDraw->HideSelection();
	}
	if (bSelectionChanged)
	{
		mpMarks->Invalidate();
	}

	if (HasFocus())
	{
		iCaretX -= miScroll;
		if (iCaretX > iWidth - 1)
		{
			iCaretX = iWidth - 1;
		}
		mpMarksDraw->SetCaret(iCaretX, iTop, iCaretX, iBottom);
	}
	else
	{
		mpMarksDraw->HideCaret();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The field has one row: all of its text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextField::HasRows(void) { return true; }
size CTextField::NumRows(void) { return 1; }
size CTextField::FindRow(size uiPosition, bool bRowEnd) { return 0; }
size CTextField::GetRowStart(size uiRow) { return 0; }
size CTextField::FindRowAt(int32 y) { return 0; }
size CTextField::FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd) { *pbRowEnd = false; return FindPositionAt(x); }
int32 CTextField::GetPositionX(size uiPosition, bool bRowEnd) { return GetTextLeft() + GetCharacterX(uiPosition) - miScroll; }
size CTextField::ScrollPage(size uiRow, bool bDown) { return uiRow; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextField::GetRowEnd(size uiRow, bool* pbRowEnd)
{
	*pbRowEnd = false;
	return mcDocument.Length();
}


//////////////////////////////////////////////////////////////////////////
//
// The marks and the scroll that keeps the caret seen are worked out when
// the field is laid out (LayoutText).
//
//////////////////////////////////////////////////////////////////////////
void CTextField::MarksChanged(void) { LayoutChanged(); }
void CTextField::ShowCaret(void) {}
void CTextField::UndoChanged(STextUndoChange* psChange) {}


//////////////////////////////////////////////////////////////////////////
//
// The label shows the edited text and the change event is called.  An
// edit shows the caret, even one that doesn't move it.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::TextEdited(void)
{
	mpText->SetText(mcDocument.Text());
	mcChanged.Call(this);
	if (HasFocus())
	{
		mpWindow->GetFocus()->ResetCaretBlink();
	}

	//What it is laid out from has changed.
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Called when the text changes, typed or set, with the text field:
// pField->GetChangeEvent()->Add(this, &CMyContent::FilterChanged).
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CTextField>* CTextField::GetChangeEvent(void)
{
	return &mcChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// The text goes back to how it was before the last edit (Undo), or
// forward to after the next (Redo).  False if there is nothing to undo or
// redo.
//
//////////////////////////////////////////////////////////////////////////
bool CTextField::Undo(void) { return mcEditing.Undo(); }
bool CTextField::Redo(void) { return mcEditing.Redo(); }
bool CTextField::CanUndo(void) { return mcDocument.CanUndo(); }
bool CTextField::CanRedo(void) { return mcDocument.CanRedo(); }
void CTextField::ClearUndo(void) { mcEditing.ClearUndo(); }  //The history is forgotten; it starts again with the text as it is.
void CTextField::SetTypingPause(int32 iMilliseconds) { mcDocument.GetUndo()->SetTypingPause(iMilliseconds); }  //Typing that stops for iMilliseconds starts a new edit.


//////////////////////////////////////////////////////////////////////////
//
// Losing focus ends a drag.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::FocusOrMouseOverChanged(void)
{
	if (!HasFocus())
	{
		mcInput.FocusLost();
	}
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
// While it has focus: the caret the marks were given, in the window, cut
// to what of the field shows.
//
//////////////////////////////////////////////////////////////////////////
bool CTextField::GetCaretRect(CInt32Rect* pcDest, ARGB32* psColour)
{
	CInt32Rect	cCaret;
	CInt32Rect	cBounds;
	CInt32Rect	cClip;
	SInt32Vec2	sPosition;

	if (!HasFocus() || !mpMarksDraw->IsCaretShown())
	{
		return false;
	}
	mpMarksDraw->GetCaret(&cCaret);
	sPosition = mpMarks->GetPosition();

	//The marks' rectangles include their right and bottom edges.
	pcDest->Init(sPosition.x + cCaret.GetLeft(), sPosition.y + cCaret.GetTop(), sPosition.x + cCaret.GetRight() + 1, sPosition.y + cCaret.GetBottom() + 1);
	cBounds.Init(sPosition.x, sPosition.y, sPosition.x + mpMarks->GetActualSize().x, sPosition.y + mpMarks->GetActualSize().y);
	pcDest->ClipTo(&cBounds);
	if (mpMarks->GetClip(&cClip))
	{
		pcDest->ClipTo(&cClip);
	}
	*psColour = mpMarksDraw->GetCaretColour();
	return (pcDest->GetWidth() > 0) && (pcDest->GetHeight() > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Enter clicks the window's default button.
//
//////////////////////////////////////////////////////////////////////////
bool CTextField::WantsEnter(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Undo and redo only with something to undo or redo.
//
//////////////////////////////////////////////////////////////////////////
bool CTextField::CanDoCommand(EComponentInputCommand eCommand)
{
	switch (eCommand)
	{
	case CIC_Undo:
		return CanUndo();
	case CIC_Redo:
		return CanRedo();
	case CIC_PasteFromHistory:
		return mcInput.CanPasteFromHistory();
	default:
		return CImageBorderContainer::CanDoCommand(eCommand);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::UpdateLook(void)
{
	UpdateBorderState(HasFocus());
}


//////////////////////////////////////////////////////////////////////////
//
// The moves and edits of the field's editing (see CTextEditing).  The
// clipboard is the window's.
//
//////////////////////////////////////////////////////////////////////////
char* CTextField::GetText(void) { return mcDocument.Text(); }
CTextEditing* CTextField::GetEditing(void) { return &mcEditing; }  //The caret, the selection and the edits: its moves and edits are made through it.
size CTextField::GetCaretPosition(void) { return mcEditing.GetCaretPosition(); }
void CTextField::SetCaretPosition(size uiPosition) { mcEditing.SetCaretPosition(uiPosition); }
bool CTextField::GetSelection(size* puiStart, size* puiEnd) { return mcEditing.GetSelection(puiStart, puiEnd); }
void CTextField::Select(size uiStart, size uiEnd) { mcEditing.Select(uiStart, uiEnd); }
void CTextField::SelectAll(void) { mcEditing.SelectAll(); }
void CTextField::CutToClipboard(void) { mcInput.CutToClipboard(); }
void CTextField::CopyToClipboard(void) { mcInput.CopyToClipboard(); }
void CTextField::PasteFromClipboard(void) { mcInput.PasteFromClipboard(); }
int32 CTextField::GetScroll(void) { return miScroll; }
Ptr<CLabel> CTextField::GetTextLabel(void) { return mpText; }
Ptr<CTextMarksCanvasDraw> CTextField::GetMarksDraw(void) { return mpMarksDraw; }

