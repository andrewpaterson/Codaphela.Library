#ifndef __MENU_BAR_ITEM_H__
#define __MENU_BAR_ITEM_H__
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
#include "Button.h"
#include "MenuStyle.h"


class CMenuContainer;
class CMenuBar;


//A button on a menu bar that opens its menu below it.  It has the style's small flat bar look: highlighted while the
//pointer is over it or its menu is open, and never drawn pushed.  Pressing it opens its menu, or closes it if it is open.
//While a menu from the bar is open, moving over another of the bar's buttons opens that button's menu instead.
class CMenuBarItem : public CButton
{
CONSTRUCTABLE(CMenuBarItem);
DESTRUCTABLE(CMenuBarItem);
protected:
	Ptr<CMenuBar>		mpMenuBar;
	Ptr<CMenuContainer>	mpMenu;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CMenuBar> pMenuBar, Ptr<CMenuStyle> pStyle, Ptr<CMenuContainer> pMenu, const char* szText, Ptr<CImageCel> pIcon);
	void				Class(void) override;
	void 				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CMenuContainer>	GetMenu(void);
	bool				IsMenuOpen(void);

	void				MovedInto(void) override;
	void				ActivateComponent(int x, int y) override;
	void				ExecuteComponent(int x, int y) override;
	void				Space(bool bInsert) override;
	void				Enter(void) override;
	void				Down(void) override;
	void				Left(void) override;
	void				Right(void) override;
	void				PopupClosed(Ptr<CComponent> pContent) override;
};

#endif // __MENU_BAR_ITEM_H__

