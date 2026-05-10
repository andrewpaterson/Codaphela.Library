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
#ifndef __INT64_VEC4_H__
#define __INT64_VEC4_H__
#include "Chars.h"
#include "Vec4.h"


class SInt64Vec4 : public SVec4<int64, int64>
{
CONSTRUCTABLE(SInt64Vec4);
public:
			SInt64Vec4() {};
			SInt64Vec4(int64 x, int64 y, int64 z, int64 w);

	using SVec4<int64, int64>::operator=;

	void	Print(CChars* psz, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};


typedef CArrayTemplate<SInt64Vec4>		CArrayInt64Vec4;


int64 			Int64Vec4Dot(const SInt64Vec4* pV1, const SInt64Vec4* pV2);
SInt64Vec4* 	Int64Vec4Add(SInt64Vec4* pOut, const SInt64Vec4* pV1, const SInt64Vec4* pV2);
SInt64Vec4* 	Int64Vec4Subtract(SInt64Vec4* pOut, const SInt64Vec4* pV1, const SInt64Vec4* pV2);
SInt64Vec4* 	Int64Vec4Minimize(SInt64Vec4* pOut, const SInt64Vec4* pV1, const SInt64Vec4* pV2);
SInt64Vec4* 	Int64Vec4Maximize(SInt64Vec4* pOut, const SInt64Vec4* pV1, const SInt64Vec4* pV2);
SInt64Vec4* 	Int64Vec4Scale(SInt64Vec4* pOut, const SInt64Vec4* pV, float32 s);
SInt64Vec4* 	Int64Vec4Lerp(SInt64Vec4* pOut, const SInt64Vec4* pV1, const SInt64Vec4* pV2, float32 s);
void			Int64Vec4MinMax(SInt64Vec4* psMin, SInt64Vec4* psMax, SInt64Vec4* asIn, int iInStride, int iNumPoints);
void			Int64Vec4Swap(SInt64Vec4* ps1, SInt64Vec4* ps2);
void 			Int64Vec4InterpolatePosition(SInt64Vec4* psVecDest, const SInt64Vec4* psVec1, const SInt64Vec4* psVec2, float32 fWeight);


#endif // __INT64_VEC4_H__

