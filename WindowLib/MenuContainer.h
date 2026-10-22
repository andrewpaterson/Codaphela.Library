#ifndef __MENU_CONTAINER_H__
#define __MENU_CONTAINER_H__
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
#include "BaseLib/ArrayInt.h"
#include "ListContainer.h"
#include "Popup.h"
#include "MenuItem.h"
#include "MenuStyle.h"
#include "MenuListener.h"


class CMenuBar;


//A list of menu items, and separators, shown in a popup.  The row under the pointer is selected; pressing and
//releasing on an item, or enter or space, clicks it.  An item that opens another menu opens it to its right when the
//pointer moves over it, or when it is clicked or right is pressed, and moving to another row closes it again.  Left
//closes a menu opened from another and goes back to that one.  On a menu from a menu bar left and right open the
//bar's neighbouring menus.
//
//Clicking an item closes every menu it was opened from and tells the listeners of the first of them, and of its menu
//bar if it has one, see CMenuListener.
class CMenuContainer : public CListContainer
{
CONSTRUCTABLE(CMenuContainer);
DESTRUCTABLE(CMenuContainer);
protected:
	Ptr<CMenuStyle>		mpStyle;
	CArray<CMenuItem>	maItems;
	CArrayInt			maiItemRows;  //The row of each item in maItems.
	Ptr<CMenuContainer>	mpParentMenu;  //The menu this one opens from, NULL for a menu that opens on its own.
	Ptr<CMenuBar>		mpMenuBar;  //The bar this menu hangs from, NULL if it doesn't.
	Ptr<CMenuContainer>	mpOpenSubMenu;
	int32				miOpenSubMenuRow;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CMenuStyle> pStyle);
	void				Class(void) override;
	void 				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CMenuItem>		AddItem(const char* szText, Ptr<CImageCel> pIcon = NULL, Ptr<CMenuContainer> pSubMenu = NULL);
	int32				AddSeparator(void);
	size				NumItems(void);
	Ptr<CMenuItem>		GetItem(size uiIndex);
	Ptr<CMenuItem>		GetItemInRow(int32 iRow);
	int32				GetItemRow(Ptr<CMenuItem> pItem);

	Ptr<CPopup>			Open(SInt32Vec2 sPosition, Ptr<CComponent> pOwner, bool bFocus);
	void				Close(void);
	bool				IsOpen(void);
	void				OpenSubMenu(int32 iRow, bool bFocus);
	void				CloseSubMenu(void);
	Ptr<CMenuContainer>	GetOpenSubMenu(void);
	Ptr<CMenuContainer>	GetParentMenu(void);
	void				SetMenuBar(Ptr<CMenuBar> pMenuBar);
	Ptr<CMenuBar>		GetMenuBar(void);
	Ptr<CMenuStyle>		GetStyle(void);

	using CListContainer::AddListener;
	using CListContainer::RemoveListener;
	bool				AddListener(CMenuListener* pcListener);
	bool				RemoveListener(CMenuListener* pcListener);

	void				MovedOver(int x, int y) override;
	void				Left(void) override;
	void				Right(void) override;
	void				PopupClosed(Ptr<CComponent> pContent) override;

protected:
	void				RowSelected(int32 iRow) override;
	void				RowActivated(int32 iRow) override;
	void				Reset(void);
	void				ItemClicked(Ptr<CMenuItem> pItem);
};

#endif // __MENU_CONTAINER_H__

