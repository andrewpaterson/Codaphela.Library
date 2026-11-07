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
#ifndef __DOCK_FLOATING_WINDOW_H__
#define __DOCK_FLOATING_WINDOW_H__
#include "SupportLib/Int32Rect.h"
#include "StandardLib/Object.h"
#include "GridContainer.h"
#include "DockEdge.h"
#include "DockTitleBar.h"
#include "DockNode.h"




//Panes floated in a tool window of their own (CWindow::ShowOwned) over the main window.  It has no platform frame: a
//border that resizes it and, unless it holds a single tool group (whose title bar does the job, as in Visual Studio),
//a title bar that moves it, docks it back when double clicked and closes it.  Inside is a dock area of its own, so
//panes can be docked together in it.
class CDockFloatingWindow : public CObject, public CDockEdgeOwner
{
CONSTRUCTABLE(CDockFloatingWindow);
DESTRUCTABLE(CDockFloatingWindow);
protected:
	Ptr<CDockManager>		mpManager;
	Ptr<CWindow>			mpWindow;
	Ptr<CGridContainer>		mpFrame;
	Ptr<CGridContainer>		mpInner;
	Ptr<CDockTitleBar>		mpTitleBar;
	Ptr<CDockArea>			mpArea;
	CArray<CDockEdge>		maEdges;
	bool					mbTitleShown;
	CInt32Rect				mcRectangle;  //The client area on the screen.
	CInt32Rect				mcResizeStart;
	SInt32Vec2				msResizePointer;

public:
	void					Init(Ptr<CDockManager> pManager, CInt32Rect* pcScreenRectangle);
	void					Class(void) override;
	void					Free(void) override;
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	bool					Show(void);
	void					Close(void);
	bool					IsShown(void);
	Ptr<CWindow>			GetWindow(void);
	Ptr<CDockArea>			GetArea(void);
	Ptr<CDockTitleBar>		GetTitleBar(void);
	void					SetRoot(Ptr<CDockNode> pRoot);
	Ptr<CDockNode>			GetRoot(void);
	Ptr<CDockNode>			TakeRoot(void);
	bool					IsSingleToolGroup(void);
	bool					HasDocuments(void);
	void					UpdateTitle(void);
	void					Move(CInt32Rect* pcScreenRectangle);
	void					GetRectangle(CInt32Rect* pcDest);
	size					NumEdges(void);
	Ptr<CDockEdge>			GetEdge(size uiIndex);

	void					TitleDragged(CDockTitleBar* pcTitleBar);
	void					TitleDoubleClicked(CDockTitleBar* pcTitleBar);
	void					ClosePressed(CDockTitleBar* pcTitleBar);
	void					MenuPressed(CDockTitleBar* pcTitleBar);
	void					EdgeDragStarted(CDockEdge* pcEdge, SInt32Vec2 sScreen) override;
	void					EdgeDragged(CDockEdge* pcEdge, SInt32Vec2 sScreen) override;
	void					EdgeReleased(CDockEdge* pcEdge) override;
	void					WindowClosed(CWindow* pcWindow);

protected:
	void					AddEdge(uint8 uiEdges, int iColumn, int iRow, int iWidthWeight, int iHeightWeight);
	int32					MoveLowEdge(int32 iEdge, int32 iHigh);
	int32					MoveHighEdge(int32 iEdge, int32 iLow);
};


#endif // __DOCK_FLOATING_WINDOW_H__

