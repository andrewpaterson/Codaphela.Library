/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "Float32Vec4.h"


class SFloat32Vec2;
class SFloat32Vec3;
class SFloat32Vec4;
class SQuaternion;
class SFloat4x4
{
CONSTRUCTABLE(SFloat4x4);
public:
	SFloat32Vec4		x;
	SFloat32Vec4		y;
	SFloat32Vec4		z;
	SFloat32Vec4		pos;

	void 			Init(void);
	void 			Init(SFloat32Vec3* psX, SFloat32Vec3* psY, SFloat32Vec3* psZ, SFloat32Vec3* psPos);
	void			Zero(void);

	bool 			Save(CFileWriter* pcFileWriter);
	bool 			Load(CFileReader* pcFileReader);

	void 			Fix(void);
	void			Identity(void);
	bool 			Inverse(SFloat4x4* psOut, float* pfDeterminant);
	void 			Adjoint(SFloat4x4* psOut);
	float			Determinant(void);
	SFloat32Vec3*	At(void);
	SFloat32Vec3*	Up(void);
	SFloat32Vec3*	Across(void);
	SFloat32Vec3*	Pos(void);
	void			Print(CChars* psz, bool bOneLine, int iWholeNumbers = -1, int iDecimals = 2);
	void 			Dump(void);
};


typedef CArrayTemplate<SFloat4x4>		CArrayFloat4x4;


SFloat4x4*	Float4x4Identity(SFloat4x4* psOut);
SFloat4x4*	Float4x4Multiply(SFloat4x4* psOut, SFloat4x4* psLeft, SFloat4x4* psRight);
void		Float4x4RotationX(SFloat4x4* psOut, float fRad);
void		Float4x4RotationY(SFloat4x4* psOut, float fRad);
void		Float4x4RotationZ(SFloat4x4* psOut, float fRad);
void		Float4x4RotationVector(SFloat4x4* psOut, SFloat32Vec3* psV, float fRad);
void		Float4x4Translation(SFloat4x4* psOut, float x, float y, float z);
bool		Float4x4Inverse(SFloat4x4* psOut, float* pfDeterminant, SFloat4x4* psIn);
SFloat4x4* 	Float4x4LookAtRH(SFloat4x4* psOut, const SFloat32Vec3* psEye, const SFloat32Vec3* psAt, const SFloat32Vec3* psUp);
SFloat4x4* 	Float4x4PerspectiveFovRH(SFloat4x4* psOut, float fFovY, float fAspect, float fZNear, float fZFar);
bool		Float4x4Equals(SFloat4x4* ps1, SFloat4x4* ps2);

//Rotations are row major with vectors on their left, as the rest: a matrix's x, y and z rows are where its x, y and z axes go.
//Float4x4Decompose splits a matrix into a scale along each axis, a rotation and a position, which Float4x4Compose puts back
//together.  A mirrored matrix has a negative z scale, so its rotation is still a rotation.  The rows are made at right angles
//to those before them, so a shear is lost.
void		Float4x4RotationQuaternion(SFloat4x4* psOut, SQuaternion* psRotation);
void		QuaternionRotationMatrix(SQuaternion* psOut, SFloat4x4* psRotation);
void		Float4x4Decompose(SFloat32Vec3* psScale, SQuaternion* psRotation, SFloat32Vec3* psPosition, SFloat4x4* psIn);
void		Float4x4Compose(SFloat4x4* psOut, SFloat32Vec3* psScale, SQuaternion* psRotation, SFloat32Vec3* psPosition);


#endif // __FLOAT_4_X_4_H__

