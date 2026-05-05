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
	SFloat32Vec3		msStart;		//Starting point of line
	SFloat32Vec3		msDirection;	//unit direction from start

	void	Init(SFloat32Vec3* psStart, SFloat32Vec3* psDirection);
	void	SetFromPositions(SFloat32Vec3* s, SFloat32Vec3* e);
	void	SetFromDirection(SFloat32Vec3* sStartOfRay, SFloat32Vec3* sDirection);
	void	SetFromDirection(float32 xs, float32 ys, float32 zs, float32 xd, float32 yd, float32 zd);
	void	Copy(CRay* pcSource);
};


#endif // __RAY_H__

