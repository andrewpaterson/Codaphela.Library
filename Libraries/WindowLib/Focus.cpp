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
#include "StandardLib/ClassDefines.h"
#include "BaseLib/Timer.h"
#include "Focus.h"
#include "NativeWindowFactory.h"
#include "Window.h"
#include "Button.h"
#include "ComponentInput.h"
#include "ToolTips.h"


//What a drag's commands tell the component dragging: it started, the pointer moved, it stopped.
enum EFocusDragStage
{
	FDS_Start,
	FDS_Continue,
	FDS_Stop,
	NUM_FOCUS_DRAG_STAGES
};


//The commands of each stage of a select drag, then of a scroll drag.
static EComponentInputCommand	gaaeFocusDragCommands[2][NUM_FOCUS_DRAG_STAGES] =
{
	{ CIC_StartDragSelect, CIC_ContDragSelect, CIC_StopDragSelect },
	{ CIC_StartDragScroll, CIC_ContDragScroll, CIC_StopDragScroll },
};


//////////////////////////////////////////////////////////////////////////
//
// eDrag is FD_Select or FD_Scroll.
//
//////////////////////////////////////////////////////////////////////////
static EComponentInputCommand GetDragCommand(EFocusDrag eDrag, EFocusDragStage eStage)
{
	return gaaeFocusDragCommands[(eDrag == FD_Select) ? 0 : 1][eStage];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Init(Ptr<CWindow> pWindow)
{
	mpWindow = pWindow;
	mpMouseOverComponent = NULL;
	mpFocussedComponent = NULL;
	mpDragComponent = NULL;
	mpKeyComponent = NULL;
	msPointer.Init(0, 0);
	mbPointerIn = false;
	mbActive = true;
	meDrag = FD_None;
	mcFocusChanged.Init();
	miCaretBlinkPeriod = FOCUS_CARET_BLINK_PERIOD;
	miCaretBlinkTime = 0;
	miDragRepeatTime = 0;
	mbCaretOn = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Free(void)
{
	mcFocusChanged.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Class(void)
{
	M_Pointer(mpMouseOverComponent);
	M_Pointer(mpFocussedComponent);
	M_Pointer(mpDragComponent);
	M_Pointer(mpKeyComponent);
	M_Pointer(mpWindow);
	U_2Int32(msPointer);
	U_Bool(mbPointerIn);
	U_Bool(mbActive);
	U_Enum(meDrag);
	U_Unknown(CComponentEvent<CFocus>, mcFocusChanged);
	U_Int32(miCaretBlinkPeriod);
	U_Int64(miCaretBlinkTime);
	U_Int64(miDragRepeatTime);
	U_Bool(mbCaretOn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are in window client space.  The component under the pointer is
// told where in it the pointer is.  A component that started a drag keeps
// getting CIC_ContDrag* wherever the pointer goes until the drag stops.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::PointerMoved(int x, int y)
{
	int		lx;
	int		ly;

	msPointer.Init(x, y);
	mbPointerIn = true;
	Refresh();

	if (mpMouseOverComponent)
	{
		ToComponentSpace(mpMouseOverComponent, &lx, &ly);
		mpMouseOverComponent->MovedOver(lx, ly);
	}

	if (mpDragComponent)
	{
		ContinueDrag();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The component dragging is told where the pointer is.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::ContinueDrag(void)
{
	SComponentInputEvent	sEvent;

	miDragRepeatTime = GetDebugTickCount();
	InitEvent(&sEvent, CIET_Command, true);
	sEvent.eCommand = GetDragCommand(meDrag, FDS_Continue);
	Deliver(mpDragComponent, &sEvent);

	//A drag is a command like any other: selecting and scrolling move what is laid out, unless the dragged
	//component's action keeps the layout.
	if (!sEvent.bLayoutKept)
	{
		mpWindow->InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How long until a select drag is told where the pointer is again though
// it hasn't moved (RepeatDrag): while the pointer is outside the view the
// component dragging is scrolled in, so a selection dragged past the
// view's edge and held there goes on scrolling, as editors do.  -1 if
// there is no such drag.
//
//////////////////////////////////////////////////////////////////////////
int64 CFocus::GetDragRepeatWait(int64 iNow)
{
	CInt32Rect	cView;
	int64		iWait;

	if (mpDragComponent.IsNull() || (meDrag != FD_Select) || !mpDragComponent->GetScrollView(&cView) || cView.ContainsPoint(msPointer.x, msPointer.y))
	{
		return -1;
	}
	iWait = miDragRepeatTime + FOCUS_DRAG_REPEAT_PERIOD - iNow;
	return (iWait > 0) ? iWait : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Tells the component dragging where the pointer is again if it is time
// to (GetDragRepeatWait).  True if it did.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::RepeatDrag(int64 iNow)
{
	if (GetDragRepeatWait(iNow) != 0)
	{
		return false;
	}
	ContinueDrag();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::PointerLeft(void)
{
	mbPointerIn = false;
	Refresh();
}


//////////////////////////////////////////////////////////////////////////
//
// Hit tests the last pointer position again.  Called after a layout in
// case components moved under a pointer that didn't.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Refresh(void)
{
	Ptr<CComponent>	pComponent;

	if (mbPointerIn)
	{
		pComponent = mpWindow->FindComponentAt(msPointer.x, msPointer.y);
	}
	SetMouseOver(pComponent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::SetMouseOver(Ptr<CComponent> pComponent)
{
	Ptr<CComponent>	pLeft;

	if (mpMouseOverComponent != pComponent)
	{
		pLeft = mpMouseOverComponent;
		mpMouseOverComponent = pComponent;
		if (pLeft)
		{
			pLeft->MovedOutof();
		}
		if (mpMouseOverComponent)
		{
			mpMouseOverComponent->MovedInto();
		}
		mpWindow->UpdateCursor();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Focus goes to the nearest component under the pointer that can take it.
// If there is none, as over the background, nothing has focus.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Request(void)
{
	Ptr<CComponent>	pComponent;

	pComponent = mpMouseOverComponent;
	while (pComponent && !pComponent->CanGetFocus())
	{
		pComponent = pComponent->GetParent();
	}
	SetFocus(pComponent);
}


//////////////////////////////////////////////////////////////////////////
//
// Gives pComponent focus, or takes focus from everything if it is NULL,
// as a menu does to take the keys once it opens.  The component losing
// focus is told after it has gone, so it doesn't still seem focussed.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::SetFocus(Ptr<CComponent> pComponent)
{
	Ptr<CComponent>	pLost;

	if (pComponent != mpFocussedComponent)
	{
		pLost = mpFocussedComponent;
		mpFocussedComponent = pComponent;
		if (pLost)
		{
			pLost->FocusLost();
		}
		if (mpFocussedComponent)
		{
			mpFocussedComponent->FocusGained();
		}
		ResetCaretBlink();
		mcFocusChanged.Call(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Translates a queued event (see CComponentInput::Translate) and
// dispatches the events it means.  Returns true if the window needn't be
// laid out again after it: a key typing a character
// (CComponentInput::IsTyping), an event taken by actions that keep the
// layout, or a key's release nothing took.  A click that moved the focus
// lays it out.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::Input(SWindowInputEvent* psWindowEvent, CComponentInput* pcComponentInput)
{
	SComponentInputEvent	asEvents[COMPONENT_INPUT_MAX_EVENTS];
	SComponentInputEvent*	psTaken;
	size					uiNumEvents;
	Ptr<CComponent>			pFocussed;
	const char*				szContext;

	if (!mbActive)
	{
		return false;
	}
	pFocussed = mpFocussedComponent;
	mpKeyComponent = FindKeyComponent(pcComponentInput, &szContext);
	uiNumEvents = pcComponentInput->Translate(psWindowEvent, asEvents, szContext);
	psTaken = DispatchEvents(asEvents, uiNumEvents);
	mpKeyComponent = NULL;
	if (mpFocussedComponent != pFocussed)
	{
		return false;
	}
	if (pcComponentInput->IsTyping(psWindowEvent, asEvents, uiNumEvents))
	{
		return true;
	}
	if (psTaken)
	{
		return psTaken->bLayoutKept;
	}
	return (uiNumEvents == 1) && (asEvents[0].eType == CIET_KeyUp);
}


//////////////////////////////////////////////////////////////////////////
//
// Gives each event, in order, to DispatchEvent, and returns the one taken
// (NULL for none).  Once one is taken the later ones aren't given, so a
// key taken by its key event isn't also a command.  If any is a click the component under the pointer is focussed
// first, so all of them go to it; a click outside the window, on its
// title bar say, isn't for any component so none are given.
//
//////////////////////////////////////////////////////////////////////////
SComponentInputEvent* CFocus::DispatchEvents(SComponentInputEvent* asEvents, size uiNumEvents)
{
	size	i;

	for (i = 0; i < uiNumEvents; i++)
	{
		if (IsClick(&asEvents[i]))
		{
			if (!FocusClick())
			{
				return NULL;
			}
			break;
		}
	}

	for (i = 0; i < uiNumEvents; i++)
	{
		if (DispatchEvent(&asEvents[i]))
		{
			return &asEvents[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The window first, then the input actions.  Escape closes the window's
// popups or clicks its cancel button, and enter clicks its default button
// unless the focussed component wants it.  A drag's start goes to the
// focussed component and the rest of it to whatever started it.
// Otherwise the event goes to the focussed component, or for movement and
// the wheel the component under the pointer, then up through its parents
// to the first with an action that matches; all its matching actions are
// called.  Returns true if the event was taken.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::DispatchEvent(SComponentInputEvent* psEvent)
{
	Ptr<CButton>	pDefault;
	Ptr<CButton>	pCancel;
	int				x;
	int				y;

	if (psEvent->eType == CIET_Command)
	{
		switch (psEvent->eCommand)
		{
		case CIC_Ignored:
			return false;
		case CIC_Hover:
			if (mpMouseOverComponent)
			{
				return Deliver(mpMouseOverComponent, psEvent);
			}
			return false;
		case CIC_WheelUp:
		case CIC_WheelDown:
		case CIC_WheelLeft:
		case CIC_WheelRight:
			GetWheel(psEvent->eCommand, &psEvent->iRight, &psEvent->iDown);
			psEvent->eType = CIET_Wheel;
			psEvent->eCommand = CIC_Ignored;
			return Dispatch(mpMouseOverComponent, psEvent);
		case CIC_StopDragSelect:
			StopDrag(FD_Select);
			return true;
		case CIC_StopDragScroll:
			StopDrag(FD_Scroll);
			return true;
		case CIC_ClearSelection:
			if (mpWindow->NumPopups() > 0)
			{
				mpWindow->CloseAllPopups();
				return true;
			}
			pCancel = mpWindow->GetCancelButton();
			if (pCancel && !(mpFocussedComponent && mpFocussedComponent->WantsEscape()))
			{
				pCancel->Click();
				return true;
			}
			break;
		case CIC_Enter:
			pDefault = mpWindow->GetDefaultButton();
			if (pDefault && !(mpFocussedComponent && mpFocussedComponent->WantsEnter()))
			{
				pDefault->KeyClick();
				return true;
			}
			break;
		default:
			break;
		}

		if ((psEvent->eCommand == CIC_StartDragSelect) || (psEvent->eCommand == CIC_StartDragScroll))
		{
			if (!mpFocussedComponent)
			{
				return false;
			}
			ToComponentSpace(mpFocussedComponent, &x, &y);
			StartDrag(psEvent->eCommand == CIC_StartDragSelect ? FD_Select : FD_Scroll, x, y);
			return true;
		}
	}

	if ((psEvent->eType == CIET_Movement) || (psEvent->eType == CIET_Wheel))
	{
		return Dispatch(mpMouseOverComponent, psEvent);
	}

	//A key bound in the context of the component under the pointer is for it (see FindKeyComponent).
	if (Dispatch(((psEvent->szContext != NULL) && mpKeyComponent.IsNotNull()) ? mpKeyComponent : mpFocussedComponent, psEvent))
	{
		return true;
	}
	if (psEvent->eType == CIET_Command)
	{
		return DispatchUnhandledCommand(psEvent);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A command nothing focussed took, Control+N say, goes to the window's
// content (with nothing focussed, or from a popup, it wasn't asked), then
// to the window a tool window floats over, so the application's commands
// work wherever the keys are.  Keys bound to an application's command
// (CIC_Command) run it, from the window's commands or its owner's.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::DispatchUnhandledCommand(SComponentInputEvent* psEvent)
{
	Ptr<CComponent>	pContent;
	Ptr<CWindow>	pOwner;

	if (psEvent->eCommand == CIC_Command)
	{
		return RunCommand(mpWindow, psEvent->szName, psEvent->szContext);
	}
	if (((psEvent->eCommand == CIC_Tab) || (psEvent->eCommand == CIC_BackTab)) && mpWindow->IsModal())
	{
		return MoveFocus(psEvent->eCommand == CIC_Tab);
	}

	pContent = mpWindow->GetContainer();
	if (pContent.IsNotNull() && (mpFocussedComponent.IsNull() || !mpFocussedComponent->IsWithin(pContent)))
	{
		if (Dispatch(pContent, psEvent))
		{
			return true;
		}
	}
	pOwner = mpWindow->GetOwner();
	if (pOwner.IsNotNull())
	{
		pContent = pOwner->GetContainer();
		if (pContent.IsNotNull())
		{
			return pOwner->GetFocus()->Dispatch(pContent, psEvent);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Tab, in a dialog, with nothing focussed wanting it: the focus goes to
// the next or previous component that can be tabbed to, round from the
// last to the first.  False if there's nowhere for it to go.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::MoveFocus(bool bForward)
{
	SFocusTabStops	sStops;
	Ptr<CComponent>	pContent;
	Ptr<CComponent>	pTarget;

	pContent = mpWindow->GetContainer();
	if (pContent.IsNull())
	{
		return false;
	}
	sStops.bPassed = false;
	FindTabStops(pContent, &sStops);
	if (bForward)
	{
		pTarget = sStops.pAfter.IsNotNull() ? sStops.pAfter : sStops.pFirst;
	}
	else
	{
		pTarget = (sStops.bPassed && sStops.pBefore.IsNotNull()) ? sStops.pBefore : sStops.pLast;
	}
	if (pTarget.IsNull())
	{
		return false;
	}
	SetFocus(pTarget);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// pComponent then what's inside it.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::FindTabStops(Ptr<CComponent> pComponent, SFocusTabStops* psStops)
{
	size	i;

	if (!pComponent->IsDrawn())
	{
		return;
	}
	if (pComponent->IsTabStop())
	{
		if (psStops->pFirst.IsNull())
		{
			psStops->pFirst = pComponent;
		}
		if (psStops->bPassed && psStops->pAfter.IsNull())
		{
			psStops->pAfter = pComponent;
		}
		if (pComponent == mpFocussedComponent)
		{
			psStops->bPassed = true;
			psStops->pBefore = psStops->pLast;
		}
		psStops->pLast = pComponent;
	}
	for (i = 0; i < pComponent->NumComponents(); i++)
	{
		FindTabStops(pComponent->GetComponent(i), psStops);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The command bound to szKeys in szContext (NULL for the global keys) in
// pWindow's commands, or its owner's (a dialog's or tool window's), runs.
// False if there is none or it can't run now.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::RunCommand(Ptr<CWindow> pWindow, char* szKeys, const char* szContext)
{
	Ptr<CCommandSet>	pCommands;

	while (pWindow.IsNotNull())
	{
		pCommands = pWindow->GetCommands();
		if (pCommands.IsNotNull() && pCommands->FindKeys(szKeys, szContext).IsNotNull())
		{
			return pCommands->RunKeys(szKeys, szContext);
		}
		pWindow = pWindow->GetOwner();
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Focusses the component under the pointer for a click, closing popups it
// isn't in.  Returns false for a click outside the window, on its title
// bar say, which isn't for any component.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::FocusClick(void)
{
	if (!mbPointerIn || !mpWindow->IsPointIn(msPointer.x, msPointer.y))
	{
		return false;
	}
	mpWindow->ClosePopupsOutside(mpMouseOverComponent);
	Request();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns true if a component from pComponent up took the event.  The
// event's x and y are the pointer relative to the component given it.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::Dispatch(Ptr<CComponent> pComponent, SComponentInputEvent* psEvent)
{
	while (pComponent)
	{
		if (Deliver(pComponent, psEvent))
		{
			return true;
		}
		pComponent = pComponent->GetParent();
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// To pComponent's actions only, the pointer relative to it.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::Deliver(Ptr<CComponent> pComponent, SComponentInputEvent* psEvent)
{
	ToComponentSpace(pComponent, &psEvent->x, &psEvent->y);
	return pComponent->CallInputActions(psEvent);
}


//////////////////////////////////////////////////////////////////////////
//
// An event the focus makes itself (a drag's, a command run by name): the
// modifier keys held are the window's component input's, if it has one,
// as they are on the events that come from it.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::InitEvent(SComponentInputEvent* psEvent, EComponentInputEventType eType, bool bInsert)
{
	CComponentInput*	pcInput;

	memset(psEvent, 0, sizeof(SComponentInputEvent));
	psEvent->eType = eType;
	psEvent->eCommand = CIC_Ignored;
	psEvent->bInsert = bInsert;

	pcInput = mpWindow.IsNotNull() ? mpWindow->GetComponentInput() : NULL;
	if (pcInput)
	{
		psEvent->bShift = pcInput->IsShift();
		psEvent->bControl = pcInput->IsControl();
		psEvent->bAlt = pcInput->IsAlt();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer button press that is for the component under the pointer.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::IsClick(SComponentInputEvent* psEvent)
{
	if (psEvent->eType != CIET_Command)
	{
		return false;
	}
	return (psEvent->eCommand == CIC_ActivateComponent) || (psEvent->eCommand == CIC_ContextMenu) || (psEvent->eCommand == CIC_StartDragSelect) || (psEvent->eCommand == CIC_StartDragBox) || (psEvent->eCommand == CIC_StartDragScroll);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if the command isn't the wheel's.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::GetWheel(EComponentInputCommand eCommand, int* piRight, int* piDown)
{
	*piRight = 0;
	*piDown = 0;
	switch (eCommand)
	{
	case CIC_WheelUp:
		*piDown = -1;
		return true;
	case CIC_WheelDown:
		*piDown = 1;
		return true;
	case CIC_WheelLeft:
		*piRight = -1;
		return true;
	case CIC_WheelRight:
		*piRight = 1;
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A command not from the input, as a test gives one.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::Command(EComponentInputCommand eCommand, bool bInsert)
{
	SComponentInputEvent	sEvent;
	SComponentInputEvent*	psTaken;
	Ptr<CComponent>			pFocussed;

	if (!mbActive)
	{
		return false;
	}
	pFocussed = mpFocussedComponent;
	InitEvent(&sEvent, CIET_Command, bInsert);
	sEvent.eCommand = eCommand;
	psTaken = DispatchEvents(&sEvent, 1);
	return (psTaken != NULL) && psTaken->bLayoutKept && (mpFocussedComponent == pFocussed);
}


//////////////////////////////////////////////////////////////////////////
//
// Keys bound to an application's command ("Control+Shift+N"), not from
// the input, as a test gives them: what is focussed is asked first, then
// they run the command (DispatchUnhandledCommand).  False if nothing
// took them.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::Keys(const char* szKeys)
{
	SComponentInputEvent	sEvent;

	if (!mbActive)
	{
		return false;
	}
	InitEvent(&sEvent, CIET_Command, true);
	sEvent.eCommand = CIC_Command;
	sEvent.szName = (char*)szKeys;
	return DispatchEvents(&sEvent, 1) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Printable(char c, bool bInsert)
{
	SComponentInputEvent	sEvent;

	if (mbActive)
	{
		InitEvent(&sEvent, CIET_Printable, bInsert);
		sEvent.cPrintable = c;
		DispatchEvents(&sEvent, 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The focussed component starts dragging, as told by CIC_StartDragSelect
// or CIC_StartDragScroll at x, y relative to it.  It is given the rest of
// the drag, CIC_ContDrag* as the pointer moves and CIC_StopDrag* when it
// stops, wherever the pointer goes.  Starting a drag while one is going
// stops the first one.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::StartDrag(EFocusDrag eDrag, int x, int y)
{
	SComponentInputEvent	sEvent;

	StopDrag(meDrag);
	if (!mpFocussedComponent)
	{
		return;
	}

	meDrag = eDrag;
	mpDragComponent = mpFocussedComponent;
	InitEvent(&sEvent, CIET_Command, true);
	sEvent.eCommand = GetDragCommand(eDrag, FDS_Start);
	sEvent.x = x;
	sEvent.y = y;
	mpDragComponent->CallInputActions(&sEvent);
	mpWindow->UpdateCursor();
}


//////////////////////////////////////////////////////////////////////////
//
// Only stops a drag of the given kind.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::StopDrag(EFocusDrag eDrag)
{
	Ptr<CComponent>			pComponent;
	SComponentInputEvent	sEvent;

	if ((meDrag == FD_None) || (meDrag != eDrag))
	{
		return;
	}

	pComponent = mpDragComponent;
	mpDragComponent = NULL;
	meDrag = FD_None;

	InitEvent(&sEvent, CIET_Command, true);
	sEvent.eCommand = GetDragCommand(eDrag, FDS_Stop);
	Deliver(pComponent, &sEvent);
	mpWindow->UpdateCursor();
}


//////////////////////////////////////////////////////////////////////////
//
// The rest of the drag going on goes to pComponent instead of the
// component that started it, which isn't told.  pComponent can be outside
// the window's tree, a dock manager's drag tracker say, so the drag
// survives the component that started it being removed.  The tracker
// should be focussed too so the release that ends the drag reaches it.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::HandOverDrag(Ptr<CComponent> pComponent)
{
	if (meDrag != FD_None)
	{
		mpDragComponent = pComponent;
		mpWindow->UpdateCursor();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The window has become the active one again.  The component that had
// focus when it was deactivated gets it back.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Activate(void)
{
	if (!mbActive)
	{
		mbActive = true;
		ResetCaretBlink();
		if (mpFocussedComponent)
		{
			mpFocussedComponent->FocusGained();
		}
	}
	Refresh();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Deactivate(void)
{
	if (mbActive)
	{
		StopDrag(meDrag);
		mbActive = false;
		mbPointerIn = false;
		SetMouseOver(NULL);
		if (mpFocussedComponent)
		{
			mpFocussedComponent->FocusLost();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Called before pComponent is removed from its parent.  Nothing in or
// under pComponent may stay hovered, focussed or dragging.  A drag is
// dropped without telling the component.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Forget(Ptr<CComponent> pComponent)
{
	Ptr<CComponent>	pLost;

	if (mpDragComponent && mpDragComponent->IsWithin(pComponent))
	{
		mpDragComponent = NULL;
		meDrag = FD_None;
	}
	if (mpMouseOverComponent && mpMouseOverComponent->IsWithin(pComponent))
	{
		SetMouseOver(NULL);
	}
	if (mpFocussedComponent && mpFocussedComponent->IsWithin(pComponent))
	{
		pLost = mpFocussedComponent;
		mpFocussedComponent = NULL;
		if (mbActive)
		{
			pLost->FocusLost();
		}
	}
	if (mpWindow->GetToolTips().IsNotNull())
	{
		mpWindow->GetToolTips()->Forget(pComponent);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::ToComponentSpace(Ptr<CComponent> pComponent, int* px, int* py)
{
	SInt32Vec2	sPosition;

	sPosition = pComponent->GetPosition();
	*px = msPointer.x - sPosition.x;
	*py = msPointer.y - sPosition.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent>	CFocus::GetFocussedComponent(void) { return mpFocussedComponent; }
Ptr<CComponent>	CFocus::GetDragComponent(void) { return mpDragComponent; }
EFocusDrag		CFocus::GetDrag(void) { return meDrag; }
CComponentEvent<CFocus>*	CFocus::GetFocusChangeEvent(void) { return &mcFocusChanged; }
Ptr<CComponent>	CFocus::GetMouseOverComponent(void) { return mpMouseOverComponent; }


//////////////////////////////////////////////////////////////////////////
//
// The context pComponent's keys are read in: its own, or the nearest of
// its parents' (see CComponent::GetKeyContext); NULL for the global keys.
//
//////////////////////////////////////////////////////////////////////////
const char* CFocus::GetKeyContext(Ptr<CComponent> pComponent)
{
	const char*	szContext;

	for (; pComponent.IsNotNull(); pComponent = pComponent->GetParent())
	{
		szContext = pComponent->GetKeyContext();
		if (szContext != NULL)
		{
			return szContext;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The context a key is read in, in pszContext, and the component it's
// for if that isn't the focussed one.  In a dialog it's the global keys,
// so what's done to a context's keys can't stop it working.  Over a
// component in a typing context (LogiCim's circuit), its keys are read
// wherever the focus is, unless the focussed component takes typing (a
// text field), and the key is for the component under the pointer.
// Otherwise it's the focussed component's context.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CFocus::FindKeyComponent(CComponentInput* pcComponentInput, const char** pszContext)
{
	const char*	szHovered;

	*pszContext = NULL;
	if (mpWindow->IsModal())
	{
		return NULL;
	}
	if (mbPointerIn && mpMouseOverComponent.IsNotNull() && (mpMouseOverComponent != mpFocussedComponent) && !(mpFocussedComponent.IsNotNull() && mpFocussedComponent->TakesTyping()))
	{
		szHovered = GetKeyContext(mpMouseOverComponent);
		if ((szHovered != NULL) && pcComponentInput->IsTypingContext(szHovered))
		{
			*pszContext = szHovered;
			return mpMouseOverComponent;
		}
	}
	*pszContext = GetKeyContext(mpFocussedComponent);
	return NULL;
}
SInt32Vec2		CFocus::GetPointer(void) { return msPointer; }
bool			CFocus::IsPointerIn(void) { return mbPointerIn; }
bool			CFocus::IsActive(void) { return mbActive; }


//////////////////////////////////////////////////////////////////////////
//
// Where the focussed component's caret is (window space, what of it
// shows) and its colour.  False if it has none, or the window isn't
// active.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::GetCaret(CInt32Rect* pcDest, ARGB32* psColour)
{
	if (!mbActive || mpFocussedComponent.IsNull())
	{
		return false;
	}
	return mpFocussedComponent->GetCaretRect(pcDest, psColour);
}


//////////////////////////////////////////////////////////////////////////
//
// 0 keeps the caret shown.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::SetCaretBlinkPeriod(int32 iMilliseconds)
{
	miCaretBlinkPeriod = (iMilliseconds > 0) ? iMilliseconds : 0;
	ResetCaretBlink();
}


//////////////////////////////////////////////////////////////////////////
//
// The caret is shown from now: it moved, its text was edited or the
// focus changed.  A hidden caret is drawn by the next paint (the window
// draws it over what it paints), so only a paint is asked for: nothing
// else changed.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::ResetCaretBlink(void)
{
	if (!mbCaretOn && mpWindow.IsNotNull())
	{
		mpWindow->RequestPaint();
	}
	mbCaretOn = true;
	miCaretBlinkTime = GetDebugTickCount();
}


//////////////////////////////////////////////////////////////////////////
//
// How long until the caret is next shown or hidden, -1 if it isn't
// blinking (none, an inactive window, or a period of 0).
//
//////////////////////////////////////////////////////////////////////////
int64 CFocus::GetCaretBlinkWait(int64 iNow)
{
	CInt32Rect	cCaret;
	ARGB32		sColour;
	int64		iWait;

	if ((miCaretBlinkPeriod == 0) || !GetCaret(&cCaret, &sColour))
	{
		return -1;
	}
	iWait = miCaretBlinkTime + miCaretBlinkPeriod - iNow;
	return (iWait > 0) ? iWait : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Shows or hides the caret if its period is up.  True if it did.
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::BlinkCaret(int64 iNow)
{
	if (GetCaretBlinkWait(iNow) != 0)
	{
		return false;
	}
	mbCaretOn = !mbCaretOn;
	miCaretBlinkTime = iNow;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::IsCaretOn(void) { return mbCaretOn || (miCaretBlinkPeriod == 0); }
