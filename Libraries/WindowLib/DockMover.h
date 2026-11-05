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
#ifndef __DOCK_MOVER_H__
#define __DOCK_MOVER_H__
#include "SupportLib/Int32Rect.h"
#include "DockNode.h"


class CDockManager;
class CDockPane;
class CDockGroup;
class CDockFloatingWindow;
class CComponent;
class CWindow;
class CFocus;
struct SDockRestore;


//Moving panes and groups between docked, floating and auto hidden, and making and placing the floating windows they
//float in.  Part of its manager.
class CDockMover
{
protected:
	CDockManager*	mpcManager;

public:
	void						Init(CDockManager* pcManager);
	void						Kill(void);

	void						Float(Ptr<CDockPane> pPane, CInt32Rect* pcScreenRectangle = NULL);
	void						FloatGroup(Ptr<CDockGroup> pGroup, CInt32Rect* pcScreenRectangle = NULL);
	void						Dock(Ptr<CDockPane> pPane);
	void						DockAsDocument(Ptr<CDockPane> pPane);
	void						AutoHide(Ptr<CDockPane> pPane);
	void						AutoHideGroup(Ptr<CDockGroup> pGroup);
	void						ToggleFloat(Ptr<CDockPane> pPane);
	void						ToggleFloatGroup(Ptr<CDockGroup> pGroup);
	void						DockFloating(Ptr<CDockFloatingWindow> pFloating);
	void						CloseFloating(Ptr<CDockFloatingWindow> pFloating);
	EDockSide					NearestEdge(Ptr<CDockGroup> pGroup);
	void						FloatRectangleFor(Ptr<CComponent> pComponent, Ptr<CDockPane> pPane, CInt32Rect* pcDest);
	void						KeepOnScreen(CInt32Rect* pcRect);
	Ptr<CDockFloatingWindow>	CreateFloating(CInt32Rect* pcScreenRectangle);
};


#endif // __DOCK_MOVER_H__

