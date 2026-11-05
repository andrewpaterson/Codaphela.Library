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
#ifndef __DOCK_AREA_H__
#define __DOCK_AREA_H__
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "DockNode.h"


class CAutoHideStrip;
class CDockGuides;


//The root of a window's docking: a node (a split, a group, or the document well) filling it.  The main window's area
//has an auto hide strip on each edge, shown while it has tabs, and always has the document well somewhere in its tree.
//While a pane is dragged over it the area shows the docking guides over everything else.
class CDockArea : public CContainer
{
CONSTRUCTABLE(CDockArea);
DESTRUCTABLE(CDockArea);
protected:
	Ptr<CDockManager>		mpManager;
	Ptr<CDockNode>			mpRoot;
	bool					mbMain;
	Ptr<CCanvas>			mpBackground;
	Ptr<CAutoHideStrip>		mpLeftStrip;
	Ptr<CAutoHideStrip>		mpRightStrip;
	Ptr<CAutoHideStrip>		mpTopStrip;
	Ptr<CAutoHideStrip>		mpBottomStrip;
	Ptr<CDockGuides>		mpGuides;  //Over everything while a drag is over the area, otherwise NULL.
	CInt32Rect				mcRootBounds;  //Where the root was last laid out, in window client space.

public:
	void					Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, bool bMain);
	void					Class(void) override;
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;
	void					SetRequiredSize(void) override;
	void					Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;

	void					SetRoot(Ptr<CDockNode> pRoot);
	Ptr<CDockNode>			GetRoot(void);
	bool					IsMain(void);
	Ptr<CAutoHideStrip>		GetStrip(EDockSide eSide);
	void					GetRootBounds(CInt32Rect* pcDest);
	Ptr<CDockGroup>			FindGroupAt(int x, int y);
	void					ShowGuides(Ptr<CDockGuides> pGuides);
	void					HideGuides(void);
	Ptr<CDockGuides>		GetGuides(void);
	void					StripsChanged(void);

protected:
	void					PlaceChildren(void);
};


#endif // __DOCK_AREA_H__

