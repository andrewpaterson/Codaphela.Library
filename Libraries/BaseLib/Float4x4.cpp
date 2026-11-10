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
#include "PointerFunctions.h"
#include "IntegerHelper.h"
#include "Numbers.h"
#include "FloatHelper.h"
#include "DataIO.h"
#include "Float32Vec2.h"
#include "Float32Vec3.h"
#include "Float32Vec4.h"
#include "Quaternion.h"
#include "Float4x4.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4x4::Init(void)
{
	x.x = y.y = z.z = pos.w = 1.0f;
	x.y = x.z = x.w = y.x = y.z = y.w = z.x = z.y = z.w = pos.x = pos.y = pos.z = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4x4::Zero(void)
{
	x.x = y.y = z.z = pos.w = 0.0f;
	x.y = x.z = x.w = y.x = y.z = y.w = z.x = z.y = z.w = pos.x = pos.y = pos.z = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4x4::Identity(void)
{
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4x4::Init(SFloat32Vec3* psX, SFloat32Vec3* psY, SFloat32Vec3* psZ, SFloat32Vec3* psP)
{
	//Optimise me later!
	x.Init(psX->x, psX->y, psX->y, 0.0f);
	y.Init(psY->x, psY->y, psY->y, 0.0f);
	z.Init(psZ->x, psZ->y, psZ->y, 0.0f);
	pos.Init(psP->x, psP->y, psP->y, 0.0f);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4x4::Fix(void)
{
	x.Fix();
	y.Fix();
	z.Fix();
	pos.Fix();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat4x4::Save(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = x.Save(pcFileWriter);
	bResult |= y.Save(pcFileWriter);
	bResult |= z.Save(pcFileWriter);
	bResult |= pos.Save(pcFileWriter);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat4x4::Load(CFileReader* pcFileReader)
{
	bool	bResult;

	bResult = x.Load(pcFileReader);
	bResult |= y.Load(pcFileReader);
	bResult |= z.Load(pcFileReader);
	bResult |= pos.Load(pcFileReader);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4x4::Print(CChars* psz, bool bOneLine, int iWholeNumbers, int iDecimals)
{
	int32	i[4];

	if (bOneLine)
	{
		iWholeNumbers = 0;
	}
	else
	{
		i[0] = x.WholeNumbers();
		i[1] = y.WholeNumbers();
		i[2] = z.WholeNumbers();
		i[3] = pos.WholeNumbers();

		iWholeNumbers = LargestInt(i, 4);
	}

	x.Print(psz, iWholeNumbers, iDecimals);
	if (!bOneLine) psz->AppendNewLine();
	y.Print(psz, iWholeNumbers, iDecimals);
	if (!bOneLine) psz->AppendNewLine();
	z.Print(psz, iWholeNumbers, iDecimals);
	if (!bOneLine) psz->AppendNewLine();
	pos.Print(psz, iWholeNumbers, iDecimals);
	if (!bOneLine) psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4x4::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz, false);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SFloat4x4::Inverse(SFloat4x4* psOut, float* pfDeterminant)
{
	float		fDet;
	float		fDetInv;
	SFloat4x4	sOut;

	/* calculate the adjoint matrix */
	Adjoint(&sOut);

	/*  calculate the 4x4 determinant
	*  if the determinant is zero,
	*  then the inverse matrix is not unique.
	*/
	fDet = Determinant();

	if (fabsf(fDet) < SMALL_NUMBER)
	{
		return false;
	}

	/* scale the adjoint matrix to get the inverse */
	fDetInv = 1.0f / fDet;
	sOut.x *= fDetInv;
	sOut.y *=fDetInv;
	sOut.z *= fDetInv;
	sOut.pos *= fDetInv;

	*psOut = sOut;
	SafeAssign(pfDeterminant, fDet);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SFloat4x4::Adjoint(SFloat4x4* psOut)
{
	double a1, a2, a3, a4, b1, b2, b3, b4;
	double c1, c2, c3, c4, d1, d2, d3, d4;

	//assign to individual variable names to aid
	//selecting correct values
	a1 = x.x; b1 = x.y;
	c1 = x.z; d1 = x.w;

	a2 = y.x; b2 = y.y;
	c2 = y.z; d2 = y.w;

	a3 = z.x; b3 = z.y;
	c3 = z.z; d3 = z.w;

	a4 = pos.x; b4 = pos.y;
	c4 = pos.z; d4 = pos.w;

	//row column labeling reversed since we transpose rows & columns
	psOut->x.x  =   (float)det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4);
	psOut->y.x  = - (float)det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4);
	psOut->z.x  =   (float)det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4);
	psOut->pos.x = -(float)det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);

	psOut->x.y  = - (float)det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4);
	psOut->y.y  =   (float)det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4);
	psOut->z.y  = - (float)det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4);
	psOut->pos.y  = (float)det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4);

	psOut->x.z  =   (float)det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4);
	psOut->y.z  = - (float)det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4);
	psOut->z.z  =   (float)det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4);
	psOut->pos.z = -(float)det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4);

	psOut->x.w  = - (float)det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3);
	psOut->y.w  =   (float)det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3);
	psOut->z.w  = - (float)det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3);
	psOut->pos.w  = (float)det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float SFloat4x4::Determinant(void)
{
	double ans;
	double a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;

	//assign to individual variable names to aid selecting
	//correct elements
	a1 = x.x; b1 = x.y;
	c1 = x.z; d1 = x.w;

	a2 = y.x; b2 = y.y;
	c2 = y.z; d2 = y.w;

	a3 = z.x; b3 = z.y;
	c3 = z.z; d3 = z.w;

	a4 = pos.x; b4 = pos.y;
	c4 = pos.z; d4 = pos.w;

	ans = a1 * det3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4)
		- b1 * det3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4)
		+ c1 * det3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4)
		- d1 * det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);
	return (float)ans;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* SFloat4x4::At(void)
{
	return (SFloat32Vec3*)&z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* SFloat4x4::Up(void)
{
	return (SFloat32Vec3*)&y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* SFloat4x4::Across(void)
{
	return (SFloat32Vec3*)&x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* SFloat4x4::Pos(void)
{
	return (SFloat32Vec3*)&pos;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat4x4* Float4x4Identity(SFloat4x4* psOut)
{
	psOut->Identity();
	return psOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFloat4x4* Float4x4Multiply(SFloat4x4* psOut, SFloat4x4* psLeft, SFloat4x4* psRight)
{
	SFloat4x4	s;

	Float32Vec4TransformCoord(&s.x, &psLeft->x, psRight);
	Float32Vec4TransformCoord(&s.y, &psLeft->y, psRight);
	Float32Vec4TransformCoord(&s.z, &psLeft->z, psRight);
	Float32Vec4TransformCoord(&s.pos, &psLeft->pos, psRight);
	*psOut = s;
	return psOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void Float4x4RotationX(SFloat4x4* psOut, float fRad)
{
	float	s;
	float	c;

	s = sinf(fRad);
	c = cosf(fRad);

	psOut->x.x = 1.0f;		psOut->x.y = 0.0f;		psOut->x.z = 0.0f;		psOut->x.w = 0.0f;
	psOut->y.x = 0.0f;		psOut->y.y = c;			psOut->y.z = s;			psOut->y.w = 0.0f;
	psOut->z.x = 0.0f;		psOut->z.y = -s;		psOut->z.z = c;			psOut->z.w = 0.0f;
	psOut->pos.x = 0.0f;	psOut->pos.y = 0.0f;	psOut->pos.z = 0.0f;	psOut->pos.w = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void Float4x4RotationY(SFloat4x4* psOut, float fRad)
{
	float	s;
	float	c;

	s = sinf(fRad);
	c = cosf(fRad);

	psOut->x.x = c;			psOut->x.y = 0.0f;		psOut->x.z = -s;		psOut->x.w = 0.0f;
	psOut->y.x = 0.0f;		psOut->y.y = 1.0f;		psOut->y.z = 0.0f;		psOut->y.w = 0.0f;
	psOut->z.x = s;			psOut->z.y = 0.0f;		psOut->z.z = c;			psOut->z.w = 0.0f;
	psOut->pos.x = 0.0f;	psOut->pos.y = 0.0f;	psOut->pos.z = 0.0f;	psOut->pos.w = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void Float4x4RotationZ(SFloat4x4* psOut, float fRad)
{
	float	s;
	float	c;

	s = sinf(fRad);
	c = cosf(fRad);

	psOut->x.x = c;			psOut->x.y = s;			psOut->x.z = 0.0f;		psOut->x.w = 0.0f;
	psOut->y.x = -s;		psOut->y.y = c;			psOut->y.z = 0.0f;		psOut->y.w = 0.0f;
	psOut->z.x = 0.0f;		psOut->z.y = 0.0f;		psOut->z.z = 1.0f;		psOut->z.w = 0.0f;
	psOut->pos.x = 0.0f;	psOut->pos.y = 0.0f;	psOut->pos.z = 0.0f;	psOut->pos.w = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void Float4x4RotationVector(SFloat4x4* psOut, SFloat32Vec3* psV, float fRad)
{
	float	u;
	float	s;
	float	c;

	s = -sinf(fRad);
	c = cosf(fRad);
	u = 1.0f - c;

	psOut->x.x = psV->x* psV->x*u + c;			psOut->x.y = psV->x*psV->y*u - psV->z*s;	psOut->x.z = psV->x*psV->z*u + psV->y*s;	psOut->x.w = 0.0f;
	psOut->y.x = psV->x*psV->y*u + psV->z*s;	psOut->y.y = psV->y* psV->y*u + c;			psOut->y.z = psV->z*psV->y*u - psV->x*s;	psOut->y.w = 0.0f;
	psOut->z.x = psV->x*psV->z*u - psV->y*s; 	psOut->z.y = psV->y*psV->z*u + psV->x*s;	psOut->z.z = psV->z* psV->z *u + c;			psOut->z.w = 0.0f;
	psOut->pos.x = 0.0f;						psOut->pos.y = 0.0f;						psOut->pos.z = 0.0f;						psOut->pos.w = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void Float4x4Translation(SFloat4x4* psOut, float x, float y, float z)
{
	psOut->x.x = 1.0f;	psOut->x.y = 0.0f;	psOut->x.z = 0.0f;	psOut->x.w = 0.0f;
	psOut->y.x = 0.0f;	psOut->y.y = 1.0f;	psOut->y.z = 0.0f;	psOut->y.w = 0.0f;
	psOut->z.x = 0.0f;	psOut->z.y = 0.0f;	psOut->z.z = 1.0f;	psOut->z.w = 0.0f;
	psOut->pos.x = x;	psOut->pos.y = y;	psOut->pos.z = z;	psOut->pos.w = 1.0f;
}


bool Float4x4Inverse(SFloat4x4* psOut, float* pfDeterminant, SFloat4x4* psIn)
{
	return psIn->Inverse(psOut, pfDeterminant);
}


SFloat4x4* Float4x4LookAtRH(SFloat4x4* psOut, const SFloat32Vec3* psEye, const SFloat32Vec3* psAt, const SFloat32Vec3* psUp)
{
	//zaxis = normal(Eye - At)
	//xaxis = normal(cross(Up, zaxis))
	//yaxis = cross(zaxis, xaxis)

	//xaxis.x           yaxis.x           zaxis.x          0
	//xaxis.y           yaxis.y           zaxis.y          0
	//xaxis.z           yaxis.z           zaxis.z          0
	//-dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  l

	SFloat32Vec3 sZAxis;
	SFloat32Vec3 sXAxis;
	SFloat32Vec3 sYAxis;

	sZAxis.Copy(psEye);
	sZAxis -= *psAt;
	sZAxis.Normalize();

	Float32Vec3Cross(&sXAxis, psUp, &sZAxis);
	sXAxis.Normalize();

	Float32Vec3Cross(&sYAxis, &sZAxis, &sXAxis);

	psOut->x.x = sXAxis.x;	
	psOut->x.y = sYAxis.x;	
	psOut->x.z = sZAxis.x;
	psOut->x.w = 0.0f;

	psOut->y.x = sXAxis.y;	
	psOut->y.y = sYAxis.y;	
	psOut->y.z = sZAxis.y;
	psOut->y.w = 0.0f;

	psOut->z.x = sXAxis.z;	
	psOut->z.y = sYAxis.z;	
	psOut->z.z = sZAxis.z;
	psOut->z.w = 0.0f;

	psOut->pos.x = -Float32Vec3Dot(&sXAxis, psEye);
	psOut->pos.y = -Float32Vec3Dot(&sYAxis, psEye);
	psOut->pos.z = -Float32Vec3Dot(&sZAxis, psEye);
	psOut->pos.w = 1.0f;

	return psOut;
}


SFloat4x4* Float4x4PerspectiveFovRH(SFloat4x4* psOut, float fFovY, float fAspect, float fZNear, float fZFar)
{
	//yScale = cot(fovY/2)
	//xScale = yScale / aspect ratio

	//xScale     0          0              0
	//0        yScale       0              0
	//0          0      zf/(zn-zf)        -1
	//0          0      zn*zf/(zn-zf)      0

	float fXScale;
	float fYScale;

	fYScale = 1.0f / tanf(fFovY/2.0f);
	fXScale = fYScale / fAspect;

	psOut->x.x = fXScale;	
	psOut->x.y = 0.0f;
	psOut->x.z = 0.0f;
	psOut->x.w = 0.0f;

	psOut->y.x = 0.0f;
	psOut->y.y = fYScale;
	psOut->y.z = 0.0f;
	psOut->y.w = 0.0f;

	psOut->z.x = 0.0f;
	psOut->z.y = 0.0f;
	psOut->z.z = fZFar / (fZNear-fZFar);
	psOut->z.w = -1.0f;

	psOut->pos.x = 0.0f;
	psOut->pos.y = 0.0f;
	psOut->pos.z = fZNear * (fZFar / (fZNear-fZFar));
	psOut->pos.w = 0.0f;

	return psOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool Float4x4Equals(SFloat4x4* ps1, SFloat4x4* ps2)
{
	return memcmp(ps1, ps2, sizeof(SFloat4x4)) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Only the rotation's rows and columns are set, and no position.
//
//////////////////////////////////////////////////////////////////////////
void Float4x4RotationQuaternion(SFloat4x4* psOut, SQuaternion* psRotation)
{
	float64	x;
	float64	y;
	float64	z;
	float64	w;

	x = psRotation->x;
	y = psRotation->y;
	z = psRotation->z;
	w = psRotation->w;

	psOut->x.x = (float)(1.0 - 2.0 * (y * y + z * z));	psOut->x.y = (float)(2.0 * (x * y + z * w));	psOut->x.z = (float)(2.0 * (x * z - y * w));	psOut->x.w = 0.0f;
	psOut->y.x = (float)(2.0 * (x * y - z * w));	psOut->y.y = (float)(1.0 - 2.0 * (x * x + z * z));	psOut->y.z = (float)(2.0 * (y * z + x * w));	psOut->y.w = 0.0f;
	psOut->z.x = (float)(2.0 * (x * z + y * w));	psOut->z.y = (float)(2.0 * (y * z - x * w));	psOut->z.z = (float)(1.0 - 2.0 * (x * x + y * y));	psOut->z.w = 0.0f;
	psOut->pos.x = 0.0f;	psOut->pos.y = 0.0f;	psOut->pos.z = 0.0f;	psOut->pos.w = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// psRotation's x, y and z rows must be units at right angles.  The largest
// of w, x, y and z is worked out first, so that nothing is divided by
// almost nothing.
//
//////////////////////////////////////////////////////////////////////////
void QuaternionRotationMatrix(SQuaternion* psOut, SFloat4x4* psRotation)
{
	float64		m11;
	float64		m12;
	float64		m13;
	float64		m21;
	float64		m22;
	float64		m23;
	float64		m31;
	float64		m32;
	float64		m33;
	float64		fTrace;
	float64		s;
	SQuaternion	sRotation;

	m11 = psRotation->x.x;	m12 = psRotation->x.y;	m13 = psRotation->x.z;
	m21 = psRotation->y.x;	m22 = psRotation->y.y;	m23 = psRotation->y.z;
	m31 = psRotation->z.x;	m32 = psRotation->z.y;	m33 = psRotation->z.z;

	fTrace = m11 + m22 + m33;
	if (fTrace > 0.0)
	{
		s = sqrt(fTrace + 1.0) * 2.0;
		sRotation.w = (float)(0.25 * s);
		sRotation.x = (float)((m23 - m32) / s);
		sRotation.y = (float)((m31 - m13) / s);
		sRotation.z = (float)((m12 - m21) / s);
	}
	else if ((m11 > m22) && (m11 > m33))
	{
		s = sqrt(1.0 + m11 - m22 - m33) * 2.0;
		sRotation.w = (float)((m23 - m32) / s);
		sRotation.x = (float)(0.25 * s);
		sRotation.y = (float)((m12 + m21) / s);
		sRotation.z = (float)((m31 + m13) / s);
	}
	else if (m22 > m33)
	{
		s = sqrt(1.0 + m22 - m11 - m33) * 2.0;
		sRotation.w = (float)((m31 - m13) / s);
		sRotation.x = (float)((m12 + m21) / s);
		sRotation.y = (float)(0.25 * s);
		sRotation.z = (float)((m23 + m32) / s);
	}
	else
	{
		s = sqrt(1.0 + m33 - m11 - m22) * 2.0;
		sRotation.w = (float)((m12 - m21) / s);
		sRotation.x = (float)((m31 + m13) / s);
		sRotation.y = (float)((m23 + m32) / s);
		sRotation.z = (float)(0.25 * s);
	}
	QuaternionNormalize(psOut, &sRotation);
}


//////////////////////////////////////////////////////////////////////////
//
// A row of no length is taken as along its own axis.
//
//////////////////////////////////////////////////////////////////////////
void Float4x4Decompose(SFloat32Vec3* psScale, SQuaternion* psRotation, SFloat32Vec3* psPosition, SFloat4x4* psIn)
{
	float64		afRows[3][3];
	float64		afScale[3];
	float64		fDot;
	float64		fDeterminant;
	SFloat4x4	sRotation;
	SFloat32Vec4*	psRow;
	int			iRow;
	int			iBefore;
	int			k;

	for (iRow = 0; iRow < 3; iRow++)
	{
		psRow = &(&psIn->x)[iRow];
		afRows[iRow][0] = psRow->x;
		afRows[iRow][1] = psRow->y;
		afRows[iRow][2] = psRow->z;
		for (iBefore = 0; iBefore < iRow; iBefore++)
		{
			fDot = afRows[iRow][0] * afRows[iBefore][0] + afRows[iRow][1] * afRows[iBefore][1] + afRows[iRow][2] * afRows[iBefore][2];
			for (k = 0; k < 3; k++)
			{
				afRows[iRow][k] -= fDot * afRows[iBefore][k];
			}
		}
		afScale[iRow] = sqrt(afRows[iRow][0] * afRows[iRow][0] + afRows[iRow][1] * afRows[iRow][1] + afRows[iRow][2] * afRows[iRow][2]);
		for (k = 0; k < 3; k++)
		{
			afRows[iRow][k] = (afScale[iRow] > 0.0) ? afRows[iRow][k] / afScale[iRow] : ((k == iRow) ? 1.0 : 0.0);
		}
	}

	fDeterminant = afRows[0][0] * (afRows[1][1] * afRows[2][2] - afRows[1][2] * afRows[2][1])
				 - afRows[0][1] * (afRows[1][0] * afRows[2][2] - afRows[1][2] * afRows[2][0])
				 + afRows[0][2] * (afRows[1][0] * afRows[2][1] - afRows[1][1] * afRows[2][0]);
	if (fDeterminant < 0.0)
	{
		afScale[2] = -afScale[2];
		for (k = 0; k < 3; k++)
		{
			afRows[2][k] = -afRows[2][k];
		}
	}

	sRotation.Identity();
	for (iRow = 0; iRow < 3; iRow++)
	{
		psRow = &(&sRotation.x)[iRow];
		psRow->x = (float)afRows[iRow][0];
		psRow->y = (float)afRows[iRow][1];
		psRow->z = (float)afRows[iRow][2];
	}
	QuaternionRotationMatrix(psRotation, &sRotation);
	psScale->Init((float)afScale[0], (float)afScale[1], (float)afScale[2]);
	psPosition->Init(psIn->pos.x, psIn->pos.y, psIn->pos.z);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Float4x4Compose(SFloat4x4* psOut, SFloat32Vec3* psScale, SQuaternion* psRotation, SFloat32Vec3* psPosition)
{
	Float4x4RotationQuaternion(psOut, psRotation);
	psOut->x.x *= psScale->x;	psOut->x.y *= psScale->x;	psOut->x.z *= psScale->x;
	psOut->y.x *= psScale->y;	psOut->y.y *= psScale->y;	psOut->y.z *= psScale->y;
	psOut->z.x *= psScale->z;	psOut->z.y *= psScale->z;	psOut->z.z *= psScale->z;
	psOut->pos.x = psPosition->x;
	psOut->pos.y = psPosition->y;
	psOut->pos.z = psPosition->z;
	psOut->pos.w = 1.0f;
}
