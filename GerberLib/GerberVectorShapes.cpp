#include <math.h>
#include "GerberVectorShapes.h"


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
			RotateVectorPoint(&asPoints[i], 90.0 * iQuarter);
			asPoints[i].x += psCenter->x;
			asPoints[i].y += psCenter->y;
			RotateVectorPoint(&asPoints[i], fRotation);
		}
		sOrigin = *psCenter;
		RotateVectorPoint(&sOrigin, fRotation);

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
			RotateVectorPoint(&sCenter, fRotation);
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
				RotateVectorPoint(&asPoints[i], fRotation);
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
				RotateVectorPoint(&asPoints[i], fRotation);
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
				RotateVectorPoint(&psOutline[i], fRotation);
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
			AddVectorRegularPolygon(pcObject, &sCenter, pcPrimitive->GetValue(4)->DoubleValue() * fUnit, iVertices, 0, fRotation);
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
			AddVectorObround(pcObject, pcAperture->GetParameter(0)->DoubleValue() * fUnit, pcAperture->GetParameter(1)->DoubleValue() * fUnit);
			AddGerberHole(pcObject, pcAperture, 2, fUnit);
			return true;

		case GAT_Polygon:
			pcObject->BeginPart(true);
			fRotation = (pcAperture->NumParameters() > 2) ? pcAperture->GetParameter(2)->DoubleValue() : 0;
			AddVectorRegularPolygon(pcObject, &sCenter, pcAperture->GetParameter(0)->DoubleValue() * fUnit, pcAperture->GetParameter(1)->IntValue(), fRotation, 0);
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

