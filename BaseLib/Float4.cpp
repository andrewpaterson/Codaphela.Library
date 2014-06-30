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
#include "IntegerHelper.h"
#include "FloatHelper.h"
#include "Float2.h"
#include "Float3.h"
#include "Float4x4.h"
#include "Float4.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat4::SFloat4(const float *pf)
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
SFloat4::SFloat4(const SFloat3& v, float f)
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
SFloat4::SFloat4(float fx, float fy, float fz, float fw)
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
SFloat4& SFloat4::operator += (const SFloat4& v)
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
SFloat4& SFloat4::operator -= (const SFloat4& v)
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
SFloat4& SFloat4::operator *= (float f)
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
SFloat4& SFloat4::operator /= (float f)
{
	float fInv = 1.0f / f;
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
SFloat4 SFloat4::operator + () const
{
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat4 SFloat4::operator - () const
{
	return SFloat4(-x, -y, -z, -w);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat4 SFloat4::operator + (const SFloat4& v) const
{
	return SFloat4(x + v.x, y + v.y, z + v.z, w + v.w);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat4 SFloat4::operator - (const SFloat4& v) const
{
	return SFloat4(x - v.x, y - v.y, z - v.z, w - v.w);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat4 SFloat4::operator * (float f) const
{
	return SFloat4(x * f, y * f, z * f, w * f);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat4 SFloat4::operator / (float f) const
{
	float fInv = 1.0f / f;
	return SFloat4(x * fInv, y * fInv, z * fInv, w * fInv);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat4 operator * (float f, const SFloat4& v)
{
	return SFloat4(f * v.x, f * v.y, f * v.z, f * v.w);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SFloat4::operator == (const SFloat4& v) const
{
	return x == v.x && y == v.y && z == v.z && w == v.w;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SFloat4::operator != (const SFloat4& v) const
{
	return x != v.x || y != v.y || z != v.z || w != v.w;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4::Init(float x, float y, float z, float w)
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
void SFloat4::Zero(void)
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
BOOL SFloat4::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteFloat(x));
	ReturnOnFalse(pcFile->WriteFloat(y));
	ReturnOnFalse(pcFile->WriteFloat(z));
	ReturnOnFalse(pcFile->WriteFloat(w));
	return TRUE;

}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SFloat4::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadFloat(&x));
	ReturnOnFalse(pcFile->ReadFloat(&y));
	ReturnOnFalse(pcFile->ReadFloat(&z));
	ReturnOnFalse(pcFile->ReadFloat(&w));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4::Fix(void)
{
	x = RoundFloat(x, BINARY_PRECISION);
	y = RoundFloat(y, BINARY_PRECISION);
	z = RoundFloat(z, BINARY_PRECISION);
	w = RoundFloat(w, BINARY_PRECISION);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4::Dump(void)
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
int SFloat4::WholeNumbers(void)
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
void SFloat4::ToString(CChars* psz, int iWholeNumbers, int iDecimals)
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
float Float4Length(const SFloat4 *pV)
{
	return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w);
}

float Float4LengthSq(const SFloat4 *pV)
{
	return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w;
}

float Float4Dot(const SFloat4 *pV1, const SFloat4 *pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
}

SFloat4* Float4Add(SFloat4 *pOut, const SFloat4 *pV1, const SFloat4 *pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;
	pOut->w = pV1->w + pV2->w;
	return pOut;
}

SFloat4* Float4Subtract(SFloat4 *pOut, const SFloat4 *pV1, const SFloat4 *pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
	pOut->w = pV1->w - pV2->w;
	return pOut;
}

SFloat4* Float4Minimize(SFloat4 *pOut, const SFloat4 *pV1, const SFloat4 *pV2)
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
SFloat4* Float4Maximize(SFloat4 *pOut, const SFloat4 *pV1, const SFloat4 *pV2)
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
SFloat4* Float4Scale(SFloat4 *pOut, const SFloat4 *pV, float s)
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
SFloat4* Float4Lerp(SFloat4 *pOut, const SFloat4 *pV1, const SFloat4 *pV2, float s)
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
SFloat4* Float4Transform(SFloat4* psOut, SFloat4x4* psMat, SFloat4* psV)
{
	psOut->x = psV->x * psMat->x.x + psV->y * psMat->y.x + psV->z * psMat->z.x + psV->w * psMat->pos.x;
	psOut->y = psV->x * psMat->x.y + psV->y * psMat->y.y + psV->z * psMat->z.y + psV->w * psMat->pos.y;
	psOut->z = psV->x * psMat->x.z + psV->y * psMat->y.z + psV->z * psMat->z.z + psV->w * psMat->pos.z;
	psOut->w = psV->x * psMat->x.w + psV->y * psMat->y.w + psV->z * psMat->z.w + psV->w * psMat->pos.w;
	return psOut;
}

