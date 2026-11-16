#ifndef __FLOAT32_SPHERE_H__
#define __FLOAT32_SPHERE_H__
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
#include "Float32Bounds.h"


class SFloat32Triangle;
//A ball.  Distance is negative inside, positive outside and exactly zero is outside, as a CShape's is.
class SFloat32Sphere
{
public:
	SFloat32Vec3	sCenter;
	float32			fRadius;

	void	Init(SFloat32Vec3* psCenter, float32 fRadius);
	void	Init(float32 x, float32 y, float32 z, float32 fRadius);
	void	Init(SFloat32Vec3* psPoints, size uiStride, size uiNumPoints);  //Around points: centred on their average, out to the furthest.

	float32	Distance(SFloat32Vec3* psPosition);
	bool	Contains(SFloat32Vec3* psPosition);
	bool	Contains(SFloat32Triangle* psTriangle);  //All its corners.
	float32	Volume(void) const;
	float32	IntersectionVolume(SFloat32Sphere* psOther);
	void	GetBounds(SFloat32Bounds* psDest);
};


#endif // __FLOAT32_SPHERE_H__

