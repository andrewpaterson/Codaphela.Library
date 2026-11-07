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
#include "MenuContainer.h"
#include "ContextMenu.h"
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
	mbHitAsWhole = false;
	mpParent = NULL;
	mpWindow = pWindow;
	mpContextMenu = NULL;
	meCursor = NC_Arrow;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Free(void)
{
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
		U_Bool(mbHitAsWhole);
		M_Pointer(mpParent);
		M_Embedded(maChildren);
	Nes(M_Pointer(mpWindow));
		M_Embedded(maInputActions);
		M_Pointer(mpContextMenu);
		U_Enum(meCursor);
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
// did set the event's bPass.  An action can remove actions, as taking a
// dragged tab off its group takes its context menu's away.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::CallInputActions(SComponentInputEvent* psEvent)
{
	Ptr<CInputAction>	pAction;
	size				i;
	bool				bTaken;

	bTaken = false;
	for (i = 0; i < maInputActions.Size(); i++)
	{
		pAction = maInputActions.Get(i);
		if (pAction.IsNotNull() && pAction->Matches(psEvent))
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
	size					i;
	Ptr<CComponent>			pComponent;
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
	if (!maChildren.Remove(pComponent))
	{
		return false;
	}

	LetGo(pComponent);
	InvalidateLayout();
	return true;
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
		LetGo(pComponent);
	}
	InvalidateLayout();
	return maChildren.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//
// A child being removed: nothing in it stays hovered, focussed or
// dragging, and it has no parent.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::LetGo(Ptr<CComponent> pComponent)
{
	if (mpWindow)
	{
		mpWindow->GetFocus()->Forget(pComponent);
	}
	pComponent->mpParent = NULL;
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
	return mcActualBounds.ContainsPoint(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CComponent::FindComponentAt(int x, int y)
{
	size					i;
	Ptr<CComponent>			pComponent;
	size					uiSize;
	CInt32Rect				cBounds;

	//x and y are in window client space.
	if (!IsPointIn(x, y))
	{
		return NULL;
	}
	if (mbHitAsWhole)
	{
		return this;
	}

	//Children are only found where they can be drawn.  The right and bottom edges belong to the next component over.
	if (ClipsChildren() && GetContainerBounds(&cBounds))
	{
		if (!cBounds.ContainsPoint(x, y))
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
	Ptr<CComponent>			pComponent;
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
// A component with a caret, a text area say, says where it is in the
// window, cut to what of it shows (see GetClip), right and bottom
// exclusive, and its colour.  The window draws it, blinking, over what it
// paints (see CWindow::PaintCaret).  False for none, as here.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::GetCaretRect(CInt32Rect* pcDest, ARGB32* psColour)
{
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
// The focus tells a component it has gained or lost focus and that the
// pointer has moved into, out of or over it; MovedOver's x and y are
// relative to the component's top left.
//
// All but MovedOver tell FocusOrMouseOverChanged, for a component whose
// look only depends on HasFocus and IsMouseOver.  The focus has already
// changed when they are called, so those say what is now true.  A
// component overriding one calls it too if it uses
// FocusOrMouseOverChanged.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::FocusGained(void) { FocusOrMouseOverChanged(); }
void CComponent::FocusLost(void) { FocusOrMouseOverChanged(); }
void CComponent::MovedInto(void) { FocusOrMouseOverChanged(); }
void CComponent::MovedOutof(void) { FocusOrMouseOverChanged(); }
void CComponent::MovedOver(int x, int y) {}
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
void CComponent::SetHitAsWhole(bool bHitAsWhole) { mbHitAsWhole = bHitAsWhole; }  //A component made of others, like a button, has the pointer over it and takes focus rather than its parts.
bool CComponent::WantsEnter(void) { return false; }  //Enter goes to the window's default button unless the focussed component wants it.
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



//////////////////////////////////////////////////////////////////////////
//
// A popup this component opened, see CWindow::OpenPopup, has closed.  If
// it is the context menu the keys go back where they were.  Overrides
// call it.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::PopupClosed(Ptr<CComponent> pContent)
{
	if (mpContextMenu.IsNotNull())
	{
		mpContextMenu->PopupClosed(pContent);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pMenu opens on this component, and the components in it without menus
// of their own, with the right button, the Menu key or Shift+F10 (see
// CContextMenu).  NULL for none.  Its items' clicks are heard through its
// item click event.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::SetContextMenu(Ptr<CMenuContainer> pMenu)
{
	if (mpContextMenu.IsNotNull())
	{
		mpContextMenu->RemoveActions();
		mpContextMenu = NULL;
	}
	if (pMenu.IsNotNull())
	{
		mpContextMenu = OMalloc<CContextMenu>(this, pMenu);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMenuContainer> CComponent::GetContextMenu(void)
{
	if (mpContextMenu.IsNull())
	{
		return NULL;
	}
	return mpContextMenu->GetMenu();
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the context menu with its top left at sPosition, in window client
// space, as the right button does.  False if there is none or it has no
// items.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::OpenContextMenu(SInt32Vec2 sPosition)
{
	if (mpContextMenu.IsNull())
	{
		return false;
	}
	return mpContextMenu->Open(sPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// Where a context menu opened from the keys (the Menu key or Shift+F10)
// has its top left, in window client space, while this component has the
// keys.  Its top left; a text area says its caret.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CComponent::GetContextMenuPosition(void)
{
	return GetPosition();
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcComponent is in a window's tree: its top parent is a window.
//
//////////////////////////////////////////////////////////////////////////
static bool IsInWindowTree(CComponent* pcComponent)
{
	Ptr<CComponent>	pTop;
	Ptr<CComponent>	pParent;

	pTop = pcComponent;
	pParent = pTop->GetParent();
	while (pParent.IsNotNull())
	{
		pTop = pParent;
		pParent = pTop->GetParent();
	}
	return dynamic_cast<CWindow*>(&pTop) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The component, and everything of it, now belongs to pWindow, as a
// docked pane's content does when it is floated in a window of its own.
// Remove it from its parent first.  Its children move, and so do the
// components it points to that belong to its old window but aren't in
// the rest of that window's tree: pages of tabs not shown, menus, scroll
// bars not shown and the like.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::MoveToWindow(Ptr<CWindow> pWindow)
{
	Ptr<CWindow>					pOld;
	CArrayTemplatePtr<CComponent>	apcToMove;
	CArrayEmbeddedObjectPtr			apcTos;
	CComponent*						pcComponent;
	CComponent*						pcTo;
	size							i;

	pOld = mpWindow;
	if (pOld == pWindow)
	{
		return;
	}

	apcToMove.Init();
	apcToMove.Add(this);
	while (apcToMove.NumElements() > 0)
	{
		pcComponent = apcToMove.GetPtr(apcToMove.NumElements() - 1);
		apcToMove.RemoveTail();
		if (pcComponent->mpWindow != pOld)
		{
			continue;
		}
		pcComponent->mpWindow = pWindow;

		apcTos.Init();
		pcComponent->GetPointerTos(&apcTos);
		for (i = 0; i < apcTos.NumElements(); i++)
		{
			pcTo = dynamic_cast<CComponent*>(apcTos.GetPtr(i));
			if ((pcTo == NULL) || (dynamic_cast<CWindow*>(pcTo) != NULL) || (pcTo->mpWindow != pOld))
			{
				continue;
			}
			if (pcTo->IsWithin(this) || !IsInWindowTree(pcTo))
			{
				apcToMove.Add(pcTo);
			}
		}
		apcTos.Kill();
	}
	apcToMove.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer takes this shape over the component and its children that
// have none of their own.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::SetCursor(ENativeCursor eCursor)
{
	meCursor = eCursor;
	if (mpWindow.IsNotNull())
	{
		mpWindow->UpdateCursor();
	}
}


ENativeCursor CComponent::GetCursor(void) { return meCursor; }


//////////////////////////////////////////////////////////////////////////
//
// Where the component was last laid out, in window client space.
//
//////////////////////////////////////////////////////////////////////////
void CComponent::GetActualBounds(CInt32Rect* pcDest)
{
	pcDest->Init(&mcActualBounds);
}


//////////////////////////////////////////////////////////////////////////
//
// The child at uiIndex, in the order they are drawn.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CComponent::GetComponent(size uiIndex)
{
	return maChildren.Get(uiIndex);
}

