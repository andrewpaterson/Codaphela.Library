#ifndef __WIN_GDI_CANVAS_H__
#define __WIN_GDI_CANVAS_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include <windows.h>
#include "StandardLib/Pointer.h"
#include "SupportLib/Image.h"
#include "WindowLib/NativeCanvas.h"


//A canvas held in an image that owns its pixels (B8G8R8X8, the layout of a 32 bit BI_RGB DIB).  Everything is drawn on its
//pixels directly, never with GDI: GDI queues its drawing and would have to be flushed before every direct write, and there
//are hundreds a frame.  The only GDI that reads them is the window's present (Paint), after the frame's drawing is done.
//The image owns the memory (not GDI) so whoever holds the image after the canvas is resized or freed holds valid pixels.
class CWinGDICanvas : public CNativeCanvas
{
CONSTRUCTABLE(CWinGDICanvas);
protected:
	uint8*			mpuiPixelData;  //The image's pixels.  The canvas holds the image (CNativeCanvas::SetImage).

public:
	void			Init(CCanvas* pcCanvas, CNativeWindowFactory* pcWindowFactory);
	void			Kill(void) override;

	bool			CreateNativeCanvas(void) override;

	uint8*			GetPixelData(void);

	void			Paint(HDC hDC, int32 iWidth, int32 iHeight);

	void			CopyCanvas(CNativeCanvas* pcSourceCanvas);
	Ptr<CImage>		GetImageOrNull(void);

	void			DrawBox(CInt32Rect* pcRectangle, bool bFilled, ARGB32 sColour);
	void			DrawPixel(int32 iX, int32 iY, ARGB32 sColour);
	void			DrawCanvas(int iX, int iY, CNativeCanvas* pcSource, CInt32Rect* pcClip);
	void			DrawCel(int iX, int iY, Ptr<CImageCel> pSource);
	void			DrawImage(int iX, int iY, Ptr<CImage> pSource);

protected:
	uint32			GetPixel(ARGB32 sColour);
	void			FillPixels(int iLeft, int iTop, int iRight, int iBottom, uint32 uiPixel);
};


#endif // __WIN_GDI_CANVAS_H__

