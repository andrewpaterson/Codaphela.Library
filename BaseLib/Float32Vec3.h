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

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __FLOAT32_VEC3_H__
#define __FLOAT32_VEC3_H__
#include "Vec3.h"


class SFloat32Vec4;
class SFloat32Vec2;
class SFloat4x4;
class SFloat32Vec3 : public SVec3<float32, float64>
{
CONSTRUCTABLE(SFloat32Vec3);
public:
				SFloat32Vec3() {};
				SFloat32Vec3(float32 x, float32 y, float32 z);

	using SVec3<float32, float64>::operator=;

	void 		Fix(void);
	int			WholeNumbers(void);
	void 		Print(CChars* psx, int iWholeNumbers = -1, int iDecimals = 2);

	void 		Copy(const SFloat32Vec4* ps);
	void 		Copy(const SFloat32Vec3* ps);
	void 		Copy(const SFloat32Vec2* ps);

	void 		Normalize(void);
	bool 		CloselyEqual(SFloat32Vec3* ps);
	bool 		CloselyEqual(SFloat32Vec3* ps, float32 fTolerance);
};


typedef CArrayTemplate<SFloat32Vec3>		CArrayFloat32Vec3;


float32 		Float32Vec3Dot(const SFloat32Vec3* pV1, const SFloat32Vec3* pV2);
SFloat32Vec3* 	Float32Vec3Cross(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2);
SFloat32Vec3* 	Float32Vec3Add(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2);
SFloat32Vec3* 	Float32Vec3Subtract(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2);
SFloat32Vec3* 	Float32Vec3Minimize(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2);
SFloat32Vec3* 	Float32Vec3Maximize(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2);
SFloat32Vec3* 	Float32Vec3Scale(SFloat32Vec3* pOut, const SFloat32Vec3* pV, float32 s);
SFloat32Vec3* 	Float32Vec3Lerp(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2, float32 s);
SFloat32Vec3*	Float32Vec3TransformCoord(SFloat32Vec3* pOut, SFloat32Vec3* pV, SFloat4x4* psMat);
void			Float32Vec3TransformCoords(SFloat32Vec3* asOut, int iOutStride, SFloat32Vec3* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints);
SFloat32Vec3*	Float32Vec3TransformNormal(SFloat32Vec3* pOut, SFloat32Vec3* pV, SFloat4x4* psMat);
void			Float32Vec3TransformNormals(SFloat32Vec3* asOut, int iOutStride, SFloat32Vec3* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints);
void			Float32Vec3MinMax(SFloat32Vec3* psMin, SFloat32Vec3* psMax, SFloat32Vec3* asIn, int iInStride, int iNumPoints);
void			Float32Vec3Swap(SFloat32Vec3* ps1, SFloat32Vec3* ps2);
void 			Float32Vec3InterpolatePosition(SFloat32Vec3* psVecDest, const SFloat32Vec3* psVec1, const SFloat32Vec3* psVec2, float32 fWeight);
void 			Float32Vec3InterpolateNormal(SFloat32Vec3* psVecDest, const SFloat32Vec3* psVec1, const SFloat32Vec3* psVec2, float32 fWeight);
void 			Float32Vec3RotateAboutNormalisedAxis(SFloat32Vec3* psVecDest, SFloat32Vec3 p, float32 fTheta, SFloat32Vec3 r);
void 			Float32Vec3Normalise(SFloat32Vec3* pOut, SFloat32Vec3* pV);


#endif // __FLOAT32_VEC3_H__

