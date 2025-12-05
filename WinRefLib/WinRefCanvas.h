#ifndef __WIN_Ref_CANVAS_H__
#define __WIN_Ref_CANVAS_H__
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
#include "StandardLib/Pointer.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageDraw.h"
#include "WindowLib/NativeCanvas.h"


class CWinRefCanvas : public CNativeCanvas
{
CONSTRUCTABLE(CWinRefCanvas);
protected:
	Ptr<CImage>		mpImage;
	CImageDraw		mcDraw;
	ARGB32			msLastColour;

public:
	void			Init(CCanvas* pcCanvas, CNativeWindowFactory* pcWindowFactory);
	void			Kill(void) override;

	bool			CreateNativeCanvas(void) override;

	uint8*			GetPixelData(void);
	Ptr<CImage>		GetImage(void);

	void			CopyCanvas(CNativeCanvas* pcSourceCanvas) override;
	void			DrawBox(CRectangle* pcRectangle, bool bFilled, ARGB32 sColour) override;
	void			DrawPixel(int32 iX, int32 iY, ARGB32 sColour) override;
	void			DrawCanvas(int iX, int iY, CNativeCanvas* pcSource) override;

protected:
	void			SetColour(ARGB32 sColour);
};


#endif // __WIN_Ref_CANVAS_H__

