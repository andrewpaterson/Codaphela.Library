/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "BaseLib/UTF8.h"
#include "BaseLib/ArraySize.h"
#include "BaseLib/FixedPoint.h"
#include "BaseLib/IntegerHelper.h"
#include "ImageAccessorCreator.h"
#include "TrueTypeRasteriser.h"


//A pixel completely covered accumulates cover * 2 * TRUE_TYPE_SUBPIXELS - area of this.
#define TRUE_TYPE_FULL_COVERAGE			(2 * TRUE_TYPE_SUBPIXELS * TRUE_TYPE_SUBPIXELS)

//Curves whose control point is less than this far (squared, in subpixels) from straight are drawn as one line;
//one third of a pixel squared.
#define TRUE_TYPE_FLAT_DEVIATION		(TRUE_TYPE_SUBPIXELS * TRUE_TYPE_SUBPIXELS / 3)


//////////////////////////////////////////////////////////////////////////
//
// fPixelSize is the height of one em in pixels.  It is rounded to 1/256
// of a pixel.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::Init(CTrueTypeFont* pcFont, float32 fPixelSize)
{
	mpcFont = pcFont;
	miPixelSize = FixedFromFloat(fPixelSize, TRUE_TYPE_SUBPIXEL_BITS);
	miUnitsPerEm = pcFont->GetUnitsPerEm();
	mcOutline.Init();
	mpiCover = NULL;
	mpiArea = NULL;
	mpuiCoverage = NULL;
	miWidth = 0;
	miHeight = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::Kill(void)
{
	EndCoverage();
	mcOutline.Kill();
	mpcFont = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels per font unit.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetScale(void)
{
	return FixedToFloat(miPixelSize, TRUE_TYPE_SUBPIXEL_BITS) / (float32)miUnitsPerEm;
}


//////////////////////////////////////////////////////////////////////////
//
// Font units to subpixels, rounded.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeRasteriser::Scale(int32 iFontUnits)
{
	return (int32)DivideRound((int64)iFontUnits * miPixelSize, miUnitsPerEm);
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels above the baseline.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetAscent(void)
{
	return FixedToFloat(GetAscentSubpixels(), TRUE_TYPE_SUBPIXEL_BITS);
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels below the baseline, positive.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetDescent(void)
{
	return FixedToFloat(GetDescentSubpixels(), TRUE_TYPE_SUBPIXEL_BITS);
}


//////////////////////////////////////////////////////////////////////////
//
// The distance between baselines.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetLineHeight(void)
{
	return FixedToFloat(Scale(mpcFont->GetAscender() - mpcFont->GetDescender() + mpcFont->GetLineGap()), TRUE_TYPE_SUBPIXEL_BITS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetAdvance(int iGlyph)
{
	return FixedToFloat(GetAdvanceSubpixels(iGlyph), TRUE_TYPE_SUBPIXEL_BITS);
}


//////////////////////////////////////////////////////////////////////////
//
// The sum of the advances of the characters.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetTextWidth(char* szUTF8)
{
	return FixedToFloat(GetTextWidthSubpixels(szUTF8), TRUE_TYPE_SUBPIXEL_BITS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeRasteriser::GetAscentSubpixels(void)
{
	return Scale(mpcFont->GetAscender());
}


//////////////////////////////////////////////////////////////////////////
//
// Positive.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeRasteriser::GetDescentSubpixels(void)
{
	return Scale(-mpcFont->GetDescender());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeRasteriser::GetAdvanceSubpixels(int iGlyph)
{
	STrueTypeGlyphMetrics	sMetrics;

	if (!mpcFont->GetGlyphMetrics(iGlyph, &sMetrics))
	{
		return 0;
	}
	return Scale(sMetrics.iAdvanceWidth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeRasteriser::GetTextWidthSubpixels(char* szUTF8)
{
	CUTF8	cUTF8;
	uint32	uiCodePoint;
	int32	iWidth;

	iWidth = 0;
	cUTF8.Init(szUTF8);
	for (;;)
	{
		uiCodePoint = cUTF8.GetCodePointUint32();
		if (uiCodePoint == 0)
		{
			break;
		}
		iWidth += GetAdvanceSubpixels(mpcFont->GetGlyphIndex(uiCodePoint));
	}
	cUTF8.Kill();
	return iWidth;
}


//////////////////////////////////////////////////////////////////////////
//
// The pixels a glyph covers with its pen origin at iPenX (subpixels) on
// the baseline.  Returns false for glyphs without an outline (and
// corrupt glyphs).
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::GetGlyphBounds(int iGlyph, int32 iPenX, STrueTypePixelBounds* psBounds)
{
	size			i;
	size			uiNumPoints;
	STrueTypePoint*	psPoint;
	int32			iXMin;
	int32			iYMin;
	int32			iXMax;
	int32			iYMax;

	mcOutline.Clear();
	if (!mpcFont->GetGlyphOutline(iGlyph, &mcOutline))
	{
		return false;
	}
	uiNumPoints = mcOutline.NumPoints();
	if (uiNumPoints == 0)
	{
		return false;
	}

	//Control points bound their curves.
	psPoint = mcOutline.GetPoint(0);
	iXMin = psPoint->x;
	iYMin = psPoint->y;
	iXMax = psPoint->x;
	iYMax = psPoint->y;
	for (i = 1; i < uiNumPoints; i++)
	{
		psPoint = mcOutline.GetPoint(i);
		if (psPoint->x < iXMin)
		{
			iXMin = psPoint->x;
		}
		if (psPoint->x > iXMax)
		{
			iXMax = psPoint->x;
		}
		if (psPoint->y < iYMin)
		{
			iYMin = psPoint->y;
		}
		if (psPoint->y > iYMax)
		{
			iYMax = psPoint->y;
		}
	}

	//Scaling keeps the order of points, so the scaled extremes are the extremes.
	psBounds->iLeft = FixedFloor(iPenX + Scale(iXMin), TRUE_TYPE_SUBPIXEL_BITS);
	psBounds->iRight = FixedCeiling(iPenX + Scale(iXMax), TRUE_TYPE_SUBPIXEL_BITS);
	psBounds->iTop = FixedFloor(-Scale(iYMax), TRUE_TYPE_SUBPIXEL_BITS);
	psBounds->iBottom = FixedCeiling(-Scale(iYMin), TRUE_TYPE_SUBPIXEL_BITS);
	return (psBounds->iRight > psBounds->iLeft) && (psBounds->iBottom > psBounds->iTop);
}


//////////////////////////////////////////////////////////////////////////
//
// The pixels text covers with the pen starting at the origin.  Returns
// false if no character has an outline.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::GetTextBounds(char* szUTF8, STrueTypePixelBounds* psBounds)
{
	CUTF8					cUTF8;
	uint32					uiCodePoint;
	int						iGlyph;
	int32					iX;
	STrueTypePixelBounds	sGlyph;
	bool					bAny;

	bAny = false;
	iX = 0;
	cUTF8.Init(szUTF8);
	for (;;)
	{
		uiCodePoint = cUTF8.GetCodePointUint32();
		if (uiCodePoint == 0)
		{
			break;
		}

		iGlyph = mpcFont->GetGlyphIndex(uiCodePoint);
		if (GetGlyphBounds(iGlyph, iX, &sGlyph))
		{
			if (!bAny)
			{
				*psBounds = sGlyph;
				bAny = true;
			}
			else
			{
				psBounds->iLeft = (sGlyph.iLeft < psBounds->iLeft) ? sGlyph.iLeft : psBounds->iLeft;
				psBounds->iTop = (sGlyph.iTop < psBounds->iTop) ? sGlyph.iTop : psBounds->iTop;
				psBounds->iRight = (sGlyph.iRight > psBounds->iRight) ? sGlyph.iRight : psBounds->iRight;
				psBounds->iBottom = (sGlyph.iBottom > psBounds->iBottom) ? sGlyph.iBottom : psBounds->iBottom;
			}
		}
		iX += GetAdvanceSubpixels(iGlyph);
	}
	cUTF8.Kill();
	return bAny;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns an opacity image just big enough for the glyph.  piLeft and
// piTop are where the image's top left is relative to the pen origin
// (piTop is usually negative).  Returns NULL for glyphs without an
// outline, such as space.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CTrueTypeRasteriser::RasteriseGlyph(int iGlyph, int* piLeft, int* piTop)
{
	STrueTypePixelBounds	sBounds;
	Ptr<CImage>				pcImage;

	*piLeft = 0;
	*piTop = 0;
	if (!GetGlyphBounds(iGlyph, 0, &sBounds))
	{
		return NULL;
	}
	if (!BeginCoverage(sBounds.iRight - sBounds.iLeft, sBounds.iBottom - sBounds.iTop))
	{
		return NULL;
	}

	AddGlyph(iGlyph, FixedFromInt(-sBounds.iLeft, TRUE_TYPE_SUBPIXEL_BITS), FixedFromInt(-sBounds.iTop, TRUE_TYPE_SUBPIXEL_BITS));
	Accumulate();
	pcImage = CoverageToImage();
	EndCoverage();

	*piLeft = sBounds.iLeft;
	*piTop = sBounds.iTop;
	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns an opacity image just big enough for one line of text.
// piOriginX and piOriginY are where the pen started on the baseline in
// the image.  Returns NULL if no character has an outline.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CTrueTypeRasteriser::RasteriseText(char* szUTF8, int* piOriginX, int* piOriginY)
{
	STrueTypePixelBounds	sBounds;
	Ptr<CImage>				pcImage;

	*piOriginX = 0;
	*piOriginY = 0;
	if (!GetTextBounds(szUTF8, &sBounds))
	{
		return NULL;
	}
	if (!BeginCoverage(sBounds.iRight - sBounds.iLeft, sBounds.iBottom - sBounds.iTop))
	{
		return NULL;
	}

	AddText(szUTF8, FixedFromInt(-sBounds.iLeft, TRUE_TYPE_SUBPIXEL_BITS), FixedFromInt(-sBounds.iTop, TRUE_TYPE_SUBPIXEL_BITS));
	Accumulate();
	pcImage = CoverageToImage();
	EndCoverage();

	*piOriginX = -sBounds.iLeft;
	*piOriginY = -sBounds.iTop;
	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Blends one line of text onto pcDest with the pen starting at fX on the
// baseline fY (both rounded to 1/256 of a pixel).  The colour's channels
// (red, green and blue for CImageColourRGB) are blended by coverage;
// pcDest must have them all.  Text outside pcDest is clipped.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::DrawText(Ptr<CImage> pcDest, float32 fX, float32 fY, char* szUTF8, CImageColour* pcColour)
{
	STrueTypePixelBounds	sBounds;
	int32					iX;
	int32					iY;
	int						iLeft;
	int						iTop;
	int						iRight;
	int						iBottom;
	bool					bResult;

	if (!GetTextBounds(szUTF8, &sBounds))
	{
		return true;
	}

	//The bounds are for a pen at a whole pixel; the fraction can move them one more pixel.
	iX = FixedFromFloat(fX, TRUE_TYPE_SUBPIXEL_BITS);
	iY = FixedFromFloat(fY, TRUE_TYPE_SUBPIXEL_BITS);
	iLeft = FixedFloor(iX, TRUE_TYPE_SUBPIXEL_BITS) + sBounds.iLeft;
	iRight = FixedCeiling(iX, TRUE_TYPE_SUBPIXEL_BITS) + sBounds.iRight;
	iTop = FixedFloor(iY, TRUE_TYPE_SUBPIXEL_BITS) + sBounds.iTop;
	iBottom = FixedCeiling(iY, TRUE_TYPE_SUBPIXEL_BITS) + sBounds.iBottom;

	if (iLeft < 0)
	{
		iLeft = 0;
	}
	if (iTop < 0)
	{
		iTop = 0;
	}
	if (iRight > pcDest->GetWidth())
	{
		iRight = pcDest->GetWidth();
	}
	if (iBottom > pcDest->GetHeight())
	{
		iBottom = pcDest->GetHeight();
	}
	if ((iRight <= iLeft) || (iBottom <= iTop))
	{
		return true;
	}

	if (!BeginCoverage(iRight - iLeft, iBottom - iTop))
	{
		return false;
	}
	AddText(szUTF8, iX - FixedFromInt(iLeft, TRUE_TYPE_SUBPIXEL_BITS), iY - FixedFromInt(iTop, TRUE_TYPE_SUBPIXEL_BITS));
	Accumulate();
	bResult = BlendCoverage(pcDest, iLeft, iTop, pcColour);
	EndCoverage();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::BeginCoverage(int iWidth, int iHeight)
{
	EndCoverage();

	miWidth = iWidth;
	miHeight = iHeight;
	mpiCover = (int32*)calloc((size)miWidth * miHeight, sizeof(int32));
	mpiArea = (int32*)calloc((size)miWidth * miHeight, sizeof(int32));
	mpuiCoverage = (uint8*)malloc((size)miWidth * miHeight);
	if ((mpiCover == NULL) || (mpiArea == NULL) || (mpuiCoverage == NULL))
	{
		EndCoverage();
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::EndCoverage(void)
{
	free(mpiCover);
	free(mpiArea);
	free(mpuiCoverage);
	mpiCover = NULL;
	mpiArea = NULL;
	mpuiCoverage = NULL;
	miWidth = 0;
	miHeight = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds one line of text with the pen starting at (iX, iY) in coverage
// buffer subpixels.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::AddText(char* szUTF8, int32 iX, int32 iY)
{
	CUTF8	cUTF8;
	uint32	uiCodePoint;
	int		iGlyph;
	bool	bResult;

	bResult = true;
	cUTF8.Init(szUTF8);
	for (;;)
	{
		uiCodePoint = cUTF8.GetCodePointUint32();
		if (uiCodePoint == 0)
		{
			break;
		}

		iGlyph = mpcFont->GetGlyphIndex(uiCodePoint);
		bResult &= AddGlyph(iGlyph, iX, iY);
		iX += GetAdvanceSubpixels(iGlyph);
	}
	cUTF8.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a glyph's edges with its pen origin at (iX, iY) in coverage buffer
// subpixels.  Points off the curve are quadratic control points; two in a
// row have an implied on curve point half way between them.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::AddGlyph(int iGlyph, int32 iX, int32 iY)
{
	size			uiContour;
	size			uiStart;
	size			uiNumPoints;
	size			uiFirstOn;
	size			k;
	STrueTypePoint*	psPoint;
	STrueTypePoint*	psLast;
	int32			iStartX;
	int32			iStartY;
	int32			iCurrentX;
	int32			iCurrentY;
	int32			iControlX;
	int32			iControlY;
	int32			iPointX;
	int32			iPointY;
	int32			iMiddleX;
	int32			iMiddleY;
	bool			bControl;

	mcOutline.Clear();
	if (!mpcFont->GetGlyphOutline(iGlyph, &mcOutline))
	{
		return false;
	}

	for (uiContour = 0; uiContour < mcOutline.NumContours(); uiContour++)
	{
		uiStart = mcOutline.GetContourStart(uiContour);
		uiNumPoints = mcOutline.GetContourEnd(uiContour) - uiStart + 1;

		for (uiFirstOn = 0; uiFirstOn < uiNumPoints; uiFirstOn++)
		{
			if (mcOutline.GetPoint(uiStart + uiFirstOn)->bOnCurve)
			{
				break;
			}
		}

		//Start on a point on the curve, or half way between the last and first points if there isn't one.
		if (uiFirstOn < uiNumPoints)
		{
			psPoint = mcOutline.GetPoint(uiStart + uiFirstOn);
			iStartX = iX + Scale(psPoint->x);
			iStartY = iY - Scale(psPoint->y);
		}
		else
		{
			psPoint = mcOutline.GetPoint(uiStart);
			psLast = mcOutline.GetPoint(uiStart + uiNumPoints - 1);
			iStartX = iX + (int32)DivideRound((int64)Scale(psPoint->x) + Scale(psLast->x), 2);
			iStartY = iY - (int32)DivideRound((int64)Scale(psPoint->y) + Scale(psLast->y), 2);
			uiFirstOn = uiNumPoints - 1;
		}

		iCurrentX = iStartX;
		iCurrentY = iStartY;
		iControlX = 0;
		iControlY = 0;
		bControl = false;
		for (k = 1; k <= uiNumPoints; k++)
		{
			psPoint = mcOutline.GetPoint(uiStart + (uiFirstOn + k) % uiNumPoints);
			iPointX = iX + Scale(psPoint->x);
			iPointY = iY - Scale(psPoint->y);

			if (psPoint->bOnCurve)
			{
				if (bControl)
				{
					AddQuadratic(iCurrentX, iCurrentY, iControlX, iControlY, iPointX, iPointY);
				}
				else
				{
					AddLine(iCurrentX, iCurrentY, iPointX, iPointY);
				}
				iCurrentX = iPointX;
				iCurrentY = iPointY;
				bControl = false;
			}
			else
			{
				if (bControl)
				{
					iMiddleX = (int32)DivideRound((int64)iControlX + iPointX, 2);
					iMiddleY = (int32)DivideRound((int64)iControlY + iPointY, 2);
					AddQuadratic(iCurrentX, iCurrentY, iControlX, iControlY, iMiddleX, iMiddleY);
					iCurrentX = iMiddleX;
					iCurrentY = iMiddleY;
				}
				iControlX = iPointX;
				iControlY = iPointY;
				bControl = true;
			}
		}

		//Close the contour.
		if (bControl)
		{
			AddQuadratic(iCurrentX, iCurrentY, iControlX, iControlY, iStartX, iStartY);
		}
		else
		{
			AddLine(iCurrentX, iCurrentY, iStartX, iStartY);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Splits a quadratic Bezier curve into lines; more lines the further the
// control point is from the chord.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::AddQuadratic(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2)
{
	int64	iDeviationX;
	int64	iDeviationY;
	int64	iDeviation;
	int64	iNumLines;
	int64	i;
	int64	iDivisor;
	int64	iWeight0;
	int64	iWeight1;
	int64	iWeight2;
	int32	iLastX;
	int32	iLastY;
	int32	x;
	int32	y;

	iDeviationX = (int64)x0 - 2 * (int64)x1 + x2;
	iDeviationY = (int64)y0 - 2 * (int64)y1 + y2;
	iDeviation = iDeviationX * iDeviationX + iDeviationY * iDeviationY;
	if (iDeviation < TRUE_TYPE_FLAT_DEVIATION)
	{
		AddLine(x0, y0, x2, y2);
		return;
	}

	//1 + the fourth root of three times the deviation in pixels squared.  A subpixel is 1/16th of a pixel's fourth root.
	iNumLines = 1 + SquareRoot(SquareRoot(3 * iDeviation)) / 16;

	//Points at t = i / n are ((n - i)^2 * p0 + 2 * i * (n - i) * p1 + i^2 * p2) / n^2.
	iDivisor = iNumLines * iNumLines;
	iLastX = x0;
	iLastY = y0;
	for (i = 1; i < iNumLines; i++)
	{
		iWeight0 = (iNumLines - i) * (iNumLines - i);
		iWeight1 = 2 * i * (iNumLines - i);
		iWeight2 = i * i;
		x = (int32)DivideRound(iWeight0 * x0 + iWeight1 * x1 + iWeight2 * x2, iDivisor);
		y = (int32)DivideRound(iWeight0 * y0 + iWeight1 * y1 + iWeight2 * y2, iDivisor);
		AddLine(iLastX, iLastY, x, y);
		iLastX = x;
		iLastY = y;
	}
	AddLine(iLastX, iLastY, x2, y2);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a line (in subpixels) one pixel row at a time.  Lines going down
// add coverage, lines going up subtract it.  Where a line crosses a row
// boundary is calculated the same way for both rows, so the pieces join
// exactly.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::AddLine(int32 x1, int32 y1, int32 x2, int32 y2)
{
	int32	iYMin;
	int32	iYMax;
	int		iRow;
	int		iRowStart;
	int		iRowEnd;
	int32	iTop;
	int32	iBottom;
	int32	iTopX;
	int32	iBottomX;

	if (y1 == y2)
	{
		return;
	}

	iYMin = (y1 < y2) ? y1 : y2;
	iYMax = (y1 < y2) ? y2 : y1;
	iRowStart = FixedFloor(iYMin, TRUE_TYPE_SUBPIXEL_BITS);
	if (iRowStart < 0)
	{
		iRowStart = 0;
	}
	iRowEnd = FixedCeiling(iYMax, TRUE_TYPE_SUBPIXEL_BITS);
	if (iRowEnd > miHeight)
	{
		iRowEnd = miHeight;
	}

	for (iRow = iRowStart; iRow < iRowEnd; iRow++)
	{
		iTop = FixedFromInt(iRow, TRUE_TYPE_SUBPIXEL_BITS);
		iBottom = FixedFromInt(iRow + 1, TRUE_TYPE_SUBPIXEL_BITS);
		iTop = (iTop > iYMin) ? iTop : iYMin;
		iBottom = (iBottom < iYMax) ? iBottom : iYMax;
		if (iTop >= iBottom)
		{
			continue;
		}

		iTopX = x1 + (int32)DivideRound((int64)(x2 - x1) * (iTop - y1), y2 - y1);
		iBottomX = x1 + (int32)DivideRound((int64)(x2 - x1) * (iBottom - y1), y2 - y1);
		if (y1 < y2)
		{
			AddRowLine(iRow, iTopX, iTop, iBottomX, iBottom);
		}
		else
		{
			AddRowLine(iRow, iBottomX, iBottom, iTopX, iTop);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Splits a line inside one pixel row where it crosses pixel columns.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::AddRowLine(int iRow, int32 x1, int32 y1, int32 x2, int32 y2)
{
	int32	iBoundary;
	int32	iStartX;
	int32	iStartY;
	int32	y;

	iStartX = x1;
	iStartY = y1;
	if (x1 < x2)
	{
		for (iBoundary = FixedFromInt(FixedFloor(x1, TRUE_TYPE_SUBPIXEL_BITS) + 1, TRUE_TYPE_SUBPIXEL_BITS); iBoundary < x2; iBoundary += TRUE_TYPE_SUBPIXELS)
		{
			y = y1 + (int32)DivideRound((int64)(y2 - y1) * (iBoundary - x1), x2 - x1);
			AddCellLine(iRow, iStartX, iStartY, iBoundary, y);
			iStartX = iBoundary;
			iStartY = y;
		}
	}
	else if (x1 > x2)
	{
		for (iBoundary = FixedFromInt(FixedCeiling(x1, TRUE_TYPE_SUBPIXEL_BITS) - 1, TRUE_TYPE_SUBPIXEL_BITS); iBoundary > x2; iBoundary -= TRUE_TYPE_SUBPIXELS)
		{
			y = y1 + (int32)DivideRound((int64)(y2 - y1) * (iBoundary - x1), x2 - x1);
			AddCellLine(iRow, iStartX, iStartY, iBoundary, y);
			iStartX = iBoundary;
			iStartY = y;
		}
	}
	AddCellLine(iRow, iStartX, iStartY, x2, y2);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a line inside one pixel.  The cover is its height and the area is
// twice the area between it and the pixel's left edge (times its
// height's sign).  Lines left of the buffer cover the first column
// completely; lines right of it are dropped.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::AddCellLine(int iRow, int32 x1, int32 y1, int32 x2, int32 y2)
{
	int32	iHeight;
	int		iColumn;
	int32	iLeft;
	int32	iX1;
	int32	iX2;
	size	uiCell;

	iHeight = y2 - y1;
	if (iHeight == 0)
	{
		return;
	}

	iColumn = FixedFloor((x1 < x2) ? x1 : x2, TRUE_TYPE_SUBPIXEL_BITS);
	if (iColumn >= miWidth)
	{
		return;
	}
	if (iColumn < 0)
	{
		iColumn = 0;
		iX1 = 0;
		iX2 = 0;
	}
	else
	{
		iLeft = FixedFromInt(iColumn, TRUE_TYPE_SUBPIXEL_BITS);
		iX1 = x1 - iLeft;
		iX2 = x2 - iLeft;
	}

	uiCell = (size)iRow * miWidth + iColumn;
	mpiCover[uiCell] += iHeight;
	mpiArea[uiCell] += (iX1 + iX2) * iHeight;
}


//////////////////////////////////////////////////////////////////////////
//
// Sums each row from the left into coverage from 0 to 255.  A pixel's
// coverage is the cover of the cells left of it (which cross it
// completely) less the part of its own cell's edges left of it.  Any
// winding counts as covered.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::Accumulate(void)
{
	int		x;
	int		y;
	size	uiCell;
	int64	iCover;
	int64	iCoverage;

	for (y = 0; y < miHeight; y++)
	{
		iCover = 0;
		for (x = 0; x < miWidth; x++)
		{
			uiCell = (size)y * miWidth + x;
			iCover += mpiCover[uiCell];
			iCoverage = iCover * 2 * TRUE_TYPE_SUBPIXELS - mpiArea[uiCell];
			if (iCoverage < 0)
			{
				iCoverage = -iCoverage;
			}
			iCoverage = DivideRound(iCoverage * 255, TRUE_TYPE_FULL_COVERAGE);
			mpuiCoverage[uiCell] = (uint8)((iCoverage > 255) ? 255 : iCoverage);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CTrueTypeRasteriser::CoverageToImage(void)
{
	Ptr<CImage>		pcImage;

	pcImage = OMalloc<CImage>(miWidth, miHeight, PT_uint8, IMAGE_OPACITY, CHANNEL_STOP);
	memcpy(pcImage->GetData(), mpuiCoverage, (size)miWidth * miHeight);
	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Blends the colour onto pcDest by the coverage, which starts at
// (iDestX, iDestY) and must lie inside pcDest.  Channels are blended as
// bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::BlendCoverage(Ptr<CImage> pcDest, int iDestX, int iDestY, CImageColour* pcColour)
{
	CArraySize				aiChannels;
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;
	size					uiNumChannels;
	size					i;
	int32					aiColour[MAX_IMAGE_COLOUR_BYTES];
	uint8					auiPixel[MAX_IMAGE_COLOUR_BYTES];
	int						x;
	int						y;
	int32					iCoverage;
	int32					iColour;

	aiChannels.Init();
	pcColour->GetChannels(&aiChannels);
	uiNumChannels = aiChannels.NumElements();
	if ((uiNumChannels == 0) || (uiNumChannels > MAX_IMAGE_COLOUR_BYTES))
	{
		aiChannels.Kill();
		return false;
	}
	for (i = 0; i < uiNumChannels; i++)
	{
		if (!pcDest->HasChannel(aiChannels.GetValue(i)))
		{
			aiChannels.Kill();
			return false;
		}
		iColour = FloatToIntScaled(pcColour->GetValue((EChannel)aiChannels.GetValue(i)), 255);
		aiColour[i] = (iColour < 0) ? 0 : ((iColour > 255) ? 255 : iColour);
	}

	cCreator.Init(&pcDest);
	cCreator.AddAccess(&aiChannels, PT_uint8);
	pcAccessor = cCreator.CreateAndKill();
	aiChannels.Kill();
	if (pcAccessor == NULL)
	{
		return false;
	}

	for (y = 0; y < miHeight; y++)
	{
		for (x = 0; x < miWidth; x++)
		{
			iCoverage = mpuiCoverage[(size)y * miWidth + x];
			if (iCoverage == 0)
			{
				continue;
			}
			pcAccessor->Get(iDestX + x, iDestY + y, auiPixel);
			for (i = 0; i < uiNumChannels; i++)
			{
				auiPixel[i] = (uint8)DivideRound((int64)auiPixel[i] * (255 - iCoverage) + (int64)aiColour[i] * iCoverage, 255);
			}
			pcAccessor->Set(iDestX + x, iDestY + y, auiPixel);
		}
	}
	pcAccessor->Kill();
	return true;
}
