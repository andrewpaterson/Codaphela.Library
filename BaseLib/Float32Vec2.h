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
#ifndef __FLOAT32_VEC2_H__
#define __FLOAT32_VEC2_H__
#include "Vec2.h"


class SFloat4;
class SFloat3;
class SFloat4x4;
class SFloat32Vec2 : public SVec2<float32, float64>
{
CONSTRUCTABLE(SFloat32Vec2);
public:
	using SVec2<float32, float64>::operator=;

	void	Fix(void);

	int		WholeNumbers(void);
	void	Print(CChars* psx, int iWholeNumbers = -1, int iDecimals = 2);

	void	Normalize(void);
	bool	CloselyEqual(SFloat32Vec2* ps);
	bool	CloselyEqual(SFloat32Vec2* ps, float32 fTolerance);
};


typedef CArrayTemplate<SFloat32Vec2>		CArrayFloat32Vec2;


float32 		Float2Dot(const SFloat32Vec2* pV1, const SFloat32Vec2* pV2);
float32			Float2Cross(const SFloat32Vec2* pV1, const SFloat32Vec2* pV2);
SFloat32Vec2* 	Float2Add(SFloat32Vec2* pOut, const SFloat32Vec2* pV1, const SFloat32Vec2* pV2);
SFloat32Vec2* 	Float2Subtract(SFloat32Vec2* pOut, const SFloat32Vec2* pV1, const SFloat32Vec2* pV2);
SFloat32Vec2* 	Float2Minimize(SFloat32Vec2* pOut, const SFloat32Vec2* pV1, const SFloat32Vec2* pV2);
SFloat32Vec2* 	Float2Maximize(SFloat32Vec2* pOut, const SFloat32Vec2* pV1, const SFloat32Vec2* pV2);
SFloat32Vec2* 	Float2Scale(SFloat32Vec2* pOut, const SFloat32Vec2* pV, float32 s);
SFloat32Vec2* 	Float2Lerp(SFloat32Vec2* pOut, const SFloat32Vec2* pV1, const SFloat32Vec2* pV2, float32 s);
SFloat32Vec2*	Float2TransformCoord(SFloat32Vec2* pOut, SFloat32Vec2* pV, SFloat4x4* psMat);
void			Float2TransformCoords(SFloat32Vec2* asOut, int iOutStride, SFloat32Vec2* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints);
SFloat32Vec2*	Float2TransformNormal(SFloat32Vec2* pOut, SFloat32Vec2* pV, SFloat4x4* psMat);
void			Float2TransformNormals(SFloat32Vec2* asOut, int iOutStride, SFloat32Vec2* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints);
void			Float2MinMax(SFloat32Vec2* psMin, SFloat32Vec2* psMax, SFloat32Vec2* asIn, int iInStride, int iNumPoints);
void			Float2Swap(SFloat32Vec2* ps1, SFloat32Vec2* ps2);
void 			Float2InterpolatePosition(SFloat32Vec2* psVecDest, const SFloat32Vec2* psVec1, const SFloat32Vec2* psVec2, float32 fWeight);
void 			Float2InterpolateNormal(SFloat32Vec2* psVecDest, const SFloat32Vec2* psVec1, const SFloat32Vec2* psVec2, float32 fWeight);
void 			Float2RotateAboutNormalisedAxis(SFloat3* psVecDest, SFloat3 p, float32 fTheta, SFloat32Vec2 r);
void 			Float2Normalize(SFloat32Vec2* pOut, SFloat32Vec2* pV);


#endif // __FLOAT32_VEC2_H__

