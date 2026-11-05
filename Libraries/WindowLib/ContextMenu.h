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
#ifndef __CONTEXT_MENU_H__
#define __CONTEXT_MENU_H__
#include "StandardLib/Object.h"
#include "ComponentInputEvent.h"
#include "InputAction.h"


class CComponent;
class CMenuContainer;


//A component's context menu (see CComponent::SetContextMenu): its menu, opened by the right button under the pointer,
//or by the Menu key or Shift+F10 where the focussed component says (GetContextMenuPosition), as Windows opens them.
//
//The right button focusses the component under the pointer first, then goes up from it to the first component with
//a context menu, so a menu on a page is opened over everything in the page that hasn't one of its own.  While the
//menu is open the keys are in it; when it closes they go back to the component that had them.  A press anywhere
//outside the menu, in its component too, closes it.
//
//An item clicked calls the menu's item click event (CMenuContainer::GetItemClickEvent) and the component's
//PopupItemClicked.
class CContextMenu : public CObject
{
CONSTRUCTABLE(CContextMenu);
DESTRUCTABLE(CContextMenu);
protected:
	Ptr<CComponent>		mpComponent;	//Whose menu it is.
	Ptr<CMenuContainer>	mpMenu;
	Ptr<CInputAction>	mpClickAction;	//On the component.
	Ptr<CInputAction>	mpKeyAction;
	Ptr<CComponent>		mpFocusBefore;	//Had the keys when the menu opened, NULL while it is closed.

public:
	void				Init(Ptr<CComponent> pComponent, Ptr<CMenuContainer> pMenu);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				RemoveActions(void);
	bool				Open(SInt32Vec2 sPosition);
	void				PopupClosed(Ptr<CComponent> pContent);

	void				ClickAction(SComponentInputEvent* psEvent);
	void				KeyAction(SComponentInputEvent* psEvent);

	Ptr<CMenuContainer>	GetMenu(void);
	bool				IsOpen(void);
};


#endif // __CONTEXT_MENU_H__

