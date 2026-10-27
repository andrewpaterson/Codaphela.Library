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
#include "JewelColours.h"
#include "WindowTheme.h"


//////////////////////////////////////////////////////////////////////////
//
// pFont can be NULL for a theme whose widgets have no text.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::Init(Ptr<CFont> pFont)
{
	PreInit();

	mpFont = pFont;
	mpButtonImages = NULL;
	mpDefaultButtonImages = NULL;
	mpToolButtonImages = NULL;
	mpCheckBoxImages = NULL;
	mpMenuStyle = NULL;
	mpComboMenuStyle = NULL;
	mpComboChevron = NULL;
	mpScrollBarImages = NULL;
	msTextSelection = JEWEL_SELECTION;
	msCaret = JEWEL_ICON;
	msListSelected = JEWEL_SELECTION;
	msListHover = JEWEL_HOVER;
	msTabBar = JEWEL_EDITOR;
	msTabHover = JEWEL_HOVER;
	msTabUnderline = JEWEL_BLUE;
	msTabSeparator = JEWEL_SEPARATOR;
	mpTabClose = NULL;
	mpTabCloseHover = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::Class(void)
{
	M_Pointer(mpFont);
	M_Pointer(mpButtonImages);
	M_Pointer(mpDefaultButtonImages);
	M_Pointer(mpToolButtonImages);
	M_Pointer(mpCheckBoxImages);
	M_Pointer(mpMenuStyle);
	M_Pointer(mpComboMenuStyle);
	M_Pointer(mpComboChevron);
	M_Pointer(mpScrollBarImages);
	U_UInt32(msTextSelection);
	U_UInt32(msCaret);
	U_UInt32(msListSelected);
	U_UInt32(msListHover);
	U_UInt32(msTabBar);
	U_UInt32(msTabHover);
	U_UInt32(msTabUnderline);
	U_UInt32(msTabSeparator);
	M_Pointer(mpTabClose);
	M_Pointer(mpTabCloseHover);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindowTheme::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindowTheme::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// pDefaultImages can be NULL, a default button then looks like any other.
// It must have the same tile size as pImages.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetButtonImages(Ptr<CButtonImages> pImages, Ptr<CButtonImages> pDefaultImages)
{
	mpButtonImages = pImages;
	mpDefaultButtonImages = pDefaultImages;
}


//////////////////////////////////////////////////////////////////////////
//
// A combo box's list is a menu, it has its own style so that it can have
// a narrower icon column than the main menus.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetComboBoxLook(Ptr<CMenuStyle> pStyle, Ptr<CImageCel> pChevron)
{
	mpComboMenuStyle = pStyle;
	mpComboChevron = pChevron;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetTextColours(ARGB32 sSelection, ARGB32 sCaret)
{
	msTextSelection = sSelection;
	msCaret = sCaret;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetListColours(ARGB32 sSelected, ARGB32 sHover)
{
	msListSelected = sSelected;
	msListHover = sHover;
}


//////////////////////////////////////////////////////////////////////////
//
// A tabbed container's look.  Without close images a tab can't show its
// cross so it can't be closed from its tab.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetTabLook(ARGB32 sBar, ARGB32 sHover, ARGB32 sUnderline, ARGB32 sSeparator, Ptr<CImageCel> pClose, Ptr<CImageCel> pCloseHover)
{
	msTabBar = sBar;
	msTabHover = sHover;
	msTabUnderline = sUnderline;
	msTabSeparator = sSeparator;
	mpTabClose = pClose;
	mpTabCloseHover = pCloseHover;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> CWindowTheme::GetButtonImages(EButtonLook eLook)
{
	if (eLook == BL_Tool)
	{
		return mpToolButtonImages;
	}
	return mpButtonImages;
}


//////////////////////////////////////////////////////////////////////////
//
// A tool button is never drawn as a default button.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> CWindowTheme::GetDefaultButtonImages(EButtonLook eLook)
{
	if (eLook == BL_Tool)
	{
		return NULL;
	}
	return mpDefaultButtonImages;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetFont(Ptr<CFont> pFont) { mpFont = pFont; }
void CWindowTheme::SetToolButtonImages(Ptr<CButtonImages> pImages) { mpToolButtonImages = pImages; }
void CWindowTheme::SetCheckBoxImages(Ptr<CCheckBoxImages> pImages) { mpCheckBoxImages = pImages; }
void CWindowTheme::SetMenuStyle(Ptr<CMenuStyle> pStyle) { mpMenuStyle = pStyle; }
void CWindowTheme::SetScrollBarImages(Ptr<CScrollBarImages> pImages) { mpScrollBarImages = pImages; }
Ptr<CFont> CWindowTheme::GetFont(void) { return mpFont; }
Ptr<CCheckBoxImages> CWindowTheme::GetCheckBoxImages(void) { return mpCheckBoxImages; }
Ptr<CMenuStyle> CWindowTheme::GetMenuStyle(void) { return mpMenuStyle; }
Ptr<CMenuStyle> CWindowTheme::GetComboMenuStyle(void) { return mpComboMenuStyle; }
Ptr<CImageCel> CWindowTheme::GetComboChevron(void) { return mpComboChevron; }
Ptr<CScrollBarImages> CWindowTheme::GetScrollBarImages(void) { return mpScrollBarImages; }
ARGB32 CWindowTheme::GetTextSelectionColour(void) { return msTextSelection; }
ARGB32 CWindowTheme::GetCaretColour(void) { return msCaret; }
ARGB32 CWindowTheme::GetListSelectedColour(void) { return msListSelected; }
ARGB32 CWindowTheme::GetListHoverColour(void) { return msListHover; }
ARGB32 CWindowTheme::GetTabBarColour(void) { return msTabBar; }
ARGB32 CWindowTheme::GetTabHoverColour(void) { return msTabHover; }
ARGB32 CWindowTheme::GetTabUnderlineColour(void) { return msTabUnderline; }
ARGB32 CWindowTheme::GetTabSeparatorColour(void) { return msTabSeparator; }
Ptr<CImageCel> CWindowTheme::GetTabClose(void) { return mpTabClose; }
Ptr<CImageCel> CWindowTheme::GetTabCloseHover(void) { return mpTabCloseHover; }

