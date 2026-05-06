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
#ifndef __INT32_VEC2_H__
#define __INT32_VEC2_H__
#include "Vec2.h"


class SIntVec2  : public SVec2<int, long long>
{
CONSTRUCTABLE(SIntVec2);
public:
	using SVec2<int, long long>::operator=;

	void 	Print(CChars* psx, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};


typedef CArrayTemplate<SIntVec2>		CArrayIntVec2;


int 		IntVec2Dot(const SIntVec2* pV1, const SIntVec2* pV2);
int			IntVec2Cross(const SIntVec2* pV1, const SIntVec2* pV2);
SIntVec2* 	IntVec2Add(SIntVec2* pOut, const SIntVec2* pV1, const SIntVec2* pV2);
SIntVec2* 	IntVec2Subtract(SIntVec2* pOut, const SIntVec2* pV1, const SIntVec2* pV2);
SIntVec2* 	IntVec2Minimize(SIntVec2* pOut, const SIntVec2* pV1, const SIntVec2* pV2);
SIntVec2* 	IntVec2Maximize(SIntVec2* pOut, const SIntVec2* pV1, const SIntVec2* pV2);
SIntVec2* 	IntVec2Scale(SIntVec2* pOut, const SIntVec2* pV, float32 s);
SIntVec2* 	IntVec2Lerp(SIntVec2* pOut, const SIntVec2* pV1, const SIntVec2* pV2, float32 s);
void		IntVec2MinMax(SIntVec2* psMin, SIntVec2* psMax, SIntVec2* asIn, int iInStride, int iNumPoints);
void		IntVec2Swap(SIntVec2* ps1, SIntVec2* ps2);
void 		IntVec2InterpolatePosition(SIntVec2* psVecDest, const SIntVec2* psVec1, const SIntVec2* psVec2, float32 fWeight);


#endif // __INT32_VEC2_H__

