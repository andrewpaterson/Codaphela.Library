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
#ifndef __DOCUMENT_WELL_H__
#define __DOCUMENT_WELL_H__
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "DockNode.h"


//Where documents are, in the main window's dock area; tool panes dock around it.  It holds its own tree of splits and
//document groups (Visual Studio's New Horizontal and Vertical Tab Groups), or nothing, when it shows its background.
//It is always in the main dock area, even empty, and takes whatever space the panes around it leave.
class CDocumentWell : public CDockNode, public CDockNodeHolder
{
CONSTRUCTABLE(CDocumentWell);
DESTRUCTABLE(CDocumentWell);
protected:
	Ptr<CDockNode>	mpRoot;
	Ptr<CCanvas>	mpBackground;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, ARGB32 sBackground);
	void				Class(void) override;
	EDockNodeKind		GetKind(void) override;
	bool				ContainsDocumentWell(void) override;
	void				GetPanes(CArray<CDockPane>* paPanes) override;
	size				CountPanes(void) override;
	Ptr<CDockPane>		GetTreePane(size uiIndex) override;
	Ptr<CDockGroup>		FindGroupAt(int x, int y) override;
	Ptr<CDockGroup>		GetFirstGroup(void) override;
	void				SetRequiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	void				SetRoot(Ptr<CDockNode> pRoot);
	bool				ReplaceNode(Ptr<CDockNode> pOld, Ptr<CDockNode> pNew) override;
	bool				RemoveNode(Ptr<CDockNode> pNode) override;
	Ptr<CDockNode>		GetRoot(void);
};


#endif // __DOCUMENT_WELL_H__

