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
#include "StandardLib/Objects.h"
#include "SupportLib/ImageDraw.h"
#include "SupportLib/ImageColourConverter.h"
#include "JewelButtonImages.h"


//////////////////////////////////////////////////////////////////////////
//
// Draws a corner tile's part of one layer of a Jewel button's border.  The
// layer is a rounded rectangle iInset in from the tile's outside sides
// with corners iRadius round; it runs on to the tile's inside sides.  The
// tiles are iTile square.
//
//////////////////////////////////////////////////////////////////////////
void CJewelButtonImages::DrawCorner(CImageDraw* pcDraw, int iTile, int iTileTop, bool bRight, bool bBottom, int iInset, int iRadius)
{
	int		iCircle;
	int		iStartAngle;
	int		iLeft;
	int		iRight;
	int		iBeyondLeft;
	int		iBeyondRight;
	int		iBeyondTop;
	int		iBeyondBottom;
	int		iCurveTop;
	int		iCurveBottom;

	//The quarter circle's square is iInset in from the outside sides.
	iCircle = iTile - iInset - iRadius * 2;
	if (bRight)
	{
		iStartAngle = bBottom ? 270 : 0;
	}
	else
	{
		iStartAngle = bBottom ? 180 : 90;
	}
	pcDraw->DrawSemicircle(bRight ? iCircle : iInset, iTileTop + (bBottom ? iCircle : iInset), iRadius * 2, iStartAngle, iStartAngle + 90, true);

	//All of the layer across the tile, and the part beyond the quarter circle, for each axis.
	iLeft = bRight ? 0 : iInset;
	iRight = bRight ? iTile - 1 - iInset : iTile - 1;
	iBeyondLeft = bRight ? 0 : iInset + iRadius;
	iBeyondRight = bRight ? iTile - 1 - iInset - iRadius : iTile - 1;
	iBeyondTop = iTileTop + (bBottom ? 0 : iInset + iRadius);
	iBeyondBottom = iTileTop + (bBottom ? iTile - 1 - iInset - iRadius : iTile - 1);
	iCurveTop = iTileTop + (bBottom ? iTile - iInset - iRadius : iInset);
	iCurveBottom = iTileTop + (bBottom ? iTile - 1 - iInset : iInset + iRadius - 1);

	pcDraw->DrawBox(iLeft, iBeyondTop, iRight, iBeyondBottom, true);
	pcDraw->DrawBox(iBeyondLeft, iCurveTop, iBeyondRight, iCurveBottom, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Draws one layer of a Jewel button's border on every tile of the strip.
//
//////////////////////////////////////////////////////////////////////////
void CJewelButtonImages::DrawLayer(CImageDraw* pcDraw, int iTile, int iInset, int iRadius, ARGB32 sColour)
{
	CImageColourARGB	cColour;
	int					i;

	cColour.Init(Get8BitAlphaColour(sColour) / 255.0f, Get8BitRedColour(sColour) / 255.0f, Get8BitGreenColour(sColour) / 255.0f, Get8BitBlueColour(sColour) / 255.0f);
	pcDraw->SetColour(&cColour);
	cColour.Kill();

	//The edges all have their outside along their top row.
	for (i = 0; i < 4; i++)
	{
		pcDraw->DrawBox(0, i * iTile + iInset, iTile - 1, i * iTile + iTile - 1, true);
	}

	DrawCorner(pcDraw, iTile, iTile * 4, false, false, iInset, iRadius);
	DrawCorner(pcDraw, iTile, iTile * 5, true, false, iInset, iRadius);
	DrawCorner(pcDraw, iTile, iTile * 6, true, true, iInset, iRadius);
	DrawCorner(pcDraw, iTile, iTile * 7, false, true, iInset, iRadius);
}


//////////////////////////////////////////////////////////////////////////
//
// A strip of 8 x 8 tiles for a button from JetBrains' Jewel dark theme.
// From the outside in: a two pixel focus ring with corners six round,
// drawn only if bFocusRing, then a one pixel border with corners four
// round, then the face with corners three round.  The border is drawn
// filled and the face over it.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CJewelButtonImages::DrawStrip(bool bFocusRing, ARGB32 sFocusRing, ARGB32 sBorder, ARGB32 sFace, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;

	pImage = OMalloc<CImage>(8, 64, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();

	cDraw.Init(&pImage);
	if (bFocusRing)
	{
		DrawLayer(&cDraw, 8, 0, 6, sFocusRing);
	}
	DrawLayer(&cDraw, 8, 2, 4, sBorder);
	DrawLayer(&cDraw, 8, 3, 3, sFace);
	cDraw.Kill();

	return CImageColourConverter::ConvertColour(pImage, pcFormat->meOrder);
}


//////////////////////////////////////////////////////////////////////////
//
// The outlined button from JetBrains' Jewel dark theme.  The outside two
// pixels of every tile are its focus ring, empty except when selected.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> CJewelButtonImages::CreateButtonImages(CColourFormatHelper* pcFormat)
{
	ARGB32			sFace;
	ARGB32			sBlue;
	Ptr<CImage>		pNormal;
	Ptr<CImage>		pMouseOver;
	Ptr<CImage>		pSelected;
	Ptr<CImage>		pPushed;

	sFace = Set32BitColour((uint8)0x2B, 0x2D, 0x30);
	sBlue = Set32BitColour((uint8)0x35, 0x74, 0xF0);
	pNormal = DrawStrip(false, 0, Set32BitColour((uint8)0x4E, 0x51, 0x57), sFace, pcFormat);
	pMouseOver = DrawStrip(false, 0, Set32BitColour((uint8)0x6F, 0x73, 0x7A), sFace, pcFormat);
	pSelected = DrawStrip(true, sBlue, sBlue, sFace, pcFormat);
	pPushed = DrawStrip(false, 0, Set32BitColour((uint8)0x86, 0x8A, 0x91), sFace, pcFormat);
	return OMalloc<CButtonImages>(pNormal, pMouseOver, pSelected, pPushed, sFace);
}


//////////////////////////////////////////////////////////////////////////
//
// The default button from JetBrains' Jewel dark theme.  It is filled blue,
// lighter with the pointer over it and darker when pushed.  Selected it has
// a dark line between its face and its focus ring.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> CJewelButtonImages::CreateDefaultButtonImages(CColourFormatHelper* pcFormat)
{
	ARGB32			sDark;
	ARGB32			sBlue;
	ARGB32			sLightBlue;
	ARGB32			sDarkBlue;
	Ptr<CImage>		pNormal;
	Ptr<CImage>		pMouseOver;
	Ptr<CImage>		pSelected;
	Ptr<CImage>		pPushed;

	sDark = Set32BitColour((uint8)0x2B, 0x2D, 0x30);
	sBlue = Set32BitColour((uint8)0x35, 0x74, 0xF0);
	sLightBlue = Set32BitColour((uint8)0x46, 0x82, 0xFA);
	sDarkBlue = Set32BitColour((uint8)0x36, 0x6A, 0xCE);
	pNormal = DrawStrip(false, 0, sBlue, sBlue, pcFormat);
	pMouseOver = DrawStrip(false, 0, sLightBlue, sLightBlue, pcFormat);
	pSelected = DrawStrip(true, sBlue, sDark, sBlue, pcFormat);
	pPushed = DrawStrip(false, 0, sDarkBlue, sDarkBlue, pcFormat);
	return OMalloc<CButtonImages>(pNormal, pMouseOver, pSelected, pPushed, sBlue, sLightBlue, sBlue, sDarkBlue);
}


//////////////////////////////////////////////////////////////////////////
//
// A strip of 7 x 7 tiles for a flat button, like the main menu and toolbar
// buttons of IntelliJ IDEA's new UI.  Filled, the tiles are a rectangle
// with corners four round, two pixels in from the outside so neighbouring
// buttons' blocks don't touch; otherwise they are empty.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CJewelButtonImages::DrawFlatStrip(bool bFilled, ARGB32 sFill, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;

	pImage = OMalloc<CImage>(7, 56, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();

	if (bFilled)
	{
		cDraw.Init(&pImage);
		DrawLayer(&cDraw, 7, 2, 4, sFill);
		cDraw.Kill();
	}

	return CImageColourConverter::ConvertColour(pImage, pcFormat->meOrder);
}


//////////////////////////////////////////////////////////////////////////
//
// A small flat button: no border and no focus ring, only its face, which
// is the colour of what it sits on.  Under the pointer, and pushed, it is
// a block with rounded corners.  It looks the same with focus as without.
// The border is only seven pixels, two of them empty, so with a little
// padding it is not much bigger than its caption.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> CJewelButtonImages::CreateFlatButtonImages(ARGB32 sBackground, ARGB32 sHover, ARGB32 sPushed, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>		pNormal;
	Ptr<CImage>		pHover;
	Ptr<CImage>		pPushed;

	pNormal = DrawFlatStrip(false, 0, pcFormat);
	pHover = DrawFlatStrip(true, sHover, pcFormat);
	pPushed = DrawFlatStrip(true, sPushed, pcFormat);
	return OMalloc<CButtonImages>(pNormal, pHover, pNormal, pPushed, sBackground, sHover, sBackground, sPushed);
}


//////////////////////////////////////////////////////////////////////////
//
// The chevron on a menu item that opens another menu.  Two one pixel
// lines meet at a point on the right, with nothing around them.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelButtonImages::CreateChevron(ARGB32 sColour, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;
	CImageColourARGB	cColour;

	pImage = OMalloc<CImage>(8, 9, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();

	cDraw.Init(&pImage);
	cColour.Init(Get8BitAlphaColour(sColour) / 255.0f, Get8BitRedColour(sColour) / 255.0f, Get8BitGreenColour(sColour) / 255.0f, Get8BitBlueColour(sColour) / 255.0f);
	cDraw.SetColour(&cColour);
	cColour.Kill();
	cDraw.DrawLine(2, 1, 5, 4);
	cDraw.DrawLine(5, 4, 2, 7);
	cDraw.Kill();

	pImage = CImageColourConverter::ConvertColour(pImage, pcFormat->meOrder);
	return OMalloc<CImageCel>(pImage);
}

