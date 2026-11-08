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
#ifndef __TOOL_TIPS_H__
#define __TOOL_TIPS_H__
#include "StandardLib/Object.h"
#include "Label.h"
#include "WindowTheme.h"


#define TOOL_TIPS_DELAY		700		//Milliseconds the pointer rests on a component before its tool tip shows.


class CWindow;


//A window's tool tips: the pointer resting on a component (CComponent::GetToolTip, or its nearest parent's when it
//has none) for a while shows the tip under the pointer, in a popup the pointer passes through.  The tip goes when the
//pointer leaves the component; a press, key or the wheel takes it away until the pointer moves onto another component.
//A window shows none until it is given one (CWindow::SetToolTips).  Times are GetDebugTickCount's, given in so that
//they can be tested.
class CToolTips : public CObject
{
CONSTRUCTABLE(CToolTips);
DESTRUCTABLE(CToolTips);
protected:
	Ptr<CWindow>		mpWindow;
	Ptr<CWindowTheme>	mpTheme;
	Ptr<CLabel>			mpLabel;  //The tip, in a popup while it shows.
	Ptr<CComponent>		mpTipped;  //Under the pointer with a tip waited for or shown, NULL for none.
	int64				miRestTime;  //When the pointer last moved on mpTipped.
	int32				miDelay;
	bool				mbShown;
	bool				mbDismissed;  //Taken away; not shown again until the pointer is on another component.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetDelay(int32 iMilliseconds);
	int32				GetDelay(void);

	void				PointerMoved(Ptr<CComponent> pOver, int64 iNow);
	void				PointerLeft(void);
	void				Dismiss(void);
	void				Forget(Ptr<CComponent> pComponent);

	int64				GetWait(int64 iNow);
	bool				ShowIfDue(int64 iNow);
	bool				IsShown(void);
	char*				GetText(void);
	Ptr<CComponent>		GetTipped(void);

protected:
	Ptr<CComponent>		FindTipped(Ptr<CComponent> pOver);
	void				Hide(void);
};


#endif // __TOOL_TIPS_H__

