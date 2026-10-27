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


//What a button looks like: a normal (outlined) one, or a flat one for a toolbar.
enum EButtonLook
{
	BL_Normal,
	BL_Tool,
};


//How the widgets look, shared by every widget of a window (and its dialogs).  Each widget takes the theme and its own
//content, never images or colours of its own.  CJewelTheme makes IntelliJ IDEA's dark theme; a theme can be put
//together from other images instead, as the tests do.
//
//The colours start as Jewel's dark theme.  The images and styles start NULL and only the widgets that use them need
//them: a theme without check box images can't make check boxes.
class CWindowTheme : public CObject
{
CONSTRUCTABLE(CWindowTheme);
DESTRUCTABLE(CWindowTheme);
protected:
	Ptr<CFont>				mpFont;  //Of captions, text fields and combo boxes.
	Ptr<CButtonImages>		mpButtonImages;
	Ptr<CButtonImages>		mpDefaultButtonImages;  //Drawn instead while a button is its window's default.  Optional.
	Ptr<CButtonImages>		mpToolButtonImages;
	Ptr<CCheckBoxImages>	mpCheckBoxImages;
	Ptr<CMenuStyle>			mpMenuStyle;  //The menu bar and its menus.
	Ptr<CMenuStyle>			mpComboMenuStyle;  //A combo box's list.
	Ptr<CImageCel>			mpComboChevron;
	Ptr<CScrollBarImages>	mpScrollBarImages;
	ARGB32					msTextSelection;
	ARGB32					msCaret;
	ARGB32					msListSelected;
	ARGB32					msListHover;
	ARGB32					msTabBar;  //Behind a tabbed container's tabs.
	ARGB32					msTabHover;  //Behind a tab under the pointer.
	ARGB32					msTabUnderline;  //Under the selected tab.
	ARGB32					msTabSeparator;  //The line between the tabs and the page.
	Ptr<CImageCel>			mpTabClose;  //A tab's close cross, NULL for none.
	Ptr<CImageCel>			mpTabCloseHover;  //The cross under the pointer.

public:
	void					Init(Ptr<CFont> pFont);
	void					Class(void) override;
	void 					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void					SetFont(Ptr<CFont> pFont);
	void					SetButtonImages(Ptr<CButtonImages> pImages, Ptr<CButtonImages> pDefaultImages);
	void					SetToolButtonImages(Ptr<CButtonImages> pImages);
	void					SetCheckBoxImages(Ptr<CCheckBoxImages> pImages);
	void					SetMenuStyle(Ptr<CMenuStyle> pStyle);
	void					SetComboBoxLook(Ptr<CMenuStyle> pStyle, Ptr<CImageCel> pChevron);
	void					SetScrollBarImages(Ptr<CScrollBarImages> pImages);
	void					SetTextColours(ARGB32 sSelection, ARGB32 sCaret);
	void					SetListColours(ARGB32 sSelected, ARGB32 sHover);
	void					SetTabLook(ARGB32 sBar, ARGB32 sHover, ARGB32 sUnderline, ARGB32 sSeparator, Ptr<CImageCel> pClose, Ptr<CImageCel> pCloseHover);

	Ptr<CFont>				GetFont(void);
	Ptr<CButtonImages>		GetButtonImages(EButtonLook eLook);
	Ptr<CButtonImages>		GetDefaultButtonImages(EButtonLook eLook);
	Ptr<CCheckBoxImages>	GetCheckBoxImages(void);
	Ptr<CMenuStyle>			GetMenuStyle(void);
	Ptr<CMenuStyle>			GetComboMenuStyle(void);
	Ptr<CImageCel>			GetComboChevron(void);
	Ptr<CScrollBarImages>	GetScrollBarImages(void);
	ARGB32					GetTextSelectionColour(void);
	ARGB32					GetCaretColour(void);
	ARGB32					GetListSelectedColour(void);
	ARGB32					GetListHoverColour(void);
	ARGB32					GetTabBarColour(void);
	ARGB32					GetTabHoverColour(void);
	ARGB32					GetTabUnderlineColour(void);
	ARGB32					GetTabSeparatorColour(void);
	Ptr<CImageCel>			GetTabClose(void);
	Ptr<CImageCel>			GetTabCloseHover(void);
};


#endif // __WINDOW_THEME_H__

