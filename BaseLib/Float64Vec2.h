#ifndef __FLOAT64_VEC2_H__
#define __FLOAT64_VEC2_H__
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
#include "Vec2.h"


class SFloat64Vec3;
class SFloat4x4;
class SFloat64Vec2 : public SVec2<float64, float128>
{
CONSTRUCTABLE(SFloat64Vec2);
public:
	using SVec2<float64, float128>::operator=;

	void	Fix(void);

	int		WholeNumbers(void);
	void	Print(CChars* psx, int iWholeNumbers = -1, int iDecimals = 2);

	void	Normalize(void);
	bool	CloselyEqual(SFloat64Vec2* ps);
	bool	CloselyEqual(SFloat64Vec2* ps, float64 fTolerance);
};


typedef CArrayTemplate<SFloat64Vec2>		CArrayFloat64Vec2;


float64 		Float64Vec2Dot(const SFloat64Vec2* pV1, const SFloat64Vec2* pV2);
float64			Float64Vec2Cross(const SFloat64Vec2* pV1, const SFloat64Vec2* pV2);
SFloat64Vec2* 	Float64Vec2Add(SFloat64Vec2* pOut, const SFloat64Vec2* pV1, const SFloat64Vec2* pV2);
SFloat64Vec2* 	Float64Vec2Subtract(SFloat64Vec2* pOut, const SFloat64Vec2* pV1, const SFloat64Vec2* pV2);
SFloat64Vec2* 	Float64Vec2Minimize(SFloat64Vec2* pOut, const SFloat64Vec2* pV1, const SFloat64Vec2* pV2);
SFloat64Vec2* 	Float64Vec2Maximize(SFloat64Vec2* pOut, const SFloat64Vec2* pV1, const SFloat64Vec2* pV2);
SFloat64Vec2* 	Float64Vec2Scale(SFloat64Vec2* pOut, const SFloat64Vec2* pV, float64 s);
SFloat64Vec2* 	Float64Vec2Lerp(SFloat64Vec2* pOut, const SFloat64Vec2* pV1, const SFloat64Vec2* pV2, float64 s);
SFloat64Vec2*	Float64Vec2TransformCoord(SFloat64Vec2* pOut, SFloat64Vec2* pV, SFloat4x4* psMat);
void			Float64Vec2TransformCoords(SFloat64Vec2* asOut, int iOutStride, SFloat64Vec2* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints);
SFloat64Vec2*	Float64Vec2TransformNormal(SFloat64Vec2* pOut, SFloat64Vec2* pV, SFloat4x4* psMat);
void			Float64Vec2TransformNormals(SFloat64Vec2* asOut, int iOutStride, SFloat64Vec2* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints);
void			Float64Vec2MinMax(SFloat64Vec2* psMin, SFloat64Vec2* psMax, SFloat64Vec2* asIn, int iInStride, int iNumPoints);
void			Float64Vec2Swap(SFloat64Vec2* ps1, SFloat64Vec2* ps2);
void 			Float64Vec2InterpolatePosition(SFloat64Vec2* psVecDest, const SFloat64Vec2* psVec1, const SFloat64Vec2* psVec2, float64 fWeight);
void 			Float64Vec2InterpolateNormal(SFloat64Vec2* psVecDest, const SFloat64Vec2* psVec1, const SFloat64Vec2* psVec2, float64 fWeight);
void 			Float64Vec2RotateAboutNormalisedAxis(SFloat64Vec3* psVecDest, SFloat64Vec3 p, float64 fTheta, SFloat64Vec2 r);
void 			Float64Vec2Normalise(SFloat64Vec2* pOut, SFloat64Vec2* pV);


#endif // __FLOAT64_VEC2_H__

