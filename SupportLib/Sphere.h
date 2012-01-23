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
#ifndef __SHAPE_SPHERE_H__
#define __SHAPE_SPHERE_H__
#include "BaseLib/GeometricTypes.h"


class CTriangle;
class CSphere
{
public:
	SFloat3*	mpsPosition;
	float		mfRadius;

	void	Init(SFloat3*	psPosition);

	//Set from center of sphere and radius
	void 	Set(SFloat3* pos, float rad);
	void 	Set(float x, float y, float z, float rad);
	void 	SetFromPointsUsingAveragePosition(SFloat3* psPoints, int iStride, int iNumPoints);
	void 	SetFromPointsUsingBestFit(SFloat3* psPoints, int iStride, int iNumPoints);
	void 	BoundingBox(SFloat3* min, SFloat3* max);
	void 	Copy(CSphere* source);
	float 	IntersectSphereVolume(CSphere* pcOther);
	float 	Volume(void);
	BOOL	Contains(CTriangle* pcTriangle);
};


#endif // __SHAPE_SPHERE_H__

