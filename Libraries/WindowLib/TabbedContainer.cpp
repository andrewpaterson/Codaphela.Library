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
#include "JewelImageGenerator.h"
#include "MenuContainer.h"
#include "MenuItem.h"
#include "TabbedContainer.h"


//////////////////////////////////////////////////////////////////////////
//
// The bar is the theme's tab bar colour with a separator line between it
// and the page.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	PreInit();

	CGridContainer::Init(pWindow);
	mpTheme = pTheme;
	maTabs.Init();
	maPages.Init();
	mpSelected = NULL;
	meTabSide = TS_Top;
	mbHideSingleTab = false;
	mbBarShown = false;
	muiShownTabs = MAX_SIZE;
	mcSelected.Init();
	mcRemoved.Init();
	mcDoubleClicked.Init();
	mcClicked.Init();
	mcDragged.Init();
	mcCloseRequested.Init();
	msDoubleClick.Init();
	mpOverflow = NULL;
	mpOverflowMenu = NULL;

	mpBarBackground = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_TabBar)));
	mpBarBackground->SetAlignment(CSH_Stretch, CSV_Stretch);

	mpBar = OMalloc<CGridContainer>(pWindow);
	SetBarFit(false);
	mpFiller = OMalloc<CComponent>(pWindow);
	mpFiller->SetDesiredSize(0, 0);

	mpSeparator = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_TabSeparator)));
	mpSeparator->SetDesiredSize(-1, 1);
	mpSeparator->SetAlignment(CSH_Stretch, CSV_Stretch);

	mpPage = OMalloc<CFillContainer>(pWindow);
	mpPage->SetAlignment(CSH_Stretch, CSV_Stretch);

	PlaceTabs();
	PlaceRows();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpTheme);
	M_Pointer(mpBar);
	M_Pointer(mpFiller);
	M_Pointer(mpBarBackground);
	M_Pointer(mpSeparator);
	M_Pointer(mpOverflow);
	M_Pointer(mpOverflowMenu);
	M_Pointer(mpPage);
	M_Embedded(maTabs);
	M_Embedded(maPages);
	M_Pointer(mpSelected);
	U_Enum(meTabSide);
	U_Bool(mbHideSingleTab);
	U_Bool(mbBarShown);
	U_Size(muiShownTabs);
	U_Unknown(CComponentEvent<CTabComponent>, mcSelected);
	U_Unknown(CTabRemovedEvent, mcRemoved);
	U_Unknown(CComponentEvent<CTabComponent>, mcDoubleClicked);
	U_Unknown(CComponentEvent<CTabComponent>, mcClicked);
	U_Unknown(CComponentEvent<CTabComponent>, mcDragged);
	U_Unknown(CComponentEvent<CTabComponent>, mcCloseRequested);
	U_Data(SDoubleClick, msDoubleClick);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::Free(void)
{
	if (mpOverflowMenu.IsNotNull())
	{
		mpOverflowMenu->GetItemClickEvent()->Remove(this);
	}
	mcCloseRequested.Kill();
	mcDragged.Kill();
	mcClicked.Kill();
	mcDoubleClicked.Kill();
	mcRemoved.Kill();
	mcSelected.Kill();
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTabbedContainer::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTabbedContainer::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The bar (over its background), the separator and the page, in rows
// from the top or the bottom.  The bar and separator are left out while
// they are hidden.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::PlaceRows(void)
{
	bool	bShowBar;
	int		iBarRow;
	int		iSeparatorRow;
	int		iPageRow;

	bShowBar = !mbHideSingleTab || (maTabs.NumElements() > 1);
	if ((NumComponents() > 0) && (bShowBar == mbBarShown))
	{
		return;
	}
	mbBarShown = bShowBar;

	RemoveAllComponents();
	if (!bShowBar)
	{
		AddComponent(mpPage, 0, 0, 1, 1, 1, 1);
		InvalidateLayout();
		return;
	}

	if (meTabSide == TS_Top)
	{
		iBarRow = 0;
		iSeparatorRow = 1;
		iPageRow = 2;
	}
	else
	{
		iPageRow = 0;
		iSeparatorRow = 1;
		iBarRow = 2;
	}
	//The background is in the bar's cell first so the bar is drawn over it.
	AddComponent(mpBarBackground, 0, iBarRow, 1, 1, 1, 0);
	AddComponent(mpBar, 0, iBarRow, 1, 1, 1, 0);
	AddComponent(mpSeparator, 0, iSeparatorRow, 1, 1, 1, 0);
	AddComponent(mpPage, 0, iPageRow, 1, 1, 1, 1);
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The tabs in order along the bar, then the filler taking the spare width
// and the overflow chevron at the right.  A grid is variable sized so the
// bar is told how tall its tallest tab is, or its row would get no height.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::PlaceTabs(void)
{
	size	i;
	int32	iHeight;
	int32	iTabHeight;

	size	uiShown;

	mpBar->RemoveAllComponents();
	iHeight = 0;
	uiShown = (muiShownTabs < maTabs.NumElements()) ? muiShownTabs : maTabs.NumElements();
	for (i = 0; i < maTabs.NumElements(); i++)
	{
		if (i < uiShown)
		{
			mpBar->AddComponent(maTabs.Get(i), (int)i, 0);
		}
		iTabHeight = maTabs.Get(i)->GetDesiredSize().y;
		if (iTabHeight > iHeight)
		{
			iHeight = iTabHeight;
		}
	}
	mpBar->AddComponent(mpFiller, (int)uiShown, 0, 1, 1, 1, 0);
	if (mpOverflow.IsNotNull())
	{
		mpBar->AddComponent(mpOverflow, (int)uiShown + 1, 0, 1, 1, 0, 0);
	}
	if (iHeight > mpBar->GetDesiredSize().y)
	{
		mpBar->SetDesiredSize(-1, iHeight);
	}
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The tab goes after the others on the bar, with pIcon (which can be
// NULL) on its left and, if bClosable, a cross on its right.  The page
// should stretch to fill the area under the bar.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTabComponent> CTabbedContainer::AddTab(const char* szName, Ptr<CComponent> pPage, Ptr<CImageCel> pIcon, bool bClosable)
{
	return InsertTab(maTabs.NumElements(), szName, pPage, pIcon, bClosable);
}


//////////////////////////////////////////////////////////////////////////
//
// As AddTab but the tab goes at uiIndex on the bar, or after the others
// if uiIndex is past them.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTabComponent> CTabbedContainer::InsertTab(size uiIndex, const char* szName, Ptr<CComponent> pPage, Ptr<CImageCel> pIcon, bool bClosable)
{
	Ptr<CTabComponent>	pTab;

	pTab = OMalloc<CTabComponent>(mpWindow, mpTheme, Ptr<CTabbedContainer>(this), szName, pIcon, bClosable);
	if (uiIndex > maTabs.NumElements())
	{
		uiIndex = maTabs.NumElements();
	}
	//The tabs after it move up an index.
	msDoubleClick.Forget();
	maTabs.Insert(uiIndex, pTab);
	maPages.Insert(uiIndex, pPage);
	PlaceTabs();
	PlaceRows();
	if (mpSelected.IsNull())
	{
		Select(pTab);
	}
	return pTab;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the tab at uiIndex and its page off the bar and returns the page.
// The selected tab's page comes out of the page area.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CTabbedContainer::RemoveTabOnly(size uiIndex, bool* pbWasSelected)
{
	Ptr<CTabComponent>	pTab;
	Ptr<CComponent>		pPage;

	pTab = maTabs.Get(uiIndex);

	//The tabs after it move down an index.
	msDoubleClick.Forget();
	*pbWasSelected = (mpSelected == pTab);
	if (*pbWasSelected)
	{
		mpPage->RemoveAllComponents();
		mpSelected = NULL;
	}

	pPage = maPages.Get(uiIndex);
	mpBar->RemoveComponent(pTab);
	maTabs.RemoveAt(uiIndex);
	maPages.RemoveAt(uiIndex);
	PlaceTabs();
	PlaceRows();
	return pPage;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the tab and its page and calls the removal event.  If it was
// selected the tab after it, or before it if it was the last, is selected
// next.  Returns false if the tab isn't this container's.
//
//////////////////////////////////////////////////////////////////////////
bool CTabbedContainer::RemoveTab(Ptr<CTabComponent> pTab)
{
	size			uiIndex;
	bool			bWasSelected;
	Ptr<CComponent>	pPage;

	uiIndex = IndexOf(pTab);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	pPage = RemoveTabOnly(uiIndex, &bWasSelected);

	//The page is held while the removal event is called.
	mcRemoved.Call(&pTab, &pPage);
	if (bWasSelected && (maTabs.NumElements() > 0) && mpSelected.IsNull())
	{
		Select(uiIndex < maTabs.NumElements() ? uiIndex : maTabs.NumElements() - 1);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the tab off the bar and returns its page, without calling the
// removal event: the page is going somewhere else, another tabbed
// container say.  The next tab is selected as RemoveTab selects it.
// Returns NULL if the tab isn't this container's.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CTabbedContainer::TakeTab(Ptr<CTabComponent> pTab)
{
	size			uiIndex;
	bool			bWasSelected;
	Ptr<CComponent>	pPage;

	uiIndex = IndexOf(pTab);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return NULL;
	}
	pPage = RemoveTabOnly(uiIndex, &bWasSelected);
	if (bWasSelected && (maTabs.NumElements() > 0))
	{
		Select(uiIndex < maTabs.NumElements() ? uiIndex : maTabs.NumElements() - 1);
	}
	return pPage;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the tab, with its page, to uiIndex on the bar.
//
//////////////////////////////////////////////////////////////////////////
bool CTabbedContainer::MoveTab(Ptr<CTabComponent> pTab, size uiIndex)
{
	size			uiOld;
	Ptr<CComponent>	pPage;

	uiOld = IndexOf(pTab);
	if (uiOld == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	if (uiIndex >= maTabs.NumElements())
	{
		uiIndex = maTabs.NumElements() - 1;
	}
	if (uiIndex == uiOld)
	{
		return true;
	}
	msDoubleClick.Forget();
	pPage = maPages.Get(uiOld);
	maTabs.RemoveAt(uiOld);
	maPages.RemoveAt(uiOld);
	maTabs.Insert(uiIndex, pTab);
	maPages.Insert(uiIndex, pPage);
	PlaceTabs();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the tab's page in place of the selected one's.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::Select(Ptr<CTabComponent> pTab)
{
	size	uiIndex;

	if (mpSelected == pTab)
	{
		return;
	}

	uiIndex = IndexOf(pTab);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		Select(uiIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The selection event is called when the selected tab changes.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::Select(size uiIndex)
{
	Ptr<CTabComponent>	pTab;

	if (uiIndex >= maTabs.NumElements())
	{
		return;
	}

	pTab = maTabs.Get(uiIndex);
	if (mpSelected == pTab)
	{
		return;
	}

	if (mpSelected)
	{
		mpSelected->SetSelected(false);
	}
	mpPage->RemoveAllComponents();
	mpSelected = pTab;
	mpSelected->SetSelected(true);
	mpPage->AddComponent(maPages.Get(uiIndex));
	InvalidateLayout();
	mcSelected.Call(&pTab);
}


//////////////////////////////////////////////////////////////////////////
//
// Visual Studio's tool windows have their tabs along the bottom.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::SetTabSide(ETabSide eSide)
{
	if (meTabSide != eSide)
	{
		meTabSide = eSide;
		RemoveAllComponents();
		PlaceRows();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// With one tab or none the bar and its separator aren't shown, the page
// takes the whole container, as a tool window on its own has no tabs.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::SetHideSingleTab(bool bHide)
{
	if (mbHideSingleTab != bHide)
	{
		mbHideSingleTab = bHide;
		RemoveAllComponents();
		PlaceRows();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A chevron at the bar's right opens a list of every tab.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::SetOverflowMenu(bool bOverflow)
{
	CColourFormatHelper	cFormat;

	if (bOverflow == mpOverflow.IsNotNull())
	{
		return;
	}
	if (bOverflow)
	{
		mpWindow->GetColourFormat(&cFormat);
		mpOverflow = OMalloc<CIconComponent>(mpWindow, CJewelImageGenerator::CreateDownChevron(mpTheme->GetColour(TC_Caret), &cFormat), 24, -1);
		mpOverflow->SetCanGetFocus(true);
		mpOverflow->AddInputAction(CIC_ActivateComponent, this, &CTabbedContainer::OverflowPressed);
		mpOverflowMenu = OMalloc<CMenuContainer>(mpWindow, mpTheme);
		mpOverflowMenu->GetItemClickEvent()->Add(this, &CTabbedContainer::OverflowItemClicked);
	}
	else
	{
		mpOverflowMenu->GetItemClickEvent()->Remove(this);
		mpOverflow = NULL;
		mpOverflowMenu = NULL;
	}
	SetBarFit(bOverflow);
	PlaceTabs();
}


//////////////////////////////////////////////////////////////////////////
//
// With the overflow menu the bar stretches across the container and only
// the tabs that fit are on it (see Layout).  Without it the bar is as wide
// as its tabs, from the left, and a container too narrow for them cuts the
// last ones off: it clips them, so they are neither drawn nor hit past its
// edge.  A grid bar stretched across too little space would squeeze every
// tab's column and the tabs, centred in them, would overlap.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::SetBarFit(bool bStretch)
{
	if (bStretch)
	{
		mpBar->SetSizedByContent(false);
		mpBar->SetAlignment(CSH_Stretch, CSV_Top);
	}
	else
	{
		mpBar->SetSizedByContent(true);
		mpBar->SetAlignment(CSH_Left, CSV_Top);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::OverflowPressed(int x, int y)
{
	OpenOverflowMenu();
}


//////////////////////////////////////////////////////////////////////////
//
// Lists the tabs, in order, under the chevron.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::OpenOverflowMenu(void)
{
	size				i;
	SInt32Vec2			sPosition;
	Ptr<CIconComponent>	pIcon;
	Ptr<CImageCel>		pCel;

	if (mpOverflowMenu.IsNull() || (maTabs.NumElements() == 0))
	{
		return;
	}
	mpOverflowMenu->RemoveAllItems();
	for (i = 0; i < maTabs.NumElements(); i++)
	{
		pIcon = maTabs.Get(i)->GetIcon();
		pCel = pIcon.IsNotNull() ? pIcon->GetCel() : Ptr<CImageCel>();
		mpOverflowMenu->AddItem(maTabs.Get(i)->GetText(), pCel);
	}
	sPosition = mpOverflow->GetPosition();
	sPosition.y += mpOverflow->GetActualSize().y;
	mpOverflowMenu->Open(sPosition, mpOverflow, true);
}


//////////////////////////////////////////////////////////////////////////
//
// The tab chosen from the overflow menu moves to the front, so it shows,
// and is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::OverflowItemClicked(CMenuItem* pcItem)
{
	size				i;
	Ptr<CTabComponent>	pTab;

	for (i = 0; i < mpOverflowMenu->NumItems(); i++)
	{
		if (&mpOverflowMenu->GetItem(i) == pcItem)
		{
			if (i < maTabs.NumElements())
			{
				pTab = maTabs.Get(i);
				MoveTab(pTab, 0);
				Select(pTab);
			}
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The tab whose bounds hold x, y in window client space, NULL if none
// does.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTabComponent> CTabbedContainer::FindTabAt(int x, int y)
{
	size				i;
	Ptr<CTabComponent>	pTab;

	if (!mbBarShown)
	{
		return NULL;
	}
	for (i = 0; i < maTabs.NumElements(); i++)
	{
		pTab = maTabs.Get(i);
		if ((i < NumShownTabs()) && pTab->IsPointIn(x, y))
		{
			return pTab;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTabbedContainer::IndexOf(Ptr<CTabComponent> pTab)
{
	size	i;
	size	uiNumTabs;

	uiNumTabs = maTabs.NumElements();
	for (i = 0; i < uiNumTabs; i++)
	{
		if (maTabs.Get(i) == pTab)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTabbedContainer::IndexOfPage(Ptr<CComponent> pPage)
{
	size	i;
	size	uiNumTabs;

	uiNumTabs = maPages.NumElements();
	for (i = 0; i < uiNumTabs; i++)
	{
		if (maPages.Get(i) == pPage)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTabComponent> CTabbedContainer::GetSelected(void) { return mpSelected; }
size CTabbedContainer::NumTabs(void) { return maTabs.NumElements(); }
Ptr<CTabComponent> CTabbedContainer::GetTab(size uiIndex) { return maTabs.Get(uiIndex); }
Ptr<CComponent> CTabbedContainer::GetPage(size uiIndex) { return maPages.Get(uiIndex); }
ETabSide CTabbedContainer::GetTabSide(void) { return meTabSide; }
bool CTabbedContainer::IsBarShown(void) { return mbBarShown; }
Ptr<CMenuContainer> CTabbedContainer::GetOverflowMenu(void) { return mpOverflowMenu; }


//////////////////////////////////////////////////////////////////////////
//
// Called with the tab when another tab is selected:
// pTabbed->GetSelectionEvent()->Add(this, &CMyContent::TabSelected).
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CTabComponent>* CTabbedContainer::GetSelectionEvent(void)
{
	return &mcSelected;
}


//////////////////////////////////////////////////////////////////////////
//
// Called with the tab, and its page if a function takes it, after a tab
// is removed.
//
//////////////////////////////////////////////////////////////////////////
CTabRemovedEvent* CTabbedContainer::GetRemovalEvent(void)
{
	return &mcRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
// Called with the tab when it is double clicked.
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CTabComponent>* CTabbedContainer::GetDoubleClickEvent(void)
{
	return &mcDoubleClicked;
}


//////////////////////////////////////////////////////////////////////////
//
// A tab pressed and let go without being dragged, whether or not it was
// selected already: a document's tab gives its editor the keys.  The tab
// keeps them while it's pressed, so it can be dragged.
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CTabComponent>* CTabbedContainer::GetClickEvent(void)
{
	return &mcClicked;
}


//////////////////////////////////////////////////////////////////////////
//
// Called with the tab when it is pulled along while pressed.  Tabs can
// only be dragged while something listens.
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CTabComponent>* CTabbedContainer::GetDragEvent(void)
{
	return &mcDragged;
}


//////////////////////////////////////////////////////////////////////////
//
// Called with the tab when its cross is pressed, instead of the tab being
// removed, while anything listens.
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CTabComponent>* CTabbedContainer::GetCloseRequestEvent(void)
{
	return &mcCloseRequested;
}


//////////////////////////////////////////////////////////////////////////
//
// Called by a tab whose cross was pressed.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::TabClosePressed(Ptr<CTabComponent> pTab)
{
	msDoubleClick.Forget();
	if (mcCloseRequested.NumCalls() > 0)
	{
		mcCloseRequested.Call(&pTab);
	}
	else
	{
		RemoveTab(pTab);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTabbedContainer::IsTabDraggable(void)
{
	return mcDragged.NumCalls() > 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Called by a tab when button one is released on it, x and y relative to
// its top left; bClick is false on its cross.  A double click (see
// SDoubleClick) calls the double click event.  It is called after the
// release so a dialog it opens doesn't take it.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::TabReleased(Ptr<CTabComponent> pTab, int x, int y, bool bClick)
{
	size	uiIndex;

	uiIndex = IndexOf(pTab);
	if (!bClick || (uiIndex == ARRAY_ELEMENT_NOT_FOUND))
	{
		msDoubleClick.Forget();
		return;
	}

	mcClicked.Call(&pTab);
	if (msDoubleClick.Released((int32)uiIndex, x, y))
	{
		mcDoubleClicked.Call(&pTab);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Called by a tab pulled a few pixels while pressed.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::TabDragged(Ptr<CTabComponent> pTab)
{
	msDoubleClick.Forget();
	mcDragged.Call(&pTab);
}


//////////////////////////////////////////////////////////////////////////
//
// With the overflow menu only the tabs that fit on the bar are on it; the
// selected tab is moved to the front if it wouldn't fit.  Without it every
// tab is on the bar however narrow it is.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	size	i;
	size	uiShown;
	int32	iAvailable;
	int32	iUsed;
	size	uiSelected;

	if (mpOverflow.IsNotNull() && mbBarShown)
	{
		iAvailable = sAreaSize.x - mpOverflow->GetDesiredSize().x;
		uiShown = 0;
		iUsed = 0;
		for (i = 0; i < maTabs.NumElements(); i++)
		{
			iUsed += maTabs.Get(i)->GetDesiredSize().x;
			if (iUsed > iAvailable)
			{
				break;
			}
			uiShown++;
		}
		if ((uiShown == 0) && (maTabs.NumElements() > 0))
		{
			uiShown = 1;
		}
		uiSelected = mpSelected.IsNotNull() ? IndexOf(mpSelected) : 0;
		if ((uiSelected != ARRAY_ELEMENT_NOT_FOUND) && (uiSelected >= uiShown))
		{
			MoveTab(mpSelected, 0);
		}
		if (uiShown != muiShownTabs)
		{
			muiShownTabs = uiShown;
			PlaceTabs();
		}
	}
	else
	{
		if (muiShownTabs != MAX_SIZE)
		{
			muiShownTabs = MAX_SIZE;
			PlaceTabs();
		}
		//The tabs past the edge are cut off; a selected one among them is moved to the front so it shows.
		if (mbBarShown && mpSelected.IsNotNull())
		{
			uiSelected = IndexOf(mpSelected);
			iUsed = 0;
			for (i = 0; (uiSelected != ARRAY_ELEMENT_NOT_FOUND) && (i <= uiSelected); i++)
			{
				iUsed += maTabs.Get(i)->GetDesiredSize().x;
			}
			if ((uiSelected != ARRAY_ELEMENT_NOT_FOUND) && (uiSelected > 0) && (iUsed > sAreaSize.x))
			{
				MoveTab(mpSelected, 0);
			}
		}
	}
	CGridContainer::Layout(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The tab is on the bar, not only in the overflow menu.
//
//////////////////////////////////////////////////////////////////////////
bool CTabbedContainer::IsTabShown(Ptr<CTabComponent> pTab)
{
	size	uiIndex;

	uiIndex = IndexOf(pTab);
	return mbBarShown && (uiIndex != ARRAY_ELEMENT_NOT_FOUND) && (uiIndex < NumShownTabs());
}


size CTabbedContainer::NumShownTabs(void) { return (muiShownTabs < maTabs.NumElements()) ? muiShownTabs : maTabs.NumElements(); }

