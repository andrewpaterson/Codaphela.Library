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
// getting ContDrag* wherever the pointer goes until the drag stops.
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
		ToComponentSpace(mpDragComponent, &lx, &ly);
		if (meDrag == FD_Select)
		{
			mpDragComponent->ContDragSelect(lx, ly);
		}
		else if (meDrag == FD_Scroll)
		{
			mpDragComponent->ContDragScroll(lx, ly);
		}
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
// Pointer commands are given x and y relative to the component's top left.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Command(EComponentInputCommand eCommand, bool bInsert)
{
	int				x;
	int				y;
	CChars			szClipboard;
	Ptr<CButton>	pDefault;
	Ptr<CButton>	pCancel;

	if (!mbActive)
	{
		return;
	}

	if (eCommand == CIC_Hover)
	{
		if (mpMouseOverComponent)
		{
			ToComponentSpace(mpMouseOverComponent, &x, &y);
			mpMouseOverComponent->Hover(x, y);
		}
		return;
	}

	if ((eCommand == CIC_WheelUp) || (eCommand == CIC_WheelDown) || (eCommand == CIC_WheelLeft) || (eCommand == CIC_WheelRight))
	{
		Wheel(eCommand);
		return;
	}

	if ((eCommand == CIC_StopDragSelect) || (eCommand == CIC_StopDragScroll))
	{
		StopDrag(eCommand == CIC_StopDragSelect ? FD_Select : FD_Scroll);
		return;
	}

	if ((eCommand == CIC_ActivateComponent) || (eCommand == CIC_ContextMenu) || (eCommand == CIC_StartDragSelect) || (eCommand == CIC_StartDragScroll))
	{
		//A press outside the window, on its title bar say, isn't for any component.
		if (!mbPointerIn || !mpWindow->IsPointIn(msPointer.x, msPointer.y))
		{
			return;
		}
		mpWindow->ClosePopupsOutside(mpMouseOverComponent);
		Request();
	}

	//Escape closes the window's popups, like an open menu, or clicks its cancel button before it clears a selection.
	if (eCommand == CIC_ClearSelection)
	{
		if (mpWindow->NumPopups() > 0)
		{
			mpWindow->CloseAllPopups();
			return;
		}
		pCancel = mpWindow->GetCancelButton();
		if (pCancel)
		{
			pCancel->Click();
			return;
		}
	}

	//Enter clicks the window's default button unless the focussed component wants it.
	if (eCommand == CIC_Enter)
	{
		pDefault = mpWindow->GetDefaultButton();
		if (pDefault && !(mpFocussedComponent && mpFocussedComponent->WantsEnter()))
		{
			pDefault->Enter();
			return;
		}
	}

	if (!mpFocussedComponent)
	{
		return;
	}
	ToComponentSpace(mpFocussedComponent, &x, &y);

	switch (eCommand)
	{
	case CIC_Up:
		mpFocussedComponent->Up();
		break;
	case CIC_Down:
		mpFocussedComponent->Down();
		break;
	case CIC_Left:
		mpFocussedComponent->Left();
		break;
	case CIC_Right:
		mpFocussedComponent->Right();
		break;
	case CIC_WordLeft:
		mpFocussedComponent->WordLeft();
		break;
	case CIC_WordRight:
		mpFocussedComponent->WordRight();
		break;
	case CIC_Home:
		mpFocussedComponent->Home();
		break;
	case CIC_End:
		mpFocussedComponent->End();
		break;
	case CIC_DocumentHome:
		mpFocussedComponent->DocumentHome();
		break;
	case CIC_DocumentEnd:
		mpFocussedComponent->DocumentEnd();
		break;
	case CIC_PageUp:
		mpFocussedComponent->PageUp();
		break;
	case CIC_PageDown:
		mpFocussedComponent->PageDown();
		break;
	case CIC_LineUp:
		mpFocussedComponent->LineUp();
		break;
	case CIC_LineDown:
		mpFocussedComponent->LineDown();
		break;
	case CIC_SelectUp:
		mpFocussedComponent->SelectUp();
		break;
	case CIC_SelectDown:
		mpFocussedComponent->SelectDown();
		break;
	case CIC_SelectLeft:
		mpFocussedComponent->SelectLeft();
		break;
	case CIC_SelectRight:
		mpFocussedComponent->SelectRight();
		break;
	case CIC_SelectWordLeft:
		mpFocussedComponent->SelectWordLeft();
		break;
	case CIC_SelectWordRight:
		mpFocussedComponent->SelectWordRight();
		break;
	case CIC_SelectHome:
		mpFocussedComponent->SelectHome();
		break;
	case CIC_SelectEnd:
		mpFocussedComponent->SelectEnd();
		break;
	case CIC_SelectDocumentHome:
		mpFocussedComponent->SelectDocumentHome();
		break;
	case CIC_SelectDocumentEnd:
		mpFocussedComponent->SelectDocumentEnd();
		break;
	case CIC_SelectPageUp:
		mpFocussedComponent->SelectPageUp();
		break;
	case CIC_SelectPageDown:
		mpFocussedComponent->SelectPageDown();
		break;
	case CIC_SelectWholeWord:
		mpFocussedComponent->SelectWholeWord();
		break;
	case CIC_SelectAll:
		mpFocussedComponent->SelectAll();
		break;
	case CIC_ClearSelection:
		mpFocussedComponent->ClearSelection();
		break;
	case CIC_Space:
		mpFocussedComponent->Space(bInsert);
		break;
	case CIC_Tab:
		mpFocussedComponent->Tab(bInsert);
		break;
	case CIC_BackTab:
		mpFocussedComponent->BackTab();
		break;
	case CIC_Enter:
		mpFocussedComponent->Enter();
		break;
	case CIC_HomeEnter:
		mpFocussedComponent->HomeEnter();
		break;
	case CIC_EndEnter:
		mpFocussedComponent->EndEnter();
		break;
	case CIC_Delete:
		mpFocussedComponent->Delete();
		break;
	case CIC_Backspace:
		mpFocussedComponent->Backspace();
		break;
	case CIC_Cut:
	case CIC_Copy:
	case CIC_Paste:
	case CIC_CutLine:
		//The clipboard is the native window factory's.  Cutting or copying nothing leaves it as it was.
		szClipboard.Init();
		if (eCommand == CIC_Paste)
		{
			mpWindow->GetFactory()->GetClipboardText(&szClipboard);
			mpFocussedComponent->Paste(szClipboard.Text());
		}
		else
		{
			if (eCommand == CIC_Cut)
			{
				mpFocussedComponent->Cut(&szClipboard);
			}
			else if (eCommand == CIC_Copy)
			{
				mpFocussedComponent->Copy(&szClipboard);
			}
			else
			{
				mpFocussedComponent->CutLine(&szClipboard);
			}
			if (!szClipboard.Empty())
			{
				mpWindow->GetFactory()->SetClipboardText(szClipboard.Text());
			}
		}
		szClipboard.Kill();
		break;
	case CIC_Duplicate:
		mpFocussedComponent->Duplicate();
		break;
	case CIC_DeleteWordRemainingLeft:
		mpFocussedComponent->DeleteWordRemainingLeft();
		break;
	case CIC_DeleteWordRemainingRight:
		mpFocussedComponent->DeleteWordRemainingRight();
		break;
	case CIC_DeleteLine:
		mpFocussedComponent->DeleteLine();
		break;
	case CIC_Undo:
		mpFocussedComponent->Undo();
		break;
	case CIC_StepUndo:
		mpFocussedComponent->StepUndo();
		break;
	case CIC_Redo:
		mpFocussedComponent->Redo();
		break;
	case CIC_StepRedo:
		mpFocussedComponent->StepRedo();
		break;
	case CIC_ActivateComponent:
		mpFocussedComponent->ActivateComponent(x, y);
		break;
	case CIC_ExecuteComponent:
		mpFocussedComponent->ExecuteComponent(x, y);
		break;
	case CIC_ContextMenu:
		mpFocussedComponent->ContextMenu(x, y);
		break;
	case CIC_StartDragSelect:
		StartDrag(FD_Select, x, y);
		break;
	case CIC_StartDragScroll:
		StartDrag(FD_Scroll, x, y);
		break;
	case CIC_StopDragSelect:
	case CIC_StopDragScroll:
	case CIC_Hover:
	case CIC_WheelUp:
	case CIC_WheelDown:
	case CIC_WheelLeft:
	case CIC_WheelRight:
	case CIC_Ignored:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Printable(char c, bool bInsert)
{
	if (mbActive && mpFocussedComponent)
	{
		mpFocussedComponent->Printable(c, bInsert);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The wheel turns the component under the pointer, or the nearest of its
// ancestors that uses it, like the scroll pane it is in.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Wheel(EComponentInputCommand eCommand)
{
	Ptr<CComponent>	pComponent;
	int				iRight;
	int				iDown;

	iRight = 0;
	iDown = 0;
	if (eCommand == CIC_WheelUp)
	{
		iDown = -1;
	}
	else if (eCommand == CIC_WheelDown)
	{
		iDown = 1;
	}
	else if (eCommand == CIC_WheelLeft)
	{
		iRight = -1;
	}
	else if (eCommand == CIC_WheelRight)
	{
		iRight = 1;
	}

	pComponent = mpMouseOverComponent;
	while (pComponent && !pComponent->Wheel(iRight, iDown))
	{
		pComponent = pComponent->GetParent();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Starting a drag while one is going stops the first one.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::StartDrag(EFocusDrag eDrag, int x, int y)
{
	StopDrag(meDrag);

	meDrag = eDrag;
	mpDragComponent = mpFocussedComponent;
	if (eDrag == FD_Select)
	{
		mpDragComponent->StartDragSelect(x, y);
	}
	else if (eDrag == FD_Scroll)
	{
		mpDragComponent->StartDragScroll(x, y);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Only stops a drag of the given kind.
//
//////////////////////////////////////////////////////////////////////////
void CFocus::StopDrag(EFocusDrag eDrag)
{
	Ptr<CComponent>	pComponent;
	int				x;
	int				y;

	if ((meDrag == FD_None) || (meDrag != eDrag))
	{
		return;
	}

	pComponent = mpDragComponent;
	mpDragComponent = NULL;
	meDrag = FD_None;

	ToComponentSpace(pComponent, &x, &y);
	if (eDrag == FD_Select)
	{
		pComponent->StopDragSelect(x, y);
	}
	else if (eDrag == FD_Scroll)
	{
		pComponent->StopDragScroll(x, y);
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

