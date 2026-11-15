/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/FloatHelper.h"
#include "RevolvedShape.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRevolvedShape::Init(SFloat32Vec3* psAxis, SFloat32Vec3* psStart, float32 fLength)
{
	PreInit();
	CShape::Init();
	msStart = *psStart;
	Float32Vec3Normalise(&msAxis, psAxis);
	mfLength = fLength;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRevolvedShape::Class(void)
{
	CShape::Class();
	U_3Float32(msStart);
	U_3Float32(msAxis);
	U_Float32(mfLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRevolvedShape::Free(void)
{
	CShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CRevolvedShape::Distance(SFloat32Vec3* psPosition)
{
	float32			fRadial;
	float32			fAlong;
	SFloat32Vec3	sOutward;
	SFloat32Vec2	sGradient;

	ToProfile(psPosition, &fRadial, &fAlong, &sOutward);
	return ProfileDistance(fRadial, fAlong, &sGradient);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRevolvedShape::Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	float32			fRadial;
	float32			fAlong;
	SFloat32Vec3	sOutward;
	SFloat32Vec2	sGradient;

	ToProfile(psPosition, &fRadial, &fAlong, &sOutward);
	ProfileDistance(fRadial, fAlong, &sGradient);
	*psDest = sOutward * sGradient.x + msAxis * sGradient.y;
}


//////////////////////////////////////////////////////////////////////////
//
// psOutward is the unit from the axis towards the point, or any unit across
// the axis for a point on it.
//
//////////////////////////////////////////////////////////////////////////
void CRevolvedShape::ToProfile(SFloat32Vec3* psPosition, float32* pfRadial, float32* pfAlong, SFloat32Vec3* psOutward)
{
	SFloat32Vec3	sOffset;
	SFloat32Vec3	sAcross;

	Float32Vec3Subtract(&sOffset, psPosition, &msStart);
	*pfAlong = Float32Vec3Dot(&sOffset, &msAxis);
	*psOutward = sOffset - msAxis * *pfAlong;
	*pfRadial = psOutward->Magnitude();
	if (*pfRadial > 0.0f)
	{
		*psOutward /= *pfRadial;
	}
	else
	{
		sAcross.Init(1, 0, 0);
		if (fabsf(msAxis.x) > 0.9f)
		{
			sAcross.Init(0, 1, 0);
		}
		Float32Vec3Cross(psOutward, &msAxis, &sAcross);
		psOutward->Normalize();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The bounds of a disc across the axis, fAlong it, of fRadius.
//
//////////////////////////////////////////////////////////////////////////
void CRevolvedShape::AddDisc(SFloat32Bounds* psBounds, float32 fAlong, float32 fRadius)
{
	SFloat32Vec3	sCenter;
	SFloat32Vec3	sReach;
	SFloat32Vec3	sCorner;

	sCenter = msStart + msAxis * fAlong;
	sReach.Init(fRadius * SquareRoot(maxf(0.0f, 1.0f - msAxis.x * msAxis.x)),
				fRadius * SquareRoot(maxf(0.0f, 1.0f - msAxis.y * msAxis.y)),
				fRadius * SquareRoot(maxf(0.0f, 1.0f - msAxis.z * msAxis.z)));
	sCorner = sCenter - sReach;
	psBounds->Add(&sCorner);
	sCorner = sCenter + sReach;
	psBounds->Add(&sCorner);
}


//////////////////////////////////////////////////////////////////////////
//
// A rectangle centred on the origin of the profile's plane, exact.  Its
// gradient points from its nearest point outside, and out through its
// nearest side inside or on it.
//
//////////////////////////////////////////////////////////////////////////
float32 CRevolvedShape::BoxProfileDistance(float32 x, float32 y, float32 fHalfWidth, float32 fHalfHeight, SFloat32Vec2* psGradient)
{
	float32		fBeyondX;
	float32		fBeyondY;
	float32		fSignX;
	float32		fSignY;
	float32		fOutX;
	float32		fOutY;

	fBeyondX = fabsf(x) - fHalfWidth;
	fBeyondY = fabsf(y) - fHalfHeight;
	fSignX = (x < 0.0f) ? -1.0f : 1.0f;
	fSignY = (y < 0.0f) ? -1.0f : 1.0f;
	if ((fBeyondX > 0.0f) || (fBeyondY > 0.0f))
	{
		fOutX = maxf(fBeyondX, 0.0f);
		fOutY = maxf(fBeyondY, 0.0f);
		psGradient->Init(fOutX * fSignX, fOutY * fSignY);
		psGradient->Normalize();
		return SquareRoot(fOutX * fOutX + fOutY * fOutY);
	}

	if (fBeyondX >= fBeyondY)
	{
		psGradient->Init(fSignX, 0.0f);
		return fBeyondX;
	}
	psGradient->Init(0.0f, fSignY);
	return fBeyondY;
}


//////////////////////////////////////////////////////////////////////////
//
// From the origin of the profile's plane, exact.  Its gradient points away
// from the origin, or straight out from the axis at the origin, where every
// way is away.
//
//////////////////////////////////////////////////////////////////////////
float32 CRevolvedShape::PointProfileDistance(float32 x, float32 y, SFloat32Vec2* psGradient)
{
	float32		fLength;

	fLength = SquareRoot(x * x + y * y);
	if (fLength > 0.0f)
	{
		psGradient->Init(x / fLength, y / fLength);
	}
	else
	{
		psGradient->Init(1.0f, 0.0f);
	}
	return fLength;
}
