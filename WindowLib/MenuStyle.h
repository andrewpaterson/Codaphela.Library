#ifndef __MENU_STYLE_H__
#define __MENU_STYLE_H__
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
#include "SupportLib/Font.h"
#include "SupportLib/ImageCel.h"
#include "SupportLib/ColourARGB32.h"
#include "ButtonImages.h"
#include "Insets.h"


class CWindow;


//How a menu bar and its menus look, shared by all of them.  The defaults are like the main menu of IntelliJ IDEA 2026's
//new UI in its dark theme: flat bar buttons, rounded highlights inset from the menu's sides and separators that stop
//short of them.  The font, Inter 13 in IntelliJ, is given.
//
//A menu item is a row of three columns: the icon column, the label and the chevron column.  The icon and chevron
//columns are a fixed width, whether or not an item has an icon or a chevron, so the labels line up down the menu.
class CMenuStyle : public CObject
{
CONSTRUCTABLE(CMenuStyle);
DESTRUCTABLE(CMenuStyle);
protected:
	Ptr<CFont>			mpFont;
	Ptr<CButtonImages>	mpBarButtonImages;
	Ptr<CImageCel>		mpChevron;
	ARGB32				msBarBackground;
	ARGB32				msMenuBackground;
	ARGB32				msMenuBorder;
	ARGB32				msSelected;
	ARGB32				msHover;
	ARGB32				msSeparator;
	SInsets				msMenuInsets;  //Between a menu's popup and its items, the border included.
	int32				miIconWidth;
	int32				miChevronWidth;
	int32				miItemPadding;  //Above and below an item's label.
	int32				miSeparatorHeight;
	SInt32Vec2			msBarPadding;  //Around a bar button's caption, inside its seven pixel border.
	int32				miSelectionRadius;  //Of the rounded block behind a menu's selected item.
	int32				miSelectionInset;  //Between the selected item's block and the menu's sides.
	int32				miSeparatorInset;  //Between a separator's line and the menu's sides.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CFont> pFont);
	void				Class(void) override;
	void 				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CFont>			GetFont(void);
	Ptr<CButtonImages>	GetBarButtonImages(void);
	Ptr<CImageCel>		GetChevron(void);
	ARGB32				GetBarBackground(void);
	ARGB32				GetMenuBackground(void);
	ARGB32				GetMenuBorder(void);
	ARGB32				GetSelected(void);
	ARGB32				GetHover(void);
	ARGB32				GetSeparator(void);
	SInsets*			GetMenuInsets(void);
	int32				GetIconWidth(void);
	int32				GetChevronWidth(void);
	int32				GetItemPadding(void);
	int32				GetSeparatorHeight(void);
	SInt32Vec2			GetBarPadding(void);
	int32				GetSelectionRadius(void);
	int32				GetSelectionInset(void);
	int32				GetSeparatorInset(void);
	void				SetIconWidth(int32 iIconWidth);
};

#endif // __MENU_STYLE_H__

