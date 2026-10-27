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
#ifndef __VECTOR_SHAPES_H__
#define __VECTOR_SHAPES_H__
#include "VectorPath.h"


//Contours added to the current part of a path.  Angles are degrees counterclockwise, y up.  Each shape winds
//counterclockwise unless it says otherwise.
void	RotateVectorPoint(SFloat64Vec2* psPoint, float64 fDegrees);
void	AddVectorStroke(CVectorPath* pcPath, SFloat64Vec2* psStart, SFloat64Vec2* psEnd, float64 fRadius);
void	AddVectorArcStroke(CVectorPath* pcPath, SFloat64Vec2* psStart, SFloat64Vec2* psEnd, SFloat64Vec2* psCenter, bool bClockwise, float64 fRadius);
void	AddVectorObround(CVectorPath* pcPath, float64 fWidth, float64 fHeight);
void	AddVectorRegularPolygon(CVectorPath* pcPath, SFloat64Vec2* psCenter, float64 fDiameter, int iVertices, float64 fRotation, float64 fPrimitiveRotation);
void	AddVectorRectangle(CVectorPath* pcPath, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum);
void	AddVectorRoundedRectangle(CVectorPath* pcPath, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, float64 fRadius);
void	AddVectorRoundedRectangle(CVectorPath* pcPath, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, float64 fMinimumYRadius, float64 fMaximumYRadius);
void	AddVectorSector(CVectorPath* pcPath, SFloat64Vec2* psCenter, float64 fRadius, float64 fStartAngle, float64 fEndAngle);
void	AddVectorRing(CVectorPath* pcPath, SFloat64Vec2* psCenter, float64 fOuterRadius, float64 fInnerRadius, float64 fStartAngle, float64 fEndAngle);


#endif // __VECTOR_SHAPES_H__

