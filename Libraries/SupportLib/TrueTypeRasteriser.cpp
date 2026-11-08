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
#include "ColourBlend.h"
#include "ImageAccessorCreator.h"
#include "TrueTypeRasteriser.h"


//////////////////////////////////////////////////////////////////////////
//
// fPixelSize is the height of one em in pixels.  It is rounded to 1/256
// of a pixel.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::Init(CTrueTypeFont* pcFont, float32 fPixelSize)
{
	mpcFont = pcFont;
	msPixelSize = SCoverageSubpixel::FromFloat(fPixelSize);
	miUnitsPerEm = pcFont->GetUnitsPerEm();
	mcOutline.Init();
	mcCoverage.Init();
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
	return msPixelSize.ToFloat() / (float32)miUnitsPerEm;
}


//////////////////////////////////////////////////////////////////////////
//
// Font units to subpixels, rounded.
//
//////////////////////////////////////////////////////////////////////////
SCoverageSubpixel CTrueTypeRasteriser::Scale(int32 iFontUnits)
{
	return (SCoverageWhole::FromRaw(iFontUnits) * msPixelSize).DivideRound(SCoverageWhole::FromRaw(miUnitsPerEm)).To<SCoverageSubpixel>();
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels above the baseline.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetAscent(void)
{
	return GetAscentSubpixels().ToFloat();
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels below the baseline, positive.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetDescent(void)
{
	return GetDescentSubpixels().ToFloat();
}


//////////////////////////////////////////////////////////////////////////
//
// The distance between baselines.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetLineHeight(void)
{
	return Scale(mpcFont->GetAscender() - mpcFont->GetDescender() + mpcFont->GetLineGap()).ToFloat();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetAdvance(int iGlyph)
{
	return GetAdvanceSubpixels(iGlyph).ToFloat();
}


//////////////////////////////////////////////////////////////////////////
//
// The sum of the advances of the characters.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrueTypeRasteriser::GetTextWidth(char* szUTF8)
{
	return GetTextWidthSubpixels(szUTF8).ToFloat();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCoverageSubpixel CTrueTypeRasteriser::GetAscentSubpixels(void)
{
	return Scale(mpcFont->GetAscender());
}


//////////////////////////////////////////////////////////////////////////
//
// Positive.
//
//////////////////////////////////////////////////////////////////////////
SCoverageSubpixel CTrueTypeRasteriser::GetDescentSubpixels(void)
{
	return Scale(-mpcFont->GetDescender());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCoverageSubpixel CTrueTypeRasteriser::GetAdvanceSubpixels(int iGlyph)
{
	int		iAdvanceWidth;

	if (!mpcFont->GetGlyphAdvanceWidth(iGlyph, &iAdvanceWidth))
	{
		return SCoverageSubpixel::FromRaw(0);
	}
	return Scale(iAdvanceWidth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCoverageSubpixel CTrueTypeRasteriser::GetTextWidthSubpixels(char* szUTF8)
{
	CUTF8				cUTF8;
	uint32				uiCodePoint;
	SCoverageSubpixel	sWidth;

	sWidth = SCoverageSubpixel::FromRaw(0);
	cUTF8.Init(szUTF8);
	for (;;)
	{
		uiCodePoint = cUTF8.GetCodePointUint32();
		if (uiCodePoint == 0)
		{
			break;
		}
		sWidth += GetAdvanceSubpixels(mpcFont->GetGlyphIndex(uiCodePoint));
	}
	cUTF8.Kill();
	return sWidth;
}


//////////////////////////////////////////////////////////////////////////
//
// The pixels a glyph covers with its pen origin at sPenX on
// the baseline.  Returns false for glyphs without an outline (and
// corrupt glyphs).
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::GetGlyphBounds(int iGlyph, SCoverageSubpixel sPenX, STrueTypePixelBounds* psBounds)
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
	psBounds->iLeft = (sPenX + Scale(iXMin)).Floor();
	psBounds->iRight = (sPenX + Scale(iXMax)).Ceiling();
	psBounds->iTop = (-Scale(iYMax)).Floor();
	psBounds->iBottom = (-Scale(iYMin)).Ceiling();
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
	SCoverageSubpixel		sX;
	STrueTypePixelBounds	sGlyph;
	bool					bAny;

	bAny = false;
	sX = SCoverageSubpixel::FromRaw(0);
	cUTF8.Init(szUTF8);
	for (;;)
	{
		uiCodePoint = cUTF8.GetCodePointUint32();
		if (uiCodePoint == 0)
		{
			break;
		}

		iGlyph = mpcFont->GetGlyphIndex(uiCodePoint);
		if (GetGlyphBounds(iGlyph, sX, &sGlyph))
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
		sX += GetAdvanceSubpixels(iGlyph);
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
	Ptr<CImage>				pImage;

	*piLeft = 0;
	*piTop = 0;
	if (!GetGlyphBounds(iGlyph, SCoverageSubpixel::FromRaw(0), &sBounds))
	{
		return NULL;
	}
	if (!BeginCoverage(sBounds.iRight - sBounds.iLeft, sBounds.iBottom - sBounds.iTop))
	{
		return NULL;
	}

	AddGlyph(iGlyph, SCoverageSubpixel::FromInt(-sBounds.iLeft), SCoverageSubpixel::FromInt(-sBounds.iTop));
	mcCoverage.Accumulate();
	pImage = CoverageToImage();
	EndCoverage();

	*piLeft = sBounds.iLeft;
	*piTop = sBounds.iTop;
	return pImage;
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
	Ptr<CImage>				pImage;

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

	AddText(szUTF8, SCoverageSubpixel::FromInt(-sBounds.iLeft), SCoverageSubpixel::FromInt(-sBounds.iTop));
	mcCoverage.Accumulate();
	pImage = CoverageToImage();
	EndCoverage();

	*piOriginX = -sBounds.iLeft;
	*piOriginY = -sBounds.iTop;
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Blends one line of text onto pcDest with the pen starting at fX on the
// baseline fY (both rounded to 1/256 of a pixel).  The colour's channels
// (red, green and blue for CImageColourRGB) are blended by coverage;
// pcDest must have them all.  Text outside pcDest is clipped.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::DrawText(Ptr<CImage> pDest, float32 fX, float32 fY, char* szUTF8, CImageColour* pcColour)
{
	STrueTypePixelBounds	sBounds;
	SCoverageSubpixel		sX;
	SCoverageSubpixel		sY;
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
	sX = SCoverageSubpixel::FromFloat(fX);
	sY = SCoverageSubpixel::FromFloat(fY);
	iLeft = sX.Floor() + sBounds.iLeft;
	iRight = sX.Ceiling() + sBounds.iRight;
	iTop = sY.Floor() + sBounds.iTop;
	iBottom = sY.Ceiling() + sBounds.iBottom;

	if (iLeft < 0)
	{
		iLeft = 0;
	}
	if (iTop < 0)
	{
		iTop = 0;
	}
	//The image size is unsigned; compare as int so a glyph wholly left of or above the image (negative right or bottom) is not clamped to the full size.
	if (iRight > (int)pDest->GetWidth())
	{
		iRight = (int)pDest->GetWidth();
	}
	if (iBottom > (int)pDest->GetHeight())
	{
		iBottom = (int)pDest->GetHeight();
	}
	if ((iRight <= iLeft) || (iBottom <= iTop))
	{
		return true;
	}

	if (!BeginCoverage(iRight - iLeft, iBottom - iTop))
	{
		return false;
	}
	AddText(szUTF8, sX - SCoverageSubpixel::FromInt(iLeft), sY - SCoverageSubpixel::FromInt(iTop));
	mcCoverage.Accumulate();
	bResult = BlendCoverage(pDest, iLeft, iTop, pcColour);
	EndCoverage();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::BeginCoverage(int iWidth, int iHeight)
{
	return mcCoverage.Begin(iWidth, iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeRasteriser::EndCoverage(void)
{
	mcCoverage.End();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds one line of text with the pen starting at (sX, sY) in coverage
// buffer subpixels.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::AddText(char* szUTF8, SCoverageSubpixel sX, SCoverageSubpixel sY)
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
		bResult &= AddGlyph(iGlyph, sX, sY);
		sX += GetAdvanceSubpixels(iGlyph);
	}
	cUTF8.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a glyph's edges with its pen origin at (sX, sY) in coverage buffer
// subpixels.  Points off the curve are quadratic control points; two in a
// row have an implied on curve point half way between them.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::AddGlyph(int iGlyph, SCoverageSubpixel sX, SCoverageSubpixel sY)
{
	size			uiContour;
	size			uiStart;
	size			uiNumPoints;
	size			uiFirstOn;
	size			k;
	STrueTypePoint*	psPoint;
	STrueTypePoint*	psLast;
	SCoverageWhole	sTwo;
	SCoverageSubpixel	sStartX;
	SCoverageSubpixel	sStartY;
	SCoverageSubpixel	sCurrentX;
	SCoverageSubpixel	sCurrentY;
	SCoverageSubpixel	sControlX;
	SCoverageSubpixel	sControlY;
	SCoverageSubpixel	sPointX;
	SCoverageSubpixel	sPointY;
	SCoverageSubpixel	sMiddleX;
	SCoverageSubpixel	sMiddleY;
	bool			bControl;

	sTwo = SCoverageWhole::FromRaw(2);
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
			sStartX = sX + Scale(psPoint->x);
			sStartY = sY - Scale(psPoint->y);
		}
		else
		{
			psPoint = mcOutline.GetPoint(uiStart);
			psLast = mcOutline.GetPoint(uiStart + uiNumPoints - 1);
			sStartX = sX + (Scale(psPoint->x).To<SCoverageWideSubpixel>() + Scale(psLast->x).To<SCoverageWideSubpixel>()).DivideRound(sTwo).To<SCoverageSubpixel>();
			sStartY = sY - (Scale(psPoint->y).To<SCoverageWideSubpixel>() + Scale(psLast->y).To<SCoverageWideSubpixel>()).DivideRound(sTwo).To<SCoverageSubpixel>();
			uiFirstOn = uiNumPoints - 1;
		}

		sCurrentX = sStartX;
		sCurrentY = sStartY;
		sControlX = SCoverageSubpixel::FromRaw(0);
		sControlY = SCoverageSubpixel::FromRaw(0);
		bControl = false;
		for (k = 1; k <= uiNumPoints; k++)
		{
			psPoint = mcOutline.GetPoint(uiStart + (uiFirstOn + k) % uiNumPoints);
			sPointX = sX + Scale(psPoint->x);
			sPointY = sY - Scale(psPoint->y);

			if (psPoint->bOnCurve)
			{
				if (bControl)
				{
					mcCoverage.AddQuadratic(sCurrentX, sCurrentY, sControlX, sControlY, sPointX, sPointY);
				}
				else
				{
					mcCoverage.AddLine(sCurrentX, sCurrentY, sPointX, sPointY);
				}
				sCurrentX = sPointX;
				sCurrentY = sPointY;
				bControl = false;
			}
			else
			{
				if (bControl)
				{
					sMiddleX = (sControlX.To<SCoverageWideSubpixel>() + sPointX.To<SCoverageWideSubpixel>()).DivideRound(sTwo).To<SCoverageSubpixel>();
					sMiddleY = (sControlY.To<SCoverageWideSubpixel>() + sPointY.To<SCoverageWideSubpixel>()).DivideRound(sTwo).To<SCoverageSubpixel>();
					mcCoverage.AddQuadratic(sCurrentX, sCurrentY, sControlX, sControlY, sMiddleX, sMiddleY);
					sCurrentX = sMiddleX;
					sCurrentY = sMiddleY;
				}
				sControlX = sPointX;
				sControlY = sPointY;
				bControl = true;
			}
		}

		//Close the contour.
		if (bControl)
		{
			mcCoverage.AddQuadratic(sCurrentX, sCurrentY, sControlX, sControlY, sStartX, sStartY);
		}
		else
		{
			mcCoverage.AddLine(sCurrentX, sCurrentY, sStartX, sStartY);
		}
	}
	return true;
}








//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CTrueTypeRasteriser::CoverageToImage(void)
{
	Ptr<CImage>		pImage;

	pImage = OMalloc<CImage>(mcCoverage.GetWidth(), mcCoverage.GetHeight(), PT_uint8, IMAGE_OPACITY, CHANNEL_STOP);
	memcpy(pImage->GetData(), mcCoverage.GetCoverage(), (lsize)mcCoverage.GetWidth() * mcCoverage.GetHeight());
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Blends the colour onto pcDest by the coverage, which starts at
// (iDestX, iDestY) and must lie inside pcDest.  Channels are blended as
// bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeRasteriser::BlendCoverage(Ptr<CImage> pDest, int iDestX, int iDestY, CImageColour* pcColour)
{
	CArraySize				aiChannels;
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;
	size					uiNumChannels;
	size					i;
	size					auiColour[MAX_IMAGE_COLOUR_BYTES];
	uint8					auiPixel[MAX_IMAGE_COLOUR_BYTES];
	uint					x;
	uint					y;
	size					uiCoverage;
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
		if (!pDest->HasChannel(aiChannels.GetValue(i)))
		{
			aiChannels.Kill();
			return false;
		}
		iColour = FloatToIntScaled(pcColour->GetValue((EChannel)aiChannels.GetValue(i)), 255);
		auiColour[i] = (iColour < 0) ? 0 : ((iColour > 255) ? 255 : (size)iColour);
	}

	cCreator.Init(&pDest);
	cCreator.AddAccess(&aiChannels, PT_uint8);
	pcAccessor = cCreator.CreateAndKill();
	aiChannels.Kill();
	if (pcAccessor == NULL)
	{
		return false;
	}

	for (y = 0; y < (uint)mcCoverage.GetHeight(); y++)
	{
		for (x = 0; x < (uint)mcCoverage.GetWidth(); x++)
		{
			uiCoverage = mcCoverage.GetCoverage()[(lsize)y * mcCoverage.GetWidth() + x];
			if (uiCoverage == 0)
			{
				continue;
			}
			pcAccessor->Get(iDestX + x, iDestY + y, auiPixel);
			for (i = 0; i < uiNumChannels; i++)
			{
				auiPixel[i] = BlendLerpByte(auiPixel[i], auiColour[i], uiCoverage);
			}
			pcAccessor->Set(iDestX + x, iDestY + y, auiPixel);
		}
	}
	pcAccessor->Kill();
	return true;
}
