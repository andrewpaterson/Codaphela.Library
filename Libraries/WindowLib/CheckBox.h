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
#ifndef __CHECK_BOX_H__
#define __CHECK_BOX_H__
#include "StandardLib/ListenerCall.h"
#include "SupportLib/Font.h"
#include "GridContainer.h"
#include "IconComponent.h"
#include "Label.h"
#include "CheckBoxImages.h"
#include "WindowTheme.h"
#include "CheckBoxListener.h"


//A box that is ticked or not with a caption beside it, like IntelliJ's check box.  It is a grid of the box, an icon
//on the left, and the caption, a label, centred beside it with a gap between them.
//
//Like a button it is hit as a whole so it, not its box or caption, has the pointer over it and takes focus.  Pressing
//and releasing with the pointer over it, or space while it has focus, ticks or unticks it and tells the listeners.  It
//doesn't want enter, so enter still clicks the window's default button.
//
//The box keeps its focus ring while the pointer is over it, see ECheckBoxState.
class CCheckBox : public CGridContainer
{
CONSTRUCTABLE(CCheckBox);
DESTRUCTABLE(CCheckBox);
protected:
	Ptr<CCheckBoxImages>	mpImages;
	Ptr<CIconComponent>		mpBox;
	Ptr<CLabel>				mpCaption;
	CListenerCall			mcListeners;
	ECheckBoxState			meState;
	int32					miGap;
	bool					mbTicked;
	bool					mbMouseOver;
	bool					mbFocussed;
	bool					mbPressed;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szCaption, bool bTicked = false);
	void				Class(void) override;
	void 				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetTicked(bool bTicked);
	bool				IsTicked(void);
	void				Toggle(void);
	void				SetCaption(const char* szCaption);
	char*				GetCaption(void);
	void				SetGap(int32 iGap);
	ECheckBoxState		GetState(void);
	Ptr<CIconComponent>	GetBoxComponent(void);
	Ptr<CLabel>			GetCaptionLabel(void);

	bool				AddListener(CCheckBoxListener* pcListener);
	bool				RemoveListener(CCheckBoxListener* pcListener);

	SInt32Vec2			GetDesiredSize(void) override;
	Ptr<CComponent>		FindComponentAt(int x, int y) override;

	void				FocusGained(void) override;
	void				FocusLost(void) override;
	void				MovedInto(void) override;
	void				MovedOutof(void) override;
	void				CommandAction(SComponentInputEvent* psEvent);
	void				ActivateComponent(int x, int y);
	void				ExecuteComponent(int x, int y);
	void				Space(bool bInsert);

protected:
	void				UpdateState(void);
	void				UpdateLook(void);
	void				UpdateGap(void);
};


#endif // __CHECK_BOX_H__

