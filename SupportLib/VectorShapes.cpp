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
#include "VectorShapes.h"
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void RotateVectorPoint(SFloat64Vec2* psPoint, float64 fDegrees)
{
	SFloat64Vec2	sOrigin;

	sOrigin.Init(0, 0);
	psPoint->Transform(false, false, fDegrees, 1, &sOrigin);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddVectorStroke(CVectorPath* pcPath, SFloat64Vec2* psStart, SFloat64Vec2* psEnd, float64 fRadius)
{
	SFloat64Vec2	sDirection;
	SFloat64Vec2	sNormal;
	SFloat64Vec2	sPoint;
	float64			fLength;

	//A draw with a circle is a stadium; a zero length draw is a circle.  Counterclockwise.
	if (fRadius <= 0)
	{
		return;
	}
	fLength = hypot(psEnd->x - psStart->x, psEnd->y - psStart->y);
	if (fLength == 0)
	{
		pcPath->AddCircle(psStart, fRadius, false);
		return;
	}

	sDirection.Init((psEnd->x - psStart->x) / fLength, (psEnd->y - psStart->y) / fLength);
	sNormal.Init(-sDirection.y * fRadius, sDirection.x * fRadius);

	sPoint.Init(psStart->x - sNormal.x, psStart->y - sNormal.y);
	pcPath->MoveTo(&sPoint);
	sPoint.Init(psEnd->x - sNormal.x, psEnd->y - sNormal.y);
	pcPath->LineTo(&sPoint);
	sPoint.Init(psEnd->x + sNormal.x, psEnd->y + sNormal.y);
	pcPath->ArcTo(&sPoint, psEnd, false);
	sPoint.Init(psStart->x + sNormal.x, psStart->y + sNormal.y);
	pcPath->LineTo(&sPoint);
	sPoint.Init(psStart->x - sNormal.x, psStart->y - sNormal.y);
	pcPath->ArcTo(&sPoint, psStart, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddVectorArcStroke(CVectorPath* pcPath, SFloat64Vec2* psStart, SFloat64Vec2* psEnd, SFloat64Vec2* psCenter, bool bClockwise, float64 fRadius)
{
	float64			fArcRadius;
	float64			fEndRadius;
	SFloat64Vec2	sStartRadial;
	SFloat64Vec2	sEndRadial;
	SFloat64Vec2	sOuterStart;
	SFloat64Vec2	sOuterEnd;
	SFloat64Vec2	sInnerStart;
	SFloat64Vec2	sInnerEnd;
	SFloat64Vec2	sEndCap;
	float64			fInner;

	if (fRadius <= 0)
	{
		return;
	}

	fArcRadius = hypot(psStart->x - psCenter->x, psStart->y - psCenter->y);
	if (fArcRadius == 0)
	{
		pcPath->AddCircle(psStart, fRadius, false);
		return;
	}

	if ((psStart->x == psEnd->x) && (psStart->y == psEnd->y))
	{
		//A full circle is a ring; the inner circle winds the other way.
		pcPath->AddCircle(psCenter, fArcRadius + fRadius, false);
		if (fArcRadius > fRadius)
		{
			pcPath->AddCircle(psCenter, fArcRadius - fRadius, true);
		}
		return;
	}

	//The end is put on the start radius; the specification allows them to differ slightly.
	fEndRadius = hypot(psEnd->x - psCenter->x, psEnd->y - psCenter->y);
	sStartRadial.Init((psStart->x - psCenter->x) / fArcRadius, (psStart->y - psCenter->y) / fArcRadius);
	if (fEndRadius == 0)
	{
		sEndRadial = sStartRadial;
	}
	else
	{
		sEndRadial.Init((psEnd->x - psCenter->x) / fEndRadius, (psEnd->y - psCenter->y) / fEndRadius);
	}

	sOuterStart.Init(psCenter->x + sStartRadial.x * (fArcRadius + fRadius), psCenter->y + sStartRadial.y * (fArcRadius + fRadius));
	sOuterEnd.Init(psCenter->x + sEndRadial.x * (fArcRadius + fRadius), psCenter->y + sEndRadial.y * (fArcRadius + fRadius));
	sEndCap.Init(psCenter->x + sEndRadial.x * fArcRadius, psCenter->y + sEndRadial.y * fArcRadius);

	fInner = fArcRadius - fRadius;
	if (fInner > 0)
	{
		//Outer arc, the end cap, the inner arc back and the start cap; all turning the same way.
		sInnerStart.Init(psCenter->x + sStartRadial.x * fInner, psCenter->y + sStartRadial.y * fInner);
		sInnerEnd.Init(psCenter->x + sEndRadial.x * fInner, psCenter->y + sEndRadial.y * fInner);
		pcPath->MoveTo(&sOuterStart);
		pcPath->ArcTo(&sOuterEnd, psCenter, bClockwise);
		pcPath->ArcTo(&sInnerEnd, &sEndCap, bClockwise);
		pcPath->ArcTo(&sInnerStart, psCenter, !bClockwise);
		pcPath->ArcTo(&sOuterStart, psStart, bClockwise);
	}
	else
	{
		//The stroke is wider than the arc radius; a sector to the center and the two caps, all winding the same way.
		pcPath->MoveTo(&sOuterStart);
		pcPath->ArcTo(&sOuterEnd, psCenter, bClockwise);
		pcPath->LineTo(psCenter);
		pcPath->LineTo(&sOuterStart);
		pcPath->AddCircle(psStart, fRadius, bClockwise);
		pcPath->AddCircle(&sEndCap, fRadius, bClockwise);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddVectorObround(CVectorPath* pcPath, float64 fWidth, float64 fHeight)
{
	SFloat64Vec2	sCenter;
	SFloat64Vec2	sPoint;
	SFloat64Vec2	sStart;
	SFloat64Vec2	sEnd;
	float64			fRadius;

	if (fWidth == fHeight)
	{
		sCenter.Init(0, 0);
		pcPath->AddCircle(&sCenter, fWidth / 2, false);
		return;
	}

	//A stadium along the longer side.
	if (fWidth > fHeight)
	{
		fRadius = fHeight / 2;
		sStart.Init(-(fWidth - fHeight) / 2, 0);
		sEnd.Init((fWidth - fHeight) / 2, 0);
	}
	else
	{
		fRadius = fWidth / 2;
		sStart.Init(0, -(fHeight - fWidth) / 2);
		sEnd.Init(0, (fHeight - fWidth) / 2);
	}
	AddVectorStroke(pcPath, &sStart, &sEnd, fRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddVectorRegularPolygon(CVectorPath* pcPath, SFloat64Vec2* psCenter, float64 fDiameter, int iVertices, float64 fRotation, float64 fPrimitiveRotation)
{
	SFloat64Vec2	asPoints[12];
	int				i;

	//The first vertex is on the positive X axis through the center, then the polygon is rotated about its center
	//and, for a macro primitive, the whole primitive about the macro origin.
	for (i = 0; i < iVertices; i++)
	{
		asPoints[i].Init(fDiameter / 2, 0);
		RotateVectorPoint(&asPoints[i], fRotation + 360.0 * i / iVertices);
		asPoints[i].x += psCenter->x;
		asPoints[i].y += psCenter->y;
		RotateVectorPoint(&asPoints[i], fPrimitiveRotation);
	}
	pcPath->AddPolygon(asPoints, iVertices, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddVectorRectangle(CVectorPath* pcPath, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum)
{
	SFloat64Vec2	asPoints[4];

	asPoints[0].Init(psMinimum->x, psMinimum->y);
	asPoints[1].Init(psMaximum->x, psMinimum->y);
	asPoints[2].Init(psMaximum->x, psMaximum->y);
	asPoints[3].Init(psMinimum->x, psMaximum->y);
	pcPath->AddPolygon(asPoints, 4, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// A rectangle with quarter circle corners.  The radius is limited to half
// the shorter side, which makes an obround.
//
//////////////////////////////////////////////////////////////////////////
void AddVectorRoundedRectangle(CVectorPath* pcPath, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, float64 fRadius)
{
	float64			fLimit;
	SFloat64Vec2	sPoint;
	SFloat64Vec2	sCenter;

	fLimit = (psMaximum->x - psMinimum->x) / 2;
	if ((psMaximum->y - psMinimum->y) / 2 < fLimit)
	{
		fLimit = (psMaximum->y - psMinimum->y) / 2;
	}
	if (fRadius > fLimit)
	{
		fRadius = fLimit;
	}
	if (fRadius <= 0)
	{
		AddVectorRectangle(pcPath, psMinimum, psMaximum);
		return;
	}

	sPoint.Init(psMinimum->x + fRadius, psMinimum->y);
	pcPath->MoveTo(&sPoint);
	sPoint.Init(psMaximum->x - fRadius, psMinimum->y);
	pcPath->LineTo(&sPoint);
	sCenter.Init(psMaximum->x - fRadius, psMinimum->y + fRadius);
	sPoint.Init(psMaximum->x, psMinimum->y + fRadius);
	pcPath->ArcTo(&sPoint, &sCenter, false);
	sPoint.Init(psMaximum->x, psMaximum->y - fRadius);
	pcPath->LineTo(&sPoint);
	sCenter.Init(psMaximum->x - fRadius, psMaximum->y - fRadius);
	sPoint.Init(psMaximum->x - fRadius, psMaximum->y);
	pcPath->ArcTo(&sPoint, &sCenter, false);
	sPoint.Init(psMinimum->x + fRadius, psMaximum->y);
	pcPath->LineTo(&sPoint);
	sCenter.Init(psMinimum->x + fRadius, psMaximum->y - fRadius);
	sPoint.Init(psMinimum->x, psMaximum->y - fRadius);
	pcPath->ArcTo(&sPoint, &sCenter, false);
	sPoint.Init(psMinimum->x, psMinimum->y + fRadius);
	pcPath->LineTo(&sPoint);
	sCenter.Init(psMinimum->x + fRadius, psMinimum->y + fRadius);
	sPoint.Init(psMinimum->x + fRadius, psMinimum->y);
	pcPath->ArcTo(&sPoint, &sCenter, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// The part of a circle counterclockwise from the start angle to the end
// angle, a pie slice.  An end 360 or more past the start is the whole
// circle.
//
//////////////////////////////////////////////////////////////////////////
void AddVectorSector(CVectorPath* pcPath, SFloat64Vec2* psCenter, float64 fRadius, float64 fStartAngle, float64 fEndAngle)
{
	SFloat64Vec2	sStart;
	SFloat64Vec2	sEnd;

	if (fRadius <= 0)
	{
		return;
	}
	if (fEndAngle - fStartAngle >= 360)
	{
		pcPath->AddCircle(psCenter, fRadius, false);
		return;
	}

	sStart.Init(fRadius, 0);
	RotateVectorPoint(&sStart, fStartAngle);
	sStart.x += psCenter->x;
	sStart.y += psCenter->y;
	sEnd.Init(fRadius, 0);
	RotateVectorPoint(&sEnd, fEndAngle);
	sEnd.x += psCenter->x;
	sEnd.y += psCenter->y;

	pcPath->MoveTo(psCenter);
	pcPath->LineTo(&sStart);
	pcPath->ArcTo(&sEnd, psCenter, false);
	pcPath->LineTo(psCenter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// The part of a ring between two radii counterclockwise from the start
// angle to the end angle; its ends are straight.  An end 360 or more past
// the start is the whole ring.  No inner radius is a sector.
//
//////////////////////////////////////////////////////////////////////////
void AddVectorRing(CVectorPath* pcPath, SFloat64Vec2* psCenter, float64 fOuterRadius, float64 fInnerRadius, float64 fStartAngle, float64 fEndAngle)
{
	SFloat64Vec2	sOuterStart;
	SFloat64Vec2	sOuterEnd;
	SFloat64Vec2	sInnerStart;
	SFloat64Vec2	sInnerEnd;

	if (fInnerRadius <= 0)
	{
		AddVectorSector(pcPath, psCenter, fOuterRadius, fStartAngle, fEndAngle);
		return;
	}
	if (fOuterRadius <= fInnerRadius)
	{
		return;
	}
	if (fEndAngle - fStartAngle >= 360)
	{
		pcPath->AddCircle(psCenter, fOuterRadius, false);
		pcPath->AddCircle(psCenter, fInnerRadius, true);
		return;
	}

	sOuterStart.Init(fOuterRadius, 0);
	RotateVectorPoint(&sOuterStart, fStartAngle);
	sOuterEnd.Init(fOuterRadius, 0);
	RotateVectorPoint(&sOuterEnd, fEndAngle);
	sInnerStart.Init(fInnerRadius, 0);
	RotateVectorPoint(&sInnerStart, fStartAngle);
	sInnerEnd.Init(fInnerRadius, 0);
	RotateVectorPoint(&sInnerEnd, fEndAngle);
	sOuterStart.x += psCenter->x;
	sOuterStart.y += psCenter->y;
	sOuterEnd.x += psCenter->x;
	sOuterEnd.y += psCenter->y;
	sInnerStart.x += psCenter->x;
	sInnerStart.y += psCenter->y;
	sInnerEnd.x += psCenter->x;
	sInnerEnd.y += psCenter->y;

	pcPath->MoveTo(&sOuterStart);
	pcPath->ArcTo(&sOuterEnd, psCenter, false);
	pcPath->LineTo(&sInnerEnd);
	pcPath->ArcTo(&sInnerStart, psCenter, true);
	pcPath->LineTo(&sOuterStart);
}

