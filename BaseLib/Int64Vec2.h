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
#ifndef __INT64_VEC2_H__
#define __INT64_VEC2_H__
#include "Vec2.h"


class SInt64Vec2  : public SVec2<int64, int64>
{
CONSTRUCTABLE(SInt64Vec2);
public:
	using SVec2<int64, int64>::operator=;

	void 	Print(CChars* psx, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};


typedef CArrayTemplate<SInt64Vec2>		CArrayInt64Vec2;


int64 			Int64Vec2Dot(const SInt64Vec2* pV1, const SInt64Vec2* pV2);
int64			Int64Vec2Cross(const SInt64Vec2* pV1, const SInt64Vec2* pV2);
SInt64Vec2* 	Int64Vec2Add(SInt64Vec2* pOut, const SInt64Vec2* pV1, const SInt64Vec2* pV2);
SInt64Vec2* 	Int64Vec2Subtract(SInt64Vec2* pOut, const SInt64Vec2* pV1, const SInt64Vec2* pV2);
SInt64Vec2* 	Int64Vec2Minimize(SInt64Vec2* pOut, const SInt64Vec2* pV1, const SInt64Vec2* pV2);
SInt64Vec2* 	Int64Vec2Maximize(SInt64Vec2* pOut, const SInt64Vec2* pV1, const SInt64Vec2* pV2);
SInt64Vec2* 	Int64Vec2Scale(SInt64Vec2* pOut, const SInt64Vec2* pV, float32 s);
SInt64Vec2* 	Int64Vec2Lerp(SInt64Vec2* pOut, const SInt64Vec2* pV1, const SInt64Vec2* pV2, float32 s);
void			Int64Vec2MinMax(SInt64Vec2* psMin, SInt64Vec2* psMax, SInt64Vec2* asIn, int iInStride, int iNumPoints);
void			Int64Vec2Swap(SInt64Vec2* ps1, SInt64Vec2* ps2);
void 			Int64Vec2InterpolatePosition(SInt64Vec2* psVecDest, const SInt64Vec2* psVec1, const SInt64Vec2* psVec2, float32 fWeight);


#endif // __INT64_VEC2_H__

