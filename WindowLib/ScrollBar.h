#ifndef __SCROLL_BAR_H__
#define __SCROLL_BAR_H__
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
#include "Container.h"
#include "Canvas.h"
#include "ImageCanvasDraw.h"
#include "ScrollBarImages.h"


//A scroll pane's scroll bar in the style of IntelliJ IDEA: only a translucent thumb, drawn over what is being
//scrolled, with no track or arrows.  The thumb is three canvases: a half circle at each end and a bar stretched
//between them.  The thumb's length shows how much of the content is in view and its place how far it is scrolled.
//
//The scroll bar is laid out to its track, the strip along the side of the scroll pane the thumb moves in.  The whole
//track is hit, even though only the thumb is drawn.  Pressing the thumb and dragging it scrolls the scroll pane.
//Pressing the track elsewhere jumps the thumb's middle to the pointer and drags it from there.  A scroll bar is only
//shown, and only hit, while the content is longer than the view.
class CScrollPane;
class CScrollBar : public CContainer
{
CONSTRUCTABLE(CScrollBar);
DESTRUCTABLE(CScrollBar);
protected:
	Ptr<CScrollBarImages>	mpImages;
	EScrollBarOrientation	meOrientation;
	Ptr<CScrollPane>		mpScrollPane;
	Ptr<CCanvas>			mpStart;
	Ptr<CCanvas>			mpMiddle;
	Ptr<CCanvas>			mpEnd;
	int32					miContentLength;
	int32					miViewLength;
	int32					miPosition;
	int32					miThumbStart;  //From the start of the track.
	int32					miThumbLength;
	bool					mbDragging;
	int32					miGrab;  //Where along the thumb it was pressed.

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CScrollPane> pScrollPane, Ptr<CScrollBarImages> pImages, EScrollBarOrientation eOrientation);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			SetRange(int32 iContentLength, int32 iViewLength, int32 iPosition);
	bool			IsShown(void);
	void			GetThumbBounds(CRectangle* pcDest);
	EScrollBarOrientation	GetOrientation(void);
	bool			IsDragging(void);

	void			SetRequiredSize(void) override;
	void			Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool			Draw(void) override;
	Ptr<CComponent>	FindComponentAt(int x, int y) override;

	void			FocusLost(void) override;
	void			ActivateComponent(int x, int y) override;
	void			ExecuteComponent(int x, int y) override;
	void			StartDragScroll(int x, int y) override;
	void			ContDragScroll(int x, int y) override;
	void			StopDragScroll(int x, int y) override;

protected:
	Ptr<CCanvas>	CreatePiece(Ptr<CWindow> pWindow, EScrollBarPiece ePiece);
	int32			GetTrackLength(void);
	int32			GetMinimumThumbLength(void);
	int32			Along(int x, int y);
	bool			IsInThumb(int x, int y);
	void			MoveThumb(int32 iThumbStart);
	void			LayoutPiece(Ptr<CCanvas> pPiece, int32 iStart, int32 iLength);
};


#endif // __SCROLL_BAR_H__

