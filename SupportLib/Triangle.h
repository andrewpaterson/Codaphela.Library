/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ShapeLib

Codaphela ShapeLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ShapeLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ShapeLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SHAPE_TRIANGLE_H__
#define __SHAPE_TRIANGLE_H__
#include "Plane.h"
#include "BaseLib/ArrayTemplate.h"


class CTriangle : public CPlane
{
public:
	SFloat32Vec3*	mpsPosition1;
	SFloat32Vec3*	mpsPosition2;

	void 			Init(SFloat32Vec3* psPoint1, SFloat32Vec3* psPoint2, SFloat32Vec3* psPoint3, SFloat32Vec3* psNormal);
	void 			Set(void);
	void 			Set(float32 x1, float32 y1, float32 z1, float32 x2, float32 y2, float32 z2, float32 x3, float32 y3, float32 z3);
	void 			Set(SFloat32Vec3* psPoint1, SFloat32Vec3* psPoint2, SFloat32Vec3* psPoint3);
	SFloat32Vec3*	GetPoint(size iPoint);
	bool			HasPositionPtr(SFloat32Vec3* psPosition);
	bool			IsDegenerate(void);
	size			IsSliver(float32 fAngleSliver, SFloat32Vec3** pps0, SFloat32Vec3** pps1, SFloat32Vec3** pps2);
	void			GetEdgeDirections(SFloat32Vec3* psEdge1, SFloat32Vec3* psEdge2, SFloat32Vec3* psEdge3);
	void			GetAngles(float32* psAngle1, float32* psAngle2, float32* psAngle3);
	void			Center(SFloat32Vec3* psCenter);
	void			Dump(void);
};


typedef CArrayTemplate<CTriangle> CArrayTriangle;


#endif //__SHAPE_TRIANGLE_H__

