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
#ifndef __DOCK_MANAGER_H__
#define __DOCK_MANAGER_H__
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Font.h"
#include "SupportLib/ImageCel.h"
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"
#include "WindowTheme.h"
#include "DockNode.h"
#include "DockPane.h"
#include "DockGroup.h"
#include "DockTree.h"
#include "DockRestorer.h"
#include "DockMover.h"
#include "DockFocus.h"


enum EDockImage
{
	DI_Menu,		//The window position menu's chevron.
	DI_Pinned,
	DI_Unpinned,
	DI_Close,
	DI_NumImages,
};


//What a drop would do: dock at the edge of a dock area, beside a group or tab into it, beside the document well, or
//into the empty document well.
enum EDockTargetKind
{
	DTK_None,
	DTK_AreaEdge,
	DTK_Group,
	DTK_WellOuter,
	DTK_WellCentre,
};


class CDockArea;
class CDocumentWell;
class CDockFloatingWindow;
class CDockDragTracker;
class CDockDrag;
class CDockGuides;
class CAutoHideFlyout;
class CMarkupTag;
class CXMLWriter;
class CFocus;


//Where something would be docked and the preview of it, in the target window's client space.  Plain pointers: it is
//only kept while a drag is going on, by the manager that holds what they point to.
struct SDockTarget
{
	EDockTargetKind		eKind;
	EDockSide			eSide;
	CDockArea*			pcArea;
	CDockNode*			pcNode;  //The group, or the document well.
	CInt32Rect			cPreview;

	void	Init(void);
	bool	IsValid(void);
	bool	Equals(SDockTarget* psOther);
};


//What the application is told by the dock manager.  The listener isn't held; it must outlive the manager or be
//removed (SetListener(NULL)).
class CDockManagerListener
{
public:
	virtual bool			PaneClosing(CDockPane* pcPane);  //A document is about to close; false keeps it.
	virtual void			PaneClosed(CDockPane* pcPane);  //A document was closed and removed, or a tool hidden.
	virtual void			ActivePaneChanged(CDockPane* pcPane);  //NULL when no pane is active.
	virtual void			DocumentSelected(CDockPane* pcPane);  //The document shown in front changed.
	virtual Ptr<CDockPane>	CreatePane(const char* szID);  //A layout names a pane that isn't added; NULL drops it.
	virtual void			LayoutChanged(void);
};


//Visual Studio's docking: tool panes and documents in groups of tabs, split beside each other and around the document
//well, floated in windows of their own, auto hidden on the main window's edges, or hidden; dragged between all of
//these with docking guides showing where they would go; and the layout written to and read from an XML file.  See
//WindowLib/Docking.txt.
//
//The application puts the manager's dock area (GetArea) in its window, adds its panes (AddPane) and listens for what
//happens to them (SetListener).  Everything that moves panes goes through the manager: the panes' menus, their title
//bars and tabs, dragging, the layout file, and the application calling the API.
class CDockManager : public CObject
{
CONSTRUCTABLE(CDockManager);
DESTRUCTABLE(CDockManager);
friend class CDockDrag;
friend class CDockTree;
friend class CDockRestorer;
friend class CDockMover;
friend class CDockFocus;
friend class CDockLayoutWriter;
friend class CDockLayoutReader;
protected:
	Ptr<CWindow>				mpWindow;  //The main window.
	Ptr<CWindowTheme>			mpTheme;
	Ptr<CFont>					mpFont;
	Ptr<CFont>					mpTurnedFont;  //For the left and right auto hide strips, NULL for upright text.
	CArray<CImageCel>			maImages;  //By EDockImage.
	Ptr<CDockArea>				mpArea;
	Ptr<CDocumentWell>			mpWell;
	CArray<CDockPane>			maPanes;
	CArray<CDockFloatingWindow>	maFloating;
	int32						miNextGroupID;
	Ptr<CDockGroup>				mpDocumentGroup;  //The well's group documents are added to.
	CDockManagerListener*		mpcListener;

	Ptr<CDockDrag>				mpDrag;
	Ptr<CDockFocus>				mpFocus;  //The active pane and document, and the flyout.
	CDockTree					mcTree;  //The trees' edits.
	CDockRestorer				mcRestorer;  //Where panes go back to.
	CDockMover					mcMover;  //Floating, docking and auto hiding.

