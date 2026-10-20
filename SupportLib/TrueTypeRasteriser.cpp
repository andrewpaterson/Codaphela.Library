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
#include <math.h>
#include "BaseLib/UTF8.h"
#include "BaseLib/ArraySize.h"
#include "ImageAccessorCreator.h"
#include "TrueTypeRasteriser.h"


//Curves are split into more lines the further their control point is from the chord.
#define TRUE_TYPE_FLATNESS_TOLERANCE	3.0f
#define TRUE_TYPE_FLAT_DEVIATION		0.333f

//The accumulation buffer has two spare columns on the right so lines on the right edge don't touch the next row.
#define TRUE_TYPE_COVERAGE_PADDING		2


//////////////////////////////////////////////////////////////////////////
//
// fPixelSize is the height of one em in pixels.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::Init(CTrueTypeFont* pcFont, float32 fPixelSize)
{
	mpcFont = pcFont;
	mfScale = fPixelSize / (float32)pcFont->GetUnitsPerEm();
	mcOutline.Init();
	mpfCoverage = NULL;
	miWidth = 0;
	miHeight = 0;
	miStride = 0;
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
	return mfScale;
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels above the baseline.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetAscent(void)
{
	return mpcFont->GetAscender() * mfScale;
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels below the baseline, positive.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetDescent(void)
{
	return -mpcFont->GetDescender() * mfScale;
}


//////////////////////////////////////////////////////////////////////////
//
// The distance between baselines.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetLineHeight(void)
{
	return (mpcFont->GetAscender() - mpcFont->GetDescender() + mpcFont->GetLineGap()) * mfScale;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetAdvance(int iGlyph)
{
	STrueTypeGlyphMetrics	sMetrics;

	if (!mpcFont->GetGlyphMetrics(iGlyph, &sMetrics))
	{
		return 0.0f;
	}
	return sMetrics.iAdvanceWidth * mfScale;
}


//////////////////////////////////////////////////////////////////////////
//
// The sum of the advances of the characters.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetTextWidth(char* szUTF8)
{
	CUTF8	cUTF8;
	uint32	uiCodePoint;
	float32	fWidth;

	fWidth = 0.0f;
	cUTF8.Init(szUTF8);
	for (;;)
	{
		uiCodePoint = cUTF8.GetCodePointUint32();
		if (uiCodePoint == 0)
		{
			break;
		}
		fWidth += GetAdvance(mpcFont->GetGlyphIndex(uiCodePoint));
	}
	cUTF8.Kill();
	return fWidth;
}


//////////////////////////////////////////////////////////////////////////
//
// The pixels a glyph covers with its pen origin at fX on the baseline.
// Returns false for glyphs without an outline (and corrupt glyphs).
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::GetGlyphBounds(int iGlyph, float32 fX, STrueTypePixelBounds* psBounds)
{
	size			i;
	size			uiNumPoints;
	STrueTypePoint*	psPoint;
	float32			fXMin;
	float32			fYMin;
	float32			fXMax;
	float32			fYMax;

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
	fXMin = psPoint->x;
	fYMin = psPoint->y;
	fXMax = psPoint->x;
	fYMax = psPoint->y;
	for (i = 1; i < uiNumPoints; i++)
	{
		psPoint = mcOutline.GetPoint(i);
		if (psPoint->x < fXMin)
		{
			fXMin = psPoint->x;
		}
		if (psPoint->x > fXMax)
		{
			fXMax = psPoint->x;
		}
		if (psPoint->y < fYMin)
		{
			fYMin = psPoint->y;
		}
		if (psPoint->y > fYMax)
		{
			fYMax = psPoint->y;
		}
	}

	psBounds->iLeft = (int)floorf(fX + fXMin * mfScale);
	psBounds->iRight = (int)ceilf(fX + fXMax * mfScale);
	psBounds->iTop = (int)floorf(-fYMax * mfScale);
	psBounds->iBottom = (int)ceilf(-fYMin * mfScale);
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
	float32					fX;
	STrueTypePixelBounds	sGlyph;
	bool					bAny;

	bAny = false;
	fX = 0.0f;
	cUTF8.Init(szUTF8);
	for (;;)
	{
		uiCodePoint = cUTF8.GetCodePointUint32();
		if (uiCodePoint == 0)
		{
			break;
		}

		iGlyph = mpcFont->GetGlyphIndex(uiCodePoint);
		if (GetGlyphBounds(iGlyph, fX, &sGlyph))
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
		fX += GetAdvance(iGlyph);
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
	if (!GetGlyphBounds(iGlyph, 0.0f, &sBounds))
	{
		return NULL;
	}
	if (!BeginCoverage(sBounds.iRight - sBounds.iLeft, sBounds.iBottom - sBounds.iTop))
	{
		return NULL;
	}

	AddGlyph(iGlyph, (float32)-sBounds.iLeft, (float32)-sBounds.iTop);
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

	AddText(szUTF8, (float32)-sBounds.iLeft, (float32)-sBounds.iTop);
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
// baseline fY.  The colour's channels (red, green and blue for
// CImageColourRGB) are blended by coverage; pcDest must have them all.
// Text outside pcDest is clipped.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::DrawText(Ptr<CImage> pcDest, float32 fX, float32 fY, char* szUTF8, CImageColour* pcColour)
{
	STrueTypePixelBounds	sBounds;
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
	iLeft = (int)floorf(fX) + sBounds.iLeft;
	iRight = (int)ceilf(fX) + sBounds.iRight;
	iTop = (int)floorf(fY) + sBounds.iTop;
	iBottom = (int)ceilf(fY) + sBounds.iBottom;

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
	AddText(szUTF8, fX - iLeft, fY - iTop);
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
	miStride = iWidth + TRUE_TYPE_COVERAGE_PADDING;
	mpfCoverage = (float32*)calloc((size)miStride * miHeight, sizeof(float32));
	return mpfCoverage != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::EndCoverage(void)
{
	if (mpfCoverage)
	{
		free(mpfCoverage);
		mpfCoverage = NULL;
	}
	miWidth = 0;
	miHeight = 0;
	miStride = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds one line of text with the pen starting at (fX, fY) in coverage
// buffer pixels.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::AddText(char* szUTF8, float32 fX, float32 fY)
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
		bResult &= AddGlyph(iGlyph, fX, fY);
		fX += GetAdvance(iGlyph);
	}
	cUTF8.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a glyph's edges with its pen origin at (fX, fY) in coverage buffer
// pixels.  Points off the curve are quadratic control points; two in a
// row have an implied on curve point half way between them.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::AddGlyph(int iGlyph, float32 fX, float32 fY)
{
	size			uiContour;
	size			uiStart;
	size			uiNumPoints;
	size			uiFirstOn;
	size			k;
	size			uiNumSteps;
	STrueTypePoint*	psPoint;
	STrueTypePoint*	psLast;
	float32			fStartX;
	float32			fStartY;
	float32			fCurrentX;
	float32			fCurrentY;
	float32			fControlX;
	float32			fControlY;
	float32			fPointX;
	float32			fPointY;
	float32			fMiddleX;
	float32			fMiddleY;
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
			fStartX = psPoint->x;
			fStartY = psPoint->y;
			uiNumSteps = uiNumPoints;
		}
		else
		{
			psPoint = mcOutline.GetPoint(uiStart);
			psLast = mcOutline.GetPoint(uiStart + uiNumPoints - 1);
			fStartX = (psPoint->x + psLast->x) * 0.5f;
			fStartY = (psPoint->y + psLast->y) * 0.5f;
			uiFirstOn = uiNumPoints - 1;
			uiNumSteps = uiNumPoints;
		}
		fStartX = fX + fStartX * mfScale;
		fStartY = fY - fStartY * mfScale;

		fCurrentX = fStartX;
		fCurrentY = fStartY;
		fControlX = 0.0f;
		fControlY = 0.0f;
		bControl = false;
		for (k = 1; k <= uiNumSteps; k++)
		{
			psPoint = mcOutline.GetPoint(uiStart + (uiFirstOn + k) % uiNumPoints);
			fPointX = fX + psPoint->x * mfScale;
			fPointY = fY - psPoint->y * mfScale;

			if (psPoint->bOnCurve)
			{
				if (bControl)
				{
					AddQuadratic(fCurrentX, fCurrentY, fControlX, fControlY, fPointX, fPointY);
				}
				else
				{
					AddLine(fCurrentX, fCurrentY, fPointX, fPointY);
				}
				fCurrentX = fPointX;
				fCurrentY = fPointY;
				bControl = false;
			}
			else
			{
				if (bControl)
				{
					fMiddleX = (fControlX + fPointX) * 0.5f;
					fMiddleY = (fControlY + fPointY) * 0.5f;
					AddQuadratic(fCurrentX, fCurrentY, fControlX, fControlY, fMiddleX, fMiddleY);
					fCurrentX = fMiddleX;
					fCurrentY = fMiddleY;
				}
				fControlX = fPointX;
				fControlY = fPointY;
				bControl = true;
			}
		}

		//Close the contour.
		if (bControl)
		{
			AddQuadratic(fCurrentX, fCurrentY, fControlX, fControlY, fStartX, fStartY);
		}
		else
		{
			AddLine(fCurrentX, fCurrentY, fStartX, fStartY);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Splits a quadratic Bezier curve into lines.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::AddQuadratic(float32 x0, float32 y0, float32 x1, float32 y1, float32 x2, float32 y2)
{
	float32	fDeviationX;
	float32	fDeviationY;
	float32	fDeviation;
	int		iNumLines;
	int		i;
	float32	t;
	float32	u;
	float32	fLastX;
	float32	fLastY;
	float32	x;
	float32	y;

	fDeviationX = x0 - 2.0f * x1 + x2;
	fDeviationY = y0 - 2.0f * y1 + y2;
	fDeviation = fDeviationX * fDeviationX + fDeviationY * fDeviationY;
	if (fDeviation < TRUE_TYPE_FLAT_DEVIATION)
	{
		AddLine(x0, y0, x2, y2);
		return;
	}

	iNumLines = 1 + (int)floorf(sqrtf(sqrtf(TRUE_TYPE_FLATNESS_TOLERANCE * fDeviation)));
	fLastX = x0;
	fLastY = y0;
	for (i = 1; i < iNumLines; i++)
	{
		t = (float32)i / (float32)iNumLines;
		u = 1.0f - t;
		x = u * u * x0 + 2.0f * u * t * x1 + t * t * x2;
		y = u * u * y0 + 2.0f * u * t * y1 + t * t * y2;
		AddLine(fLastX, fLastY, x, y);
		fLastX = x;
		fLastY = y;
	}
	AddLine(fLastX, fLastY, x2, y2);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the signed area a line covers to the accumulation buffer.  Summing
// a row from the left then gives the coverage of each pixel.  Lines going
// down add, lines going up subtract.  Parts of a line left of the buffer
// count as being on its left edge and parts right of it are dropped.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::AddLine(float32 x0, float32 y0, float32 x1, float32 y1)
{
	float32		fDirection;
	float32		fSwap;
	float32		fDxDy;
	float32		x;
	float32		fXNext;
	int			y;
	int			iYStart;
	int			iYEnd;
	float32*	pfRow;
	float32		fDY;
	float32		fD;
	float32		fXA;
	float32		fXB;
	float32		fXAFloor;
	int			iXA;
	int			iXB;
	float32		fXMiddle;
	float32		fSlope;
	float32		fXAFraction;
	float32		fXBFraction;
	float32		fAreaA;
	float32		fAreaB;
	float32		fAreaMiddle;
	float32		fAreaEnd;
	int			i;
	float32		fWidth;

	if (y0 == y1)
	{
		return;
	}

	fDirection = 1.0f;
	if (y0 > y1)
	{
		fDirection = -1.0f;
		fSwap = x0;  x0 = x1;  x1 = fSwap;
		fSwap = y0;  y0 = y1;  y1 = fSwap;
	}

	fDxDy = (x1 - x0) / (y1 - y0);
	x = x0;
	if (y0 < 0.0f)
	{
		x -= y0 * fDxDy;
	}

	iYStart = (int)floorf(y0);
	if (iYStart < 0)
	{
		iYStart = 0;
	}
	iYEnd = (int)ceilf(y1);
	if (iYEnd > miHeight)
	{
		iYEnd = miHeight;
	}

	fWidth = (float32)miWidth;
	for (y = iYStart; y < iYEnd; y++)
	{
		pfRow = &mpfCoverage[y * miStride];
		fDY = (((float32)(y + 1) < y1) ? (float32)(y + 1) : y1) - (((float32)y > y0) ? (float32)y : y0);
		fXNext = x + fDxDy * fDY;
		fD = fDY * fDirection;

		fXA = (x < fXNext) ? x : fXNext;
		fXB = (x < fXNext) ? fXNext : x;
		fXA = (fXA < 0.0f) ? 0.0f : ((fXA > fWidth) ? fWidth : fXA);
		fXB = (fXB < 0.0f) ? 0.0f : ((fXB > fWidth) ? fWidth : fXB);

		fXAFloor = floorf(fXA);
		iXA = (int)fXAFloor;
		iXB = (int)ceilf(fXB);

		if (iXB <= iXA + 1)
		{
			//The line is in one pixel column on this row.
			fXMiddle = 0.5f * (fXA + fXB) - fXAFloor;
			pfRow[iXA] += fD - fD * fXMiddle;
			pfRow[iXA + 1] += fD * fXMiddle;
		}
		else
		{
			//The line crosses columns; the area right of it in each column grows linearly between the ends.
			fSlope = 1.0f / (fXB - fXA);
			fXAFraction = fXA - fXAFloor;
			fAreaA = 0.5f * fSlope * (1.0f - fXAFraction) * (1.0f - fXAFraction);
			fXBFraction = fXB - (float32)iXB + 1.0f;
			fAreaEnd = 0.5f * fSlope * fXBFraction * fXBFraction;

			pfRow[iXA] += fD * fAreaA;
			if (iXB == iXA + 2)
			{
				pfRow[iXA + 1] += fD * (1.0f - fAreaA - fAreaEnd);
			}
			else
			{
				fAreaB = fSlope * (1.5f - fXAFraction);
				pfRow[iXA + 1] += fD * (fAreaB - fAreaA);
				for (i = iXA + 2; i < iXB - 1; i++)
				{
					pfRow[i] += fD * fSlope;
				}
				fAreaMiddle = fAreaB + (float32)(iXB - iXA - 3) * fSlope;
				pfRow[iXB - 1] += fD * (1.0f - fAreaMiddle - fAreaEnd);
			}
			pfRow[iXB] += fD * fAreaEnd;
		}
		x = fXNext;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Turns the accumulated signed areas into coverage from 0 to 1.  Any
// winding counts as covered.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::Accumulate(void)
{
	int			x;
	int			y;
	float32*	pfRow;
	float32		fSum;
	float32		fCoverage;

	for (y = 0; y < miHeight; y++)
	{
		pfRow = &mpfCoverage[y * miStride];
		fSum = 0.0f;
		for (x = 0; x < miWidth; x++)
		{
			fSum += pfRow[x];
			fCoverage = fabsf(fSum);
			pfRow[x] = (fCoverage > 1.0f) ? 1.0f : fCoverage;
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
	uint8*			puiData;
	int				x;
	int				y;
	float32*		pfRow;

	pcImage = OMalloc<CImage>(miWidth, miHeight, PT_uint8, IMAGE_OPACITY, CHANNEL_STOP);
	puiData = (uint8*)pcImage->GetData();
	for (y = 0; y < miHeight; y++)
	{
		pfRow = &mpfCoverage[y * miStride];
		for (x = 0; x < miWidth; x++)
		{
			puiData[y * miWidth + x] = (uint8)(pfRow[x] * 255.0f + 0.5f);
		}
	}
	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Blends the colour onto pcDest by the coverage, which starts at
// (iDestX, iDestY) and must lie inside pcDest.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::BlendCoverage(Ptr<CImage> pcDest, int iDestX, int iDestY, CImageColour* pcColour)
{
	CArraySize				aiChannels;
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;
	size					uiNumChannels;
	size					i;
	float32					afColour[MAX_IMAGE_COLOUR_BYTES];
	float32					afPixel[MAX_IMAGE_COLOUR_BYTES];
	int						x;
	int						y;
	float32*				pfRow;
	float32					fCoverage;

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
		afColour[i] = pcColour->GetValue((EChannel)aiChannels.GetValue(i));
	}

	cCreator.Init(&pcDest);
	cCreator.AddAccess(&aiChannels, PT_float32);
	pcAccessor = cCreator.CreateAndKill();
	aiChannels.Kill();
	if (pcAccessor == NULL)
	{
		return false;
	}

	for (y = 0; y < miHeight; y++)
	{
		pfRow = &mpfCoverage[y * miStride];
		for (x = 0; x < miWidth; x++)
		{
			fCoverage = pfRow[x];
			if (fCoverage <= 0.0f)
			{
				continue;
			}
			pcAccessor->Get(iDestX + x, iDestY + y, afPixel);
			for (i = 0; i < uiNumChannels; i++)
			{
				afPixel[i] += (afColour[i] - afPixel[i]) * fCoverage;
			}
			pcAccessor->Set(iDestX + x, iDestY + y, afPixel);
		}
	}
	pcAccessor->Kill();
	return true;
}
