/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/Numbers.h"
#include "ComponentShapes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IncludeBoundsPoint(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, float64 x, float64 y)
{
	if (x < psMinimum->x)
	{
		psMinimum->x = x;
	}
	if (y < psMinimum->y)
	{
		psMinimum->y = y;
	}
	if (x > psMaximum->x)
	{
		psMaximum->x = x;
	}
	if (y > psMaximum->y)
	{
		psMaximum->y = y;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentShapes::Init(void)
{
	masShapes.Init();
	masPoints.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentShapes::Kill(void)
{
	masPoints.Kill();
	masShapes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentShapes::Clear(void)
{
	masPoints.ReInit();
	masShapes.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentShape* CComponentShapes::AddShape(EComponentShape eType, bool bFill, bool bBorder)
{
	SComponentShape*	psShape;

	psShape = masShapes.Add();
	psShape->eType = eType;
	psShape->bFill = bFill;
	psShape->bBorder = bBorder;
	psShape->fLineWidth = SHAPE_LINE_WIDTH;
	psShape->eFill = SC_Shape;
	psShape->eBorder = SC_Shape;
	psShape->sFill = 0;
	psShape->sBorder = 0;
	psShape->uiFirstPoint = 0;
	psShape->uiNumPoints = 0;
	psShape->sCentre.Init(0, 0);
	psShape->fRadiusX = 0;
	psShape->fRadiusY = 0;
	psShape->fStartAngle = 0;
	psShape->fArcAngle = 0;
	psShape->bVisible = true;
	return psShape;
}


//////////////////////////////////////////////////////////////////////////
//
// Drawn in the border colour.
//
//////////////////////////////////////////////////////////////////////////
SComponentShape* CComponentShapes::AddLine(float64 x0, float64 y0, float64 x1, float64 y1, float64 fWidth)
{
	SComponentShape*	psShape;

	psShape = AddShape(CS_Line, false, true);
	psShape->fLineWidth = fWidth;
	psShape->uiFirstPoint = masPoints.NumElements();
	psShape->uiNumPoints = 2;
	masPoints.Add()->Init(x0, y0);
	masPoints.Add()->Init(x1, y1);
	return psShape;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentShape* CComponentShapes::AddRectangle(float64 fLeft, float64 fTop, float64 fRight, float64 fBottom, bool bFill, bool bBorder)
{
	SFloat64Vec2	asCorners[4];

	asCorners[0].Init(fLeft, fTop);
	asCorners[1].Init(fRight, fTop);
	asCorners[2].Init(fRight, fBottom);
	asCorners[3].Init(fLeft, fBottom);
	return AddPolygon(asCorners, 4, bFill, bBorder);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentShape* CComponentShapes::AddPolygon(SFloat64Vec2* asPoints, size uiNumPoints, bool bFill, bool bBorder)
{
	SComponentShape*	psShape;

	psShape = AddShape(CS_Polygon, bFill, bBorder);
	psShape->uiFirstPoint = masPoints.NumElements();
	psShape->uiNumPoints = uiNumPoints;
	masPoints.InsertBlockAfterEnd(asPoints, uiNumPoints);
	return psShape;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentShape* CComponentShapes::AddTriangle(float64 x0, float64 y0, float64 x1, float64 y1, float64 x2, float64 y2, bool bFill, bool bBorder)
{
	SFloat64Vec2	asCorners[3];

	asCorners[0].Init(x0, y0);
	asCorners[1].Init(x1, y1);
	asCorners[2].Init(x2, y2);
	return AddPolygon(asCorners, 3, bFill, bBorder);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentShape* CComponentShapes::AddArc(float64 fCentreX, float64 fCentreY, float64 fRadiusX, float64 fRadiusY, float64 fStartAngle, float64 fArcAngle, bool bFill, bool bBorder)
{
	SComponentShape*	psShape;

	psShape = AddShape(CS_Arc, bFill, bBorder);
	psShape->sCentre.Init(fCentreX, fCentreY);
	psShape->fRadiusX = fRadiusX;
	psShape->fRadiusY = fRadiusY;
	psShape->fStartAngle = fStartAngle;
	psShape->fArcAngle = fArcAngle;
	return psShape;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentShape* CComponentShapes::AddCircle(float64 fCentreX, float64 fCentreY, float64 fRadius, bool bFill, bool bBorder)
{
	SComponentShape*	psShape;

	psShape = AddShape(CS_Circle, bFill, bBorder);
	psShape->sCentre.Init(fCentreX, fCentreY);
	psShape->fRadiusX = fRadius;
	psShape->fRadiusY = fRadius;
	return psShape;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponentShapes::NumShapes(void)
{
	return masShapes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentShape* CComponentShapes::GetShape(size uiIndex)
{
	return masShapes.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2* CComponentShapes::GetPoint(size uiIndex)
{
	return masPoints.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CComponentShapes::GetFill(CCircuitPainter* pcPainter, SComponentShape* psShape)
{
	if (psShape->eFill == SC_Explicit)
	{
		return psShape->sFill;
	}
	else if (psShape->eFill == SC_DarkShape)
	{
		return pcPainter->GetColours()->msDarkShapeFill;
	}
	return pcPainter->GetColours()->msShapeFill;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CComponentShapes::GetBorder(CCircuitPainter* pcPainter, SComponentShape* psShape)
{
	if (psShape->eBorder == SC_Explicit)
	{
		return psShape->sBorder;
	}
	return pcPainter->GetColours()->msShapeBorder;
}


//////////////////////////////////////////////////////////////////////////
//
// The painter's transform places the shapes.
//
//////////////////////////////////////////////////////////////////////////
void CComponentShapes::Paint(CCircuitPainter* pcPainter)
{
	size	i;

	for (i = 0; i < masShapes.NumElements(); i++)
	{
		Paint(pcPainter, masShapes.Get(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentShapes::Paint(CCircuitPainter* pcPainter, SComponentShape* psShape)
{
	SFloat64Vec2*	asPoints;

	if (!psShape->bVisible)
	{
		return;
	}

	switch (psShape->eType)
	{
	case CS_Line:
		asPoints = masPoints.Get(psShape->uiFirstPoint);
		pcPainter->Line(asPoints[0].x, asPoints[0].y, asPoints[1].x, asPoints[1].y, psShape->fLineWidth, GetBorder(pcPainter, psShape));
		break;
	case CS_Polygon:
		pcPainter->Polygon(masPoints.Get(psShape->uiFirstPoint), psShape->uiNumPoints, psShape->bFill, psShape->bBorder, psShape->fLineWidth, GetFill(pcPainter, psShape), GetBorder(pcPainter, psShape));
		break;
	case CS_Arc:
		pcPainter->Arc(psShape->sCentre.x, psShape->sCentre.y, psShape->fRadiusX, psShape->fRadiusY, psShape->fStartAngle, psShape->fArcAngle,
					   psShape->bFill, psShape->bBorder, psShape->fLineWidth, GetFill(pcPainter, psShape), GetBorder(pcPainter, psShape));
		break;
	case CS_Circle:
		pcPainter->Circle(psShape->sCentre.x, psShape->sCentre.y, psShape->fRadiusX, psShape->bFill, psShape->bBorder, psShape->fLineWidth, GetFill(pcPainter, psShape), GetBorder(pcPainter, psShape));
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void IncludeArcPoint(SComponentShape* psShape, float64 fDegrees, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool* pbHasBounds)
{
	float64		fAngle;
	float64		x;
	float64		y;

	fAngle = fDegrees * N_PI_64 / 180.0;
	x = psShape->sCentre.x + psShape->fRadiusX * cos(fAngle);
	y = psShape->sCentre.y - psShape->fRadiusY * sin(fAngle);
	if (!*pbHasBounds)
	{
		psMinimum->Init(x, y);
		psMaximum->Init(x, y);
		*pbHasBounds = true;
	}
	IncludeBoundsPoint(psMinimum, psMaximum, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// Grows the box to hold every shape, in the shapes' unturned grid space.
// Line widths aren't included.  Arcs include only the part of the ellipse
// they sweep (and the centre when filled).  Returns whether the box holds
// anything.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentShapes::IncludeBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds)
{
	size				i;
	size				j;
	SComponentShape*	psShape;
	SFloat64Vec2*		psPoint;
	float64				fAngle;
	float64				fStart;
	float64				fEnd;

	for (i = 0; i < masShapes.NumElements(); i++)
	{
		psShape = masShapes.Get(i);
		if ((psShape->eType == CS_Line) || (psShape->eType == CS_Polygon))
		{
			for (j = 0; j < psShape->uiNumPoints; j++)
			{
				psPoint = masPoints.Get(psShape->uiFirstPoint + j);
				if (!bHasBounds)
				{
					psMinimum->Init(psPoint->x, psPoint->y);
					psMaximum->Init(psPoint->x, psPoint->y);
					bHasBounds = true;
				}
				IncludeBoundsPoint(psMinimum, psMaximum, psPoint->x, psPoint->y);
			}
		}
		else
		{
			//The swept ends and every quarter turn between them are the arc's extremes.
			if (psShape->eType == CS_Circle)
			{
				fStart = 0;
				fEnd = 360;
			}
			else if (psShape->fArcAngle >= 0)
			{
				fStart = psShape->fStartAngle;
				fEnd = psShape->fStartAngle + psShape->fArcAngle;
			}
			else
			{
				fStart = psShape->fStartAngle + psShape->fArcAngle;
				fEnd = psShape->fStartAngle;
			}
			IncludeArcPoint(psShape, fStart, psMinimum, psMaximum, &bHasBounds);
			IncludeArcPoint(psShape, fEnd, psMinimum, psMaximum, &bHasBounds);
			for (fAngle = ceil(fStart / 90.0) * 90.0; fAngle < fEnd; fAngle += 90.0)
			{
				IncludeArcPoint(psShape, fAngle, psMinimum, psMaximum, &bHasBounds);
			}
			if ((psShape->eType == CS_Arc) && psShape->bFill)
			{
				IncludeBoundsPoint(psMinimum, psMaximum, psShape->sCentre.x, psShape->sCentre.y);
			}
		}
	}
	return bHasBounds;
}

