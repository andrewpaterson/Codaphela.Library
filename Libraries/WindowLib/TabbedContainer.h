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
#ifndef __TABBED_CONTAINER_H__
#define __TABBED_CONTAINER_H__
#include "GridContainer.h"
#include "FillContainer.h"
#include "IconComponent.h"
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "DoubleClick.h"
#include "WindowTheme.h"
#include "TabComponent.h"
#include "ComponentEvent.h"


//A tab that was removed, and its page.
typedef CComponentValueEvent<CTabComponent, CComponent*>	CTabRemovedEvent;


//Where the bar of tabs is.  Visual Studio's tool windows have theirs along the bottom.
enum ETabSide
{
	TS_Top,
	TS_Bottom,
};


class CMenuContainer;
class CMenuItem;


//A bar of tabs over a page, like IntelliJ IDEA's editor tabs.  Each tab has its own page; the selected tab's page
//fills the area under the bar and the others are kept, not shown.  The first tab added is selected.  A closable tab's
//cross removes it and its page; the tab after it (or before, if it was the last) is selected if it was.  Its events are
//called with the tab when a tab is selected, removed (with its page too) or double clicked.
//
//The bar can be along the bottom (SetTabSide) and hidden while there is only one tab (SetHideSingleTab), as Visual
//Studio's tool windows have it.  With the overflow menu (SetOverflowMenu) a chevron at the bar's right lists every tab;
//choosing one moves it to the front and selects it.  With it, only the tabs that fit on the bar are shown, as in
//Visual Studio; a selected tab that doesn't fit is moved to the front.  Without it, tabs that don't fit are cut off at
//the container's right edge, and a selected one is moved to the front too.  A tab pulled a few
//pixels along while pressed calls the drag event, once, with the tab: a dock group listens to it to tear the tab off.
//The drag event's listener takes over the rest of the drag (see CFocus::HandOverDrag).  While something listens to the
//close request event a tab's cross calls it instead of removing the tab, so the listener can decide.
class CTabbedContainer : public CGridContainer
{
CONSTRUCTABLE(CTabbedContainer);
DESTRUCTABLE(CTabbedContainer);
protected:
	Ptr<CWindowTheme>				mpTheme;
	Ptr<CGridContainer>				mpBar;
	Ptr<CComponent>					mpFiller;  //The space after the last tab.
	Ptr<CCanvas>					mpBarBackground;
	Ptr<CCanvas>					mpSeparator;
	Ptr<CIconComponent>				mpOverflow;  //The chevron opening the list of tabs, NULL without the overflow menu.
	Ptr<CMenuContainer>				mpOverflowMenu;
	Ptr<CFillContainer>				mpPage;
	CArray<CTabComponent>			maTabs;
	CArray<CComponent>				maPages;  //By tab.
	Ptr<CTabComponent>				mpSelected;
	ETabSide						meTabSide;
	bool							mbHideSingleTab;
	bool							mbBarShown;
	size							muiShownTabs;  //The tabs at the front of the bar that fit on it; the rest are only in the overflow menu.
	CComponentEvent<CTabComponent>	mcSelected;
	CTabRemovedEvent				mcRemoved;
	CComponentEvent<CTabComponent>	mcDoubleClicked;
	CComponentEvent<CTabComponent>	mcClicked;  //Pressed and let go without being dragged, selected or not.
	CComponentEvent<CTabComponent>	mcDragged;
	CComponentEvent<CTabComponent>	mcCloseRequested;
	SDoubleClick					msDoubleClick;  //Keyed by tab index, positions relative to the tab.

public:
	void					Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void					Class(void) override;
	void					Free(void) override;
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	Ptr<CTabComponent>				AddTab(const char* szName, Ptr<CComponent> pPage, Ptr<CImageCel> pIcon = NULL, bool bClosable = false);
	Ptr<CTabComponent>				InsertTab(size uiIndex, const char* szName, Ptr<CComponent> pPage, Ptr<CImageCel> pIcon = NULL, bool bClosable = false);
	bool							RemoveTab(Ptr<CTabComponent> pTab);
	Ptr<CComponent>					TakeTab(Ptr<CTabComponent> pTab);
	bool							MoveTab(Ptr<CTabComponent> pTab, size uiIndex);
	void							SetTabSide(ETabSide eSide);
	ETabSide						GetTabSide(void);
	void							SetHideSingleTab(bool bHide);
	bool							IsBarShown(void);
	void							SetOverflowMenu(bool bOverflow);
	void							OpenOverflowMenu(void);
	Ptr<CMenuContainer>				GetOverflowMenu(void);
	size							IndexOf(Ptr<CTabComponent> pTab);
	size							IndexOfPage(Ptr<CComponent> pPage);
	Ptr<CTabComponent>				FindTabAt(int x, int y);
	bool							IsTabShown(Ptr<CTabComponent> pTab);
	size							NumShownTabs(void);
	void							Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	void							Select(Ptr<CTabComponent> pTab);
	void							Select(size uiIndex);
	Ptr<CTabComponent>				GetSelected(void);
	bool							IsPointInBar(int x, int y);
	size							NumTabs(void);
	Ptr<CTabComponent>				GetTab(size uiIndex);
	Ptr<CComponent>					GetPage(size uiIndex);
	CComponentEvent<CTabComponent>*	GetSelectionEvent(void);
	CTabRemovedEvent*				GetRemovalEvent(void);
	CComponentEvent<CTabComponent>*	GetDoubleClickEvent(void);
	CComponentEvent<CTabComponent>*	GetClickEvent(void);
	CComponentEvent<CTabComponent>*	GetDragEvent(void);
	CComponentEvent<CTabComponent>*	GetCloseRequestEvent(void);
	void							TabClosePressed(Ptr<CTabComponent> pTab);
	void							TabReleased(Ptr<CTabComponent> pTab, int x, int y, bool bClick);
	void							TabDragged(Ptr<CTabComponent> pTab);
	bool							IsTabDraggable(void);
	void							OverflowItemClicked(CMenuItem* pcItem);

protected:
	void					PlaceRows(void);
	void					PlaceTabs(void);
	void					SetBarFit(bool bStretch);
	void					OverflowPressed(int x, int y);
	Ptr<CComponent>			RemoveTabOnly(size uiIndex, bool* pbWasSelected);
};


#endif // __TABBED_CONTAINER_H__
