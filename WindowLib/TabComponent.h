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
#ifndef __TAB_COMPONENT_H__
#define __TAB_COMPONENT_H__
#include "GridContainer.h"
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "IconComponent.h"
#include "Label.h"
#include "WindowTheme.h"


class CTabbedContainer;


//A tab on a tabbed container's bar, like IntelliJ IDEA's editor tabs: an optional icon on the left, the name, and an
//optional close cross on the right, flat on the bar with the selected tab underlined.  The tab is lighter under the
//pointer, as is the cross.  Pressing the cross closes the tab; pressing anywhere else, or space or enter while it has
//focus, shows its page.  Releasing the pointer anywhere but the cross tells the tabbed container, for double clicks.
class CTabComponent : public CGridContainer
{
CONSTRUCTABLE(CTabComponent);
DESTRUCTABLE(CTabComponent);
protected:
	Ptr<CTabbedContainer>	mpTabbed;
	Ptr<CWindowTheme>		mpTheme;
	Ptr<CClearCanvasDraw>	mpBackgroundDraw;
	Ptr<CClearCanvasDraw>	mpUnderlineDraw;
	Ptr<CIconComponent>		mpIcon;  //NULL without an icon.
	Ptr<CLabel>				mpName;
	Ptr<CIconComponent>		mpClose;  //NULL if the tab can't be closed.
	bool					mbSelected;
	bool					mbMouseOver;
	bool					mbOverClose;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CTabbedContainer> pTabbed, const char* szName, Ptr<CImageCel> pIcon, bool bClosable);
	void				Class(void) override;
	void 				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	SInt32Vec2			GetDesiredSize(void) override;
	Ptr<CComponent>		FindComponentAt(int x, int y) override;
	void				SetSelected(bool bSelected);
	bool				IsSelected(void);
	bool				IsClosable(void);
	bool				IsMouseOver(void);
	char*				GetText(void);
	void				SetText(const char* szName);
	Ptr<CIconComponent>	GetIcon(void);
	Ptr<CIconComponent>	GetClose(void);

	void				MovedInto(void) override;
	void				MovedOutof(void) override;
	void				MovedOver(int x, int y) override;
	void				CommandAction(SComponentInputEvent* psEvent);

protected:
	bool				IsInClose(int x, int y);
	void				UpdateLook(void);
};


#endif // __TAB_COMPONENT_H__
