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
#include "Cone.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCone::Init(SFloat32Vec3* psAxis, SFloat32Vec3* psStart, float32 fTopRadius, float32 fBottomRadius, float32 fLength)
{
	PreInit();
	CRevolvedShape::Init(psAxis, psStart, fLength);
	mfTopRadius = fTopRadius;
	mfBottomRadius = fBottomRadius;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCone::Class(void)
{
	CRevolvedShape::Class();
	U_Float32(mfTopRadius);
	U_Float32(mfBottomRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCone::Free(void)
{
	CRevolvedShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// Measured from its middle, the nearer of its end on the point's side (sEnd)
// and its slanted side (sSide), each as the offset from its nearest point
// to the point.  It's inside if it's between the ends and inside the side.
// The gradient is that offset made a unit, pointing out, or on the surface,
// where the offset is nothing, the nearest one's normal.
//
//////////////////////////////////////////////////////////////////////////
float32 CCone::ProfileDistance(float32 fRadial, float32 fAlong, SFloat32Vec2* psGradient)
{
	float32			fHalf;
	float32			fY;
	float32			fSignY;
	float32			fEndRadius;
	float32			fT;
	float32			fSlope;
	float32			fEndSquared;
	float32			fSideSquared;
	float32			fSign;
	float32			fLength;
	SFloat32Vec2	sEnd;
	SFloat32Vec2	sSide;
	SFloat32Vec2	sTop;
	SFloat32Vec2	sSlant;
	SFloat32Vec2	sPointToTop;
	SFloat32Vec2*	psNearest;
	SFloat32Vec2	sNormal;

	fHalf = mfLength / 2.0f;
	fY = fAlong - fHalf;
	fSignY = (fY < 0.0f) ? -1.0f : 1.0f;
	fEndRadius = (fY < 0.0f) ? mfBottomRadius : mfTopRadius;
	sEnd.Init(fRadial - minf(fRadial, fEndRadius), fabsf(fY) - fHalf);

	sTop.Init(mfTopRadius, fHalf);
	sSlant.Init(mfTopRadius - mfBottomRadius, 2.0f * fHalf);
	sPointToTop.Init(sTop.x - fRadial, sTop.y - fY);
	fSlope = sSlant.x * sSlant.x + sSlant.y * sSlant.y;
	fT = (fSlope > 0.0f) ? minf(maxf((sPointToTop.x * sSlant.x + sPointToTop.y * sSlant.y) / fSlope, 0.0f), 1.0f) : 0.0f;
	sSide.Init(fRadial - sTop.x + sSlant.x * fT, fY - sTop.y + sSlant.y * fT);

	fSign = ((sSide.x < 0.0f) && (sEnd.y < 0.0f)) ? -1.0f : 1.0f;
	fEndSquared = sEnd.x * sEnd.x + sEnd.y * sEnd.y;
	fSideSquared = sSide.x * sSide.x + sSide.y * sSide.y;
	if (fEndSquared < fSideSquared)
	{
		sEnd.y *= fSignY;
		psNearest = &sEnd;
		sNormal.Init(0.0f, fSignY);
	}
	else
	{
		psNearest = &sSide;
		sNormal.Init(sSlant.y, -sSlant.x);
		sNormal.Normalize();
	}

	fLength = psNearest->Magnitude();
	if (fLength > 0.0f)
	{
		psGradient->Init(fSign * psNearest->x / fLength, fSign * psNearest->y / fLength);
	}
	else
	{
		*psGradient = sNormal;
	}
	return fSign * SquareRoot(minf(fEndSquared, fSideSquared));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCone::GetBounds(SFloat32Bounds* psDest)
{
	psDest->Init();
	AddDisc(psDest, 0.0f, mfBottomRadius);
	AddDisc(psDest, mfLength, mfTopRadius);
}
