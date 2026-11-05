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
#ifndef __DOCK_NODE_H__
#define __DOCK_NODE_H__
#include "Container.h"


#define DOCK_SASH_SIZE			4	//Between a split's children.
#define DOCK_MINIMUM_SIZE		40	//The least a split shrinks a child to.
#define DOCK_DEFAULT_SIZE		260	//A tool pane docked beside something, along the split.
#define DOCK_DRAG_DISTANCE		6	//Pixels a title bar is pulled before it is dragged.
#define DOCK_FLOAT_WIDTH		360	//A pane floated with no size of its own.
#define DOCK_FLOAT_HEIGHT		420
#define DOCK_WELL_NEIGHBOUR		"#DocumentWell"	//A pane's restore neighbour when it was beside the document well.


//The sides something can be docked on, of a group, the document well or a dock area, and the centre: tabbed into a
//group.
enum EDockSide
{
	DS_Left,
	DS_Right,
	DS_Top,
	DS_Bottom,
	DS_Centre,
	DS_None,
};


enum EDockNodeKind
{
	DNK_Split,
	DNK_Group,
	DNK_DocumentWell,
};


class CDockManager;
class CDockPane;
class CDockGroup;
class CDockSplit;
class CDockArea;
class CDocumentWell;


//A piece of a dock area's tree: a split of other nodes, a group of panes shown as tabs, or the document well.  Its
//parent is a split, the document well or the dock area.  The dock size is what it wants along its parent split, in
//pixels, 0 for a share of what is left.
class CDockNode : public CContainer
{
CONSTRUCTABLE(CDockNode);
DESTRUCTABLE(CDockNode);
protected:
	Ptr<CDockManager>	mpManager;
	int32				miDockSize;

public:
			void				Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager);
			void				Class(void) override;
			bool				Save(CObjectWriter* pcFile) override;
			bool				Load(CObjectReader* pcFile) override;
	virtual	EDockNodeKind		GetKind(void) =0;
	virtual	bool				ContainsDocumentWell(void);
	virtual	void				GetPanes(CArray<CDockPane>* paPanes) =0;
	virtual	Ptr<CDockGroup>		FindGroupAt(int x, int y);
	virtual	Ptr<CDockGroup>		GetFirstGroup(void) =0;
			void				SetDockSize(int32 iSize);
			int32				GetDockSize(void);
			Ptr<CDockManager>	GetManager(void);
			Ptr<CDockSplit>		GetParentSplit(void);
			bool				IsSplit(void);
			bool				IsGroup(void);
			bool				IsDocumentWell(void);
			bool				HasDocuments(void);
			bool				IsEmpty(void);
};


EDockSide	OppositeDockSide(EDockSide eSide);
bool		IsHorizontalDockSide(EDockSide eSide);
const char*	DockSideToString(EDockSide eSide);
EDockSide	StringToDockSide(const char* szSide);


#endif // __DOCK_NODE_H__

