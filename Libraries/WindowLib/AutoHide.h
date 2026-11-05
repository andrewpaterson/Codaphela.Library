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
#ifndef __AUTO_HIDE_H__
#define __AUTO_HIDE_H__
#include "SupportLib/Font.h"
#include "SupportLib/SpriteMap.h"
#include "SupportLib/Maps.h"
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "Label.h"
#include "DockEdge.h"
#include "DockTitleBar.h"
#include "DockNode.h"


#define AUTO_HIDE_STRIP_SIZE	24	//An auto hide strip's thickness.
#define AUTO_HIDE_TAB_GAP		10	//Before and after a strip tab's text.
#define AUTO_HIDE_TAB_SPACING	4	//Between a strip's tabs.
#define AUTO_HIDE_LINE			3	//A strip tab's line along the strip's outer side.
#define AUTO_HIDE_EDGE			4	//A flyout's inner edge, dragged to resize it.


//One line of text read top to bottom, drawn with a font turned clockwise (FT_Clockwise): Visual Studio's auto hide tabs
//on the left and right edges.  Its required size is the line height across and the text's length down.
class CTurnedLabel : public CCanvas
{
CONSTRUCTABLE(CTurnedLabel);
DESTRUCTABLE(CTurnedLabel);
protected:
	CChars				mszText;
	Ptr<CFont>			mpFont;
	Ptr<CSpriteMap>		mpSpriteMap;
	Ptr<CMaps>			mpMaps;
	SInt32Vec2			msRequiredSize;

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CFont> pTurnedFont, const char* szText);
	void			Class(void) override;
	void 			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;
	void			SetText(const char* szText);
	char*			GetText(void);
	SInt32Vec2		GetDesiredSize(void) override;

protected:
	void			LayoutText(void);
};


class CAutoHideStrip;
class CDockPane;


//A pane's tab on an auto hide strip: its title, upright on the top and bottom strips and turned on the left and right
//(with the manager's turned font; upright without one), and a line along the strip's outer side.  Pressing it, or
//resting the pointer on it, slides the pane out.
class CAutoHideTab : public CContainer
{
CONSTRUCTABLE(CAutoHideTab);
DESTRUCTABLE(CAutoHideTab);
protected:
	Ptr<CDockManager>		mpManager;
	Ptr<CDockPane>			mpPane;
	EDockSide				meSide;
	Ptr<CClearCanvasDraw>	mpBackgroundDraw;
	Ptr<CCanvas>			mpBackground;
	Ptr<CClearCanvasDraw>	mpLineDraw;
	Ptr<CCanvas>			mpLine;
	Ptr<CCanvas>			mpText;  //A CLabel or a CTurnedLabel.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, Ptr<CDockPane> pPane, EDockSide eSide);
	void				Class(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	void				SetRequiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	Ptr<CDockPane>		GetPane(void);
	bool				IsVertical(void);
	int32				GetLength(void);
	void				TitleChanged(void);
	void				FocusOrMouseOverChanged(void) override;
	void				ActivateComponent(int x, int y);
	void				Hover(void);
	void				PopupClosed(Ptr<CComponent> pContent) override;

protected:
	void				UpdateLook(void);
};


//The tabs of the panes auto hidden on one edge of the main window, in order along it.
class CAutoHideStrip : public CContainer
{
CONSTRUCTABLE(CAutoHideStrip);
DESTRUCTABLE(CAutoHideStrip);
protected:
	Ptr<CDockManager>		mpManager;
	EDockSide				meSide;
	Ptr<CCanvas>			mpBackground;
	CArray<CAutoHideTab>	maTabs;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, EDockSide eSide);
	void				Class(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	void				SetRequiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	Ptr<CAutoHideTab>	AddPane(Ptr<CDockPane> pPane);
	bool				RemovePane(Ptr<CDockPane> pPane);
	bool				HasTabs(void);
	size				NumTabs(void);
	Ptr<CAutoHideTab>	GetTab(size uiIndex);
	Ptr<CAutoHideTab>	FindTab(Ptr<CDockPane> pPane);
	EDockSide			GetSide(void);
};


//An auto hidden pane slid out over the main window from its strip: a title bar with the pin (to dock it back) and the
//close cross over the pane's content, and an edge on the side away from the strip that is dragged to resize it.  It is
//a popup, so a press anywhere else slides it back.
class CAutoHideFlyout : public CContainer, public CDockEdgeOwner
{
CONSTRUCTABLE(CAutoHideFlyout);
DESTRUCTABLE(CAutoHideFlyout);
protected:
	Ptr<CDockManager>		mpManager;
	Ptr<CDockPane>			mpPane;
	EDockSide				meSide;
	Ptr<CDockTitleBar>		mpTitleBar;
	Ptr<CDockEdge>			mpEdge;
	Ptr<CCanvas>			mpBackground;
	int32					miStartSize;
	SInt32Vec2				msStartPointer;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, Ptr<CDockPane> pPane);
	void				Class(void) override;
	void				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	void				SetRequiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	Ptr<CDockPane>		GetPane(void);
	Ptr<CDockTitleBar>	GetTitleBar(void);
	Ptr<CDockEdge>		GetEdge(void);
	EDockSide			GetSide(void);
	void				Place(CInt32Rect* pcRootBounds, SInt32Vec2* psPosition);
	void				Detach(void);
	void				PinPressed(CDockTitleBar* pcTitleBar);
	void				ClosePressed(CDockTitleBar* pcTitleBar);
	void				EdgeDragStarted(CDockEdge* pcEdge, SInt32Vec2 sScreen) override;
	void				EdgeDragged(CDockEdge* pcEdge, SInt32Vec2 sScreen) override;
	void				EdgeReleased(CDockEdge* pcEdge) override;
};


#endif // __AUTO_HIDE_H__

