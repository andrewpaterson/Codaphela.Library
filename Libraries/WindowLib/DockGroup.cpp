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
#include "MenuContainer.h"
#include "MenuItem.h"
#include "DockManager.h"
#include "DockPane.h"
#include "DockGroup.h"


static const char* gaszDockMenuItems[DMI_NumItems] = { "Float", "Dock", "Dock as Tabbed Document", "Auto Hide", "Hide", "Close" };


//////////////////////////////////////////////////////////////////////////
//
// A tool group (bDocuments false) or a document group.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, int32 iID, bool bDocuments)
{
	PreInit();

	CDockNode::Init(pWindow, pManager);
	miID = iID;
	mbDocuments = bDocuments;
	mbActive = false;
	maPanes.Init();

	mpTabs = OMalloc<CTabbedContainer>(pWindow, pManager->GetTheme());
	mpTabs->SetAlignment(CSH_Stretch, CSV_Stretch);
	if (bDocuments)
	{
		mpTabs->SetTabSide(TS_Top);
		mpTabs->SetOverflowMenu(true);
		mpTitleBar = NULL;
	}
	else
	{
		mpTabs->SetTabSide(TS_Bottom);
		mpTabs->SetHideSingleTab(true);
		mpTitleBar = OMalloc<CDockTitleBar>(pWindow, pManager, "", true, true, true);
		mpTitleBar->GetDragEvent()->Add(this, &CDockGroup::TitleDragged);
		mpTitleBar->GetDoubleClickEvent()->Add(this, &CDockGroup::TitleDoubleClicked);
		mpTitleBar->GetButtonEvent(DTB_Menu)->Add(this, &CDockGroup::MenuButtonPressed);
		mpTitleBar->GetButtonEvent(DTB_Pin)->Add(this, &CDockGroup::PinButtonPressed);
		mpTitleBar->GetButtonEvent(DTB_Close)->Add(this, &CDockGroup::CloseButtonPressed);
		AddComponent(mpTitleBar);
	}
	AddComponent(mpTabs);
	mpTabs->GetSelectionEvent()->Add(this, &CDockGroup::TabSelected);
	mpTabs->GetDragEvent()->Add(this, &CDockGroup::TabDragged);
	mpTabs->GetDoubleClickEvent()->Add(this, &CDockGroup::TabDoubleClicked);
	mpTabs->GetClickEvent()->Add(this, &CDockGroup::TabClicked);
	mpTabs->GetCloseRequestEvent()->Add(this, &CDockGroup::TabCloseRequested);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::Class(void)
{
	CDockNode::Class();

	U_Int32(miID);
	U_Bool(mbDocuments);
	U_Bool(mbActive);
	M_Pointer(mpTitleBar);
	M_Pointer(mpTabs);
	M_Embedded(maPanes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::Free(void)
{
	size				i;
	Ptr<CMenuContainer>	pMenu;

	for (i = 0; i < maPanes.Size(); i++)
	{
		pMenu = GetMenu(maPanes.Get(i));
		if (pMenu.IsNotNull())
		{
			pMenu->GetItemClickEvent()->Remove(this);
		}
	}
	if (mpTabs.IsNotNull())
	{
		mpTabs->GetSelectionEvent()->Remove(this);
		mpTabs->GetDragEvent()->Remove(this);
		mpTabs->GetDoubleClickEvent()->Remove(this);
		mpTabs->GetClickEvent()->Remove(this);
		mpTabs->GetCloseRequestEvent()->Remove(this);
	}
	if (mpTitleBar.IsNotNull())
	{
		mpTitleBar->RemoveListener(this);
	}
	CDockNode::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EDockNodeKind CDockGroup::GetKind(void)
{
	return DNK_Group;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::GetPanes(CArray<CDockPane>* paPanes)
{
	size	i;

	for (i = 0; i < NumPanes(); i++)
	{
		paPanes->Add(GetPane(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockGroup::FindGroupAt(int x, int y)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockGroup::GetFirstGroup(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::SetRequiredSize(void)
{
	msRequiredSize.Init(DOCK_MINIMUM_SIZE, DOCK_MINIMUM_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
// The title bar along the top, the tabs under it.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	int32		iTitle;
	SInt32Vec2	sTabsPosition;
	SInt32Vec2	sTabsSize;

	SetActualBounds(sPosition, sAreaSize);
	SetContainerBounds(sPosition, sAreaSize);
	iTitle = 0;
	if (mpTitleBar.IsNotNull())
	{
		iTitle = DOCK_TITLE_BAR_HEIGHT;
		if (iTitle > sAreaSize.y)
		{
			iTitle = sAreaSize.y;
		}
		mpTitleBar->Layout(sPosition, SInt32Vec2(sAreaSize.x, iTitle));
	}
	sTabsPosition.Init(sPosition.x, sPosition.y + iTitle);
	sTabsSize.Init(sAreaSize.x, sAreaSize.y - iTitle);
	mpTabs->Layout(sTabsPosition, sTabsSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The pane's tab goes at uiIndex, or after the others.  It is selected.
// A document's tab has a close cross; a tool is closed by the title bar.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::AddPane(Ptr<CDockPane> pPane, size uiIndex)
{
	Ptr<CTabComponent>	pTab;
	Ptr<CMenuContainer>	pMenu;
	bool				bClosable;

	if (uiIndex > maPanes.Size())
	{
		uiIndex = maPanes.Size();
	}
	bClosable = mbDocuments && pPane->IsClosable();
	maPanes.Add(pPane);
	pMenu = CreateMenu(pPane);
	pPane->SetGroup(this);
	pTab = mpTabs->InsertTab(uiIndex, pPane->GetTitle(), pPane->GetContent(), pPane->GetIcon(), bClosable);
	pTab->SetContextMenu(pMenu);
	if (pPane->GetTabFont().IsNotNull())
	{
		pTab->SetTextFont(pPane->GetTabFont());
	}
	mpTabs->Select(pTab);
	UpdateTitle();
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the pane's tab off without closing it.  Returns false if the pane
// isn't in the group.
//
//////////////////////////////////////////////////////////////////////////
bool CDockGroup::TakePane(Ptr<CDockPane> pPane)
{
	size				uiSlot;
	Ptr<CTabComponent>	pTab;

	uiSlot = SlotOf(pPane);
	pTab = GetTab(pPane);
	if ((uiSlot == ARRAY_ELEMENT_NOT_FOUND) || pTab.IsNull())
	{
		return false;
	}
	pTab->GetContextMenu()->GetItemClickEvent()->Remove(this);
	pTab->SetContextMenu(NULL);
	maPanes.RemoveAt(uiSlot);
	mpTabs->TakeTab(pTab);
	if (pPane->GetGroup() == this)
	{
		pPane->SetGroup(NULL);
	}
	UpdateTitle();
	InvalidateLayout();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::SelectPane(Ptr<CDockPane> pPane)
{
	size	uiIndex;

	uiIndex = IndexOfPane(pPane);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		mpTabs->Select(uiIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// NULL for an empty group.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockPane> CDockGroup::GetSelectedPane(void)
{
	Ptr<CTabComponent>	pTab;

	pTab = mpTabs->GetSelected();
	if (pTab.IsNull())
	{
		return NULL;
	}
	return GetPane(mpTabs->IndexOf(pTab));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDockGroup::IndexOfPane(Ptr<CDockPane> pPane)
{
	if (SlotOf(pPane) == ARRAY_ELEMENT_NOT_FOUND)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	return mpTabs->IndexOfPage(pPane->GetContent());
}


//////////////////////////////////////////////////////////////////////////
//
// Where the pane is in maPanes, which isn't in tab order:
// the tabbed container moves tabs itself, to show the selected one.
//
//////////////////////////////////////////////////////////////////////////
size CDockGroup::SlotOf(Ptr<CDockPane> pPane)
{
	size	i;

	for (i = 0; i < maPanes.Size(); i++)
	{
		if (maPanes.Get(i) == pPane)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// The pane on the tab at uiIndex, in the order the tabs are on the bar.
// NULL past the last.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockPane> CDockGroup::GetPane(size uiIndex)
{
	Ptr<CComponent>	pPage;
	size			i;

	if (uiIndex >= mpTabs->NumTabs())
	{
		return NULL;
	}
	pPage = mpTabs->GetPage(uiIndex);
	for (i = 0; i < maPanes.Size(); i++)
	{
		if (maPanes.Get(i)->GetContent() == pPage)
		{
			return maPanes.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockGroup::MovePane(Ptr<CDockPane> pPane, size uiIndex)
{
	Ptr<CTabComponent>	pTab;

	pTab = GetTab(pPane);
	if (pTab.IsNull())
	{
		return false;
	}
	return mpTabs->MoveTab(pTab, uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The pane's title or icon changed.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::PaneChanged(Ptr<CDockPane> pPane)
{
	Ptr<CTabComponent>	pTab;

	pTab = GetTab(pPane);
	if (pTab.IsNull())
	{
		return;
	}
	if (StringCompare(pTab->GetText(), pPane->GetTitle()) != 0)
	{
		pTab->SetText(pPane->GetTitle());
	}
	if (pTab->GetIcon().IsNotNull() && pPane->GetIcon().IsNotNull())
	{
		pTab->GetIcon()->SetCel(pPane->GetIcon());
	}
	pTab->SetTextFont(pPane->GetTabFont());
	UpdateTitle();
}


//////////////////////////////////////////////////////////////////////////
//
// The title bar shows the selected pane's title.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::UpdateTitle(void)
{
	Ptr<CDockPane>	pPane;

	if (mpTitleBar.IsNull())
	{
		return;
	}
	pPane = GetSelectedPane();
	mpTitleBar->SetTitle(pPane.IsNotNull() ? pPane->GetTitle() : "");
	mpTitleBar->ShowButton(DTB_Close, pPane.IsNotNull() && pPane->IsClosable());
}


//////////////////////////////////////////////////////////////////////////
//
// The group with the active pane has its title bar highlighted.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::SetActive(bool bActive)
{
	mbActive = bActive;
	if (mpTitleBar.IsNotNull())
	{
		mpTitleBar->SetActive(bActive);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::SetPinned(bool bPinned)
{
	if (mpTitleBar.IsNotNull())
	{
		mpTitleBar->SetPinned(bPinned);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pane's window position menu.  A document's has Float, Dock and
// Close; a tool's the rest.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMenuContainer> CDockGroup::CreateMenu(Ptr<CDockPane> pPane)
{
	Ptr<CMenuContainer>	pMenu;

	pMenu = OMalloc<CMenuContainer>(mpWindow, mpManager->GetTheme());
	AddMenuItem(pMenu, DMI_Float);
	AddMenuItem(pMenu, DMI_Dock);
	if (pPane->IsDocument())
	{
		pMenu->AddSeparator();
		AddMenuItem(pMenu, DMI_Close);
	}
	else
	{
		AddMenuItem(pMenu, DMI_DockAsDocument);
		AddMenuItem(pMenu, DMI_AutoHide);
		AddMenuItem(pMenu, DMI_Hide);
	}
	pMenu->GetItemClickEvent()->Add(this, &CDockGroup::MenuItemClicked);
	return pMenu;
}


//////////////////////////////////////////////////////////////////////////
//
// The item is tagged with what it does.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::AddMenuItem(Ptr<CMenuContainer> pMenu, EDockMenuItem eItem)
{
	pMenu->AddItem(gaszDockMenuItems[eItem])->SetTag(eItem);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMenuContainer> CDockGroup::GetMenu(Ptr<CDockPane> pPane)
{
	Ptr<CTabComponent>	pTab;

	pTab = GetTab(pPane);
	if (pTab.IsNull())
	{
		return NULL;
	}
	return pTab->GetContextMenu();
}


//////////////////////////////////////////////////////////////////////////
//
// sPosition is in window client space.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::OpenMenu(Ptr<CDockPane> pPane, SInt32Vec2 sPosition)
{
	Ptr<CMenuContainer>	pMenu;

	pMenu = GetMenu(pPane);
	if (pMenu.IsNotNull())
	{
		pMenu->Open(sPosition, this, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTabComponent> CDockGroup::GetTab(Ptr<CDockPane> pPane)
{
	size	uiIndex;

	uiIndex = IndexOfPane(pPane);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return NULL;
	}
	return mpTabs->GetTab(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDockGroup::FindTab(CTabComponent* pcTab)
{
	return mpTabs->IndexOf(pcTab);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::TabSelected(CTabComponent* pcTab)
{
	size	uiIndex;

	UpdateTitle();
	uiIndex = FindTab(pcTab);
	if (uiIndex < NumPanes())
	{
		mpManager->PaneSelected(GetPane(uiIndex));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A tab clicked activates its pane: its content (its focus target) gets
// the keys, as IntelliJ's editor does when its tab is clicked.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::TabClicked(CTabComponent* pcTab)
{
	size	uiIndex;

	uiIndex = FindTab(pcTab);
	if (uiIndex < NumPanes())
	{
		mpManager->Activate(GetPane(uiIndex));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The manager takes the rest of the drag.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::TabDragged(CTabComponent* pcTab)
{
	size	uiIndex;

	uiIndex = FindTab(pcTab);
	if (uiIndex < NumPanes())
	{
		mpManager->BeginPaneDrag(GetPane(uiIndex));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A docked pane floats, a floating one docks back.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::TabDoubleClicked(CTabComponent* pcTab)
{
	size	uiIndex;

	uiIndex = FindTab(pcTab);
	if (uiIndex < NumPanes())
	{
		mpManager->ToggleFloat(GetPane(uiIndex));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The manager asks the application before a document closes.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::TabCloseRequested(CTabComponent* pcTab)
{
	size	uiIndex;

	uiIndex = FindTab(pcTab);
	if (uiIndex < NumPanes())
	{
		mpManager->Close(GetPane(uiIndex));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::TitleDragged(CDockTitleBar* pcTitleBar)
{
	mpManager->BeginGroupDrag(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::TitleDoubleClicked(CDockTitleBar* pcTitleBar)
{
	mpManager->ToggleFloatGroup(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The selected pane's menu opens under the chevron.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::MenuButtonPressed(CDockTitleBar* pcTitleBar)
{
	Ptr<CIconComponent>	pButton;
	SInt32Vec2			sPosition;
	Ptr<CDockPane>		pPane;

	pPane = GetSelectedPane();
	if (pPane.IsNull())
	{
		return;
	}
	pButton = mpTitleBar->GetButton(DTB_Menu);
	sPosition = pButton->GetPosition();
	sPosition.y += pButton->GetActualSize().y;
	OpenMenu(pPane, sPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// The whole group auto hides, as Visual Studio's pin does.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::PinButtonPressed(CDockTitleBar* pcTitleBar)
{
	mpManager->AutoHideGroup(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The selected pane is hidden, as Visual Studio's cross does.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::CloseButtonPressed(CDockTitleBar* pcTitleBar)
{
	Ptr<CDockPane>	pPane;

	pPane = GetSelectedPane();
	if (pPane.IsNotNull())
	{
		mpManager->Close(pPane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An item of one of the panes' menus.
//
//////////////////////////////////////////////////////////////////////////
void CDockGroup::MenuItemClicked(CMenuItem* pcItem)
{
	size				i;
	Ptr<CDockPane>		pPane;
	Ptr<CMenuContainer>	pMenu;
	int32				iItem;

	iItem = pcItem->GetTag();
	if ((iItem < 0) || (iItem >= DMI_NumItems))
	{
		return;
	}
	for (i = 0; i < NumPanes(); i++)
	{
		pPane = GetPane(i);
		pMenu = GetMenu(pPane);
		if (pMenu.IsNotNull() && pcItem->IsWithin(pMenu))
		{
			mpManager->MenuChosen(pPane, (EDockMenuItem)iItem);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size					CDockGroup::NumPanes(void) { return maPanes.Size(); }
bool					CDockGroup::IsActive(void) { return mbActive; }
int32					CDockGroup::GetID(void) { return miID; }
void					CDockGroup::SetID(int32 iID) { miID = iID; }
bool					CDockGroup::IsDocumentGroup(void) { return mbDocuments; }
Ptr<CTabbedContainer>	CDockGroup::GetTabs(void) { return mpTabs; }
Ptr<CDockTitleBar>		CDockGroup::GetTitleBar(void) { return mpTitleBar; }


size					CDockGroup::CountPanes(void) { return NumPanes(); }
Ptr<CDockPane>			CDockGroup::GetTreePane(size uiIndex) { return GetPane(uiIndex); }

