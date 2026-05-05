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
#ifndef __PLANE_H__
#define __PLANE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/GeometricTypes.h"
#include "HalfSpace.h"


class CPlane : public CHalfSpace
{
public:
	SFloat32Vec3*	mpsPosition;	//For a triangle this is the first of three points.

	void Init(SFloat32Vec3* psPosition, SFloat32Vec3* mpsNormal);

	//Set from three points
	void Set(SFloat32Vec3* v1, SFloat32Vec3* v2, SFloat32Vec3* v3);
	void Set(float32 x1, float32 y1, float32 z1, float32 x2, float32 y2, float32 z2, float32 x3, float32 y3, float32 z3);
	void Set(SFloat32Vec3* psPoint, SFloat32Vec3* psNormal);
	void Set(void);
	void Copy(CPlane* psSource);
	bool Coplanar(CPlane* psOther);
};


typedef CArrayTemplate<CPlane> CPlaneArray;


#endif //__PLANE_H__

