#ifndef __FLOAT32_BOUNDS_H__
#define __FLOAT32_BOUNDS_H__
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


#define FLOAT32_BOUNDS_LARGEST		(3.4e38f)


//A box along the axes.  Empty, it has its minimum above its maximum, so the first point added is all of it.  Distance is negative
//inside, positive outside and exactly zero is outside, as a CShape's is.
class SFloat32Bounds
{
public:
	SFloat32Vec3	sMinimum;
	SFloat32Vec3	sMaximum;

	void	Init(void);
	void	Init(SFloat32Vec3* psMinimum, SFloat32Vec3* psMaximum);
	void	Init(SFloat32Vec3* psPoints, size uiStride, size uiNumPoints);

	bool	IsEmpty(void) const;
	void	Add(SFloat32Vec3* psPosition);
	void	Add(SFloat32Bounds* psBounds);

	void	GetCenter(SFloat32Vec3* psDest);
	void	GetSize(SFloat32Vec3* psDest);
	float32	Distance(SFloat32Vec3* psPosition);
	bool	Contains(SFloat32Vec3* psPosition);
};


//The distance from a box centred on the origin, with its sides along the axes, to the point psOffset.
float32 Float32CenteredBoxDistance(SFloat32Vec3* psOffset, SFloat32Vec3* psHalfLengths);


#endif // __FLOAT32_BOUNDS_H__

