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
#include "JewelButtonImages.h"
#include "Window.h"
#include "MenuStyle.h"


//////////////////////////////////////////////////////////////////////////
//
// The images are made in the window's colour order.  The font is used as
// it is, white say, for every label.
//
//////////////////////////////////////////////////////////////////////////
void CMenuStyle::Init(Ptr<CWindow> pWindow, Ptr<CFont> pFont)
{
	CColourFormatHelper	cFormat;
	int					aiMenuInsets[4] = { 6, 1, 6, 1 };  //Top, Right, Bottom, Left.

	PreInit();

	pWindow->GetColourFormat(&cFormat);

	mpFont = pFont;
	msBarBackground = Set32BitColour((uint8)0x27, 0x28, 0x2E);
	msMenuBackground = Set32BitColour((uint8)0x2B, 0x2D, 0x30);
	msMenuBorder = Set32BitColour((uint8)0x43, 0x45, 0x4A);
	msSelected = Set32BitColour((uint8)0x2E, 0x43, 0x6E);
	msHover = Set32BitColour((uint8)0x39, 0x3B, 0x40);
	msSeparator = Set32BitColour((uint8)0x43, 0x45, 0x4A);
	msMenuInsets.Init(aiMenuInsets);
	miIconWidth = 32;
	miChevronWidth = 24;
	miItemPadding = 5;
	miSeparatorHeight = 9;
	msBarPadding.Init(1, 1);
	miSelectionRadius = 4;
	miSelectionInset = 6;
	miSeparatorInset = 8;

	mpBarButtonImages = CJewelButtonImages::CreateFlatButtonImages(msBarBackground, msHover, msHover, &cFormat);
	mpChevron = CJewelButtonImages::CreateChevron(Set32BitColour((uint8)0xCE, 0xD0, 0xD6), &cFormat);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuStyle::Class(void)
{
	M_Pointer(mpFont);
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

