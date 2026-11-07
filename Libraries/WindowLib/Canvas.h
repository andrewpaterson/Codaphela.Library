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
#ifndef __CANVAS_H__
#define __CANVAS_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/Pointer.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Int32Rect.h"
#include "SupportLib/Image.h"
#include "Component.h"
#include "CanvasDraw.h"


class CNativeCanvas;
class CContainer;
class CCanvas : public CComponent
{
CONSTRUCTABLE(CCanvas);
DESTRUCTABLE(CCanvas);
friend class CNativeCanvas;
protected:
	CNativeCanvas*		mpcNativeCanvas;
	Ptr<CImage>			mpNativeImage;  //The native canvas's pixels as an image, when it has one (CNativeCanvas::SetImage).  Held here, not by the native canvas, so the image is in the object graph.
	CColourFormatHelper	mcFormat;  //As made; GetColourFormat and GetAlphaBits give the format with opacity while it is transparent.
	Ptr<CCanvasDraw>	mpCanvasDraw;
	Ptr<CContainer>		mpContainer;
	bool				mbTransparent;  //What is behind the canvas shows through wherever it hasn't been drawn opaquely.
	SInt32Vec2			msNativeOrigin;  //Where the native canvas's top left is in the canvas.  The native canvas only holds the part that shows, see UpdateNativeArea.
	SInt32Vec2			msNativeSize;  //The size the native canvas should be.
	bool				mbContentDirty;  //The canvas draws itself again at the next paint; until then its native canvas holds what it last drew.
	bool				mbClearBeforeDraw;  //The native canvas holds what was drawn for another part of the canvas, see PrepareArea.
	bool				mbPaintClipped;  //Only the window's canvas, while a partial paint draws mcPaintClip of it.
	CInt32Rect			mcPaintClip;  //In the canvas's space, right and bottom exclusive.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CCanvasDraw> pDraw, CColourFormatHelper* pcFormat);
	void				Init(Ptr<CWindow> pWindow, Ptr<CCanvasDraw> pDraw);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	EColourFormat		GetColourFormat(void);
	EColourOrder		GetColourOrder(void);
	ERGBColourBits		GetColourBits(void);
	ERGBAlphaBits		GetAlphaBits(void);
	void				GetColourFormat(CColourFormatHelper* pcDest);
	bool				IsValid(void);

	bool				SetTransparent(bool bTransparent);
	bool				IsTransparent(void);

	uint8*				GetPixelData(void);

	CNativeCanvas*		GetNativeCanvas(void);
	Ptr<CCanvasDraw>	GetCanvasDraw(void);
	void				SetCanvasDraw(Ptr<CCanvasDraw> pDraw);
	bool				HasNativeChanged(void);
	bool				HasArea(void);
	bool				IsSolidInWindow(Ptr<CCanvas> pWindowCanvas, ARGB32* psColour);
	void				UpdateNativeArea(void);
	void				UpdateNativeArea(bool bClipped, CInt32Rect* pcClip);
	bool				IsContentDirty(void);
	SInt32Vec2			GetNativeOrigin(void);
	SInt32Vec2			GetNativeSize(void);
	bool				PrepareNativeCanvas(SInt32Vec2 sSize);

	bool				SetContainer(Ptr<CContainer> pContainer);
	Ptr<CContainer>		GetContainer(void);
	bool				ClearContainer(void);

	bool				Draw(void) override;

	void				CopyCanvas(Ptr<CCanvas> pSourceCanvas);
	Ptr<CImage>			GetImageOrNull(void);  //The native canvas's pixels as an image, NULL if it has none (a canvas draw then blits into an image of its own).  Its top left is GetNativeOrigin in the canvas.

	void				DrawCanvas(int iX, int iY, Ptr<CCanvas> pSourceCanvas, CInt32Rect* pcClip = NULL);  //Into the native canvas's own space.
	void				DrawBox(CInt32Rect* pcRect, bool bFilled, ARGB32 sColour);  //These three are in the canvas's space and cut to its native canvas.
	void				DrawPixel(int iX, int iY, ARGB32 sColour);
	void				DrawImage(int iX, int iY, Ptr<CImage> pImage);
	void				SetPaintClip(CInt32Rect* pcClip);
	void				ClearPaintClip(void);
	bool				GetPaintClip(CInt32Rect* pcDest);

	SInt32Vec2			GetDesiredSize(void) override;

	void				CreateNativeCanvas(void);
	void				DestroyNativeCanvas(void);

protected:
	void				PrepareArea(bool bDrawn, bool bClipped, CInt32Rect* pcClip, CInt32Rect* pcArea) override;
	void				MarkContentChanged(void) override;
	EColourFormat		GetTransparentColourFormat(void);
	void				DrawFilledBoxClipped(int iLeft, int iTop, int iRight, int iBottom, ARGB32 sColour);
	void				GetDrawableBounds(CInt32Rect* pcDest);
};


#endif // __CANVAS_H__

