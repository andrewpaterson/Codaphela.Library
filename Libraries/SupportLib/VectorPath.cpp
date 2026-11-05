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
#include "VectorPath.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::Init(void)
{
	masSegments.Init();
	msMinimum.Init(0, 0);
	msMaximum.Init(0, 0);
	mbBounds = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::Init(CVectorPath* pcSource)
{
	size	i;
	size	uiNumSegments;

	Init();
	uiNumSegments = pcSource->masSegments.NumElements();
	for (i = 0; i < uiNumSegments; i++)
	{
		*masSegments.Add() = *pcSource->masSegments.Get(i);
	}
	msMinimum = pcSource->msMinimum;
	msMaximum = pcSource->msMaximum;
	mbBounds = pcSource->mbBounds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::Kill(void)
{
	masSegments.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::BeginPart(bool bExposure)
{
	SVectorSegment*	psSegment;

	psSegment = masSegments.Add();
	psSegment->eType = VS_Part;
	psSegment->bExposure = bExposure;
	psSegment->bClockwise = false;
	psSegment->sPoint.Init(0, 0);
	psSegment->sCenter.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::MoveTo(SFloat64Vec2* psPoint)
{
	SVectorSegment*	psSegment;

	psSegment = masSegments.Add();
	psSegment->eType = VS_Move;
	psSegment->bExposure = true;
	psSegment->bClockwise = false;
	psSegment->sPoint = *psPoint;
	psSegment->sCenter.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::LineTo(SFloat64Vec2* psPoint)
{
	SVectorSegment*	psSegment;

	psSegment = masSegments.Add();
	psSegment->eType = VS_Line;
	psSegment->bExposure = true;
	psSegment->bClockwise = false;
	psSegment->sPoint = *psPoint;
	psSegment->sCenter.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A cubic Bezier curve from the last point to psPoint pulled towards
// psControl1 and psControl2, added as straight edges no further than
// fTolerance from the curve.  How many edges is Wang's formula: the
// square root of 3/4 of the larger second difference of the control
// points over the tolerance.  With no last point it is a move to psPoint.
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::CubicTo(SFloat64Vec2* psControl1, SFloat64Vec2* psControl2, SFloat64Vec2* psPoint, float64 fTolerance)
{
	SFloat64Vec2	sStart;
	SFloat64Vec2	sPoint;
	float64			fX;
	float64			fY;
	float64			fDifference1;
	float64			fDifference2;
	float64			fT;
	float64			fU;
	size			uiLines;
	size			i;

	if (masSegments.NumElements() == 0)
	{
		MoveTo(psPoint);
		return;
	}
	sStart = masSegments.Get(masSegments.NumElements() - 1)->sPoint;

	fX = sStart.x - 2 * psControl1->x + psControl2->x;
	fY = sStart.y - 2 * psControl1->y + psControl2->y;
	fDifference1 = sqrt(fX * fX + fY * fY);
	fX = psControl1->x - 2 * psControl2->x + psPoint->x;
	fY = psControl1->y - 2 * psControl2->y + psPoint->y;
	fDifference2 = sqrt(fX * fX + fY * fY);
	uiLines = (size)ceil(sqrt(0.75 * ((fDifference1 > fDifference2) ? fDifference1 : fDifference2) / fTolerance));
	if (uiLines < 1)
	{
		uiLines = 1;
	}
	else if (uiLines > VECTOR_PATH_MAX_CUBIC_LINES)
	{
		uiLines = VECTOR_PATH_MAX_CUBIC_LINES;
	}

	for (i = 1; i < uiLines; i++)
	{
		fT = (float64)i / (float64)uiLines;
		fU = 1 - fT;
		sPoint.x = fU * fU * fU * sStart.x + 3 * fU * fU * fT * psControl1->x + 3 * fU * fT * fT * psControl2->x + fT * fT * fT * psPoint->x;
		sPoint.y = fU * fU * fU * sStart.y + 3 * fU * fU * fT * psControl1->y + 3 * fU * fT * fT * psControl2->y + fT * fT * fT * psPoint->y;
		LineTo(&sPoint);
	}
	LineTo(psPoint);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::ArcTo(SFloat64Vec2* psPoint, SFloat64Vec2* psCenter, bool bClockwise)
{
	SVectorSegment*	psSegment;

	psSegment = masSegments.Add();
	psSegment->eType = VS_Arc;
	psSegment->bExposure = true;
	psSegment->bClockwise = bClockwise;
	psSegment->sPoint = *psPoint;
	psSegment->sCenter = *psCenter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::AddCircle(SFloat64Vec2* psCenter, float64 fRadius, bool bClockwise)
{
	SFloat64Vec2	sStart;

	sStart.Init(psCenter->x + fRadius, psCenter->y);
	MoveTo(&sStart);
	ArcTo(&sStart, psCenter, bClockwise);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::AddPolygon(SFloat64Vec2* asPoints, size uiNumPoints, bool bReverse)
{
	size	i;

	if (uiNumPoints == 0)
	{
		return;
	}
	if (!bReverse)
	{
		MoveTo(&asPoints[0]);
		for (i = 1; i < uiNumPoints; i++)
		{
			LineTo(&asPoints[i]);
		}
		LineTo(&asPoints[0]);
	}
	else
	{
		MoveTo(&asPoints[uiNumPoints - 1]);
		for (i = uiNumPoints - 1; i > 0; i--)
		{
			LineTo(&asPoints[i - 1]);
		}
		LineTo(&asPoints[uiNumPoints - 1]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::Transform(bool bMirrorX, bool bMirrorY, float64 fRotation, float64 fScale, SFloat64Vec2* psTranslation)
{
	size					i;
	size					uiNumSegments;
	SVectorSegment*	psSegment;
	bool					bFlip;

	//Mirroring in one axis turns clockwise into counterclockwise.
	bFlip = bMirrorX != bMirrorY;
	uiNumSegments = masSegments.NumElements();
	for (i = 0; i < uiNumSegments; i++)
	{
		psSegment = masSegments.Get(i);
		if (psSegment->eType != VS_Part)
		{
			psSegment->sPoint.Transform(bMirrorX, bMirrorY, fRotation, fScale, psTranslation);
		}
		if (psSegment->eType == VS_Arc)
		{
			psSegment->sCenter.Transform(bMirrorX, bMirrorY, fRotation, fScale, psTranslation);
			if (bFlip)
			{
				psSegment->bClockwise = !psSegment->bClockwise;
			}
		}
	}
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IncludeVectorPoint(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool* pbBounds, float64 x, float64 y)
{
	SFloat64Vec2	sPoint;

	sPoint.Init(x, y);
	if (!*pbBounds)
	{
		*psMinimum = sPoint;
		*psMaximum = sPoint;
		*pbBounds = true;
		return;
	}
	Float64Vec2Minimize(psMinimum, psMinimum, &sPoint);
	Float64Vec2Maximize(psMaximum, psMaximum, &sPoint);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVectorPath::CalculateBounds(void)
{
	size					i;
	size					uiNumSegments;
	SVectorSegment*	psSegment;
	SFloat64Vec2			sCurrent;
	float64					fRadius;
	float64					fStart;
	float64					fSweep;
	float64					fAngle;
	uint					iQuarter;

	mbBounds = false;
	sCurrent.Init(0, 0);
	uiNumSegments = masSegments.NumElements();
	for (i = 0; i < uiNumSegments; i++)
	{
		psSegment = masSegments.Get(i);
		if (psSegment->eType == VS_Part)
		{
			continue;
		}

		IncludeVectorPoint(&msMinimum, &msMaximum, &mbBounds, psSegment->sPoint.x, psSegment->sPoint.y);
		if (psSegment->eType == VS_Arc)
		{
			//The axis extremes the arc passes through.
			fRadius = hypot(sCurrent.x - psSegment->sCenter.x, sCurrent.y - psSegment->sCenter.y);
			fStart = atan2(sCurrent.y - psSegment->sCenter.y, sCurrent.x - psSegment->sCenter.x);
			fSweep = Float64Vec2ArcSweep(&sCurrent, &psSegment->sPoint, &psSegment->sCenter, psSegment->bClockwise);
			for (iQuarter = 0; iQuarter < 4; iQuarter++)
			{
				fAngle = iQuarter * N_PI_64 / 2 - fStart;
				if (psSegment->bClockwise)
				{
					fAngle = -fAngle;
				}
				while (fAngle < 0)
				{
					fAngle += 2 * N_PI_64;
				}
				while (fAngle >= 2 * N_PI_64)
				{
					fAngle -= 2 * N_PI_64;
				}
				if (fAngle <= fSweep)
				{
					IncludeVectorPoint(&msMinimum, &msMaximum, &mbBounds,
						psSegment->sCenter.x + fRadius * cos(iQuarter * N_PI_64 / 2),
						psSegment->sCenter.y + fRadius * sin(iQuarter * N_PI_64 / 2));
				}
			}
		}
		sCurrent = psSegment->sPoint;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CVectorPath::NumSegments(void)
{
	return masSegments.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SVectorSegment* CVectorPath::GetSegment(size uiIndex)
{
	return masSegments.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVectorPath::HasBounds(void)
{
	return mbBounds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2* CVectorPath::GetMinimum(void)
{
	return &msMinimum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2* CVectorPath::GetMaximum(void)
{
	return &msMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVectorPath::HasExposureOff(void)
{
	size	i;
	size	uiNumSegments;

	uiNumSegments = masSegments.NumElements();
	for (i = 0; i < uiNumSegments; i++)
	{
		if ((masSegments.Get(i)->eType == VS_Part) && !masSegments.Get(i)->bExposure)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CVectorPath::Area(void)
{
	size					i;
	size					uiNumSegments;
	SVectorSegment*	psSegment;
	SFloat64Vec2			sCurrent;
	float64					fArea;
	float64					fRadius;
	float64					fSweep;
	float64					fSegment;

	//The signed area of all the contours (shoelace), counterclockwise positive.  An arc adds the circular segment between
	//its chord and the arc.  Overlapping parts and exposure are not taken into account.
	fArea = 0;
	sCurrent.Init(0, 0);
	uiNumSegments = masSegments.NumElements();
	for (i = 0; i < uiNumSegments; i++)
	{
		psSegment = masSegments.Get(i);
		if ((psSegment->eType == VS_Line) || (psSegment->eType == VS_Arc))
		{
			fArea += (sCurrent.x * psSegment->sPoint.y - psSegment->sPoint.x * sCurrent.y) / 2;
		}
		if (psSegment->eType == VS_Arc)
		{
			fRadius = hypot(sCurrent.x - psSegment->sCenter.x, sCurrent.y - psSegment->sCenter.y);
			fSweep = Float64Vec2ArcSweep(&sCurrent, &psSegment->sPoint, &psSegment->sCenter, psSegment->bClockwise);
			fSegment = fRadius * fRadius * (fSweep - sin(fSweep)) / 2;
			fArea += psSegment->bClockwise ? -fSegment : fSegment;
		}
		if (psSegment->eType != VS_Part)
		{
			sCurrent = psSegment->sPoint;
		}
	}
	return fArea;
}

