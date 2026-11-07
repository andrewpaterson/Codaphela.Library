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
#ifndef __MENU_ITEM_H__
#define __MENU_ITEM_H__
#include "GridContainer.h"
#include "ComponentEvent.h"
#include "IconComponent.h"
#include "Label.h"
#include "MenuStyle.h"
#include "ComponentInputCommand.h"


class CMenuContainer;


//Between an item's label and its shortcut, in the shortcut's label.
#define MENU_ITEM_SHORTCUT_GAP	"      "


//One row of a menu: an icon on the left, a label in the middle, the shortcut of what it does, Ctrl+Z say, right aligned
//in the style's dim font as IntelliJ's and, if the item opens another menu, a chevron on the right.  They are four
//columns of a grid; the icon and chevron columns are the style's fixed widths, so the labels of
//every item in a menu line up, and the label's column takes whatever width the menu has spare.  An item with no icon
//leaves its icon column empty.
class CMenuItem : public CGridContainer
{
CONSTRUCTABLE(CMenuItem);
DESTRUCTABLE(CMenuItem);
friend class CMenuContainer;
protected:
	Ptr<CMenuStyle>			mpStyle;
	Ptr<CIconComponent>		mpIcon;
	Ptr<CLabel>				mpLabel;
	Ptr<CLabel>				mpShortcut;
	Ptr<CIconComponent>		mpChevron;
	Ptr<CMenuContainer>		mpSubMenu;  //NULL unless the item opens another menu.
	Ptr<CMenuContainer>		mpMenu;  //The menu the item is in.
	int32					miTag;  //What the item means to whoever made it, -1 until set.
	bool					mbEnabled;  //A disabled item is drawn in the dim font and its row in the menu can't be selected.
	EComponentInputCommand	meShortcut;  //The command whose keys are shown (SetShortcut), CIC_Ignored if none.
	CComponentEvent<CMenuItem>	mcClicked;  //Called when the item is clicked, before its menus' item click events.

public:
	void					Init(Ptr<CWindow> pWindow, Ptr<CMenuStyle> pStyle, const char* szText, Ptr<CImageCel> pIcon, Ptr<CMenuContainer> pSubMenu);
	void					Class(void) override;
	void					Free(void) override;
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void					SetText(const char* szText);
	char*					GetText(void);
	void					SetIcon(Ptr<CImageCel> pIcon);
	Ptr<CImageCel>			GetIcon(void);
	Ptr<CMenuContainer>		GetSubMenu(void);
	void					SetShortcut(const char* szShortcut);
	void					SetShortcut(EComponentInputCommand eCommand);
	char*					GetShortcut(void);
	void					SetEnabled(bool bEnabled);
	bool					IsEnabled(void);
	void					SetTag(int32 iTag);
	int32					GetTag(void);
	bool					HasSubMenu(void);
	Ptr<CIconComponent>		GetIconComponent(void);
	Ptr<CLabel>				GetLabel(void);
	Ptr<CLabel>				GetShortcutLabel(void);
	Ptr<CIconComponent>		GetChevronComponent(void);
	CComponentEvent<CMenuItem>*	GetClickEvent(void);

protected:
	void					ShowEnabled(bool bEnabled);
	void					SetShortcutCommand(EComponentInputCommand eCommand);
	void					ShowShortcut(const char* szShortcut);
	void					MenuOpening(void);
};

#endif // __MENU_ITEM_H__

