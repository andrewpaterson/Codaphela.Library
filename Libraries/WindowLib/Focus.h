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
#ifndef __FOCUS_H__
#define __FOCUS_H__
#include "SupportLib/Int32Rect.h"
#include "SupportLib/ColourARGB32.h"
#include "ComponentInputCommand.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "ComponentInputEvent.h"
#include "ComponentEvent.h"


enum EFocusDrag
{
	FD_None,
	FD_Select,
	FD_Scroll,
};


class CWindow;
class CComponent;
struct SWindowInputEvent;
class CComponentInput;
//How long a caret is shown, then hidden, for (IntelliJ's default), see CFocus::SetCaretBlinkPeriod.
#define FOCUS_CARET_BLINK_PERIOD	500


class CFocus : public CObject
{
CONSTRUCTABLE(CFocus);
DESTRUCTABLE(CFocus);
protected:
	Ptr<CComponent>			mpMouseOverComponent;
	Ptr<CComponent>			mpFocussedComponent;
	Ptr<CComponent>			mpDragComponent;  //Gets ContDrag* until the drag stops, even once the pointer has left it.
	Ptr<CWindow>			mpWindow;
	SInt32Vec2				msPointer;  //In window client space.
	bool					mbPointerIn;
	bool					mbActive;  //An inactive window remembers its focussed component but sends it no commands.
	EFocusDrag				meDrag;
	CComponentEvent<CFocus>	mcFocusChanged;  //The focussed component changed, see GetFocusChangeEvent.
	int32					miCaretBlinkPeriod;  //Milliseconds the caret is shown, then hidden, for; 0 for always shown.
	int64					miCaretBlinkTime;  //When the caret was last shown or hidden (GetDebugTickCount).
	bool					mbCaretOn;  //The focussed component's caret is shown.

public:
	void				Init(Ptr<CWindow> pWindow);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				PointerMoved(int x, int y);
	void				PointerLeft(void);
	void				Refresh(void);
	void				Request(void);
	void				SetFocus(Ptr<CComponent> pComponent);
	void				Input(SWindowInputEvent* psWindowEvent, CComponentInput* pcComponentInput);
	void				Command(EComponentInputCommand eCommand, bool bInsert);
	void				Printable(char c, bool bInsert);
	void				Activate(void);
	void				Deactivate(void);
	void				Forget(Ptr<CComponent> pComponent);

	Ptr<CComponent>		GetFocussedComponent(void);
	Ptr<CComponent>		GetMouseOverComponent(void);
	SInt32Vec2			GetPointer(void);
	bool				IsPointerIn(void);
	bool				IsActive(void);

	//A focussed component can start a drag itself, as a scroll bar does when its thumb is pressed, so it gets the
	//pointer's moves until it stops the drag.
	void						StartDrag(EFocusDrag eDrag, int x, int y);
	void						StopDrag(EFocusDrag eDrag);
	void						HandOverDrag(Ptr<CComponent> pComponent);
	Ptr<CComponent>				GetDragComponent(void);
	EFocusDrag					GetDrag(void);
	CComponentEvent<CFocus>*	GetFocusChangeEvent(void);

	//The focussed component's caret blinks, as IntelliJ's: shown and hidden every period, shown again whenever it
	//moves, its text is edited or the focus changes (ResetCaretBlink).  The window draws it over what it paints.
	bool				GetCaret(CInt32Rect* pcDest, ARGB32* psColour);
	void				SetCaretBlinkPeriod(int32 iMilliseconds);
	void				ResetCaretBlink(void);
	bool				IsCaretOn(void);
	int64				GetCaretBlinkWait(int64 iNow);
	bool				BlinkCaret(int64 iNow);

protected:
	void				SetMouseOver(Ptr<CComponent> pComponent);
	void				DispatchEvents(SComponentInputEvent* asEvents, size uiNumEvents);
	bool				DispatchEvent(SComponentInputEvent* psEvent);
	bool				Dispatch(Ptr<CComponent> pComponent, SComponentInputEvent* psEvent);
	bool				Deliver(Ptr<CComponent> pComponent, SComponentInputEvent* psEvent);
	bool				DispatchUnhandledCommand(SComponentInputEvent* psEvent);
	bool				FocusClick(void);
	void				InitEvent(SComponentInputEvent* psEvent, EComponentInputEventType eType, bool bInsert);
	bool				IsClick(SComponentInputEvent* psEvent);
	bool				GetWheel(EComponentInputCommand eCommand, int* piRight, int* piDown);
	void				ToComponentSpace(Ptr<CComponent> pComponent, int* px, int* py);
};


#endif // __FOCUS_H__

