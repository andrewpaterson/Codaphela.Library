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
#include "PointerFunctions.h"
#include "IntegerHelper.h"
#include "Numbers.h"
#include "FloatHelper.h"
#include "DataIO.h"
#include "Double2.h"
#include "Double3.h"
#include "Double4.h"
#include "Double4x4.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4x4::Init(void)
{
	x.x = y.y = z.z = pos.w = 1.0f;
	x.y = x.z = x.w = y.x = y.z = y.w = z.x = z.y = z.w = pos.x = pos.y = pos.z = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4x4::Zero(void)
{
	x.x = y.y = z.z = pos.w = 0.0f;
	x.y = x.z = x.w = y.x = y.z = y.w = z.x = z.y = z.w = pos.x = pos.y = pos.z = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4x4::Identity(void)
{
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4x4::Init(SDouble3* psX, SDouble3* psY, SDouble3* psZ, SDouble3* psPos)
{
	//Optimise me later!
	x = SDouble4((double*)psX, 0.0f);
	y = SDouble4((double*)psY, 0.0f);
	z = SDouble4((double*)psZ, 0.0f);
	pos = SDouble4((double*)psPos, 1.0f);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4x4::Fix(void)
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
BOOL SDouble4x4::Save(CFileWriter* pcFile)
{
	BOOL	bResult;

	bResult = x.Save(pcFile);
	bResult |= y.Save(pcFile);
	bResult |= z.Save(pcFile);
	bResult |= pos.Save(pcFile);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SDouble4x4::Load(CFileReader* pcFile)
{
	BOOL	bResult;

	bResult = x.Load(pcFile);
	bResult |= y.Load(pcFile);
	bResult |= z.Load(pcFile);
	bResult |= pos.Load(pcFile);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4x4::ToString(CChars* psz, BOOL bOneLine, int iWholeNumbers, int iDecimals)
{
	int	i[4];

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

	x.ToString(psz, iWholeNumbers, iDecimals);
	if (!bOneLine) psz->AppendNewLine();
	y.ToString(psz, iWholeNumbers, iDecimals);
	if (!bOneLine) psz->AppendNewLine();
	z.ToString(psz, iWholeNumbers, iDecimals);
	if (!bOneLine) psz->AppendNewLine();
	pos.ToString(psz, iWholeNumbers, iDecimals);
	if (!bOneLine) psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4x4::Dump(void)
{
	CChars	sz;

	sz.Init();
	ToString(&sz, FALSE);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SDouble4x4::Inverse(SDouble4x4* psOut, double* pfDeterminant)
{
	double		fDet;
	double		fDetInv;
	SDouble4x4	sOut;

	/* calculate the adjoint matrix */
	Adjoint(&sOut);

	/*  calculate the 4x4 determinant
	*  if the determinant is zero,
	*  then the inverse matrix is not unique.
	*/
	fDet = Determinant();

	if (fabs(fDet) < SMALL_NUMBER)
	{
		return FALSE;
	}

	/* scale the adjoint matrix to get the inverse */
	fDetInv = 1.0f / fDet;
	sOut.x *= fDetInv;
	sOut.y *=fDetInv;
	sOut.z *= fDetInv;
	sOut.pos *= fDetInv;

	*psOut = sOut;
	SafeAssign(pfDeterminant, fDet);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDouble4x4::Adjoint(SDouble4x4* psOut)
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
	psOut->x.x  =   (double)det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4);
	psOut->y.x  = - (double)det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4);
	psOut->z.x  =   (double)det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4);
	psOut->pos.x = -(double)det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);

	psOut->x.y  = - (double)det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4);
	psOut->y.y  =   (double)det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4);
	psOut->z.y  = - (double)det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4);
	psOut->pos.y  = (double)det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4);

	psOut->x.z  =   (double)det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4);
	psOut->y.z  = - (double)det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4);
	psOut->z.z  =   (double)det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4);
	psOut->pos.z = -(double)det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4);

	psOut->x.w  = - (double)det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3);
	psOut->y.w  =   (double)det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3);
	psOut->z.w  = - (double)det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3);
	psOut->pos.w  = (double)det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
