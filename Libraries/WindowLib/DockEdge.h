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
#ifndef __DOCK_EDGE_H__
#define __DOCK_EDGE_H__
#include "Canvas.h"
#include "ClearCanvasDraw.h"


#define DOCK_EDGE_LEFT		0x01
#define DOCK_EDGE_RIGHT		0x02
#define DOCK_EDGE_TOP		0x04
#define DOCK_EDGE_BOTTOM	0x08


class CDockEdge;
class CDockManager;


//Told about an edge being dragged.  Positions are where the pointer is on the screen, so an edge that moves its own
//window as it is dragged, a floating window's, sees steady positions.
class CDockEdgeOwner
{
public:
	virtual void	EdgeDragStarted(CDockEdge* pcEdge, SInt32Vec2 sScreen) =0;
	virtual void	EdgeDragged(CDockEdge* pcEdge, SInt32Vec2 sScreen) =0;
	virtual void	EdgeReleased(CDockEdge* pcEdge) =0;
};


//An edge or corner of something that can be resized by dragging it: a floating window's border, an auto hide
//flyout's inner edge.  Which edges it moves (DOCK_EDGE_*) sets the pointer's shape.  Its owner isn't held; it must
//outlive the edge.
class CDockEdge : public CCanvas
{
CONSTRUCTABLE(CDockEdge);
DESTRUCTABLE(CDockEdge);
protected:
	CDockEdgeOwner*			mpcOwner;
	uint8					muiEdges;
	bool					mbDragging;
	Ptr<CDockManager>		mpManager;

public:
	void		Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, CDockEdgeOwner* pcOwner, uint8 uiEdges, ARGB32 sColour);
	void		Class(void) override;
	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;
	uint8		GetEdges(void);
	bool		IsDragging(void);
	void		ActivateComponent(int x, int y);
	void		ExecuteComponent(int x, int y);
	void		ContDragSelect(int x, int y);
	void		StopDragSelect(int x, int y);
};


//Takes over a pane's drag from the tab or title bar that started it (CFocus::HandOverDrag), so the drag goes on when
//the tab is taken off its group.  It is never in a window's tree; it is focussed so the release and Escape reach it.
class CDockDragTracker : public CComponent
{
CONSTRUCTABLE(CDockDragTracker);
DESTRUCTABLE(CDockDragTracker);
protected:
	Ptr<CDockManager>	mpManager;

public:
	void		Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager);
	void		Class(void) override;
	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;
	void		ExecuteComponent(int x, int y);
	void		ContDragSelect(int x, int y);
	void		StopDragSelect(int x, int y);
	void		Cancel(void);
};


#endif // __DOCK_EDGE_H__

