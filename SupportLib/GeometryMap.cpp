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
#include <math.h>
#include "BaseLib/FixedPoint.h"
#include "BaseLib/Logger.h"
#include "StandardLib/ClassDefines.h"
#include "ImageAccessorCreator.h"
#include "ColourBlend.h"
#include "VectorShapes.h"
#include "GeometryMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::Init(void)
{
	PreInit();
	CBaseMap::Init();
	masCommands.Init();
	mpViewport = NULL;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::Free(void)
{
	masCommands.Kill();
	CBaseMap::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::Class(void)
{
	CBaseMap::Class();
	U_Data(CArrayGeometryCommand, masCommands);
	M_Pointer(mpViewport);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGeometryMap::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGeometryMap::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::Clear(void)
{
	masCommands.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// A filled rectangle in map pixels.  An empty rectangle isn't kept.
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::AddRectangle(int32 iLeft, int32 iTop, int32 iWidth, int32 iHeight, ARGB32 sColour)
{
	SGeometryCommand*	psCommand;

	if ((iWidth <= 0) || (iHeight <= 0))
	{
		return;
	}
	psCommand = masCommands.Add();
	psCommand->eType = GC_Rectangle;
	psCommand->cBounds.Init(iLeft, iTop, iLeft + iWidth, iTop + iHeight);
	psCommand->sColour = sColour;
	psCommand->sStart.Init(0, 0);
	psCommand->sEnd.Init(0, 0);
	psCommand->fRadius = 0;
	psCommand->fThickness = 0;
	psCommand->eCap = GLC_Butt;
}


//////////////////////////////////////////////////////////////////////////
//
// A line fThickness pixels thick centred on the points from x0, y0 to
// x1, y1.  A line with no thickness isn't kept, nor is a butt capped line
// with no length.  A square or round capped line with no length is a dot.
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::AddLine(float64 x0, float64 y0, float64 x1, float64 y1, float64 fThickness, ARGB32 sColour, EGeometryLineCap eCap)
{
	SGeometryCommand*	psCommand;
	float64				fExtent;

	if (fThickness <= 0)
	{
		return;
	}
	if ((eCap == GLC_Butt) && (x0 == x1) && (y0 == y1))
	{
		return;
	}

	//A square cap's corner can be half the thickness out along both axes.
	fExtent = fThickness / 2;
	if (eCap == GLC_Square)
	{
		fExtent *= sqrt(2.0);
	}

	psCommand = AddShape(GC_Line,
						 ((x0 < x1) ? x0 : x1) - fExtent, ((y0 < y1) ? y0 : y1) - fExtent,
						 ((x0 > x1) ? x0 : x1) + fExtent, ((y0 > y1) ? y0 : y1) + fExtent,
						 sColour);
	psCommand->sStart.Init(x0, y0);
	psCommand->sEnd.Init(x1, y1);
	psCommand->fThickness = fThickness;
	psCommand->eCap = eCap;
}


//////////////////////////////////////////////////////////////////////////
//
// The outline of a circle, fThickness pixels thick centred on the radius.
// An outline thicker than the circle is wide fills it.
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::AddCircle(float64 x, float64 y, float64 fRadius, float64 fThickness, ARGB32 sColour)
{
	SGeometryCommand*	psCommand;
	float64				fOuter;

	if ((fThickness <= 0) || (fRadius < 0))
	{
		return;
	}

	fOuter = fRadius + fThickness / 2;
	psCommand = AddShape(GC_Circle, x - fOuter, y - fOuter, x + fOuter, y + fOuter, sColour);
	psCommand->sStart.Init(x, y);
	psCommand->fRadius = fRadius;
	psCommand->fThickness = fThickness;
}


//////////////////////////////////////////////////////////////////////////
//
// A filled circle, such as the junction where traces meet.
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::AddFilledCircle(float64 x, float64 y, float64 fRadius, ARGB32 sColour)
{
	SGeometryCommand*	psCommand;

	if (fRadius <= 0)
	{
		return;
	}

	psCommand = AddShape(GC_FilledCircle, x - fRadius, y - fRadius, x + fRadius, y + fRadius, sColour);
	psCommand->sStart.Init(x, y);
	psCommand->fRadius = fRadius;
}


//////////////////////////////////////////////////////////////////////////
//
// A shape reaching from fLeft, fTop to fRight, fBottom in pixel centre
// points.  The bounds hold the pixels it touches.
//
//////////////////////////////////////////////////////////////////////////
SGeometryCommand* CGeometryMap::AddShape(EGeometryCommand eType, float64 fLeft, float64 fTop, float64 fRight, float64 fBottom, ARGB32 sColour)
{
	SGeometryCommand*	psCommand;

	psCommand = masCommands.Add();
	psCommand->eType = eType;
	psCommand->cBounds.Init((int32)floor(fLeft + 0.5), (int32)floor(fTop + 0.5), (int32)ceil(fRight + 0.5), (int32)ceil(fBottom + 0.5));
	psCommand->sColour = sColour;
	psCommand->sStart.Init(0, 0);
	psCommand->sEnd.Init(0, 0);
	psCommand->fRadius = 0;
	psCommand->fThickness = 0;
	psCommand->eCap = GLC_Butt;
	return psCommand;
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the part of each shape inside pcViewportRect (in map pixels, right
// and bottom exclusive) onto the viewport, the viewport rectangle's top
// left at the viewport's top left.
//
//////////////////////////////////////////////////////////////////////////
bool CGeometryMap::Blit(CInt32Rect* pcViewportRect, bool bEnsureBlitters)
{
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;
	CVectorRasteriser		cRasteriser;
	bool					bResult;
	bool					bOpacity;
	size					i;
	SGeometryCommand*		psCommand;
	int32					iLeft;
	int32					iTop;
	int32					iRight;
	int32					iBottom;
	int32					iViewportRight;
	int32					iViewportBottom;

	if (mpViewport.IsNull() || (masCommands.NumElements() == 0))
	{
		return true;
	}

	//The viewport image clips too.
	iViewportRight = pcViewportRect->msTopLeft.x + (int32)mpViewport->GetWidth();
	if (pcViewportRect->msBottomRight.x < iViewportRight)
	{
		iViewportRight = pcViewportRect->msBottomRight.x;
	}
	iViewportBottom = pcViewportRect->msTopLeft.y + (int32)mpViewport->GetHeight();
	if (pcViewportRect->msBottomRight.y < iViewportBottom)
	{
		iViewportBottom = pcViewportRect->msBottomRight.y;
	}

	bOpacity = mpViewport->HasChannel(IMAGE_OPACITY);
	cCreator.Init(&mpViewport);
	if (bOpacity)
	{
		cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, PT_uint8);
	}
	else
	{
		cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, PT_uint8);
	}
	pcAccessor = cCreator.CreateAndKill();
	cRasteriser.Init();

	bResult = true;
	for (i = 0; i < masCommands.NumElements(); i++)
	{
		psCommand = masCommands.Get(i);
		iLeft = (psCommand->cBounds.msTopLeft.x > pcViewportRect->msTopLeft.x) ? psCommand->cBounds.msTopLeft.x : pcViewportRect->msTopLeft.x;
		iTop = (psCommand->cBounds.msTopLeft.y > pcViewportRect->msTopLeft.y) ? psCommand->cBounds.msTopLeft.y : pcViewportRect->msTopLeft.y;
		iRight = (psCommand->cBounds.msBottomRight.x < iViewportRight) ? psCommand->cBounds.msBottomRight.x : iViewportRight;
		iBottom = (psCommand->cBounds.msBottomRight.y < iViewportBottom) ? psCommand->cBounds.msBottomRight.y : iViewportBottom;
		if ((iLeft < iRight) && (iTop < iBottom))
		{
			if (psCommand->eType == GC_Rectangle)
			{
				FillRectangle(pcAccessor, bOpacity,
							  iLeft - pcViewportRect->msTopLeft.x, iTop - pcViewportRect->msTopLeft.y,
							  iRight - pcViewportRect->msTopLeft.x, iBottom - pcViewportRect->msTopLeft.y,
							  psCommand->sColour);
			}
			else
			{
				bResult &= FillShape(pcAccessor, &cRasteriser, bOpacity, psCommand, pcViewportRect,
									 iViewportRight - pcViewportRect->msTopLeft.x, iViewportBottom - pcViewportRect->msTopLeft.y);
			}
		}
	}

	cRasteriser.Kill();
	pcAccessor->Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Rasterises a line or circle and blends its colour by the coverage onto
// the viewport, clipped to iClipWidth by iClipHeight viewport pixels.
//
//////////////////////////////////////////////////////////////////////////
bool CGeometryMap::FillShape(CImageAccessor* pcAccessor, CVectorRasteriser* pcRasteriser, bool bOpacity, SGeometryCommand* psCommand, CInt32Rect* pcViewportRect, int32 iClipWidth, int32 iClipHeight)
{
	CVectorPath		cPath;
	SFloat64Vec2	sOrigin;
	int				iLeft;
	int				iTop;
	int				iRight;
	int				iBottom;
	int				x;
	int				y;
	uint8*			puiCoverage;
	uint32			uiAlpha;
	uint32			uiCoverage;

	cPath.Init();
	AddShapePath(&cPath, psCommand);
	cPath.CalculateBounds();

	//Pixel centres are whole numbers so a pixel's edges are half a pixel either side.
	sOrigin.Init(pcViewportRect->msTopLeft.x - 0.5, pcViewportRect->msTopLeft.y - 0.5);
	pcRasteriser->SetTransform(&sOrigin, 1, false);
	if (!pcRasteriser->GetPixelBounds(&cPath, &iLeft, &iTop, &iRight, &iBottom))
	{
		cPath.Kill();
		return true;
	}
	iLeft = (iLeft < 0) ? 0 : iLeft;
	iTop = (iTop < 0) ? 0 : iTop;
	iRight = (iRight > iClipWidth) ? iClipWidth : iRight;
	iBottom = (iBottom > iClipHeight) ? iClipHeight : iBottom;
	if ((iRight <= iLeft) || (iBottom <= iTop))
	{
		cPath.Kill();
		return true;
	}

	if (!pcRasteriser->Rasterise(&cPath, iLeft, iTop, iRight - iLeft, iBottom - iTop))
	{
		cPath.Kill();
		return gcLogger.Error2(__METHOD__, " Could not rasterise shape.", NULL);
	}
	cPath.Kill();

	uiAlpha = Get8BitAlphaColour(psCommand->sColour);
	puiCoverage = pcRasteriser->GetCoverage();
	for (y = 0; y < pcRasteriser->GetHeight(); y++)
	{
		for (x = 0; x < pcRasteriser->GetWidth(); x++)
		{
			uiCoverage = puiCoverage[(size)y * pcRasteriser->GetWidth() + x];
			if (uiCoverage > 0)
			{
				BlendPixel(pcAccessor, bOpacity, pcRasteriser->GetLeft() + x, pcRasteriser->GetTop() + y, psCommand->sColour, (uint32)DivideRound(uiAlpha * uiCoverage, 255));
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The outline of a line or circle in map pixels, y down.
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::AddShapePath(CVectorPath* pcPath, SGeometryCommand* psCommand)
{
	SFloat64Vec2	asCorners[4];
	SFloat64Vec2	sDirection;
	SFloat64Vec2	sNormal;
	SFloat64Vec2	sStart;
	SFloat64Vec2	sEnd;
	float64			fHalf;
	float64			fLength;

	fHalf = psCommand->fThickness / 2;
	if (psCommand->eType == GC_FilledCircle)
	{
		pcPath->AddCircle(&psCommand->sStart, psCommand->fRadius, false);
	}
	else if (psCommand->eType == GC_Circle)
	{
		AddVectorRing(pcPath, &psCommand->sStart, psCommand->fRadius + fHalf, psCommand->fRadius - fHalf, 0, 360);
	}
	else if (psCommand->eType == GC_Line)
	{
		if (psCommand->eCap == GLC_Round)
		{
			AddVectorStroke(pcPath, &psCommand->sStart, &psCommand->sEnd, fHalf);
			return;
		}

		fLength = hypot(psCommand->sEnd.x - psCommand->sStart.x, psCommand->sEnd.y - psCommand->sStart.y);
		if (fLength == 0)
		{
			//Only a square cap is kept with no length; it's a square dot.
			sDirection.Init(1, 0);
		}
		else
		{
			sDirection.Init((psCommand->sEnd.x - psCommand->sStart.x) / fLength, (psCommand->sEnd.y - psCommand->sStart.y) / fLength);
		}
		sNormal.Init(-sDirection.y * fHalf, sDirection.x * fHalf);

		sStart = psCommand->sStart;
		sEnd = psCommand->sEnd;
		if (psCommand->eCap == GLC_Square)
		{
			sStart.Init(sStart.x - sDirection.x * fHalf, sStart.y - sDirection.y * fHalf);
			sEnd.Init(sEnd.x + sDirection.x * fHalf, sEnd.y + sDirection.y * fHalf);
		}
		asCorners[0].Init(sStart.x - sNormal.x, sStart.y - sNormal.y);
		asCorners[1].Init(sEnd.x - sNormal.x, sEnd.y - sNormal.y);
		asCorners[2].Init(sEnd.x + sNormal.x, sEnd.y + sNormal.y);
		asCorners[3].Init(sStart.x + sNormal.x, sStart.y + sNormal.y);
		pcPath->AddPolygon(asCorners, 4, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Blends the colour over each pixel as a translucent cel's pixel is
// blended (see BlendRGBByteAlphaByte and BlendAlphaByte): the colour by
// lerping with the alpha and the opacity, if there is one, by over.
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::FillRectangle(CImageAccessor* pcAccessor, bool bOpacity, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, ARGB32 sColour)
{
	uint32	uiAlpha;
	int32	x;
	int32	y;

	uiAlpha = Get8BitAlphaColour(sColour);
	for (y = iTop; y < iBottom; y++)
	{
		for (x = iLeft; x < iRight; x++)
		{
			BlendPixel(pcAccessor, bOpacity, x, y, sColour, uiAlpha);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Blends the colour over the pixel by uiAlpha rather than the colour's own
// alpha, so a partly covered pixel of a shape gets less of it.
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::BlendPixel(CImageAccessor* pcAccessor, bool bOpacity, int32 x, int32 y, ARGB32 sColour, uint32 uiAlpha)
{
	uint8	auiPixel[4];

	pcAccessor->Get(x, y, auiPixel);
	auiPixel[0] = BlendLerpByte(auiPixel[0], Get8BitRedColour(sColour), uiAlpha);
	auiPixel[1] = BlendLerpByte(auiPixel[1], Get8BitGreenColour(sColour), uiAlpha);
	auiPixel[2] = BlendLerpByte(auiPixel[2], Get8BitBlueColour(sColour), uiAlpha);
	if (bOpacity)
	{
		auiPixel[3] = BlendOverByte(auiPixel[3], uiAlpha);
	}
	pcAccessor->Set(x, y, auiPixel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::SetViewport(Ptr<CImage> pViewport)
{
	mpViewport = pViewport;
}


Ptr<CImage> CGeometryMap::GetViewport(void) { return mpViewport; }
size CGeometryMap::NumCommands(void) { return masCommands.NumElements(); }
SGeometryCommand* CGeometryMap::GetCommand(size uiIndex) { return masCommands.Get(uiIndex); }

