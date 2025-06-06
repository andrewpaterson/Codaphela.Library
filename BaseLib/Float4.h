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
#ifndef __FLOAT_4_H__
#define __FLOAT_4_H__
#include "Define.h"
#include "Constructable.h"
#include "ChunkFile.h"


class SFloat2;
class SFloat3;
class SFloat4x4;
class SFloat4
{
CONSTRUCTABLE(SFloat4);
public:
	float x;
	float y;
	float z;
	float w;

	SFloat4() {};
	SFloat4(const float*);
	SFloat4(const SFloat3& v, float f);
	SFloat4(float x, float y, float z, float w);

	SFloat4& operator += (const SFloat4&);
	SFloat4& operator -= (const SFloat4&);
	SFloat4& operator *= (float);
	SFloat4& operator /= (float);

	SFloat4 operator + () const;
	SFloat4 operator - () const;

	SFloat4 operator + (const SFloat4&) const;
	SFloat4 operator - (const SFloat4&) const;
	SFloat4 operator * (float) const;
	SFloat4 operator / (float) const;

	friend SFloat4 operator * (float, const SFloat4&);

	bool operator == (const SFloat4&) const;
	bool operator != (const SFloat4&) const;	

	void 	Init(float x, float y, float z, float w);
	void 	Zero(void);

	bool 	Save(CFileWriter* pcFile);
	bool 	Load(CFileReader* pcFile);

	void 	Fix(void);
	int		WholeNumbers(void);
	void	Print(CChars* psz, int iWholeNumbers = -1, int iDecimals = 2);
	void	Dump(void);
};


typedef CArrayTemplate<SFloat4>		CArrayFloat4;


float 		Float4Length(const SFloat4* pV);
float 		Float4LengthSq(const SFloat4* pV);
float 		Float4Dot(const SFloat4* pV1, const SFloat4* pV2);
SFloat4* 	Float4Add(SFloat4* pOut, const SFloat4* pV1, const SFloat4* pV2);
SFloat4* 	Float4Subtract(SFloat4* pOut, const SFloat4* pV1, const SFloat4* pV2);
SFloat4* 	Float4Minimize(SFloat4* pOut, const SFloat4* pV1, const SFloat4* pV2);
SFloat4* 	Float4Maximize(SFloat4* pOut, const SFloat4* pV1, const SFloat4* pV2);
SFloat4* 	Float4Scale(SFloat4* pOut, const SFloat4* pV, float s);
SFloat4* 	Float4Lerp(SFloat4* pOut, const SFloat4* pV1, const SFloat4* pV2, float s);
SFloat4*    Float4Transform(SFloat4* psOut, SFloat4x4* psMat, SFloat4* psV);


#endif // __FLOAT_4_H__

