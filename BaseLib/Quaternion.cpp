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
SQuaternion& SQuaternion::operator *= (const SQuaternion& q)
{
	QuaternionMultiply(this, this, &q);
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
SQuaternion SQuaternion::operator * (const SQuaternion& q) const
{
	SQuaternion qT;
	QuaternionMultiply(&qT, this, &q);
	return qT;
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
BOOL SQuaternion::operator == (const SQuaternion& q) const
{
	return x == q.x && y == q.y && z == q.z && w == q.w;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SQuaternion::operator != (const SQuaternion& q) const
{
	return x != q.x || y != q.y || z != q.z || w != q.w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void QuaternionMultiply(SQuaternion* pOut, const SQuaternion* pQ1, const SQuaternion* pQ2)
{
	exit(1);

	//Let Q1 and Q2 be two quaternions, which are defined, respectively, as (x1, y1, z1, w1) and (x2, y2, z2, w2).
	//	(Q1 * Q2).w = (w1w2 - x1x2 - y1y2 - z1z2)
	//	(Q1 * Q2).x = (w1x2 + x1w2 + y1z2 - z1y2)
	//	(Q1 * Q2).y = (w1y2 - x1z2 + y1w2 + z1x2)
	//	(Q1 * Q2).z = (w1z2 + x1y2 - y1x2 + z1w2)
}
