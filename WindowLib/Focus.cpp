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
#include "Focus.h"
#include "NativeWindowFactory.h"
#include "Window.h"
#include "Button.h"
#include "ComponentInput.h"


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
	msPointer.Init(0, 0);
	mbPointerIn = false;
	mbActive = true;
	meDrag = FD_None;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Free(void)
{
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
	M_Pointer(mpWindow);
	U_2Int32(msPointer);
	U_Bool(mbPointerIn);
	U_Bool(mbActive);
	U_Enum(meDrag);
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
	int						lx;
	int						ly;
	SComponentInputEvent	sEvent;

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
		InitEvent(&sEvent, CIET_Command, true);
		sEvent.eCommand = (meDrag == FD_Select) ? CIC_ContDragSelect : CIC_ContDragScroll;
		Deliver(mpDragComponent, &sEvent);
	}
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
	if (mpMouseOverComponent != pComponent)
	{
		if (mpMouseOverComponent)
		{
			mpMouseOverComponent->MovedOutof();
		}
		mpMouseOverComponent = pComponent;
		if (mpMouseOverComponent)
		{
			mpMouseOverComponent->MovedInto();
		}
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
// as a menu does to take the keys once it opens.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::SetFocus(Ptr<CComponent> pComponent)
{
	if (pComponent != mpFocussedComponent)
	{
		if (mpFocussedComponent)
		{
			mpFocussedComponent->FocusLost();
		}
		mpFocussedComponent = pComponent;
		if (mpFocussedComponent)
		{
			mpFocussedComponent->FocusGained();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Translates a queued event (see CComponentInput::Translate) and gives
// each event it means, in order, to DispatchEvent.  Once one is taken the
// later ones aren't given, so a key taken by its key event isn't also a
// command.  A click focusses the component under the pointer before
// anything is given.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Input(SWindowInputEvent* psWindowEvent, CComponentInput* pcComponentInput)
{
	SComponentInputEvent	asEvents[COMPONENT_INPUT_MAX_EVENTS];
	size					uiNumEvents;
	size					i;

	if (!mbActive)
	{
		return;
	}

	uiNumEvents = pcComponentInput->Translate(psWindowEvent, asEvents);
	for (i = 0; i < uiNumEvents; i++)
	{
		if (IsClick(&asEvents[i]))
		{
			if (!FocusClick())
			{
				return;
			}
			break;
		}
	}

	for (i = 0; i < uiNumEvents; i++)
	{
		if (DispatchEvent(&asEvents[i]))
		{
			return;
		}
	}
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
			if (pCancel)
			{
				pCancel->Click();
				return true;
			}
			break;
		case CIC_Enter:
			pDefault = mpWindow->GetDefaultButton();
			if (pDefault && !(mpFocussedComponent && mpFocussedComponent->WantsEnter()))
			{
				pDefault->Enter();
				return true;
			}
			break;
		default:
			break;
		}

		if (IsClick(psEvent))
		{
			if (!FocusClick())
			{
				return true;
			}
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
	return Dispatch(mpFocussedComponent, psEvent);
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
//
//////////////////////////////////////////////////////////////////////////
void CFocus::InitEvent(SComponentInputEvent* psEvent, EComponentInputEventType eType, bool bInsert)
{
	memset(psEvent, 0, sizeof(SComponentInputEvent));
	psEvent->eType = eType;
	psEvent->eCommand = CIC_Ignored;
	psEvent->bInsert = bInsert;
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
	return (psEvent->eCommand == CIC_ActivateComponent) || (psEvent->eCommand == CIC_ContextMenu) || (psEvent->eCommand == CIC_StartDragSelect) || (psEvent->eCommand == CIC_StartDragScroll);
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
void CFocus::Command(EComponentInputCommand eCommand, bool bInsert)
{
	SComponentInputEvent	sEvent;

	if (mbActive)
	{
		InitEvent(&sEvent, CIET_Command, bInsert);
		sEvent.eCommand = eCommand;
		DispatchEvent(&sEvent);
	}
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
		DispatchEvent(&sEvent);
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
	sEvent.eCommand = (eDrag == FD_Select) ? CIC_StartDragSelect : CIC_StartDragScroll;
	sEvent.x = x;
	sEvent.y = y;
	mpDragComponent->CallInputActions(&sEvent);
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
	sEvent.eCommand = (eDrag == FD_Select) ? CIC_StopDragSelect : CIC_StopDragScroll;
	Deliver(pComponent, &sEvent);
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
	if (IsWithin(mpDragComponent, pComponent))
	{
		mpDragComponent = NULL;
		meDrag = FD_None;
	}
	if (IsWithin(mpMouseOverComponent, pComponent))
	{
		SetMouseOver(NULL);
	}
	if (IsWithin(mpFocussedComponent, pComponent))
	{
		if (mbActive)
		{
			mpFocussedComponent->FocusLost();
		}
		mpFocussedComponent = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::IsWithin(Ptr<CComponent> pComponent, Ptr<CComponent> pAncestor)
{
	while (pComponent)
	{
		if (pComponent == pAncestor)
		{
			return true;
		}
		pComponent = pComponent->GetParent();
	}
	return false;
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
Ptr<CComponent>	CFocus::GetMouseOverComponent(void) { return mpMouseOverComponent; }
SInt32Vec2		CFocus::GetPointer(void) { return msPointer; }
bool			CFocus::IsPointerIn(void) { return mbPointerIn; }
bool			CFocus::IsActive(void) { return mbActive; }

