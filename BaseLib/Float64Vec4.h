#ifndef __FLOAT64_VEC4_H__
#define __FLOAT64_VEC4_H__
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
#include "Vec4.h"


class SFloat64Vec2;
class SFloat64Vec3;
class SDouble4x4;
class SFloat64Vec4 : public SVec4<float64, float128>
{
CONSTRUCTABLE(SFloat64Vec4);
public:
			SFloat64Vec4() {};
			SFloat64Vec4(float64 x, float64 y, float64 z, float64 w);

	using SVec4<float64, float128>::operator=;

	void 	Fix(void);
	int		WholeNumbers(void);
	void	Print(CChars* psz, int iWholeNumbers = -1, int iDecimals = 2);

	void 	Copy(const SFloat64Vec4* ps);
	void 	Copy(const SFloat64Vec3* ps);
	void 	Copy(const SFloat64Vec2* ps);

	void 	Normalize(void);
	bool 	CloselyEqual(SFloat64Vec4* ps);
	bool 	CloselyEqual(SFloat64Vec4* ps, float64 fTolerance);
};


typedef CArrayTemplate<SFloat64Vec4>		CArrayFloat64Vec4;


float64 		Float64Vec4Dot(const SFloat64Vec4* pV1, const SFloat64Vec4* pV2);
//SFloat64Vec4* Float64Vec4Cross(SFloat64Vec4* pOut, const SFloat64Vec4* pV1, const SFloat64Vec4* pV2);
SFloat64Vec4* 	Float64Vec4Add(SFloat64Vec4* pOut, const SFloat64Vec4* pV1, const SFloat64Vec4* pV2);
SFloat64Vec4* 	Float64Vec4Subtract(SFloat64Vec4* pOut, const SFloat64Vec4* pV1, const SFloat64Vec4* pV2);
SFloat64Vec4* 	Float64Vec4Minimize(SFloat64Vec4* pOut, const SFloat64Vec4* pV1, const SFloat64Vec4* pV2);
SFloat64Vec4* 	Float64Vec4Maximize(SFloat64Vec4* pOut, const SFloat64Vec4* pV1, const SFloat64Vec4* pV2);
SFloat64Vec4* 	Float64Vec4Scale(SFloat64Vec4* pOut, const SFloat64Vec4* pV, float64 s);
SFloat64Vec4* 	Float64Vec4Lerp(SFloat64Vec4* pOut, const SFloat64Vec4* pV1, const SFloat64Vec4* pV2, float64 s);
SFloat64Vec4*   Float64Vec4TransformCoord(SFloat64Vec4* psOut, SFloat64Vec4* psV, SDouble4x4* psMat);
void			Float64Vec4TransformCoords(SFloat64Vec4* asOut, int iOutStride, SFloat64Vec4* asIn, int iInStride, SDouble4x4* psMat, int iNumPoints);
SFloat64Vec4*	Float64Vec4TransformNormal(SFloat64Vec4* pOut, SFloat64Vec4* pV, SDouble4x4* psMat);
void			Float64Vec4TransformNormals(SFloat64Vec4* asOut, int iOutStride, SFloat64Vec4* asIn, int iInStride, SDouble4x4* psMat, int iNumPoints);
void			Float64Vec4MinMax(SFloat64Vec4* psMin, SFloat64Vec4* psMax, SFloat64Vec4* asIn, int iInStride, int iNumPoints);
void			Float64Vec4Swap(SFloat64Vec4* ps1, SFloat64Vec4* ps2);
void 			Float64Vec4InterpolatePosition(SFloat64Vec4* psVecDest, const SFloat64Vec4* psVec1, const SFloat64Vec4* psVec2, float64 fWeight);
void 			Float64Vec4InterpolateNormal(SFloat64Vec4* psVecDest, const SFloat64Vec4* psVec1, const SFloat64Vec4* psVec2, float64 fWeight);
//void 			Float64Vec4RotateAboutNormalisedAxis(SFloat64Vec4* psVecDest, SFloat64Vec4 p, float64 fTheta, SFloat64Vec4 r);
void 			Float64Vec4Normalise(SFloat64Vec4* pOut, SFloat64Vec4* pV);


#endif // __FLOAT64_VEC4_H__

