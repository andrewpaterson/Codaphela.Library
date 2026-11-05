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
#ifndef __DOCK_SPLIT_H__
#define __DOCK_SPLIT_H__
#include "BaseLib/ArrayInt.h"
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "DockNode.h"


class CDockSplit;


//The bar between two of a split's children.  Dragging it moves the bar, resizing the children on each side of it as
//it goes.  The pointer shows which way it moves.
class CDockSash : public CCanvas
{
CONSTRUCTABLE(CDockSash);
DESTRUCTABLE(CDockSash);
protected:
	Ptr<CDockSplit>		mpSplit;
	bool				mbDragging;
	int32				miPressed;  //Where it was pressed along the split, in window client space.

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CDockSplit> pSplit, bool bHorizontal, ARGB32 sColour);
	void			Class(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;
	void			SetHorizontal(bool bHorizontal);
	void			ActivateComponent(int x, int y);
	void			ExecuteComponent(int x, int y);
	void			ContDragSelect(int x, int y);
	void			StopDragSelect(int x, int y);
	bool			IsDragging(void);
};


//Two or more nodes side by side (horizontal) or one above the other, with a sash between each pair.  Each node has a
//dock size along the split.  The node holding the document well takes whatever the others leave, so tool panes keep
//their size when the window is resized; in a split without the well every node shares the space by its size.  No node
//is made smaller than DOCK_MINIMUM_SIZE while there is room.
//
//A split left with one node is replaced by that node, and a node that is a split of the same direction is merged into
//its parent; the dock manager keeps them so (see CDockManager::Tidy).
class CDockSplit : public CDockNode
{
CONSTRUCTABLE(CDockSplit);
DESTRUCTABLE(CDockSplit);
protected:
	bool				mbHorizontal;
	CArray<CDockNode>	maNodes;
	CArray<CDockSash>	maSashes;  //maSashes[i] is between maNodes[i] and maNodes[i + 1].
	CArrayInt			maiSizes;  //Each node's size along the split, the last time it was laid out.
	ARGB32				msSashColour;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, bool bHorizontal, ARGB32 sSashColour);
	void				Class(void) override;
	void				Free(void) override;
	EDockNodeKind		GetKind(void) override;
	bool				ContainsDocumentWell(void) override;
	void				GetPanes(CArray<CDockPane>* paPanes) override;
	Ptr<CDockGroup>		FindGroupAt(int x, int y) override;
	Ptr<CDockGroup>		GetFirstGroup(void) override;

	void				InsertNode(Ptr<CDockNode> pNode, size uiIndex);
	void				AddNode(Ptr<CDockNode> pNode);
	bool				RemoveNode(Ptr<CDockNode> pNode);
	bool				ReplaceNode(Ptr<CDockNode> pOld, Ptr<CDockNode> pNew);
	size				NumNodes(void);
	Ptr<CDockNode>		GetNode(size uiIndex);
	size				IndexOf(Ptr<CDockNode> pNode);
	bool				IsHorizontal(void);
	void				SetHorizontal(bool bHorizontal);
	Ptr<CDockSash>		GetSash(size uiIndex);
	int32				GetNodeSize(size uiIndex);

	void				SetRequiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	void				SashMoved(Ptr<CDockSash> pSash, int32 iDelta);

protected:
	void				PlaceChildren(void);
	void				WorkOutSizes(int32 iAvailable, CArrayInt* paiSizes);
	int32				GetAlong(SInt32Vec2 sSize);
};


#endif // __DOCK_SPLIT_H__

