#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/FixedPoint.h"
#include "GerberRasteriser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberRaster::Init(int iWidth, int iHeight, SFloat64Vec2* psOrigin, float64 fPixelsPerMillimetre)
{
	miWidth = iWidth;
	miHeight = iHeight;
	msOrigin = *psOrigin;
	mfPixelsPerMillimetre = fPixelsPerMillimetre;
	mpuiData = NULL;
	if ((iWidth <= 0) || (iHeight <= 0))
	{
		miWidth = 0;
		miHeight = 0;
		return false;
	}
	mpuiData = (uint8*)calloc((size)iWidth * iHeight, 1);
	return mpuiData != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberRaster::Init(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, float64 fPixelsPerMillimetre)
{
	SFloat64Vec2	sOrigin;

	//Covers the area; the left and top edges are on the minimum x and maximum y.
	sOrigin.Init(psMinimum->x, psMaximum->y);
	return Init((int)ceil((psMaximum->x - psMinimum->x) * fPixelsPerMillimetre), (int)ceil((psMaximum->y - psMinimum->y) * fPixelsPerMillimetre), &sOrigin, fPixelsPerMillimetre);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRaster::Kill(void)
{
	free(mpuiData);
	mpuiData = NULL;
	miWidth = 0;
	miHeight = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRaster::Clear(void)
{
	if (mpuiData)
	{
		memset(mpuiData, 0, (size)miWidth * miHeight);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CGerberRaster::GetData(void)
{
	return mpuiData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGerberRaster::GetWidth(void)
{
	return miWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGerberRaster::GetHeight(void)
{
	return miHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CGerberRaster::Get(int x, int y)
{
	return mpuiData[(size)y * miWidth + x];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2* CGerberRaster::GetOrigin(void)
{
	return &msOrigin;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CGerberRaster::GetPixelsPerMillimetre(void)
{
	return mfPixelsPerMillimetre;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CGerberRaster::Area(void)
{
	size	i;
	size	uiCells;
	uint64	ulliSum;

	//The dark area in square millimetres.
	ulliSum = 0;
	uiCells = (size)miWidth * miHeight;
	for (i = 0; i < uiCells; i++)
	{
		ulliSum += mpuiData[i];
	}
	return (float64)ulliSum / 255.0 / (mfPixelsPerMillimetre * mfPixelsPerMillimetre);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRasteriser::Init(float64 fTolerance)
{
	mcCoverage.Init();
	mpuiObject = NULL;
	muiObjectCapacity = 0;
	mfTolerance = fTolerance;
	mpcRaster = NULL;
	miLeft = 0;
	miTop = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRasteriser::Kill(void)
{
	free(mpuiObject);
	mpuiObject = NULL;
	muiObjectCapacity = 0;
	mcCoverage.Kill();
	mpcRaster = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberRasteriser::Rasterise(CGerberVectorImage* pcImage, CGerberRaster* pcRaster)
{
	size	i;
	size	uiNumObjects;

	mpcRaster = pcRaster;
	uiNumObjects = pcImage->NumObjects();
	for (i = 0; i < uiNumObjects; i++)
	{
		if (!RasteriseObject(pcImage->GetObject(i)))
		{
			mpcRaster = NULL;
			return false;
		}
	}
	mpcRaster = NULL;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CGerberRasteriser::ToSubpixelX(float64 x)
{
	return (int32)llround(((x - mpcRaster->GetOrigin()->x) * mpcRaster->GetPixelsPerMillimetre() - miLeft) * COVERAGE_SUBPIXELS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CGerberRasteriser::ToSubpixelY(float64 y)
{
	//The raster is y down.
	return (int32)llround(((mpcRaster->GetOrigin()->y - y) * mpcRaster->GetPixelsPerMillimetre() - miTop) * COVERAGE_SUBPIXELS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRasteriser::AddLine(SFloat64Vec2* psStart, SFloat64Vec2* psEnd)
{
	mcCoverage.AddLine(ToSubpixelX(psStart->x), ToSubpixelY(psStart->y), ToSubpixelX(psEnd->x), ToSubpixelY(psEnd->y));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRasteriser::AddArc(SFloat64Vec2* psStart, SFloat64Vec2* psEnd, SFloat64Vec2* psCenter, bool bClockwise)
{
	float64			fRadius;
	float64			fPixelRadius;
	float64			fStart;
	float64			fSweep;
	float64			fStep;
	float64			fAngle;
	float64			fVertexRadius;
	int				iSegments;
	int				iMinimum;
	int				i;
	SFloat64Vec2	sLast;
	SFloat64Vec2	sPoint;

	fRadius = hypot(psStart->x - psCenter->x, psStart->y - psCenter->y);
	if (fRadius == 0)
	{
		AddLine(psStart, psEnd);
		return;
	}

	fStart = atan2(psStart->y - psCenter->y, psStart->x - psCenter->x);
	fSweep = GetGerberArcSweep(psStart, psEnd, psCenter, bClockwise);

	//The chord of an angle a is r * (1 - cos(a / 2)) from the arc.  At least eight segments to a circle.
	fPixelRadius = fRadius * mpcRaster->GetPixelsPerMillimetre();
	iMinimum = (int)ceil(fSweep / (GERBER_PI / 4));
	if (fPixelRadius > mfTolerance)
	{
		fStep = 2 * acos(1 - mfTolerance / fPixelRadius);
		iSegments = (int)ceil(fSweep / fStep);
	}
	else
	{
		iSegments = iMinimum;
	}
	if (iSegments < iMinimum)
	{
		iSegments = iMinimum;
	}

	//Chords inside the arc would lose area, so the vertices between the ends are moved out to the radius at which a
	//polygon of these chords has the area of the circle; r * sqrt(a / sin(a)).
	fStep = fSweep / iSegments;
	fVertexRadius = fRadius * sqrt(fStep / sin(fStep));

	sLast = *psStart;
	for (i = 1; i < iSegments; i++)
	{
		fAngle = fStart + (bClockwise ? -fSweep : fSweep) * i / iSegments;
		sPoint.Init(psCenter->x + fVertexRadius * cos(fAngle), psCenter->y + fVertexRadius * sin(fAngle));
		AddLine(&sLast, &sPoint);
		sLast = sPoint;
	}
	AddLine(&sLast, psEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberRasteriser::RasterisePart(CGerberVectorObject* pcObject, size uiStart, size uiEnd, int iWidth, int iHeight)
{
	size					i;
	SGerberVectorSegment*	psSegment;
	SFloat64Vec2			sCurrent;
	SFloat64Vec2			sContourStart;
	bool					bContour;

	if (!mcCoverage.Begin(iWidth, iHeight))
	{
		return false;
	}

	//Contours are closed back to their start.
	bContour = false;
	sCurrent.Init(0, 0);
	sContourStart.Init(0, 0);
	for (i = uiStart; i < uiEnd; i++)
	{
		psSegment = pcObject->GetSegment(i);
		switch (psSegment->eType)
		{
			case GVS_Move:
				if (bContour)
				{
					AddLine(&sCurrent, &sContourStart);
				}
				sContourStart = psSegment->sPoint;
				bContour = true;
				break;
			case GVS_Line:
				AddLine(&sCurrent, &psSegment->sPoint);
				break;
			case GVS_Arc:
				AddArc(&sCurrent, &psSegment->sPoint, &psSegment->sCenter, psSegment->bClockwise);
				break;
			default:
				break;
		}
		if (psSegment->eType != GVS_Part)
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
//
//////////////////////////////////////////////////////////////////////////
void CGerberRasteriser::Composite(uint8* puiCoverage, int iWidth, int iHeight, bool bDark)
{
	int		x;
	int		y;
	int32	iCoverage;
	int32	iDest;
	uint8*	puiDest;

	//Dark: d + c (1 - d).  Clear: d (1 - c).
	for (y = 0; y < iHeight; y++)
	{
		puiDest = mpcRaster->GetData() + (size)(miTop + y) * mpcRaster->GetWidth() + miLeft;
		for (x = 0; x < iWidth; x++)
		{
			iCoverage = puiCoverage[(size)y * iWidth + x];
			if (iCoverage == 0)
			{
				continue;
			}
			iDest = puiDest[x];
			if (bDark)
			{
				puiDest[x] = (uint8)(iDest + DivideRound((int64)iCoverage * (255 - iDest), 255));
			}
			else
			{
				puiDest[x] = (uint8)DivideRound((int64)iDest * (255 - iCoverage), 255);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberRasteriser::RasteriseObject(CGerberVectorObject* pcObject)
{
	float64			fPixels;
	int				iRight;
	int				iBottom;
	int				iWidth;
	int				iHeight;
	size			i;
	size			uiNumSegments;
	size			uiPartStart;
	size			uiNumParts;
	size			uiCells;
	size			uiCell;
	bool			bExposure;
	uint8*			puiPart;
	int32			iPart;
	int32			iObject;

	//Objects without an image (zero width draws) and objects outside the raster draw nothing.
	if (!pcObject->HasBounds())
	{
		return true;
	}
	fPixels = mpcRaster->GetPixelsPerMillimetre();
	miLeft = (int)floor((pcObject->GetMinimum()->x - mpcRaster->GetOrigin()->x) * fPixels);
	iRight = (int)ceil((pcObject->GetMaximum()->x - mpcRaster->GetOrigin()->x) * fPixels);
	miTop = (int)floor((mpcRaster->GetOrigin()->y - pcObject->GetMaximum()->y) * fPixels);
	iBottom = (int)ceil((mpcRaster->GetOrigin()->y - pcObject->GetMinimum()->y) * fPixels);
	if (miLeft < 0)
	{
		miLeft = 0;
	}
	if (miTop < 0)
	{
		miTop = 0;
	}
	if (iRight > mpcRaster->GetWidth())
	{
		iRight = mpcRaster->GetWidth();
	}
	if (iBottom > mpcRaster->GetHeight())
	{
		iBottom = mpcRaster->GetHeight();
	}
	iWidth = iRight - miLeft;
	iHeight = iBottom - miTop;
	if ((iWidth <= 0) || (iHeight <= 0))
	{
		return true;
	}

	uiNumSegments = pcObject->NumSegments();
	uiNumParts = 0;
	for (i = 0; i < uiNumSegments; i++)
	{
		if (pcObject->GetSegment(i)->eType == GVS_Part)
		{
			uiNumParts++;
		}
	}

	if (uiNumParts <= 1)
	{
		if (!RasterisePart(pcObject, 0, uiNumSegments, iWidth, iHeight))
		{
			return false;
		}
		if ((uiNumParts == 0) || pcObject->GetSegment(0)->bExposure)
		{
			Composite(mcCoverage.GetCoverage(), iWidth, iHeight, pcObject->IsDark());
		}
		return true;
	}

	//Several parts may overlap and wind differently so each is filled on its own and combined: on adds, off erases.
	uiCells = (size)iWidth * iHeight;
	if (uiCells > muiObjectCapacity)
	{
		free(mpuiObject);
		mpuiObject = (uint8*)malloc(uiCells);
		if (mpuiObject == NULL)
		{
			muiObjectCapacity = 0;
			return false;
		}
		muiObjectCapacity = uiCells;
	}
	memset(mpuiObject, 0, uiCells);

	uiPartStart = 0;
	bExposure = true;
	for (i = 0; i <= uiNumSegments; i++)
	{
		if ((i < uiNumSegments) && (pcObject->GetSegment(i)->eType != GVS_Part))
		{
			continue;
		}
		if (i > uiPartStart)
		{
			if (!RasterisePart(pcObject, uiPartStart, i, iWidth, iHeight))
			{
				return false;
			}
			puiPart = mcCoverage.GetCoverage();
			for (uiCell = 0; uiCell < uiCells; uiCell++)
			{
				iPart = puiPart[uiCell];
				iObject = mpuiObject[uiCell];
				if (bExposure)
				{
					mpuiObject[uiCell] = (uint8)(iObject + DivideRound((int64)iPart * (255 - iObject), 255));
				}
				else
				{
					mpuiObject[uiCell] = (uint8)DivideRound((int64)iObject * (255 - iPart), 255);
				}
			}
		}
		if (i < uiNumSegments)
		{
			bExposure = pcObject->GetSegment(i)->bExposure;
			uiPartStart = i;
		}
	}

	Composite(mpuiObject, iWidth, iHeight, pcObject->IsDark());
	return true;
}

