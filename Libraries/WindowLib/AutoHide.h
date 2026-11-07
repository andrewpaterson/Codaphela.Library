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




//A label whose one line of text reads top to bottom, drawn with a font turned clockwise (FT_Clockwise): Visual Studio's
//auto hide tabs on the left and right edges.  Its required size is the line height across and the text's length down.
class CTurnedLabel : public CLabel
{
CONSTRUCTABLE(CTurnedLabel);
DESTRUCTABLE(CTurnedLabel);
public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CFont> pTurnedFont, const char* szText);
	void			Class(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

protected:
	void			LayoutText(void) override;
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
	Ptr<CLabel>				mpText;  //A CTurnedLabel on a side strip with a turned font.

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
	char*				GetText(void);
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


#endif // __AUTO_HIDE_H__

