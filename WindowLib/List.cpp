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
#include "List.h"


//////////////////////////////////////////////////////////////////////////
//
// The highlights are the Jewel dark selection and hover colours.  Page up
// and page down move ten rows.
//
//////////////////////////////////////////////////////////////////////////
void CList::Init(Ptr<CWindow> pWindow)
{
	PreInit();

	CGridContainer::Init(pWindow);
	mabEnabled.Init();
	mcListeners.Init();
	mcListeners.AddAllowedClass<CListListener>();
	miSelected = -1;
	miHover = -1;
	miPageRows = 10;
	mbPressed = false;
	mbSelectOnHover = false;
	SetCanGetFocus(true);

	mpSelectedDraw = OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x2E, (uint8)0x43, (uint8)0x6E));
	mpSelectedHighlight = CreateHighlight(mpSelectedDraw);
	mpHoverDraw = OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x39, (uint8)0x3B, (uint8)0x40));
	mpHoverHighlight = CreateHighlight(mpHoverDraw);
	AddHighlights();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpSelectedDraw);
	M_Pointer(mpSelectedHighlight);
	M_Pointer(mpHoverDraw);
	M_Pointer(mpHoverHighlight);
	U_Data(CArrayBit, mabEnabled);
	U_Data(CListenerCall, mcListeners);
	U_Int32(miSelected);
	U_Int32(miHover);
	U_Int32(miPageRows);
	U_Bool(mbPressed);
	U_Bool(mbSelectOnHover);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::Free(void)
{
	mcListeners.Kill();
	mabEnabled.Kill();
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CList::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CList::Load(CObjectReader* pcFile)
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
void CList::AddHighlights(void)
{
	CGridContainer::AddComponent(mpSelectedHighlight, 0, 0);
	CGridContainer::AddComponent(mpHoverHighlight, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvas> CList::CreateHighlight(Ptr<CClearCanvasDraw> pDraw)
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
int32 CList::AddRow(bool bEnabled)
{
	mabEnabled.Add(bEnabled);
	return NumRows() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// A column with a width weight takes the width the list has spare.
//
//////////////////////////////////////////////////////////////////////////
bool CList::AddToRow(int32 iRow, Ptr<CComponent> pComponent, int iColumn, int iColumnSpan, int iWidthWeight)
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
int32 CList::AddItem(Ptr<CComponent> pItem, bool bEnabled)
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
bool CList::RemoveAllComponents(void)
{
	bool	bResult;

	Select(-1);
	miHover = -1;
	mbPressed = false;
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
void CList::SetRowEnabled(int32 iRow, bool bEnabled)
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
bool CList::IsRowEnabled(int32 iRow)
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
bool CList::Select(int32 iRow)
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
		mcListeners.CallListeners(&CListListener::SelectionChanged, this, NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::Activate(void)
{
	if (miSelected >= 0)
	{
		mcListeners.CallListeners(&CListListener::ItemActivated, this, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are in window client space.  Returns -1 if they aren't over a
// row, as when the list is taller than its rows.
//
//////////////////////////////////////////////////////////////////////////
int32 CList::FindRowAt(int x, int y)
{
	int32	i;
	int32	iTop;
	int32	iHeight;

	if (!IsPointIn(x, y))
	{
		return -1;
	}

	for (i = 0; i < NumRows(); i++)
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
void CList::SetHighlightColours(ARGB32 sSelected, ARGB32 sHover)
{
	mpSelectedDraw->SetColour(sSelected);
	mpHoverDraw->SetColour(sHover);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CList::AddListener(CListListener* pcListener)
{
	return mcListeners.AddListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CList::RemoveListener(CListListener* pcListener)
{
	return mcListeners.RemoveListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
// Like a button a list keeps the size its rows need unless a desired size
// is set on an axis.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CList::GetDesiredSize(void)
{
	return GetDesiredOrRequiredSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
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
bool CList::Draw(void)
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
// x and y are in window client space.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CList::FindComponentAt(int x, int y)
{
	if (IsPointIn(x, y))
	{
		return this;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Enter activates the selected row rather than clicking the default
// button.  With nothing selected the default button gets it.
//
//////////////////////////////////////////////////////////////////////////
bool CList::WantsEnter(void)
{
	return miSelected >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Losing focus, as when the window is deactivated, ends a press.
//
//////////////////////////////////////////////////////////////////////////
void CList::FocusLost(void)
{
	mbPressed = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::MovedOutof(void)
{
	SetHover(-1);
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the list's top left.
//
//////////////////////////////////////////////////////////////////////////
void CList::MovedOver(int x, int y)
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
void CList::ActivateComponent(int x, int y)
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
// The release goes to the list with focus wherever the pointer is.  It
// only activates if the pointer is still over the selected row.
//
//////////////////////////////////////////////////////////////////////////
void CList::ExecuteComponent(int x, int y)
{
	if (mbPressed)
	{
		mbPressed = false;
		if ((miHover >= 0) && (miHover == miSelected))
		{
			Activate();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// With nothing selected up selects the last enabled row and down the
// first.
//
//////////////////////////////////////////////////////////////////////////
void CList::Up(void)
{
	SelectEnabled(FindEnabled(miSelected < 0 ? NumRows() - 1 : miSelected - 1, -1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::Down(void)
{
	SelectEnabled(FindEnabled(miSelected + 1, 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::Home(void)
{
	SelectEnabled(FindEnabled(0, 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::End(void)
{
	SelectEnabled(FindEnabled(NumRows() - 1, -1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::DocumentHome(void)
{
	Home();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::DocumentEnd(void)
{
	End();
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the page rows up to the nearest enabled row at or below there, or
// above there if none is between there and the selected row.
//
//////////////////////////////////////////////////////////////////////////
void CList::PageUp(void)
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
void CList::PageDown(void)
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
//
//////////////////////////////////////////////////////////////////////////
void CList::Space(bool bInsert)
{
	Activate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::Enter(void)
{
	Activate();
}


//////////////////////////////////////////////////////////////////////////
//
// The row under the pointer is only marked if it isn't selected and can
// be.
//
//////////////////////////////////////////////////////////////////////////
bool CList::IsHoverShown(void)
{
	return (miHover != miSelected) && IsRowEnabled(miHover);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CList::UpdateHighlights(void)
{
	LayoutHighlight(mpSelectedHighlight, miSelected);
	if (IsHoverShown())
	{
		LayoutHighlight(mpHoverHighlight, miHover);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The highlight covers the row from the left to the right of the list.
// Before the list has been laid out there is no row to cover.
//
//////////////////////////////////////////////////////////////////////////
void CList::LayoutHighlight(Ptr<CCanvas> pHighlight, int32 iRow)
{
	int32		iTop;
	int32		iHeight;
	SInt32Vec2	sPosition;
	SInt32Vec2	sSize;

	if (iRow < 0)
	{
		return;
	}
	if (!GetTrackBounds(false, (size)iRow, &iTop, &iHeight))
	{
		return;
	}

	sPosition.Init(GetPosition().x, iTop);
	sSize.Init(GetActualSize().x, iHeight);
	LayoutComponentInArea(pHighlight, sPosition, sSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The first enabled row from iFrom going by iStep (1 or -1), or -1 if
// there is none.
//
//////////////////////////////////////////////////////////////////////////
int32 CList::FindEnabled(int32 iFrom, int32 iStep)
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
void CList::SelectEnabled(int32 iRow)
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
void CList::SetHover(int32 iRow)
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
int32 CList::NumRows(void) { return (int32)mabEnabled.NumElements(); }
int32 CList::GetSelected(void) { return miSelected; }
int32 CList::GetHover(void) { return miHover; }
void CList::SetSelectOnHover(bool bSelectOnHover) { mbSelectOnHover = bSelectOnHover; }
bool CList::IsSelectOnHover(void) { return mbSelectOnHover; }
void CList::SetPageRows(int32 iPageRows) { miPageRows = iPageRows < 1 ? 1 : iPageRows; }
Ptr<CCanvas> CList::GetSelectedHighlight(void) { return mpSelectedHighlight; }
Ptr<CCanvas> CList::GetHoverHighlight(void) { return mpHoverHighlight; }

