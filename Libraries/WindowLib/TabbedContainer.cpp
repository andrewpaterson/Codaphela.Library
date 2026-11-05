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
#include "TabbedContainer.h"


//////////////////////////////////////////////////////////////////////////
//
// The bar is the theme's tab bar colour with a separator line under it.
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	Ptr<CCanvas>	pBackground;
	Ptr<CCanvas>	pSeparator;

	PreInit();

	CGridContainer::Init(pWindow);
	mpTheme = pTheme;
	maTabs.Init();
	maPages.Init();
	mpSelected = NULL;
	miNextColumn = 0;
	mcSelected.Init();
	mcRemoved.Init();
	mcDoubleClicked.Init();
	msDoubleClick.Init();

	//The background is in the bar's cell first so the bar is drawn over it.
	pBackground = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pTheme->GetTabBarColour()));
	pBackground->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(pBackground, 0, 0, 1, 1, 1, 0);

	mpBar = OMalloc<CGridContainer>(pWindow);
	mpBar->SetAlignment(CSH_Stretch, CSV_Top);
	mpFiller = OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), 0, 0);
	mpBar->AddComponent(mpFiller, 0, 0, 1, 1, 1, 0);
	AddComponent(mpBar, 0, 0, 1, 1, 1, 0);

	pSeparator = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pTheme->GetTabSeparatorColour()));
	pSeparator->SetDesiredSize(-1, 1);
	pSeparator->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(pSeparator, 0, 1, 1, 1, 1, 0);

	mpPage = OMalloc<CFillContainer>(pWindow);
	mpPage->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(mpPage, 0, 2, 1, 1, 1, 1);

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
	M_Pointer(mpPage);
	M_Embedded(maTabs);
	M_Embedded(maPages);
	M_Pointer(mpSelected);
	U_Int32(miNextColumn);
	U_Data(CComponentEvent<CTabComponent>, mcSelected);
	U_Data(CTabRemovedEvent, mcRemoved);
	U_Data(CComponentEvent<CTabComponent>, mcDoubleClicked);
	U_Data(SDoubleClick, msDoubleClick);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTabbedContainer::Free(void)
{
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
// The tab goes after the others on the bar, with pIcon (which can be
// NULL) on its left and, if bClosable, a cross on its right.  The page
// should stretch to fill the area under the bar.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTabComponent> CTabbedContainer::AddTab(const char* szName, Ptr<CComponent> pPage, Ptr<CImageCel> pIcon, bool bClosable)
{
	Ptr<CTabComponent>	pTab;
	int32				iHeight;

	pTab = OMalloc<CTabComponent>(mpWindow, mpTheme, Ptr<CTabbedContainer>(this), szName, pIcon, bClosable);

	//A grid is variable sized so the bar is told how tall its tallest tab is, or its row would get no height.
	iHeight = pTab->GetDesiredSize().y;
	if (iHeight > mpBar->GetDesiredSize().y)
	{
		mpBar->SetDesiredSize(-1, iHeight);
	}

	//The filler always takes the column after the last tab.
	mpBar->RemoveComponent(mpFiller);
	mpBar->AddComponent(pTab, miNextColumn, 0);
	mpBar->AddComponent(mpFiller, miNextColumn + 1, 0, 1, 1, 1, 0);
	miNextColumn++;
	maTabs.Add(pTab);
	maPages.Add(pPage);

	if (mpSelected.IsNull())
	{
		Select(pTab);
	}
	return pTab;
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

	//The tabs after it move down an index.
	msDoubleClick.Forget();

	bWasSelected = (mpSelected == pTab);
	if (bWasSelected)
	{
		mpPage->RemoveAllComponents();
		mpSelected = NULL;
	}

	//The page is held while the removal event is called.
	pPage = maPages.Get(uiIndex);
	mpBar->RemoveComponent(pTab);
	maTabs.RemoveAt(uiIndex);
	maPages.RemoveAt(uiIndex);
	mcRemoved.Call(&pTab, &pPage);

	if (bWasSelected && (maTabs.Size() > 0))
	{
		Select(uiIndex < maTabs.Size() ? uiIndex : maTabs.Size() - 1);
	}
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

	if (uiIndex >= maTabs.Size())
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
	mcSelected.Call(&pTab);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTabbedContainer::IndexOf(Ptr<CTabComponent> pTab)
{
	size	i;
	size	uiNumTabs;

	uiNumTabs = maTabs.Size();
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
Ptr<CTabComponent> CTabbedContainer::GetSelected(void) { return mpSelected; }
size CTabbedContainer::NumTabs(void) { return maTabs.Size(); }
Ptr<CTabComponent> CTabbedContainer::GetTab(size uiIndex) { return maTabs.Get(uiIndex); }
Ptr<CComponent> CTabbedContainer::GetPage(size uiIndex) { return maPages.Get(uiIndex); }


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

	if (msDoubleClick.Released((int32)uiIndex, x, y))
	{
		mcDoubleClicked.Call(&pTab);
	}
}

