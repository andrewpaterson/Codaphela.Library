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
	SFloat3*	mpsPosition1;
	SFloat3*	mpsPosition2;

	void 		Init(SFloat3* psPoint1, SFloat3* psPoint2, SFloat3* psPoint3, SFloat3* psNormal);
	void 		Set(void);
	void 		Set(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);
	void 		Set(SFloat3* psPoint1, SFloat3* psPoint2, SFloat3* psPoint3);
	SFloat3*	GetPoint(int iPoint);
	bool		HasPositionPtr(SFloat3* psPosition);
	bool		IsDegenerate(void);
	int			IsSliver(float fAngleSliver, SFloat3** pps0, SFloat3** pps1, SFloat3** pps2);
	void		GetEdgeDirections(SFloat3* psEdge1, SFloat3* psEdge2, SFloat3* psEdge3);
	void		GetAngles(float* psAngle1, float* psAngle2, float* psAngle3);
	void		Center(SFloat3* psCenter);
	void		Dump(void);
};


typedef CArrayTemplate<CTriangle> CArrayTriangle;


#endif //__SHAPE_TRIANGLE_H__

