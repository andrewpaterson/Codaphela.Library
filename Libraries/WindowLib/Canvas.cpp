/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "NativeWindowFactory.h"
#include "Window.h"
#include "NativeCanvas.h"
#include "Canvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Init(Ptr<CWindow> pWindow, Ptr<CCanvasDraw> pDraw, CColourFormatHelper* pcFormat)
{
	PreInit();

	mcFormat = *pcFormat;

	mpcNativeCanvas = NULL;
	CComponent::Init(pWindow);

	mpCanvasDraw = pDraw;
	mbTransparent = false;
	msNativeOrigin.Zero();
	msNativeSize.Zero();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Init(Ptr<CWindow> pWindow, Ptr<CCanvasDraw> pDraw)
{
	PreInit();

	pWindow->GetColourFormat(&mcFormat);

	mpcNativeCanvas = NULL;
	CComponent::Init(pWindow);

	mpCanvasDraw = pDraw;
	mbTransparent = false;
	msNativeOrigin.Zero();
	msNativeSize.Zero();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Free(void)
{
	DestroyNativeCanvas();

	mcFormat.Init(CFT_Unknown, CCO_Unknown, CRGB_Unknown, ARGB_Unknown);

	CComponent::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Class(void)
{
	CComponent::Class();

	U_Pointer(mpcNativeCanvas);
	U_Data(CColourFormatHelper, mcFormat);
	M_Pointer(mpNativeImage);
	M_Pointer(mpCanvasDraw);
	M_Pointer(mpContainer);
	U_Bool(mbTransparent);
	U_2Int32(msNativeOrigin);
	U_2Int32(msNativeSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CCanvas::GetPixelData(void)
{
	return mpcNativeCanvas->GetPixelData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::Draw(void)
{
	Ptr<CCanvas>			pWindowCanvas;
	SInt32Vec2				sPreviousOrigin;

	sPreviousOrigin = msNativeOrigin;
	UpdateNativeArea();
	if (HasNativeChanged())
	{
		DestroyNativeCanvas();
		CreateNativeCanvas();
	}
	else if ((mpcNativeCanvas != NULL) && !mbTransparent && ((sPreviousOrigin.x != msNativeOrigin.x) || (sPreviousOrigin.y != msNativeOrigin.y)))
	{
		//What the native canvas holds was drawn for another part of the canvas.  It starts black, as a new one would.
		mpcNativeCanvas->Clear();
	}

	//A canvas with no area, like a label with no text, may have no native canvas.  Only its children are drawn.
	if (mpcNativeCanvas == NULL)
	{
		CComponent::Draw();
		return true;
	}

	//A transparent canvas starts each draw empty so that only what is drawn this time shows over what is behind it.
	if (mbTransparent)
	{
		mpcNativeCanvas->Clear();
	}

	//The canvas draws itself and is copied onto the window before its children so that they are drawn over it.
	if (mpCanvasDraw)
	{
		mpCanvasDraw->Draw(this);
	}

	//The native canvas holds only the part of the canvas that shows, so all of it is copied.
	pWindowCanvas = mpWindow->GetCanvas();
	if (&pWindowCanvas != this)
	{
		pWindowCanvas->DrawCanvas(mcActualBounds.GetLeft() + msNativeOrigin.x, mcActualBounds.GetTop() + msNativeOrigin.y, this);
	}

	CComponent::Draw();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A canvas with no area has no native canvas.  The factory isn't asked
// for one it could only fail to make.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::CreateNativeCanvas(void)
{
	CNativeWindowFactory* pcFactory;

	if (!HasArea())
	{
		mpcNativeCanvas = NULL;
		return;
	}

	pcFactory = mpWindow->GetFactory();
	mpcNativeCanvas = pcFactory->CreateNativeCanvas(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The native canvas goes back to the factory that made it.  It is asked
// for that, not the window: the window can be killed before the canvas.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DestroyNativeCanvas(void)
{
	CNativeWindowFactory* pcFactory;

	if (mpcNativeCanvas)
	{
		pcFactory = mpcNativeCanvas->GetFactory();
		pcFactory->DestroyNativeCanvas(mpcNativeCanvas);
		mpcNativeCanvas = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// For a canvas that isn't drawn into the window, but drawn on and from,
// as the window's caret keeps what is under it: it is sSize, with a
// native canvas.  False if it can't have one.
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::PrepareNativeCanvas(SInt32Vec2 sSize)
{
	SetActualSize(sSize);
	UpdateNativeArea();
	if (HasNativeChanged())
	{
		DestroyNativeCanvas();
		CreateNativeCanvas();
	}
	return mpcNativeCanvas != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the native canvas must be made again.  A canvas with no area and
// no native canvas is unchanged, so it isn't made again on every draw.
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::HasNativeChanged(void)
{
	SInt32Vec2	sSize;

	if (mpcNativeCanvas == NULL)
	{
		return HasArea();
	}

	sSize = mpcNativeCanvas->GetSize();
	if ((msNativeSize.x != sSize.x) ||
		(msNativeSize.y != sSize.y))
	{
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::CopyCanvas(Ptr<CCanvas> pSourceCanvas)
{
	CNativeCanvas*	pcSourceNativeCanvas;
	CNativeCanvas*	pcDestNativeCanvas;
	Ptr<CContainer>	pContainer;

	pcDestNativeCanvas = GetNativeCanvas();
	pcSourceNativeCanvas = pSourceCanvas->GetNativeCanvas();
	pContainer = pSourceCanvas->GetContainer();

	if (pcDestNativeCanvas)
	{
		pcDestNativeCanvas->CopyCanvas(pcSourceNativeCanvas);
	}

	SetContainer(pContainer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CCanvas::GetImageOrNull(void)
{
	if (mpcNativeCanvas == NULL)
	{
		return NULL;
	}
	return mpcNativeCanvas->GetImageOrNull();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::HasArea(void)
{
	return (msNativeSize.x > 0) && (msNativeSize.y > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The part of the canvas its native canvas holds: the part that shows
// through the components that clip it (see GetClip), like a scroll pane.
// A long document in a scroll pane only has a native canvas the size of
// the view.  Nothing shows of a canvas scrolled out of sight, so it has
// no native canvas.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::UpdateNativeArea(void)
{
	CInt32Rect	cVisible;
	CInt32Rect	cClip;

	cVisible.Init(&mcActualBounds);
	if (GetClip(&cClip))
	{
		cVisible.ClipTo(&cClip);
	}

	msNativeSize.Init(cVisible.GetWidth(), cVisible.GetHeight());
	if ((msNativeSize.x <= 0) || (msNativeSize.y <= 0))
	{
		msNativeSize.Zero();
		msNativeOrigin.Zero();
	}
	else
	{
		msNativeOrigin.Init(cVisible.GetLeft() - mcActualBounds.GetLeft(), cVisible.GetTop() - mcActualBounds.GetTop());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawCanvas(int iX, int iY, Ptr<CCanvas> pSourceCanvas, CInt32Rect* pcClip)
{
	CNativeCanvas*	pcSourceNativeCanvas;

	pcSourceNativeCanvas = pSourceCanvas->GetNativeCanvas();
	mpcNativeCanvas->DrawCanvas(iX, iY, pcSourceNativeCanvas, pcClip);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawBox(CInt32Rect* pcRect, bool bFilled, ARGB32 sColour)
{
	int		iLeft;
	int		iTop;
	int		iRight;
	int		iBottom;

	if (mpcNativeCanvas == NULL)
	{
		return;
	}

	//The rectangle includes its right and bottom edges.  An outline cut by the native canvas's edge is drawn as its four sides so the cut edges aren't drawn.
	iLeft = pcRect->msTopLeft.x - msNativeOrigin.x;
	iTop = pcRect->msTopLeft.y - msNativeOrigin.y;
	iRight = pcRect->msBottomRight.x - msNativeOrigin.x;
	iBottom = pcRect->msBottomRight.y - msNativeOrigin.y;
	if (bFilled)
	{
		DrawFilledBoxClipped(iLeft, iTop, iRight, iBottom, sColour);
	}
	else if ((iLeft >= 0) && (iTop >= 0) && (iRight < msNativeSize.x) && (iBottom < msNativeSize.y))
	{
		CInt32Rect	cRect;

		cRect.Init(iLeft, iTop, iRight, iBottom);
		mpcNativeCanvas->DrawBox(&cRect, false, sColour);
	}
	else
	{
		DrawFilledBoxClipped(iLeft, iTop, iRight, iTop, sColour);
		DrawFilledBoxClipped(iLeft, iBottom, iRight, iBottom, sColour);
		DrawFilledBoxClipped(iLeft, iTop + 1, iLeft, iBottom - 1, sColour);
		DrawFilledBoxClipped(iRight, iTop + 1, iRight, iBottom - 1, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// In the native canvas's space, right and bottom included.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawFilledBoxClipped(int iLeft, int iTop, int iRight, int iBottom, ARGB32 sColour)
{
	CInt32Rect	cRect;

	if (iLeft < 0)
	{
		iLeft = 0;
	}
	if (iTop < 0)
	{
		iTop = 0;
	}
	if (iRight >= msNativeSize.x)
	{
		iRight = msNativeSize.x - 1;
	}
	if (iBottom >= msNativeSize.y)
	{
		iBottom = msNativeSize.y - 1;
	}
	if ((iLeft > iRight) || (iTop > iBottom))
	{
		return;
	}
	cRect.Init(iLeft, iTop, iRight, iBottom);
	mpcNativeCanvas->DrawBox(&cRect, true, sColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawPixel(int iX, int iY, ARGB32 sColour)
{
	if (mpcNativeCanvas == NULL)
	{
		return;
	}
	iX -= msNativeOrigin.x;
	iY -= msNativeOrigin.y;
	if ((iX >= 0) && (iY >= 0) && (iX < msNativeSize.x) && (iY < msNativeSize.y))
	{
		mpcNativeCanvas->DrawPixel(iX, iY, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawImage(int iX, int iY, Ptr<CImage> pImage)
{
	if (mpcNativeCanvas == NULL)
	{
		return;
	}
	mpcNativeCanvas->DrawImage(iX - msNativeOrigin.x, iY - msNativeOrigin.y, pImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::IsValid(void)
{
	if (mcFormat.meFormat == CFT_Unknown)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::SetContainer(Ptr<CContainer> pContainer)
{
	if (!mpContainer)
	{
		if (pContainer)
		{
			AddComponent(pContainer);
		}
		mpContainer = pContainer;
		return true;
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Container is already set on Canvas.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::ClearContainer(void)
{
	bool	bResult;

	if (mpContainer)
	{
		bResult = RemoveComponent(mpContainer);
		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " Container is set on Canvas but is not a child component.", NULL);
		}
		mpContainer = NULL;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CCanvas::GetDesiredSize(void)
{
	//A desired size of -1, -1 on the canvas means use the size its draw wants.
	if ((msDesiredSize.x == -1) && (msDesiredSize.y == -1) && mpCanvasDraw.IsNotNull())
	{
		return mpCanvasDraw->GetDesiredSize();
	}
	return msDesiredSize;
}


//////////////////////////////////////////////////////////////////////////
//
// A canvas is opaque unless it is made transparent; blending it onto what
// is behind is slower than copying it.  A transparent canvas has an 8 bit
// opacity channel and its colours are multiplied by its opacity, so only
// canvases with 8 bit colour channels can be transparent.
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::SetTransparent(bool bTransparent)
{
	if (bTransparent && (mcFormat.meColourBits != CRGB_24bit))
	{
		return gcLogger.Error2(__METHOD__, " Only a canvas with 8 bit colour channels can be transparent.", NULL);
	}

	if (mbTransparent != bTransparent)
	{
		//The native canvas has a different format so it is made again when next drawn.
		mbTransparent = bTransparent;
		DestroyNativeCanvas();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The format with an opacity channel where the canvas's format has one or
// an ignored channel.
//
//////////////////////////////////////////////////////////////////////////
EColourFormat CCanvas::GetTransparentColourFormat(void)
{
	if ((mcFormat.meFormat == CFT_XRGB) || (mcFormat.meFormat == CFT_ARGB))
	{
		return CFT_ARGB;
	}
	return CFT_RGBA;
}


//////////////////////////////////////////////////////////////////////////
//
// The format of the canvas's image, with an opacity channel if the canvas
// is transparent.
//
//////////////////////////////////////////////////////////////////////////
EColourFormat CCanvas::GetColourFormat(void)
{
	if (mbTransparent)
	{
		return GetTransparentColourFormat();
	}
	return mcFormat.meFormat;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ERGBAlphaBits CCanvas::GetAlphaBits(void)
{
	if (mbTransparent)
	{
		return ARGB_8bit;
	}
	return mcFormat.meAlphaBits;
}


//////////////////////////////////////////////////////////////////////////
//
// The format of the canvas's image, as the four getters give it.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::GetColourFormat(CColourFormatHelper* pcDest)
{
	pcDest->Init(GetColourFormat(), GetColourOrder(), GetColourBits(), GetAlphaBits());
}


EColourOrder CCanvas::GetColourOrder(void) { return mcFormat.meOrder; }
ERGBColourBits CCanvas::GetColourBits(void) { return mcFormat.meColourBits; }
bool CCanvas::IsTransparent(void) { return mbTransparent; }
CNativeCanvas* CCanvas::GetNativeCanvas(void) { return mpcNativeCanvas; }
SInt32Vec2 CCanvas::GetNativeOrigin(void) { return msNativeOrigin; }
SInt32Vec2 CCanvas::GetNativeSize(void) { return msNativeSize; }
Ptr<CCanvasDraw> CCanvas::GetCanvasDraw(void) { return mpCanvasDraw; }
void CCanvas::SetCanvasDraw(Ptr<CCanvasDraw> pDraw) { mpCanvasDraw = pDraw; InvalidateLayout(); }
Ptr<CContainer> CCanvas::GetContainer(void) { return mpContainer; }

