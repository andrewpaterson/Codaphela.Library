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
#ifndef __DOUBLE_2_H__
#define __DOUBLE_2_H__
#include "Define.h"
#include "ChunkFile.h"


//You should really inline the Double2 bit of this function and move it next to Deg2Rad.
double	Deg2Dot(double fDegrees);


class SDouble3;
class SDouble2
{
CONSTRUCTABLE(SDouble2);
public:
	double x, y;

	SDouble2() {};
	SDouble2(const double *);
	SDouble2(double x, double y);

	SDouble2& operator += (const SDouble2&);
	SDouble2& operator -= (const SDouble2&);
	SDouble2& operator *= (double);
	SDouble2& operator /= (double);

	SDouble2 operator + () const;
	SDouble2 operator - () const;

	SDouble2 operator + (const SDouble2&) const;
	SDouble2 operator - (const SDouble2&) const;
	SDouble2 operator * (double) const;
	SDouble2 operator / (double) const;

	friend SDouble2 operator * (double, const SDouble2&);

	BOOL operator == (const SDouble2&) const;
	BOOL operator != (const SDouble2&) const;

	void Init(double x, double y);
	void Init(int x, int y);
	void Zero(void);

	BOOL Save(CFileWriter* pcFile);
	BOOL Load(CFileReader* pcFile);

	void Fix(void);
	void Dump(void);
};


typedef CArrayTemplate<SDouble2>		CArrayDouble2;


void 		Double2Assign(SDouble2* psVecDest, double x, double y);
void 		Double2Assign(SDouble2* psVecDest, SDouble2* psVecSource);
double 		Double2Length(const SDouble2* pV);
double 		Double2LengthSq (const SDouble2* pV);
double 		Double2Dot(const SDouble2* pV1, const SDouble2* pV2);
double 		Double2CCW(const SDouble2* pV1, const SDouble2* pV2);
SDouble2* 	Double2Add(SDouble2* pOut, const SDouble2* pV1, const SDouble2* pV2);
SDouble2* 	Double2Subtract(SDouble2* pOut, const SDouble2* pV1, const SDouble2* pV2);
SDouble2* 	Double2Minimize(SDouble2* pOut, const SDouble2* pV1, const SDouble2* pV2);
SDouble2* 	Double2Maximize(SDouble2* pOut, const SDouble2* pV1, const SDouble2* pV2);
SDouble2* 	Double2Scale(SDouble2* pOut, const SDouble2* pV, double s);
SDouble2* 	Double2Lerp(SDouble2* pOut, const SDouble2* pV1, const SDouble2* pV2, double s);


#endif // !__DOUBLE_2_H__

