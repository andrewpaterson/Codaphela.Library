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
#include "NativeWindowFactory.h"
#include "TextField.h"


static EComponentInputCommand	gaeTextFieldCommands[] =
{
	CIC_ActivateComponent, CIC_ExecuteComponent, CIC_StartDragSelect, CIC_ContDragSelect, CIC_StopDragSelect,
	CIC_Left, CIC_Right, CIC_WordLeft, CIC_WordRight, CIC_Home, CIC_End, CIC_DocumentHome, CIC_DocumentEnd,
	CIC_SelectLeft, CIC_SelectRight, CIC_SelectWordLeft, CIC_SelectWordRight, CIC_SelectHome, CIC_SelectEnd,
	CIC_SelectDocumentHome, CIC_SelectDocumentEnd, CIC_SelectWholeWord, CIC_SelectAll, CIC_ClearSelection,
	CIC_Space, CIC_Delete, CIC_Backspace, CIC_Cut, CIC_Copy, CIC_Paste, CIC_DeleteWordRemainingRight, CIC_DeleteWordRemainingLeft,
};


//////////////////////////////////////////////////////////////////////////
//
// The theme's font is drawn as it is, so it should already be the text's
// colour.  The selection is drawn under the text and the caret beside it.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szText)
{
	PreInit();

	CImageBorderContainer::Init(pWindow, pTheme->GetButtonImages(BL_Normal));
	mcEditor.Init(szText == NULL ? "" : szText);
	mcEditor.mbMultiLine = false;
	mcEditor.SetPosition(mcEditor.Length());
	mpFont = pTheme->GetFont();
	mcListeners.Init();
	mcListeners.AddAllowedClass<CTextFieldListener>();
	msPadding.Init(6, 2);
	miColumns = 20;
	miScroll = 0;
	muiDragAnchor = 0;
	mbFocussed = false;
	mbMouseOver = false;
	mbDragging = false;
	SetCanGetFocus(true);

	mpPadded = OMalloc<CFillContainer>(pWindow);
	mpPadded->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpFace->AddComponent(mpPadded);

	mpMarksDraw = OMalloc<CTextMarksCanvasDraw>(pTheme->GetTextSelectionColour(), pTheme->GetCaretColour());
	mpMarks = OMalloc<CCanvas>(pWindow, mpMarksDraw);
	mpMarks->SetTransparent(true);
	mpMarks->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpPadded->AddComponent(mpMarks);

	mpText = OMalloc<CLabel>(pWindow, mpFont, mcEditor.Text());
	mpText->SetAlignment(CSH_Left, CSV_Center);
	mpPadded->AddComponent(mpText);

	UpdateFaceSize();
	AddInputActions(gaeTextFieldCommands, sizeof(gaeTextFieldCommands) / sizeof(EComponentInputCommand), this, &CTextField::CommandAction);
	AddInputAction(IAF_Printable, NULL, this, &CTextField::PrintableAction);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The input actions of gaeTextFieldCommands.  The clipboard is the native
// window factory's; cutting or copying nothing leaves it as it was.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::CommandAction(SComponentInputEvent* psEvent)
{
	CChars	szClipboard;

	switch (psEvent->eCommand)
	{
	case CIC_ActivateComponent:			ActivateComponent(psEvent->x, psEvent->y);	break;
	case CIC_ExecuteComponent:			ExecuteComponent(psEvent->x, psEvent->y);	break;
	case CIC_StartDragSelect:			StartDragSelect(psEvent->x, psEvent->y);	break;
	case CIC_ContDragSelect:			ContDragSelect(psEvent->x, psEvent->y);		break;
	case CIC_StopDragSelect:			StopDragSelect(psEvent->x, psEvent->y);		break;
	case CIC_Left:						Left();						break;
	case CIC_Right:						Right();					break;
	case CIC_WordLeft:					WordLeft();					break;
	case CIC_WordRight:					WordRight();				break;
	case CIC_Home:						Home();						break;
	case CIC_End:						End();						break;
	case CIC_DocumentHome:				DocumentHome();				break;
	case CIC_DocumentEnd:				DocumentEnd();				break;
	case CIC_SelectLeft:				SelectLeft();				break;
	case CIC_SelectRight:				SelectRight();				break;
	case CIC_SelectWordLeft:			SelectWordLeft();			break;
	case CIC_SelectWordRight:			SelectWordRight();			break;
	case CIC_SelectHome:				SelectHome();				break;
	case CIC_SelectEnd:					SelectEnd();				break;
	case CIC_SelectDocumentHome:		SelectDocumentHome();		break;
	case CIC_SelectDocumentEnd:			SelectDocumentEnd();		break;
	case CIC_SelectWholeWord:			SelectWholeWord();			break;
	case CIC_SelectAll:					SelectAll();				break;
	case CIC_ClearSelection:			ClearSelection();			break;
	case CIC_Space:						Space(psEvent->bInsert);	break;
	case CIC_Delete:					Delete();					break;
	case CIC_Backspace:					Backspace();				break;
	case CIC_DeleteWordRemainingRight:	DeleteWordRemainingRight();	break;
	case CIC_DeleteWordRemainingLeft:	DeleteWordRemainingLeft();	break;
	case CIC_Cut:
	case CIC_Copy:
		szClipboard.Init();
		if (psEvent->eCommand == CIC_Cut)
		{
			Cut(&szClipboard);
		}
		else
		{
			Copy(&szClipboard);
		}
		if (!szClipboard.Empty())
		{
			mpWindow->GetFactory()->SetClipboardText(szClipboard.Text());
		}
		szClipboard.Kill();
		break;
	case CIC_Paste:
		szClipboard.Init();
		mpWindow->GetFactory()->GetClipboardText(&szClipboard);
		Paste(szClipboard.Text());
		szClipboard.Kill();
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::PrintableAction(SComponentInputEvent* psEvent)
{
	Printable(psEvent->cPrintable, psEvent->bInsert);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Class(void)
{
	CImageBorderContainer::Class();

	U_Data(CPlainTextEditor, mcEditor);
	M_Pointer(mpFont);
	M_Pointer(mpPadded);
	M_Pointer(mpMarksDraw);
	M_Pointer(mpMarks);
	M_Pointer(mpText);
	U_Data(CListenerCall, mcListeners);
	U_2Int32(msPadding);
	U_Int32(miColumns);
	U_Int32(miScroll);
	U_Size(muiDragAnchor);
	U_Bool(mbFocussed);
	U_Bool(mbMouseOver);
	U_Bool(mbDragging);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Free(void)
{
	mcListeners.Kill();
	mcEditor.Kill();
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
// The caret goes to the end of the new text.  Line feeds become spaces,
// the field is one line.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::SetText(const char* szText)
{
	mcEditor.mszText.Kill();
	mcEditor.mszText.Init(szText == NULL ? "" : szText);
	mcEditor.mszText.Replace("\r\n", " ");
	mcEditor.mszText.Replace('\n', ' ');
	mcEditor.mszText.Replace('\r', ' ');
	mcEditor.SetPosition(mcEditor.Length());
	TextEdited();
}


//////////////////////////////////////////////////////////////////////////
//
// The face is wide enough for the columns of average characters and a
// line high, plus the padding.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::UpdateFaceSize(void)
{
	SInsets		sInsets;
	int			aiInsets[4];

	aiInsets[0] = msPadding.y;
	aiInsets[1] = msPadding.x;
	aiInsets[2] = msPadding.y;
	aiInsets[3] = msPadding.x;
	sInsets.Init(aiInsets);
	mpPadded->SetInsets(&sInsets);

	mpFace->SetDesiredSize((int)(miColumns * mpFont->GetAverageWidth()) + msPadding.x * 2, mpFont->GetLineHeight() + msPadding.y * 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::SetColumns(int32 iColumns)
{
	miColumns = iColumns;
	UpdateFaceSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::SetPadding(int32 iHorizontal, int32 iVertical)
{
	msPadding.Init(iHorizontal, iVertical);
	UpdateFaceSize();
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

	if (uiPosition > mcEditor.Length())
	{
		uiPosition = mcEditor.Length();
	}
	szPrefix.Init();
	szPrefix.AppendSubString(mcEditor.mszText, 0, uiPosition);
	iX = (int32)mpFont->CalculateWidth(szPrefix.Text());
	szPrefix.Kill();
	return iX;
}


//////////////////////////////////////////////////////////////////////////
//
// x is relative to the field's left.  The gap between characters
// nearest to it.
//
//////////////////////////////////////////////////////////////////////////
size CTextField::FindPositionAt(int x)
{
	CInt32Rect	cText;
	int32		iTextX;
	size		uiPosition;
	size		uiLength;
	int32		iLeft;
	int32		iRight;

	mpPadded->GetContainerBounds(&cText);
	iTextX = x + GetPosition().x - cText.GetLeft() + miScroll;

	uiLength = mcEditor.Length();
	iLeft = 0;
	for (uiPosition = 0; uiPosition < uiLength; uiPosition++)
	{
		iRight = GetCharacterX(uiPosition + 1);
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

	mpPadded->GetContainerBounds(&cText);
	iWidth = cText.GetWidth();
	if (iWidth <= 0)
	{
		return;
	}

	//The caret is kept in view, and no more of the text than needed is scrolled out of it on the left.
	iCaretX = GetCharacterX(mcEditor.miEditPos);
	iTextWidth = GetCharacterX(mcEditor.Length());
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
	if (mbFocussed && GetSelection(&uiStart, &uiEnd))
	{
		mpMarksDraw->SetSelection(GetCharacterX(uiStart) - miScroll, iTop, GetCharacterX(uiEnd) - miScroll - 1, iBottom);
	}
	else
	{
		mpMarksDraw->HideSelection();
	}

	if (mbFocussed)
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
// The label shows the editor's text.  The listeners are only told if it
// changed.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::TextEdited(void)
{
	if (!mcEditor.mszText.Equals(mpText->GetText()))
	{
		mpText->SetText(mcEditor.Text());
		mcListeners.CallListeners(&CTextFieldListener::TextChanged, this, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if nothing is selected.  The start is before the end.
//
//////////////////////////////////////////////////////////////////////////
bool CTextField::GetSelection(size* puiStart, size* puiEnd)
{
	if ((mcEditor.miSelectionAnchor == MAX_SIZE) || (mcEditor.miSelectionFloating == MAX_SIZE))
	{
		*puiStart = mcEditor.miEditPos;
		*puiEnd = mcEditor.miEditPos;
		return false;
	}
	if (mcEditor.miSelectionAnchor < mcEditor.miSelectionFloating)
	{
		*puiStart = mcEditor.miSelectionAnchor;
		*puiEnd = mcEditor.miSelectionFloating;
	}
	else
	{
		*puiStart = mcEditor.miSelectionFloating;
		*puiEnd = mcEditor.miSelectionAnchor;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Selects from uiStart to uiEnd with the caret at uiEnd.  The same start
// and end selects nothing.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Select(size uiStart, size uiEnd)
{
	if (uiStart > mcEditor.Length())
	{
		uiStart = mcEditor.Length();
	}
	mcEditor.SetPosition(uiStart);
	muiDragAnchor = uiStart;
	SelectTo(uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the caret to uiPosition with the selection from the drag anchor.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::SelectTo(size uiPosition)
{
	//The text may have been edited, shortened, since the anchor was set.
	if (uiPosition > mcEditor.Length())
	{
		uiPosition = mcEditor.Length();
	}
	if (muiDragAnchor > mcEditor.Length())
	{
		muiDragAnchor = mcEditor.Length();
	}
	mcEditor.miEditPos = uiPosition;
	if (uiPosition == muiDragAnchor)
	{
		mcEditor.ClearSelection();
	}
	else
	{
		mcEditor.miSelectionAnchor = muiDragAnchor;
		mcEditor.miSelectionFloating = uiPosition;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::SetCaretPosition(size uiPosition)
{
	if (uiPosition > mcEditor.Length())
	{
		uiPosition = mcEditor.Length();
	}
	mcEditor.SetPosition(uiPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextField::AddListener(CTextFieldListener* pcListener)
{
	return mcListeners.AddListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextField::RemoveListener(CTextFieldListener* pcListener)
{
	return mcListeners.RemoveListener(pcListener);
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
//
//////////////////////////////////////////////////////////////////////////
void CTextField::UpdateLook(void)
{
	if (mbFocussed)
	{
		SetBorderState(BS_Focussed);
	}
	else if (mbMouseOver)
	{
		SetBorderState(BS_MouseOver);
	}
	else
	{
		SetBorderState(BS_Normal);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::MovedInto(void)
{
	mbMouseOver = true;
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::MovedOutof(void)
{
	mbMouseOver = false;
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::FocusGained(void)
{
	mbFocussed = true;
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
// Losing focus, as when the window is deactivated, ends a drag.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::FocusLost(void)
{
	mbFocussed = false;
	mbDragging = false;
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
// The press has already given the field focus.  The caret goes to the
// gap nearest the pointer and dragging selects from there.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::ActivateComponent(int x, int y)
{
	mcEditor.SetPosition(FindPositionAt(x));
	mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::ExecuteComponent(int x, int y)
{
	mpWindow->GetFocus()->StopDrag(FD_Select);
}


//////////////////////////////////////////////////////////////////////////
//
// A shift press extends the selection from where it started, or from the
// caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::StartDragSelect(int x, int y)
{
	size	uiStart;
	size	uiEnd;

	if (GetSelection(&uiStart, &uiEnd))
	{
		muiDragAnchor = mcEditor.miSelectionAnchor;
	}
	else
	{
		muiDragAnchor = mcEditor.miEditPos;
	}
	mbDragging = true;
	SelectTo(FindPositionAt(x));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::ContDragSelect(int x, int y)
{
	if (mbDragging)
	{
		SelectTo(FindPositionAt(x));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextField::StopDragSelect(int x, int y)
{
	if (mbDragging)
	{
		SelectTo(FindPositionAt(x));
		mbDragging = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Editing.  The editor does the work and the label is updated after
// anything that can change the text.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Left(void) { mcEditor.Left(); }
void CTextField::Right(void) { mcEditor.Right(); }
void CTextField::WordLeft(void) { mcEditor.WordLeft(); }
void CTextField::WordRight(void) { mcEditor.WordRight(); }
void CTextField::Home(void) { mcEditor.Home(); }
void CTextField::End(void) { mcEditor.End(); }
void CTextField::DocumentHome(void) { mcEditor.DocumentHome(); }
void CTextField::DocumentEnd(void) { mcEditor.DocumentEnd(); }
void CTextField::SelectLeft(void) { mcEditor.SelectLeft(); }
void CTextField::SelectRight(void) { mcEditor.SelectRight(); }
void CTextField::SelectWordLeft(void) { mcEditor.SelectWordLeft(); }
void CTextField::SelectWordRight(void) { mcEditor.SelectWordRight(); }
void CTextField::SelectHome(void) { mcEditor.SelectHome(); }
void CTextField::SelectEnd(void) { mcEditor.SelectEnd(); }
void CTextField::SelectDocumentHome(void) { mcEditor.SelectDocumentHome(); }
void CTextField::SelectDocumentEnd(void) { mcEditor.SelectDocumentEnd(); }
void CTextField::SelectWholeWord(void) { mcEditor.SelectWholeWord(); }
void CTextField::SelectAll(void) { mcEditor.SelectAll(); }
void CTextField::ClearSelection(void) { mcEditor.ClearSelection(); }
void CTextField::Printable(char c, bool bInsert) { mcEditor.Printable(c, bInsert); TextEdited(); }
void CTextField::Space(bool bInsert) { mcEditor.Printable(' ', bInsert); TextEdited(); }
void CTextField::Delete(void) { mcEditor.Delete(); TextEdited(); }
void CTextField::Backspace(void) { mcEditor.Backspace(); TextEdited(); }

void CTextField::Copy(CChars* pszDest) { mcEditor.Copy(pszDest); }
void CTextField::DeleteWordRemainingRight(void) { mcEditor.DeleteWordRemainingRight(); TextEdited(); }
void CTextField::DeleteWordRemainingLeft(void) { mcEditor.DeleteWordRemainingLeft(); TextEdited(); }


//////////////////////////////////////////////////////////////////////////
//
// Without a selection nothing is cut.  The editor would cut the whole
// line, all of a field's text.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Cut(CChars* pszDest)
{
	size	uiStart;
	size	uiEnd;

	if (GetSelection(&uiStart, &uiEnd))
	{
		mcEditor.Cut(pszDest);
		TextEdited();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Line feeds become spaces, the field is one line.  Pasting nothing, from
// an empty clipboard, leaves a selection where it is.
//
//////////////////////////////////////////////////////////////////////////
void CTextField::Paste(char* szSource)
{
	CChars	szLine;

	if ((szSource == NULL) || (szSource[0] == '\0'))
	{
		return;
	}

	szLine.Init(szSource);
	szLine.Replace("\r\n", " ");
	szLine.Replace('\n', ' ');
	szLine.Replace('\r', ' ');
	mcEditor.Paste(szLine.Text());
	szLine.Kill();
	TextEdited();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CTextField::GetText(void) { return mcEditor.Text(); }
size CTextField::GetCaretPosition(void) { return mcEditor.miEditPos; }
int32 CTextField::GetScroll(void) { return miScroll; }
Ptr<CLabel> CTextField::GetTextLabel(void) { return mpText; }
Ptr<CTextMarksCanvasDraw> CTextField::GetMarksDraw(void) { return mpMarksDraw; }

