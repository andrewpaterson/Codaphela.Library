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
#ifndef __DOCK_DRAG_H__
#define __DOCK_DRAG_H__
#include "StandardLib/Object.h"
#include "DockManager.h"


class CDockState;


class CDockDragTracker;


//Dragging panes, as Visual Studio drags them.  A tab or title bar pulled off floats what it pulls (BeginPane,
//BeginGroup) and every drag is then of a floating window (BeginFloating): a drag tracker takes the drag from the
//component that started it (Take), the window follows the pointer held where it was grabbed, the window under the
//pointer shows its docking guides, and the drop docks it at the guide under the pointer, leaves it floating, or, with
//Escape, puts it back.  Control held keeps the guides away.
class CDockDrag : public CObject
{
CONSTRUCTABLE(CDockDrag);
DESTRUCTABLE(CDockDrag);
protected:
	Ptr<CDockManager>			mpManager;
	CDockState*					mpcState;  //The manager's, shared with its other parts.
	Ptr<CDockFloatingWindow>	mpFloating;  //What is dragged.
	Ptr<CWindow>				mpSource;  //The window the drag started in; it has the pointer.
	Ptr<CDockDragTracker>		mpTracker;
	SInt32Vec2					msGrab;  //The pointer relative to the floating window's top left.
	CInt32Rect					mcStart;  //Where the floating window was when the drag started.
	bool						mbTornOff;  //The drag floated what it drags, Escape docks it back.
	bool						mbCancelled;
	bool						mbNoDock;  //Dropped without docking, see SetNoDock.
	SDockTarget					msTarget;
	Ptr<CDockArea>				mpGuidesArea;  //The area showing the guides.

public:
	void						Init(Ptr<CDockManager> pManager, CDockState* pcState);
	void						Class(void) override;
	void						Free(void) override;
	bool						Save(CObjectWriter* pcFile) override;
	bool						Load(CObjectReader* pcFile) override;

	void						BeginPane(Ptr<CDockPane> pPane);
	void						BeginGroup(Ptr<CDockGroup> pGroup);
	void						BeginFloating(Ptr<CDockFloatingWindow> pFloating);
	void						Moved(void);
	void						Dropped(void);
	void						Cancelled(void);
	void						SetNoDock(bool bNoDock);
	bool						IsDragging(void);
	SDockTarget*				GetTarget(void);
	Ptr<CDockFloatingWindow>	GetFloating(void);
	void						FindTarget(SInt32Vec2 sScreen, SDockTarget* psTarget);

protected:
	bool						Take(Ptr<CWindow> pSource);
	void						Start(Ptr<CDockFloatingWindow> pFloating, SInt32Vec2 sGrab, Ptr<CWindow> pSource, bool bTornOff);
	void						End(void);
	void						Drop(Ptr<CDockFloatingWindow> pFloating, SDockTarget* psTarget, bool bCancelled, bool bTornOff, CInt32Rect* pcStart);
	void						ShowGuides(SDockTarget* psTarget, Ptr<CDockArea> pArea, SInt32Vec2 sClient);
	void						HideGuides(void);
	bool						IsDraggingDocuments(void);
	bool						IsControlHeld(void);
};


#endif // __DOCK_DRAG_H__

