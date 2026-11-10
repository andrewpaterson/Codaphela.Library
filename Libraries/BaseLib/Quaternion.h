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
#ifndef __QUATERNION_H__
#define __QUATERNION_H__
#include "Define.h"
#include "ChunkFile.h"


class SQuaternion
{
CONSTRUCTABLE(SQuaternion);
public:
	float x, y, z, w;

	SQuaternion() {}
	SQuaternion(const float *);
	SQuaternion(float x, float y, float z, float w);

	// assignment operators
	SQuaternion& operator += (const SQuaternion&);
	SQuaternion& operator -= (const SQuaternion&);
	SQuaternion& operator *= (float);
	SQuaternion& operator /= (float);

	// unary operators
	SQuaternion  operator + () const;
	SQuaternion  operator - () const;

	// binary operators
	SQuaternion operator + (const SQuaternion&) const;
	SQuaternion operator - (const SQuaternion&) const;
	SQuaternion operator * (float) const;
	SQuaternion operator / (float) const;

	friend SQuaternion operator * (float, const SQuaternion&);

	bool operator == (const SQuaternion&) const;
	bool operator != (const SQuaternion&) const;
};


//A rotation is a unit quaternion (x, y, z, w): sin(a / 2) times its axis, and cos(a / 2), for a turn of a about the axis.
//QuaternionSlerp turns from psFrom to psTo the shorter way round, steadily: fT of 0 is psFrom and 1 is psTo.
float			QuaternionDot(SQuaternion* ps1, SQuaternion* ps2);
SQuaternion*	QuaternionNormalize(SQuaternion* psOut, SQuaternion* psIn);
SQuaternion*	QuaternionSlerp(SQuaternion* psOut, SQuaternion* psFrom, SQuaternion* psTo, float fT);



#endif // __QUATERNION_H__

