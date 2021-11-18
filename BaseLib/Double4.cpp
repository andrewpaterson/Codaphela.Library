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
#include "IntegerHelper.h"
#include "FloatHelper.h"
#include "DataIO.h"
#include "Double2.h"
#include "Double3.h"
#include "Double4x4.h"
#include "Double4.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4::SDouble4(const double *pf)
{
	x = pf[0];
	y = pf[1];
	z = pf[2];
	w = pf[3];
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4::SDouble4(const SDouble3& v, double f)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = f;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4::SDouble4(double fx, double fy, double fz, double fw)
{
	x = fx;
	y = fy;
	z = fz;
	w = fw;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4& SDouble4::operator += (const SDouble4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4& SDouble4::operator -= (const SDouble4& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4& SDouble4::operator *= (double f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4& SDouble4::operator /= (double f)
{
	double fInv = 1.0f / f;
	x *= fInv;
	y *= fInv;
	z *= fInv;
	w *= fInv;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4 SDouble4::operator + () const
{
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4 SDouble4::operator - () const
{
	return SDouble4(-x, -y, -z, -w);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4 SDouble4::operator + (const SDouble4& v) const
{
	return SDouble4(x + v.x, y + v.y, z + v.z, w + v.w);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4 SDouble4::operator - (const SDouble4& v) const
{
	return SDouble4(x - v.x, y - v.y, z - v.z, w - v.w);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4 SDouble4::operator * (double f) const
{
	return SDouble4(x * f, y * f, z * f, w * f);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4 SDouble4::operator / (double f) const
{
	double fInv = 1.0f / f;
	return SDouble4(x * fInv, y * fInv, z * fInv, w * fInv);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4 operator * (double f, const SDouble4& v)
{
	return SDouble4(f * v.x, f * v.y, f * v.z, f * v.w);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SDouble4::operator == (const SDouble4& v) const
{
	return x == v.x && y == v.y && z == v.z && w == v.w;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SDouble4::operator != (const SDouble4& v) const
{
	return x != v.x || y != v.y || z != v.z || w != v.w;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4::Init(double x, double y, double z, double w)
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
void SDouble4::Zero(void)
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SDouble4::Save(CFileWriter* pcFile)
{
	BOOL	bResult;

	bResult = pcFile->WriteFloat(x);
	bResult &= pcFile->WriteFloat(y);
	bResult &= pcFile->WriteFloat(z);
	bResult &= pcFile->WriteFloat(w);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SDouble4::Load(CFileReader* pcFile)
{
	BOOL	bResult;

	bResult = pcFile->ReadFloat(&x);
	bResult &= pcFile->ReadFloat(&y);
	bResult &= pcFile->ReadFloat(&z);
	bResult &= pcFile->ReadFloat(&w);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4::Fix(void)
{
	x = RoundDouble(x, BINARY_PRECISION);
	y = RoundDouble(y, BINARY_PRECISION);
	z = RoundDouble(z, BINARY_PRECISION);
	w = RoundDouble(w, BINARY_PRECISION);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4::Dump(void)
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
int SDouble4::WholeNumbers(void)
{
	int i[4];

	i[0] = DoubleWholeNumbers(x);
	i[1] = DoubleWholeNumbers(y);
	i[2] = DoubleWholeNumbers(z);
	i[3] = DoubleWholeNumbers(w);

	return LargestInt(i, 4);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4::ToString(CChars* psz, int iWholeNumbers, int iDecimals)
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
	psz->Append('[');
	sz.Init();
	sz.Append(x, iDecimals);
	sz.RightAlign(' ', iWidth);
	psz->Append(sz);
	psz->Append(',');
	sz.Clear();
	sz.Append(y, iDecimals);
	sz.RightAlign(' ', iWidth);
	psz->Append(sz);
	psz->Append(',');
	sz.Clear();
	sz.Append(z, iDecimals);
	sz.RightAlign(' ', iWidth);
	psz->Append(sz);
	psz->Append(',');
	sz.Clear();
	sz.Append(w, iDecimals);
	sz.RightAlign(' ', iWidth);
	psz->Append(sz);
	psz->Append(']');
	sz.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
double Double4Length(const SDouble4 *pV)
{
	return sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w);
}

double Double4LengthSq(const SDouble4 *pV)
{
	return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w;
}

double Double4Dot(const SDouble4 *pV1, const SDouble4 *pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
}

SDouble4* Double4Add(SDouble4 *pOut, const SDouble4 *pV1, const SDouble4 *pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;
	pOut->w = pV1->w + pV2->w;
	return pOut;
}

SDouble4* Double4Subtract(SDouble4 *pOut, const SDouble4 *pV1, const SDouble4 *pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
	pOut->w = pV1->w - pV2->w;
	return pOut;
}

SDouble4* Double4Minimize(SDouble4 *pOut, const SDouble4 *pV1, const SDouble4 *pV2)
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
SDouble4* Double4Maximize(SDouble4 *pOut, const SDouble4 *pV1, const SDouble4 *pV2)
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
SDouble4* Double4Scale(SDouble4 *pOut, const SDouble4 *pV, double s)
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
SDouble4* Double4Lerp(SDouble4 *pOut, const SDouble4 *pV1, const SDouble4 *pV2, double s)
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
SDouble4* Double4Transform(SDouble4* psOut, SDouble4x4* psMat, SDouble4* psV)
{
	psOut->x = psV->x * psMat->x.x + psV->y * psMat->y.x + psV->z * psMat->z.x + psV->w * psMat->pos.x;
	psOut->y = psV->x * psMat->x.y + psV->y * psMat->y.y + psV->z * psMat->z.y + psV->w * psMat->pos.y;
	psOut->z = psV->x * psMat->x.z + psV->y * psMat->y.z + psV->z * psMat->z.z + psV->w * psMat->pos.z;
	psOut->w = psV->x * psMat->x.w + psV->y * psMat->y.w + psV->z * psMat->z.w + psV->w * psMat->pos.w;
	return psOut;
}

