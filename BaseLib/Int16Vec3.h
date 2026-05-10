#ifndef __INT16_VEC3_H__
#define __INT16_VEC3_H__
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
#include "Vec3.h"


class SInt16Vec3 : public SVec3<int16, int32>
{
CONSTRUCTABLE(SInt16Vec3);
public:
	using SVec3<int16, int32>::operator=;

	void 	Print(CChars* psx, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};


typedef CArrayTemplate<SInt16Vec3>		CArrayInt16Vec3;


int16 			Int16Vec3Dot(const SInt16Vec3* pV1, const SInt16Vec3* pV2);
SInt16Vec3*		Int16Vec3Cross(SInt16Vec3* pOut, const SInt16Vec3* pV1, const SInt16Vec3* pV2);
SInt16Vec3* 	Int16Vec3Add(SInt16Vec3* pOut, const SInt16Vec3* pV1, const SInt16Vec3* pV2);
SInt16Vec3* 	Int16Vec3Subtract(SInt16Vec3* pOut, const SInt16Vec3* pV1, const SInt16Vec3* pV2);
SInt16Vec3* 	Int16Vec3Minimize(SInt16Vec3* pOut, const SInt16Vec3* pV1, const SInt16Vec3* pV2);
SInt16Vec3* 	Int16Vec3Maximize(SInt16Vec3* pOut, const SInt16Vec3* pV1, const SInt16Vec3* pV2);
SInt16Vec3*		Int16Vec3Scale(SInt16Vec3* pOut, const SInt16Vec3* pV, float32 s);
SInt16Vec3* 	Int16Vec3Lerp(SInt16Vec3* pOut, const SInt16Vec3* pV1, const SInt16Vec3* pV2, float32 s);
void			Int16Vec3MinMax(SInt16Vec3* psMin, SInt16Vec3* psMax, SInt16Vec3* asIn, int iInStride, int iNumPoints);
void			Int16Vec3Swap(SInt16Vec3* ps1, SInt16Vec3* ps2);
void 			Int16Vec3InterpolatePosition(SInt16Vec3* psVecDest, const SInt16Vec3* psVec1, const SInt16Vec3* psVec3, float32 fWeight);


#endif // __INT16_VEC3_H__
