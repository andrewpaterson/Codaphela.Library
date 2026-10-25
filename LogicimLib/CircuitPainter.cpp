/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/Numbers.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/UTF8.h"
#include "SupportLib/Glyph.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "CircuitPainter.h"


#define ARC_TOLERANCE_PIXELS	0.1
#define MAX_ARC_SEGMENTS		1024


typedef CArrayTemplate<SFloat64Vec2>	CArrayPainterPoint;


//////////////////////////////////////////////////////////////////////////
//
// The views are drawn as they are in the circuit simulation of the view
// path, with voltages coloured at the simulation's time.  Without a
// simulation (NULL) every trace and port is drawn as disconnected.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::Init(CCircuitViewport* pcViewport, CGeometryMap* pcMap, CSpriteMap* pcSprites, CCircuitColours* pcColours, CCircuitFonts* pcFonts, CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	mpcViewport = pcViewport;
	mpcMap = pcMap;
	mpcSprites = pcSprites;
	mpcColours = pcColours;
	mpcFonts = pcFonts;
	mpcViewPath = pcViewPath;
	mpcCircuitSimulation = pcCircuitSimulation;
	miTime = (pcCircuitSimulation != NULL) ? pcCircuitSimulation->GetTime() : -1;
	ClearTransform();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::Kill(void)
{
	mpcViewport = NULL;
	mpcMap = NULL;
	mpcSprites = NULL;
	mpcColours = NULL;
	mpcFonts = NULL;
	mpcViewPath = NULL;
	mpcCircuitSimulation = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::SetTransform(SInt32Vec2 sPosition, ERotation eRotation)
{
	msPosition = sPosition;
	meRotation = eRotation;
}


//////////////////////////////////////////////////////////////////////////
//
// Shapes are then in the circuit's grid space.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::ClearTransform(void)
{
	msPosition.Init(0, 0);
	meRotation = ROT_East;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::ToScreen(SFloat64Vec2* psScreen, SFloat64Vec2* psLocal)
{
	SFloat64Vec2	sGrid;

	sGrid = *psLocal;
	RotatePoint(meRotation, &sGrid);
	sGrid.x += msPosition.x;
	sGrid.y += msPosition.y;
	mpcViewport->GridToScreen(psScreen, &sGrid);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::ToScreen(SFloat64Vec2* psScreen, float64 x, float64 y)
{
	SFloat64Vec2	sLocal;

	sLocal.Init(x, y);
	ToScreen(psScreen, &sLocal);
}


//////////////////////////////////////////////////////////////////////////
//
// Square capped, as Java's default stroke.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::Line(float64 x0, float64 y0, float64 x1, float64 y1, float64 fWidth, ARGB32 sColour)
{
	SFloat64Vec2	sStart;
	SFloat64Vec2	sEnd;

	ToScreen(&sStart, x0, y0);
	ToScreen(&sEnd, x1, y1);
	mpcMap->AddLine(sStart.x, sStart.y, sEnd.x, sEnd.y, mpcViewport->GetLineWidth(fWidth), sColour, GLC_Square);
}


//////////////////////////////////////////////////////////////////////////
//
// The fill is drawn first and the border, centred on the edges, over it.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::Polygon(SFloat64Vec2* asPoints, size uiNumPoints, bool bFill, bool bBorder, float64 fWidth, ARGB32 sFill, ARGB32 sBorder)
{
	CArrayPainterPoint	asScreen;
	size				i;

	asScreen.Init();
	asScreen.SetUsedElements(uiNumPoints);
	for (i = 0; i < uiNumPoints; i++)
	{
		ToScreen(asScreen.Get(i), &asPoints[i]);
	}
	if (bFill)
	{
		mpcMap->AddPolygon(asScreen.GetData(), uiNumPoints, sFill);
	}
	if (bBorder)
	{
		mpcMap->AddPolyline(asScreen.GetData(), uiNumPoints, true, mpcViewport->GetLineWidth(fWidth), sBorder);
	}
	asScreen.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An arc of an ellipse with its centre and radii in grid units.  Angles are
// degrees counterclockwise on the screen from three o'clock, before the
// component is turned, as Java's drawArc.  The fill is a pie slice to the
// centre, as fillArc; the border is the open arc only.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::Arc(float64 fCentreX, float64 fCentreY, float64 fRadiusX, float64 fRadiusY, float64 fStartAngle, float64 fArcAngle, bool bFill, bool bBorder, float64 fWidth, ARGB32 sFill, ARGB32 sBorder)
{
	CArrayPainterPoint	asScreen;
	size				uiSegments;
	size				i;
	float64				fAngle;
	SFloat64Vec2		sLocal;

	uiSegments = NumArcSegments(fRadiusX, fRadiusY, fArcAngle);

	asScreen.Init();
	asScreen.SetUsedElements(uiSegments + 2);
	for (i = 0; i <= uiSegments; i++)
	{
		fAngle = (fStartAngle + fArcAngle * (float64)i / (float64)uiSegments) * N_PI_64 / 180.0;
		sLocal.Init(fCentreX + fRadiusX * cos(fAngle), fCentreY - fRadiusY * sin(fAngle));
		ToScreen(asScreen.Get(i + 1), &sLocal);
	}

	if (bFill)
	{
		ToScreen(asScreen.Get(0), fCentreX, fCentreY);
		mpcMap->AddPolygon(asScreen.GetData(), uiSegments + 2, sFill);
	}
	if (bBorder)
	{
		mpcMap->AddPolyline(asScreen.Get(1), uiSegments + 1, false, mpcViewport->GetLineWidth(fWidth), sBorder);
	}
	asScreen.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Enough straight segments that none is more than ARC_TOLERANCE_PIXELS
// from the curve on the screen.
//
//////////////////////////////////////////////////////////////////////////
size CCircuitPainter::NumArcSegments(float64 fRadiusX, float64 fRadiusY, float64 fArcAngle)
{
	float64		fRadius;
	float64		fStep;
	float64		fSegments;

	fRadius = ((fRadiusX > fRadiusY) ? fRadiusX : fRadiusY) * mpcViewport->GetPixelsPerGrid();
	if (fRadius <= ARC_TOLERANCE_PIXELS)
	{
		return 1;
	}
	fStep = 2 * acos(1 - ARC_TOLERANCE_PIXELS / fRadius) * 180.0 / N_PI_64;
	fSegments = ceil(fabs(fArcAngle) / fStep);
	if (fSegments < 1)
	{
		return 1;
	}
	if (fSegments > MAX_ARC_SEGMENTS)
	{
		return MAX_ARC_SEGMENTS;
	}
	return (size)fSegments;
}


//////////////////////////////////////////////////////////////////////////
//
// The radius is in grid units.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::Circle(float64 fCentreX, float64 fCentreY, float64 fRadius, bool bFill, bool bBorder, float64 fWidth, ARGB32 sFill, ARGB32 sBorder)
{
	SFloat64Vec2	sCentre;
	float64			fPixelRadius;

	ToScreen(&sCentre, fCentreX, fCentreY);
	fPixelRadius = fRadius * mpcViewport->GetPixelsPerGrid();
	if (bFill)
	{
		mpcMap->AddFilledCircle(sCentre.x, sCentre.y, fPixelRadius, sFill);
	}
	if (bBorder)
	{
		mpcMap->AddCircle(sCentre.x, sCentre.y, fPixelRadius, mpcViewport->GetLineWidth(fWidth), sBorder);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A filled circle whose radius is in pixels, for ports and junctions.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::Dot(float64 x, float64 y, float64 fPixelRadius, ARGB32 sColour)
{
	SFloat64Vec2	sCentre;

	ToScreen(&sCentre, x, y);
	mpcMap->AddFilledCircle(sCentre.x, sCentre.y, fPixelRadius, sColour);
}


//////////////////////////////////////////////////////////////////////////
//
// A line of text at x, y in the component's grid space, placed as
// Logicim's TextView.paint places it.  The text is measured at its font
// size (see SCircuitTextSize) and its alignment end is a tenth of a grid
// unit past x, y along the text.  Across the text the baseline is a 3.5th
// of the text's height past the position, which roughly centres capitals
// on it.  Text facing North or West ends at the position and text facing
// South or East starts there (for left aligned text), so port labels
// point into the component whichever way it's turned.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::Text(float64 x, float64 y, const char* szText, ECircuitFont eFont, float64 fFontSize, ETextAlignment eAlignment, int iRelativeRightRotations, ARGB32 sColour)
{
	SCircuitTextSize	sSize;
	float64				fFactor;
	float64				fWidth;
	float64				fHalfHeight;
	float64				fWidthAdjust;
	float32				fPixelSize;
	SFloat64Vec2		sAnchor;
	SFloat64Vec2		sOrigin;
	ERotation			eRotation;
	EFontTurn			eTurn;
	Ptr<CFontColours>	pColours;
	Ptr<CFont>			pFont;
	Ptr<CGlyph>			pGlyph;
	CUTF8				cUTF8;
	int32				iPen;
	int32				iBaseline;
	size				uiUTFLength;

	if ((mpcFonts == NULL) || (mpcSprites == NULL) || (szText == NULL) || (szText[0] == '\0'))
	{
		return;
	}

	sSize = mpcFonts->Measure(eFont, fFontSize, szText);
	fFactor = mpcViewport->GetPixelsPerGrid();
	fWidth = sSize.fWidth * fFactor;
	fHalfHeight = sSize.fHeight * fFactor / 3.5;
	fWidthAdjust = GetTextWidthAdjust(0.1, sSize.fWidth, eAlignment) * fFactor;

	eRotation = RotateRight(meRotation, iRelativeRightRotations);
	ToScreen(&sAnchor, x, y);
	switch (eRotation)
	{
	case ROT_South:
		sOrigin.Init(sAnchor.x - fHalfHeight, sAnchor.y + fWidthAdjust);
		eTurn = FT_Clockwise;
		break;
	case ROT_East:
		sOrigin.Init(sAnchor.x + fWidthAdjust, sAnchor.y + fHalfHeight);
		eTurn = FT_Upright;
		break;
	case ROT_West:
		sOrigin.Init(sAnchor.x - fWidth - fWidthAdjust, sAnchor.y + fHalfHeight);
		eTurn = FT_Upright;
		break;
	default:
		sOrigin.Init(sAnchor.x - fHalfHeight, sAnchor.y - fWidth - fWidthAdjust);
		eTurn = FT_Clockwise;
		break;
	}

	fPixelSize = (float32)floor(fFontSize * mpcViewport->GetZoom() + 0.5);
	if (fPixelSize < 1.0f)
	{
		return;
	}
	pColours = mpcFonts->GetColours(eFont, fPixelSize, eTurn);
	if (pColours.IsNull())
	{
		return;
	}
	pFont = pColours->GetFont(sColour);

	//Sprites are placed by their full top left: the top of the line for upright glyphs, left of the descent for turned.
	if (eTurn == FT_Clockwise)
	{
		iBaseline = (int32)floor(sOrigin.x + 0.5);
		iPen = (int32)floor(sOrigin.y + 0.5);
	}
	else
	{
		iBaseline = (int32)floor(sOrigin.y + 0.5);
		iPen = (int32)floor(sOrigin.x + 0.5);
	}
	cUTF8.Init(szText);
	for (;;)
	{
		uiUTFLength = cUTF8.PeekUTFBytes();
		if ((uiUTFLength == 0) || (uiUTFLength == cUTF8.GetError()))
		{
			break;
		}
		pGlyph = pFont->GetGlyph(&cUTF8);
		if (pGlyph.IsNull())
		{
			continue;
		}
		if (eTurn == FT_Clockwise)
		{
			mpcSprites->AddSprite(pGlyph->GetCel(), iBaseline - pFont->GetDescent(), iPen);
		}
		else
		{
			mpcSprites->AddSprite(pGlyph->GetCel(), iPen, iBaseline - pFont->GetAscent());
		}
		iPen += pGlyph->GetStep();
	}
	cUTF8.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A line of text centred on x, y and always upright, as Logicim's
// PositivePowerView draws its voltage: measured at its drawn size and its
// line's height centred.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitPainter::CentredText(float64 x, float64 y, const char* szText, ECircuitFont eFont, float64 fFontSize, ARGB32 sColour)
{
	float32				fPixelSize;
	Ptr<CFontColours>	pColours;
	Ptr<CFont>			pFont;
	Ptr<CGlyph>			pGlyph;
	SFloat64Vec2		sCentre;
	CUTF8				cUTF8;
	int32				iPen;
	int32				iTop;
	size				uiUTFLength;

	if ((mpcFonts == NULL) || (mpcSprites == NULL) || (szText == NULL) || (szText[0] == '\0'))
	{
		return;
	}
	fPixelSize = (float32)floor(fFontSize * mpcViewport->GetZoom() + 0.5);
	if (fPixelSize < 1.0f)
	{
		return;
	}
	pColours = mpcFonts->GetColours(eFont, fPixelSize, FT_Upright);
	if (pColours.IsNull())
	{
		return;
	}
	pFont = pColours->GetFont(sColour);

	ToScreen(&sCentre, x, y);
	iPen = (int32)floor(sCentre.x - pFont->CalculateWidth((char*)szText) / 2.0 + 0.5);
	iTop = (int32)floor(sCentre.y - pFont->GetLineHeight() / 2.0 + 0.5);
	cUTF8.Init(szText);
	for (;;)
	{
		uiUTFLength = cUTF8.PeekUTFBytes();
		if ((uiUTFLength == 0) || (uiUTFLength == cUTF8.GetError()))
		{
			break;
		}
		pGlyph = pFont->GetGlyph(&cUTF8);
		if (pGlyph.IsNull())
		{
			continue;
		}
		mpcSprites->AddSprite(pGlyph->GetCel(), iPen, iTop);
		iPen += pGlyph->GetStep();
	}
	cUTF8.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's TextView.getWidthAdjust, in grid units.
//
//////////////////////////////////////////////////////////////////////////
float64 CCircuitPainter::GetTextWidthAdjust(float64 fOffset, float64 fWidth, ETextAlignment eAlignment)
{
	if (eAlignment == TA_Centre)
	{
		return fOffset - fWidth / 2;
	}
	else if (eAlignment == TA_Right)
	{
		return fOffset - fWidth;
	}
	return fOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuitViewport* CCircuitPainter::GetViewport(void) { return mpcViewport; }
CCircuitColours* CCircuitPainter::GetColours(void) { return mpcColours; }
CCircuitFonts* CCircuitPainter::GetFonts(void) { return mpcFonts; }
CGeometryMap* CCircuitPainter::GetMap(void) { return mpcMap; }
int64 CCircuitPainter::GetTime(void) { return miTime; }
CViewPath* CCircuitPainter::GetViewPath(void) { return mpcViewPath; }
CCircuitSimulation* CCircuitPainter::GetCircuitSimulation(void) { return mpcCircuitSimulation; }

