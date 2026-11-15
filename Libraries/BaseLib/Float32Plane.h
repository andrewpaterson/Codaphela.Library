#ifndef __FLOAT32_PLANE_H__
#define __FLOAT32_PLANE_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Float32Vec3.h"


//A plane: the points whose dot with its unit normal is its distance.  It is a half space too, whose inside is behind it: Distance
//is negative behind it, positive in front, and exactly zero is outside, as a CShape's is.
class SFloat32Plane
{
public:
	SFloat32Vec3	sNormal;
	float32			fDistance;

	void	Init(SFloat32Vec3* psUnitNormal, float32 fDistance);
	void	Init(SFloat32Vec3* psNormal, SFloat32Vec3* psPoint);  //Through psPoint.  psNormal needn't be a unit.
	bool	Init(SFloat32Vec3* ps1, SFloat32Vec3* ps2, SFloat32Vec3* ps3);  //Facing the side they go anticlockwise from.  False if they're in a line.

	float32	Distance(SFloat32Vec3* psPosition);
	bool	Contains(SFloat32Vec3* psPosition);
	void	ClosestPoint(SFloat32Vec3* psDest, SFloat32Vec3* psPosition);
	void	Flip(void);
};


#endif // __FLOAT32_PLANE_H__

