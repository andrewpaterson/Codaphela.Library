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
#ifndef __PROMPT_POPUP_H__
#define __PROMPT_POPUP_H__
#include "ComponentEvent.h"
#include "Dialog.h"
#include "GridContainer.h"
#include "Label.h"
#include "TextField.h"
#include "Popup.h"


//A popup that asks for one line of text, where a dialog would be too much: a caption, a field with what it starts
//with selected, and under them a line saying what is wrong with what was typed (SetProblem), which is empty until
//something is.  IntelliJ IDEA's Go to Line is one.
//
//It is opened at a point of the window (Open) and the field gets the keys.  Enter calls the accepted event: whoever
//hears it reads the text (GetText) and either closes the popup (Close) or says what is wrong with it (SetProblem),
//which leaves it open.  Escape, or a press outside it, closes it; the closed event is called however it went, and
//what had the keys gets them back.
//
//It is an object: make it with OMalloc and keep it in the root, or an object that is, while it is open.
class CPromptPopup : public CObject
{
CONSTRUCTABLE(CPromptPopup);
DESTRUCTABLE(CPromptPopup);
protected:
	Ptr<CWindow>			mpWindow;
	SDialogLook*			mpsLook;  //Not held: whoever made the prompt keeps it.
	Ptr<CGridContainer>		mpPanel;
	Ptr<CLabel>				mpCaption;
	Ptr<CTextField>			mpField;
	Ptr<CLabel>				mpProblem;
	Ptr<CPopup>				mpPopup;  //While it is open.
	Ptr<CComponent>			mpPreviousFocus;  //What had the keys when it was opened, which gets them back.

	CComponentEvent<CPromptPopup>	mcAccepted;
	CComponentEvent<CPromptPopup>	mcClosed;

public:
	void			Init(Ptr<CWindow> pWindow, SDialogLook* psLook);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			Open(SInt32Vec2 sPosition, Ptr<CComponent> pOwner, const char* szCaption, const char* szText);
	void			Close(void);
	bool			IsOpen(void);
	const char*		GetText(void);
	void			SetText(const char* szText);
	void			SetProblem(const char* szProblem);
	const char*		GetProblem(void);
	const char*		GetCaption(void);
	void			Accept(void);

	Ptr<CTextField>	GetField(void) const;
	Ptr<CComponent>	GetPanel(void) const;

	CComponentEvent<CPromptPopup>*	GetAcceptedEvent(void);
	CComponentEvent<CPromptPopup>*	GetClosedEvent(void);

protected:
	void			PopupClosed(void);
	void			TextChanged(void);
};


#endif // __PROMPT_POPUP_H__

