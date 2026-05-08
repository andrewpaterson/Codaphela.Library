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
#ifndef __FLOAT32_VEC4_H__
#define __FLOAT32_VEC4_H__
#include "Vec4.h"


class SFloat32Vec2;
class SFloat32Vec3;
class SFloat4x4;
class SFloat32Vec4 : public SVec4<float32, float64>
{
CONSTRUCTABLE(SFloat32Vec4);
public:
			SFloat32Vec4() {};
			SFloat32Vec4(float32 x, float32 y, float32 z, float32 w);

	using SVec4<float32, float64>::operator=;

	void 	Fix(void);
	int		WholeNumbers(void);
	void	Print(CChars* psz, int iWholeNumbers = -1, int iDecimals = 2);

	void 	Copy(const SFloat32Vec4* ps);
	void 	Copy(const SFloat32Vec3* ps);
	void 	Copy(const SFloat32Vec2* ps);

	void 	Normalize(void);
	bool 	CloselyEqual(SFloat32Vec4* ps);
	bool 	CloselyEqual(SFloat32Vec4* ps, float32 fTolerance);
};


typedef CArrayTemplate<SFloat32Vec4>		CArrayFloat4;


float32 		Float4Dot(const SFloat32Vec4* pV1, const SFloat32Vec4* pV2);
SFloat32Vec4* 	Float4Add(SFloat32Vec4* pOut, const SFloat32Vec4* pV1, const SFloat32Vec4* pV2);
SFloat32Vec4* 	Float4Subtract(SFloat32Vec4* pOut, const SFloat32Vec4* pV1, const SFloat32Vec4* pV2);
SFloat32Vec4* 	Float4Minimize(SFloat32Vec4* pOut, const SFloat32Vec4* pV1, const SFloat32Vec4* pV2);
SFloat32Vec4* 	Float4Maximize(SFloat32Vec4* pOut, const SFloat32Vec4* pV1, const SFloat32Vec4* pV2);
SFloat32Vec4* 	Float4Scale(SFloat32Vec4* pOut, const SFloat32Vec4* pV, float32 s);
SFloat32Vec4* 	Float4Lerp(SFloat32Vec4* pOut, const SFloat32Vec4* pV1, const SFloat32Vec4* pV2, float32 s);
SFloat32Vec4*   Float4TransformCoord(SFloat32Vec4* psOut, SFloat4x4* psMat, SFloat32Vec4* psV);


#endif // __FLOAT32_VEC4_H__

