#include <math.h>
#include "GerberVectorShapes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void RotateGerberPoint(SFloat64Vec2* psPoint, float64 fDegrees)
{
	SGerberVectorTransform	sRotation;

	sRotation.Init();
	sRotation.fRotation = fDegrees;
	sRotation.Transform(psPoint);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddGerberStroke(CGerberVectorObject* pcObject, SFloat64Vec2* psStart, SFloat64Vec2* psEnd, float64 fRadius)
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
		pcObject->AddCircle(psStart, fRadius, false);
		return;
	}

	sDirection.Init((psEnd->x - psStart->x) / fLength, (psEnd->y - psStart->y) / fLength);
	sNormal.Init(-sDirection.y * fRadius, sDirection.x * fRadius);

	sPoint.Init(psStart->x - sNormal.x, psStart->y - sNormal.y);
	pcObject->MoveTo(&sPoint);
	sPoint.Init(psEnd->x - sNormal.x, psEnd->y - sNormal.y);
	pcObject->LineTo(&sPoint);
	sPoint.Init(psEnd->x + sNormal.x, psEnd->y + sNormal.y);
	pcObject->ArcTo(&sPoint, psEnd, false);
	sPoint.Init(psStart->x + sNormal.x, psStart->y + sNormal.y);
	pcObject->LineTo(&sPoint);
	sPoint.Init(psStart->x - sNormal.x, psStart->y - sNormal.y);
	pcObject->ArcTo(&sPoint, psStart, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddGerberArcStroke(CGerberVectorObject* pcObject, SFloat64Vec2* psStart, SFloat64Vec2* psEnd, SFloat64Vec2* psCenter, bool bClockwise, float64 fRadius)
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
		pcObject->AddCircle(psStart, fRadius, false);
		return;
	}

	if ((psStart->x == psEnd->x) && (psStart->y == psEnd->y))
	{
		//A full circle is a ring; the inner circle winds the other way.
		pcObject->AddCircle(psCenter, fArcRadius + fRadius, false);
		if (fArcRadius > fRadius)
		{
			pcObject->AddCircle(psCenter, fArcRadius - fRadius, true);
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
		pcObject->MoveTo(&sOuterStart);
		pcObject->ArcTo(&sOuterEnd, psCenter, bClockwise);
		pcObject->ArcTo(&sInnerEnd, &sEndCap, bClockwise);
		pcObject->ArcTo(&sInnerStart, psCenter, !bClockwise);
		pcObject->ArcTo(&sOuterStart, psStart, bClockwise);
	}
	else
	{
		//The stroke is wider than the arc radius; a sector to the center and the two caps, all winding the same way.
		pcObject->MoveTo(&sOuterStart);
		pcObject->ArcTo(&sOuterEnd, psCenter, bClockwise);
		pcObject->LineTo(psCenter);
		pcObject->LineTo(&sOuterStart);
		pcObject->AddCircle(psStart, fRadius, bClockwise);
		pcObject->AddCircle(&sEndCap, fRadius, bClockwise);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddGerberObround(CGerberVectorObject* pcObject, float64 fWidth, float64 fHeight)
{
	SFloat64Vec2	sCenter;
	SFloat64Vec2	sPoint;
	SFloat64Vec2	sStart;
	SFloat64Vec2	sEnd;
	float64			fRadius;

	if (fWidth == fHeight)
	{
		sCenter.Init(0, 0);
		pcObject->AddCircle(&sCenter, fWidth / 2, false);
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
	AddGerberStroke(pcObject, &sStart, &sEnd, fRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddGerberRegularPolygon(CGerberVectorObject* pcObject, SFloat64Vec2* psCenter, float64 fDiameter, int iVertices, float64 fRotation, float64 fPrimitiveRotation)
{
	SFloat64Vec2	asPoints[12];
	int				i;

	//The first vertex is on the positive X axis through the center, then the polygon is rotated about its center
	//and, for a macro primitive, the whole primitive about the macro origin.
	for (i = 0; i < iVertices; i++)
	{
		asPoints[i].Init(fDiameter / 2, 0);
		RotateGerberPoint(&asPoints[i], fRotation + 360.0 * i / iVertices);
		asPoints[i].x += psCenter->x;
		asPoints[i].y += psCenter->y;
		RotateGerberPoint(&asPoints[i], fPrimitiveRotation);
	}
	pcObject->AddPolygon(asPoints, iVertices, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddGerberHole(CGerberVectorObject* pcObject, CGerberAperture* pcAperture, size uiIndex, float64 fUnit)
{
	SFloat64Vec2	sCenter;

	//The optional round hole of a standard aperture winds the other way.  4.4.6
	if (pcAperture->NumParameters() > uiIndex)
	{
		sCenter.Init(0, 0);
		pcObject->AddCircle(&sCenter, pcAperture->GetParameter(uiIndex)->DoubleValue() * fUnit / 2, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddGerberThermal(CGerberVectorObject* pcObject, SFloat64Vec2* psCenter, float64 fOuter, float64 fInner, float64 fGap, float64 fRotation)
{
	float64			fOuterRadius;
	float64			fInnerRadius;
	float64			fHalfGap;
	float64			fOuterExtent;
	float64			fInnerExtent;
	SFloat64Vec2	asPoints[5];
	SFloat64Vec2	sOrigin;
	int				iQuarter;
	int				i;
	bool			bInner;

	//A ring broken by gaps on the X and Y axes through its center; four quarter pieces.  4.5.1.8
	fOuterRadius = fOuter / 2;
	fInnerRadius = fInner / 2;
	fHalfGap = fGap / 2;
	if (fOuterRadius * fOuterRadius <= 2 * fHalfGap * fHalfGap)
	{
		return;
	}
	fOuterExtent = sqrt(fOuterRadius * fOuterRadius - fHalfGap * fHalfGap);
	bInner = fInnerRadius * fInnerRadius > 2 * fHalfGap * fHalfGap;
	fInnerExtent = bInner ? sqrt(fInnerRadius * fInnerRadius - fHalfGap * fHalfGap) : fHalfGap;

	for (iQuarter = 0; iQuarter < 4; iQuarter++)
	{
		asPoints[0].Init(fOuterExtent, fHalfGap);
		asPoints[1].Init(fHalfGap, fOuterExtent);
		asPoints[2].Init(fHalfGap, fInnerExtent);
		asPoints[3].Init(fInnerExtent, fHalfGap);
		sOrigin.Init(0, 0);
		for (i = 0; i < 4; i++)
		{
			RotateGerberPoint(&asPoints[i], 90.0 * iQuarter);
			asPoints[i].x += psCenter->x;
			asPoints[i].y += psCenter->y;
			RotateGerberPoint(&asPoints[i], fRotation);
		}
		sOrigin = *psCenter;
		RotateGerberPoint(&sOrigin, fRotation);

		pcObject->MoveTo(&asPoints[0]);
		pcObject->ArcTo(&asPoints[1], &sOrigin, false);
		pcObject->LineTo(&asPoints[2]);
		if (bInner)
		{
			pcObject->ArcTo(&asPoints[3], &sOrigin, true);
		}
		else
		{
			pcObject->LineTo(&asPoints[3]);
		}
		pcObject->LineTo(&asPoints[0]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool AddGerberMacroPrimitive(CGerberVectorObject* pcObject, CGerberEvaluatedPrimitive* pcPrimitive, float64 fUnit)
{
	SFloat64Vec2	asPoints[4];
	SFloat64Vec2	sCenter;
	SFloat64Vec2	sDirection;
	SFloat64Vec2	sNormal;
	SFloat64Vec2*	psOutline;
	float64			fLength;
	float64			fRotation;
	size			uiNumValues;
	size			uiVertices;
	size			i;
	int				iVertices;

	uiNumValues = pcPrimitive->NumValues();
	fRotation = pcPrimitive->GetValue(uiNumValues - 1)->DoubleValue();

	switch (pcPrimitive->GetPrimitive())
	{
		case GAMP_Circle:
			//Exposure, Diameter, Center X, Center Y, Rotation
			pcObject->BeginPart(!pcPrimitive->GetValue(0)->IsZero());
			sCenter.Init(pcPrimitive->GetValue(2)->DoubleValue() * fUnit, pcPrimitive->GetValue(3)->DoubleValue() * fUnit);
			RotateGerberPoint(&sCenter, fRotation);
			pcObject->AddCircle(&sCenter, pcPrimitive->GetValue(1)->DoubleValue() * fUnit / 2, false);
			return true;

		case GAMP_Vector_Line:
			//Exposure, Width, Start X, Start Y, End X, End Y, Rotation.  The ends are square.
			pcObject->BeginPart(!pcPrimitive->GetValue(0)->IsZero());
			asPoints[0].Init(pcPrimitive->GetValue(2)->DoubleValue() * fUnit, pcPrimitive->GetValue(3)->DoubleValue() * fUnit);
			asPoints[1].Init(pcPrimitive->GetValue(4)->DoubleValue() * fUnit, pcPrimitive->GetValue(5)->DoubleValue() * fUnit);
			fLength = hypot(asPoints[1].x - asPoints[0].x, asPoints[1].y - asPoints[0].y);
			if (fLength == 0)
			{
				return true;
			}
			sDirection.Init((asPoints[1].x - asPoints[0].x) / fLength, (asPoints[1].y - asPoints[0].y) / fLength);
			sNormal.Init(-sDirection.y * pcPrimitive->GetValue(1)->DoubleValue() * fUnit / 2, sDirection.x * pcPrimitive->GetValue(1)->DoubleValue() * fUnit / 2);
			asPoints[2].Init(asPoints[1].x + sNormal.x, asPoints[1].y + sNormal.y);
			asPoints[3].Init(asPoints[0].x + sNormal.x, asPoints[0].y + sNormal.y);
			asPoints[1].Init(asPoints[1].x - sNormal.x, asPoints[1].y - sNormal.y);
			asPoints[0].Init(asPoints[0].x - sNormal.x, asPoints[0].y - sNormal.y);
			for (i = 0; i < 4; i++)
			{
				RotateGerberPoint(&asPoints[i], fRotation);
			}
			pcObject->AddPolygon(asPoints, 4, false);
			return true;

		case GAMP_Center_Line:
			//Exposure, Width, Height, Center X, Center Y, Rotation
			pcObject->BeginPart(!pcPrimitive->GetValue(0)->IsZero());
			sCenter.Init(pcPrimitive->GetValue(3)->DoubleValue() * fUnit, pcPrimitive->GetValue(4)->DoubleValue() * fUnit);
			sNormal.Init(pcPrimitive->GetValue(1)->DoubleValue() * fUnit / 2, pcPrimitive->GetValue(2)->DoubleValue() * fUnit / 2);
			asPoints[0].Init(sCenter.x - sNormal.x, sCenter.y - sNormal.y);
			asPoints[1].Init(sCenter.x + sNormal.x, sCenter.y - sNormal.y);
			asPoints[2].Init(sCenter.x + sNormal.x, sCenter.y + sNormal.y);
			asPoints[3].Init(sCenter.x - sNormal.x, sCenter.y + sNormal.y);
			for (i = 0; i < 4; i++)
			{
				RotateGerberPoint(&asPoints[i], fRotation);
			}
			pcObject->AddPolygon(asPoints, 4, false);
			return true;

		case GAMP_Outline:
			//Exposure, Number of vertices n, n + 1 points with the last equal to the first, Rotation
			pcObject->BeginPart(!pcPrimitive->GetValue(0)->IsZero());
			uiVertices = (size)pcPrimitive->GetValue(1)->IntValue();
			if (uiNumValues != 2 + 2 * (uiVertices + 1) + 1)
			{
				return false;
			}
			psOutline = (SFloat64Vec2*)malloc(sizeof(SFloat64Vec2) * uiVertices);
			for (i = 0; i < uiVertices; i++)
			{
				psOutline[i].Init(pcPrimitive->GetValue(2 + 2 * i)->DoubleValue() * fUnit, pcPrimitive->GetValue(3 + 2 * i)->DoubleValue() * fUnit);
				RotateGerberPoint(&psOutline[i], fRotation);
			}
			pcObject->AddPolygon(psOutline, uiVertices, false);
			free(psOutline);
			return true;

		case GAMP_Polygon:
			//Exposure, Number of vertices, Center X, Center Y, Diameter, Rotation
			pcObject->BeginPart(!pcPrimitive->GetValue(0)->IsZero());
			iVertices = pcPrimitive->GetValue(1)->IntValue();
			if ((iVertices < 3) || (iVertices > 12))
			{
				return false;
			}
			sCenter.Init(pcPrimitive->GetValue(2)->DoubleValue() * fUnit, pcPrimitive->GetValue(3)->DoubleValue() * fUnit);
			AddGerberRegularPolygon(pcObject, &sCenter, pcPrimitive->GetValue(4)->DoubleValue() * fUnit, iVertices, 0, fRotation);
			return true;

		case GAMP_Thermal_Center:
			//Center X, Center Y, Outer diameter, Inner diameter, Gap thickness, Rotation.  Exposure is always on.
			pcObject->BeginPart(true);
			sCenter.Init(pcPrimitive->GetValue(0)->DoubleValue() * fUnit, pcPrimitive->GetValue(1)->DoubleValue() * fUnit);
			AddGerberThermal(pcObject, &sCenter, pcPrimitive->GetValue(2)->DoubleValue() * fUnit, pcPrimitive->GetValue(3)->DoubleValue() * fUnit, pcPrimitive->GetValue(4)->DoubleValue() * fUnit, fRotation);
			return true;

		default:
			return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool AddGerberApertureShape(CGerberVectorObject* pcObject, CGerberAperture* pcAperture, float64 fUnit)
{
	SFloat64Vec2				asPoints[4];
	SFloat64Vec2				sCenter;
	float64						fWidth;
	float64						fHeight;
	float64						fRotation;
	CGerberEvaluatedPrimitives*	pcPrimitives;
	size						i;

	//The aperture around its origin, in millimetres.
	sCenter.Init(0, 0);
	switch (pcAperture->GetType())
	{
		case GAT_Circle:
			pcObject->BeginPart(true);
			pcObject->AddCircle(&sCenter, pcAperture->GetParameter(0)->DoubleValue() * fUnit / 2, false);
			AddGerberHole(pcObject, pcAperture, 1, fUnit);
			return true;

		case GAT_Rectangle:
			pcObject->BeginPart(true);
			fWidth = pcAperture->GetParameter(0)->DoubleValue() * fUnit;
			fHeight = pcAperture->GetParameter(1)->DoubleValue() * fUnit;
			asPoints[0].Init(-fWidth / 2, -fHeight / 2);
			asPoints[1].Init(fWidth / 2, -fHeight / 2);
			asPoints[2].Init(fWidth / 2, fHeight / 2);
			asPoints[3].Init(-fWidth / 2, fHeight / 2);
			pcObject->AddPolygon(asPoints, 4, false);
			AddGerberHole(pcObject, pcAperture, 2, fUnit);
			return true;

		case GAT_Obround:
			pcObject->BeginPart(true);
			AddGerberObround(pcObject, pcAperture->GetParameter(0)->DoubleValue() * fUnit, pcAperture->GetParameter(1)->DoubleValue() * fUnit);
			AddGerberHole(pcObject, pcAperture, 2, fUnit);
			return true;

		case GAT_Polygon:
			pcObject->BeginPart(true);
			fRotation = (pcAperture->NumParameters() > 2) ? pcAperture->GetParameter(2)->DoubleValue() : 0;
			AddGerberRegularPolygon(pcObject, &sCenter, pcAperture->GetParameter(0)->DoubleValue() * fUnit, pcAperture->GetParameter(1)->IntValue(), fRotation, 0);
			AddGerberHole(pcObject, pcAperture, 3, fUnit);
			return true;

		case GAT_Macro:
			pcPrimitives = pcAperture->GetPrimitives();
			for (i = 0; i < pcPrimitives->NumPrimitives(); i++)
			{
				if (!AddGerberMacroPrimitive(pcObject, pcPrimitives->GetPrimitive(i), fUnit))
				{
					return false;
				}
			}
			return true;

		default:
			return false;
	}
}

