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
#ifndef __DOUBLE_4_H__
#define __DOUBLE_4_H__
#include "Define.h"
#include "ChunkFile.h"


class SDouble2;
class SDouble3;
class SDouble4x4;
class SDouble4
{
CONSTRUCTABLE(SDouble4);
public:
	double x, y, z, w;

	SDouble4() {};
	SDouble4(const double*);
	SDouble4(const SDouble3& v, double f);
	SDouble4(double x, double y, double z, double w);

	SDouble4& operator += (const SDouble4&);
	SDouble4& operator -= (const SDouble4&);
	SDouble4& operator *= (double);
	SDouble4& operator /= (double);

	SDouble4 operator + () const;
	SDouble4 operator - () const;

	SDouble4 operator + (const SDouble4&) const;
	SDouble4 operator - (const SDouble4&) const;
	SDouble4 operator * (double) const;
	SDouble4 operator / (double) const;

	friend SDouble4 operator * (double, const SDouble4&);

	BOOL operator == (const SDouble4&) const;
	BOOL operator != (const SDouble4&) const;	

	void 	Init(double x, double y, double z, double w);
	void 	Zero(void);

	BOOL 	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);
	
	void 	Fix(void);
	int		WholeNumbers(void);
	void	Print(CChars* psz, int iWholeNumbers = -1, int iDecimals = 2);
	void	Dump(void);
};


typedef CArrayTemplate<SDouble4>		CArrayDouble4;


double 		Double4Length(const SDouble4* pV);
double 		Double4LengthSq(const SDouble4* pV);
double 		Double4Dot(const SDouble4* pV1, const SDouble4* pV2);
SDouble4* 	Double4Add(SDouble4* pOut, const SDouble4* pV1, const SDouble4* pV2);
SDouble4* 	Double4Subtract(SDouble4* pOut, const SDouble4* pV1, const SDouble4* pV2);
SDouble4* 	Double4Minimize(SDouble4* pOut, const SDouble4* pV1, const SDouble4* pV2);
SDouble4* 	Double4Maximize(SDouble4* pOut, const SDouble4* pV1, const SDouble4* pV2);
SDouble4* 	Double4Scale(SDouble4* pOut, const SDouble4* pV, double s);
SDouble4* 	Double4Lerp(SDouble4* pOut, const SDouble4* pV1, const SDouble4* pV2, double s);
SDouble4*   Double4Transform(SDouble4* psOut, SDouble4x4* psMat, SDouble4* psV);


#endif // !__DOUBLE_4_H__

