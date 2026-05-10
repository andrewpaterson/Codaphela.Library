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
#ifndef __FLOAT64_VEC3_H__
#define __FLOAT64_VEC3_H__
#include "Vec3.h"


class SFloat32Vec4;
class SFloat64Vec2;
class SFloat4x4;
class SFloat64Vec3 : public SVec3<float64, float128>
{
	CONSTRUCTABLE(SFloat64Vec3);
public:
	using SVec3<float64, float128>::operator=;

	void 		Fix(void);
	int			WholeNumbers(void);
	void 		Print(CChars* psx, int iWholeNumbers = -1, int iDecimals = 2);

	void 		Copy(const SFloat32Vec4* ps);
	void 		Copy(const SFloat64Vec3* ps);
	void 		Copy(const SFloat64Vec2* ps);

	void 		Normalize(void);
	bool 		CloselyEqual(SFloat64Vec3* ps);
	bool 		CloselyEqual(SFloat64Vec3* ps, float64 fTolerance);
};


typedef CArrayTemplate<SFloat64Vec3>		CArrayFloat64Vec3;


float64 		Float64Vec3Dot(const SFloat64Vec3* pV1, const SFloat64Vec3* pV2);
SFloat64Vec3*	Float64Vec3Cross(SFloat64Vec3* pOut, const SFloat64Vec3* pV1, const SFloat64Vec3* pV2);
SFloat64Vec3*	Float64Vec3Add(SFloat64Vec3* pOut, const SFloat64Vec3* pV1, const SFloat64Vec3* pV2);
SFloat64Vec3*	Float64Vec3Subtract(SFloat64Vec3* pOut, const SFloat64Vec3* pV1, const SFloat64Vec3* pV2);
SFloat64Vec3*	Float64Vec3Minimize(SFloat64Vec3* pOut, const SFloat64Vec3* pV1, const SFloat64Vec3* pV2);
SFloat64Vec3*	Float64Vec3Maximize(SFloat64Vec3* pOut, const SFloat64Vec3* pV1, const SFloat64Vec3* pV2);
SFloat64Vec3*	Float64Vec3Scale(SFloat64Vec3* pOut, const SFloat64Vec3* pV, float64 s);
SFloat64Vec3*	Float64Vec3Lerp(SFloat64Vec3* pOut, const SFloat64Vec3* pV1, const SFloat64Vec3* pV2, float64 s);
SFloat64Vec3*	Float64Vec3TransformCoord(SFloat64Vec3* pOut, SFloat64Vec3* pV, SFloat4x4* psMat);
void			Float64Vec3TransformCoords(SFloat64Vec3* asOut, int iOutStride, SFloat64Vec3* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints);
SFloat64Vec3*	Float64Vec3TransformNormal(SFloat64Vec3* pOut, SFloat64Vec3* pV, SFloat4x4* psMat);
void			Float64Vec3TransformNormals(SFloat64Vec3* asOut, int iOutStride, SFloat64Vec3* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints);
void			Float64Vec3MinMax(SFloat64Vec3* psMin, SFloat64Vec3* psMax, SFloat64Vec3* asIn, int iInStride, int iNumPoints);
void			Float64Vec3Swap(SFloat64Vec3* ps1, SFloat64Vec3* ps2);
void 			Float64Vec3InterpolatePosition(SFloat64Vec3* psVecDest, const SFloat64Vec3* psVec1, const SFloat64Vec3* psVec2, float64 fWeight);
void 			Float64Vec3InterpolateNormal(SFloat64Vec3* psVecDest, const SFloat64Vec3* psVec1, const SFloat64Vec3* psVec2, float64 fWeight);
void 			Float64Vec3RotateAboutNormalisedAxis(SFloat64Vec3* psVecDest, SFloat64Vec3 p, float64 fTheta, SFloat64Vec3 r);
void 			Float64Vec3Normalise(SFloat64Vec3* pOut, SFloat64Vec3* pV);


#endif // __FLOAT64_VEC3_H__

