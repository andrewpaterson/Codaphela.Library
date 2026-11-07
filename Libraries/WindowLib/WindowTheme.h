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
#ifndef __WINDOW_THEME_H__
#define __WINDOW_THEME_H__
#include "StandardLib/Object.h"
#include "SupportLib/Font.h"
#include "SupportLib/ImageCel.h"
#include "SupportLib/ColourARGB32.h"
#include "ButtonImages.h"
#include "CheckBoxImages.h"
#include "ScrollBarImages.h"
#include "MenuStyle.h"
#include "ThemeColour.h"
#include "ComponentEvent.h"


//What a button looks like: a normal (outlined) one, a small one crowded in with text, or a flat one for a toolbar.
enum EButtonLook
{
	BL_Normal,
	BL_Small,
	BL_Flat,
};


//How the widgets look, shared by the widgets of any windows given it (a window doesn't own its theme).  Each widget
//takes the theme and its own content, never images or colours of its own, and reads the theme as it lays out and
//draws.  CJewelTheme makes IntelliJ IDEA's dark theme; a theme can be put together from other images instead, as the
//tests do.
//
//A theme has no looks of its own: whoever makes it sets every colour its widgets use, and reading one that wasn't set
//is an error.  The images and styles start NULL and only the widgets that use them need them: a theme without check
//box images can't make check boxes.  Changed tells the theme's users that it changed.
class CWindowTheme : public CObject
{
CONSTRUCTABLE(CWindowTheme);
DESTRUCTABLE(CWindowTheme);
protected:
	Ptr<CFont>				mpFont;  //Of captions, text fields and combo boxes.
	Ptr<CButtonImages>		mpButtonImages;
	Ptr<CButtonImages>		mpDefaultButtonImages;  //Drawn instead while a button is its window's default.  Optional.
	Ptr<CButtonImages>		mpSmallButtonImages;  //Optional, small buttons are normal without them.
	Ptr<CButtonImages>		mpToolButtonImages;
	Ptr<CCheckBoxImages>	mpCheckBoxImages;
	Ptr<CMenuStyle>			mpMenuStyle;  //The menu bar and its menus.
	Ptr<CMenuStyle>			mpComboMenuStyle;  //A combo box's list.
	Ptr<CImageCel>			mpComboChevron;
	Ptr<CScrollBarImages>	mpScrollBarImages;
	ARGB32					masColours[TC_NumColours];  //By EThemeColour.
	bool					mabColoursSet[TC_NumColours];
	CComponentEvent<CWindowTheme>	mcChangeEvent;
	Ptr<CImageCel>			mpTabClose;  //A tab's close cross, NULL for none.
	Ptr<CImageCel>			mpTabCloseHover;  //The cross under the pointer.

public:
	void					Init(Ptr<CFont> pFont);
	void					Class(void) override;
	void					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void					SetFont(Ptr<CFont> pFont);
	void					SetButtonImages(Ptr<CButtonImages> pImages, Ptr<CButtonImages> pDefaultImages);
	void					SetSmallButtonImages(Ptr<CButtonImages> pImages);
	void					SetToolButtonImages(Ptr<CButtonImages> pImages);
	void					SetCheckBoxImages(Ptr<CCheckBoxImages> pImages);
	void					SetMenuStyle(Ptr<CMenuStyle> pStyle);
	void					SetComboBoxLook(Ptr<CMenuStyle> pStyle, Ptr<CImageCel> pChevron);
	void					SetScrollBarImages(Ptr<CScrollBarImages> pImages);
	void					SetTabCloseImages(Ptr<CImageCel> pClose, Ptr<CImageCel> pCloseHover);
	void					SetColour(EThemeColour eColour, ARGB32 sColour);
	bool					IsColourSet(EThemeColour eColour);
	void					Changed(void);

	Ptr<CFont>				GetFont(void);
	Ptr<CButtonImages>		GetButtonImages(EButtonLook eLook);
	Ptr<CButtonImages>		GetDefaultButtonImages(EButtonLook eLook);
	Ptr<CCheckBoxImages>	GetCheckBoxImages(void);
	Ptr<CMenuStyle>			GetMenuStyle(void);
	Ptr<CMenuStyle>			GetComboMenuStyle(void);
	Ptr<CImageCel>			GetComboChevron(void);
	Ptr<CScrollBarImages>	GetScrollBarImages(void);
	ARGB32					GetColour(EThemeColour eColour);
	CComponentEvent<CWindowTheme>*	GetChangeEvent(void);
	Ptr<CImageCel>			GetTabClose(void);
	Ptr<CImageCel>			GetTabCloseHover(void);
};


#endif // __WINDOW_THEME_H__

