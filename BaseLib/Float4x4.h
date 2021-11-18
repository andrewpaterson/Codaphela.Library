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
#ifndef __FLOAT_4_X_4_H__
#define __FLOAT_4_X_4_H__
#include "Define.h"
#include "ChunkFile.h"
#include "Float4.h"


class SFloat2;
class SFloat3;
class SFloat4;
class SFloat4x4
{
CONSTRUCTABLE(SFloat4x4);
public:
	SFloat4	x;
	SFloat4	y;
	SFloat4 z;
	SFloat4 pos;

	void 		Init(void);
	void 		Init(SFloat3* psX, SFloat3* psY, SFloat3* psZ, SFloat3* psPos);
	void		Zero(void);

	BOOL 		Save(CFileWriter* pcFile);
	BOOL 		Load(CFileReader* pcFile);

	void 		Fix(void);
	void		Identity(void);
	BOOL 		Inverse(SFloat4x4* psOut, float* pfDeterminant);
	void 		Adjoint(SFloat4x4* psOut);
	float		Determinant(void);
	SFloat3*	At(void);
	SFloat3*	Up(void);
	SFloat3*	Across(void);
	SFloat3*	Pos(void);
	void		ToString(CChars* psz, BOOL bOneLine, int iWholeNumbers = -1, int iDecimals = 2);
	void 		Dump(void);
};


typedef CArrayTemplate<SFloat4x4>		CArrayFloat4x4;


SFloat4x4*	Float4x4Identity(SFloat4x4* psOut);
SFloat4x4*	Float4x4Multiply(SFloat4x4* psOut, SFloat4x4* psLeft, SFloat4x4* psRight);
void		Float4x4RotationX(SFloat4x4* psOut, float fRad);
void		Float4x4RotationY(SFloat4x4* psOut, float fRad);
void		Float4x4RotationZ(SFloat4x4* psOut, float fRad);
void		Float4x4RotationVector(SFloat4x4* psOut, SFloat3* psV, float fRad);
void		Float4x4Translation(SFloat4x4* psOut, float x, float y, float z);
BOOL		Float4x4Inverse(SFloat4x4* psOut, float* pfDeterminant, SFloat4x4* psIn);
SFloat4x4* 	Float4x4LookAtRH(SFloat4x4* psOut, const SFloat3* psEye, const SFloat3* psAt, const SFloat3* psUp);
SFloat4x4* 	Float4x4PerspectiveFovRH(SFloat4x4* psOut, float fFovY, float fAspect, float fZNear, float fZFar);
BOOL		Float4x4Equals(SFloat4x4* ps1, SFloat4x4* ps2);


#endif // !__FLOAT_4_X_4_H__