	CInt32Rect					mcLayoutWindow;  //The main window's client area on the screen, from the last layout read.
	bool						mbLayoutWindow;

public:
	void						Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTurnedFont = NULL);
	void						Class(void) override;
	void						Free(void) override;
	bool						Save(CObjectWriter* pcFile) override;
	bool						Load(CObjectReader* pcFile) override;

	Ptr<CDockArea>				GetArea(void);
	Ptr<CDocumentWell>			GetWell(void);
	Ptr<CWindow>				GetWindow(void);
	void						SetListener(CDockManagerListener* pcListener);
	Ptr<CWindowTheme>			GetTheme(void);
	Ptr<CFont>					GetFont(void);
	Ptr<CFont>					GetTurnedFont(void);
	ARGB32						GetColour(EDockColour eColour);
	void						SetColour(EDockColour eColour, ARGB32 sColour);
	Ptr<CImageCel>				GetImage(EDockImage eImage);

	//Panes.
	Ptr<CDockPane>				AddPane(const char* szID, const char* szTitle, Ptr<CImageCel> pIcon, Ptr<CComponent> pContent, EDockPaneKind eKind, SDockPlace* psDefault = NULL, bool bShow = true);
	bool						RemovePane(Ptr<CDockPane> pPane);
	Ptr<CDockPane>				GetPane(const char* szID);
	size						NumPanes(void);
	Ptr<CDockPane>				GetPane(size uiIndex);
	Ptr<CDockPane>				FindPane(Ptr<CComponent> pComponent);

	//Moving panes: shown, hidden, floated, docked and auto hidden.
	void						Show(Ptr<CDockPane> pPane);
	void						Activate(Ptr<CDockPane> pPane);
	void						Hide(Ptr<CDockPane> pPane);
	bool						Close(Ptr<CDockPane> pPane);
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
	bool						DockPaneAt(Ptr<CDockPane> pPane, SDockTarget* psTarget);
	bool						DockNodeAt(Ptr<CDockNode> pNode, SDockTarget* psTarget);

	//The active pane and document, and the commands on them.
	Ptr<CDockPane>				GetActivePane(void);
	Ptr<CDockPane>				GetActiveDocument(void);
	void						CloseActiveDocument(void);
	void						HideActiveTool(void);

	//Floating windows.
	size						NumFloating(void);
	Ptr<CDockFloatingWindow>	GetFloating(size uiIndex);
	Ptr<CDockFloatingWindow>	FindFloating(Ptr<CDockPane> pPane);
	void						RemoveFloating(Ptr<CDockFloatingWindow> pFloating);
	void						FloatingWindowClosed(CWindow* pcWindow);

	//Groups and the document well, for the layout file and tests.
	Ptr<CDockGroup>				FindGroup(int32 iID);
	Ptr<CDockGroup>				CreateGroup(bool bDocuments, int32 iID = 0);
	void						PlaceInGroup(Ptr<CDockGroup> pGroup, Ptr<CDockPane> pPane, size uiIndex = MAX_SIZE);
	void						AddToWell(Ptr<CDockPane> pPane, Ptr<CDockGroup> pGroup = NULL);
	bool						IsInWell(Ptr<CDockNode> pNode);
	void						SetNextGroupID(int32 iID);
	int32						GetNextGroupID(void);

	//Called by the groups, title bars and tabs as the user acts on them.
	void						PaneSelected(Ptr<CDockPane> pPane);
	void						PaneChanged(Ptr<CDockPane> pPane);
	void						MenuChosen(Ptr<CDockPane> pPane, EDockMenuItem eItem);
	void						LayoutChanged(void);

	//Dragging, called by the title bars, tabs and the drag tracker.
	void						BeginPaneDrag(Ptr<CDockPane> pPane);
	void						BeginGroupDrag(Ptr<CDockGroup> pGroup);
	void						BeginFloatingDrag(Ptr<CDockFloatingWindow> pFloating);
	void						SetDragNoDock(bool bNoDock);
	bool						IsDragging(void);
	SDockTarget*				GetTarget(void);
	Ptr<CDockDrag>				GetDrag(void);

	//Focus and the auto hide flyout.
	void						WatchFocus(Ptr<CWindow> pWindow);
	void						UnwatchFocus(Ptr<CWindow> pWindow);
	void						RestoreFocus(Ptr<CWindow> pWindow);
	void						ShowFlyout(Ptr<CDockPane> pPane);
	void						CloseFlyout(void);
	void						FlyoutClosed(void);
	Ptr<CAutoHideFlyout>		GetFlyout(void);

	//The layout.
	bool						SaveLayout(const char* szFileName);
	bool						LoadLayout(const char* szFileName);
	void						WriteLayout(CXMLWriter* pcWriter);
	bool						ReadLayout(CMarkupTag* pcRoot);
	void						ResetLayout(void);
	bool						GetLayoutWindowRectangle(CInt32Rect* pcDest);

protected:
	void						CreateImages(void);
	void						TakeOut(Ptr<CDockPane> pPane, bool bRecordRestore);
	void						SetStates(Ptr<CDockNode> pNode, EDockPaneState eState);
	void						ClearLayout(void);
};


#endif // __DOCK_MANAGER_H__

