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
#include <stdlib.h>
#include <string.h>
#include "ColourBlend.h"
#include "VectorRasteriser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorRasteriser::Init(float64 fTolerance)
{
	mcCoverage.Init();
	mpuiPath = NULL;
	muiPathCapacity = 0;
	mpuiResult = NULL;
	mfTolerance = fTolerance;
	msOrigin.Init(0, 0);
	mfScale = 1;
	mbYUp = false;
	miLeft = 0;
	miTop = 0;
	miWidth = 0;
	miHeight = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorRasteriser::Kill(void)
{
	free(mpuiPath);
	mpuiPath = NULL;
	muiPathCapacity = 0;
	mpuiResult = NULL;
	mcCoverage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorRasteriser::SetTransform(SFloat64Vec2* psOrigin, float64 fPixelsPerUnit, bool bYUp)
{
	msOrigin = *psOrigin;
	mfScale = fPixelsPerUnit;
	mbYUp = bYUp;
}


//////////////////////////////////////////////////////////////////////////
//
// The pixels the path's bounds touch; right and bottom are past the last
// pixel.  They are not clipped.  False if the path has no bounds.
//
//////////////////////////////////////////////////////////////////////////
bool CVectorRasteriser::GetPixelBounds(CVectorPath* pcPath, int* piLeft, int* piTop, int* piRight, int* piBottom)
{
	float64	fTop;
	float64	fBottom;

	if (!pcPath->HasBounds())
	{
		return false;
	}
	fTop = ToPixelY(pcPath->GetMaximum()->y);
	fBottom = ToPixelY(pcPath->GetMinimum()->y);
	if (!mbYUp)
	{
		fTop = ToPixelY(pcPath->GetMinimum()->y);
		fBottom = ToPixelY(pcPath->GetMaximum()->y);
	}
	*piLeft = (int)floor(ToPixelX(pcPath->GetMinimum()->x));
	*piRight = (int)ceil(ToPixelX(pcPath->GetMaximum()->x));
	*piTop = (int)floor(fTop);
	*piBottom = (int)ceil(fBottom);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CVectorRasteriser::ToPixelX(float64 x)
{
	return (x - msOrigin.x) * mfScale;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CVectorRasteriser::ToPixelY(float64 y)
{
	if (mbYUp)
	{
		return (msOrigin.y - y) * mfScale;
	}
	return (y - msOrigin.y) * mfScale;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CVectorRasteriser::ToSubpixel(float64 fPixel, int iCorner)
{
	return (int32)llround((fPixel - iCorner) * COVERAGE_SUBPIXELS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorRasteriser::AddLine(SFloat64Vec2* psStart, SFloat64Vec2* psEnd)
{
	mcCoverage.AddLine(ToSubpixel(ToPixelX(psStart->x), miLeft), ToSubpixel(ToPixelY(psStart->y), miTop), ToSubpixel(ToPixelX(psEnd->x), miLeft), ToSubpixel(ToPixelY(psEnd->y), miTop));
}


//////////////////////////////////////////////////////////////////////////
//
// The number of steps to a whole circle of the radius in pixels; the
// chord of an angle a is r * (1 - cos(a / 2)) from the arc.  A multiple of
// eight so the grid is symmetric about the axes and the diagonals.
//
//////////////////////////////////////////////////////////////////////////
int32 CVectorRasteriser::ArcSteps(float64 fPixelRadius)
{
	float64	fSteps;
	int32	iSteps;

	if (fPixelRadius <= mfTolerance)
	{
		return 8;
	}
	fSteps = 2 * N_PI_64 / (2 * acos(1 - mfTolerance / fPixelRadius));
	if (fSteps >= VECTOR_ARC_MAX_STEPS)
	{
		return VECTOR_ARC_MAX_STEPS;
	}
	iSteps = ((int32)ceil(fSteps) + 7) / 8 * 8;
	if (iSteps < 8)
	{
		iSteps = 8;
	}
	return iSteps;
}


//////////////////////////////////////////////////////////////////////////
//
// The cosine and sine of grid angle iIndex of iSteps to a circle.  Only
// the first eighth is worked out; the rest are its mirror images, so the
// grid is exactly symmetric.
//
//////////////////////////////////////////////////////////////////////////
void CVectorRasteriser::GridCosSin(int64 iIndex, int32 iSteps, float64* pfCos, float64* pfSin)
{
	int64	iEighth;
	int64	iOctant;
	int64	iStep;
	float64	fCos;
	float64	fSin;

	iEighth = iSteps / 8;
	iIndex = ((iIndex % iSteps) + iSteps) % iSteps;
	iOctant = iIndex / iEighth;
	iStep = iIndex % iEighth;
	if (iOctant & 1)
	{
		iStep = iEighth - iStep;
	}

	if (iStep == 0)
	{
		fCos = 1;
		fSin = 0;
	}
	else if (iStep == iEighth)
	{
		fCos = sqrt(0.5);
		fSin = fCos;
	}
	else
	{
		fCos = cos(2 * N_PI_64 * (float64)iStep / iSteps);
		fSin = sin(2 * N_PI_64 * (float64)iStep / iSteps);
	}

	switch (iOctant)
	{
		case 0: *pfCos = fCos;		*pfSin = fSin;		break;
		case 1: *pfCos = fSin;		*pfSin = fCos;		break;
		case 2: *pfCos = -fSin;		*pfSin = fCos;		break;
		case 3: *pfCos = -fCos;		*pfSin = fSin;		break;
		case 4: *pfCos = -fCos;		*pfSin = -fSin;		break;
		case 5: *pfCos = -fSin;		*pfSin = -fCos;		break;
		case 6: *pfCos = fSin;		*pfSin = -fCos;		break;
		default: *pfCos = fCos;		*pfSin = -fSin;		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The ends are exact and the vertices between them are the grid angles
// between the ends.  An end on a grid angle (to within a millionth of a
// step) has that angle's vertex too, joined to it along the radius, so an
// arc ending there covers the same pixels as a longer arc through it and
// no sliver step is made next to it.  Chords inside
// the arc would lose area, so the vertices are moved out to the radius at
// which a polygon of whole steps has the area of the circle;
// r * sqrt(a / sin(a)).
//
//////////////////////////////////////////////////////////////////////////
void CVectorRasteriser::AddArc(SFloat64Vec2* psStart, SFloat64Vec2* psEnd, SFloat64Vec2* psCenter, bool bClockwise)
{
	float64			fRadius;
	float64			fSweep;
	float64			fStep;
	float64			fVertexRadius;
	float64			fStart;
	float64			fEnd;
	float64			fCenterX;
	float64			fCenterY;
	float64			fCos;
	float64			fSin;
	int32			iSteps;
	int64			iFirst;
	int64			iLast;
	int64			i;
	SFloat64Vec2	sLast;
	SFloat64Vec2	sPoint;

	fRadius = hypot(psStart->x - psCenter->x, psStart->y - psCenter->y);
	if (fRadius == 0)
	{
		AddLine(psStart, psEnd);
		return;
	}

	iSteps = ArcSteps(fRadius * fabs(mfScale));
	fStep = 2 * N_PI_64 / iSteps;
	fVertexRadius = fRadius * sqrt(fStep / sin(fStep));

	//The ends in steps around the grid.
	fSweep = Float64Vec2ArcSweep(psStart, psEnd, psCenter, bClockwise);
	fStart = atan2(psStart->y - psCenter->y, psStart->x - psCenter->x) / fStep;
	fEnd = fStart + (bClockwise ? -fSweep : fSweep) / fStep;
	if (!bClockwise)
	{
		iFirst = (int64)floor(fStart) + 1;
		iLast = (int64)ceil(fEnd) - 1;
	}
	else
	{
		iFirst = (int64)ceil(fStart) - 1;
		iLast = (int64)floor(fEnd) + 1;
	}
	if (fabs(fStart - llround(fStart)) < 1e-6)
	{
		iFirst = llround(fStart);
	}
	if (fabs(fEnd - llround(fEnd)) < 1e-6)
	{
		iLast = llround(fEnd);
	}

	fCenterX = psCenter->x;
	fCenterY = psCenter->y;
	sLast = *psStart;
	i = iFirst;
	for (;;)
	{
		if ((!bClockwise && (i > iLast)) || (bClockwise && (i < iLast)))
		{
			break;
		}
		GridCosSin(i, iSteps, &fCos, &fSin);
		sPoint.Init(fCenterX + fVertexRadius * fCos, fCenterY + fVertexRadius * fSin);
		AddLine(&sLast, &sPoint);
		sLast = sPoint;
		i += bClockwise ? -1 : 1;
	}
	AddLine(&sLast, psEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVectorRasteriser::RasterisePart(CVectorPath* pcPath, size uiStart, size uiEnd)
{
	size				i;
	SVectorSegment*		psSegment;
	SFloat64Vec2		sCurrent;
	SFloat64Vec2		sContourStart;
	bool				bContour;

	if (!mcCoverage.Begin(miWidth, miHeight))
	{
		return false;
	}

	//Contours are closed back to their start.
	bContour = false;
	sCurrent.Init(0, 0);
	sContourStart.Init(0, 0);
	for (i = uiStart; i < uiEnd; i++)
	{
		psSegment = pcPath->GetSegment(i);
		switch (psSegment->eType)
		{
			case VS_Move:
				if (bContour)
				{
					AddLine(&sCurrent, &sContourStart);
				}
				sContourStart = psSegment->sPoint;
				bContour = true;
				break;
			case VS_Line:
				AddLine(&sCurrent, &psSegment->sPoint);
				break;
			case VS_Arc:
				AddArc(&sCurrent, &psSegment->sPoint, &psSegment->sCenter, psSegment->bClockwise);
				break;
			default:
				break;
		}
		if (psSegment->eType != VS_Part)
		{
			sCurrent = psSegment->sPoint;
		}
	}
	if (bContour)
	{
		AddLine(&sCurrent, &sContourStart);
	}

	mcCoverage.Accumulate();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Fills the path into a grid iWidth by iHeight pixels with its top left
// at pixel iLeft, iTop.  The coverage is kept until the next Rasterise.
//
//////////////////////////////////////////////////////////////////////////
bool CVectorRasteriser::Rasterise(CVectorPath* pcPath, int iLeft, int iTop, int iWidth, int iHeight)
{
	size		i;
	size		uiNumSegments;
	size		uiPartStart;
	size		uiNumParts;
	lsize		uiCells;
	lsize		uiCell;
	bool		bExposure;
	uint8*		puiPart;
	int32		iPart;
	int32		iPath;

	miLeft = iLeft;
	miTop = iTop;
	miWidth = iWidth;
	miHeight = iHeight;
	mpuiResult = NULL;
	if ((iWidth <= 0) || (iHeight <= 0))
	{
		miWidth = 0;
		miHeight = 0;
		return mcCoverage.Begin(0, 0);
	}

	uiNumSegments = pcPath->NumSegments();
	uiNumParts = 0;
	for (i = 0; i < uiNumSegments; i++)
	{
		if (pcPath->GetSegment(i)->eType == VS_Part)
		{
			uiNumParts++;
		}
	}

	if (uiNumParts <= 1)
	{
		if (!RasterisePart(pcPath, 0, uiNumSegments))
		{
			return false;
		}
		mpuiResult = mcCoverage.GetCoverage();
		if ((uiNumParts == 1) && !pcPath->GetSegment(0)->bExposure)
		{
			memset(mpuiResult, 0, (lsize)iWidth * iHeight);
		}
		return true;
	}

	//Several parts may overlap and wind differently so each is filled on its own and combined: on adds, off erases.
	uiCells = (lsize)iWidth * iHeight;
	if (uiCells > muiPathCapacity)
	{
		free(mpuiPath);
		mpuiPath = (uint8*)malloc(uiCells);
		if (mpuiPath == NULL)
		{
			muiPathCapacity = 0;
			return false;
		}
		muiPathCapacity = uiCells;
	}
	memset(mpuiPath, 0, uiCells);

	uiPartStart = 0;
	bExposure = true;
	for (i = 0; i <= uiNumSegments; i++)
	{
		if ((i < uiNumSegments) && (pcPath->GetSegment(i)->eType != VS_Part))
		{
			continue;
		}
		if (i > uiPartStart)
		{
			if (!RasterisePart(pcPath, uiPartStart, i))
			{
				return false;
			}
			puiPart = mcCoverage.GetCoverage();
			for (uiCell = 0; uiCell < uiCells; uiCell++)
			{
				iPart = puiPart[uiCell];
				iPath = mpuiPath[uiCell];
				mpuiPath[uiCell] = bExposure ? BlendOverByte(iPath, iPart) : BlendEraseByte(iPath, iPart);
			}
		}
		if (i < uiNumSegments)
		{
			bExposure = pcPath->GetSegment(i)->bExposure;
			uiPartStart = i;
		}
	}

	mpuiResult = mpuiPath;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The last path's coverage, GetWidth by GetHeight, one byte per pixel
// from the top left.
//
//////////////////////////////////////////////////////////////////////////
uint8* CVectorRasteriser::GetCoverage(void)
{
	return mpuiResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CVectorRasteriser::GetLeft(void)
{
	return miLeft;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CVectorRasteriser::GetTop(void)
{
	return miTop;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CVectorRasteriser::GetWidth(void)
{
	return miWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CVectorRasteriser::GetHeight(void)
{
	return miHeight;
}

