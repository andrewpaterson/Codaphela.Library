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
#ifndef __DOCK_RESTORER_H__
#define __DOCK_RESTORER_H__
#include "SupportLib/Int32Rect.h"
#include "DockNode.h"


class CDockManager;
class CDockPane;
class CDockGroup;
struct SDockRestore;


//Where a pane goes back to: recording a docked pane's place as it leaves (SDockRestore), and putting a pane back
//there, or beside or with its neighbour, or in its default place.  Part of its manager.
class CDockRestorer
{
protected:
	CDockManager*	mpcManager;

public:
	void						Init(CDockManager* pcManager);
	void						Kill(void);

	void						Record(Ptr<CDockPane> pPane);
	void						GiveToGroup(Ptr<CDockGroup> pGroup);
	void						Restore(Ptr<CDockPane> pPane);
	void						DockDefault(Ptr<CDockPane> pPane);

protected:
	bool						GetForGroup(Ptr<CDockGroup> pGroup, SDockRestore* psDest);
	void						SetNeighbour(SDockRestore* psDest, Ptr<CDockNode> pSibling);
	void						RestoreFrom(Ptr<CDockPane> pPane, Ptr<CDockPane> pFrom, int iDepth);
	bool						RestoreByNeighbour(Ptr<CDockPane> pPane, Ptr<CDockPane> pNeighbour, SDockRestore* psFrom);
	void						DockBeside(Ptr<CDockPane> pPane, Ptr<CDockNode> pNear, SDockRestore* psFrom);
};


#endif // __DOCK_RESTORER_H__

