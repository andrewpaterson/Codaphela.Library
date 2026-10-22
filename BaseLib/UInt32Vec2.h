#ifndef __UINT32_VEC2_H__
#define __UINT32_VEC2_H__
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


class SUInt32Vec2  : public SVec2<uint32, uint64>
{
CONSTRUCTABLE(SUInt32Vec2);
public:
	using SVec2<uint32, uint64>::SVec2;
	using SVec2<uint32, uint64>::operator=;

	void 	Print(CChars* psx, uint32 iWholeNumbers = -1);

	uint32	WholeNumbers(void);
};


typedef CArrayTemplate<SUInt32Vec2>		CArrayUInt32Vec2;


SUInt32Vec2* 	UInt32Vec2Add(SUInt32Vec2* pOut, const SUInt32Vec2* pV1, const SUInt32Vec2* pV2);
SUInt32Vec2* 	UInt32Vec2Subtract(SUInt32Vec2* pOut, const SUInt32Vec2* pV1, const SUInt32Vec2* pV2);
SUInt32Vec2* 	UInt32Vec2Minimize(SUInt32Vec2* pOut, const SUInt32Vec2* pV1, const SUInt32Vec2* pV2);
SUInt32Vec2* 	UInt32Vec2Maximize(SUInt32Vec2* pOut, const SUInt32Vec2* pV1, const SUInt32Vec2* pV2);
SUInt32Vec2* 	UInt32Vec2Scale(SUInt32Vec2* pOut, const SUInt32Vec2* pV, float32 s);
SUInt32Vec2* 	UInt32Vec2Lerp(SUInt32Vec2* pOut, const SUInt32Vec2* pV1, const SUInt32Vec2* pV2, float32 s);
void		UInt32Vec2MinMax(SUInt32Vec2* psMin, SUInt32Vec2* psMax, SUInt32Vec2* asIn, uint32 iInStride, uint32 iNumPoints);
void		UInt32Vec2Swap(SUInt32Vec2* ps1, SUInt32Vec2* ps2);
void 		UInt32Vec2InterpolatePosition(SUInt32Vec2* psVecDest, const SUInt32Vec2* psVec1, const SUInt32Vec2* psVec2, float32 fWeight);


#endif // __UINT32_VEC2_H__

