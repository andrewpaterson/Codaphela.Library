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
#ifndef __DOCK_TITLE_BAR_H__
#define __DOCK_TITLE_BAR_H__
#include "GridContainer.h"
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "IconComponent.h"
#include "Label.h"
#include "DoubleClick.h"
#include "ComponentEvent.h"


#define DOCK_TITLE_BAR_HEIGHT	26


enum EDockTitleButton
{
	DTB_Menu,	//The window position menu.
	DTB_Pin,	//Auto hide.
	DTB_Close,
	DTB_NumButtons,
	DTB_None = DTB_NumButtons,
};


class CDockManager;


//A tool window's title bar, as Visual Studio draws it: the title on the left and the window position menu's chevron,
//the push pin and the close cross on the right, highlighted while its window is active.  It is hit as a whole.
//Pressing a button calls its event; pulling the bar a few pixels while pressed calls the drag event, once, whose
//listener takes over the rest of the drag (CFocus::HandOverDrag); a double click calls the double click event.
class CDockTitleBar : public CGridContainer
{
CONSTRUCTABLE(CDockTitleBar);
DESTRUCTABLE(CDockTitleBar);
protected:
	Ptr<CDockManager>		mpManager;
	Ptr<CClearCanvasDraw>	mpBackgroundDraw;
	Ptr<CLabel>				mpTitle;
	Ptr<CIconComponent>		mapButtons[DTB_NumButtons];
	bool					mabShown[DTB_NumButtons];
	bool					mbActive;
	bool					mbPinned;
	bool					mbPressed;  //Pressed off the buttons, and not dragged yet.
	SInt32Vec2				msPressed;  //Where, relative to the bar.
	EDockTitleButton		mePressedButton;
	SDoubleClick			msDoubleClick;
	CComponentEvent<CDockTitleBar>	mcDragged;
	CComponentEvent<CDockTitleBar>	mcDoubleClicked;
	CComponentEvent<CDockTitleBar>	macButtonPressed[DTB_NumButtons];

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, const char* szTitle, bool bMenu, bool bPin, bool bClose);
	void				Class(void) override;
	void				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetTitle(const char* szTitle);
	char*				GetTitle(void);
	void				SetActive(bool bActive);
	bool				IsActive(void);
	void				SetPinned(bool bPinned);
	bool				IsPinned(void);
	void				ShowButton(EDockTitleButton eButton, bool bShow);
	Ptr<CIconComponent>	GetButton(EDockTitleButton eButton);

	void				ActivateComponent(int x, int y);
	void				ExecuteComponent(int x, int y);
	void				ContDragSelect(int x, int y);
	void				StopDragSelect(int x, int y);
	EDockTitleButton	FindButton(int x, int y);

	CComponentEvent<CDockTitleBar>*	GetDragEvent(void);
	CComponentEvent<CDockTitleBar>*	GetDoubleClickEvent(void);
	CComponentEvent<CDockTitleBar>*	GetButtonEvent(EDockTitleButton eButton);

protected:
	void				PlaceButtons(void);
	void				UpdateLook(void);
};


#endif // __DOCK_TITLE_BAR_H__

