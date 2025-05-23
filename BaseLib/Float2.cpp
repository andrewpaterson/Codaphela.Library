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
#include "FloatHelper.h"
#include "DataIO.h"
#include "Float3.h"
#include "Float2.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2::SFloat2(const float *pf)
{
	x = pf[0];
	y = pf[1];
}

SFloat2::SFloat2(float fx, float fy)
{
	x = fx;
	y = fy;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2& SFloat2::operator += (const SFloat2& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2& SFloat2::operator -= (const SFloat2& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2& SFloat2::operator *= (float f)
{
	x *= f;
	y *= f;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2& SFloat2::operator /= (float f)
{
	float fInv = 1.0f / f;
	x *= fInv;
	y *= fInv;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2 SFloat2::operator + () const
{
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2 SFloat2::operator - () const
{
	return SFloat2(-x, -y);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2 SFloat2::operator + (const SFloat2& v) const
{
	return SFloat2(x + v.x, y + v.y);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2 SFloat2::operator - (const SFloat2& v) const
{
	return SFloat2(x - v.x, y - v.y);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2 SFloat2::operator * (float f) const
{
	return SFloat2(x * f, y * f);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2 SFloat2::operator / (float f) const
{
	float fInv = 1.0f / f;
	return SFloat2(x * fInv, y * fInv);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2 operator * (float f, const SFloat2& v)
{
	return SFloat2(f * v.x, f * v.y);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat2::operator == (const SFloat2& v) const
{
	return x == v.x && y == v.y;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat2::operator != (const SFloat2& v) const
{
	return x != v.x || y != v.y;
}




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat2::Init(float x, float y)
{
	this->x = x;
	this->y = y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat2::Init(int x, int y)
{
	this->x = (float)x;
	this->y = (float)y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat2::Zero(void)
{
	x = 0.0f;
	y = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat2::Save(CFileWriter* pcFile)
{
	bool	bResult;

	bResult = pcFile->WriteFloat(x);
	bResult &= pcFile->WriteFloat(y);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat2::Load(CFileReader* pcFile)
{
	bool	bResult;

	bResult = pcFile->ReadFloat(&x);
	bResult &= pcFile->ReadFloat(&y);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat2::Fix(void)
{
	x = RoundFloat(x, BINARY_PRECISION);
	y = RoundFloat(y, BINARY_PRECISION);
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat2::Dump(void)
{
	CChars		sz;
	CChars		sx;
	int			iWidth;
	int			iDecimals;

	iWidth = 6;
	iDecimals = iWidth-5;
	sx.Init();
	sx.Append('[');
	sz.Init();
	sz.Append(x, iDecimals);
	sz.RightAlign(' ', iWidth);
	sx.Append(sz);
	sz.Kill();
	sx.Append(',');
	sz.Init();
	sz.Append(y, iDecimals);
	sz.RightAlign(' ', iWidth);
	sx.Append(sz);
	sz.Kill();
	sx.Append((char*)"]\n");
	sx.Dump();
	sx.Kill();
}




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float Float2Length(const SFloat2 *pV)
{
	return sqrtf(pV->x * pV->x + pV->y * pV->y);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float Float2LengthSq (const SFloat2 *pV)
{
	return pV->x * pV->x + pV->y * pV->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float Float2Dot(const SFloat2 *pV1, const SFloat2 *pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float Float2CCW(const SFloat2 *pV1, const SFloat2 *pV2)
{
	return pV1->x * pV2->y - pV1->y * pV2->x;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2* Float2Add(SFloat2 *pOut, const SFloat2 *pV1, const SFloat2 *pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2* Float2Subtract(SFloat2 *pOut, const SFloat2 *pV1, const SFloat2 *pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2* Float2Minimize(SFloat2 *pOut, const SFloat2 *pV1, const SFloat2 *pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2* Float2Maximize(SFloat2 *pOut, const SFloat2 *pV1, const SFloat2 *pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2* Float2Scale(SFloat2 *pOut, const SFloat2 *pV, float s)
{
	pOut->x = pV->x * s;
	pOut->y = pV->y * s;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat2* Float2Lerp(SFloat2 *pOut, const SFloat2 *pV1, const SFloat2 *pV2, float s)
{
	pOut->x = pV1->x + s * (pV2->x - pV1->x);
	pOut->y = pV1->y + s * (pV2->y - pV1->y);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float2Assign(SFloat2* psVecDest, float x, float y)
{
	psVecDest->x = x;
	psVecDest->y = y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float2Assign(SFloat2* psVecDest, SFloat2* psVecSource)
{
	psVecDest->x = psVecSource->x;
	psVecDest->y = psVecSource->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float Deg2Dot(float fDegrees)
{
	SFloat2	s1;
	SFloat2	s2;
	float	fRad;

	s1.Init(1.0f, 0.0f);
	fRad = Deg2Rad(fDegrees);
	s2.Init(cosf(fRad), sinf(fRad));

	return Float2Dot(&s1, &s2);
}
