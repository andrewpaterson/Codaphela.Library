#ifndef __SCROLL_BAR_IMAGES_H__
#define __SCROLL_BAR_IMAGES_H__
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
#include "StandardLib/Array.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageCel.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/ColourFormat.h"


enum EScrollBarOrientation
{
	SBO_Horizontal,
	SBO_Vertical,

	NUM_SCROLL_BAR_ORIENTATIONS
};


//The three parts of a scroll bar's thumb, from left to right or top to bottom.
enum EScrollBarPiece
{
	SBP_Start,  //Half a circle.
	SBP_Middle,  //A bar one pixel long, stretched to the length of the thumb.
	SBP_End,  //Half a circle.

	NUM_SCROLL_BAR_PIECES
};


class CImageDraw;

//The pieces of a scroll bar's thumb in both orientations, drawn in one colour with CImageDraw rather than loaded.
//Scroll bars can share them.  The colour's opacity makes the thumb translucent; the pieces are drawn over what is
//being scrolled.  The ends are half circles as wide as the thumb is thick so the thumb has round ends.  The pieces
//are made in the colour order of the format given, RGB if there is none, as they are blitted onto the window's canvases.
class CScrollBarImages : public CObject
{
CONSTRUCTABLE(CScrollBarImages);
DESTRUCTABLE(CScrollBarImages);
protected:
	CArray<CImageCel>	maCels;  //NUM_SCROLL_BAR_PIECES for each orientation in turn.
	int32				miThickness;
	ARGB32				msColour;
	EColourOrder		meOrder;

public:
	void			Init(int32 iThickness, ARGB32 sColour, CColourFormatHelper* pcFormat = NULL);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	Ptr<CImageCel>	GetCel(EScrollBarOrientation eOrientation, EScrollBarPiece ePiece);
	int32			GetThickness(void);
	int32			GetCapLength(void);
	ARGB32			GetColour(void);

protected:
	Ptr<CImageCel>	CreateCap(EScrollBarOrientation eOrientation, EScrollBarPiece ePiece);
	Ptr<CImageCel>	CreateMiddle(EScrollBarOrientation eOrientation);
	Ptr<CImage>		CreateImage(int32 iWidth, int32 iHeight);
	Ptr<CImageCel>	CreateCel(Ptr<CImage> pImage);
	void			SetColour(CImageDraw* pcDraw);
};


#endif // __SCROLL_BAR_IMAGES_H__

