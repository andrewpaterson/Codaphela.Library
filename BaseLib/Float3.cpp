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
#include "Float2.h"
#include "Float4.h"
#include "Float4x4.h"
#include "Float3.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3::SFloat3(const float *pf)
{
	x = pf[0];
	y = pf[1];
	z = pf[2];
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3::SFloat3(const SFloat3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3::SFloat3(float fx, float fy, float fz)
{
	x = fx;
	y = fy;
	z = fz;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3& SFloat3::operator += (const SFloat3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3& SFloat3::operator -= (const SFloat3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3& SFloat3::operator *= (float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3& SFloat3::operator /= (float f)
{
	float fInv = 1.0f / f;
	x *= fInv;
	y *= fInv;
	z *= fInv;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3 SFloat3::operator + () const
{
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3 SFloat3::operator - () const
{
	return SFloat3(-x, -y, -z);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3 SFloat3::operator + (const SFloat3& v) const
{
	return SFloat3(x + v.x, y + v.y, z + v.z);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3 SFloat3::operator - (const SFloat3& v) const
{
	return SFloat3(x - v.x, y - v.y, z - v.z);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3 SFloat3::operator * (float f) const
{
	return SFloat3(x * f, y * f, z * f);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat3 SFloat3::operator / (float f) const
{
	float fInv = 1.0f / f;
	return SFloat3(x * fInv, y * fInv, z * fInv);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SFloat3::operator == (const SFloat3& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SFloat3::operator != (const SFloat3& v) const
{
	return x != v.x || y != v.y || z != v.z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat3::Init(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat3::Zero(void)
{
	x = y = z = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat3::Add(SFloat3* ps)
{
	x += ps->x;
	y += ps->y;
	z += ps->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float SFloat3::Magnitude(void)
{
	return sqrtf(SquareMagnitude());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float SFloat3::SquareMagnitude(void)
{
	return x*x + y*y + z*z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat3::Normalize(void)
{
	float f;

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
BOOL SFloat3::CloselyEqual(SFloat3* ps)
{

	if ((FloatEqual(x, ps->x)) &&
		(FloatEqual(y, ps->y)) &&
		(FloatEqual(z, ps->z)))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SFloat3::CloselyEqual(SFloat3* ps, float fTolerance)
{

	if ((FloatEqual(x, ps->x, fTolerance)) &&
		(FloatEqual(y, ps->y, fTolerance)) &&
		(FloatEqual(z, ps->z, fTolerance)))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat3::Copy(const SFloat4* ps)
{
	x = ps->x;
	y = ps->y;
	z = ps->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat3::Copy(const SFloat3* ps)
{
	x = ps->x;
	y = ps->y;
	z = ps->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat3::Copy(const SFloat2* ps)
{
	x = ps->x;
	y = ps->y;
	z = 0.0f;
}




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SFloat3::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteFloat(x));
	ReturnOnFalse(pcFile->WriteFloat(y));
	ReturnOnFalse(pcFile->WriteFloat(z));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SFloat3::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadFloat(&x));
	ReturnOnFalse(pcFile->ReadFloat(&y));
	ReturnOnFalse(pcFile->ReadFloat(&z));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat3::Fix(void)
{
	x = RoundFloat(x, BINARY_PRECISION);
	y = RoundFloat(y, BINARY_PRECISION);
	z = RoundFloat(z, BINARY_PRECISION);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SFloat3::WholeNumbers(void)
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
void SFloat3::ToString(CChars* psx, int iWholeNumbers, int iDecimals)
{
	CChars		sz;
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
	sz.Init();
	sz.Append(x, iDecimals);
	sz.RightAlign(' ', iWidth);
	psx->Append(sz);
	sz.Kill();
	psx->Append(',');
	sz.Init();
	sz.Append(y, iDecimals);
	sz.RightAlign(' ', iWidth);
	psx->Append(sz);
	sz.Kill();
	psx->Append(',');
	sz.Init();
	sz.Append(z, iDecimals);
	sz.RightAlign(' ', iWidth);
	psx->Append(sz);
	sz.Kill();
	psx->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat3::Dump(void)
{
	CChars	sz;

	sz.Init();
	ToString(&sz, 4);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float Float3Length(const SFloat3* pV)
{
	return sqrtf(pV->x*  pV->x + pV->y*  pV->y + pV->z*  pV->z);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float Float3LengthSq(const SFloat3* pV)
{
	return pV->x*  pV->x + pV->y*  pV->y + pV->z*  pV->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float Float3Dot(const SFloat3* pV1, const SFloat3* pV2)
{
	return pV1->x*  pV2->x + pV1->y*  pV2->y + pV1->z*  pV2->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* Float3Cross(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2)
{
	SFloat3 v;

	v.x = pV1->y * pV2->z - pV1->z * pV2->y;
	v.y = pV1->z * pV2->x - pV1->x * pV2->z;
	v.z = pV1->x * pV2->y - pV1->y * pV2->x;

	*pOut = v;
	return pOut;
}

SFloat3* Float3Add(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;
	return pOut;
}

SFloat3* Float3Subtract(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
	return pOut;
}

SFloat3* Float3Minimize(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z < pV2->z ? pV1->z : pV2->z;
	return pOut;
}

SFloat3* Float3Maximize(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2)
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
SFloat3* Float3Scale(SFloat3 *pOut, const SFloat3 *pV, float s)
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
SFloat3* Float3Lerp(SFloat3 *pOut, const SFloat3 *pV1, const SFloat3 *pV2, float s)
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
SFloat3* Float3TransformCoord(SFloat3 *pOut, SFloat3* pV, SFloat4x4* psMat)
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
SFloat3* Float3TransformNormal(SFloat3 *pOut, SFloat3* pV, SFloat4x4* psMat)
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
void Float3TransformCoords(SFloat3* asOut, int iOutStride, SFloat3* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints)
{
	int			i;
	SFloat3*	psOut;
	SFloat3*	psIn;

	for (i = 0; i < iNumPoints; i++)
	{
		psOut = (SFloat3*)RemapSinglePointer(asOut, i * iOutStride);
		psIn = (SFloat3*)RemapSinglePointer(asIn, i * iInStride);
		Float3TransformCoord(psOut, psIn, psMat);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float3TransformNormals(SFloat3* asOut, int iOutStride, SFloat3* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints)
{
	int			i;
	SFloat3*	psOut;
	SFloat3*	psIn;

	for (i = 0; i < iNumPoints; i++)
	{
		psOut = (SFloat3*)RemapSinglePointer(asOut, i * iOutStride);
		psIn = (SFloat3*)RemapSinglePointer(asIn, i * iInStride);
		Float3TransformNormal(psOut, psIn, psMat);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float3Swap(SFloat3* ps1, SFloat3* ps2)
{
	SFloat3 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float3MinMax(SFloat3* psMin, SFloat3* psMax, SFloat3* asIn, int iInStride, int iNumPoints)
{
	int			i;
	SFloat3*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SFloat3*)RemapSinglePointer(asIn, i * iInStride);

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
void Float3Assign(SFloat3* psVecDest, float x, float y, float z)
{
	psVecDest->x = x;
	psVecDest->y = y;
	psVecDest->z = z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float3Assign(SFloat3* psVecDest, SFloat3* psVecSource)
{
	psVecDest->x = psVecSource->x;
	psVecDest->y = psVecSource->y;
	psVecDest->z = psVecSource->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float3Zero(SFloat3* psVecDest)
{
	psVecDest->x = psVecDest->y = psVecDest->z = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float3Normalize(SFloat3 *pOut, const SFloat3 *pV)
{
	float fInvLen;

	fInvLen = 1.0f / Float3Length(pV);
	pOut->x = pV->x * fInvLen;
	pOut->y = pV->y * fInvLen;
	pOut->z = pV->z * fInvLen;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float3InterpolatePosition(SFloat3* psVecDest, const SFloat3* psVec1, const SFloat3* psVec2, float fWeight)
{
	SFloat3		sVec1;
	SFloat3		sVec2;

	Float3Scale(&sVec1, psVec1, fWeight);
	Float3Scale(&sVec2, psVec2, 1.0f - fWeight);
	Float3Add(psVecDest, &sVec1, &sVec2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float3InterpolateNormal(SFloat3* psVecDest, const SFloat3* psVec1, const SFloat3* psVec2, float fWeight)
{
	SFloat3		sVec1;
	SFloat3		sVec2;

	Float3Scale(&sVec1, psVec1, fWeight);
	Float3Scale(&sVec2, psVec2, 1.0f - fWeight);
	Float3Add(&sVec1, &sVec1, &sVec2);

	//Should possibly check for zero length vectors.
	Float3Normalize(psVecDest, &sVec1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float3RotateAboutNormalisedAxis(SFloat3* psVecDest, SFloat3 p, float fTheta, SFloat3 r)
{
	float fCosTheta;
	float fSinTheta;
	float fInvCosTheta;

	Float3Zero(psVecDest);
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

