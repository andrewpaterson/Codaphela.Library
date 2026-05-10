#ifndef __INT32_VEC2_H__
#define __INT32_VEC2_H__
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
#include "Vec2.h"


class SInt32Vec2  : public SVec2<int32, int64>
{
CONSTRUCTABLE(SInt32Vec2);
public:
	using SVec2<int32, int64>::operator=;

	void 	Print(CChars* psx, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};


typedef CArrayTemplate<SInt32Vec2>		CArrayInt32Vec2;


int32 			Int32Vec2Dot(const SInt32Vec2* pV1, const SInt32Vec2* pV2);
int32			Int32Vec2Cross(const SInt32Vec2* pV1, const SInt32Vec2* pV2);
SInt32Vec2* 	Int32Vec2Add(SInt32Vec2* pOut, const SInt32Vec2* pV1, const SInt32Vec2* pV2);
SInt32Vec2* 	Int32Vec2Subtract(SInt32Vec2* pOut, const SInt32Vec2* pV1, const SInt32Vec2* pV2);
SInt32Vec2* 	Int32Vec2Minimize(SInt32Vec2* pOut, const SInt32Vec2* pV1, const SInt32Vec2* pV2);
SInt32Vec2* 	Int32Vec2Maximize(SInt32Vec2* pOut, const SInt32Vec2* pV1, const SInt32Vec2* pV2);
SInt32Vec2* 	Int32Vec2Scale(SInt32Vec2* pOut, const SInt32Vec2* pV, float32 s);
SInt32Vec2* 	Int32Vec2Lerp(SInt32Vec2* pOut, const SInt32Vec2* pV1, const SInt32Vec2* pV2, float32 s);
void			Int32Vec2MinMax(SInt32Vec2* psMin, SInt32Vec2* psMax, SInt32Vec2* asIn, int iInStride, int iNumPoints);
void			Int32Vec2Swap(SInt32Vec2* ps1, SInt32Vec2* ps2);
void 			Int32Vec2InterpolatePosition(SInt32Vec2* psVecDest, const SInt32Vec2* psVec1, const SInt32Vec2* psVec2, float32 fWeight);


#endif // __INT32_VEC2_H__

