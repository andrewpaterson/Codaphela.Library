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
#include "Float32Vec2.h"
#include "Float32Vec4.h"
#include "Float4x4.h"
#include "Float32Vec3.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3::SFloat32Vec3(float32 x, float32 y, float32 z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat32Vec3::Normalize(void)
{
	float32 f;

	f = Magnitude();
	if (f != 0.0f)
	{
		f = 1.0f/f;
		x *= f;
		y *= f;
		z *= f;
	}
	else
	{
		x = y = z = 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Vec3::CloselyEqual(SFloat32Vec3* ps)
{
	if ((FloatEqual(x, ps->x)) &&
		(FloatEqual(y, ps->y)) &&
		(FloatEqual(z, ps->z)))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Vec3::CloselyEqual(SFloat32Vec3* ps, float32 fTolerance)
{
	if ((FloatEqual(x, ps->x, fTolerance)) &&
		(FloatEqual(y, ps->y, fTolerance)) &&
		(FloatEqual(z, ps->z, fTolerance)))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat32Vec3::Copy(const SFloat32Vec4* ps)
{
	x = ps->x;
	y = ps->y;
	z = ps->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat32Vec3::Copy(const SFloat32Vec3* ps)
{
	x = ps->x;
	y = ps->y;
	z = ps->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat32Vec3::Copy(const SFloat32Vec2* ps)
{
	x = ps->x;
	y = ps->y;
	z = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat32Vec3::Fix(void)
{
	x = RoundFloat(x, BINARY_PRECISION);
	y = RoundFloat(y, BINARY_PRECISION);
	z = RoundFloat(z, BINARY_PRECISION);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SFloat32Vec3::WholeNumbers(void)
{
	int i[3];

	i[0] = FloatWholeNumbers(x);
	i[1] = FloatWholeNumbers(y);
	i[2] = FloatWholeNumbers(z);

	return LargestInt(i, 3);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat32Vec3::Print(CChars* psx, int iWholeNumbers, int iDecimals)
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
	psx->Append(',');
	szzz.Init();
	szzz.Append(z, iDecimals);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 Float32Vec3Dot(const SFloat32Vec3* pV1, const SFloat32Vec3* pV2)
{
	return pV1->x*  pV2->x + pV1->y*  pV2->y + pV1->z*  pV2->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* Float32Vec3Cross(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2)
{
	SFloat32Vec3 v;

	v.x = pV1->y * pV2->z - pV1->z * pV2->y;
	v.y = pV1->z * pV2->x - pV1->x * pV2->z;
	v.z = pV1->x * pV2->y - pV1->y * pV2->x;

	*pOut = v;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* Float32Vec3Add(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* Float32Vec3Subtract(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* Float32Vec3Minimize(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z < pV2->z ? pV1->z : pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* Float32Vec3Maximize(SFloat32Vec3* pOut, const SFloat32Vec3* pV1, const SFloat32Vec3* pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z > pV2->z ? pV1->z : pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* Float32Vec3Scale(SFloat32Vec3 *pOut, const SFloat32Vec3 *pV, float32 s)
{
	pOut->x = pV->x * s;
	pOut->y = pV->y * s;
	pOut->z = pV->z * s;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* Float32Vec3Lerp(SFloat32Vec3 *pOut, const SFloat32Vec3 *pV1, const SFloat32Vec3 *pV2, float32 s)
{
	pOut->x = pV1->x + s * (pV2->x - pV1->x);
	pOut->y = pV1->y + s * (pV2->y - pV1->y);
	pOut->z = pV1->z + s * (pV2->z - pV1->z);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* Float32Vec3TransformCoord(SFloat32Vec3 *pOut, SFloat32Vec3* pV, SFloat4x4* psMat)
{
	pOut->x = pV->x * psMat->x.x + pV->y * psMat->y.x + pV->z * psMat->z.x + 1.0f * psMat->pos.x;
	pOut->y = pV->x * psMat->x.y + pV->y * psMat->y.y + pV->z * psMat->z.y + 1.0f * psMat->pos.y;
	pOut->z = pV->x * psMat->x.z + pV->y * psMat->y.z + pV->z * psMat->z.z + 1.0f * psMat->pos.z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* Float32Vec3TransformNormal(SFloat32Vec3 *pOut, SFloat32Vec3* pV, SFloat4x4* psMat)
{
	pOut->x = pV->x * psMat->x.x + pV->y * psMat->y.x + pV->z * psMat->z.x;
	pOut->y = pV->x * psMat->x.y + pV->y * psMat->y.y + pV->z * psMat->z.y;
	pOut->z = pV->x * psMat->x.z + pV->y * psMat->y.z + pV->z * psMat->z.z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec3TransformCoords(SFloat32Vec3* asOut, int iOutStride, SFloat32Vec3* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints)
{
	int			i;
	SFloat32Vec3*	psOut;
	SFloat32Vec3*	psIn;

	for (i = 0; i < iNumPoints; i++)
	{
		psOut = (SFloat32Vec3*)RemapSinglePointer(asOut, i * iOutStride);
		psIn = (SFloat32Vec3*)RemapSinglePointer(asIn, i * iInStride);
		Float32Vec3TransformCoord(psOut, psIn, psMat);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec3TransformNormals(SFloat32Vec3* asOut, int iOutStride, SFloat32Vec3* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints)
{
	int			i;
	SFloat32Vec3*	psOut;
	SFloat32Vec3*	psIn;

	for (i = 0; i < iNumPoints; i++)
	{
		psOut = (SFloat32Vec3*)RemapSinglePointer(asOut, i * iOutStride);
		psIn = (SFloat32Vec3*)RemapSinglePointer(asIn, i * iInStride);
		Float32Vec3TransformNormal(psOut, psIn, psMat);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec3Swap(SFloat32Vec3* ps1, SFloat32Vec3* ps2)
{
	SFloat32Vec3 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec3MinMax(SFloat32Vec3* psMin, SFloat32Vec3* psMax, SFloat32Vec3* asIn, int iInStride, int iNumPoints)
{
	int			i;
	SFloat32Vec3*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SFloat32Vec3*)RemapSinglePointer(asIn, i * iInStride);

			if (psIn->x < psMin->x)
			{
				psMin->x = psIn->x;
			}
			if (psIn->y < psMin->y)
			{
				psMin->y = psIn->y;
			}
			if (psIn->z < psMin->z)
			{
				psMin->z = psIn->z;
			}

			if (psIn->x > psMax->x)
			{
				psMax->x = psIn->x;
			}
			if (psIn->y > psMax->y)
			{
				psMax->y = psIn->y;
			}
			if (psIn->z > psMax->z)
			{
				psMax->z = psIn->z;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec3Normalise(SFloat32Vec3 *pOut, SFloat32Vec3 *pV)
{
	float32 fInvLen;

	fInvLen = 1.0f / pV->Magnitude();
	pOut->x = pV->x * fInvLen;
	pOut->y = pV->y * fInvLen;
	pOut->z = pV->z * fInvLen;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec3InterpolatePosition(SFloat32Vec3* psVecDest, const SFloat32Vec3* psVec1, const SFloat32Vec3* psVec2, float32 fWeight)
{
	SFloat32Vec3	sVec1;
	SFloat32Vec3	sVec2;

	Float32Vec3Scale(&sVec1, psVec1, fWeight);
	Float32Vec3Scale(&sVec2, psVec2, 1.0f - fWeight);
	Float32Vec3Add(psVecDest, &sVec1, &sVec2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec3InterpolateNormal(SFloat32Vec3* psVecDest, const SFloat32Vec3* psVec1, const SFloat32Vec3* psVec2, float32 fWeight)
{
	SFloat32Vec3	sVec1;
	SFloat32Vec3	sVec2;

	Float32Vec3Scale(&sVec1, psVec1, fWeight);
	Float32Vec3Scale(&sVec2, psVec2, 1.0f - fWeight);
	Float32Vec3Add(&sVec1, &sVec1, &sVec2);

	//Should possibly check for zero length vectors.
	Float32Vec3Normalise(psVecDest, &sVec1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float32Vec3RotateAboutNormalisedAxis(SFloat32Vec3* psVecDest, SFloat32Vec3 p, float32 fTheta, SFloat32Vec3 r)
{
	float32		fCosTheta;
	float32		fSinTheta;
	float32		fInvCosTheta;

	psVecDest->Zero();
	fCosTheta = cosf(fTheta);
	fSinTheta = sinf(fTheta);
	fInvCosTheta = 1.0f - fCosTheta;

	psVecDest->x += (fCosTheta + fInvCosTheta * r.x * r.x) * p.x;
	psVecDest->x += (fInvCosTheta * r.x * r.y - r.z * fSinTheta) * p.y;
	psVecDest->x += (fInvCosTheta * r.x * r.z + r.y * fSinTheta) * p.z;

	psVecDest->y += (fInvCosTheta * r.x * r.y + r.z * fSinTheta) * p.x;
	psVecDest->y += (fCosTheta + fInvCosTheta * r.y * r.y) * p.y;
	psVecDest->y += (fInvCosTheta * r.y * r.z - r.x * fSinTheta) * p.z;

	psVecDest->z += (fInvCosTheta * r.x * r.z - r.y * fSinTheta) * p.x;
	psVecDest->z += (fInvCosTheta * r.y * r.z + r.x * fSinTheta) * p.y;
	psVecDest->z += (fCosTheta + fInvCosTheta * r.z * r.z) * p.z;
}

