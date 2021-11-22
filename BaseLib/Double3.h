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
#ifndef __DOUBLE_3_H__
#define __DOUBLE_3_H__
#include "Define.h"
#include "Constructable.h"
#include "DatasIO.h"
#include "ChunkFile.h"


class SDouble4;
class SDouble2;
class SDouble4x4;
class SDouble3
{
CONSTRUCTABLE(SDouble3);
public:
	double x, y, z;

	SDouble3() {};
	SDouble3(const double *);
	SDouble3(const SDouble3& v);
	SDouble3(double x, double y, double z);

	SDouble3& operator += (const SDouble3&);
	SDouble3& operator -= (const SDouble3&);
	SDouble3& operator *= (double);
	SDouble3& operator /= (double);

	SDouble3 operator + () const;
	SDouble3 operator - () const;

	SDouble3 operator + (const SDouble3&) const;
	SDouble3 operator - (const SDouble3&) const;
	SDouble3 operator * (double) const;
	SDouble3 operator / (double) const;

	friend SDouble3 operator * (double, const SDouble3&);

	BOOL operator == (const SDouble3&) const;
	BOOL operator != (const SDouble3&) const;

	void 	Init(double x, double y, double z);
	void 	Zero(void);

	BOOL 	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);

	void 	Fix(void);
	void 	Print(CChars* psx, int iWholeNumbers = -1, int iDecimals = 2);
	int		WholeNumbers(void);
	void	Dump(void);

	void	Add(SDouble3* ps);
	double 	Magnitude(void);
	double 	SquareMagnitude(void);
	void 	Normalize(void);
	BOOL 	CloselyEqual(SDouble3* ps);
	BOOL 	CloselyEqual(SDouble3* ps, double fTolerance);
	void 	Copy(const SDouble4* ps);
	void 	Copy(const SDouble3* ps);
	void 	Copy(const SDouble2* ps);
};


typedef CArrayTemplate<SDouble3>		CArrayDouble3;


double 		Double3Length(const SDouble3* pV);
double 		Double3LengthSq(const SDouble3* pV);
double 		Double3Dot(const SDouble3* pV1, const SDouble3* pV2);
SDouble3* 	Double3Cross(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2);
SDouble3* 	Double3Add(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2);
SDouble3* 	Double3Subtract(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2);
SDouble3* 	Double3Minimize(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2);
SDouble3* 	Double3Maximize(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2);
SDouble3* 	Double3Scale(SDouble3* pOut, const SDouble3* pV, double s);
SDouble3* 	Double3Lerp(SDouble3* pOut, const SDouble3* pV1, const SDouble3* pV2, double s);
SDouble3*	Double3TransformCoord(SDouble3* pOut, SDouble3* pV, SDouble4x4* psMat);
void		Double3TransformCoords(SDouble3* asOut, int iOutStride, SDouble3* asIn, int iInStride, SDouble4x4* psMat, int iNumPoints);
SDouble3*	Double3TransformNormal(SDouble3* pOut, SDouble3* pV, SDouble4x4* psMat);
void		Double3TransformNormals(SDouble3* asOut, int iOutStride, SDouble3* asIn, int iInStride, SDouble4x4* psMat, int iNumPoints);
void		Double3MinMax(SDouble3* psMin, SDouble3* psMax, SDouble3* asIn, int iInStride, int iNumPoints);
void		Double3Swap(SDouble3* ps1, SDouble3* ps2);
void 		Double3Assign(SDouble3* psVecDest, double x, double y, double z);
void 		Double3Assign(SDouble3* psVecDest, SDouble3* psVecSource);
void 		Double3Zero(SDouble3* psVecDest);
void 		Double3InterpolatePosition(SDouble3* psVecDest, const SDouble3* psVec1, const SDouble3* psVec2, double fWeight);
void 		Double3InterpolateNormal(SDouble3* psVecDest, const SDouble3* psVec1, const SDouble3* psVec2, double fWeight);
void 		Double3RotateAboutNormalisedAxis(SDouble3* psVecDest, SDouble3 p, double fTheta, SDouble3 r);
void 		Double3Normalize(SDouble3* pOut, const SDouble3* pV);


#endif // !__DOUBLE_3_H__

