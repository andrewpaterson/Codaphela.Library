/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "DataIO.h"
#include "Double2.h"
#include "Double4.h"
#include "Double4x4.h"
#include "Double3.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3::SDouble3(const double *pf)
{
	x = pf[0];
	y = pf[1];
	z = pf[2];
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3::SDouble3(const SDouble3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3::SDouble3(double fx, double fy, double fz)
{
	x = fx;
	y = fy;
	z = fz;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3& SDouble3::operator += (const SDouble3& v)
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
SDouble3& SDouble3::operator -= (const SDouble3& v)
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
SDouble3& SDouble3::operator *= (double f)
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
SDouble3& SDouble3::operator /= (double f)
{
	double fInv = 1.0f / f;
	x *= fInv;
	y *= fInv;
	z *= fInv;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3 SDouble3::operator + () const
{
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3 SDouble3::operator - () const
{
	return SDouble3(-x, -y, -z);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3 SDouble3::operator + (const SDouble3& v) const
{
	return SDouble3(x + v.x, y + v.y, z + v.z);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3 SDouble3::operator - (const SDouble3& v) const
{
	return SDouble3(x - v.x, y - v.y, z - v.z);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3 SDouble3::operator * (double f) const
{
	return SDouble3(x * f, y * f, z * f);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3 SDouble3::operator / (double f) const
{
	double fInv = 1.0f / f;
	return SDouble3(x * fInv, y * fInv, z * fInv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble3 operator * (double f, const SDouble3& v)
{
	return SDouble3(f * v.x, f * v.y, f * v.z);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SDouble3::operator == (const SDouble3& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SDouble3::operator != (const SDouble3& v) const
{
	return x != v.x || y != v.y || z != v.z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble3::Init(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble3::Zero(void)
{
	x = y = z = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble3::Add(SDouble3* ps)
{
	x += ps->x;
	y += ps->y;
	z += ps->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
double SDouble3::Magnitude(void)
{
	return sqrt(SquareMagnitude());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
double SDouble3::SquareMagnitude(void)
{
	return x*x + y*y + z*z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble3::Normalize(void)
{
	double f;

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
bool SDouble3::CloselyEqual(SDouble3* ps)
{

	if ((DoubleEqual(x, ps->x)) &&
		(DoubleEqual(y, ps->y)) &&
		(DoubleEqual(z, ps->z)))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SDouble3::CloselyEqual(SDouble3* ps, double fTolerance)
{

	if ((DoubleEqual(x, ps->x, fTolerance)) &&
		(DoubleEqual(y, ps->y, fTolerance)) &&
		(DoubleEqual(z, ps->z, fTolerance)))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble3::Copy(const SDouble4* ps)
{
	x = ps->x;
	y = ps->y;
	z = ps->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble3::Copy(const SDouble3* ps)
{
	x = ps->x;
	y = ps->y;
	z = ps->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble3::Copy(const SDouble2* ps)
{
	x = ps->x;
	y = ps->y;
	z = 0.0f;
}




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SDouble3::Save(CFileWriter* pcFile)
{
	bool	bResult;

	bResult = pcFile->WriteFloat(x);
	bResult &= pcFile->WriteFloat(y);
	bResult &= pcFile->WriteFloat(z);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SDouble3::Load(CFileReader* pcFile)
{
	bool	bResult;

	bResult = pcFile->ReadFloat(&x);
	bResult &= pcFile->ReadFloat(&y);
	bResult &= pcFile->ReadFloat(&z);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble3::Fix(void)
{
	x = RoundDouble(x, BINARY_PRECISION);
	y = RoundDouble(y, BINARY_PRECISION);
	z = RoundDouble(z, BINARY_PRECISION);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SDouble3::WholeNumbers(void)
{
	int i[3];

	i[0] = DoubleWholeNumbers(x);
	i[1] = DoubleWholeNumbers(y);
	i[2] = DoubleWholeNumbers(z);

	return LargestInt(i, 3);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble3::Print(CChars* psx, int iWholeNumbers, int iDecimals)
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
void SDouble3::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz, 4);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
double Double3Length(const SDouble3* pV)
{
	return sqrt(pV->x*  pV->x + pV->y*  pV->y + pV->z*  pV->z);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
double Double3LengthSq(const SDouble3* pV)
{
	return pV->x*  pV->x + pV->y*  pV->y + pV->z*  pV->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
double Double3Dot(const SDouble3* pV1, const SDouble3* pV2)
{
	return pV1->x*  pV2->x + pV1->y*  pV2->y + pV1->z*  pV2->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDouble3* Double3Cross(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2)
{
	SDouble3 v;

	v.x = pV1->y * pV2->z - pV1->z * pV2->y;
	v.y = pV1->z * pV2->x - pV1->x * pV2->z;
	v.z = pV1->x * pV2->y - pV1->y * pV2->x;

	*pOut = v;
	return pOut;
}

SDouble3* Double3Add(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;
	return pOut;
}

SDouble3* Double3Subtract(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
	return pOut;
}

SDouble3* Double3Minimize(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z < pV2->z ? pV1->z : pV2->z;
	return pOut;
}

SDouble3* Double3Maximize(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2)
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
SDouble3* Double3Scale(SDouble3 *pOut, const SDouble3 *pV, double s)
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
SDouble3* Double3Lerp(SDouble3 *pOut, const SDouble3 *pV1, const SDouble3 *pV2, double s)
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
SDouble3* Double3TransformCoord(SDouble3 *pOut, SDouble3* pV, SDouble4x4* psMat)
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
SDouble3* Double3TransformNormal(SDouble3 *pOut, SDouble3* pV, SDouble4x4* psMat)
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
void Double3TransformCoords(SDouble3* asOut, int iOutStride, SDouble3* asIn, int iInStride, SDouble4x4* psMat, int iNumPoints)
{
	int			i;
	SDouble3*	psOut;
	SDouble3*	psIn;

	for (i = 0; i < iNumPoints; i++)
	{
		psOut = (SDouble3*)RemapSinglePointer(asOut, i * iOutStride);
		psIn = (SDouble3*)RemapSinglePointer(asIn, i * iInStride);
		Double3TransformCoord(psOut, psIn, psMat);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Double3TransformNormals(SDouble3* asOut, int iOutStride, SDouble3* asIn, int iInStride, SDouble4x4* psMat, int iNumPoints)
{
	int			i;
	SDouble3*	psOut;
	SDouble3*	psIn;

	for (i = 0; i < iNumPoints; i++)
	{
		psOut = (SDouble3*)RemapSinglePointer(asOut, i * iOutStride);
		psIn = (SDouble3*)RemapSinglePointer(asIn, i * iInStride);
		Double3TransformNormal(psOut, psIn, psMat);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Double3Swap(SDouble3* ps1, SDouble3* ps2)
{
	SDouble3 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Double3MinMax(SDouble3* psMin, SDouble3* psMax, SDouble3* asIn, int iInStride, int iNumPoints)
{
	int			i;
	SDouble3*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SDouble3*)RemapSinglePointer(asIn, i * iInStride);

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
void Double3Assign(SDouble3* psVecDest, double x, double y, double z)
{
	psVecDest->x = x;
	psVecDest->y = y;
	psVecDest->z = z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Double3Assign(SDouble3* psVecDest, SDouble3* psVecSource)
{
	psVecDest->x = psVecSource->x;
	psVecDest->y = psVecSource->y;
	psVecDest->z = psVecSource->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Double3Zero(SDouble3* psVecDest)
{
	psVecDest->x = psVecDest->y = psVecDest->z = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Double3Normalize(SDouble3 *pOut, const SDouble3 *pV)
{
	double fInvLen;

	fInvLen = 1.0f / Double3Length(pV);
	pOut->x = pV->x * fInvLen;
	pOut->y = pV->y * fInvLen;
	pOut->z = pV->z * fInvLen;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Double3InterpolatePosition(SDouble3* psVecDest, const SDouble3* psVec1, const SDouble3* psVec2, double fWeight)
{
	SDouble3		sVec1;
	SDouble3		sVec2;

	Double3Scale(&sVec1, psVec1, fWeight);
	Double3Scale(&sVec2, psVec2, 1.0f - fWeight);
	Double3Add(psVecDest, &sVec1, &sVec2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Double3InterpolateNormal(SDouble3* psVecDest, const SDouble3* psVec1, const SDouble3* psVec2, double fWeight)
{
	SDouble3		sVec1;
	SDouble3		sVec2;

	Double3Scale(&sVec1, psVec1, fWeight);
	Double3Scale(&sVec2, psVec2, 1.0f - fWeight);
	Double3Add(&sVec1, &sVec1, &sVec2);

	//Should possibly check for zero length vectors.
	Double3Normalize(psVecDest, &sVec1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Double3RotateAboutNormalisedAxis(SDouble3* psVecDest, SDouble3 p, double fTheta, SDouble3 r)
{
	double fCosTheta;
	double fSinTheta;
	double fInvCosTheta;

	Double3Zero(psVecDest);
	fCosTheta = cos(fTheta);
	fSinTheta = sin(fTheta);
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

