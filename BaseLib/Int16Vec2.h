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
#ifndef __INT16_VEC2_H__
#define __INT16_VEC2_H__
#include "Vec2.h"


class SInt16Vec2  : public SVec2<int16, int32>
{
CONSTRUCTABLE(SInt16Vec2);
public:
	using SVec2<int16, int32>::operator=;

	void 	Print(CChars* psx, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};


typedef CArrayTemplate<SInt16Vec2>		CArrayInt16Vec2;


int16 			Int16Vec2Dot(const SInt16Vec2* pV1, const SInt16Vec2* pV2);
int16			Int16Vec2Cross(const SInt16Vec2* pV1, const SInt16Vec2* pV2);
SInt16Vec2* 	Int16Vec2Add(SInt16Vec2* pOut, const SInt16Vec2* pV1, const SInt16Vec2* pV2);
SInt16Vec2* 	Int16Vec2Subtract(SInt16Vec2* pOut, const SInt16Vec2* pV1, const SInt16Vec2* pV2);
SInt16Vec2* 	Int16Vec2Minimize(SInt16Vec2* pOut, const SInt16Vec2* pV1, const SInt16Vec2* pV2);
SInt16Vec2* 	Int16Vec2Maximize(SInt16Vec2* pOut, const SInt16Vec2* pV1, const SInt16Vec2* pV2);
SInt16Vec2* 	Int16Vec2Scale(SInt16Vec2* pOut, const SInt16Vec2* pV, float32 s);
SInt16Vec2* 	Int16Vec2Lerp(SInt16Vec2* pOut, const SInt16Vec2* pV1, const SInt16Vec2* pV2, float32 s);
void			Int16Vec2MinMax(SInt16Vec2* psMin, SInt16Vec2* psMax, SInt16Vec2* asIn, int iInStride, int iNumPoints);
void			Int16Vec2Swap(SInt16Vec2* ps1, SInt16Vec2* ps2);
void 			Int16Vec2InterpolatePosition(SInt16Vec2* psVecDest, const SInt16Vec2* psVec1, const SInt16Vec2* psVec2, float32 fWeight);


#endif // __INT16_VEC2_H__

