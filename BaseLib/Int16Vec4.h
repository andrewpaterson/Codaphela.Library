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
#ifndef __INT16_VEC4_H__
#define __INT16_VEC4_H__
#include "Chars.h"
#include "Vec4.h"


class SInt16Vec4 : public SVec4<int16, int32>
{
CONSTRUCTABLE(SInt16Vec4);
public:
			SInt16Vec4() {};
			SInt16Vec4(int16 x, int16 y, int16 z, int16 w);

	using SVec4<int16, int32>::operator=;

	void	Print(CChars* psz, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};

typedef CArrayTemplate<SInt16Vec4>		CArrayInt16Vec4;


int16 			Int16Vec4Dot(const SInt16Vec4* pV1, const SInt16Vec4* pV2);
SInt16Vec4* 	Int16Vec4Add(SInt16Vec4* pOut, const SInt16Vec4* pV1, const SInt16Vec4* pV2);
SInt16Vec4* 	Int16Vec4Subtract(SInt16Vec4* pOut, const SInt16Vec4* pV1, const SInt16Vec4* pV2);
SInt16Vec4* 	Int16Vec4Minimize(SInt16Vec4* pOut, const SInt16Vec4* pV1, const SInt16Vec4* pV2);
SInt16Vec4* 	Int16Vec4Maximize(SInt16Vec4* pOut, const SInt16Vec4* pV1, const SInt16Vec4* pV2);
SInt16Vec4* 	Int16Vec4Scale(SInt16Vec4* pOut, const SInt16Vec4* pV, float32 s);
SInt16Vec4* 	Int16Vec4Lerp(SInt16Vec4* pOut, const SInt16Vec4* pV1, const SInt16Vec4* pV2, float32 s);
void			Int16Vec4MinMax(SInt16Vec4* psMin, SInt16Vec4* psMax, SInt16Vec4* asIn, int iInStride, int iNumPoints);
void			Int16Vec4Swap(SInt16Vec4* ps1, SInt16Vec4* ps2);
void 			Int16Vec4InterpolatePosition(SInt16Vec4* psVecDest, const SInt16Vec4* psVec1, const SInt16Vec4* psVec2, float32 fWeight);


#endif // __INT16_VEC4_H__

