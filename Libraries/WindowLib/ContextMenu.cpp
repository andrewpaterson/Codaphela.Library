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
#include "MenuContainer.h"
#include "ContextMenu.h"


//////////////////////////////////////////////////////////////////////////
//
// pMenu becomes pComponent's context menu: its actions for the right
// button and the menu keys are added to pComponent.
//
//////////////////////////////////////////////////////////////////////////
void CContextMenu::Init(Ptr<CComponent> pComponent, Ptr<CMenuContainer> pMenu)
{
	PreInit();

	mpComponent = pComponent;
	mpMenu = pMenu;
	mpClickAction = pComponent->AddInputAction(CIC_ContextMenu, this, &CContextMenu::ClickAction);
	mpKeyAction = pComponent->AddInputAction(CIC_ContextMenuKey, this, &CContextMenu::KeyAction);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContextMenu::Class(void)
{
	M_Pointer(mpComponent);
	M_Pointer(mpMenu);
	M_Pointer(mpClickAction);
	M_Pointer(mpKeyAction);
	M_Pointer(mpFocusBefore);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContextMenu::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CContextMenu::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CContextMenu::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The component's actions for the menu go, and the menu closes, when it
// is no longer the component's context menu.
//
//////////////////////////////////////////////////////////////////////////
void CContextMenu::RemoveActions(void)
{
	if (IsOpen())
	{
		mpMenu->Close();
	}
	if (mpClickAction.IsNotNull())
	{
		mpComponent->RemoveInputAction(mpClickAction);
		mpClickAction = NULL;
	}
	if (mpKeyAction.IsNotNull())
	{
		mpComponent->RemoveInputAction(mpKeyAction);
		mpKeyAction = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the menu with its top left at sPosition, in window client space,
// the keys in it.  The component that had them gets them back when it
// closes.  False if the menu has no items.
//
//////////////////////////////////////////////////////////////////////////
bool CContextMenu::Open(SInt32Vec2 sPosition)
{
	Ptr<CWindow>	pWindow;
	Ptr<CComponent>	pFocus;
	Ptr<CPopup>		pPopup;

	if (mpMenu->NumItems() == 0)
	{
		return false;
	}

	//Opened again while open, the keys still go back to where they were before it first opened.  Opening it closes it
	//first, which gives them back and forgets them.
	pWindow = mpComponent->GetWindow();
	pFocus = IsOpen() ? mpFocusBefore : pWindow->GetFocus()->GetFocussedComponent();
	pPopup = mpMenu->Open(sPosition, mpComponent, true);
	mpFocusBefore = pFocus;
	pPopup->SetOwnerPressKeepsOpen(false);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The component's popup pContent has closed.  If it is the menu the keys
// go back where they were.
//
//////////////////////////////////////////////////////////////////////////
void CContextMenu::PopupClosed(Ptr<CComponent> pContent)
{
	Ptr<CComponent>	pFocus;

	if (pContent != mpMenu)
	{
		return;
	}

	pFocus = mpFocusBefore;
	mpFocusBefore = NULL;
	if (pFocus.IsNotNull())
	{
		mpComponent->GetWindow()->GetFocus()->SetFocus(pFocus);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The right button: the menu opens with its top left at the pointer.
// The event's x and y are relative to the component.
//
//////////////////////////////////////////////////////////////////////////
void CContextMenu::ClickAction(SComponentInputEvent* psEvent)
{
	SInt32Vec2	sPosition;

	sPosition = mpComponent->GetPosition();
	sPosition.x += psEvent->x;
	sPosition.y += psEvent->y;
	Open(sPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// The Menu key or Shift+F10: the menu opens where the focussed component
// says, such as under a text area's caret.
//
//////////////////////////////////////////////////////////////////////////
void CContextMenu::KeyAction(SComponentInputEvent* psEvent)
{
	Ptr<CComponent>	pFocus;
	SInt32Vec2		sPosition;

	pFocus = mpComponent->GetWindow()->GetFocus()->GetFocussedComponent();
	if (pFocus.IsNull() || !pFocus->IsWithin(mpComponent))
	{
		pFocus = mpComponent;
	}
	sPosition = pFocus->GetContextMenuPosition();
	Open(sPosition);
}


Ptr<CMenuContainer> CContextMenu::GetMenu(void) { return mpMenu; }
bool CContextMenu::IsOpen(void) { return mpMenu->IsOpen(); }

