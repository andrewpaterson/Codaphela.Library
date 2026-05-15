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
	SFloat32Vec3*	mpsCenter;

	SFloat32Vec3*	mpsLongAxis;
	SFloat32Vec3*	mpsMiddleAxis; 
	SFloat32Vec3*	mpsShortAxis;

	float32			mfLongLength;
	float32			mfMiddleLength;
	float32			mfShortLength;

	void		Init(SFloat32Vec3* psCenter, SFloat32Vec3* psLongAxis, SFloat32Vec3* psMiddleAxis, SFloat32Vec3* psShortAxis);
	void 		SetFromPointsAxisAligned(SFloat32Vec3* psPoints, int iStride, int iNumPoints);
	bool		SetFromPointsUsingBestFit(SFloat32Vec3* psPoints, int iStride, int iNumPoints, bool bPointsExtreme = false);
	float32		Volume(void);
	void		Find8Box(int* piLongCount, int* piMiddleCount, int* piShortCount);
	float32		PrivateVolumeOfCubesContaining(float32 ix, float32 iy, float32 iz);
};


#endif // __BOX_H__

