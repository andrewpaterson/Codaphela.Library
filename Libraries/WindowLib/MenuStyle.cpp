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
#include "WindowTheme.h"
#include "MenuStyle.h"


//////////////////////////////////////////////////////////////////////////
//
// The theme's menu colours, sizes, chevron and bar button images, and its
// font, used as it is (white say) for every label.
//
//////////////////////////////////////////////////////////////////////////
void CMenuStyle::Init(Ptr<CWindowTheme> pTheme)
{
	PreInit();

	mpFont = pTheme->GetFont();
	mpDimFont = mpFont;
	msBarBackground = pTheme->GetColour(TC_MenuBar);
	msMenuBackground = pTheme->GetColour(TC_MenuBackground);
	msMenuBorder = pTheme->GetColour(TC_MenuBorder);
	msSelected = pTheme->GetColour(TC_MenuSelected);
	msHover = pTheme->GetColour(TC_MenuHover);
	msSeparator = pTheme->GetColour(TC_MenuSeparator);
	msMenuInsets.Init(pTheme->GetMetric(TM_MenuInsetY), pTheme->GetMetric(TM_MenuInsetX), pTheme->GetMetric(TM_MenuInsetY), pTheme->GetMetric(TM_MenuInsetX));
	miIconWidth = pTheme->GetMetric(TM_MenuIconWidth);
	miChevronWidth = pTheme->GetMetric(TM_MenuChevronWidth);
	miItemPadding = pTheme->GetMetric(TM_MenuItemPadding);
	miSeparatorHeight = pTheme->GetMetric(TM_MenuSeparatorHeight);
	msBarPadding.Init(pTheme->GetMetric(TM_MenuBarPaddingX), pTheme->GetMetric(TM_MenuBarPaddingY));
	miSelectionRadius = pTheme->GetMetric(TM_MenuSelectionRadius);
	miSelectionInset = pTheme->GetMetric(TM_MenuSelectionInset);
	miSeparatorInset = pTheme->GetMetric(TM_MenuSeparatorInset);
	mpBarButtonImages = pTheme->GetMenuBarButtonImages();
	mpChevron = pTheme->GetImage(TI_Chevron);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuStyle::Class(void)
{
	M_Pointer(mpFont);
	M_Pointer(mpDimFont);
	M_Pointer(mpBarButtonImages);
	M_Pointer(mpChevron);
	U_UInt32(msBarBackground);
	U_UInt32(msMenuBackground);
	U_UInt32(msMenuBorder);
	U_UInt32(msSelected);
	U_UInt32(msHover);
	U_UInt32(msSeparator);
	U_Data(SInsets, msMenuInsets);
	U_Int32(miIconWidth);
	U_Int32(miChevronWidth);
	U_Int32(miItemPadding);
	U_Int32(miSeparatorHeight);
	U_2Int32(msBarPadding);
	U_Int32(miSelectionRadius);
	U_Int32(miSelectionInset);
	U_Int32(miSeparatorInset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuStyle::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuStyle::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuStyle::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The font of disabled items' text and of shortcuts, greyer than the
// font, as IntelliJ's.  Only items made after it is changed use it.
//
//////////////////////////////////////////////////////////////////////////
void CMenuStyle::SetDimFont(Ptr<CFont> pDimFont)
{
	mpDimFont = pDimFont;
}


//////////////////////////////////////////////////////////////////////////
//
// Only items made after it is changed use the new width.
//
//////////////////////////////////////////////////////////////////////////
void CMenuStyle::SetIconWidth(int32 iIconWidth)
{
	miIconWidth = iIconWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CMenuStyle::GetFont(void) { return mpFont; }
Ptr<CFont> CMenuStyle::GetDimFont(void) { return mpDimFont; }
Ptr<CButtonImages> CMenuStyle::GetBarButtonImages(void) { return mpBarButtonImages; }
Ptr<CImageCel> CMenuStyle::GetChevron(void) { return mpChevron; }
ARGB32 CMenuStyle::GetBarBackground(void) { return msBarBackground; }
ARGB32 CMenuStyle::GetMenuBackground(void) { return msMenuBackground; }
ARGB32 CMenuStyle::GetMenuBorder(void) { return msMenuBorder; }
ARGB32 CMenuStyle::GetSelected(void) { return msSelected; }
ARGB32 CMenuStyle::GetHover(void) { return msHover; }
ARGB32 CMenuStyle::GetSeparator(void) { return msSeparator; }
SInsets* CMenuStyle::GetMenuInsets(void) { return &msMenuInsets; }
int32 CMenuStyle::GetIconWidth(void) { return miIconWidth; }
int32 CMenuStyle::GetChevronWidth(void) { return miChevronWidth; }
int32 CMenuStyle::GetItemPadding(void) { return miItemPadding; }
int32 CMenuStyle::GetSeparatorHeight(void) { return miSeparatorHeight; }
SInt32Vec2 CMenuStyle::GetBarPadding(void) { return msBarPadding; }
int32 CMenuStyle::GetSelectionRadius(void) { return miSelectionRadius; }
int32 CMenuStyle::GetSelectionInset(void) { return miSelectionInset; }
int32 CMenuStyle::GetSeparatorInset(void) { return miSeparatorInset; }

