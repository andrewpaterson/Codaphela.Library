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
#include "DataIO.h"
#include "Float64Vec2.h"
#include "Float64Vec3.h"
#include "Double4x4.h"
#include "Float64Vec4.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec4::SFloat64Vec4(float64 x, float64 y, float64 z, float64 w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat64Vec4::Normalize(void)
{
	float64 f;

	f = Magnitude();
	if (f != 0.0f)
	{
		f = 1.0f/f;
		x *= f;
		y *= f;
		z *= f;
		w *= f;
	}
	else
	{
		x = y = z = w = 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat64Vec4::CloselyEqual(SFloat64Vec4* ps)
{
	if ((FloatEqual(x, ps->x)) &&
		(FloatEqual(y, ps->y)) &&
		(FloatEqual(z, ps->z)) &&
		(FloatEqual(w, ps->w)))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat64Vec4::CloselyEqual(SFloat64Vec4* ps, float64 fTolerance)
{
	if ((FloatEqual(x, ps->x, fTolerance)) &&
		(FloatEqual(y, ps->y, fTolerance)) &&
		(FloatEqual(z, ps->z, fTolerance)) &&
		(FloatEqual(w, ps->w, fTolerance)))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat64Vec4::Copy(const SFloat64Vec4* ps)
{
	x = ps->x;
	y = ps->y;
	z = ps->z;
	w = ps->w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat64Vec4::Copy(const SFloat64Vec3* ps)
{
	x = ps->x;
	y = ps->y;
	z = ps->z;
	w = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat64Vec4::Copy(const SFloat64Vec2* ps)
{
	x = ps->x;
	y = ps->y;
	z = 0.0f;
	w = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat64Vec4::Fix(void)
{
	x = RoundFloat(x, (int64)BINARY_PRECISION);
	y = RoundFloat(y, (int64)BINARY_PRECISION);
	z = RoundFloat(z, (int64)BINARY_PRECISION);
	w = RoundFloat(w, (int64)BINARY_PRECISION);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SFloat64Vec4::WholeNumbers(void)
{
	int i[4];

	i[0] = FloatWholeNumbers(x);
	i[1] = FloatWholeNumbers(y);
	i[2] = FloatWholeNumbers(z);
	i[3] = FloatWholeNumbers(w);

	return LargestInt(i, 4);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat64Vec4::Print(CChars* psz, int iWholeNumbers, int iDecimals)
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
	
	psz->Append('[');
	szzz.Init();
	szzz.Append(x, iDecimals);
	szzz.RightAlign(' ', iWidth);
	psz->Append(szzz);
	psz->Append(',');
	szzz.Clear();
	szzz.Append(y, iDecimals);
	szzz.RightAlign(' ', iWidth);
	psz->Append(szzz);
	psz->Append(',');
	szzz.Clear();
	szzz.Append(z, iDecimals);
	szzz.RightAlign(' ', iWidth);
	psz->Append(szzz);
	psz->Append(',');
	szzz.Clear();
	szzz.Append(w, iDecimals);
	szzz.RightAlign(' ', iWidth);
	psz->Append(szzz);
	psz->Append(']');
	szzz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 Float64Vec4Dot(const SFloat64Vec4 *pV1, const SFloat64Vec4 *pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec4* Float64Vec4Add(SFloat64Vec4 *pOut, const SFloat64Vec4 *pV1, const SFloat64Vec4 *pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;
	pOut->w = pV1->w + pV2->w;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec4* Float64Vec4Subtract(SFloat64Vec4 *pOut, const SFloat64Vec4 *pV1, const SFloat64Vec4 *pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
	pOut->w = pV1->w - pV2->w;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec4* Float64Vec4Minimize(SFloat64Vec4 *pOut, const SFloat64Vec4 *pV1, const SFloat64Vec4 *pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z < pV2->z ? pV1->z : pV2->z;
	pOut->w = pV1->w < pV2->w ? pV1->w : pV2->w;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec4* Float64Vec4Maximize(SFloat64Vec4 *pOut, const SFloat64Vec4 *pV1, const SFloat64Vec4 *pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z > pV2->z ? pV1->z : pV2->z;
	pOut->w = pV1->w > pV2->w ? pV1->w : pV2->w;
	return pOut;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec4* Float64Vec4Scale(SFloat64Vec4 *pOut, const SFloat64Vec4 *pV, float64 s)
{
	pOut->x = pV->x * s;
	pOut->y = pV->y * s;
	pOut->z = pV->z * s;
	pOut->w = pV->w * s;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec4* Float64Vec4Lerp(SFloat64Vec4 *pOut, const SFloat64Vec4 *pV1, const SFloat64Vec4 *pV2, float64 s)
{
	pOut->x = pV1->x + s * (pV2->x - pV1->x);
	pOut->y = pV1->y + s * (pV2->y - pV1->y);
	pOut->z = pV1->z + s * (pV2->z - pV1->z);
	pOut->w = pV1->w + s * (pV2->w - pV1->w);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec4* Float64Vec4TransformCoord(SFloat64Vec4* psOut, SFloat64Vec4* psV, SDouble4x4* psMat)
{
	psOut->x = psV->x * psMat->x.x + psV->y * psMat->y.x + psV->z * psMat->z.x + psV->w * psMat->pos.x;
	psOut->y = psV->x * psMat->x.y + psV->y * psMat->y.y + psV->z * psMat->z.y + psV->w * psMat->pos.y;
	psOut->z = psV->x * psMat->x.z + psV->y * psMat->y.z + psV->z * psMat->z.z + psV->w * psMat->pos.z;
	psOut->w = psV->x * psMat->x.w + psV->y * psMat->y.w + psV->z * psMat->z.w + psV->w * psMat->pos.w;
	return psOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec4* Float64Vec4TransformNormal(SFloat64Vec4* psOut, SFloat64Vec4* psV, SDouble4x4* psMat)
{
	psOut->x = psV->x * psMat->x.x + psV->y * psMat->y.x + psV->z * psMat->z.x;
	psOut->y = psV->x * psMat->x.y + psV->y * psMat->y.y + psV->z * psMat->z.y;
	psOut->z = psV->x * psMat->x.z + psV->y * psMat->y.z + psV->z * psMat->z.z;
	psOut->w = 0.0f;
	return psOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float64Vec4TransformCoords(SFloat64Vec4* asOut, int iOutStride, SFloat64Vec4* asIn, int iInStride, SDouble4x4* psMat, int iNumPoints)
{
	int				i;
	SFloat64Vec4*	psOut;
	SFloat64Vec4*	psIn;

	for (i = 0; i < iNumPoints; i++)
	{
		psOut = (SFloat64Vec4*)RemapSinglePointer(asOut, i * iOutStride);
		psIn = (SFloat64Vec4*)RemapSinglePointer(asIn, i * iInStride);
		Float64Vec4TransformCoord(psOut, psIn, psMat);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float64Vec4TransformNormals(SFloat64Vec4* asOut, int iOutStride, SFloat64Vec4* asIn, int iInStride, SDouble4x4* psMat, int iNumPoints)
{
	int			i;
	SFloat64Vec4*	psOut;
	SFloat64Vec4*	psIn;

	for (i = 0; i < iNumPoints; i++)
	{
		psOut = (SFloat64Vec4*)RemapSinglePointer(asOut, i * iOutStride);
		psIn = (SFloat64Vec4*)RemapSinglePointer(asIn, i * iInStride);
		Float64Vec4TransformNormal(psOut, psIn, psMat);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float64Vec4MinMax(SFloat64Vec4* psMin, SFloat64Vec4* psMax, SFloat64Vec4* asIn, int iInStride, int iNumPoints)
{
	int				i;
	SFloat64Vec4*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SFloat64Vec4*)RemapSinglePointer(asIn, i * iInStride);

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
			if (psIn->w < psMin->w)
			{
				psMin->w = psIn->w;
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
			if (psIn->w > psMax->w)
			{
				psMax->w = psIn->w;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float64Vec4Normalise(SFloat64Vec4 *pOut, SFloat64Vec4 *pV)
{
	float64 fInvLen;

	fInvLen = 1.0f / pV->Magnitude();
	pOut->x = pV->x * fInvLen;
	pOut->y = pV->y * fInvLen;
	pOut->z = pV->z * fInvLen;
	pOut->w = pV->w * fInvLen;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float64Vec4InterpolatePosition(SFloat64Vec4* psVecDest, const SFloat64Vec4* psVec1, const SFloat64Vec4* psVec2, float64 fWeight)
{
	SFloat64Vec4	sVec1;
	SFloat64Vec4	sVec2;

	Float64Vec4Scale(&sVec1, psVec1, fWeight);
	Float64Vec4Scale(&sVec2, psVec2, 1.0f - fWeight);
	Float64Vec4Add(psVecDest, &sVec1, &sVec2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float64Vec4InterpolateNormal(SFloat64Vec4* psVecDest, const SFloat64Vec4* psVec1, const SFloat64Vec4* psVec2, float64 fWeight)
{
	SFloat64Vec4	sVec1;
	SFloat64Vec4	sVec2;

	Float64Vec4Scale(&sVec1, psVec1, fWeight);
	Float64Vec4Scale(&sVec2, psVec2, 1.0f - fWeight);
	Float64Vec4Add(&sVec1, &sVec1, &sVec2);

	//Should possibly check for zero length vectors.
	Float64Vec4Normalise(psVecDest, &sVec1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float64Vec4Swap(SFloat64Vec4* ps1, SFloat64Vec4* ps2)
{
	SFloat64Vec4 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}

