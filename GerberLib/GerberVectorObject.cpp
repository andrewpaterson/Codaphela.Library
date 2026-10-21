#include <math.h>
#include "GerberVectorObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SGerberVectorTransform::Init(void)
{
	bMirrorX = false;
	bMirrorY = false;
	fRotation = 0;
	fScale = 1;
	sTranslation.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void GetGerberRotation(float64 fDegrees, float64* pfCos, float64* pfSin)
{
	float64	fQuarters;
	float64	fRadians;

	//Multiples of 90 degrees are exact.
	fQuarters = fDegrees / 90.0;
	if (fQuarters == floor(fQuarters))
	{
		switch (((int64)fQuarters % 4 + 4) % 4)
		{
			case 0:
				*pfCos = 1;
				*pfSin = 0;
				return;
			case 1:
				*pfCos = 0;
				*pfSin = 1;
				return;
			case 2:
				*pfCos = -1;
				*pfSin = 0;
				return;
			default:
				*pfCos = 0;
				*pfSin = -1;
				return;
		}
	}
	fRadians = fDegrees * GERBER_PI / 180.0;
	*pfCos = cos(fRadians);
	*pfSin = sin(fRadians);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SGerberVectorTransform::Transform(SFloat64Vec2* psPoint)
{
	float64	fCos;
	float64	fSin;
	float64	x;
	float64	y;

	x = bMirrorX ? -psPoint->x : psPoint->x;
	y = bMirrorY ? -psPoint->y : psPoint->y;
	GetGerberRotation(fRotation, &fCos, &fSin);
	psPoint->x = (x * fCos - y * fSin) * fScale + sTranslation.x;
	psPoint->y = (x * fSin + y * fCos) * fScale + sTranslation.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SGerberVectorTransform::FlipsWinding(void)
{
	return bMirrorX != bMirrorY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectorObject::Init(EGerberPolarity ePolarity)
{
	mePolarity = ePolarity;
	masSegments.Init();
	msMinimum.Init(0, 0);
	msMaximum.Init(0, 0);
	mbBounds = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectorObject::Init(CGerberVectorObject* pcSource)
{
	size	i;
	size	uiNumSegments;

	Init(pcSource->mePolarity);
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
void CGerberVectorObject::Kill(void)
{
	masSegments.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectorObject::BeginPart(bool bExposure)
{
	SGerberVectorSegment*	psSegment;

	psSegment = masSegments.Add();
	psSegment->eType = GVS_Part;
	psSegment->bExposure = bExposure;
	psSegment->bClockwise = false;
	psSegment->sPoint.Init(0, 0);
	psSegment->sCenter.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectorObject::MoveTo(SFloat64Vec2* psPoint)
{
	SGerberVectorSegment*	psSegment;

	psSegment = masSegments.Add();
	psSegment->eType = GVS_Move;
	psSegment->bExposure = true;
	psSegment->bClockwise = false;
	psSegment->sPoint = *psPoint;
	psSegment->sCenter.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectorObject::LineTo(SFloat64Vec2* psPoint)
{
	SGerberVectorSegment*	psSegment;

	psSegment = masSegments.Add();
	psSegment->eType = GVS_Line;
	psSegment->bExposure = true;
	psSegment->bClockwise = false;
	psSegment->sPoint = *psPoint;
	psSegment->sCenter.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectorObject::ArcTo(SFloat64Vec2* psPoint, SFloat64Vec2* psCenter, bool bClockwise)
{
	SGerberVectorSegment*	psSegment;

	psSegment = masSegments.Add();
	psSegment->eType = GVS_Arc;
	psSegment->bExposure = true;
	psSegment->bClockwise = bClockwise;
	psSegment->sPoint = *psPoint;
	psSegment->sCenter = *psCenter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectorObject::AddCircle(SFloat64Vec2* psCenter, float64 fRadius, bool bClockwise)
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
void CGerberVectorObject::AddPolygon(SFloat64Vec2* asPoints, size uiNumPoints, bool bReverse)
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
void CGerberVectorObject::Transform(SGerberVectorTransform* psTransform)
{
	size					i;
	size					uiNumSegments;
	SGerberVectorSegment*	psSegment;
	bool					bFlip;

	bFlip = psTransform->FlipsWinding();
	uiNumSegments = masSegments.NumElements();
	for (i = 0; i < uiNumSegments; i++)
	{
		psSegment = masSegments.Get(i);
		if (psSegment->eType != GVS_Part)
		{
			psTransform->Transform(&psSegment->sPoint);
		}
		if (psSegment->eType == GVS_Arc)
		{
			psTransform->Transform(&psSegment->sCenter);
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
void CGerberVectorObject::SetPolarity(EGerberPolarity ePolarity)
{
	mePolarity = ePolarity;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IncludeGerberPoint(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool* pbBounds, float64 x, float64 y)
{
	if (!*pbBounds)
	{
		psMinimum->Init(x, y);
		psMaximum->Init(x, y);
		*pbBounds = true;
		return;
	}
	if (x < psMinimum->x) psMinimum->x = x;
	if (y < psMinimum->y) psMinimum->y = y;
	if (x > psMaximum->x) psMaximum->x = x;
	if (y > psMaximum->y) psMaximum->y = y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 GetGerberArcSweep(SFloat64Vec2* psStart, SFloat64Vec2* psEnd, SFloat64Vec2* psCenter, bool bClockwise)
{
	float64	fStart;
	float64	fEnd;
	float64	fSweep;

	fStart = atan2(psStart->y - psCenter->y, psStart->x - psCenter->x);
	fEnd = atan2(psEnd->y - psCenter->y, psEnd->x - psCenter->x);
	fSweep = bClockwise ? fStart - fEnd : fEnd - fStart;
	while (fSweep <= 0)
	{
		fSweep += 2 * GERBER_PI;
	}
	while (fSweep > 2 * GERBER_PI)
	{
		fSweep -= 2 * GERBER_PI;
	}
	if ((psStart->x == psEnd->x) && (psStart->y == psEnd->y))
	{
		fSweep = 2 * GERBER_PI;
	}
	return fSweep;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectorObject::CalculateBounds(void)
{
	size					i;
	size					uiNumSegments;
	SGerberVectorSegment*	psSegment;
	SFloat64Vec2			sCurrent;
	float64					fRadius;
	float64					fStart;
	float64					fSweep;
	float64					fAngle;
	int						iQuarter;

	mbBounds = false;
	sCurrent.Init(0, 0);
	uiNumSegments = masSegments.NumElements();
	for (i = 0; i < uiNumSegments; i++)
	{
		psSegment = masSegments.Get(i);
		if (psSegment->eType == GVS_Part)
		{
			continue;
		}

		IncludeGerberPoint(&msMinimum, &msMaximum, &mbBounds, psSegment->sPoint.x, psSegment->sPoint.y);
		if (psSegment->eType == GVS_Arc)
		{
			//The axis extremes the arc passes through.
			fRadius = hypot(sCurrent.x - psSegment->sCenter.x, sCurrent.y - psSegment->sCenter.y);
			fStart = atan2(sCurrent.y - psSegment->sCenter.y, sCurrent.x - psSegment->sCenter.x);
			fSweep = GetGerberArcSweep(&sCurrent, &psSegment->sPoint, &psSegment->sCenter, psSegment->bClockwise);
			for (iQuarter = 0; iQuarter < 4; iQuarter++)
			{
				fAngle = iQuarter * GERBER_PI / 2 - fStart;
				if (psSegment->bClockwise)
				{
					fAngle = -fAngle;
				}
				while (fAngle < 0)
				{
					fAngle += 2 * GERBER_PI;
				}
				while (fAngle >= 2 * GERBER_PI)
				{
					fAngle -= 2 * GERBER_PI;
				}
				if (fAngle <= fSweep)
				{
					IncludeGerberPoint(&msMinimum, &msMaximum, &mbBounds,
						psSegment->sCenter.x + fRadius * cos(iQuarter * GERBER_PI / 2),
						psSegment->sCenter.y + fRadius * sin(iQuarter * GERBER_PI / 2));
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
EGerberPolarity CGerberVectorObject::GetPolarity(void)
{
	return mePolarity;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberVectorObject::IsDark(void)
{
	return mePolarity != GP_Clear;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGerberVectorObject::NumSegments(void)
{
	return masSegments.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SGerberVectorSegment* CGerberVectorObject::GetSegment(size uiIndex)
{
	return masSegments.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberVectorObject::HasBounds(void)
{
	return mbBounds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2* CGerberVectorObject::GetMinimum(void)
{
	return &msMinimum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2* CGerberVectorObject::GetMaximum(void)
{
	return &msMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberVectorObject::HasExposureOff(void)
{
	size	i;
	size	uiNumSegments;

	uiNumSegments = masSegments.NumElements();
	for (i = 0; i < uiNumSegments; i++)
	{
		if ((masSegments.Get(i)->eType == GVS_Part) && !masSegments.Get(i)->bExposure)
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
float64 CGerberVectorObject::Area(void)
{
	size					i;
	size					uiNumSegments;
	SGerberVectorSegment*	psSegment;
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
		if ((psSegment->eType == GVS_Line) || (psSegment->eType == GVS_Arc))
		{
			fArea += (sCurrent.x * psSegment->sPoint.y - psSegment->sPoint.x * sCurrent.y) / 2;
		}
		if (psSegment->eType == GVS_Arc)
		{
			fRadius = hypot(sCurrent.x - psSegment->sCenter.x, sCurrent.y - psSegment->sCenter.y);
			fSweep = GetGerberArcSweep(&sCurrent, &psSegment->sPoint, &psSegment->sCenter, psSegment->bClockwise);
			fSegment = fRadius * fRadius * (fSweep - sin(fSweep)) / 2;
			fArea += psSegment->bClockwise ? -fSegment : fSegment;
		}
		if (psSegment->eType != GVS_Part)
		{
			sCurrent = psSegment->sPoint;
		}
	}
	return fArea;
}

