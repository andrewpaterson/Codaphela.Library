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
#ifndef __INT32_VEC4_H__
#define __INT32_VEC4_H__
#include "Vec4.h"


class SInt32Vec4 : public SVec4<int32, int64>
{
CONSTRUCTABLE(SInt32Vec4);
public:
			SInt32Vec4() {};
			SInt32Vec4(int32 x, int32 y, int32 z, int32 w);

	using SVec4<int32, int64>::operator=;

	void	Print(CChars* psz, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};


typedef CArrayTemplate<SInt32Vec4>		CArrayInt4;


int32 			Int4Dot(const SInt32Vec4* pV1, const SInt32Vec4* pV2);

SInt32Vec4* 	Int4Add(SInt32Vec4* pOut, const SInt32Vec4* pV1, const SInt32Vec4* pV2);
SInt32Vec4* 	Int4Subtract(SInt32Vec4* pOut, const SInt32Vec4* pV1, const SInt32Vec4* pV2);
SInt32Vec4* 	Int4Minimize(SInt32Vec4* pOut, const SInt32Vec4* pV1, const SInt32Vec4* pV2);
SInt32Vec4* 	Int4Maximize(SInt32Vec4* pOut, const SInt32Vec4* pV1, const SInt32Vec4* pV2);
SInt32Vec4* 	Int4Scale(SInt32Vec4* pOut, const SInt32Vec4* pV, float32 s);
SInt32Vec4* 	Int4Lerp(SInt32Vec4* pOut, const SInt32Vec4* pV1, const SInt32Vec4* pV2, float32 s);
void			Int4MinMax(SInt32Vec4* psMin, SInt32Vec4* psMax, SInt32Vec4* asIn, int iInStride, int iNumPoints);
void			Int4Swap(SInt32Vec4* ps1, SInt32Vec4* ps2);
void 			Int4InterpolatePosition(SInt32Vec4* psVecDest, const SInt32Vec4* psVec1, const SInt32Vec4* psVec2, float32 fWeight);


#endif // __INT32_VEC4_H__

