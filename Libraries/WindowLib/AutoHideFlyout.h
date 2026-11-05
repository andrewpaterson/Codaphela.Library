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
#ifndef __AUTO_HIDE_FLYOUT_H__
#define __AUTO_HIDE_FLYOUT_H__
#include "Canvas.h"
#include "DockEdge.h"
#include "DockTitleBar.h"
#include "DockNode.h"


#define AUTO_HIDE_EDGE			4	//A flyout's inner edge, dragged to resize it.


class CDockPane;


//An auto hidden pane slid out over the main window from its strip: a title bar with the pin (to dock it back) and the
//close cross over the pane's content, and an edge on the side away from the strip that is dragged to resize it.  It is
//a popup, so a press anywhere else slides it back.
class CAutoHideFlyout : public CContainer, public CDockEdgeOwner
{
CONSTRUCTABLE(CAutoHideFlyout);
DESTRUCTABLE(CAutoHideFlyout);
protected:
	Ptr<CDockManager>		mpManager;
	Ptr<CDockPane>			mpPane;
	EDockSide				meSide;
	Ptr<CDockTitleBar>		mpTitleBar;
	Ptr<CDockEdge>			mpEdge;
	Ptr<CCanvas>			mpBackground;
	int32					miStartSize;
	SInt32Vec2				msStartPointer;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, Ptr<CDockPane> pPane);
	void				Class(void) override;
	void				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	void				SetRequiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	Ptr<CDockPane>		GetPane(void);
	Ptr<CDockTitleBar>	GetTitleBar(void);
	Ptr<CDockEdge>		GetEdge(void);
	EDockSide			GetSide(void);
	void				Place(CInt32Rect* pcRootBounds, SInt32Vec2* psPosition);
	void				Detach(void);
	void				PinPressed(CDockTitleBar* pcTitleBar);
	void				ClosePressed(CDockTitleBar* pcTitleBar);
	void				EdgeDragStarted(CDockEdge* pcEdge, SInt32Vec2 sScreen) override;
	void				EdgeDragged(CDockEdge* pcEdge, SInt32Vec2 sScreen) override;
	void				EdgeReleased(CDockEdge* pcEdge) override;
};


#endif // __AUTO_HIDE_FLYOUT_H__

