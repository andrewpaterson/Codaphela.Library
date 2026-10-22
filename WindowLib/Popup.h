#ifndef __POPUP_H__
#define __POPUP_H__
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
#include "SupportLib/ColourARGB32.h"
#include "Container.h"
#include "Insets.h"


//Holds one component, like a menu, over everything else in the window.  CWindow::OpenPopup makes them; the window's
//canvas draws them after its container, so they are drawn over it, and they are hit before it.
//
//The popup is the size its content wants plus its insets, with its top left where it was opened unless that would put
//it past the window's right or bottom edge, then it is moved back in.  It fills its insets with its background and
//draws a one pixel border around its outside, if it has a border colour.
//
//The owner is the component that opened it.  A press on the owner, or in any popup, leaves the popups open; a press
//anywhere else closes them.  The owner is told when its popup closes.
class CPopup : public CContainer
{
CONSTRUCTABLE(CPopup);
DESTRUCTABLE(CPopup);
protected:
	Ptr<CComponent>		mpContent;
	Ptr<CComponent>		mpOwner;
	SInt32Vec2			msOpenPosition;  //In window client space.
	SInsets				msInsets;
	ARGB32				msBackground;
	ARGB32				msBorder;
	bool				mbBorder;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CComponent> pContent, SInt32Vec2 sPosition, Ptr<CComponent> pOwner);
	void				Class(void) override;
	void 				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CComponent>		GetContent(void);
	Ptr<CComponent>		GetOwner(void);
	void				SetLook(ARGB32 sBackground, ARGB32 sBorder, SInsets* psInsets);
	void				SetLook(ARGB32 sBackground, SInsets* psInsets);

	SInt32Vec2			GetDesiredSize(void) override;
	void				SetRequiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool				Draw(void) override;
};


#endif // __POPUP_H__

