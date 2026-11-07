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
	InitCanvas(pWindow, pDraw);

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
	InitCanvas(pWindow, pDraw);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// What both Inits set up, after the colour format.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::InitCanvas(Ptr<CWindow> pWindow, Ptr<CCanvasDraw> pDraw)
{
	mpcNativeCanvas = NULL;
	CComponent::Init(pWindow);

	mpCanvasDraw = pDraw;
	mbTransparent = false;
	msNativeOrigin.Zero();
	msNativeSize.Zero();
	mbContentDirty = true;
	mcPartDamage.Init();
	mbClearBeforeDraw = false;
	mbPaintClipped = false;
	mcPaintClip.Init(0, 0, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Free(void)
{
	DestroyNativeCanvas();
	mcPartDamage.Kill();

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
	U_Bool(mbContentDirty);
	U_Data(CDamageArea, mcPartDamage);
	U_Bool(mbClearBeforeDraw);
	U_Bool(mbPaintClipped);
	U_Data(CInt32Rect, mcPaintClip);
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
	if (mpcNativeCanvas == NULL)
	{
		return NULL;
	}
	return mpcNativeCanvas->GetPixelData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::Draw(void)
{
	Ptr<CCanvas>			pWindowCanvas;
	ARGB32					sColour;
	CInt32Rect				cArea;
	CInt32Rect				cClip;

	//The window's own canvas draws its background into itself.
	pWindowCanvas = mpWindow->GetCanvas();
	if (&pWindowCanvas == this)
	{
		if (HasNativeChanged())
		{
			DestroyNativeCanvas();
			CreateNativeCanvas();
		}
		if ((mpcNativeCanvas != NULL) && mpCanvasDraw)
		{
			mpCanvasDraw->Draw(this);
		}
		mbContentDirty = false;
		return CComponent::Draw();
	}

	//A canvas that shows nowhere, like a label with no text or one scrolled out of sight, draws nothing, nor does one
	//outside what a partial paint draws.  Its children might still show.
	if ((mcDrawnArea.GetWidth() <= 0) || (mcDrawnArea.GetHeight() <= 0))
	{
		return CComponent::Draw();
	}
	if (pWindowCanvas->GetPaintClip(&cClip) && !mcDrawnArea.Intersect(&cClip))
	{
		return CComponent::Draw();
	}

	//A canvas of one opaque colour fills the part of the window it shows in: it needs no native canvas to fill and copy.
	if (IsSolidInWindow(pWindowCanvas, &sColour))
	{
		cArea.Init(mcDrawnArea.GetLeft(), mcDrawnArea.GetTop(), mcDrawnArea.GetRight() - 1, mcDrawnArea.GetBottom() - 1);
		pWindowCanvas->DrawBox(&cArea, true, sColour);
		mbContentDirty = false;
		return CComponent::Draw();
	}

	if (HasNativeChanged())
	{
		DestroyNativeCanvas();
		CreateNativeCanvas();
		mbContentDirty = true;
		mbClearBeforeDraw = false;
	}
	if (mpcNativeCanvas == NULL)
	{
		return CComponent::Draw();
	}

	//The native canvas keeps what the canvas last drew, so it is only drawn again where that changed.
	if (mbContentDirty)
	{
		DrawAll();
	}
	else if (!mcPartDamage.IsEmpty())
	{
		DrawParts();
	}

	//The canvas is copied onto the window before its children so that they are drawn over it.
	pWindowCanvas->DrawCanvas(mcDrawnArea.GetLeft(), mcDrawnArea.GetTop(), this);
	return CComponent::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
// The part of the canvas that will show this paint, given what clips it,
// becomes its native area, and where in the window that is its drawn
// area.  A native canvas showing another part of the canvas than it did,
// scrolled say, draws again; an opaque one first starts black, as a new
// one would.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::PrepareArea(bool bDrawn, bool bClipped, CInt32Rect* pcClip, CInt32Rect* pcArea)
{
	SInt32Vec2		sPreviousOrigin;
	SInt32Vec2		sPreviousSize;
	ARGB32			sColour;

	sPreviousOrigin = msNativeOrigin;
	sPreviousSize = msNativeSize;
	UpdateNativeArea(bClipped, pcClip);
	if (!HasArea())
	{
		DestroyNativeCanvas();
	}
	if (!bDrawn || !HasArea())
	{
		pcArea->Init(0, 0, 0, 0);
		return;
	}
	pcArea->Init(mcActualBounds.GetLeft() + msNativeOrigin.x, mcActualBounds.GetTop() + msNativeOrigin.y,
				 mcActualBounds.GetLeft() + msNativeOrigin.x + msNativeSize.x, mcActualBounds.GetTop() + msNativeOrigin.y + msNativeSize.y);

	if (IsSolidInWindow(mpWindow->GetCanvas(), &sColour))
	{
		DestroyNativeCanvas();
		return;
	}
	if ((sPreviousOrigin.x != msNativeOrigin.x) || (sPreviousOrigin.y != msNativeOrigin.y) || (sPreviousSize.x != msNativeSize.x) || (sPreviousSize.y != msNativeSize.y))
	{
		//Scrolled, it shows another part of itself in the same place.
		mbContentDirty = true;
		mpWindow->AddDamage(pcArea);
		if (!mbTransparent && (sPreviousSize.x == msNativeSize.x) && (sPreviousSize.y == msNativeSize.y))
		{
			mbClearBeforeDraw = true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The canvas draws all of itself again.  A transparent canvas starts
// empty so that only what is drawn this time shows over what is behind
// it.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawAll(void)
{
	if (mbTransparent || mbClearBeforeDraw)
	{
		mpcNativeCanvas->Clear();
	}
	if (mpCanvasDraw)
	{
		mpCanvasDraw->Draw(this);
	}
	mbContentDirty = false;
	mbClearBeforeDraw = false;
	mcPartDamage.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// The canvas draws again only the parts of it that changed
// (InvalidateArea), each cleared if it is transparent and drawn with the
// part as its paint clip.  A draw that can't keep to a clip draws all of
// it.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawParts(void)
{
	CInt32Rect	cNative;
	CInt32Rect	cPart;
	CInt32Rect	cClear;
	size		i;

	if (mpCanvasDraw.IsNull() || !mpCanvasDraw->CanDrawPart(this))
	{
		DrawAll();
		return;
	}

	cNative.Init(msNativeOrigin.x, msNativeOrigin.y, msNativeOrigin.x + msNativeSize.x, msNativeOrigin.y + msNativeSize.y);
	for (i = 0; i < mcPartDamage.NumRectangles(); i++)
	{
		cPart.Init(mcPartDamage.GetRectangle(i));
		cPart.ClipTo(&cNative);
		if ((cPart.GetWidth() <= 0) || (cPart.GetHeight() <= 0))
		{
			continue;
		}
		if (mbTransparent)
		{
			cClear.Init(cPart.GetLeft() - msNativeOrigin.x, cPart.GetTop() - msNativeOrigin.y, cPart.GetRight() - msNativeOrigin.x, cPart.GetBottom() - msNativeOrigin.y);
			mpcNativeCanvas->Clear(&cClear);
		}
		SetPaintClip(&cPart);
		mpCanvasDraw->Draw(this);
		ClearPaintClip();
	}
	mcPartDamage.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// Part of the canvas, pcRect in its space (right and bottom exclusive),
// must be drawn again: only that part is, if the canvas's draw can draw
// part (CCanvasDraw::CanDrawPart), and only that part of the window
// shown again.  Invalidate is all of it.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::InvalidateArea(CInt32Rect* pcRect)
{
	DamagePart(pcRect);
	if (mpWindow.IsNotNull())
	{
		mpWindow->RequestPaint();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// As InvalidateArea, without asking for a paint: for a canvas that finds
// what of it changed while the window is painted (PrepareArea).
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DamagePart(CInt32Rect* pcRect)
{
	CInt32Rect	cWindow;

	if ((pcRect->GetWidth() <= 0) || (pcRect->GetHeight() <= 0))
	{
		return;
	}
	if (!mbContentDirty)
	{
		mcPartDamage.Add(pcRect);
	}
	if (mpWindow.IsNotNull())
	{
		cWindow.Init(pcRect->GetLeft() + mcActualBounds.GetLeft(), pcRect->GetTop() + mcActualBounds.GetTop(), pcRect->GetRight() + mcActualBounds.GetLeft(), pcRect->GetBottom() + mcActualBounds.GetTop());
		cWindow.ClipTo(&mcDrawnArea);
		if ((cWindow.GetWidth() > 0) && (cWindow.GetHeight() > 0))
		{
			mpWindow->AddDamage(&cWindow);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::MarkContentChanged(void)
{
	mbContentDirty = true;
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
// Whether the canvas is drawn as one opaque colour, psColour, straight
// into pWindowCanvas.  Not the window's own canvas, nor a canvas in
// another format than the window's, which can't be drawn onto it.
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::IsSolidInWindow(Ptr<CCanvas> pWindowCanvas, ARGB32* psColour)
{
	if (mbTransparent || mpCanvasDraw.IsNull() || (&pWindowCanvas == this) || pWindowCanvas.IsNull())
	{
		return false;
	}
	if ((GetColourFormat() != pWindowCanvas->GetColourFormat()) || (GetColourOrder() != pWindowCanvas->GetColourOrder()) ||
		(GetColourBits() != pWindowCanvas->GetColourBits()) || (GetAlphaBits() != pWindowCanvas->GetAlphaBits()))
	{
		return false;
	}
	return mpCanvasDraw->GetSolidColour(psColour);
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
	CInt32Rect	cClip;
	bool		bClipped;

	bClipped = GetClip(&cClip);
	UpdateNativeArea(bClipped, &cClip);
}


//////////////////////////////////////////////////////////////////////////
//
// As UpdateNativeArea, with what clips the canvas given (pcClip in window
// space, when bClipped) rather than found from its ancestors.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::UpdateNativeArea(bool bClipped, CInt32Rect* pcClip)
{
	CInt32Rect	cVisible;

	cVisible.Init(&mcActualBounds);
	if (bClipped)
	{
		cVisible.ClipTo(pcClip);
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
	CInt32Rect		cClip;

	if (mpcNativeCanvas == NULL)
	{
		return;
	}
	pcSourceNativeCanvas = pSourceCanvas->GetNativeCanvas();
	if (!mbPaintClipped)
	{
		mpcNativeCanvas->DrawCanvas(iX, iY, pcSourceNativeCanvas, pcClip);
		return;
	}

	//A partial paint only draws inside its clip.
	cClip.Init(&mcPaintClip);
	if (pcClip)
	{
		cClip.ClipTo(pcClip);
	}
	if ((cClip.GetWidth() > 0) && (cClip.GetHeight() > 0))
	{
		mpcNativeCanvas->DrawCanvas(iX, iY, pcSourceNativeCanvas, &cClip);
	}
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
	int			iBottom;
	CInt32Rect	cDrawable;

	if (mpcNativeCanvas == NULL)
	{
		return;
	}

	//The rectangle includes its right and bottom edges.  An outline cut by the native canvas's edge is drawn as its four sides so the cut edges aren't drawn.
	iLeft = pcRect->msTopLeft.x - msNativeOrigin.x;
	iTop = pcRect->msTopLeft.y - msNativeOrigin.y;
	iRight = pcRect->msBottomRight.x - msNativeOrigin.x;
	iBottom = pcRect->msBottomRight.y - msNativeOrigin.y;
	GetDrawableBounds(&cDrawable);
	if (bFilled)
	{
		DrawFilledBoxClipped(iLeft, iTop, iRight, iBottom, sColour);
	}
	else if ((iLeft >= cDrawable.GetLeft()) && (iTop >= cDrawable.GetTop()) && (iRight < cDrawable.GetRight()) && (iBottom < cDrawable.GetBottom()))
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
	CInt32Rect	cDrawable;

	GetDrawableBounds(&cDrawable);
	if (iLeft < cDrawable.GetLeft())
	{
		iLeft = cDrawable.GetLeft();
	}
	if (iTop < cDrawable.GetTop())
	{
		iTop = cDrawable.GetTop();
	}
	if (iRight >= cDrawable.GetRight())
	{
		iRight = cDrawable.GetRight() - 1;
	}
	if (iBottom >= cDrawable.GetBottom())
	{
		iBottom = cDrawable.GetBottom() - 1;
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
	CInt32Rect	cDrawable;

	if (mpcNativeCanvas == NULL)
	{
		return;
	}
	iX -= msNativeOrigin.x;
	iY -= msNativeOrigin.y;
	GetDrawableBounds(&cDrawable);
	if (cDrawable.ContainsPoint(iX, iY))
	{
		mpcNativeCanvas->DrawPixel(iX, iY, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What of the native canvas may be drawn on, in its space, right and
// bottom exclusive: all of it, or what a partial paint's clip leaves.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::GetDrawableBounds(CInt32Rect* pcDest)
{
	CInt32Rect	cClip;

	pcDest->Init(0, 0, msNativeSize.x, msNativeSize.y);
	if (mbPaintClipped)
	{
		cClip.Init(mcPaintClip.GetLeft() - msNativeOrigin.x, mcPaintClip.GetTop() - msNativeOrigin.y, mcPaintClip.GetRight() - msNativeOrigin.x, mcPaintClip.GetBottom() - msNativeOrigin.y);
		pcDest->ClipTo(&cClip);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The window's canvas, while a partial paint draws only pcClip of it (in
// the canvas's space): nothing drawn on it touches anything outside.
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::SetPaintClip(CInt32Rect* pcClip)
{
	mbPaintClipped = true;
	mcPaintClip.Init(pcClip);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::ClearPaintClip(void)
{
	mbPaintClipped = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::GetPaintClip(CInt32Rect* pcDest)
{
	if (mbPaintClipped)
	{
		pcDest->Init(&mcPaintClip);
	}
	return mbPaintClipped;
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
		//The native canvas has a different format so it is made again and drawn when next painted.
		mbTransparent = bTransparent;
		DestroyNativeCanvas();
		Invalidate();
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
void CCanvas::SetCanvasDraw(Ptr<CCanvasDraw> pDraw) { mpCanvasDraw = pDraw; Invalidate(); InvalidateLayout(); }
bool CCanvas::IsContentDirty(void) { return mbContentDirty; }
Ptr<CContainer> CCanvas::GetContainer(void) { return mpContainer; }

