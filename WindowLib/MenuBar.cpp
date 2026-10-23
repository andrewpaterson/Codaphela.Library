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
#include "Window.h"
#include "MenuBar.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuBar::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	PreInit();

	CGridContainer::Init(pWindow);
	mpStyle = pTheme->GetMenuStyle();
	maItems.Init();
	mpOpenItem = NULL;
	mcListeners.Init();
	mcListeners.AddAllowedClass<CMenuListener>();

	//Like a list's highlights the background is the first component, so the buttons are drawn over it, in the top left
	//cell with no size of its own.  It is moved over the whole bar after the bar is laid out.
	mpBackgroundDraw = OMalloc<CClearCanvasDraw>(mpStyle->GetBarBackground());
	mpBackground = OMalloc<CCanvas>(pWindow, mpBackgroundDraw);
	mpBackground->SetDesiredSize(-1, -1);
	CGridContainer::AddComponent(mpBackground, 0, 0);

	mpFiller = OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), 0, 0);
	CGridContainer::AddComponent(mpFiller, 0, 0, 1, 1, 1, 0);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuBar::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpStyle);
	M_Pointer(mpBackgroundDraw);
	M_Pointer(mpBackground);
	M_Pointer(mpFiller);
	M_Embedded(maItems);
	M_Pointer(mpOpenItem);
	U_Data(CListenerCall, mcListeners);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuBar::Free(void)
{
	mcListeners.Kill();
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuBar::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuBar::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a button to the right of the others that opens pMenu.  pIcon can
// be NULL, and szText NULL or empty for a button with only an icon.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMenuBarItem> CMenuBar::AddMenu(const char* szText, Ptr<CMenuContainer> pMenu, Ptr<CImageCel> pIcon)
{
	Ptr<CMenuBarItem>	pItem;
	int					iColumn;

	pItem = OMalloc<CMenuBarItem>(mpWindow, this, mpStyle, pMenu, szText, pIcon);
	pMenu->SetMenuBar(this);

	//The filler always takes the column after the last button.
	iColumn = (int)maItems.Size();
	CGridContainer::RemoveComponent(mpFiller);
	CGridContainer::AddComponent(pItem, iColumn, 0);
	CGridContainer::AddComponent(mpFiller, iColumn + 1, 0, 1, 1, 1, 0);
	maItems.Add(pItem);
	return pItem;
}


//////////////////////////////////////////////////////////////////////////
//
// Opens pItem's menu below it, closing any other menu from the bar.  The
// menu takes the keys.  With bSelectFirst its first item is selected, as
// when it is opened from the keyboard.
//
//////////////////////////////////////////////////////////////////////////
void CMenuBar::OpenMenu(Ptr<CMenuBarItem> pItem, bool bSelectFirst)
{
	Ptr<CMenuContainer>	pMenu;
	SInt32Vec2			sPosition;

	if (mpOpenItem != pItem)
	{
		CloseMenu();

		sPosition = pItem->GetPosition();
		sPosition.y += pItem->GetActualSize().y;
		pMenu = pItem->GetMenu();
		pMenu->Open(sPosition, pItem, true);
		pItem->SetHighlighted(true);
		mpOpenItem = pItem;
	}

	if (bSelectFirst)
	{
		pItem->GetMenu()->Home();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The button whose menu it was keeps the keys.
//
//////////////////////////////////////////////////////////////////////////
void CMenuBar::CloseMenu(void)
{
	Ptr<CMenuBarItem>	pItem;

	pItem = mpOpenItem;
	if (pItem.IsNotNull())
	{
		mpWindow->GetFocus()->SetFocus(pItem);
		pItem->GetMenu()->Close();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the menu iStep buttons along from the open one, going round from
// one end of the bar to the other, with its first item selected.
//
//////////////////////////////////////////////////////////////////////////
void CMenuBar::OpenAdjacentMenu(int32 iStep)
{
	int32	iIndex;
	int32	iNumItems;

	iIndex = IndexOf(mpOpenItem);
	iNumItems = (int32)maItems.Size();
	if ((iIndex < 0) || (iNumItems == 0))
	{
		return;
	}

	iIndex = (((iIndex + iStep) % iNumItems) + iNumItems) % iNumItems;
	OpenMenu(maItems.Get(iIndex), true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuBar::FocusAdjacentItem(Ptr<CMenuBarItem> pItem, int32 iStep)
{
	int32	iIndex;
	int32	iNumItems;

	iIndex = IndexOf(pItem);
	iNumItems = (int32)maItems.Size();
	if ((iIndex < 0) || (iNumItems == 0))
	{
		return;
	}

	iIndex = (((iIndex + iStep) % iNumItems) + iNumItems) % iNumItems;
	mpWindow->GetFocus()->SetFocus(maItems.Get(iIndex));
}


//////////////////////////////////////////////////////////////////////////
//
// Called by a button once its menu has closed, however it was closed.
//
//////////////////////////////////////////////////////////////////////////
void CMenuBar::MenuClosed(Ptr<CMenuBarItem> pItem)
{
	pItem->SetHighlighted(false);
	if (mpOpenItem == pItem)
	{
		mpOpenItem = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Called by the menu the item was reached from, once it has closed.
//
//////////////////////////////////////////////////////////////////////////
void CMenuBar::MenuItemClicked(Ptr<CMenuItem> pItem)
{
	mcListeners.CallListeners(&CMenuListener::MenuItemClicked, &pItem, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuBar::AddListener(CMenuListener* pcListener)
{
	return mcListeners.AddListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMenuBar::RemoveListener(CMenuListener* pcListener)
{
	return mcListeners.RemoveListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
// The bar is as tall as its buttons need, unless a desired height is set,
// and is usually stretched across its parent.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CMenuBar::GetDesiredSize(void)
{
	return GetDesiredOrRequiredSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMenuBar::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	CGridContainer::Layout(sPosition, sAreaSize);
	LayoutComponentInArea(mpBackground, sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
// -1 if pItem isn't on the bar.
//
//////////////////////////////////////////////////////////////////////////
int32 CMenuBar::IndexOf(Ptr<CMenuBarItem> pItem)
{
	size	i;

	for (i = 0; i < maItems.Size(); i++)
	{
		if (maItems.Get(i) == pItem)
		{
			return (int32)i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMenuBar::NumMenus(void) { return maItems.Size(); }
Ptr<CMenuBarItem> CMenuBar::GetItem(size uiIndex) { return maItems.Get(uiIndex); }
Ptr<CMenuBarItem> CMenuBar::GetOpenItem(void) { return mpOpenItem; }
Ptr<CMenuStyle> CMenuBar::GetStyle(void) { return mpStyle; }

