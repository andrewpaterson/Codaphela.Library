#ifndef __FLOAT32_SYMMETRIC_3X3_H__
#define __FLOAT32_SYMMETRIC_3X3_H__
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


//The most sweeps of rotations Eigen makes.  A 3x3 matrix needs four or five to reach float32's precision.
#define FLOAT32_SYMMETRIC_3X3_SWEEPS		16


//A 3x3 matrix equal to its transpose, held as its six numbers, with the eigen solver the isosurface's vertex fit needs
//(ProceduralLib's Isosurface.txt): a least squares fit's A'A is one.
class SFloat32Symmetric3x3
{
public:
	float32	xx;
	float32	xy;
	float32	xz;
	float32	yy;
	float32	yz;
	float32	zz;

	void	Init(void);  //Zero.
	void	Init(float32 fXX, float32 fXY, float32 fXZ, float32 fYY, float32 fYZ, float32 fZZ);

	void	AddOuter(SFloat32Vec3* psVector);  //Adds v v', as A'A does when A grows by the row v.
	void	Multiply(SFloat32Vec3* psDest, SFloat32Vec3* psVector);

	void	Eigen(float32* pafValues, SFloat32Vec3* pasVectors);  //Three values, largest first, and their unit vectors at right angles to each other.
	void	PseudoInverseMultiply(SFloat32Vec3* psDest, SFloat32Vec3* psVector, float32 fCutOff);  //As if eigenvalues under fCutOff of the largest were zero.
};


#endif // __FLOAT32_SYMMETRIC_3X3_H__

