#ifndef __MENU_BAR_H__
#define __MENU_BAR_H__
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
#include "StandardLib/ListenerCall.h"
#include "GridContainer.h"
#include "ClearCanvasDraw.h"
#include "IconComponent.h"
#include "MenuStyle.h"
#include "MenuListener.h"
#include "MenuBarItem.h"
#include "MenuContainer.h"


//A row of buttons along the top of a window, each opening a menu below it.  The buttons sit on the left; the bar is
//filled with the style's bar colour across whatever width it is given.  At most one of its menus is open at a time.
//
//Clicking an item in any of its menus, or their sub menus, tells the bar's listeners, see CMenuListener.
class CMenuBar : public CGridContainer
{
CONSTRUCTABLE(CMenuBar);
DESTRUCTABLE(CMenuBar);
protected:
	Ptr<CMenuStyle>			mpStyle;
	Ptr<CClearCanvasDraw>	mpBackgroundDraw;
	Ptr<CCanvas>			mpBackground;
	Ptr<CIconComponent>		mpFiller;  //Takes the spare width to the right of the buttons.
	CArray<CMenuBarItem>	maItems;
	Ptr<CMenuBarItem>		mpOpenItem;
	CListenerCall			mcListeners;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CMenuStyle> pStyle);
	void				Class(void) override;
	void 				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CMenuBarItem>	AddMenu(const char* szText, Ptr<CMenuContainer> pMenu, Ptr<CImageCel> pIcon = NULL);
	size				NumMenus(void);
	Ptr<CMenuBarItem>	GetItem(size uiIndex);
	Ptr<CMenuBarItem>	GetOpenItem(void);
	Ptr<CMenuStyle>		GetStyle(void);

	void				OpenMenu(Ptr<CMenuBarItem> pItem, bool bSelectFirst);
	void				CloseMenu(void);
	void				OpenAdjacentMenu(int32 iStep);
	void				FocusAdjacentItem(Ptr<CMenuBarItem> pItem, int32 iStep);
	void				MenuClosed(Ptr<CMenuBarItem> pItem);
	void				MenuItemClicked(Ptr<CMenuItem> pItem);

	bool				AddListener(CMenuListener* pcListener);
	bool				RemoveListener(CMenuListener* pcListener);

	SInt32Vec2			GetDesiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;

protected:
	int32				IndexOf(Ptr<CMenuBarItem> pItem);
};

#endif // __MENU_BAR_H__

