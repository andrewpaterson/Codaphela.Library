/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
public:
	float x, y, z, w;

	SQuaternion() {}
	SQuaternion(const float *);
	SQuaternion(float x, float y, float z, float w);

	// assignment operators
	SQuaternion& operator += (const SQuaternion&);
	SQuaternion& operator -= (const SQuaternion&);
	SQuaternion& operator *= (const SQuaternion&);
	SQuaternion& operator *= (float);
	SQuaternion& operator /= (float);

	// unary operators
	SQuaternion  operator + () const;
	SQuaternion  operator - () const;

	// binary operators
	SQuaternion operator + (const SQuaternion&) const;
	SQuaternion operator - (const SQuaternion&) const;
	SQuaternion operator * (const SQuaternion&) const;
	SQuaternion operator * (float) const;
	SQuaternion operator / (float) const;

	friend SQuaternion operator * (float, const SQuaternion&);

	BOOL operator == (const SQuaternion&) const;
	BOOL operator != (const SQuaternion&) const;
};


void	QuaternionMultiply(SQuaternion* pOut, const SQuaternion* pQ1, const SQuaternion* pQ2);


#endif __QUATERNION_H__

