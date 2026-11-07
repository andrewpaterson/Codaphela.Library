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
#ifndef __COMBO_BOX_H__
#define __COMBO_BOX_H__
#include "SupportLib/Font.h"
#include "ImageBorderContainer.h"
#include "IconComponent.h"
#include "Label.h"
#include "MenuContainer.h"
#include "WindowTheme.h"
#include "ComponentEvent.h"


//Shows one of a list of items and opens the list below itself to choose another, like IntelliJ's combo box.  It is a
//border drawn from CButtonImages around a face holding the chosen item's text on the left and a chevron on the right.
//The list is a menu, so it looks like the other menus with its style, opened as a popup as wide as the combo box.
//
//Its border lightens under the pointer, like a text field's, and has the focus ring while it has focus or its list is
//open.
//
//Like a button it is hit as a whole.  Pressing it opens the list, or closes it if it is open; space opens it too.
//Choosing an item closes the list and, if it is another item, calls the selection event.  Up and down choose the item
//before or after while the list is closed.  It doesn't want enter, so enter still clicks the window's default button.
//
//It is as wide as its widest item, plus the chevron, unless a desired width is set.
class CComboBox : public CImageBorderContainer
{
CONSTRUCTABLE(CComboBox);
DESTRUCTABLE(CComboBox);
protected:
	Ptr<CMenuStyle>				mpStyle;
	Ptr<CFont>					mpFont;
	Ptr<CGridContainer>			mpContent;  //The text and the chevron.
	Ptr<CLabel>					mpText;
	Ptr<CIconComponent>			mpChevron;
	Ptr<CMenuContainer>			mpMenu;
	CComponentEvent<CComboBox>	mcSelectionChanged;
	int32						miSelected;  //-1 if no item is chosen.
	int32						miChevronGap;  //The least space between the widest item and the chevron.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	size				AddItem(const char* szText);
	void				RemoveAllItems(void);
	size				NumItems(void);
	char*				GetItemText(size uiIndex);
	void				Select(int32 iIndex);
	int32				GetSelected(void);
	char*				GetSelectedText(void);

	void				Open(void);
	void				Close(void);
	bool				IsOpen(void);
	Ptr<CMenuContainer>	GetMenu(void);
	Ptr<CLabel>			GetTextLabel(void);

	CComponentEvent<CComboBox>*	GetSelectionEvent(void);

	bool				WantsEnter(void) override;
	void				PopupClosed(Ptr<CComponent> pContent) override;
	void				PopupItemClicked(Ptr<CComponent> pContent, Ptr<CComponent> pItem) override;

	void				FocusOrMouseOverChanged(void) override;
	void				ActivateComponent(int x, int y);
	void				Up(void);
	void				Down(void);

protected:
	void				Choose(int32 iIndex);
	void				UpdateLook(void);
	SInt32Vec2			GetContentSize(void) override;
};


#endif // __COMBO_BOX_H__

