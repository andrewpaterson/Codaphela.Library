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
#include <math.h>
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float QuaternionDot(SQuaternion* ps1, SQuaternion* ps2)
{
	return ps1->x * ps2->x + ps1->y * ps2->y + ps1->z * ps2->z + ps1->w * ps2->w;
}


//////////////////////////////////////////////////////////////////////////
//
// No rotation if psIn has no length.
//
//////////////////////////////////////////////////////////////////////////
SQuaternion* QuaternionNormalize(SQuaternion* psOut, SQuaternion* psIn)
{
	float64	fLength;

	fLength = sqrt((float64)psIn->x * psIn->x + (float64)psIn->y * psIn->y + (float64)psIn->z * psIn->z + (float64)psIn->w * psIn->w);
	if (fLength <= 0.0)
	{
		psOut->x = 0.0f;
		psOut->y = 0.0f;
		psOut->z = 0.0f;
		psOut->w = 1.0f;
		return psOut;
	}
	psOut->x = (float)(psIn->x / fLength);
	psOut->y = (float)(psIn->y / fLength);
	psOut->z = (float)(psIn->z / fLength);
	psOut->w = (float)(psIn->w / fLength);
	return psOut;
}


//////////////////////////////////////////////////////////////////////////
//
// q and -q are the same rotation, so psTo is taken whichever way is
// shorter.  Close together, blending them straight is as good and doesn't
// divide by almost nothing.
//
//////////////////////////////////////////////////////////////////////////
SQuaternion* QuaternionSlerp(SQuaternion* psOut, SQuaternion* psFrom, SQuaternion* psTo, float fT)
{
	float64		fDot;
	float64		fSign;
	float64		fAngle;
	float64		fSin;
	float64		fFrom;
	float64		fTo;
	SQuaternion	sBlend;

	fDot = (float64)psFrom->x * psTo->x + (float64)psFrom->y * psTo->y + (float64)psFrom->z * psTo->z + (float64)psFrom->w * psTo->w;
	fSign = 1.0;
	if (fDot < 0.0)
	{
		fDot = -fDot;
		fSign = -1.0;
	}

	if (fDot > 0.9995)
	{
		fFrom = 1.0 - fT;
		fTo = fT;
	}
	else
	{
		fAngle = acos(fDot);
		fSin = sin(fAngle);
		fFrom = sin((1.0 - fT) * fAngle) / fSin;
		fTo = sin(fT * fAngle) / fSin;
	}
	fTo *= fSign;

	sBlend.x = (float)(fFrom * psFrom->x + fTo * psTo->x);
	sBlend.y = (float)(fFrom * psFrom->y + fTo * psTo->y);
	sBlend.z = (float)(fFrom * psFrom->z + fTo * psTo->z);
	sBlend.w = (float)(fFrom * psFrom->w + fTo * psTo->w);
	return QuaternionNormalize(psOut, &sBlend);
}
