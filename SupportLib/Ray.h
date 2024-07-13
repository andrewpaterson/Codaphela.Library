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
#ifndef __RAY_H__
#define __RAY_H__
#include "BaseLib/GeometricTypes.h"


class CRay
{
public:
	SFloat3	msStart;  //Starting point of line
	SFloat3	msDirection;  //unit direction from start

	void SetFromPositions(SFloat3* s, SFloat3* e);
	void SetFromDirection(SFloat3* sStartOfRay, SFloat3* sDirection);
	void SetFromDirection(float xs, float ys, float zs, float xd, float yd, float zd);
	void Copy(CRay* source);
};


#endif // __SHAPE_RAY_H__