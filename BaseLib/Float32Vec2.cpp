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
#include "Numbers.h"
#include "IntegerHelper.h"
#include "FloatHelper.h"
#include "Float32Vec4.h"
#include "Float4x4.h"
#include "Float32Vec3.h"
#include "Float32Vec2.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat32Vec2::Fix(void)
{
	x = RoundFloat(x, BINARY_PRECISION);
	y = RoundFloat(y, BINARY_PRECISION);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SFloat32Vec2::WholeNumbers(void)
{
	int i[2];

	i[0] = FloatWholeNumbers(x);
	i[1] = FloatWholeNumbers(y);

	return LargestInt(i, 2);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat32Vec2::Print(CChars* psx, int iWholeNumbers, int iDecimals)
{
	CChars		szzz;
	int			iWidth;

	if (iWholeNumbers == -1)
	{
		iWholeNumbers = WholeNumbers();
		iWholeNumbers++;
		if (iWholeNumbers < 2)
		{
			iWholeNumbers = 2;
		}
	}

	iWidth = iWholeNumbers + 1 + iDecimals;

	psx->Append('[');
	szzz.Init();
	szzz.Append(x, iDecimals);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(',');
	szzz.Init();
	szzz.Append(y, iDecimals);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat32Vec2::Normalize(void)
{
	float32 f;

	f = Magnitude();
	if (f != 0.0f)
	{
		f = 1.0f / f;
		x *= f;
		y *= f;
	}
	else
	{
		x = y = 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Vec2::CloselyEqual(SFloat32Vec2* ps)
{
	if ((FloatEqual(x, ps->x)) &&
		(FloatEqual(y, ps->y)))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Vec2::CloselyEqual(SFloat32Vec2* ps, float32 fTolerance)
{
	if ((FloatEqual(x, ps->x, fTolerance)) &&
		(FloatEqual(y, ps->y, fTolerance)))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float32 Float32Vec2Dot(const SFloat32Vec2* pV1, const SFloat32Vec2* pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float32 Float32Vec2Cross(const SFloat32Vec2* pV1, const SFloat32Vec2* pV2)
{
	return pV1->x * pV2->y - pV1->y * pV2->x;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec2* Float32Vec2Add(SFloat32Vec2 *pOut, const SFloat32Vec2* pV1, const SFloat32Vec2* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec2* Float32Vec2Subtract(SFloat32Vec2 *pOut, const SFloat32Vec2* pV1, const SFloat32Vec2* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec2* Float32Vec2Minimize(SFloat32Vec2 *pOut, const SFloat32Vec2* pV1, const SFloat32Vec2* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec2* Float32Vec2Maximize(SFloat32Vec2 *pOut, const SFloat32Vec2* pV1, const SFloat32Vec2* pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec2* Float32Vec2Scale(SFloat32Vec2 *pOut, const SFloat32Vec2 *pV, float32 s)
{
	pOut->x = pV->x * s;
	pOut->y = pV->y * s;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec2* Float32Vec2Lerp(SFloat32Vec2 *pOut, const SFloat32Vec2* pV1, const SFloat32Vec2* pV2, float32 s)
{
	pOut->x = pV1->x + s * (pV2->x - pV1->x);
	pOut->y = pV1->y + s * (pV2->y - pV1->y);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec2* Float32Vec2TransformCoord(SFloat32Vec2 *pOut, SFloat32Vec2* pV, SFloat4x4* psMat)
{
	pOut->x = pV->x * psMat->x.x + pV->y * psMat->y.x + 1.0f * psMat->pos.x;
	pOut->y = pV->x * psMat->x.y + pV->y * psMat->y.y + 1.0f * psMat->pos.y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec2* Float32Vec2TransformNormal(SFloat32Vec2 *pOut, SFloat32Vec2* pV, SFloat4x4* psMat)
{
	pOut->x = pV->x * psMat->x.x + pV->y * psMat->y.x;
	pOut->y = pV->x * psMat->x.y + pV->y * psMat->y.y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec2TransformCoords(SFloat32Vec2* asOut, int iOutStride, SFloat32Vec2* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints)
{
	int			i;
	SFloat32Vec2*	psOut;
	SFloat32Vec2*	psIn;

	for (i = 0; i < iNumPoints; i++)
	{
		psOut = (SFloat32Vec2*)RemapSinglePointer(asOut, i * iOutStride);
		psIn = (SFloat32Vec2*)RemapSinglePointer(asIn, i * iInStride);
		Float32Vec2TransformCoord(psOut, psIn, psMat);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec2TransformNormals(SFloat32Vec2* asOut, int iOutStride, SFloat32Vec2* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints)
{
	int			i;
	SFloat32Vec2*	psOut;
	SFloat32Vec2*	psIn;

	for (i = 0; i < iNumPoints; i++)
	{
		psOut = (SFloat32Vec2*)RemapSinglePointer(asOut, i * iOutStride);
		psIn = (SFloat32Vec2*)RemapSinglePointer(asIn, i * iInStride);
		Float32Vec2TransformNormal(psOut, psIn, psMat);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec2Swap(SFloat32Vec2* ps1, SFloat32Vec2* ps2)
{
	SFloat32Vec2 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec2MinMax(SFloat32Vec2* psMin, SFloat32Vec2* psMax, SFloat32Vec2* asIn, int iInStride, int iNumPoints)
{
	int			i;
	SFloat32Vec2*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SFloat32Vec2*)RemapSinglePointer(asIn, i * iInStride);

			if (psIn->x < psMin->x)
			{
				psMin->x = psIn->x;
			}
			if (psIn->y < psMin->y)
			{
				psMin->y = psIn->y;
			}

			if (psIn->x > psMax->x)
			{
				psMax->x = psIn->x;
			}
			if (psIn->y > psMax->y)
			{
				psMax->y = psIn->y;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec2Normalise(SFloat32Vec2* pOut, SFloat32Vec2* pV)
{
	float32 fInvLen;

	fInvLen = 1.0f / pV->Magnitude();
	pOut->x = pV->x * fInvLen;
	pOut->y = pV->y * fInvLen;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec2InterpolatePosition(SFloat32Vec2* psVecDest, const SFloat32Vec2* psVec1, const SFloat32Vec2* psVec2, float32 fWeight)
{
	SFloat32Vec2		sVec1;
	SFloat32Vec2		sVec2;

	Float32Vec2Scale(&sVec1, psVec1, fWeight);
	Float32Vec2Scale(&sVec2, psVec2, 1.0f - fWeight);
	Float32Vec2Add(psVecDest, &sVec1, &sVec2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec2InterpolateNormal(SFloat32Vec2* psVecDest, const SFloat32Vec2* psVec1, const SFloat32Vec2* psVec2, float32 fWeight)
{
	SFloat32Vec2		sVec1;
	SFloat32Vec2		sVec2;

	Float32Vec2Scale(&sVec1, psVec1, fWeight);
	Float32Vec2Scale(&sVec2, psVec2, 1.0f - fWeight);
	Float32Vec2Add(&sVec1, &sVec1, &sVec2);

	//Should possibly check for zero length vectors.
	Float32Vec2Normalise(psVecDest, &sVec1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec2RotateAboutNormalisedAxis(SFloat32Vec3* psVecDest, SFloat32Vec3 p, float32 fTheta, SFloat32Vec2 r)
{
	float32 fCosTheta;
	float32 fSinTheta;
	float32 fInvCosTheta;

	psVecDest->Zero();
	fCosTheta = cosf(fTheta);
	fSinTheta = sinf(fTheta);
	fInvCosTheta = 1.0f - fCosTheta;

	psVecDest->x += (fCosTheta + fInvCosTheta * r.x * r.x) * p.x;
	psVecDest->x += (fInvCosTheta * r.x * r.y ) * p.y;
	psVecDest->x += (r.y * fSinTheta) * p.z;

	psVecDest->y += (fInvCosTheta * r.x * r.y ) * p.x;
	psVecDest->y += (fCosTheta + fInvCosTheta * r.y * r.y) * p.y;
	psVecDest->y += (-r.x * fSinTheta) * p.z;

	psVecDest->z += (-r.y * fSinTheta) * p.x;
	psVecDest->z += (r.x * fSinTheta) * p.y;
	psVecDest->z += (fCosTheta) * p.z;
}

