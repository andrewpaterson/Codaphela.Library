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
//
//A text menu (bText, see CWindow::SetTextMenu) is a window's, for the text it holds: it opens only while the component
//with the keys has a copy action (a text area or field), and each of its items, tagged with a standard command
//(CIC_Copy), is enabled as that component can do it and runs it there, in whatever window it is.
class CMenuItem;
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
	bool				mbText;			//A window's text menu.
	Ptr<CComponent>		mpTextTarget;	//What a text menu's items run on: what had the keys as it opened.

public:
	void				Init(Ptr<CComponent> pComponent, Ptr<CMenuContainer> pMenu, bool bText = false);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				RemoveActions(void);
	bool				Open(SInt32Vec2 sPosition);
	void				PopupClosed(Ptr<CComponent> pContent);

	void				ClickAction(SComponentInputEvent* psEvent);
	void				KeyAction(SComponentInputEvent* psEvent);

	Ptr<CMenuContainer>	GetMenu(void) const;
	bool				IsOpen(void);
	void				TextItemClicked(CMenuItem* pcItem);

protected:
	bool				IsTextTarget(Ptr<CComponent> pFocus);
};


#endif // __CONTEXT_MENU_H__

