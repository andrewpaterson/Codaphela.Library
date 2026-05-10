#ifndef __INT64_VEC3_H__
#define __INT64_VEC3_H__
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
#include "Chars.h"
#include "Vec3.h"


class SInt64Vec3 : public SVec3<int64, int64>
{
CONSTRUCTABLE(SInt64Vec3);
public:
	using SVec3<int64, int64>::operator=;

	void 	Print(CChars* psx, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};


typedef CArrayTemplate<SInt64Vec3>		CArrayInt64Vec3;


int64 			Int64Vec3Dot(const SInt64Vec3* pV1, const SInt64Vec3* pV2);
SInt64Vec3*		Int64Vec3Cross(SInt64Vec3* pOut, const SInt64Vec3* pV1, const SInt64Vec3* pV2);
SInt64Vec3* 	Int64Vec3Add(SInt64Vec3* pOut, const SInt64Vec3* pV1, const SInt64Vec3* pV2);
SInt64Vec3* 	Int64Vec3Subtract(SInt64Vec3* pOut, const SInt64Vec3* pV1, const SInt64Vec3* pV2);
SInt64Vec3* 	Int64Vec3Minimize(SInt64Vec3* pOut, const SInt64Vec3* pV1, const SInt64Vec3* pV2);
SInt64Vec3* 	Int64Vec3Maximize(SInt64Vec3* pOut, const SInt64Vec3* pV1, const SInt64Vec3* pV2);
SInt64Vec3*		Int64Vec3Scale(SInt64Vec3* pOut, const SInt64Vec3* pV, float32 s);
SInt64Vec3* 	Int64Vec3Lerp(SInt64Vec3* pOut, const SInt64Vec3* pV1, const SInt64Vec3* pV2, float32 s);
void			Int64Vec3MinMax(SInt64Vec3* psMin, SInt64Vec3* psMax, SInt64Vec3* asIn, int iInStride, int iNumPoints);
void			Int64Vec3Swap(SInt64Vec3* ps1, SInt64Vec3* ps2);
void 			Int64Vec3InterpolatePosition(SInt64Vec3* psVecDest, const SInt64Vec3* psVec1, const SInt64Vec3* psVec3, float32 fWeight);


#endif // __INT64_VEC3_H__

