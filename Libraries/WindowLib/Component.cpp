/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "StandardLib/Objects.h"
#include "Window.h"
#include "Component.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Init(Ptr<CWindow> pWindow)
{
	PreInit();

	maChildren.Init();
	maInputActions.Init();
	mcActualBounds.Init();
	msDesiredSize.Init(-1, -1);
	meHorizontalAlignment = CSH_Center;
	meVerticalAlignment = CSV_Center;
	mbCanGetFocus = false;
	mpParent = NULL;
	mpWindow = pWindow;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Free(void)
{
	//An object's memory is freed without its destructor running so it unhooks its listener here.
	RemoveFromListenerCalls();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Class(void)
{
		U_Data(CInt32Rect, mcActualBounds);
		U_2Int32(msDesiredSize);
		U_Enum(meHorizontalAlignment);
		U_Enum(meVerticalAlignment);
		U_Bool(mbCanGetFocus);
		M_Pointer(mpParent);
		M_Embedded(maChildren);
	Nes(M_Pointer(mpWindow));
		M_Embedded(maInputActions);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// For AddInputAction, which initialises it.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CInputAction> CComponent::MallocInputAction(void)
{
	return OMallocNoI<CInputAction>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::AddInputAction(Ptr<CInputAction> pAction)
{
	maInputActions.Add(pAction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::RemoveInputAction(Ptr<CInputAction> pAction)
{
	return maInputActions.Remove(pAction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponent::NumInputActions(void)
{
	return maInputActions.Size();
}


//////////////////////////////////////////////////////////////////////////
//
// Turns the component's actions for eCommand off or on again.  While
// they are off the command goes on to the parent, as if they weren't
// there; a text area that isn't selectable passes the pointer this way.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::SetInputActionsEnabled(EComponentInputCommand eCommand, bool bEnabled)
{
	size				i;
	Ptr<CInputAction>	pAction;

	for (i = 0; i < maInputActions.Size(); i++)
	{
		pAction = maInputActions.Get(i);
		if ((pAction->GetFilter() == IAF_Command) && (pAction->GetCommand() == eCommand))
		{
			pAction->SetEnabled(bEnabled);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Calls every action that matches, in the order they were added.  Returns
// true if any did, then the event goes no further, unless every one that
// did set the event's bPass.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::CallInputActions(SComponentInputEvent* psEvent)
{
	Ptr<CInputAction>	pAction;
	size				i;
	size				uiNumElements;
	bool				bTaken;

	bTaken = false;
	uiNumElements = maInputActions.Size();
	for (i = 0; i < uiNumElements; i++)
	{
		pAction = maInputActions.Get(i);
		if (pAction->Matches(psEvent))
		{
			psEvent->bPass = false;
			pAction->Call(psEvent);
			if (!psEvent->bPass)
			{
				bTaken = true;
			}
		}
	}
	psEvent->bPass = false;
	return bTaken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::Draw(void)
{
	return DrawChildren();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::DrawChildren(void)
{
	size						i;
	Ptr<CComponent>	pComponent;
	bool					bResult;
	size					uiSize;

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		bResult = pComponent->Draw();
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::AddComponent(Ptr<CComponent> pComponent)
{
	InvalidateLayout();
	maChildren.Add(pComponent);
	pComponent->mpParent = this;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::RemoveComponent(Ptr<CComponent> pComponent)
{
	bool	bResult;

	if (mpWindow)
	{
		mpWindow->GetFocus()->Forget(pComponent);
	}

	bResult = maChildren.Remove(pComponent);
	if (bResult)
	{
		pComponent->mpParent = NULL;
		InvalidateLayout();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::RemoveAllComponents(void)
{
	size				i;
	Ptr<CComponent>		pComponent;
	size				uiSize;

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		if (mpWindow)
		{
			mpWindow->GetFocus()->Forget(pComponent);
		}
		pComponent->mpParent = NULL;
	}
	InvalidateLayout();
	return maChildren.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponent::NumComponents(void)
{
	return maChildren.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::GetContainerBounds(CInt32Rect* pcDest)
{
	//The only way the function can be called is if the component is not a container.
	if (mpParent)
	{
		return mpParent->GetContainerBounds(pcDest);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the component is pAncestor or is somewhere inside it.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::IsWithin(Ptr<CComponent> pAncestor)
{
	Ptr<CComponent>	pComponent;

	if (pAncestor.IsNull())
	{
		return false;
	}

	pComponent = this;
	while (pComponent.IsNotNull())
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
// A component that clips its children, like a scroll pane, only lets them
// draw inside its container bounds.  Most components draw their children
// wherever they are laid out.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::ClipsChildren(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The area, in window client space, this component can be drawn in: the
// overlap of the container bounds of every ancestor that clips its
// children.  Returns false if no ancestor clips, so the component can be
// drawn anywhere.  The area can be empty.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::GetClip(CInt32Rect* pcClip)
{
	Ptr<CComponent>	pAncestor;
	CInt32Rect		cBounds;
	bool			bClipped;

	bClipped = false;
	pAncestor = mpParent;
	while (pAncestor.IsNotNull())
	{
		if (pAncestor->ClipsChildren() && pAncestor->GetContainerBounds(&cBounds))
		{
			if (bClipped)
			{
				pcClip->ClipTo(&cBounds);
			}
			else
			{
				pcClip->Init(&cBounds);
				bClipped = true;
			}
		}
		pAncestor = pAncestor->GetParent();
	}
	return bClipped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::IsPointIn(int x, int y)
{
	//x and y are in window client space.  The right and bottom edges belong to the next component over.
	return (x >= mcActualBounds.msTopLeft.x) && (x < mcActualBounds.msBottomRight.x) &&
		   (y >= mcActualBounds.msTopLeft.y) && (y < mcActualBounds.msBottomRight.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CComponent::FindComponentAt(int x, int y)
{
	size					i;
	Ptr<CComponent>	pComponent;
	size					uiSize;
	CInt32Rect				cBounds;

	//x and y are in window client space.
	if (!IsPointIn(x, y))
	{
		return NULL;
	}

	//Children are only found where they can be drawn.  The right and bottom edges belong to the next component over.
	if (ClipsChildren() && GetContainerBounds(&cBounds))
	{
		if ((x < cBounds.msTopLeft.x) || (x >= cBounds.msBottomRight.x) ||
			(y < cBounds.msTopLeft.y) || (y >= cBounds.msBottomRight.y))
		{
			return this;
		}
	}

	//Children drawn later are drawn over earlier ones so they are searched first.
	uiSize = maChildren.Size();
	for (i = uiSize; i > 0; i--)
	{
		pComponent = maChildren.Get(i - 1);
		pComponent = pComponent->FindComponentAt(x, y);
		if (pComponent)
		{
			return pComponent;
		}
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::ToChildSpace(Ptr<CComponent> pChildComponent, int x, int y, int* px, int* py)
{
	*px = x - pChildComponent->mcActualBounds.msTopLeft.x;
	*py = y - pChildComponent->mcActualBounds.msTopLeft.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::FromChildSpace(Ptr<CComponent> pChildComponent, int x, int y, int* px, int* py)
{
	*px = x + pChildComponent->mcActualBounds.msTopLeft.x;
	*py = y + pChildComponent->mcActualBounds.msTopLeft.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{

	SetPosition(sPosition);
	SetActualSize(sAreaSize);

	LayoutChildren(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::LayoutChildren(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	size					i;
	Ptr<CComponent>	pComponent;
	size					uiSize;

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		pComponent->Layout(sPosition, sAreaSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponent::GetDepth(void)
{
	size					iDepth;
	Ptr<CComponent>	pComponent;

	iDepth = 0;
	pComponent = mpParent;
	while (pComponent != NULL)
	{
		iDepth++;
		pComponent = pComponent->mpParent;
	}
	return iDepth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::IsFocussed(void)
{
	if (mpWindow->GetFocus()->GetFocussedComponent() == this)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Focussed in the active window, so it gets the keys.  A component keeps
// focus while its window is inactive, but it doesn't have it.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::HasFocus(void)
{
	return IsFocussed() && mpWindow->GetFocus()->IsActive();
}


//////////////////////////////////////////////////////////////////////////
//
// The component is the one under the pointer, not one of its children.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::IsMouseOver(void)
{
	return mpWindow->GetFocus()->GetMouseOverComponent() == this;
}


//////////////////////////////////////////////////////////////////////////
//
// Each tells FocusOrMouseOverChanged, for a component whose look only
// depends on HasFocus and IsMouseOver.  The focus has already changed when
// they are called, so those say what is now true.  A component overriding
// one calls it too if it uses FocusOrMouseOverChanged.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::FocusGained(void) { FocusOrMouseOverChanged(); }
void CComponent::FocusLost(void) { FocusOrMouseOverChanged(); }
void CComponent::MovedInto(void) { FocusOrMouseOverChanged(); }
void CComponent::MovedOutof(void) { FocusOrMouseOverChanged(); }
void CComponent::FocusOrMouseOverChanged(void) {}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::SetActualSize(int iWidth, int iHeight) { mcActualBounds.SetSize(iWidth, iHeight); }
void CComponent::SetActualSize(SInt32Vec2 sSize) { mcActualBounds.SetSize(sSize.x, sSize.y); }
void CComponent::SetPosition(int x, int y) { mcActualBounds.SetPos(x, y); }
void CComponent::SetPosition(SInt32Vec2 sPosition) { mcActualBounds.SetPos(sPosition.x, sPosition.y); }
SInt32Vec2 CComponent::GetPosition(void) { return mcActualBounds.msTopLeft; }
bool CComponent::CanGetFocus(void) { return mbCanGetFocus; }
void CComponent::SetCanGetFocus(bool bCanGetFocus) { mbCanGetFocus = bCanGetFocus; }
bool CComponent::WantsEnter(void) { return false; }  //Enter goes to the window's default button unless the focussed component wants it.
void CComponent::PopupClosed(Ptr<CComponent> pContent) {}  //A popup this component opened, see CWindow::OpenPopup, has closed.
void CComponent::PopupItemClicked(Ptr<CComponent> pContent, Ptr<CComponent> pItem) {}  //An item in a popup this component opened, like a menu's, was clicked.  The popup has closed.
Ptr<CComponent> CComponent::GetParent(void) { return mpParent; }
Ptr<CWindow> CComponent::GetWindow(void) { return mpWindow; }


//////////////////////////////////////////////////////////////////////////
//
// The window is painted again soon.  Input already does this; call it
// when what a component shows changes without input, a simulation
// running say.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Invalidate(void)
{
	if (mpWindow.IsNotNull())
	{
		mpWindow->Invalidate();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The window is laid out again before it is next painted, and painted.
// The window keeps its layout between paints, so call it whenever a
// component changes something its layout depends on without input: its
// text, its size, its children...  Key, button and wheel input already
// do this; pointer movement doesn't.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::InvalidateLayout(void)
{
	if (mpWindow.IsNotNull())
	{
		mpWindow->InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the position and size of this component and every component
// under it, parents first.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::SaveLayout(CArrayTemplate<SInt32Vec2>* pasBounds)
{
	size		i;
	SInt32Vec2	sPosition;
	SInt32Vec2	sSize;

	sPosition = GetPosition();
	sSize = GetActualSize();
	pasBounds->Add(&sPosition);
	pasBounds->Add(&sSize);
	for (i = 0; i < maChildren.Size(); i++)
	{
		maChildren.Get(i)->SaveLayout(pasBounds);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Logs each component, in SaveLayout's order from uiIndex, that has been
// laid out somewhere other than where it was saved.  Returns the index
// after the last component checked.
//
//////////////////////////////////////////////////////////////////////////
size CComponent::CheckLayout(CArrayTemplate<SInt32Vec2>* pasBounds, size uiIndex, size* puiNumLogged)
{
	size			i;
	SInt32Vec2*		psPosition;
	SInt32Vec2*		psSize;
	SInt32Vec2		sPosition;
	SInt32Vec2		sSize;

	if (uiIndex + 1 >= pasBounds->NumElements())
	{
		return uiIndex;
	}

	psPosition = pasBounds->Get(uiIndex);
	psSize = pasBounds->Get(uiIndex + 1);
	sPosition = GetPosition();
	sSize = GetActualSize();
	if (((psPosition->x != sPosition.x) || (psPosition->y != sPosition.y) || (psSize->x != sSize.x) || (psSize->y != sSize.y)) && (*puiNumLogged < 8))
	{
		gcLogger.Error2(__METHOD__, " [", ClassName(), "] moved from [", IntToString(psPosition->x), ", ", IntToString(psPosition->y), "] size [", IntToString(psSize->x), " x ", IntToString(psSize->y),
						"] to [", IntToString(sPosition.x), ", ", IntToString(sPosition.y), "] size [", IntToString(sSize.x), " x ", IntToString(sSize.y), "] but the layout wasn't invalidated.", NULL);
		(*puiNumLogged)++;
	}

	uiIndex += 2;
	for (i = 0; i < maChildren.Size(); i++)
	{
		uiIndex = maChildren.Get(i)->CheckLayout(pasBounds, uiIndex, puiNumLogged);
	}
	return uiIndex;
}
void CComponent::ScrollToShow(CInt32Rect* pcRect) { if (mpParent) { mpParent->ScrollToShow(pcRect); } }  //pcRect is in the window's space.  A component that scrolls its content, like a scroll pane, moves it to show the rect.
bool CComponent::GetScrollView(CInt32Rect* pcDest) { if (mpParent) { return mpParent->GetScrollView(pcDest); } return false; }  //The part of the window the component is seen through when it is scrolled.  False if nothing scrolls it.
void CComponent::SetDesiredSize(int iWidth, int iHeight) { msDesiredSize.Init(iWidth, iHeight); InvalidateLayout(); }
SInt32Vec2 CComponent::GetActualSize(void) { SInt32Vec2 sSize; sSize.Init(mcActualBounds.GetWidth(), mcActualBounds.GetHeight()); return sSize; }
SInt32Vec2 CComponent::GetDesiredSize(void) { return msDesiredSize; }
void CComponent::SetAlignment(EContainerStyleHorizontal eHorizontal, EContainerStyleVertical eVertical) { meHorizontalAlignment = eHorizontal; meVerticalAlignment = eVertical; InvalidateLayout(); }
EContainerStyleHorizontal CComponent::GetHorizontalAlignment(void) { return meHorizontalAlignment; }
EContainerStyleVertical CComponent::GetVerticalAlignment(void) { return meVerticalAlignment; }

