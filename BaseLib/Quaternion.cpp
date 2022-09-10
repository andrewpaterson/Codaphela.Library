/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "Numbers.h"
#include "FloatHelper.h"
#include "Quaternion.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion::SQuaternion(const float* pf)
{
	x = pf[0];
	y = pf[1];
	z = pf[2];
	w = pf[3];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion::SQuaternion(float fx, float fy, float fz, float fw)
{
	x = fx;
	y = fy;
	z = fz;
	w = fw;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion& SQuaternion::operator += (const SQuaternion& q)
{
	x += q.x;
	y += q.y;
	z += q.z;
	w += q.w;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion& SQuaternion::operator -= (const SQuaternion& q)
{
	x -= q.x;
	y -= q.y;
	z -= q.z;
	w -= q.w;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion& SQuaternion::operator *= (float f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion& SQuaternion::operator /= (float f)
{
	float fInv = 1.0f / f;
	x *= fInv;
	y *= fInv;
	z *= fInv;
	w *= fInv;
	return *this;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion SQuaternion::operator + () const
{
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion SQuaternion::operator - () const
{
	return SQuaternion(-x, -y, -z, -w);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion SQuaternion::operator + (const SQuaternion& q) const
{
	return SQuaternion(x + q.x, y + q.y, z + q.z, w + q.w);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion SQuaternion::operator - (const SQuaternion& q) const
{
	return SQuaternion(x - q.x, y - q.y, z - q.z, w - q.w);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion SQuaternion::operator * (float f) const
{
	return SQuaternion(x * f, y * f, z * f, w * f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion SQuaternion::operator / (float f) const
{
	float fInv = 1.0f / f;
	return SQuaternion(x * fInv, y * fInv, z * fInv, w * fInv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion operator * (float f, const SQuaternion& q)
{
	return SQuaternion(f * q.x, f * q.y, f * q.z, f * q.w);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SQuaternion::operator == (const SQuaternion& q) const
{
	return x == q.x && y == q.y && z == q.z && w == q.w;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SQuaternion::operator != (const SQuaternion& q) const
{
	return x != q.x || y != q.y || z != q.z || w != q.w;
}

