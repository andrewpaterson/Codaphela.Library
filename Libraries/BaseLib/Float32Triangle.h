#ifndef __FLOAT32_TRIANGLE_H__
#define __FLOAT32_TRIANGLE_H__
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
#include "Float32Plane.h"
#include "Float32Bounds.h"


//A triangle, facing the side its corners go anticlockwise from.  It has no inside, so its Distance is unsigned.
class SFloat32Triangle
{
public:
	SFloat32Vec3	asPositions[3];

	void	Init(SFloat32Vec3* ps1, SFloat32Vec3* ps2, SFloat32Vec3* ps3);

	bool	GetNormal(SFloat32Vec3* psDest);  //A unit normal.  False, and no normal, if it has no area.
	bool	GetPlane(SFloat32Plane* psDest);
	float32	Area(void);
	void	GetCenter(SFloat32Vec3* psDest);
	void	GetBounds(SFloat32Bounds* psDest);
	void	ClosestPoint(SFloat32Vec3* psDest, SFloat32Vec3* psPosition);
	float32	Distance(SFloat32Vec3* psPosition);
};


#endif // __FLOAT32_TRIANGLE_H__

