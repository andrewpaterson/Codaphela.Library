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
#include "BaseLib/Logger.h"
#include "BaseLib/ArrayElementNotFound.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "Window.h"
#include "BaseLib/NameMatcher.h"
#include "ScrollPane.h"
#include "ListContainer.h"


//////////////////////////////////////////////////////////////////////////
//
// The highlights are the theme's list selected and hover colours.  Page up
// and page down move ten rows unless a scroll pane shows the list (see
// GetPageRows).
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	PreInit();

	CGridContainer::Init(pWindow);
	SetSizedByContent(true);
	SetHitAsWhole(true);
	mabEnabled.Init();
	mcSelectionChanged.Init();
	mcActivated.Init();
	miSelected = -1;
	miHover = -1;
	miPageRows = 10;
	mbWrapAround = false;
	mbPressed = false;
	mbSelectOnHover = false;
	mbDoubleClickActivates = false;
	msDoubleClick.Init();
	mbWantsEscape = false;
	mbMultiSelect = false;
	mabSelected.Init();
	miAnchor = -1;
	mpTheme = pTheme;
	mbTypeToFind = false;
	mbFindActions = false;
	mbFindMatched = false;
	mszFind.Init();
	SetCanGetFocus(true);

	mpSelectedDraw = OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_ListSelected));
	mpSelectedHighlight = CreateHighlight(mpSelectedDraw);
	mpHoverDraw = OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_ListHover));
	mpHoverHighlight = CreateHighlight(mpHoverDraw);
	AddHighlights();
	AddInputAction(CIC_ActivateComponent, this, &CListContainer::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CListContainer::ExecuteComponent);
	AddInputAction(CIC_ContextMenu, this, &CListContainer::ContextMenuClick);
	AddInputAction(CIC_Up, this, &CListContainer::Up);
	AddInputAction(CIC_Down, this, &CListContainer::Down);
	AddInputAction(CIC_Home, this, &CListContainer::Home);
	AddInputAction(CIC_End, this, &CListContainer::End);
	AddInputAction(CIC_DocumentHome, this, &CListContainer::DocumentHome);
	AddInputAction(CIC_DocumentEnd, this, &CListContainer::DocumentEnd);
	AddInputAction(CIC_PageUp, this, &CListContainer::PageUp);
	AddInputAction(CIC_PageDown, this, &CListContainer::PageDown);
	AddInputAction(CIC_Space, this, &CListContainer::Space);
	AddInputAction(CIC_Enter, this, &CListContainer::Activate);
	AddInputAction(CIC_JumpToSource, this, &CListContainer::Activate);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpSelectedDraw);
	M_Pointer(mpSelectedHighlight);
	M_Pointer(mpHoverDraw);
	M_Pointer(mpHoverHighlight);
	U_Unknown(CArrayBit, mabEnabled);
	U_Unknown(CComponentEvent<CListContainer>, mcSelectionChanged);
	U_Unknown(CComponentEvent<CListContainer>, mcActivated);
	U_Int32(miSelected);
	U_Int32(miHover);
	U_Int32(miPageRows);
	U_Bool(mbPressed);
	U_Bool(mbSelectOnHover);
	U_Bool(mbDoubleClickActivates);
	U_Bool(mbWrapAround);
	U_Data(SDoubleClick, msDoubleClick);
	U_Bool(mbWantsEscape);
	U_Bool(mbMultiSelect);
	U_Unknown(CArrayBit, mabSelected);
	U_Int32(miAnchor);
	M_Pointer(mpSelectedBands);
	M_Pointer(mpTheme);
	U_Bool(mbTypeToFind);
	U_Bool(mbFindActions);
	U_Bool(mbFindMatched);
	U_String(mszFind);
	M_Pointer(mpFindNote);
	M_Pointer(mpFindBorder);
	M_Pointer(mpFindLabel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Free(void)
{
	mszFind.Kill();
	mcActivated.Kill();
	mcSelectionChanged.Kill();
	mabSelected.Kill();
	mabEnabled.Kill();
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The highlights are always the first components in the grid, so that the
// rows are drawn over them.  They sit in the top left cell with no size of
// their own, so they don't change the grid, and are moved to their rows
// after it is laid out.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::AddHighlights(void)
{
	CGridContainer::AddComponent(mpSelectedHighlight, 0, 0);
	CGridContainer::AddComponent(mpHoverHighlight, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvas> CListContainer::CreateHighlight(Ptr<CClearCanvasDraw> pDraw)
{
	Ptr<CCanvas>	pHighlight;

	pHighlight = OMalloc<CCanvas>(mpWindow, pDraw);
	pHighlight->SetDesiredSize(-1, -1);
	return pHighlight;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the new row's index.  The row is empty until components are
// added to it.
//
//////////////////////////////////////////////////////////////////////////
int32 CListContainer::AddRow(bool bEnabled)
{
	mabEnabled.Add(bEnabled);
	mabSelected.Add(false);
	return NumRows() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// A column with a width weight takes the width the list has spare.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::AddToRow(int32 iRow, Ptr<CComponent> pComponent, int iColumn, int iColumnSpan, int iWidthWeight)
{
	if ((iRow < 0) || (iRow >= NumRows()))
	{
		return gcLogger.Error2(__METHOD__, " The row doesn't exist.", NULL);
	}
	return CGridContainer::AddComponent(pComponent, iColumn, iRow, iColumnSpan, 1, iWidthWeight, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A row holding only the item, which takes the list's whole width.
// Returns the row's index.
//
//////////////////////////////////////////////////////////////////////////
int32 CListContainer::AddItem(Ptr<CComponent> pItem, bool bEnabled)
{
	int32	iRow;

	iRow = AddRow(bEnabled);
	AddToRow(iRow, pItem, 0, 1, 1);
	return iRow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::RemoveAllComponents(void)
{
	bool	bResult;

	Select(-1);
	miHover = -1;
	mbPressed = false;
	msDoubleClick.Forget();
	bResult = CGridContainer::RemoveAllComponents();
	mabEnabled.ReInit();
	mabSelected.ReInit();
	AddHighlights();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Disabling the selected row clears the selection.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SetRowEnabled(int32 iRow, bool bEnabled)
{
	if ((iRow < 0) || (iRow >= NumRows()))
	{
		return;
	}

	mabEnabled.Set(iRow, bEnabled);
	if (!bEnabled && (iRow == miSelected))
	{
		Select(-1);
	}
	UpdateHighlights();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::IsRowEnabled(int32 iRow)
{
	if ((iRow < 0) || (iRow >= NumRows()))
	{
		return false;
	}
	return mabEnabled.Get(iRow);
}


//////////////////////////////////////////////////////////////////////////
//
// -1 clears the selection.  Returns false, leaving the selection as it
// is, if the row doesn't exist or is disabled.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::Select(int32 iRow)
{
	if (iRow < 0)
	{
		iRow = -1;
	}
	else if (!IsRowEnabled(iRow))
	{
		return false;
	}

	if ((iRow != miSelected) || (mbMultiSelect && (NumSelected() != ((iRow >= 0) ? 1 : 0))))
	{
		ClearSelected();
		miSelected = iRow;
		miAnchor = iRow;
		if (iRow >= 0)
		{
			mabSelected.Set(iRow, true);
		}
		SelectionChanged();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether several rows can be selected.  Turned off, only the selected
// row stays selected.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SetMultiSelect(bool bMultiSelect)
{
	int32	iSelected;

	if (bMultiSelect == mbMultiSelect)
	{
		return;
	}
	mbMultiSelect = bMultiSelect;
	if (mbMultiSelect)
	{
		//One canvas over the whole list draws every selected row's band.  The keys and presses that select several
		//are the list's from the first time it can: a list that never can (a menu) leaves them to whatever has them,
		//the edit commands' target say.
		if (mpSelectedBands.IsNull())
		{
			mpSelectedBands = OMalloc<CBandsCanvasDraw>(mpSelectedDraw->GetColour());
			AddInputAction(CIC_SelectUp, this, &CListContainer::SelectMove);
			AddInputAction(CIC_SelectDown, this, &CListContainer::SelectMove);
			AddInputAction(CIC_SelectPageUp, this, &CListContainer::SelectMove);
			AddInputAction(CIC_SelectPageDown, this, &CListContainer::SelectMove);
			AddInputAction(CIC_SelectHome, this, &CListContainer::SelectMove);
			AddInputAction(CIC_SelectEnd, this, &CListContainer::SelectMove);
			AddInputAction(CIC_SelectDocumentHome, this, &CListContainer::SelectMove);
			AddInputAction(CIC_SelectDocumentEnd, this, &CListContainer::SelectMove);
			AddInputAction(CIC_StartDragSelect, this, &CListContainer::SelectPress);
			AddInputAction(CIC_SelectAll, this, &CListContainer::SelectAllAction);
		}
		mpSelectedHighlight->SetCanvasDraw(mpSelectedBands);
		mpSelectedHighlight->SetTransparent(true);
		UpdateHighlights();
	}
	else
	{
		mpSelectedHighlight->SetCanvasDraw(mpSelectedDraw);
		mpSelectedHighlight->SetTransparent(false);
		iSelected = IsRowSelected(miSelected) ? miSelected : -1;
		miSelected = -2;
		Select(iSelected);
	}
}


bool CListContainer::IsMultiSelect(void) const { return mbMultiSelect; }


//////////////////////////////////////////////////////////////////////////
//
// With several selectable, the selected row itself may not be: Control
// with a press unselects it.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::IsRowSelected(int32 iRow)
{
	if ((iRow < 0) || (iRow >= NumRows()))
	{
		return false;
	}
	return mabSelected.Get(iRow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CListContainer::NumSelected(void)
{
	return (int32)mabSelected.CountSet();
}


//////////////////////////////////////////////////////////////////////////
//
// The enabled rows from iFrom to iTo, either way round, are the ones
// selected, iTo the selected row and iFrom where a further range starts.
// Only when several can be selected: otherwise iTo is selected.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SelectRange(int32 iFrom, int32 iTo)
{
	int32	i;
	int32	iFirst;
	int32	iLast;

	if (!mbMultiSelect || (iFrom < 0) || (iFrom >= NumRows()))
	{
		Select(iTo);
		return;
	}
	if ((iTo < 0) || (iTo >= NumRows()))
	{
		return;
	}

	iFirst = (iFrom < iTo) ? iFrom : iTo;
	iLast = (iFrom < iTo) ? iTo : iFrom;
	ClearSelected();
	for (i = iFirst; i <= iLast; i++)
	{
		mabSelected.Set(i, IsRowEnabled(i));
	}
	miSelected = iTo;
	miAnchor = iFrom;
	SelectionChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// The row is selected if it wasn't and unselected if it was, the others
// staying as they are, and it is the selected row a range would start
// from.  Only when several can be selected: otherwise it is selected.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::ToggleRow(int32 iRow)
{
	if (!mbMultiSelect)
	{
		Select(iRow);
		return;
	}
	if (!IsRowEnabled(iRow))
	{
		return;
	}
	mabSelected.Set(iRow, !mabSelected.Get(iRow));
	miSelected = iRow;
	miAnchor = iRow;
	SelectionChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Every enabled row, when several can be selected.  The selected row
// stays the one it was, or is the first.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SelectAll(void)
{
	int32	i;

	if (!mbMultiSelect || (NumRows() == 0))
	{
		return;
	}
	for (i = 0; i < NumRows(); i++)
	{
		mabSelected.Set(i, IsRowEnabled(i));
	}
	if (miSelected < 0)
	{
		miSelected = FindEnabled(0, 1);
	}
	miAnchor = miSelected;
	SelectionChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// The selection changed: it is drawn and the listeners are told.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SelectionChanged(void)
{
	UpdateHighlights();
	RowSelected(miSelected);
	mcSelectionChanged.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::ClearSelected(void)
{
	int32	i;

	for (i = 0; i < NumRows(); i++)
	{
		mabSelected.Set(i, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Activate(void)
{
	StopFinding();
	if (miSelected >= 0)
	{
		RowActivated(miSelected);
		mcActivated.Call(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are in window client space.  Returns -1 if they aren't over a
// row, as when the list is taller than its rows.
//
//////////////////////////////////////////////////////////////////////////
int32 CListContainer::FindRowAt(int x, int y)
{
	if (!IsPointIn(x, y))
	{
		return -1;
	}
	return FindRowAtY(y);
}


//////////////////////////////////////////////////////////////////////////
//
// The row y, in window client space, is in, -1 for none.  The grid's rows
// as they were laid out last.
//
//////////////////////////////////////////////////////////////////////////
int32 CListContainer::FindRowAtY(int32 y)
{
	int32	i;
	int32	iTop;
	int32	iHeight;

	for (i = 0; i < NumRows(); i++)
	{
		if (GetRowBounds(i, &iTop, &iHeight))
		{
			if ((y >= iTop) && (y < iTop + iHeight))
			{
				return i;
			}
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Where iRow starts and how tall it is, in window client space: the
// grid's row as it was laid out last.  False if there was no such row.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::GetRowBounds(int32 iRow, int32* piTop, int32* piHeight)
{
	if (iRow < 0)
	{
		return false;
	}
	return GetTrackBounds(false, (size)iRow, piTop, piHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// For a list whose rows aren't the grid's: it has iNumRows rows, every
// one enabled and none selected, as if they had all been removed and
// added again.  The selected row is the caller's to carry over.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SetNumRows(int32 iNumRows)
{
	int32	i;

	miHover = -1;
	mbPressed = false;
	msDoubleClick.Forget();
	mabEnabled.ReInit();
	mabSelected.ReInit();
	for (i = 0; i < iNumRows; i++)
	{
		mabEnabled.Add(true);
		mabSelected.Add(false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Like any fill colour they are red, green and blue whatever the
// window's colour order; the native canvas orders them.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SetHighlightColours(ARGB32 sSelected, ARGB32 sHover)
{
	mpSelectedDraw->SetColour(sSelected);
	if (mpSelectedBands.IsNotNull())
	{
		mpSelectedBands->SetColour(sSelected);
	}
	mpHoverDraw->SetColour(sHover);
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the plain blocks the highlights are drawn with, by rounded
// ones say.  The highlights become transparent so whatever the draws
// leave empty shows the list's background.  SetHighlightColours no longer
// changes them.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SetHighlightDraws(Ptr<CCanvasDraw> pSelected, Ptr<CCanvasDraw> pHover)
{
	mpSelectedHighlight->SetCanvasDraw(pSelected);
	mpSelectedHighlight->SetTransparent(true);
	mpHoverHighlight->SetCanvasDraw(pHover);
	mpHoverHighlight->SetTransparent(true);
}


//////////////////////////////////////////////////////////////////////////
//
// Called after the selected row changes, as it is cleared too, with the
// list: pList->GetSelectionEvent()->Add(this, &CMyContent::RowChosen).
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CListContainer>* CListContainer::GetSelectionEvent(void)
{
	return &mcSelectionChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Called when the selected row is activated: by enter or space, by a
// click, or by a double click if double clicks activate.
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CListContainer>* CListContainer::GetActivationEvent(void)
{
	return &mcActivated;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	CGridContainer::Layout(sPosition, sAreaSize);
	UpdateHighlights();
}


//////////////////////////////////////////////////////////////////////////
//
// The highlights are always in the grid but are only drawn while they
// mark a row.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::IsChildDrawn(Ptr<CComponent> pChild)
{
	if (pChild == mpSelectedHighlight)
	{
		return mbMultiSelect ? (NumSelected() > 0) : (miSelected >= 0);
	}
	if (pChild == mpHoverHighlight)
	{
		return IsHoverShown();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Enter activates the selected row rather than clicking the default
// button.  With nothing selected the default button gets it.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::WantsEnter(void)
{
	return miSelected >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Escape comes to the list's input actions instead of clicking the
// window's cancel button while the list has something of its own to
// escape from, as a dialog's list has while what was typed filters it.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::WantsEscape(void) { return mbWantsEscape || IsFinding(); }
void CListContainer::SetWantsEscape(bool bWantsEscape) { mbWantsEscape = bWantsEscape; }


//////////////////////////////////////////////////////////////////////////
//
// Losing focus, as when the window is deactivated, ends a press.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::FocusLost(void)
{
	StopFinding();
	mbPressed = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::MovedOutof(void)
{
	SetHover(-1);
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the list's top left.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::MovedOver(int x, int y)
{
	SInt32Vec2	sPosition;

	sPosition = GetPosition();
	SetHover(FindRowAt(x + sPosition.x, y + sPosition.y));
}


//////////////////////////////////////////////////////////////////////////
//
// The press has already given the list focus.  Pressing a disabled row,
// or below the rows, selects nothing.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::ActivateComponent(int x, int y)
{
	SInt32Vec2	sPosition;
	int32		iRow;

	StopFinding();
	sPosition = GetPosition();
	iRow = FindRowAt(x + sPosition.x, y + sPosition.y);
	mbPressed = iRow >= 0;
	if (mbPressed)
	{
		Select(iRow);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The right button, as in Windows: the row under the pointer is selected,
// so the context menu acts on it.  Below the rows the selection stays.
// The press goes on to the context menu (the list's own, or a parent's).
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::ContextMenuClick(SComponentInputEvent* psEvent)
{
	SInt32Vec2	sPosition;
	int32		iRow;

	psEvent->bPass = true;
	sPosition = GetPosition();
	iRow = FindRowAt(psEvent->x + sPosition.x, psEvent->y + sPosition.y);
	if (iRow >= 0)
	{
		Select(iRow);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A context menu opened from the keys opens under the selected row, at
// the list's left, or at the list's top left if no row is selected.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CListContainer::GetContextMenuPosition(void)
{
	SInt32Vec2	sPosition;
	int32		iTop;
	int32		iHeight;

	sPosition = GetPosition();
	if ((miSelected >= 0) && GetRowBounds(miSelected, &iTop, &iHeight))
	{
		sPosition.y = iTop + iHeight;
	}
	return sPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// The release goes to the list with focus wherever the pointer is.  It
// only activates if the pointer is still over the selected row and, if
// double clicks activate, the row was released on just before.  The row
// is found from where the pointer is, not the hover row, which is only
// found when the pointer moves; a row can come under it without that.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::ExecuteComponent(int x, int y)
{
	SInt32Vec2	sPosition;

	if (mbPressed)
	{
		mbPressed = false;
		sPosition = GetPosition();
		if ((miSelected >= 0) && mpWindow->GetFocus()->IsPointerIn() && (FindRowAt(x + sPosition.x, y + sPosition.y) == miSelected))
		{
			if (!mbDoubleClickActivates || msDoubleClick.Released(miSelected, x, y))
			{
				Activate();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// With nothing selected up selects the last enabled row and down the
// first.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Home(void) { StopFinding(); SelectEnabled(FindMove(CIC_Home)); }
void CListContainer::End(void) { StopFinding(); SelectEnabled(FindMove(CIC_End)); }
void CListContainer::DocumentHome(void) { StopFinding(); SelectEnabled(FindMove(CIC_Home)); }
void CListContainer::DocumentEnd(void) { StopFinding(); SelectEnabled(FindMove(CIC_End)); }
void CListContainer::PageUp(void) { StopFinding(); SelectEnabled(FindMove(CIC_PageUp)); }
void CListContainer::PageDown(void) { StopFinding(); SelectEnabled(FindMove(CIC_PageDown)); }


//////////////////////////////////////////////////////////////////////////
//
// While a row is being found by what is typed, up and down go to the
// previous and the next row that matches and space is typed too.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Up(void)
{
	if (IsFinding())
	{
		FindNext(-1);
	}
	else
	{
		SelectEnabled(FindMove(CIC_Up));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Down(void)
{
	if (IsFinding())
	{
		FindNext(1);
	}
	else
	{
		SelectEnabled(FindMove(CIC_Down));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Space(void)
{
	if (IsFinding())
	{
		FindType(' ');
	}
	else
	{
		Activate();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Typed characters find a row, or don't.  The actions that take them are
// added the first time it is asked for: a character, backspace and escape
// that aren't the list's (nothing is being found) go on to its parents.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SetTypeToFind(bool bTypeToFind)
{
	if (bTypeToFind && !mbFindActions)
	{
		AddInputAction(IAF_Printable, NULL, this, &CListContainer::FindPrintableAction);
		AddInputAction(CIC_Backspace, this, &CListContainer::FindBackspaceAction);
		AddInputAction(CIC_ClearSelection, this, &CListContainer::FindEscapeAction);
		mbFindActions = true;
	}
	if (!bTypeToFind)
	{
		StopFinding();
	}
	mbTypeToFind = bTypeToFind;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::FindPrintableAction(SComponentInputEvent* psEvent)
{
	if (!mbTypeToFind || !FindType(psEvent->cPrintable))
	{
		psEvent->bPass = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::FindBackspaceAction(SComponentInputEvent* psEvent)
{
	if (!FindBackspace())
	{
		psEvent->bPass = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::FindEscapeAction(SComponentInputEvent* psEvent)
{
	if (IsFinding())
	{
		StopFinding();
	}
	else
	{
		psEvent->bPass = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// c typed: it is added to the pattern and the first row that matches it,
// from the selected one, is selected.  False if it isn't the list's: it
// doesn't find rows, c isn't a character, or it is a space with nothing
// typed before it.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::FindType(char c)
{
	if (!mbTypeToFind || ((uint8)c < 32) || ((c == ' ') && mszFind.Empty()))
	{
		return false;
	}
	if (mszFind.Length() < NAME_MATCHER_PATTERN - 1)
	{
		mszFind.Append(c);
		mbFindMatched = FindNext(0);
		ShowFindNote();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The last character typed is taken back: with none left nothing is
// being found any more.  False if nothing was.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::FindBackspace(void)
{
	if (!IsFinding())
	{
		return false;
	}
	mszFind.RemoveFromEnd(1);
	if (mszFind.Empty())
	{
		StopFinding();
	}
	else
	{
		mbFindMatched = FindNext(0);
		ShowFindNote();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The selected row if iStep is 0 and it matches, else the next (1) or
// previous (-1) row that does, round the ends, is selected and shown.
// False if no row matches, or no other row with a step.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::FindNext(int32 iStep)
{
	int32	iRow;

	if (!IsFinding())
	{
		return false;
	}
	if (iStep == 0)
	{
		iRow = FindMatch(miSelected < 0 ? 0 : miSelected, 1);
	}
	else
	{
		iRow = FindMatch(miSelected < 0 ? 0 : miSelected + iStep, iStep);
		if (iRow == miSelected)
		{
			return false;
		}
	}
	if (iRow < 0)
	{
		return false;
	}
	Select(iRow);
	ShowRow(iRow);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The first enabled row that matches what has been typed, from iFrom
// going by iStep (1 or -1) round the ends and back to it; -1 for none.
//
//////////////////////////////////////////////////////////////////////////
int32 CListContainer::FindMatch(int32 iFrom, int32 iStep)
{
	int32	i;
	int32	iRow;
	int32	iNumRows;

	iNumRows = NumRows();
	for (i = 0; i < iNumRows; i++)
	{
		iRow = (((iFrom + i * iStep) % iNumRows) + iNumRows) % iNumRows;
		if (IsRowEnabled(iRow) && IsRowFound(iRow))
		{
			return iRow;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether iRow's text matches what has been typed: as a name matches in
// Go to, or with all of it anywhere in the text, ignoring case.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::IsRowFound(int32 iRow)
{
	CChars			szText;
	CNameMatcher	cMatcher;
	SNameMatch		sMatch;
	bool			bFound;

	szText.Init();
	bFound = false;
	if (GetRowText(iRow, &szText))
	{
		cMatcher.Init(mszFind.Text());
		bFound = cMatcher.Match(szText.Text(), &sMatch) || szText.ContainsIgnoreCase(mszFind.Text());
		cMatcher.Kill();
	}
	szText.Kill();
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
// The texts of the labels in pComponent, in order, joined.
//
//////////////////////////////////////////////////////////////////////////
static void AppendLabelTexts(Ptr<CComponent> pComponent, CChars* pszDest)
{
	CLabel*		pcLabel;
	CContainer*	pcContainer;
	size		i;

	pcLabel = dynamic_cast<CLabel*>(&pComponent);
	if (pcLabel)
	{
		pszDest->Append(pcLabel->GetText());
		return;
	}
	pcContainer = dynamic_cast<CContainer*>(&pComponent);
	if (pcContainer)
	{
		for (i = 0; i < pcContainer->NumComponents(); i++)
		{
			AppendLabelTexts(pcContainer->GetComponent(i), pszDest);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What iRow is found by: the text of the first of its components, by
// column, that has any.  False if none has.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::GetRowText(int32 iRow, CChars* pszDest)
{
	Ptr<CComponent>	pChild;
	CGridCell*		pcCell;
	CChars			szText;
	size			i;
	int32			iColumn;

	iColumn = -1;
	szText.Init();
	for (i = 0; i < maChildren.NumElements(); i++)
	{
		pChild = maChildren.Get(i);
		pcCell = GetCell(i);
		if ((pcCell->msPosition.y == iRow) && ((iColumn < 0) || (pcCell->msPosition.x < iColumn)) && (pChild != mpSelectedHighlight) && (pChild != mpHoverHighlight))
		{
			szText.Clear();
			AppendLabelTexts(pChild, &szText);
			if (!szText.Empty())
			{
				iColumn = pcCell->msPosition.x;
				pszDest->Clear();
				pszDest->Append(szText);
			}
		}
	}
	szText.Kill();
	return iColumn >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The scroll pane the list is shown by, NULL if it is in none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CScrollPane> CListContainer::FindScrollPane(void)
{
	Ptr<CComponent>	pParent;
	CScrollPane*	pcPane;

	for (pParent = GetParent(); pParent.IsNotNull(); pParent = pParent->GetParent())
	{
		pcPane = dynamic_cast<CScrollPane*>(&pParent);
		if (pcPane)
		{
			return Ptr<CScrollPane>(pcPane);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What has been typed is shown in the scroll pane's corner: in a box as a
// tool tip's, its border the theme's failure colour while no row matches.
// A list in no scroll pane shows nothing.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::ShowFindNote(void)
{
	Ptr<CScrollPane>	pPane;
	Ptr<CCanvas>		pBorder;
	Ptr<CCanvas>		pBackground;
	Ptr<CFillContainer>	pInside;
	Ptr<CFillContainer>	pText;
	SInt32Vec2			sSize;
	int32				iPaddingX;
	int32				iPaddingY;

	pPane = FindScrollPane();
	if (pPane.IsNull())
	{
		return;
	}
	iPaddingX = mpTheme->GetMetric(TM_ToolTipPaddingX);
	iPaddingY = mpTheme->GetMetric(TM_ToolTipPaddingY);
	if (mpFindNote.IsNull())
	{
		mpFindBorder = OMalloc<CClearCanvasDraw>(mpTheme->GetColour(TC_ToolTipBorder));
		pBorder = OMalloc<CCanvas>(mpWindow, mpFindBorder);
		pBorder->SetAlignment(CSH_Stretch, CSV_Stretch);
		pBackground = OMalloc<CCanvas>(mpWindow, OMalloc<CClearCanvasDraw>(mpTheme->GetColour(TC_ToolTip)));
		pBackground->SetAlignment(CSH_Stretch, CSV_Stretch);
		pInside = OMalloc<CFillContainer>(mpWindow);
		pInside->SetAlignment(CSH_Stretch, CSV_Stretch);
		pInside->SetInsets(1, 1, 1, 1);
		pInside->AddComponent(pBackground);
		mpFindLabel = OMalloc<CLabel>(mpWindow, mpTheme->GetFont(), "");
		mpFindLabel->SetAlignment(CSH_Left, CSV_Center);
		pText = OMalloc<CFillContainer>(mpWindow);
		pText->SetAlignment(CSH_Stretch, CSV_Stretch);
		pText->SetInsets(iPaddingY + 1, iPaddingX + 1, iPaddingY + 1, iPaddingX + 1);
		pText->AddComponent(mpFindLabel);
		mpFindNote = OMalloc<CFillContainer>(mpWindow);
		mpFindNote->AddComponent(pBorder);
		mpFindNote->AddComponent(pInside);
		mpFindNote->AddComponent(pText);
	}
	mpFindBorder->SetColour(mpTheme->GetColour(mbFindMatched ? TC_ToolTipBorder : TC_Failure));
	mpFindLabel->SetText(mszFind.Text());
	sSize = mpFindLabel->GetRequiredSize();
	mpFindNote->SetDesiredSize(sSize.x + (iPaddingX + 1) * 2, sSize.y + (iPaddingY + 1) * 2);
	if (pPane->GetNote() != mpFindNote)
	{
		pPane->SetNote(mpFindNote);
	}
	mpWindow->InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing is being found any more: what was typed is forgotten and its
// note taken off the scroll pane.  The selection stays.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::StopFinding(void)
{
	Ptr<CScrollPane>	pPane;

	if (!IsFinding())
	{
		return;
	}
	mszFind.Clear();
	mbFindMatched = false;
	if (mpFindNote.IsNotNull())
	{
		pPane = FindScrollPane();
		if (pPane.IsNotNull() && (pPane->GetNote() == mpFindNote))
		{
			pPane->SetNote(NULL);
		}
	}
}


bool CListContainer::IsTypeToFind(void) const { return mbTypeToFind; }
bool CListContainer::IsFinding(void) const { return !mszFind.Empty(); }
bool CListContainer::IsFindMatched(void) const { return mbFindMatched; }
const char* CListContainer::GetFindText(void) { return mszFind.Text(); }
Ptr<CComponent> CListContainer::GetFindNote(void) const { return mpFindNote; }


//////////////////////////////////////////////////////////////////////////
//
// The enabled row a key moves the selection to from the selected row:
// CIC_Up, CIC_Down, CIC_Home, CIC_End, CIC_PageUp or CIC_PageDown.  -1 if
// there is none that way.  A page moves by the rows a scroll pane shows.
//
//////////////////////////////////////////////////////////////////////////
int32 CListContainer::FindMove(EComponentInputCommand eMove)
{
	int32	iTarget;
	int32	iRow;

	switch (eMove)
	{
	case CIC_Up:
		iRow = FindEnabled(miSelected < 0 ? NumRows() - 1 : miSelected - 1, -1);
		if ((iRow < 0) && mbWrapAround)
		{
			iRow = FindEnabled(NumRows() - 1, -1);
		}
		return iRow;
	case CIC_Down:
		iRow = FindEnabled(miSelected + 1, 1);
		if ((iRow < 0) && mbWrapAround)
		{
			iRow = FindEnabled(0, 1);
		}
		return iRow;
	case CIC_Home:
		return FindEnabled(0, 1);
	case CIC_End:
		return FindEnabled(NumRows() - 1, -1);
	case CIC_PageUp:
		iTarget = (miSelected < 0 ? NumRows() - 1 : miSelected) - GetPageRows();
		if (iTarget < 0)
		{
			iTarget = 0;
		}
		iRow = FindEnabled(iTarget, 1);
		if ((iRow < 0) || ((miSelected >= 0) && (iRow >= miSelected)))
		{
			iRow = FindEnabled(iTarget, -1);
		}
		return iRow;
	case CIC_PageDown:
		iTarget = (miSelected < 0 ? 0 : miSelected) + GetPageRows();
		if (iTarget > NumRows() - 1)
		{
			iTarget = NumRows() - 1;
		}
		iRow = FindEnabled(iTarget, -1);
		if (iRow <= miSelected)
		{
			iRow = FindEnabled(iTarget, 1);
		}
		return iRow;
	default:
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shift with a key that moves the selection: with several selectable, the
// rows from where the selection started to the row moved to are selected.
// Otherwise the key goes on to the parent.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SelectMove(SComponentInputEvent* psEvent)
{
	EComponentInputCommand	eMove;
	int32					iRow;

	if (!mbMultiSelect)
	{
		psEvent->bPass = true;
		return;
	}
	switch (psEvent->eCommand)
	{
	case CIC_SelectUp:				eMove = CIC_Up;			break;
	case CIC_SelectDown:			eMove = CIC_Down;		break;
	case CIC_SelectPageUp:			eMove = CIC_PageUp;		break;
	case CIC_SelectPageDown:		eMove = CIC_PageDown;	break;
	case CIC_SelectHome:
	case CIC_SelectDocumentHome:	eMove = CIC_Home;		break;
	default:						eMove = CIC_End;		break;
	}
	iRow = FindMove(eMove);
	if (iRow >= 0)
	{
		SelectRange((miAnchor >= 0) ? miAnchor : iRow, iRow);
		ShowRow(iRow);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shift or Control with a press: with several selectable, Control selects
// or unselects the row pressed and Shift selects from where the selection
// started to it.  Otherwise the press goes on to the parent.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SelectPress(SComponentInputEvent* psEvent)
{
	SInt32Vec2	sPosition;
	int32		iRow;

	if (!mbMultiSelect)
	{
		psEvent->bPass = true;
		return;
	}
	sPosition = GetPosition();
	iRow = FindRowAt(psEvent->x + sPosition.x, psEvent->y + sPosition.y);
	if (iRow < 0)
	{
		return;
	}
	if (psEvent->bControl && !psEvent->bShift)
	{
		ToggleRow(iRow);
	}
	else
	{
		SelectRange((miAnchor >= 0) ? miAnchor : iRow, iRow);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SelectAllAction(SComponentInputEvent* psEvent)
{
	if (!mbMultiSelect)
	{
		psEvent->bPass = true;
		return;
	}
	SelectAll();
}


//////////////////////////////////////////////////////////////////////////
//
// The row under the pointer is only marked if it isn't selected and can
// be.
//
//////////////////////////////////////////////////////////////////////////
bool CListContainer::IsHoverShown(void)
{
	return (miHover != miSelected) && IsRowEnabled(miHover);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::UpdateHighlights(void)
{
	lsize	uiRow;
	int32	iTop;
	int32	iHeight;

	if (mbMultiSelect)
	{
		//The selected highlight is over the whole list and draws a band for each selected row.
		mpSelectedBands->ClearBands();
		for (uiRow = mabSelected.FindSet(0); uiRow != ARRAY_LARGE_ELEMENT_NOT_FOUND; uiRow = mabSelected.FindSet(uiRow + 1))
		{
			if (GetRowBounds((int32)uiRow, &iTop, &iHeight))
			{
				mpSelectedBands->AddBand(iTop - GetPosition().y, iHeight);
			}
		}
		LayoutComponentInArea(mpSelectedHighlight, GetPosition(), GetActualSize());
		mpSelectedHighlight->Invalidate();
	}
	else
	{
		LayoutHighlight(mpSelectedHighlight, miSelected);
	}
	LayoutHighlight(mpHoverHighlight, IsHoverShown() ? miHover : -1);
}


//////////////////////////////////////////////////////////////////////////
//
// A scroll pane the list is in scrolls to show iRow, as laid out last.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::ShowRow(int32 iRow)
{
	int32		iTop;
	int32		iHeight;
	CInt32Rect	cRow;

	if ((iRow >= 0) && GetRowBounds(iRow, &iTop, &iHeight))
	{
		cRow.Init(GetPosition().x, iTop, GetPosition().x + GetActualSize().x, iTop + iHeight);
		ScrollToShow(&cRow);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The highlight covers the row from the left to the right of the list.
// A highlight that marks no row (iRow -1, or the list hasn't been laid
// out yet) has no size at the list's top left, wherever it was.  Laying
// the list out again then puts it where the row changes put it.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::LayoutHighlight(Ptr<CCanvas> pHighlight, int32 iRow)
{
	int32		iTop;
	int32		iHeight;
	SInt32Vec2	sPosition;
	SInt32Vec2	sSize;

	if ((iRow >= 0) && GetRowBounds(iRow, &iTop, &iHeight))
	{
		sPosition.Init(GetPosition().x, iTop);
		sSize.Init(GetActualSize().x, iHeight);
	}
	else
	{
		sPosition = GetPosition();
		sSize.Init(0, 0);
	}
	LayoutComponentInArea(pHighlight, sPosition, sSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The first enabled row from iFrom going by iStep (1 or -1), or -1 if
// there is none.
//
//////////////////////////////////////////////////////////////////////////
int32 CListContainer::FindEnabled(int32 iFrom, int32 iStep)
{
	int32	i;

	for (i = iFrom; (i >= 0) && (i < NumRows()); i += iStep)
	{
		if (IsRowEnabled(i))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Keyboard moves that find no row leave the selection as it is.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SelectEnabled(int32 iRow)
{
	if (iRow >= 0)
	{
		Select(iRow);
		ShowRow(iRow);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many rows page up and page down move.  Shown by a scroll pane, a
// page is the rows its view shows, less one so the row the selection
// moved from is still in sight; otherwise it is SetPageRows' (ten).
//
//////////////////////////////////////////////////////////////////////////
int32 CListContainer::GetPageRows(void)
{
	CInt32Rect	cView;
	int32		iTop;
	int32		iHeight;
	int32		iPage;

	if (GetScrollView(&cView) && GetRowBounds((miSelected < 0) ? 0 : miSelected, &iTop, &iHeight) && (iHeight > 0))
	{
		iPage = cView.GetHeight() / iHeight - 1;
		return (iPage > 1) ? iPage : 1;
	}
	return miPageRows;
}


//////////////////////////////////////////////////////////////////////////
//
// When the selection follows the pointer the row moved over is selected,
// if it can be.  Moving off the rows leaves it selected.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SetHover(int32 iRow)
{
	if (iRow != miHover)
	{
		miHover = iRow;
		if (mbSelectOnHover && (iRow >= 0))
		{
			Select(iRow);
		}
		UpdateHighlights();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::RowSelected(int32 iRow) {}
void CListContainer::RowActivated(int32 iRow) {}
int32 CListContainer::NumRows(void) const { return (int32)mabEnabled.NumElements(); }
int32 CListContainer::GetSelected(void) const { return miSelected; }
int32 CListContainer::GetHover(void) const { return miHover; }
void CListContainer::SetSelectOnHover(bool bSelectOnHover) { mbSelectOnHover = bSelectOnHover; }
bool CListContainer::IsSelectOnHover(void) const { return mbSelectOnHover; }
void CListContainer::SetDoubleClickActivates(bool bDoubleClickActivates) { mbDoubleClickActivates = bDoubleClickActivates; msDoubleClick.Forget(); }
bool CListContainer::IsDoubleClickActivates(void) const { return mbDoubleClickActivates; }
void CListContainer::SetPageRows(int32 iPageRows) { miPageRows = iPageRows < 1 ? 1 : iPageRows; LayoutChanged(); }  //A page when no scroll pane shows the list.
void CListContainer::SetWrapAround(bool bWrapAround) { mbWrapAround = bWrapAround; }  //Up from the first row goes to the last, and down from the last to the first.
Ptr<CCanvas> CListContainer::GetSelectedHighlight(void) const { return mpSelectedHighlight; }
Ptr<CCanvas> CListContainer::GetHoverHighlight(void) const { return mpHoverHighlight; }

