#ifndef __FLOAT32_BOX_H__
#define __FLOAT32_BOX_H__
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


//A box turned any way: its centre, its axes (units at right angles to each other, the longest first when it's fitted) and how far
//it reaches along each from its centre.  Distance is exact, negative inside, positive outside and exactly zero is outside, as a
//CShape's is.
class SFloat32Box
{
public:
	SFloat32Vec3	sCenter;
	SFloat32Vec3	asAxes[3];
	float32			afHalfLengths[3];

	void	Init(SFloat32Vec3* psCenter, SFloat32Vec3* psAxis0, float32 fHalfLength0, SFloat32Vec3* psAxis1, float32 fHalfLength1, SFloat32Vec3* psAxis2, float32 fHalfLength2);
	void	Init(SFloat32Bounds* psBounds);  //Along the axes, the longest first.
	void	Init(SFloat32Vec3* psPoints, size uiStride, size uiNumPoints);  //Around points along the axes, the longest first.

	float32	Distance(SFloat32Vec3* psPosition);
	bool	Contains(SFloat32Vec3* psPosition);
	float32	Volume(void);
	void	GetCorner(SFloat32Vec3* psDest, int iCorner);  //Bit i of iCorner set is the far end of axis i.
	void	GetBounds(SFloat32Bounds* psDest);
};


#endif // __FLOAT32_BOX_H__

