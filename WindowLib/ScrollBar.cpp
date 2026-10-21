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
#include "StandardLib/Objects.h"
#include "Window.h"
#include "ScrollPane.h"
#include "ScrollBar.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::Init(Ptr<CWindow> pWindow, Ptr<CScrollPane> pScrollPane, Ptr<CScrollBarImages> pImages, EScrollBarOrientation eOrientation)
{
	PreInit();

	CContainer::Init(pWindow);
	mpImages = pImages;
	meOrientation = eOrientation;
	mpScrollPane = pScrollPane;
	miContentLength = 0;
	miViewLength = 0;
	miPosition = 0;
	miThumbStart = 0;
	miThumbLength = 0;
	mbDragging = false;
	miGrab = 0;

	//The thumb is pressed and dragged so it takes focus, which is how the drag reaches it.
	SetCanGetFocus(true);

	mpStart = CreatePiece(pWindow, SBP_Start);
	mpMiddle = CreatePiece(pWindow, SBP_Middle);
	mpEnd = CreatePiece(pWindow, SBP_End);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::Class(void)
{
	CContainer::Class();

	M_Pointer(mpImages);
	U_Enum(meOrientation);
	M_Pointer(mpScrollPane);
	M_Pointer(mpStart);
	M_Pointer(mpMiddle);
	M_Pointer(mpEnd);
	U_Int32(miContentLength);
	U_Int32(miViewLength);
	U_Int32(miPosition);
	U_Int32(miThumbStart);
	U_Int32(miThumbLength);
	U_Bool(mbDragging);
	U_Int32(miGrab);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::Free(void)
{
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScrollBar::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScrollBar::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Each piece is a transparent canvas so what is scrolled shows through
// the translucent thumb and around its round ends.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvas> CScrollBar::CreatePiece(Ptr<CWindow> pWindow, EScrollBarPiece ePiece)
{
	Ptr<CImageCanvasDraw>	pDraw;
	Ptr<CCanvas>			pCanvas;

	pDraw = OMalloc<CImageCanvasDraw>(mpImages->GetCel(meOrientation, ePiece), ePiece == SBP_Middle);
	pCanvas = OMalloc<CCanvas>(pWindow, pDraw);
	pCanvas->SetTransparent(true);
	AddComponent(pCanvas);
	return pCanvas;
}


//////////////////////////////////////////////////////////////////////////
//
// The scroll pane sets the range before laying the scroll bar out.  The
// lengths and position are along the scroll bar's axis.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::SetRange(int32 iContentLength, int32 iViewLength, int32 iPosition)
{
	miContentLength = iContentLength;
	miViewLength = iViewLength;
	miPosition = iPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScrollBar::IsShown(void)
{
	return miContentLength > miViewLength;
}


//////////////////////////////////////////////////////////////////////////
//
// A scroll bar is as thick as its thumb.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::SetRequiredSize(void)
{
	if (meOrientation == SBO_Horizontal)
	{
		msRequiredSize.Init(0, mpImages->GetThickness());
	}
	else
	{
		msRequiredSize.Init(mpImages->GetThickness(), 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CScrollBar::GetTrackLength(void)
{
	if (meOrientation == SBO_Horizontal)
	{
		return mcActualBounds.GetWidth();
	}
	return mcActualBounds.GetHeight();
}


//////////////////////////////////////////////////////////////////////////
//
// Long enough to show both round ends and some bar between them.
//
//////////////////////////////////////////////////////////////////////////
int32 CScrollBar::GetMinimumThumbLength(void)
{
	return mpImages->GetThickness() * 2;
}


//////////////////////////////////////////////////////////////////////////
//
// The thumb is as much of the track as the view is of the content and is
// as far along the space left as the position is along the content left
// out of view.  The arithmetic is integer so every build places the thumb
// on the same pixels.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	int32	iTrackLength;
	int32	iScrollRange;
	int32	iCapLength;

	SetContainerBounds(sPosition, sAreaSize);
	SetActualBounds(sPosition, sAreaSize);

	iTrackLength = GetTrackLength();
	if (!IsShown() || (iTrackLength <= 0))
	{
		miThumbStart = 0;
		miThumbLength = 0;
		return;
	}

	miThumbLength = (int32)(((int64)iTrackLength * miViewLength) / miContentLength);
	if (miThumbLength < GetMinimumThumbLength())
	{
		miThumbLength = GetMinimumThumbLength();
	}
	if (miThumbLength > iTrackLength)
	{
		miThumbLength = iTrackLength;
	}

	iScrollRange = miContentLength - miViewLength;
	miThumbStart = (int32)(((int64)(iTrackLength - miThumbLength) * miPosition) / iScrollRange);

	iCapLength = mpImages->GetCapLength();
	LayoutPiece(mpStart, miThumbStart, iCapLength);
	LayoutPiece(mpMiddle, miThumbStart + iCapLength, miThumbLength - iCapLength * 2);
	LayoutPiece(mpEnd, miThumbStart + miThumbLength - iCapLength, iCapLength);
}


//////////////////////////////////////////////////////////////////////////
//
// iStart is from the start of the track.  Every piece is as thick as the
// track.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::LayoutPiece(Ptr<CCanvas> pPiece, int32 iStart, int32 iLength)
{
	SInt32Vec2	sPosition;
	SInt32Vec2	sSize;

	sPosition = GetPosition();
	sSize = GetActualSize();
	if (meOrientation == SBO_Horizontal)
	{
		sPosition.x += iStart;
		sSize.x = iLength;
	}
	else
	{
		sPosition.y += iStart;
		sSize.y = iLength;
	}
	pPiece->Layout(sPosition, sSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScrollBar::Draw(void)
{
	if (!IsShown() || (miThumbLength <= 0))
	{
		return true;
	}
	return CContainer::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
// In window client space.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::GetThumbBounds(CRectangle* pcDest)
{
	SInt32Vec2	sPosition;
	SInt32Vec2	sSize;

	sPosition = GetPosition();
	sSize = GetActualSize();
	if (meOrientation == SBO_Horizontal)
	{
		pcDest->Init(sPosition.x + miThumbStart, sPosition.y, sPosition.x + miThumbStart + miThumbLength, sPosition.y + sSize.y);
	}
	else
	{
		pcDest->Init(sPosition.x, sPosition.y + miThumbStart, sPosition.x + sSize.x, sPosition.y + miThumbStart + miThumbLength);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are in window client space.  The whole track is hit, not just
// the thumb, so it can be clicked.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CScrollBar::FindComponentAt(int x, int y)
{
	if (!IsShown() || (miThumbLength <= 0))
	{
		return NULL;
	}

	if (IsPointIn(x, y))
	{
		return this;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the scroll bar's top left.
//
//////////////////////////////////////////////////////////////////////////
int32 CScrollBar::Along(int x, int y)
{
	if (meOrientation == SBO_Horizontal)
	{
		return x;
	}
	return y;
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the scroll bar's top left.
//
//////////////////////////////////////////////////////////////////////////
bool CScrollBar::IsInThumb(int x, int y)
{
	int32	iAlong;

	if (!IsShown() || (miThumbLength <= 0))
	{
		return false;
	}
	iAlong = Along(x, y);
	return (iAlong >= miThumbStart) && (iAlong < miThumbStart + miThumbLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Pressing the thumb starts dragging it.  Pressing the track elsewhere
// first jumps the thumb so its middle is under the pointer.  The press has
// already given the scroll bar focus.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::ActivateComponent(int x, int y)
{
	if (!IsShown() || (miThumbLength <= 0))
	{
		return;
	}

	if (!IsInThumb(x, y))
	{
		MoveThumb(Along(x, y) - miThumbLength / 2);
	}
	mpWindow->GetFocus()->StartDrag(FD_Scroll, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the thumb, kept on the track, and scrolls the scroll pane to
// match.  The scroll pane works out where the thumb is again when it is
// next laid out.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::MoveThumb(int32 iThumbStart)
{
	int32	iThumbRange;
	int32	iScrollRange;
	int32	iPosition;

	iThumbRange = GetTrackLength() - miThumbLength;
	iScrollRange = miContentLength - miViewLength;
	if ((iThumbRange <= 0) || (iScrollRange <= 0))
	{
		return;
	}

	if (iThumbStart < 0)
	{
		iThumbStart = 0;
	}
	if (iThumbStart > iThumbRange)
	{
		iThumbStart = iThumbRange;
	}
	miThumbStart = iThumbStart;

	iPosition = (int32)(((int64)iThumbStart * iScrollRange + iThumbRange / 2) / iThumbRange);
	mpScrollPane->SetScrollPosition(meOrientation, iPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// The release goes to the focussed scroll bar wherever the pointer is.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::ExecuteComponent(int x, int y)
{
	if (mbDragging)
	{
		mpWindow->GetFocus()->StopDrag(FD_Scroll);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The thumb keeps the point it was grabbed at under the pointer.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::StartDragScroll(int x, int y)
{
	if (IsInThumb(x, y))
	{
		mbDragging = true;
		miGrab = Along(x, y) - miThumbStart;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the thumb to the pointer and scrolls the scroll pane to match.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::ContDragScroll(int x, int y)
{
	if (mbDragging)
	{
		MoveThumb(Along(x, y) - miGrab);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::StopDragScroll(int x, int y)
{
	mbDragging = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Losing focus, as when the window is deactivated, ends a drag.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBar::FocusLost(void)
{
	if (mbDragging)
	{
		mpWindow->GetFocus()->StopDrag(FD_Scroll);
	}
	mbDragging = false;
}


EScrollBarOrientation CScrollBar::GetOrientation(void) { return meOrientation; }
bool CScrollBar::IsDragging(void) { return mbDragging; }

