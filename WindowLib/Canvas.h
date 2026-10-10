#ifndef __CANVAS_H__
#define __CANVAS_H__
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
#include "StandardLib/Unknown.h"
#include "StandardLib/Pointer.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Rectangle.h"
#include "SupportLib/Image.h"
#include "Component.h"
#include "CanvasDraw.h"


class CNativeCanvas;
class CContainer;
class CCanvas : public CComponent
{
CONSTRUCTABLE(CCanvas);
DESTRUCTABLE(CCanvas);
protected:
	CNativeCanvas*		mpcNativeCanvas;
	EColourFormat		meColourFormat;
	EColourOrder        meColourOrder;
	ERGBColourBits      meColourBits;
	ERGBAlphaBits       meAlphaBits;
	Ptr<CCanvasDraw>	mpCanvasDraw;
	Ptr<CContainer>		mpContainer;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CCanvasDraw> pDraw, EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits);
	void				Init(Ptr<CWindow> pWindow, Ptr<CCanvasDraw> pDraw);
	void				Class(void) override;
	void 				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	EColourFormat		GetColourFormat(void);
	EColourOrder		GetColourOrder(void);
	ERGBColourBits		GetColourBits(void);
	ERGBAlphaBits		GetAlphaBits(void);
	bool				IsValid(void);

	uint8*				GetPixelData(void);

	CNativeCanvas*		GetNativeCanvas(void);
	Ptr<CCanvasDraw>	GetCanvasDraw(void);
	bool				HasNativeChanged(void);

	bool				SetContainer(Ptr<CContainer> pContainer);
	Ptr<CContainer>		GetContainer(void);
	bool				ClearContainer(void);

	bool				Draw(void) override;

	void				CopyCanvas(Ptr<CCanvas> pSourceCanvas);
	Ptr<CImage>			GetImageOrNull(void);  //This should be GetBits or something with the goal being to get chunk of memory that an image can be backed with to avoid a double image copy.

	void				DrawCanvas(int iX, int iY, Ptr<CCanvas> pSourceCanvas);
	void				DrawBox(CRectangle* pcRect, bool bFilled, ARGB32 sColour);
	void				DrawPixel(int iX, int iY, ARGB32 sColour);
	void				DrawImage(int iX, int iY, Ptr<CImage> pImage);

	void				SetRequiredSize(void) override;

	void				CreateNativeCanvas(void);
	void				DestroyNativeCanvas(void);
};


#endif // __CANVAS_H__

