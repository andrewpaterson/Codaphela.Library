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
#ifndef __JEWEL_BUTTON_IMAGES_H__
#define __JEWEL_BUTTON_IMAGES_H__
#include "SupportLib/ColourFormat.h"
#include "ButtonImages.h"
#include "CheckBoxImages.h"


class CImageDraw;

//Draws the button images of JetBrains' Jewel dark theme with CImageDraw rather than loading them.  Each state's border
//is a strip of 8 x 8 tiles laid out as CButtonImages expects, or 7 x 7 tiles for a flat button.  From the outside in each tile has a two pixel focus
//ring (empty except when selected), a one pixel border and then the face, with corners six, four and three round.
class CJewelButtonImages
{
public:
	static Ptr<CButtonImages>	CreateButtonImages(CColourFormatHelper* pcFormat);  //The outlined button.
	static Ptr<CButtonImages>	CreateDefaultButtonImages(CColourFormatHelper* pcFormat);  //The blue filled default button.
	static Ptr<CButtonImages>	CreateFlatButtonImages(ARGB32 sBackground, ARGB32 sHover, ARGB32 sPushed, CColourFormatHelper* pcFormat);  //No border, a small rounded block under the pointer.
	static Ptr<CImageCel>		CreateChevron(ARGB32 sColour, CColourFormatHelper* pcFormat);  //Points right, 8 x 9.
	static Ptr<CImageCel>		CreateDownChevron(ARGB32 sColour, CColourFormatHelper* pcFormat);  //Points down, 9 x 8.
	static Ptr<CImageCel>		CreateCross(ARGB32 sColour, CColourFormatHelper* pcFormat);  //A close cross, 16 x 16.
	static Ptr<CImageCel>		CreateKeyboard(ARGB32 sColour, CColourFormatHelper* pcFormat);  //An outlined keyboard, 16 x 16.
	static Ptr<CCheckBoxImages>	CreateCheckBoxImages(CColourFormatHelper* pcFormat);  //Each box is 20 x 20.

protected:
	static Ptr<CImage>			DrawStrip(bool bFocusRing, ARGB32 sFocusRing, ARGB32 sBorder, ARGB32 sFace, CColourFormatHelper* pcFormat);
	static Ptr<CImage>			DrawFlatStrip(bool bFilled, ARGB32 sFill, CColourFormatHelper* pcFormat);
	static Ptr<CImageCel>		DrawCheckBox(bool bTicked, bool bFocusRing, bool bMouseOver, CColourFormatHelper* pcFormat);
	static void					SetColour(CImageDraw* pcDraw, ARGB32 sColour);
	static void					DrawLayer(CImageDraw* pcDraw, int iTile, int iInset, int iRadius, ARGB32 sColour);
	static void					DrawCorner(CImageDraw* pcDraw, int iTile, int iTileTop, bool bRight, bool bBottom, int iInset, int iRadius);
};


#endif // __JEWEL_BUTTON_IMAGES_H__

