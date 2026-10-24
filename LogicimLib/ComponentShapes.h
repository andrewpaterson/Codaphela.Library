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
#ifndef __COMPONENT_SHAPES_H__
#define __COMPONENT_SHAPES_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Float64Vec2.h"
#include "CircuitPainter.h"


#define SHAPE_LINE_WIDTH	2.0


enum EComponentShape
{
	CS_Line,
	CS_Polygon,
	CS_Arc,
	CS_Circle,
};


//Which of the painter's colours a shape's fill or border is drawn in, unless it has its own.
enum EShapeColour
{
	SC_Shape,			//The shape fill or the shape border.
	SC_DarkShape,		//The dark shape fill.
	SC_Explicit,
};


//A shape in a component's grid space, facing North about the component's position.  Lines and polygons keep their
//points in the shapes' point array.  Arcs and circles are an ellipse centre and radii; arc angles are degrees
//counterclockwise on the screen from three o'clock, as Java's drawArc.
struct SComponentShape
{
	EComponentShape	eType;
	bool			bFill;
	bool			bBorder;
	float64			fLineWidth;
	EShapeColour	eFill;
	EShapeColour	eBorder;
	ARGB32			sFill;
	ARGB32			sBorder;
	size			uiFirstPoint;
	size			uiNumPoints;
	SFloat64Vec2	sCentre;
	float64			fRadiusX;
	float64			fRadiusY;
	float64			fStartAngle;
	float64			fArcAngle;
	bool			bVisible;
};


typedef CArrayTemplate<SComponentShape>	CArrayComponentShape;
typedef CArrayTemplate<SFloat64Vec2>	CArrayShapePoint;


//The shapes a component is drawn with, in the order they're drawn.
class CComponentShapes
{
protected:
	CArrayComponentShape	masShapes;
	CArrayShapePoint		masPoints;

public:
	void				Init(void);
	void				Kill(void);
	void				Clear(void);

	SComponentShape*	AddLine(float64 x0, float64 y0, float64 x1, float64 y1, float64 fWidth = SHAPE_LINE_WIDTH);
	SComponentShape*	AddRectangle(float64 fLeft, float64 fTop, float64 fRight, float64 fBottom, bool bFill, bool bBorder);
	SComponentShape*	AddPolygon(SFloat64Vec2* asPoints, size uiNumPoints, bool bFill, bool bBorder);
	SComponentShape*	AddTriangle(float64 x0, float64 y0, float64 x1, float64 y1, float64 x2, float64 y2, bool bFill, bool bBorder);
	SComponentShape*	AddArc(float64 fCentreX, float64 fCentreY, float64 fRadiusX, float64 fRadiusY, float64 fStartAngle, float64 fArcAngle, bool bFill, bool bBorder);
	SComponentShape*	AddCircle(float64 fCentreX, float64 fCentreY, float64 fRadius, bool bFill, bool bBorder);

	size				NumShapes(void);
	SComponentShape*	GetShape(size uiIndex);
	SFloat64Vec2*		GetPoint(size uiIndex);

	void				Paint(CCircuitPainter* pcPainter);
	void				Paint(CCircuitPainter* pcPainter, SComponentShape* psShape);
	bool				IncludeBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds);

protected:
	SComponentShape*	AddShape(EComponentShape eType, bool bFill, bool bBorder);
	ARGB32				GetFill(CCircuitPainter* pcPainter, SComponentShape* psShape);
	ARGB32				GetBorder(CCircuitPainter* pcPainter, SComponentShape* psShape);
};


void IncludeBoundsPoint(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, float64 x, float64 y);


#endif // __COMPONENT_SHAPES_H__
