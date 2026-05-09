#ifndef __INT32_VEC3_H__
#define __INT32_VEC3_H__
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
#include "Vec3.h"


class SInt32Vec3 : public SVec3<int32, int64>
{
CONSTRUCTABLE(SInt32Vec3);
public:
	SInt32Vec3() {};
	SInt32Vec3(int32 x, int32 y, int32 z);

	using SVec3<int32, int64>::operator=;

	void 	Print(CChars* psx, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};


typedef CArrayTemplate<SInt32Vec3>		CArrayInt32Vec3;


int32 			Int32Vec3Dot(const SInt32Vec3* pV1, const SInt32Vec3* pV2);
SInt32Vec3*		Int32Vec3Cross(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2);
SInt32Vec3* 	Int32Vec3Add(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2);
SInt32Vec3* 	Int32Vec3Subtract(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2);
SInt32Vec3* 	Int32Vec3Minimize(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2);
SInt32Vec3* 	Int32Vec3Maximize(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2);
SInt32Vec3*		Int32Vec3Scale(SInt32Vec3* pOut, const SInt32Vec3* pV, float32 s);
SInt32Vec3* 	Int32Vec3Lerp(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2, float32 s);
void			Int32Vec3MinMax(SInt32Vec3* psMin, SInt32Vec3* psMax, SInt32Vec3* asIn, int iInStride, int iNumPoints);
void			Int32Vec3Swap(SInt32Vec3* ps1, SInt32Vec3* ps2);
void 			Int32Vec3InterpolatePosition(SInt32Vec3* psVecDest, const SInt32Vec3* psVec1, const SInt32Vec3* psVec3, float32 fWeight);


#endif // __INT32_VEC3_H__

