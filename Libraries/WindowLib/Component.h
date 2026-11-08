/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __COMPONENT_H__
#define __COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "SupportLib/ColourFormat.h"
#include "Focus.h"
#include "SupportLib/Int32Rect.h"
#include "ContainerStyle.h"
#include "BaseLib/ArrayTemplate.h"
#include "InputAction.h"
#include "NativeCursor.h"


class CWindow;
class CMenuContainer;
class CContextMenu;
//Input reaches a component through the input actions added to it (see AddInputAction), given by the window's focus.
//
//A component can have a context menu (SetContextMenu), opened by the right button or the Menu key, see CContextMenu.
class CComponent : public CObject
{
CONSTRUCTABLE(CComponent);
DESTRUCTABLE(CComponent);
protected:
	CInt32Rect					mcActualBounds;
	CInt32Rect					mcDrawnArea;  //Where in the window the component drew at the last paint, empty if nowhere (see PrepareDraw).
	SInt32Vec2					msDesiredSize;
	EContainerStyleHorizontal	meHorizontalAlignment;  //Where a fixed size component sits in spare space.  Stretch makes it fill the space.
	EContainerStyleVertical		meVerticalAlignment;
	bool						mbCanGetFocus;
	bool						mbHitAsWhole;  //Found by FindComponentAt itself, never its children; see SetHitAsWhole.
	Ptr<CComponent>				mpParent;
	CArray<CComponent>			maChildren;
	Ptr<CWindow>				mpWindow;
	CArray<CInputAction>		maInputActions;
	Ptr<CContextMenu>			mpContextMenu;  //NULL for none.
	ENativeCursor				meCursor;  //The pointer's shape over the component, NC_Arrow for its parent's.
	CChars						mszToolTip;  //Shown after the pointer rests on the component (see CToolTips), empty for none.

public:
			void				Init(Ptr<CWindow> pWindow);
			void				Class(void) override;
			void				Free(void) override;

			bool				Save(CObjectWriter* pcFile) override;
			bool				Load(CObjectReader* pcFile) override;

	virtual	bool				Draw(void);
			bool				DrawChildren(void);
	virtual	bool				IsDrawn(void);
	virtual	bool				IsChildDrawn(Ptr<CComponent> pChild);
			void				PrepareDraw(bool bDrawn, bool bClipped, CInt32Rect* pcClip);
			void				GetDrawnArea(CInt32Rect* pcDest);
			void				ForgetDrawn(void);
			void				ContentChanged(void);
	virtual bool				GetContainerBounds(CInt32Rect* pcDest);
	virtual	bool				ClipsChildren(void);
			bool				GetClip(CInt32Rect* pcClip);

	virtual	bool				AddComponent(Ptr<CComponent> pComponent);
	virtual	bool				RemoveComponent(Ptr<CComponent> pComponent);
	virtual	bool				RemoveAllComponents(void);
			size				NumComponents(void);
			Ptr<CComponent>		GetComponent(size uiIndex);

			bool				IsPointIn(int x, int y);
	virtual	Ptr<CComponent>		FindComponentAt(int x, int y);

	virtual	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
			void				LayoutChildren(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);

			bool				IsFocussed(void);
			bool				HasFocus(void);
			bool				IsMouseOver(void);
	virtual	void				FocusGained(void);
	virtual	void				FocusLost(void);
	virtual	bool				GetCaretRect(CInt32Rect* pcDest, ARGB32* psColour);
	virtual	void				MovedInto(void);
	virtual	void				MovedOutof(void);
	virtual	void				MovedOver(int x, int y);
	virtual	void				FocusOrMouseOverChanged(void);
			bool				CanGetFocus(void);
			void				SetCanGetFocus(bool bCanGetFocus);
			void				SetHitAsWhole(bool bHitAsWhole);
	virtual	bool				WantsEnter(void);
	virtual	bool				CanDoCommand(EComponentInputCommand eCommand);
	virtual	void				PopupClosed(Ptr<CComponent> pContent);  //Overrides call it.
	virtual	void				PopupItemClicked(Ptr<CComponent> pContent, Ptr<CComponent> pItem);
			Ptr<CComponent>		GetParent(void);
			Ptr<CWindow>		GetWindow(void);
			void				MoveToWindow(Ptr<CWindow> pWindow);
			void				SetCursor(ENativeCursor eCursor);
			ENativeCursor		GetCursor(void);
			void				SetToolTip(const char* szToolTip);
	virtual	bool				GetToolTip(CChars* pszDest);
	virtual	void				Invalidate(void);
	virtual	void				InvalidateLayout(void);
			bool				IsWithin(Ptr<CComponent> pAncestor);

