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
#ifndef __DOCK_STATE_H__
#define __DOCK_STATE_H__
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"
#include "DockPane.h"
#include "DockGroup.h"
#include "DockFocus.h"
#include "DockTree.h"
#include "DockRestorer.h"
#include "DockMover.h"


class CDockManager;
class CDockManagerListener;
class CDockArea;
class CDocumentWell;
class CDockFloatingWindow;
class CDockDrag;


//What docking shares between CDockManager and its parts (the drag, the focus, the tree's edits, the restorer, the
//mover, the layout file): the main window, the dock area and its document well, every pane, the floating windows, the
//well's document group, the next group's ID, the listener, the main window's place in the last layout read, and the
//parts themselves.  The manager holds it and gives it its parts (CDockManager::GetState); applications use the
//manager.  A pane keeps its own place: its state, group, default place, restore record, floating rectangle and auto
//hide side and size (see CDockPane).
class CDockState : public CObject
{
CONSTRUCTABLE(CDockState);
DESTRUCTABLE(CDockState);
protected:
	Ptr<CWindow>				mpWindow;  //The main window.
	Ptr<CDockArea>				mpArea;
	Ptr<CDocumentWell>			mpWell;
	CArray<CDockPane>			maPanes;
	CArray<CDockFloatingWindow>	maFloating;
	int32						miNextGroupID;
	Ptr<CDockGroup>				mpDocumentGroup;  //The well's group documents are added to.
	CDockManagerListener*		mpcListener;
	CInt32Rect					mcLayoutWindow;  //The main window's client area on the screen, from the last layout read.
	bool						mbLayoutWindow;

	Ptr<CDockDrag>				mpDrag;
	Ptr<CDockFocus>				mpFocus;  //The active pane and document, and the flyout.
	Ptr<CDockPane>				mpMenuPane;  //The pane a tab or title bar menu was opened for last.
	CDockTree					mcTree;  //The trees' edits.
	CDockRestorer				mcRestorer;  //Where panes go back to.
	CDockMover					mcMover;  //Floating, docking and auto hiding.

public:
	void						Init(Ptr<CWindow> pWindow);
	void						Class(void) override;
	void						Free(void) override;
	bool						Save(CObjectWriter* pcFile) override;
	bool						Load(CObjectReader* pcFile) override;

	void						MakeParts(CDockManager* pcManager);

	Ptr<CWindow>					GetWindow(void);
	Ptr<CDockArea>					GetArea(void);
	Ptr<CDocumentWell>				GetWell(void);
	CArray<CDockPane>*				GetPanes(void);
	CArray<CDockFloatingWindow>*	GetFloating(void);
	int32							GetNextGroupID(void);
	void							SetNextGroupID(int32 iID);
	int32							TakeGroupID(int32 iID);
	Ptr<CDockGroup>					GetDocumentGroup(void);
	void							SetDocumentGroup(Ptr<CDockGroup> pGroup);
	CDockManagerListener*			GetListener(void);
	void							SetListener(CDockManagerListener* pcListener);
	bool							GetLayoutWindow(CInt32Rect* pcDest);
	void							SetLayoutWindow(CInt32Rect* pcRectangle);

	Ptr<CDockDrag>				GetDrag(void);
	Ptr<CDockFocus>				GetFocus(void);
	Ptr<CDockPane>				GetMenuPane(void);
	void						SetMenuPane(Ptr<CDockPane> pPane);
	CDockTree*					GetTree(void);
	CDockRestorer*				GetRestorer(void);
	CDockMover*					GetMover(void);
};


#endif // __DOCK_STATE_H__
