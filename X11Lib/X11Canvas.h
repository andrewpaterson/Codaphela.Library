#ifndef __X11_CANVAS_H__
#define __X11_CANVAS_H__
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
#include "StandardLib/Pointer.h"
#include "SupportLib/Image.h"
#include "WindowLib/NativeCanvas.h"


//Drawn in memory only; the window puts the pixels to the X server when it presents.  Laid out as the GDI canvas' DIB
//section is: B, G, R, X in 32 bits.
class CX11Canvas : public CNativeCanvas
{
CONSTRUCTABLE(CX11Canvas);
protected:
	uint8*			mpuiPixelData;
	Ptr<CImage>		mpImage;  //Wraps mpuiPixelData, it does not own the memory.

public:
	void			Init(CCanvas* pcCanvas, CNativeWindowFactory* pcWindowFactory);
	void			Kill(void) override;

	bool			CreateNativeCanvas(void) override;

	uint8*			GetPixelData(void);
	using			CNativeCanvas::GetSize;

	void			CopyCanvas(CNativeCanvas* pcSourceCanvas);
	Ptr<CImage>		GetImageOrNull(void);

	void			DrawBox(CInt32Rect* pcRectangle, bool bFilled, ARGB32 sColour);
	void			DrawPixel(int32 iX, int32 iY, ARGB32 sColour);
	void			DrawCanvas(int iX, int iY, CNativeCanvas* pcSource, CInt32Rect* pcClip);
	void			DrawCel(int iX, int iY, Ptr<CImageCel> pSource);
	void			DrawImage(int iX, int iY, Ptr<CImage> pSource);

protected:
	void			Fill(int iLeft, int iTop, int iRight, int iBottom, ARGB32 sColour);
};


#endif // __X11_CANVAS_H__

