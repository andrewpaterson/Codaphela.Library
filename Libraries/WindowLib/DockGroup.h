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
#ifndef __DOCK_GROUP_H__
#define __DOCK_GROUP_H__
#include "TabbedContainer.h"
#include "DockTitleBar.h"
#include "DockNode.h"


class CMenuContainer;
class CMenuItem;


//The items of a pane's window position menu, in order.  A tool's menu has every item but Close; a document's has
//Float, Dock and Close.
enum EDockMenuItem
{
	DMI_Float,
	DMI_Dock,
	DMI_DockAsDocument,
	DMI_AutoHide,
	DMI_Hide,
	DMI_Close,
	DMI_NumItems,
};


//Panes shown as tabs, one at a time, as Visual Studio shows them.  A tool group has a title bar showing the selected
//pane's title, with the window position menu, the pin and the close cross, and its tabs along the bottom (none while
//it holds one pane).  A document group has its tabs along the top, each with a close cross, and no title bar.
//
//Pulling a tab off calls the manager to drag that pane; pulling the title bar drags the whole group.  Each tab has the
//pane's window position menu as its context menu, as does the title bar's chevron.  The group's ID is unique in its
//manager and is how panes find their way back to it (see CDockPane::SetRestore).
class CDockGroup : public CDockNode
{
CONSTRUCTABLE(CDockGroup);
DESTRUCTABLE(CDockGroup);
protected:
	int32					miID;
	bool					mbDocuments;
	bool					mbActive;
	Ptr<CDockTitleBar>		mpTitleBar;  //NULL for a document group.
	Ptr<CTabbedContainer>	mpTabs;
	CArray<CDockPane>		maPanes;  //Not in tab order: the tabbed container moves tabs itself.  GetPane is by tab.

public:
	void					Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, int32 iID, bool bDocuments);
	void					Class(void) override;
	void					Free(void) override;
	EDockNodeKind			GetKind(void) override;
	void					GetPanes(CArray<CDockPane>* paPanes) override;
	size					CountPanes(void) override;
	Ptr<CDockPane>			GetTreePane(size uiIndex) override;
	Ptr<CDockGroup>			FindGroupAt(int x, int y) override;
	Ptr<CDockGroup>			GetFirstGroup(void) override;
	void					SetRequiredSize(void) override;
	void					Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;

	void					AddPane(Ptr<CDockPane> pPane, size uiIndex = MAX_SIZE);
	bool					TakePane(Ptr<CDockPane> pPane);
	void					SelectPane(Ptr<CDockPane> pPane);
	Ptr<CDockPane>			GetSelectedPane(void);
	size					NumPanes(void);
	Ptr<CDockPane>			GetPane(size uiIndex);
	size					IndexOfPane(Ptr<CDockPane> pPane);
	bool					MovePane(Ptr<CDockPane> pPane, size uiIndex);
	void					PaneChanged(Ptr<CDockPane> pPane);
	void					SetActive(bool bActive);
	bool					IsActive(void);
	void					SetPinned(bool bPinned);
	int32					GetID(void);
	void					SetID(int32 iID);
	bool					IsDocumentGroup(void);
	Ptr<CTabbedContainer>	GetTabs(void);
	Ptr<CDockTitleBar>		GetTitleBar(void);
	Ptr<CMenuContainer>		GetMenu(Ptr<CDockPane> pPane);
	void					OpenMenu(Ptr<CDockPane> pPane, SInt32Vec2 sPosition);
	Ptr<CTabComponent>		GetTab(Ptr<CDockPane> pPane);

	void					TabSelected(CTabComponent* pcTab);
	void					TabClicked(CTabComponent* pcTab);
	void					TabDragged(CTabComponent* pcTab);
	void					TabDoubleClicked(CTabComponent* pcTab);
	void					TabCloseRequested(CTabComponent* pcTab);
	void					TitleDragged(CDockTitleBar* pcTitleBar);
	void					TitleDoubleClicked(CDockTitleBar* pcTitleBar);
	void					MenuButtonPressed(CDockTitleBar* pcTitleBar);
	void					PinButtonPressed(CDockTitleBar* pcTitleBar);
	void					CloseButtonPressed(CDockTitleBar* pcTitleBar);
	void					MenuItemClicked(CMenuItem* pcItem);

protected:
	Ptr<CMenuContainer>		CreateMenu(Ptr<CDockPane> pPane);
	void					AddMenuItem(Ptr<CMenuContainer> pMenu, EDockMenuItem eItem);
	size					SlotOf(Ptr<CDockPane> pPane);
	size					FindTab(CTabComponent* pcTab);
	void					UpdateTitle(void);
};


#endif // __DOCK_GROUP_H__

