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
#ifndef __DOCK_GUIDES_H__
#define __DOCK_GUIDES_H__
#include "BaseLib/ArrayTemplate.h"
#include "Canvas.h"
#include "CanvasDraw.h"
#include "DockManager.h"




//One square of the docking guides: where it is, in window client space, and what dropping on it does.  The node is a
//plain pointer; the guides only last while a drag is over their area.
struct SDockGuide
{
	CInt32Rect			cRect;
	EDockTargetKind		eKind;
	EDockSide			eSide;
	CDockNode*			pcNode;
	bool				bOuter;  //The document well's outer ring, beside the whole well.
};


typedef CArrayTemplate<SDockGuide>	CArrayDockGuide;


class CDockGuides;


class CDockGuidesDraw : public CCanvasDraw
{
CONSTRUCTABLE(CDockGuidesDraw);
DESTRUCTABLE(CDockGuidesDraw);
protected:
	CDockGuides*	mpcGuides;  //Not held; the guides hold this.

public:
	void	Init(CDockGuides* pcGuides);
	void	Class(void) override;
	bool	Draw(Ptr<CCanvas> pCanvas) override;
};


//Visual Studio's docking guides, drawn over a dock area while something is dragged over it: a marker in the middle of
//each of the area's edges, to dock along that whole edge; a compass over the group under the pointer, its centre to
//tab into the group and its arrows to dock beside it; over the document well a second ring, to dock beside the whole
//well; and a translucent preview of where the guide under the pointer would put what is dragged.  What is shown
//depends on what is dragged: documents can only go into the document well.
class CDockGuides : public CCanvas
{
CONSTRUCTABLE(CDockGuides);
DESTRUCTABLE(CDockGuides);
protected:
	Ptr<CDockManager>	mpManager;
	Ptr<CDockArea>		mpArea;
	CArrayDockGuide		masGuides;
	size				muiHover;  //The guide under the pointer, MAX_SIZE for none.
	CInt32Rect			mcPreview;
	bool				mbPreview;
	CDockNode*			mpcCompassNode;  //What the compass is over.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, Ptr<CDockArea> pArea);
	void				Class(void) override;
	void				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	SInt32Vec2			GetDesiredSize(void) override;
	void				Build(int x, int y, bool bDocuments, SInt32Vec2 sDraggedSize);
	bool				Hit(int x, int y, SDockTarget* psTarget, SInt32Vec2 sDraggedSize);
	size				NumGuides(void);
	SDockGuide*			GetGuide(size uiIndex);
	SDockGuide*			FindGuide(EDockTargetKind eKind, EDockSide eSide, CDockNode* pcNode, bool bOuter);
	void				DrawGuides(Ptr<CCanvas> pCanvas);

protected:
	void				AddGuide(int x, int y, EDockTargetKind eKind, EDockSide eSide, CDockNode* pcNode, bool bOuter);
	CDockNode*			FindCompassNode(int x, int y, bool* pbInWell);
	void				AddEdgeMarkers(void);
	void				AddCompassFor(CDockNode* pcNode, bool bInWell, bool bDocuments);
	void				AddCompass(CInt32Rect* pcBounds, EDockTargetKind eSideKind, EDockTargetKind eCentreKind, CDockNode* pcNode, bool bSides, bool bCentre, int iRing, bool bOuter);
	void				PreviewFor(SDockGuide* psGuide, SInt32Vec2 sDraggedSize, CInt32Rect* pcDest);
	void				DrawGuide(Ptr<CCanvas> pCanvas, SDockGuide* psGuide, bool bHover);
	void				DrawArrow(Ptr<CCanvas> pCanvas, CInt32Rect* pcRect, EDockSide eSide, ARGB32 sColour);
	void				DrawBoxClipped(Ptr<CCanvas> pCanvas, int iLeft, int iTop, int iRight, int iBottom, bool bFilled, ARGB32 sColour);
};


void SideRectangle(CInt32Rect* pcBounds, EDockSide eSide, int32 iSize, CInt32Rect* pcDest);


#endif // __DOCK_GUIDES_H__

