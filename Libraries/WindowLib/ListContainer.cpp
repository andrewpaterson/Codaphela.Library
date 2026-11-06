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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "Window.h"
#include "ListContainer.h"


//////////////////////////////////////////////////////////////////////////
//
// The highlights are the Jewel dark selection and hover colours.  Page up
// and page down move ten rows.
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
	mbPressed = false;
	mbSelectOnHover = false;
	mbDoubleClickActivates = false;
	msDoubleClick.Init();
	SetCanGetFocus(true);

	mpSelectedDraw = OMalloc<CClearCanvasDraw>(pTheme->GetListSelectedColour());
	mpSelectedHighlight = CreateHighlight(mpSelectedDraw);
	mpHoverDraw = OMalloc<CClearCanvasDraw>(pTheme->GetListHoverColour());
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
	AddInputAction(CIC_Space, this, &CListContainer::Activate);
	AddInputAction(CIC_Enter, this, &CListContainer::Activate);

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
	U_Data(CArrayBit, mabEnabled);
	U_Data(CComponentEvent<CListContainer>, mcSelectionChanged);
	U_Data(CComponentEvent<CListContainer>, mcActivated);
	U_Int32(miSelected);
	U_Int32(miHover);
	U_Int32(miPageRows);
	U_Bool(mbPressed);
	U_Bool(mbSelectOnHover);
	U_Bool(mbDoubleClickActivates);
	U_Data(SDoubleClick, msDoubleClick);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Free(void)
{
	mcActivated.Kill();
	mcSelectionChanged.Kill();
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

	if (iRow != miSelected)
	{
		miSelected = iRow;
		UpdateHighlights();
		RowSelected(iRow);
		mcSelectionChanged.Call(this);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Activate(void)
{
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
	uint32	i;
	int32	iTop;
	int32	iHeight;

	if (!IsPointIn(x, y))
	{
		return -1;
	}

	for (i = 0; i < (uint32)NumRows(); i++)
	{
		if (GetTrackBounds(false, (size)i, &iTop, &iHeight))
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
// Like any fill colour they are red, green and blue whatever the
// window's colour order; the native canvas orders them.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::SetHighlightColours(ARGB32 sSelected, ARGB32 sHover)
{
	mpSelectedDraw->SetColour(sSelected);
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
bool CListContainer::Draw(void)
{
	size			i;
	size			uiSize;
	Ptr<CComponent>	pComponent;
	bool			bShown;

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		if (pComponent == mpSelectedHighlight)
		{
			bShown = miSelected >= 0;
		}
		else if (pComponent == mpHoverHighlight)
		{
			bShown = IsHoverShown();
		}
		else
		{
			bShown = true;
		}

		if (bShown)
		{
			if (!pComponent->Draw())
			{
				return false;
			}
		}
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
// Losing focus, as when the window is deactivated, ends a press.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::FocusLost(void)
{
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
	if ((miSelected >= 0) && GetTrackBounds(false, (size)miSelected, &iTop, &iHeight))
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
void CListContainer::Up(void)
{
	SelectEnabled(FindEnabled(miSelected < 0 ? NumRows() - 1 : miSelected - 1, -1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Down(void)
{
	SelectEnabled(FindEnabled(miSelected + 1, 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::Home(void)
{
	SelectEnabled(FindEnabled(0, 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::End(void)
{
	SelectEnabled(FindEnabled(NumRows() - 1, -1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::DocumentHome(void)
{
	Home();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::DocumentEnd(void)
{
	End();
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the page rows up to the nearest enabled row at or below there, or
// above there if none is between there and the selected row.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::PageUp(void)
{
	int32	iTarget;
	int32	iRow;

	iTarget = (miSelected < 0 ? NumRows() - 1 : miSelected) - miPageRows;
	if (iTarget < 0)
	{
		iTarget = 0;
	}

	iRow = FindEnabled(iTarget, 1);
	if ((iRow < 0) || ((miSelected >= 0) && (iRow >= miSelected)))
	{
		iRow = FindEnabled(iTarget, -1);
	}
	SelectEnabled(iRow);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the page rows down to the nearest enabled row at or above there,
// or below there if none is between the selected row and there.
//
//////////////////////////////////////////////////////////////////////////
void CListContainer::PageDown(void)
{
	int32	iTarget;
	int32	iRow;

	iTarget = (miSelected < 0 ? 0 : miSelected) + miPageRows;
	if (iTarget > NumRows() - 1)
	{
		iTarget = NumRows() - 1;
	}

	iRow = FindEnabled(iTarget, -1);
	if (iRow <= miSelected)
	{
		iRow = FindEnabled(iTarget, 1);
	}
	SelectEnabled(iRow);
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
	LayoutHighlight(mpSelectedHighlight, miSelected);
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

	if ((iRow >= 0) && GetTrackBounds(false, (size)iRow, &iTop, &iHeight))
	{
		cRow.Init(GetPosition().x, iTop, GetPosition().x + GetActualSize().x - 1, iTop + iHeight - 1);
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

	if ((iRow >= 0) && GetTrackBounds(false, (size)iRow, &iTop, &iHeight))
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
	}
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
int32 CListContainer::NumRows(void) { return (int32)mabEnabled.NumElements(); }
int32 CListContainer::GetSelected(void) { return miSelected; }
int32 CListContainer::GetHover(void) { return miHover; }
void CListContainer::SetSelectOnHover(bool bSelectOnHover) { mbSelectOnHover = bSelectOnHover; }
bool CListContainer::IsSelectOnHover(void) { return mbSelectOnHover; }
void CListContainer::SetDoubleClickActivates(bool bDoubleClickActivates) { mbDoubleClickActivates = bDoubleClickActivates; msDoubleClick.Forget(); }
bool CListContainer::IsDoubleClickActivates(void) { return mbDoubleClickActivates; }
void CListContainer::SetPageRows(int32 iPageRows) { miPageRows = iPageRows < 1 ? 1 : iPageRows; InvalidateLayout(); }
Ptr<CCanvas> CListContainer::GetSelectedHighlight(void) { return mpSelectedHighlight; }
Ptr<CCanvas> CListContainer::GetHoverHighlight(void) { return mpHoverHighlight; }

