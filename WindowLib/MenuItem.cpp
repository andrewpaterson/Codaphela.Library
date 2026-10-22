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
#include "MenuContainer.h"
#include "MenuItem.h"


//////////////////////////////////////////////////////////////////////////
//
// pIcon and pSubMenu can be NULL.
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::Init(Ptr<CWindow> pWindow, Ptr<CMenuStyle> pStyle, const char* szText, Ptr<CImageCel> pIcon, Ptr<CMenuContainer> pSubMenu)
{
	int32	iLabelHeight;

	PreInit();

	CGridContainer::Init(pWindow);
	mpStyle = pStyle;
	mpSubMenu = pSubMenu;

	//The label is a line of the font high, the icon column is as tall as that with the padding above and below it, so
	//every item is the same height whether or not it has an icon.
	mpLabel = OMalloc<CLabel>(pWindow, pStyle->GetFont(), szText == NULL ? "" : szText);
	mpLabel->SetAlignment(CSH_Left, CSV_Center);
	iLabelHeight = mpLabel->GetDesiredSize().y;

	mpIcon = OMalloc<CIconComponent>(pWindow, pIcon, pStyle->GetIconWidth(), iLabelHeight + pStyle->GetItemPadding() * 2);
	mpIcon->SetAlignment(CSH_Left, CSV_Center);

	mpChevron = OMalloc<CIconComponent>(pWindow, pSubMenu.IsNotNull() ? pStyle->GetChevron() : Ptr<CImageCel>(), pStyle->GetChevronWidth(), -1);
	mpChevron->SetAlignment(CSH_Right, CSV_Center);

	CGridContainer::AddComponent(mpIcon, 0, 0);
	CGridContainer::AddComponent(mpLabel, 1, 0, 1, 1, 1, 0);
	CGridContainer::AddComponent(mpChevron, 2, 0);

	SetAlignment(CSH_Stretch, CSV_Stretch);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpStyle);
	M_Pointer(mpIcon);
	M_Pointer(mpLabel);
	M_Pointer(mpChevron);
	M_Pointer(mpSubMenu);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::Free(void)
{
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuItem::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuItem::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::SetText(const char* szText)
{
	mpLabel->SetText(szText == NULL ? "" : szText);
}


//////////////////////////////////////////////////////////////////////////
//
// NULL takes the icon away, leaving its column empty.
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::SetIcon(Ptr<CImageCel> pIcon)
{
	mpIcon->SetCel(pIcon);
}


//////////////////////////////////////////////////////////////////////////
//
// Like a label an item keeps the size its parts need unless a desired
// size is set on an axis.  Being stretched it still fills its row.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CMenuItem::GetDesiredSize(void)
{
	return GetDesiredOrRequiredSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMenuItem::GetText(void) { return mpLabel->GetText(); }
Ptr<CImageCel> CMenuItem::GetIcon(void) { return mpIcon->GetCel(); }
Ptr<CMenuContainer> CMenuItem::GetSubMenu(void) { return mpSubMenu; }
bool CMenuItem::HasSubMenu(void) { return mpSubMenu.IsNotNull(); }
Ptr<CIconComponent> CMenuItem::GetIconComponent(void) { return mpIcon; }
Ptr<CLabel> CMenuItem::GetLabel(void) { return mpLabel; }
Ptr<CIconComponent> CMenuItem::GetChevronComponent(void) { return mpChevron; }