double SDouble4x4::Determinant(void)
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
	return (double)ans;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDouble3* SDouble4x4::At(void)
{
	return (SDouble3*)&z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDouble3* SDouble4x4::Up(void)
{
	return (SDouble3*)&y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDouble3* SDouble4x4::Across(void)
{
	return (SDouble3*)&x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDouble3* SDouble4x4::Pos(void)
{
	return (SDouble3*)&pos;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4x4* Double4x4Identity(SDouble4x4* psOut)
{
	psOut->Identity();
	return psOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDouble4x4* Double4x4Multiply(SDouble4x4* psOut, SDouble4x4* psLeft, SDouble4x4* psRight)
{
	SDouble4x4	s;

	Double4Transform(&s.x, psRight, &psLeft->x);
	Double4Transform(&s.y, psRight, &psLeft->y);
	Double4Transform(&s.z, psRight, &psLeft->z);
	Double4Transform(&s.pos, psRight, &psLeft->pos);
	*psOut = s;
	return psOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void Double4x4RotationX(SDouble4x4* psOut, double fRad)
{
	double	s;
	double	c;

	s = sin(fRad);
	c = cos(fRad);

	psOut->x.x = 1.0f;		psOut->x.y = 0.0f;		psOut->x.z = 0.0f;		psOut->x.w = 0.0f;
	psOut->y.x = 0.0f;		psOut->y.y = c;			psOut->y.z = s;			psOut->y.w = 0.0f;
	psOut->z.x = 0.0f;		psOut->z.y = -s;		psOut->z.z = c;			psOut->z.w = 0.0f;
	psOut->pos.x = 0.0f;	psOut->pos.y = 0.0f;	psOut->pos.z = 0.0f;	psOut->pos.w = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void Double4x4RotationY(SDouble4x4* psOut, double fRad)
{
	double	s;
	double	c;

	s = sin(fRad);
	c = cos(fRad);

	psOut->x.x = c;			psOut->x.y = 0.0f;		psOut->x.z = -s;		psOut->x.w = 0.0f;
	psOut->y.x = 0.0f;		psOut->y.y = 1.0f;		psOut->y.z = 0.0f;		psOut->y.w = 0.0f;
	psOut->z.x = s;			psOut->z.y = 0.0f;		psOut->z.z = c;			psOut->z.w = 0.0f;
	psOut->pos.x = 0.0f;	psOut->pos.y = 0.0f;	psOut->pos.z = 0.0f;	psOut->pos.w = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void Double4x4RotationZ(SDouble4x4* psOut, double fRad)
{
	double	s;
	double	c;

	s = sin(fRad);
	c = cos(fRad);

	psOut->x.x = c;			psOut->x.y = s;			psOut->x.z = 0.0f;		psOut->x.w = 0.0f;
	psOut->y.x = -s;		psOut->y.y = c;			psOut->y.z = 0.0f;		psOut->y.w = 0.0f;
	psOut->z.x = 0.0f;		psOut->z.y = 0.0f;		psOut->z.z = 1.0f;		psOut->z.w = 0.0f;
	psOut->pos.x = 0.0f;	psOut->pos.y = 0.0f;	psOut->pos.z = 0.0f;	psOut->pos.w = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void Double4x4RotationVector(SDouble4x4* psOut, SDouble3* psV, double fRad)
{
	double	u;
	double	s;
	double	c;

	s = -sin(fRad);
	c = cos(fRad);
	u = 1.0f - c;

	psOut->x.x = sq(psV->x)*u + c;				psOut->x.y = psV->x*psV->y*u - psV->z*s;		psOut->x.z = psV->x*psV->z*u + psV->y*s;	psOut->x.w = 0.0f;
	psOut->y.x = psV->x*psV->y*u + psV->z*s;	psOut->y.y = sq(psV->y)*u + c;					psOut->y.z = psV->z*psV->y*u - psV->x*s;	psOut->y.w = 0.0f;
	psOut->z.x = psV->x*psV->z*u - psV->y*s; 	psOut->z.y = psV->y*psV->z*u + psV->x*s;		psOut->z.z = sq(psV->z)*u + c;				psOut->z.w = 0.0f;
	psOut->pos.x = 0.0f;						psOut->pos.y = 0.0f;							psOut->pos.z = 0.0f;						psOut->pos.w = 1.0f;

	//psOut->x.x = 1 + (1-cos(fRad))*(psV->x*psV->x-1);  				psOut->x.y = -psV->z*sin(fRad)+(1-cos(fRad))*psV->x*psV->y;		psOut->x.z = psV->y*sin(fRad)+(1-cos(fRad))*psV->x*psV->z;
	//psOut->y.x = psV->z*sin(fRad)+(1-cos(fRad))*psV->x*psV->y; 		psOut->y.y = 1 + (1-cos(fRad))*(psV->y*psV->y-1); 				psOut->y.z = -psV->x*sin(fRad)+(1-cos(fRad))*psV->y*psV->z;
	//psOut->z.x = -psV->y*sin(fRad)+(1-cos(fRad))*psV->x*psV->z; 	psOut->z.y = psV->x*sin(fRad)+(1-cos(fRad))*psV->y*psV->z;		psOut->z.z = 1 + (1-cos(fRad))*(psV->z*psV->z-1);
	//psOut->x.w = 0.0f; psOut->y.w = 0.0f; psOut->z.w = 0.0f;	psOut->pos.x = 0.0f;
	//psOut->pos.y = 0.0f; psOut->pos.z = 0.0f; psOut->pos.w = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void Double4x4Translation(SDouble4x4* psOut, double x, double y, double z)
{
	psOut->x.x = 1.0f;	psOut->x.y = 0.0f;	psOut->x.z = 0.0f;	psOut->x.w = 0.0f;
	psOut->y.x = 0.0f;	psOut->y.y = 1.0f;	psOut->y.z = 0.0f;	psOut->y.w = 0.0f;
	psOut->z.x = 0.0f;	psOut->z.y = 0.0f;	psOut->z.z = 1.0f;	psOut->z.w = 0.0f;
	psOut->pos.x = x;	psOut->pos.y = y;	psOut->pos.z = z;	psOut->pos.w = 1.0f;
}


BOOL Double4x4Inverse(SDouble4x4* psOut, double* pfDeterminant, SDouble4x4* psIn)
{
	return psIn->Inverse(psOut, pfDeterminant);
}


SDouble4x4* Double4x4LookAtRH(SDouble4x4* psOut, const SDouble3* psEye, const SDouble3* psAt, const SDouble3* psUp)
{
	//zaxis = normal(Eye - At)
	//xaxis = normal(cross(Up, zaxis))
	//yaxis = cross(zaxis, xaxis)

	//xaxis.x           yaxis.x           zaxis.x          0
	//xaxis.y           yaxis.y           zaxis.y          0
	//xaxis.z           yaxis.z           zaxis.z          0
	//-dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  l

	SDouble3 sZAxis;
	SDouble3 sXAxis;
	SDouble3 sYAxis;

	sZAxis.Copy(psEye);
	sZAxis -= *psAt;
	sZAxis.Normalize();

	Double3Cross(&sXAxis, psUp, &sZAxis);
	sXAxis.Normalize();

	Double3Cross(&sYAxis, &sZAxis, &sXAxis);

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

	psOut->pos.x = -Double3Dot(&sXAxis, psEye);
	psOut->pos.y = -Double3Dot(&sYAxis, psEye);
	psOut->pos.z = -Double3Dot(&sZAxis, psEye);
	psOut->pos.w = 1.0f;

	return psOut;
}


SDouble4x4* Double4x4PerspectiveFovRH(SDouble4x4* psOut, double fFovY, double fAspect, double fZNear, double fZFar)
{
	//yScale = cot(fovY/2)
	//xScale = yScale / aspect ratio

	//xScale     0          0              0
	//0        yScale       0              0
	//0          0      zf/(zn-zf)        -1
	//0          0      zn*zf/(zn-zf)      0

	double fXScale;
	double fYScale;

	fYScale = 1.0f / tan(fFovY/2.0f);
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
BOOL Double4x4Equals(SDouble4x4* ps1, SDouble4x4* ps2)
{
	return memcmp(ps1, ps2, sizeof(SDouble4x4)) == 0;
}
