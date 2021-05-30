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
#ifndef __DOUBLE_4_X_4_H__
#define __DOUBLE_4_X_4_H__
#include "Define.h"
#include "ChunkFile.h"
#include "Double4.h"


class SDouble2;
class SDouble3;
class SDouble4;
class SDouble4x4
{
CONSTRUCTABLE(SDouble4x4);
public:
	SDouble4	x;
	SDouble4	y;
	SDouble4 z;
	SDouble4 pos;

	void 		Init(void);
	void 		Init(SDouble3* psX, SDouble3* psY, SDouble3* psZ, SDouble3* psPos);
	void		Zero(void);
	BOOL 		Save(CFileWriter* pcFile);
	BOOL 		Load(CFileReader* pcFile);
	void 		Fix(void);
	void		Identity(void);
	BOOL 		Inverse(SDouble4x4* psOut, double* pfDeterminant);
	void 		Adjoint(SDouble4x4* psOut);
	double		Determinant(void);
	SDouble3*	At(void);
	SDouble3*	Up(void);
	SDouble3*	Across(void);
	SDouble3*	Pos(void);
	void		ToString(CChars* psz, BOOL bOneLine, int iWholeNumbers = -1, int iDecimals = 2);
	void 		Dump(void);
};


typedef CArrayTemplate<SDouble4x4>		CArrayDouble4x4;


SDouble4x4*	Double4x4Identity(SDouble4x4* psOut);
SDouble4x4*	Double4x4Multiply(SDouble4x4* psOut, SDouble4x4* psLeft, SDouble4x4* psRight);
void		Double4x4RotationX(SDouble4x4* psOut, double fRad);
void		Double4x4RotationY(SDouble4x4* psOut, double fRad);
void		Double4x4RotationZ(SDouble4x4* psOut, double fRad);
void		Double4x4RotationVector(SDouble4x4* psOut, SDouble3* psV, double fRad);
void		Double4x4Translation(SDouble4x4* psOut, double x, double y, double z);
BOOL		Double4x4Inverse(SDouble4x4* psOut, double* pfDeterminant, SDouble4x4* psIn);
SDouble4x4* Double4x4LookAtRH(SDouble4x4* psOut, const SDouble3* psEye, const SDouble3* psAt, const SDouble3* psUp);
SDouble4x4* Double4x4PerspectiveFovRH(SDouble4x4* psOut, double fFovY, double fAspect, double fZNear, double fZFar);
BOOL		Double4x4Equals(SDouble4x4* ps1, SDouble4x4* ps2);


#endif // __DOUBLE_4_X_4_H__

