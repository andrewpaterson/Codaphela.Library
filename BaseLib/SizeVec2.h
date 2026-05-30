#ifndef __SIZE_VEC2_H__
#define __SIZE_VEC2_H__
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


class SSizeVec2  : public SVec2<size, ulongint>
{
CONSTRUCTABLE(SSizeVec2);
public:
	using SVec2<size, ulongint>::operator=;

	void 	Print(CChars* psx, size iWholeNumbers = -1);

	size	WholeNumbers(void);
};


typedef CArrayTemplate<SSizeVec2>		CArraySizeVec2;


SSizeVec2* 	SizeVec2Add(SSizeVec2* pOut, const SSizeVec2* pV1, const SSizeVec2* pV2);
SSizeVec2* 	SizeVec2Subtract(SSizeVec2* pOut, const SSizeVec2* pV1, const SSizeVec2* pV2);
SSizeVec2* 	SizeVec2Minimize(SSizeVec2* pOut, const SSizeVec2* pV1, const SSizeVec2* pV2);
SSizeVec2* 	SizeVec2Maximize(SSizeVec2* pOut, const SSizeVec2* pV1, const SSizeVec2* pV2);
SSizeVec2* 	SizeVec2Scale(SSizeVec2* pOut, const SSizeVec2* pV, float32 s);
SSizeVec2* 	SizeVec2Lerp(SSizeVec2* pOut, const SSizeVec2* pV1, const SSizeVec2* pV2, float32 s);
void		SizeVec2MinMax(SSizeVec2* psMin, SSizeVec2* psMax, SSizeVec2* asIn, size iInStride, size iNumPoints);
void		SizeVec2Swap(SSizeVec2* ps1, SSizeVec2* ps2);
void 		SizeVec2InterpolatePosition(SSizeVec2* psVecDest, const SSizeVec2* psVec1, const SSizeVec2* psVec2, float32 fWeight);


#endif // __SIZE_VEC2_H__

