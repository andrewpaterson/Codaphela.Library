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
#include "StandardLib/ClassDefines.h"
#include "DockArea.h"
#include "DocumentWell.h"
#include "DockFloatingWindow.h"
#include "DockDrag.h"
#include "DockManager.h"
#include "DockState.h"


//////////////////////////////////////////////////////////////////////////
//
// The parts, the dock area and its well are made by MakeParts, once the
// manager holds the state: they ask the manager for it.
//
//////////////////////////////////////////////////////////////////////////
void CDockState::Init(Ptr<CWindow> pWindow)
{
	PreInit();

	mpWindow = pWindow;
	mpArea = NULL;
	mpWell = NULL;
	maPanes.Init();
	maFloating.Init();
	miNextGroupID = 1;
	mpDocumentGroup = NULL;
	mpcListener = NULL;
	mcLayoutWindow.Init(0, 0, 0, 0);
	mbLayoutWindow = false;
	mpDrag = NULL;
	mpFocus = NULL;
	mpMenuPane = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockState::Class(void)
{
	M_Pointer(mpWindow);
	M_Pointer(mpArea);
	M_Pointer(mpWell);
	M_Embedded(maPanes);
	M_Embedded(maFloating);
	U_Int32(miNextGroupID);
	M_Pointer(mpDocumentGroup);
	U_Pointer(mpcListener);
	U_Data(CInt32Rect, mcLayoutWindow);
	U_Bool(mbLayoutWindow);
	M_Pointer(mpDrag);
	M_Pointer(mpFocus);
	M_Pointer(mpMenuPane);
	U_Unknown(CDockTree, mcTree);
	U_Unknown(CDockRestorer, mcRestorer);
	U_Unknown(CDockMover, mcMover);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockState::Free(void)
{
	mcMover.Kill();
	mcRestorer.Kill();
	mcTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The state isn't saved; a layout is, see CDockManager::SaveLayout.
//
//////////////////////////////////////////////////////////////////////////
bool CDockState::Save(CObjectWriter* pcFile) { return false; }
bool CDockState::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// The parts, and the dock area with the document well as its root.
//
//////////////////////////////////////////////////////////////////////////
void CDockState::MakeParts(CDockManager* pcManager)
{
	mpDrag = OMalloc<CDockDrag>(Ptr<CDockManager>(pcManager), this);
	mpFocus = OMalloc<CDockFocus>(Ptr<CDockManager>(pcManager), this);
	mcTree.Init(pcManager, this);
	mcRestorer.Init(pcManager, this);
	mcMover.Init(pcManager, this);
	mpArea = OMalloc<CDockArea>(mpWindow, Ptr<CDockManager>(pcManager), true);
	mpWell = OMalloc<CDocumentWell>(mpWindow, Ptr<CDockManager>(pcManager), pcManager->GetColour(TC_DockBackground));
	mpArea->SetRoot(mpWell);
}


//////////////////////////////////////////////////////////////////////////
//
// A group's ID: iID if it's above zero and not yet taken (the caller says
// whether a group has it), else the next one.
//
//////////////////////////////////////////////////////////////////////////
int32 CDockState::TakeGroupID(int32 iID)
{
	if (iID <= 0)
	{
		iID = miNextGroupID;
	}
	if (iID >= miNextGroupID)
	{
		miNextGroupID = iID + 1;
	}
	return iID;
}


//////////////////////////////////////////////////////////////////////////
//
// The main window's client area on the screen in the last layout read,
// false if there wasn't one.  pcRectangle NULL for none.
//
//////////////////////////////////////////////////////////////////////////
bool CDockState::GetLayoutWindow(CInt32Rect* pcDest)
{
	pcDest->Init(&mcLayoutWindow);
	return mbLayoutWindow;
}


void CDockState::SetLayoutWindow(CInt32Rect* pcRectangle)
{
	mbLayoutWindow = (pcRectangle != NULL);
	if (pcRectangle)
	{
		mcLayoutWindow.Init(pcRectangle);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CWindow>					CDockState::GetWindow(void) { return mpWindow; }
Ptr<CDockArea>					CDockState::GetArea(void) { return mpArea; }
Ptr<CDocumentWell>				CDockState::GetWell(void) { return mpWell; }
CArray<CDockPane>*				CDockState::GetPanes(void) { return &maPanes; }
CArray<CDockFloatingWindow>*	CDockState::GetFloating(void) { return &maFloating; }
int32							CDockState::GetNextGroupID(void) { return miNextGroupID; }
void							CDockState::SetNextGroupID(int32 iID) { miNextGroupID = iID; }
Ptr<CDockGroup>					CDockState::GetDocumentGroup(void) { return mpDocumentGroup; }
void							CDockState::SetDocumentGroup(Ptr<CDockGroup> pGroup) { mpDocumentGroup = pGroup; }
CDockManagerListener*			CDockState::GetListener(void) { return mpcListener; }
void							CDockState::SetListener(CDockManagerListener* pcListener) { mpcListener = pcListener; }
Ptr<CDockDrag>					CDockState::GetDrag(void) { return mpDrag; }
Ptr<CDockFocus>					CDockState::GetFocus(void) { return mpFocus; }
Ptr<CDockPane>					CDockState::GetMenuPane(void) { return mpMenuPane; }
void							CDockState::SetMenuPane(Ptr<CDockPane> pPane) { mpMenuPane = pPane; }
CDockTree*						CDockState::GetTree(void) { return &mcTree; }
CDockRestorer*					CDockState::GetRestorer(void) { return &mcRestorer; }
CDockMover*						CDockState::GetMover(void) { return &mcMover; }
