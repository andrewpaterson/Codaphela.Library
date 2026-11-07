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
#include "SupportLib/VectorShapes.h"
#include "SupportLib/ImageColourConverter.h"
#include "JewelColours.h"
#include "JewelImageGenerator.h"


//The push pin's strokes, upright (x0, y0, x1, y1) on 16x16.
#define NUM_PIN_STROKES	5
static float64 gaafPinStrokes[NUM_PIN_STROKES][4] =
{
	{ 5.0, 2.5, 11.0, 2.5 },	//The head.
	{ 6.5, 3.0, 6.5, 9.0 },		//The body's sides.
	{ 9.5, 3.0, 9.5, 9.0 },
	{ 4.0, 9.5, 12.0, 9.5 },	//The base.
	{ 8.0, 10.0, 8.0, 14.5 },	//The needle.
};


//////////////////////////////////////////////////////////////////////////
//
// An image to draw an icon or a strip on: red, green, blue and opacity,
// every pixel clear.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CJewelImageGenerator::CreateClearImage(int32 iWidth, int32 iHeight)
{
	Ptr<CImage>	pImage;

	pImage = OMalloc<CImage>(iWidth, iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// The icon drawn on pImage, in the window's colour order, as a cel.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelImageGenerator::FinishCel(Ptr<CImage> pImage, CColourFormatHelper* pcFormat)
{
	return OMalloc<CImageCel>(CImageColourConverter::ConvertColour(pImage, pcFormat->meOrder));
}


//////////////////////////////////////////////////////////////////////////
//
// Draws a corner tile's part of one layer of a Jewel button's border.  The
// layer is a rounded rectangle iInset in from the tile's outside sides
// with corners iRadius round; it runs on to the tile's inside sides.  The
// tiles are iTile square.
//
//////////////////////////////////////////////////////////////////////////
void CJewelImageGenerator::DrawCorner(CImageDraw* pcDraw, int iTile, int iTileTop, bool bRight, bool bBottom, int iInset, int iRadius)
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
void CJewelImageGenerator::DrawLayer(CImageDraw* pcDraw, int iTile, int iInset, int iRadius, ARGB32 sColour)
{
	uint	i;

	pcDraw->SetColour(sColour);

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
Ptr<CImage> CJewelImageGenerator::DrawStrip(bool bFocusRing, ARGB32 sFocusRing, ARGB32 sBorder, ARGB32 sFace, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;

	pImage = CreateClearImage(8, 64);

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
Ptr<CButtonImages> CJewelImageGenerator::CreateButtonImages(CColourFormatHelper* pcFormat)
{
	ARGB32			sFace;
	ARGB32			sBlue;
	Ptr<CImage>		pNormal;
	Ptr<CImage>		pMouseOver;
	Ptr<CImage>		pSelected;
	Ptr<CImage>		pPushed;

	sFace = JEWEL_PANEL;
	sBlue = JEWEL_BLUE;
	pNormal = DrawStrip(false, 0, JEWEL_BORDER, sFace, pcFormat);
	pMouseOver = DrawStrip(false, 0, JEWEL_BORDER_HOVER, sFace, pcFormat);
	pSelected = DrawStrip(true, sBlue, sBlue, sFace, pcFormat);
	pPushed = DrawStrip(false, 0, JEWEL_BORDER_PUSHED, sFace, pcFormat);
	return OMalloc<CButtonImages>(pNormal, pMouseOver, pSelected, pPushed, sFace);
}


//////////////////////////////////////////////////////////////////////////
//
// The default button from JetBrains' Jewel dark theme.  It is filled blue,
// lighter with the pointer over it and darker when pushed.  Selected it has
// a dark line between its face and its focus ring.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> CJewelImageGenerator::CreateDefaultButtonImages(CColourFormatHelper* pcFormat)
{
	ARGB32			sDark;
	ARGB32			sBlue;
	ARGB32			sLightBlue;
	ARGB32			sDarkBlue;
	Ptr<CImage>		pNormal;
	Ptr<CImage>		pMouseOver;
	Ptr<CImage>		pSelected;
	Ptr<CImage>		pPushed;

	sDark = JEWEL_PANEL;
	sBlue = JEWEL_BLUE;
	sLightBlue = JEWEL_BLUE_HOVER;
	sDarkBlue = JEWEL_BLUE_PUSHED;
	pNormal = DrawStrip(false, 0, sBlue, sBlue, pcFormat);
	pMouseOver = DrawStrip(false, 0, sLightBlue, sLightBlue, pcFormat);
	pSelected = DrawStrip(true, sBlue, sDark, sBlue, pcFormat);
	pPushed = DrawStrip(false, 0, sDarkBlue, sDarkBlue, pcFormat);
	return OMalloc<CButtonImages>(pNormal, pMouseOver, pSelected, pPushed, sBlue, sLightBlue, sBlue, sDarkBlue);
}


//////////////////////////////////////////////////////////////////////////
//
// The outlined button made small, for buttons crowded in with text.  Its
// border is at the tiles' edges, not in by the focus ring's two pixels, so
// selected it has a blue border instead.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> CJewelImageGenerator::CreateSmallButtonImages(CColourFormatHelper* pcFormat)
{
	ARGB32			sFace;
	Ptr<CImage>		pNormal;
	Ptr<CImage>		pMouseOver;
	Ptr<CImage>		pSelected;
	Ptr<CImage>		pPushed;

	sFace = JEWEL_PANEL;
	pNormal = DrawSmallStrip(JEWEL_BORDER, sFace, pcFormat);
	pMouseOver = DrawSmallStrip(JEWEL_BORDER_HOVER, sFace, pcFormat);
	pSelected = DrawSmallStrip(JEWEL_BLUE, sFace, pcFormat);
	pPushed = DrawSmallStrip(JEWEL_BORDER_PUSHED, sFace, pcFormat);
	return OMalloc<CButtonImages>(pNormal, pMouseOver, pSelected, pPushed, sFace);
}


//////////////////////////////////////////////////////////////////////////
//
// A strip of 4 x 4 tiles for a small button: a one pixel border with
// corners four round, as the outlined button's, around the face with
// corners three round.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CJewelImageGenerator::DrawSmallStrip(ARGB32 sBorder, ARGB32 sFace, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;

	pImage = CreateClearImage(4, 32);

	cDraw.Init(&pImage);
	DrawLayer(&cDraw, 4, 0, 4, sBorder);
	DrawLayer(&cDraw, 4, 1, 3, sFace);
	cDraw.Kill();

	return CImageColourConverter::ConvertColour(pImage, pcFormat->meOrder);
}


//////////////////////////////////////////////////////////////////////////
//
// A strip of 7 x 7 tiles for a flat button, like the main menu and toolbar
// buttons of IntelliJ IDEA's new UI.  Filled, the tiles are a rectangle
// with corners four round, two pixels in from the outside so neighbouring
// buttons' blocks don't touch; otherwise they are empty.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CJewelImageGenerator::DrawFlatStrip(bool bFilled, ARGB32 sFill, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;

	pImage = CreateClearImage(7, 56);

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
Ptr<CButtonImages> CJewelImageGenerator::CreateFlatButtonImages(ARGB32 sBackground, ARGB32 sHover, ARGB32 sPushed, CColourFormatHelper* pcFormat)
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
Ptr<CImageCel> CJewelImageGenerator::CreateChevron(ARGB32 sColour, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;

	pImage = CreateClearImage(8, 9);

	cDraw.Init(&pImage);
	cDraw.SetColour(sColour);
	cDraw.DrawLine(2, 1, 5, 4);
	cDraw.DrawLine(5, 4, 2, 7);
	cDraw.Kill();

	return FinishCel(pImage, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// The chevron of a combo box, 9 x 8.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelImageGenerator::CreateDownChevron(ARGB32 sColour, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;

	pImage = CreateClearImage(9, 8);

	cDraw.Init(&pImage);
	cDraw.SetColour(sColour);
	cDraw.DrawLine(1, 2, 4, 5);
	cDraw.DrawLine(4, 5, 7, 2);
	cDraw.Kill();

	return FinishCel(pImage, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// IntelliJ IDEA's previous and next difference arrows: a stem down the
// middle of a 16 x 16 cel with a head at the top (bUp) or the bottom,
// anti-aliased strokes.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelImageGenerator::CreateArrow(ARGB32 sColour, bool bUp, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;
	float64				fTip;
	float64				fBase;
	float64				fHead;

	pImage = CreateClearImage(16, 16);

	fTip = bUp ? 3.0 : 13.0;
	fBase = bUp ? 13.0 : 3.0;
	fHead = bUp ? 7.5 : 8.5;
	cDraw.Init(&pImage);
	cDraw.SetColour(sColour);
	cDraw.DrawStroke(8, fBase, 8, fTip, 1.25);
	cDraw.DrawStroke(8, fTip, 3.5, fHead, 1.25);
	cDraw.DrawStroke(8, fTip, 12.5, fHead, 1.25);
	cDraw.Kill();

	return FinishCel(pImage, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// A tab's close cross, two anti-aliased strokes 8 pixels across in the
// middle of a 16 x 16 cel.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelImageGenerator::CreateCross(ARGB32 sColour, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;

	pImage = CreateClearImage(16, 16);

	cDraw.Init(&pImage);
	cDraw.SetColour(sColour);
	cDraw.DrawStroke(4.5, 4.5, 11.5, 11.5, 1.25);
	cDraw.DrawStroke(11.5, 4.5, 4.5, 11.5, 1.25);
	cDraw.Kill();

	return FinishCel(pImage, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// An input icon in the style of the New UI's outlined icons: a keyboard
// with a one pixel border, corners two round, around two rows of five
// keys and a space bar, in the middle of a 16 x 16 cel.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelImageGenerator::CreateKeyboard(ARGB32 sColour, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;
	CVectorPath			cPath;
	SFloat64Vec2		sMinimum;
	SFloat64Vec2		sMaximum;
	uint				x;

	pImage = CreateClearImage(16, 16);

	cPath.Init();
	cPath.BeginPart(true);
	sMinimum.Init(1, 3);
	sMaximum.Init(16, 14);
	AddVectorRoundedRectangle(&cPath, &sMinimum, &sMaximum, 2);

	cPath.BeginPart(false);
	sMinimum.Init(2, 4);
	sMaximum.Init(15, 13);
	AddVectorRoundedRectangle(&cPath, &sMinimum, &sMaximum, 1);

	cPath.BeginPart(true);
	for (x = 4; x <= 12; x += 2)
	{
		sMinimum.Init(x, 6);
		sMaximum.Init(x + 1, 7);
		AddVectorRectangle(&cPath, &sMinimum, &sMaximum);
		sMinimum.Init(x, 8);
		sMaximum.Init(x + 1, 9);
		AddVectorRectangle(&cPath, &sMinimum, &sMaximum);
	}
	sMinimum.Init(6, 10);
	sMaximum.Init(11, 11);
	AddVectorRectangle(&cPath, &sMinimum, &sMaximum);

	cDraw.Init(&pImage);
	cDraw.SetColour(sColour);
	cDraw.DrawPath(&cPath);
	cDraw.Kill();
	cPath.Kill();

	return FinishCel(pImage, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// A folder in the style of the New UI's outlined icons: a tab on the left
// of its top edge, in a 16 x 16 cel.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelImageGenerator::CreateFolder(ARGB32 sColour, CColourFormatHelper* pcFormat)
{
	SFloat64Vec2	asPoints[6];

	asPoints[0].Init(1.5, 3.5);
	asPoints[1].Init(6.5, 3.5);
	asPoints[2].Init(8.5, 5.5);
	asPoints[3].Init(14.5, 5.5);
	asPoints[4].Init(14.5, 12.5);
	asPoints[5].Init(1.5, 12.5);
	return DrawOutline(asPoints, 6, NULL, 0, sColour, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// A page in the style of the New UI's outlined icons, its top right
// corner folded over, in a 16 x 16 cel.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelImageGenerator::CreateFile(ARGB32 sColour, CColourFormatHelper* pcFormat)
{
	SFloat64Vec2	asPoints[5];
	SFloat64Vec2	asFold[3];

	asPoints[0].Init(3.5, 1.5);
	asPoints[1].Init(9.5, 1.5);
	asPoints[2].Init(12.5, 4.5);
	asPoints[3].Init(12.5, 14.5);
	asPoints[4].Init(3.5, 14.5);
	asFold[0].Init(9.5, 1.5);
	asFold[1].Init(9.5, 4.5);
	asFold[2].Init(12.5, 4.5);
	return DrawOutline(asPoints, 5, asFold, 3, sColour, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// A 16 x 16 cel of one pixel wide strokes around the closed shape
// asPoints, and along the open line asExtra (which can be NULL).  Points
// on pixel centres keep straight edges sharp.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelImageGenerator::DrawOutline(SFloat64Vec2* asPoints, size uiNumPoints, SFloat64Vec2* asExtra, size uiNumExtra, ARGB32 sColour, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;
	CVectorPath			cPath;
	SFloat64Vec2*		psStart;
	SFloat64Vec2*		psEnd;
	size				i;

	pImage = CreateClearImage(16, 16);

	cPath.Init();
	cPath.BeginPart(true);
	for (i = 0; i < uiNumPoints; i++)
	{
		psStart = &asPoints[i];
		psEnd = &asPoints[(i + 1) % uiNumPoints];
		AddVectorStroke(&cPath, psStart, psEnd, 0.5);
	}
	for (i = 1; i < uiNumExtra; i++)
	{
		AddVectorStroke(&cPath, &asExtra[i - 1], &asExtra[i], 0.5);
	}

	cDraw.Init(&pImage);
	cDraw.SetColour(sColour);
	cDraw.DrawPath(&cPath);
	cDraw.Kill();
	cPath.Kill();

	return FinishCel(pImage, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// A 20 x 20 check box from JetBrains' Jewel dark theme: a focus ring,
// drawn only if bFocusRing, and a dark line inside it around a 16 x 16 box
// with corners three round.  An empty box is a grey border, the outlined
// button's, around the dark face; a ticked box is blue with a white tick.
// Both are lighter with the pointer over them.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelImageGenerator::DrawCheckBox(bool bTicked, bool bFocusRing, bool bMouseOver, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;

	pImage = CreateClearImage(20, 20);

	cDraw.Init(&pImage);
	if (bFocusRing)
	{
		cDraw.SetColour(JEWEL_BLUE);
		cDraw.DrawRoundedRectangle(0, 0, 20, 20, 5);

		//A dark line keeps the ring apart from a ticked box.
		cDraw.SetColour(JEWEL_PANEL);
		cDraw.DrawRoundedRectangle(1.5, 1.5, 18.5, 18.5, 3.5);
	}

	if (bTicked)
	{
		cDraw.SetColour(bMouseOver ? JEWEL_BLUE_HOVER : JEWEL_BLUE);
		cDraw.DrawRoundedRectangle(2, 2, 18, 18, 3);
		cDraw.SetColour(JEWEL_WHITE);
		cDraw.DrawStroke(6.0, 10.0, 8.5, 12.5, 1.75);
		cDraw.DrawStroke(8.5, 12.5, 14.0, 7.0, 1.75);
	}
	else
	{
		cDraw.SetColour(bMouseOver ? JEWEL_BORDER_HOVER : JEWEL_BORDER);
		cDraw.DrawRoundedRectangle(2, 2, 18, 18, 3);
		cDraw.SetColour(JEWEL_PANEL);
		cDraw.DrawRoundedRectangle(3, 3, 17, 17, 2);
	}
	cDraw.Kill();

	return FinishCel(pImage, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// A box for each ECheckBoxState, ticked and not.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCheckBoxImages> CJewelImageGenerator::CreateCheckBoxImages(CColourFormatHelper* pcFormat)
{
	Ptr<CCheckBoxImages>	pImages;
	uint					i;
	bool					bTicked;

	pImages = OMalloc<CCheckBoxImages>();
	for (i = 0; i < 2; i++)
	{
		bTicked = i == 1;
		pImages->SetCel(bTicked, CBS_Normal, DrawCheckBox(bTicked, false, false, pcFormat));
		pImages->SetCel(bTicked, CBS_MouseOver, DrawCheckBox(bTicked, false, true, pcFormat));
		pImages->SetCel(bTicked, CBS_Focussed, DrawCheckBox(bTicked, true, false, pcFormat));
		pImages->SetCel(bTicked, CBS_FocussedMouseOver, DrawCheckBox(bTicked, true, true, pcFormat));
	}
	return pImages;
}


//////////////////////////////////////////////////////////////////////////
//
// Visual Studio's push pin on a tool window's title bar: upright, its
// needle down, while the window is docked, and on its side, the needle to
// the left, while it auto hides.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CJewelImageGenerator::CreatePin(ARGB32 sColour, bool bPinned, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;
	size			i;
	float64			x0;
	float64			y0;
	float64			x1;
	float64			y1;

	pImage = CreateClearImage(16, 16);

	cDraw.Init(&pImage);
	cDraw.SetColour(sColour);
	for (i = 0; i < NUM_PIN_STROKES; i++)
	{
		x0 = gaafPinStrokes[i][0];
		y0 = gaafPinStrokes[i][1];
		x1 = gaafPinStrokes[i][2];
		y1 = gaafPinStrokes[i][3];
		if (!bPinned)
		{
			//Turned a quarter clockwise about the middle: down becomes left.
			cDraw.DrawStroke(16.0 - y0, x0, 16.0 - y1, x1, 1.0);
		}
		else
		{
			cDraw.DrawStroke(x0, y0, x1, y1, 1.0);
		}
	}
	cDraw.Kill();

	return FinishCel(pImage, pcFormat);
}

