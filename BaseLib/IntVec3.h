#ifndef __INT_VEC3_H__
#define __INT_VEC3_H__
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


class SIntVec3 : public SVec3<int, long long>
{
CONSTRUCTABLE(SIntVec3);
public:
	using SVec3<int, long long>::operator=;

	void 	Print(CChars* psx, int iWholeNumbers = -1);

	int		WholeNumbers(void);
};


typedef CArrayTemplate<SIntVec3>		CArrayIntVec3;


int 		Int32Vec3Dot(const SIntVec3* pV1, const SIntVec3* pV2);
SIntVec3*	Int32Vec3Cross(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2);
SIntVec3* 	Int32Vec3Add(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2);
SIntVec3* 	Int32Vec3Subtract(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2);
SIntVec3* 	Int32Vec3Minimize(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2);
SIntVec3* 	Int32Vec3Maximize(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2);
SIntVec3*	Int32Vec3Scale(SIntVec3* pOut, const SIntVec3* pV, float32 s);
SIntVec3*	Int32Vec3Lerp(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2, float32 s);
void		Int32Vec3MinMax(SIntVec3* psMin, SIntVec3* psMax, SIntVec3* asIn, int iInStride, int iNumPoints);
void		Int32Vec3Swap(SIntVec3* ps1, SIntVec3* ps2);
void 		Int32Vec3InterpolatePosition(SIntVec3* psVecDest, const SIntVec3* psVec1, const SIntVec3* psVec3, float32 fWeight);


#endif // __INT_VEC3_H__

