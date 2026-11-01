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
#include "RoundedBoxCanvasDraw.h"
#include "MenuBar.h"
#include "MenuContainer.h"


//As well as the list's.
static EComponentInputCommand	gaeMenuContainerCommands[] = { CIC_Left, CIC_Right };


//////////////////////////////////////////////////////////////////////////
//
// A menu in the theme's menu style.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	Init(pWindow, pTheme, pTheme->GetMenuStyle());
}


//////////////////////////////////////////////////////////////////////////
//
// The row under the pointer is selected, a rounded block of the style's
// selection colour inset from the menu's sides.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CMenuStyle> pStyle)
{
	SInsets		sInsets;
	int			aiInsets[4];

	PreInit();

	CListContainer::Init(pWindow, pTheme);
	mcListeners.AddAllowedClass<CMenuListener>();
	mpStyle = pStyle;
	maItems.Init();
	maiItemRows.Init();
	mpParentMenu = NULL;
	mpMenuBar = NULL;
	mpOpenSubMenu = NULL;
	miOpenSubMenuRow = -1;
	SetSelectOnHover(true);

	aiInsets[0] = 0;
	aiInsets[1] = pStyle->GetSelectionInset();
	aiInsets[2] = 0;
	aiInsets[3] = pStyle->GetSelectionInset();
	sInsets.Init(aiInsets);
	SetHighlightDraws(OMalloc<CRoundedBoxCanvasDraw>(pStyle->GetSelected(), pStyle->GetSelectionRadius(), &sInsets),
					  OMalloc<CRoundedBoxCanvasDraw>(pStyle->GetHover(), pStyle->GetSelectionRadius(), &sInsets));
	AddInputActions(gaeMenuContainerCommands, sizeof(gaeMenuContainerCommands) / sizeof(EComponentInputCommand), this, &CMenuContainer::CommandAction);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The input actions of gaeMenuContainerCommands.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::CommandAction(SComponentInputEvent* psEvent)
{
	switch (psEvent->eCommand)
	{
	case CIC_Left:
		Left();
		break;
	case CIC_Right:
		Right();
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::Class(void)
{
	CListContainer::Class();

	M_Pointer(mpStyle);
	M_Embedded(maItems);
	U_Data(CArrayInt, maiItemRows);
	M_Pointer(mpParentMenu);
	M_Pointer(mpMenuBar);
	M_Pointer(mpOpenSubMenu);
	U_Int32(miOpenSubMenuRow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::Free(void)
{
	maiItemRows.Kill();
	CListContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuContainer::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuContainer::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// pIcon and pSubMenu can be NULL.  A sub menu can only be in one item.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMenuItem> CMenuContainer::AddItem(const char* szText, Ptr<CImageCel> pIcon, Ptr<CMenuContainer> pSubMenu)
{
	Ptr<CMenuItem>	pItem;
	int32			iRow;

	pItem = OMalloc<CMenuItem>(mpWindow, mpStyle, szText, pIcon, pSubMenu);
	if (pSubMenu.IsNotNull())
	{
		pSubMenu->mpParentMenu = this;
	}
	iRow = CListContainer::AddItem(pItem);
	maItems.Add(pItem);
	maiItemRows.Add(iRow);
	return pItem;
}


//////////////////////////////////////////////////////////////////////////
//
// A line across the menu between groups of items.  Its row can't be
// selected.  Returns the row.
//
//////////////////////////////////////////////////////////////////////////
int32 CMenuContainer::AddSeparator(void)
{
	int32					iRow;
	Ptr<CIconComponent>		pSpace;
	Ptr<CRoundedBoxCanvasDraw>	pDraw;
	Ptr<CCanvas>			pLine;
	SInsets					sInsets;
	int						aiInsets[4];

	iRow = AddRow(false);

	//An empty canvas makes the row the separator's height and the line is drawn across the middle of it, stopping short
	//of the menu's sides.
	pSpace = OMalloc<CIconComponent>(mpWindow, Ptr<CImageCel>(), 0, mpStyle->GetSeparatorHeight());
	AddToRow(iRow, pSpace, 0, 1, 1);
	aiInsets[0] = 0;
	aiInsets[1] = mpStyle->GetSeparatorInset();
	aiInsets[2] = 0;
	aiInsets[3] = mpStyle->GetSeparatorInset();
	sInsets.Init(aiInsets);
	pDraw = OMalloc<CRoundedBoxCanvasDraw>(mpStyle->GetSeparator(), 0, &sInsets);
	pLine = OMalloc<CCanvas>(mpWindow, pDraw);
	pLine->SetTransparent(true);
	pLine->SetDesiredSize(-1, 1);
	pLine->SetAlignment(CSH_Stretch, CSV_Center);
	AddToRow(iRow, pLine, 0, 1, 1);
	return iRow;
}


//////////////////////////////////////////////////////////////////////////
//
// Every row goes, so a combo box can show other items.  An open sub menu
// is closed first.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::RemoveAllItems(void)
{
	CloseSubMenu();
	RemoveAllComponents();
	maItems.RemoveAll();
	maiItemRows.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The item in iRow, or NULL if the row is a separator or doesn't exist.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMenuItem> CMenuContainer::GetItemInRow(int32 iRow)
{
	size	i;

	for (i = 0; i < maiItemRows.NumElements(); i++)
	{
		if (maiItemRows.GetValue(i) == iRow)
		{
			return maItems.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// -1 if the item isn't in this menu.
//
//////////////////////////////////////////////////////////////////////////
int32 CMenuContainer::GetItemRow(Ptr<CMenuItem> pItem)
{
	size	i;

	for (i = 0; i < maItems.Size(); i++)
	{
		if (maItems.Get(i) == pItem)
		{
			return maiItemRows.GetValue(i);
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the menu in a popup with its top left at sPosition, in window
// client space.  pOwner is told when it closes.  With bFocus the menu
// takes the keys.  Nothing is selected until the pointer or a key picks
// a row.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CPopup> CMenuContainer::Open(SInt32Vec2 sPosition, Ptr<CComponent> pOwner, bool bFocus)
{
	Ptr<CPopup>	pPopup;

	Close();
	Reset();
	pPopup = mpWindow->OpenPopup(this, sPosition, pOwner);
	pPopup->SetLook(mpStyle->GetMenuBackground(), mpStyle->GetMenuBorder(), mpStyle->GetMenuInsets());
	pPopup->Layout(mpWindow->GetPosition(), mpWindow->GetActualSize());
	if (bFocus)
	{
		mpWindow->GetFocus()->SetFocus(this);
	}
	return pPopup;
}


//////////////////////////////////////////////////////////////////////////
//
// Closes the menu and any menu opened from it.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::Close(void)
{
	mpWindow->ClosePopup(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuContainer::IsOpen(void)
{
	return mpWindow->IsPopupOpen(this);
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the sub menu of the item in iRow beside this menu, its first
// item level with the item.  With bFocus the sub menu takes the keys with
// its first item selected.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::OpenSubMenu(int32 iRow, bool bFocus)
{
	Ptr<CMenuItem>		pItem;
	Ptr<CMenuContainer>	pSubMenu;
	Ptr<CPopup>			pPopup;
	SInt32Vec2			sPosition;
	int32				iTop;
	int32				iHeight;

	pItem = GetItemInRow(iRow);
	if (pItem.IsNull() || !pItem->HasSubMenu())
	{
		return;
	}
	pPopup = mpWindow->GetPopup(this);
	if (pPopup.IsNull() || !GetTrackBounds(false, (size)iRow, &iTop, &iHeight))
	{
		return;
	}

	pSubMenu = pItem->GetSubMenu();
	if (pSubMenu != mpOpenSubMenu)
	{
		CloseSubMenu();
		sPosition.x = pPopup->GetPosition().x + pPopup->GetActualSize().x;
		sPosition.y = iTop - mpStyle->GetMenuInsets()->maiWidths[BEI_Top];
		pSubMenu->Open(sPosition, this, false);
		mpOpenSubMenu = pSubMenu;
		miOpenSubMenuRow = iRow;
	}

	if (bFocus)
	{
		mpWindow->GetFocus()->SetFocus(pSubMenu);
		pSubMenu->Home();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// If the keys were in the sub menu, or a menu opened from it, they come
// back to this menu.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::CloseSubMenu(void)
{
	Ptr<CComponent>		pFocussed;
	Ptr<CMenuContainer>	pSubMenu;

	if (mpOpenSubMenu.IsNull())
	{
		return;
	}

	pFocussed = mpWindow->GetFocus()->GetFocussedComponent();
	if (pFocussed.IsNotNull())
	{
		pSubMenu = mpOpenSubMenu;
		while (pSubMenu.IsNotNull())
		{
			if (pFocussed->IsWithin(pSubMenu))
			{
				mpWindow->GetFocus()->SetFocus(this);
				break;
			}
			pSubMenu = pSubMenu->mpOpenSubMenu;
		}
	}
	mpOpenSubMenu->Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::SetMenuBar(Ptr<CMenuBar> pMenuBar)
{
	mpMenuBar = pMenuBar;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuContainer::AddListener(CMenuListener* pcListener)
{
	return mcListeners.AddListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuContainer::RemoveListener(CMenuListener* pcListener)
{
	return mcListeners.RemoveListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
// The row moved over is selected by the list.  If its item opens a menu
// that menu opens too.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::MovedOver(int x, int y)
{
	Ptr<CMenuItem>	pItem;

	CListContainer::MovedOver(x, y);
	if ((miHover >= 0) && (miHover != miOpenSubMenuRow))
	{
		pItem = GetItemInRow(miHover);
		if (pItem.IsNotNull() && pItem->HasSubMenu())
		{
			OpenSubMenu(miHover, false);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Goes back to the menu this one was opened from, or to the menu bar's
// menu on the left.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::Left(void)
{
	if (mpParentMenu.IsNotNull())
	{
		mpParentMenu->CloseSubMenu();
	}
	else if (mpMenuBar.IsNotNull())
	{
		mpMenuBar->OpenAdjacentMenu(-1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Goes into the selected item's menu, or to the menu bar's menu on the
// right.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::Right(void)
{
	Ptr<CMenuItem>	pItem;

	pItem = GetItemInRow(miSelected);
	if (pItem.IsNotNull() && pItem->HasSubMenu())
	{
		OpenSubMenu(miSelected, true);
	}
	else if (mpMenuBar.IsNotNull() && mpParentMenu.IsNull())
	{
		mpMenuBar->OpenAdjacentMenu(1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::PopupClosed(Ptr<CComponent> pContent)
{
	if (pContent == mpOpenSubMenu)
	{
		mpOpenSubMenu = NULL;
		miOpenSubMenuRow = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Selecting another row closes the menu opened from the row before.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::RowSelected(int32 iRow)
{
	if (mpOpenSubMenu.IsNotNull() && (iRow != miOpenSubMenuRow))
	{
		CloseSubMenu();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::RowActivated(int32 iRow)
{
	Ptr<CMenuItem>	pItem;

	pItem = GetItemInRow(iRow);
	if (pItem.IsNull())
	{
		return;
	}

	if (pItem->HasSubMenu())
	{
		OpenSubMenu(iRow, true);
	}
	else
	{
		ItemClicked(pItem);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A menu starts each opening with nothing selected and nothing under the
// pointer.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::Reset(void)
{
	CloseSubMenu();
	Select(-1);
	miHover = -1;
	mbPressed = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Every menu from the first one the item was reached from closes before
// the listeners are told.  The component that opened the first menu, a
// combo box say, is told too.  It is found through the object graph, not
// a listener, so it needs nothing removed when it is freed.
//
//////////////////////////////////////////////////////////////////////////
void CMenuContainer::ItemClicked(Ptr<CMenuItem> pItem)
{
	Ptr<CMenuContainer>	pRoot;
	Ptr<CPopup>			pPopup;
	Ptr<CComponent>		pOwner;

	pRoot = this;
	while (pRoot->mpParentMenu.IsNotNull() && pRoot->mpParentMenu->IsOpen())
	{
		pRoot = pRoot->mpParentMenu;
	}

	//The owner is only known while the menu's popup is open.
	pPopup = mpWindow->GetPopup(pRoot);
	if (pPopup.IsNotNull())
	{
		pOwner = pPopup->GetOwner();
	}
	pPopup = NULL;

	pRoot->Close();
	pRoot->mcListeners.CallListeners(&CMenuListener::MenuItemClicked, &pItem, NULL);
	if (pRoot->mpMenuBar.IsNotNull())
	{
		pRoot->mpMenuBar->MenuItemClicked(pItem);
	}
	if (pOwner.IsNotNull())
	{
		pOwner->PopupItemClicked(pRoot, pItem);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMenuContainer::NumItems(void) { return maItems.Size(); }
Ptr<CMenuItem> CMenuContainer::GetItem(size uiIndex) { return maItems.Get(uiIndex); }
Ptr<CMenuContainer> CMenuContainer::GetOpenSubMenu(void) { return mpOpenSubMenu; }
Ptr<CMenuContainer> CMenuContainer::GetParentMenu(void) { return mpParentMenu; }
Ptr<CMenuBar> CMenuContainer::GetMenuBar(void) { return mpMenuBar; }
Ptr<CMenuStyle> CMenuContainer::GetStyle(void) { return mpStyle; }