			void				SetActualSize(int iWidth, int iHeight);
			void				SetActualSize(SInt32Vec2 sSize);
			void				SetPosition(int x, int y);
			void				SetPosition(SInt32Vec2 sPosition);
			SInt32Vec2			GetPosition(void);
	virtual	SInt32Vec2			GetDesiredSize(void);
	virtual	void				ScrollToShow(CInt32Rect* pcRect);
	virtual	bool				GetScrollView(CInt32Rect* pcDest);
			void				SetDesiredSize(int iWidth, int iHeight);
			SInt32Vec2			GetActualSize(void);
			void				GetActualBounds(CInt32Rect* pcDest);

			void						SetAlignment(EContainerStyleHorizontal eHorizontal, EContainerStyleVertical eVertical);
			EContainerStyleHorizontal	GetHorizontalAlignment(void);
			EContainerStyleVertical		GetVerticalAlignment(void);

	template<class M, class O>
			Ptr<CInputAction>	AddInputAction(EInputActionFilter eFilter, const char* szName, O* pcObject, void(M::*fFunction)(SComponentInputEvent*));
	template<class O, class F>
			Ptr<CInputAction>	AddInputAction(EComponentInputCommand eCommand, O* pcObject, F fFunction);
	template<size N, class M, class O>
			void				AddInputActions(const EComponentInputCommand (&aeCommands)[N], O* pcObject, void(M::*fFunction)(SComponentInputEvent*));
			bool				RemoveInputAction(Ptr<CInputAction> pAction);
			size				NumInputActions(void);
			void				SetInputActionsKeepLayout(size uiFirst);
			void				SetInputActionsEnabled(EComponentInputCommand eCommand, bool bEnabled);
			bool				CallInputActions(SComponentInputEvent* psEvent);

			void				SetContextMenu(Ptr<CMenuContainer> pMenu);
			Ptr<CMenuContainer>	GetContextMenu(void);
			bool				OpenContextMenu(SInt32Vec2 sPosition);
	virtual	SInt32Vec2			GetContextMenuPosition(void);

protected:
	virtual	void				PrepareArea(bool bDrawn, bool bClipped, CInt32Rect* pcClip, CInt32Rect* pcArea);
	virtual	void				MarkContentChanged(void);
			void				ChildrenChanged(void);
			void				LayoutChanged(void);
			void				SaveLayout(CArrayTemplate<SInt32Vec2>* pasBounds);
			size				CheckLayout(CArrayTemplate<SInt32Vec2>* pasBounds, size uiIndex, size* puiNumLogged);
			void				LetGo(Ptr<CComponent> pComponent);
			Ptr<CInputAction>	MallocInputAction(void);
			void				AddInputAction(Ptr<CInputAction> pAction);
};


//////////////////////////////////////////////////////////////////////////
//
// szName is the key or chord for IAF_KeyDown, IAF_KeyUp and IAF_Chord,
// otherwise NULL.  pcObject is usually this component.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
Ptr<CInputAction> CComponent::AddInputAction(EInputActionFilter eFilter, const char* szName, O* pcObject, void(M::*fFunction)(SComponentInputEvent*))
{
	Ptr<CInputAction>	pAction;

	pAction = MallocInputAction();
	pAction->Init(eFilter, szName, pcObject, fFunction);
	AddInputAction(pAction);
	return pAction;
}


//////////////////////////////////////////////////////////////////////////
//
// An IAF_Command action.  F is a member function of pcObject's class, or
// a class it derives from, taking the event, nothing, or the pointer's x
// and y (see CInputAction).
//
//////////////////////////////////////////////////////////////////////////
template<class O, class F>
Ptr<CInputAction> CComponent::AddInputAction(EComponentInputCommand eCommand, O* pcObject, F fFunction)
{
	Ptr<CInputAction>	pAction;

	pAction = MallocInputAction();
	pAction->Init(eCommand, pcObject, fFunction);
	AddInputAction(pAction);
	return pAction;
}


//////////////////////////////////////////////////////////////////////////
//
// An IAF_Command action for each command, all calling fFunction, which
// can switch on the event's command.
//
//////////////////////////////////////////////////////////////////////////
template<size N, class M, class O>
void CComponent::AddInputActions(const EComponentInputCommand (&aeCommands)[N], O* pcObject, void(M::*fFunction)(SComponentInputEvent*))
{
	size	i;

	for (i = 0; i < N; i++)
	{
		AddInputAction(aeCommands[i], pcObject, fFunction);
	}
}


#endif // __COMPONENT_H__

