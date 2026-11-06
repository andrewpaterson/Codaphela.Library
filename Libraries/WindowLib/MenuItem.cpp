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
	mcClicked.Init();
	SetSizedByContent(true);
	mpStyle = pStyle;
	mpSubMenu = pSubMenu;
	mpMenu = NULL;
	miTag = -1;
	mbEnabled = true;

	//The label is a line of the font high, the icon column is as tall as that with the padding above and below it, so
	//every item is the same height whether or not it has an icon.
	mpLabel = OMalloc<CLabel>(pWindow, pStyle->GetFont(), szText == NULL ? "" : szText);
	mpLabel->SetAlignment(CSH_Left, CSV_Center);
	iLabelHeight = mpLabel->GetDesiredSize().y;

	mpIcon = OMalloc<CIconComponent>(pWindow, pIcon, pStyle->GetIconWidth(), iLabelHeight + pStyle->GetItemPadding() * 2);
	mpIcon->SetAlignment(CSH_Left, CSV_Center);

	mpShortcut = OMalloc<CLabel>(pWindow, pStyle->GetDimFont(), "");
	mpShortcut->SetAlignment(CSH_Right, CSV_Center);

	mpChevron = OMalloc<CIconComponent>(pWindow, pSubMenu.IsNotNull() ? pStyle->GetChevron() : Ptr<CImageCel>(), pStyle->GetChevronWidth(), -1);
	mpChevron->SetAlignment(CSH_Right, CSV_Center);

	CGridContainer::AddComponent(mpIcon, 0, 0);
	CGridContainer::AddComponent(mpLabel, 1, 0, 1, 1, 1, 0);
	CGridContainer::AddComponent(mpShortcut, 2, 0);
	CGridContainer::AddComponent(mpChevron, 3, 0);

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
	M_Pointer(mpShortcut);
	M_Pointer(mpChevron);
	M_Pointer(mpSubMenu);
	M_Pointer(mpMenu);
	U_Int32(miTag);
	U_Bool(mbEnabled);
	U_Data(CComponentEvent<CMenuItem>, mcClicked);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::Free(void)
{
	mcClicked.Kill();
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

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// NULL takes the icon away, leaving its column empty.
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::SetIcon(Ptr<CImageCel> pIcon)
{
	mpIcon->SetCel(pIcon);

	//What it is laid out from has changed.
	InvalidateLayout();
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
Ptr<CLabel> CMenuItem::GetShortcutLabel(void) { return mpShortcut; }
bool CMenuItem::IsEnabled(void) { return mbEnabled; }


//////////////////////////////////////////////////////////////////////////
//
// The keys that do what the item does, "Ctrl+Z" say, shown on its right.
// It's only shown: the keys are the component input's.  NULL or "" shows
// none.  A gap keeps it apart from the longest label.
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::SetShortcut(const char* szShortcut)
{
	CChars	szText;

	szText.Init();
	if ((szShortcut != NULL) && (szShortcut[0] != '\0'))
	{
		szText.Append(MENU_ITEM_SHORTCUT_GAP);
		szText.Append(szShortcut);
	}
	mpShortcut->SetText(szText.Text());
	szText.Kill();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Without the gap.
//
//////////////////////////////////////////////////////////////////////////
char* CMenuItem::GetShortcut(void)
{
	char*	szText;

	szText = mpShortcut->GetText();
	if (szText[0] == '\0')
	{
		return szText;
	}
	return &szText[StrLen(MENU_ITEM_SHORTCUT_GAP)];
}


//////////////////////////////////////////////////////////////////////////
//
// A disabled item's text is drawn in the style's dim font and, in its
// menu, its row can't be selected, moved over or clicked: it is the
// menu's SetRowEnabled for the item's row.
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::SetEnabled(bool bEnabled)
{
	if (mpMenu.IsNotNull())
	{
		mpMenu->SetRowEnabled(mpMenu->GetItemRow(this), bEnabled);
	}
	else
	{
		ShowEnabled(bEnabled);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The item is drawn enabled or, in the dim font, disabled.  Its menu
// calls this as its row is enabled or disabled.
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::ShowEnabled(bool bEnabled)
{
	if (mbEnabled != bEnabled)
	{
		mbEnabled = bEnabled;
		mpLabel->SetFont(bEnabled ? mpStyle->GetFont() : mpStyle->GetDimFont());
	}
}
Ptr<CIconComponent> CMenuItem::GetChevronComponent(void) { return mpChevron; }


//////////////////////////////////////////////////////////////////////////
//
// A number of the menu's maker's own, so a click is known by what the
// item does rather than by its text.
//
//////////////////////////////////////////////////////////////////////////
void CMenuItem::SetTag(int32 iTag) { miTag = iTag; }
int32 CMenuItem::GetTag(void) { return miTag; }


//////////////////////////////////////////////////////////////////////////
//
// Called with the item when it is clicked, before the item click events
// of the menus it was reached from: whoever added the item can handle it
// without a menu wide event, in a menu others add items to too.
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CMenuItem>* CMenuItem::GetClickEvent(void) { return &mcClicked; }
