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
#ifndef __BOX_H__
#define __BOX_H__
#include "BaseLib/GeometricTypes.h"


class CBox
{
public:
	SFloat3*	mpsCenter;

	SFloat3*	mpsLongAxis;
	SFloat3*	mpsMiddleAxis; 
	SFloat3*	mpsShortAxis;

	float		mfLongLength;
	float		mfMiddleLength;
	float		mfShortLength;

	void	Init(SFloat3* psCenter, SFloat3* psLongAxis, SFloat3* psMiddleAxis, SFloat3* psShortAxis);
	void 	SetFromPointsAxisAligned(SFloat3* psPoints, int iStride, int iNumPoints);
	BOOL	SetFromPointsUsingBestFit(SFloat3* psPoints, int iStride, int iNumPoints, BOOL bPointsExtreme = FALSE);
	float	Volume(void);
	void	Find8Box(int* piLongCount, int* piMiddleCount, int* piShortCount);
	float	PrivateVolumeOfCubesContaining(float ix, float iy, float iz);
};


#endif // __BOX_H__

