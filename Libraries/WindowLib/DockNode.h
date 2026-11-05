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
#include "SupportLib/Int32Rect.h"
#include "Container.h"


#define DOCK_SASH_SIZE			4	//Between a split's children.
#define DOCK_MINIMUM_SIZE		40	//The least a split shrinks a child to.
#define DOCK_DEFAULT_SIZE		260	//A tool pane docked beside something, along the split.
#define DOCK_DRAG_DISTANCE		6	//Pixels a title bar is pulled before it is dragged.
#define DOCK_FLOAT_WIDTH		360	//A pane floated with no size of its own.
#define DOCK_FLOAT_HEIGHT		420
#define DOCK_FLOAT_OFFSET		100	//A pane floated from nowhere, from the main window's top left.
#define DOCK_GRAB_LEFT			40	//A tab torn off is held this far into its floating window's title.
#define DOCK_RESTORE_DEPTH		4	//Neighbours followed back to find where a pane goes.
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


enum EDockPaneState
{
	DPS_Docked,		//In a group in the main window's dock area.
	DPS_Floating,	//In a group in a floating window.
	DPS_AutoHidden,	//A tab on one of the main window's edges.
	DPS_Hidden,		//Closed; it keeps where to go back to.
	DPS_Removed,	//Not in the manager.
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


class CDockNode;


//What holds a node: a split, the document well or a dock area.  The new node takes the old one's dock size.
class CDockNodeHolder
{
public:
	virtual bool	ReplaceNode(Ptr<CDockNode> pOld, Ptr<CDockNode> pNew) =0;
	virtual bool	RemoveNode(Ptr<CDockNode> pNode) =0;
};


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
	virtual	size				CountPanes(void) =0;
	virtual	Ptr<CDockPane>		GetTreePane(size uiIndex) =0;
	virtual	Ptr<CDockGroup>		FindGroupAt(int x, int y);
	virtual	Ptr<CDockGroup>		GetFirstGroup(void) =0;
			void				SetDockSize(int32 iSize);
			int32				GetDockSize(void);
			Ptr<CDockManager>	GetManager(void);
			Ptr<CDockNode>		GetParentNode(void);
			Ptr<CDockSplit>		GetParentSplit(void);
			Ptr<CDockArea>		GetParentArea(void);
			CDockNodeHolder*	GetHolder(void);
			Ptr<CDockSplit>		AsSplit(void);
			Ptr<CDockGroup>		AsGroup(void);
			Ptr<CDocumentWell>	AsWell(void);
			bool				IsSplit(void);
			bool				IsGroup(void);
			bool				IsDocumentWell(void);
			bool				HasDocuments(void);
			bool				IsEmpty(void);
			void				SetPaneStates(EDockPaneState eState);
			void				SetPaneFloatRectangles(CInt32Rect* pcRectangle);
};


EDockSide	OppositeDockSide(EDockSide eSide);
bool		IsHorizontalDockSide(EDockSide eSide);
bool		IsAfterDockSide(EDockSide eSide);
const char*	DockSideToString(EDockSide eSide);
EDockSide	StringToDockSide(const char* szSide);


#endif // __DOCK_NODE_H__

