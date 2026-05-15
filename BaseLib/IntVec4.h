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
#ifndef __INT_VEC4_H__
#define __INT_VEC4_H__
#include "Chars.h"
#include "Vec4.h"


class SIntVec4 : public SVec4<int, longint>
{
CONSTRUCTABLE(SIntVec4);
public:
			SIntVec4() {};
			SIntVec4(int x, int y, int z, int w);

	using SVec4<int, longint>::operator=;

	void	Print(CChars* psz, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};

typedef CArrayTemplate<SIntVec4>		CArrayIntVec4;


int 		IntVec4Dot(const SIntVec4* pV1, const SIntVec4* pV2);
SIntVec4* 	IntVec4Add(SIntVec4* pOut, const SIntVec4* pV1, const SIntVec4* pV2);
SIntVec4* 	IntVec4Subtract(SIntVec4* pOut, const SIntVec4* pV1, const SIntVec4* pV2);
SIntVec4* 	IntVec4Minimize(SIntVec4* pOut, const SIntVec4* pV1, const SIntVec4* pV2);
SIntVec4* 	IntVec4Maximize(SIntVec4* pOut, const SIntVec4* pV1, const SIntVec4* pV2);
SIntVec4* 	IntVec4Scale(SIntVec4* pOut, const SIntVec4* pV, float32 s);
SIntVec4* 	IntVec4Lerp(SIntVec4* pOut, const SIntVec4* pV1, const SIntVec4* pV2, float32 s);
void		IntVec4MinMax(SIntVec4* psMin, SIntVec4* psMax, SIntVec4* asIn, int iInStride, int iNumPoints);
void		IntVec4Swap(SIntVec4* ps1, SIntVec4* ps2);
void 		IntVec4InterpolatePosition(SIntVec4* psVecDest, const SIntVec4* psVec1, const SIntVec4* psVec2, float32 fWeight);


#endif // __INT_VEC4_H__

