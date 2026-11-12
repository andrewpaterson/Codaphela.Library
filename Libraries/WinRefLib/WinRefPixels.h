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
#ifndef __WIN_REF_PIXELS_H__
#define __WIN_REF_PIXELS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "SupportLib/Int32Rect.h"


//Pixels the reference screen keeps or reads: what a window has shown, the screen's own, or a view of a canvas's image.
//Plain memory rather than a CImage, as the native windows holding them aren't objects.  Every pixel is the same number
//of bytes; a view's rows can be further apart than its width (an image wider than the canvas shows).
class CWinRefPixels
{
protected:
	uint8*	mpuiData;
	int32	miWidth;
	int32	miHeight;
	size	muiPixelBytes;
	lsize	muiRowBytes;
	bool	mbOwned;  //False for a view of pixels held elsewhere.

public:
	void			Init(void);
	void			Init(void* pvData, int32 iWidth, int32 iHeight, size uiPixelBytes, lsize uiRowBytes);
	void			Kill(void);

	void			SetSize(int32 iWidth, int32 iHeight, size uiPixelBytes);
	void			Clear(void);

	void			Copy(CWinRefPixels* pcSource, CInt32Rect* pcSourceArea, int32 iDestX, int32 iDestY);
	uint64			Hash(CInt32Rect* pcArea, uint64 ulliHash);
	lsize			FindDifference(CWinRefPixels* pcOther, CInt32Rect* pcDiffers);

	int32			GetWidth(void);
	int32			GetHeight(void);
	size			GetPixelBytes(void);
	void			GetArea(CInt32Rect* pcDest);
	uint8*			GetPixel(int32 x, int32 y);
};


#endif // __WIN_REF_PIXELS_H__

