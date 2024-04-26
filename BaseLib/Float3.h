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
#ifndef __FLOAT_3_H__
#define __FLOAT_3_H__
#include "Define.h"
#include "Constructable.h"
#include "ChunkFile.h"


class SFloat4;
class SFloat2;
class SFloat4x4;
class SFloat3
{
CONSTRUCTABLE(SFloat3);
public:
	float x, y, z;

	SFloat3() {};
	SFloat3(const float *);
	SFloat3(const SFloat3& v);
	SFloat3(float x, float y, float z);

	SFloat3& operator += (const SFloat3&);
	SFloat3& operator -= (const SFloat3&);
	SFloat3& operator *= (float);
	SFloat3& operator /= (float);

	SFloat3 operator + () const;
	SFloat3 operator - () const;

	SFloat3 operator + (const SFloat3&) const;
	SFloat3 operator - (const SFloat3&) const;
	SFloat3 operator * (float) const;
	SFloat3 operator / (float) const;

	friend SFloat3 operator * (float, const SFloat3&);

	bool operator == (const SFloat3&) const;
	bool operator != (const SFloat3&) const;

	void 	Init(float x, float y, float z);
	void 	Init(const SFloat3& v);
	void 	Zero(void);

	bool 	Save(CFileWriter* pcFile);
	bool 	Load(CFileReader* pcFile);

	void 	Fix(void);
	void 	Print(CChars* psx, int iWholeNumbers = -1, int iDecimals = 2);
	int		WholeNumbers(void);
	void	Dump(void);

	void	Add(SFloat3* ps);
	float 	Magnitude(void);
	float 	SquareMagnitude(void);
	void 	Normalize(void);
	bool 	CloselyEqual(SFloat3* ps);
	bool 	CloselyEqual(SFloat3* ps, float fTolerance);
	void 	Copy(const SFloat4* ps);
	void 	Copy(const SFloat3* ps);
	void 	Copy(const SFloat2* ps);
};


typedef CArrayTemplate<SFloat3>		CArrayFloat3;


float 		Float3Length(const SFloat3* pV);
float 		Float3LengthSq(const SFloat3* pV);
float 		Float3Dot(const SFloat3* pV1, const SFloat3* pV2);
SFloat3* 	Float3Cross(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2);
SFloat3* 	Float3Add(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2);
SFloat3* 	Float3Subtract(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2);
SFloat3* 	Float3Minimize(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2);
SFloat3* 	Float3Maximize(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2);
SFloat3* 	Float3Scale(SFloat3* pOut, const SFloat3* pV, float s);
SFloat3* 	Float3Lerp(SFloat3* pOut, const SFloat3* pV1, const SFloat3* pV2, float s);
SFloat3*	Float3TransformCoord(SFloat3* pOut, SFloat3* pV, SFloat4x4* psMat);
void		Float3TransformCoords(SFloat3* asOut, int iOutStride, SFloat3* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints);
SFloat3*	Float3TransformNormal(SFloat3* pOut, SFloat3* pV, SFloat4x4* psMat);
void		Float3TransformNormals(SFloat3* asOut, int iOutStride, SFloat3* asIn, int iInStride, SFloat4x4* psMat, int iNumPoints);
void		Float3MinMax(SFloat3* psMin, SFloat3* psMax, SFloat3* asIn, int iInStride, int iNumPoints);
void		Float3Swap(SFloat3* ps1, SFloat3* ps2);
void 		Float3Assign(SFloat3* psVecDest, float x, float y, float z);
void 		Float3Assign(SFloat3* psVecDest, SFloat3* psVecSource);
void 		Float3Zero(SFloat3* psVecDest);
void 		Float3InterpolatePosition(SFloat3* psVecDest, const SFloat3* psVec1, const SFloat3* psVec2, float fWeight);
void 		Float3InterpolateNormal(SFloat3* psVecDest, const SFloat3* psVec1, const SFloat3* psVec2, float fWeight);
void 		Float3RotateAboutNormalisedAxis(SFloat3* psVecDest, SFloat3 p, float fTheta, SFloat3 r);
void 		Float3Normalize(SFloat3* pOut, const SFloat3* pV);


#endif // __FLOAT_3_H__

