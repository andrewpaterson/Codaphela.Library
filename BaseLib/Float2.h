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
#ifndef __FLOAT_2_H__
#define __FLOAT_2_H__
#include "Define.h"
#include "Constructable.h"
#include "ChunkFile.h"


//You should really inline the Float2 bit of this function and move it next to Deg2Rad.
float	Deg2Dot(float fDegrees);


class SFloat3;
class SFloat2
{
CONSTRUCTABLE(SFloat2);
public:
	float x, y;

	SFloat2() {};
	SFloat2(const float *);
	SFloat2(float x, float y);

	SFloat2& operator += (const SFloat2&);
	SFloat2& operator -= (const SFloat2&);
	SFloat2& operator *= (float);
	SFloat2& operator /= (float);

	SFloat2 operator + () const;
	SFloat2 operator - () const;

	SFloat2 operator + (const SFloat2&) const;
	SFloat2 operator - (const SFloat2&) const;
	SFloat2 operator * (float) const;
	SFloat2 operator / (float) const;

	friend SFloat2 operator * (float, const SFloat2&);

	bool operator == (const SFloat2&) const;
	bool operator != (const SFloat2&) const;

	void	Init(float x, float y);
	void	Init(int x, int y);
	void	Zero(void);

	bool	Save(CFileWriter* pcFile);
	bool	Load(CFileReader* pcFile);

	void	Fix(void);
	void	Dump(void);
};


typedef CArrayTemplate<SFloat2>		CArrayFloat2;


void 		Float2Assign(SFloat2* psVecDest, float x, float y);
void 		Float2Assign(SFloat2* psVecDest, SFloat2* psVecSource);
float 		Float2Length(const SFloat2* pV);
float 		Float2LengthSq (const SFloat2* pV);
float 		Float2Dot(const SFloat2* pV1, const SFloat2* pV2);
float 		Float2CCW(const SFloat2* pV1, const SFloat2* pV2);
SFloat2* 	Float2Add(SFloat2* pOut, const SFloat2* pV1, const SFloat2* pV2);
SFloat2* 	Float2Subtract(SFloat2* pOut, const SFloat2* pV1, const SFloat2* pV2);
SFloat2* 	Float2Minimize(SFloat2* pOut, const SFloat2* pV1, const SFloat2* pV2);
SFloat2* 	Float2Maximize(SFloat2* pOut, const SFloat2* pV1, const SFloat2* pV2);
SFloat2* 	Float2Scale(SFloat2* pOut, const SFloat2* pV, float s);
SFloat2* 	Float2Lerp(SFloat2* pOut, const SFloat2* pV1, const SFloat2* pV2, float s);


#endif // __FLOAT_2_H__

