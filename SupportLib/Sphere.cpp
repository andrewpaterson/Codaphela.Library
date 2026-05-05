/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ShapeLib

Codaphela ShapeLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ShapeLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ShapeLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Sphere.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/FloatHelper.h"
#include "BaseLib/FastFunctions.h"
#include "Triangle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Init(SFloat32Vec3* psPosition)
{
	mpsPosition = psPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Set(SFloat32Vec3* pos, float32 rad)
{
	*mpsPosition = *pos;
	mfRadius = rad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Set(float32 x, float32 y, float32 z, float32 rad)
{
	mpsPosition->x = x;
	mpsPosition->y = y;
	mpsPosition->z = z;
	mfRadius = rad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Copy(CSphere* source)
{
	mfRadius = source->mfRadius;
	*mpsPosition = *source->mpsPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::SetFromPointsUsingAveragePosition(SFloat32Vec3* psPoints, int iStride, int iNumPoints)
{
	int			i;
	SFloat32Vec3*	psPBase;
	SFloat32Vec3		cPoint;
	float32		fMaxLen;
	float32		fLen;
	SFloat32Vec3		cCenter;

	psPBase = psPoints;
	cCenter.Init(0,0,0);
	for (i = 0; i < iNumPoints; i++)
	{
		cCenter += *psPoints;
		psPoints = (SFloat32Vec3*)RemapSinglePointer(psPoints, iStride);
	}

	cCenter /= (float32)iNumPoints;

	fMaxLen = 0.0f;
	psPoints = psPBase;
	for (i = 0; i < iNumPoints; i++)
	{
		cPoint = cCenter - *psPoints;
		fLen = cPoint.SquareMagnitude();
		if (fLen > fMaxLen)
		{
			fMaxLen = fLen;
		}
		psPoints = (SFloat32Vec3*)RemapSinglePointer(psPoints, iStride);
	}
	mfRadius = SquareRoot(fMaxLen);
	*mpsPosition = cCenter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::SetFromPointsUsingBestFit(SFloat32Vec3* psPoints, int iStride, int iNumPoints)
{
	int			i, j;
	SFloat32Vec3*	psPoints1;
	SFloat32Vec3*	psPoints2;
	SFloat32Vec3*	psEndPoint1;
	SFloat32Vec3*	psEndPoint2;
	SFloat32Vec3	cPoint;
	float32		fMaxLen;
	float32		fLen;

	//This could be optimised quite easily by discarding points which fall inside the sphere.
	fMaxLen = 0.0f;
	psPoints1 = psPoints;
	psEndPoint1 = NULL;
	psEndPoint2 = NULL;
	for (i = 0; i < iNumPoints; i++)
	{
		psPoints2 = psPoints;
		for (j = 0; j < iNumPoints; j++)
		{
			if (i != j)
			{
				cPoint = *psPoints1 - *psPoints2;
				fLen = cPoint.SquareMagnitude();
				if (fLen > fMaxLen)
				{
					psEndPoint1 = psPoints1;
					psEndPoint2 = psPoints2;
					fMaxLen = fLen;
				}
			}
			psPoints2 = (SFloat32Vec3*)RemapSinglePointer(psPoints2, iStride);
		}
		psPoints1 = (SFloat32Vec3*)RemapSinglePointer(psPoints1, iStride);
	}
	mfRadius = SquareRoot(fMaxLen) / 2.0f;
	*mpsPosition = (*psEndPoint1 + *psEndPoint2) / 2.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CSphere::Volume(void)
{
	return ((4.0f/3.0f)*N_PI)*mfRadius*mfRadius*mfRadius;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CSphere::IntersectSphereVolume(CSphere* pcOther)
{
	float32 		r1;
	float32 		r2;
	SFloat32Vec3		st;
	float32		d;
	float32		v;

	r1 = this->mfRadius;
	r2 = pcOther->mfRadius;
	ga_memcpy_fast[sizeof(SFloat32Vec3)](&st, this->mpsPosition);
	st -= *((SFloat32Vec3*)pcOther->mpsPosition);
	d = st.Magnitude();

	if (d > r1+r2)
	{
		return 0.0f;
	}
	if (d + minf(r2, r1) <= maxf(r2, r1)) 
	{
		if (r2 < r1)
		{
			return pcOther->Volume();
		}
		else
		{
			return Volume();
		}
	}

	v = N_PI * Square(r1+r2-d)*(Square(d)+(2.0f*d*r2)-(3.0f* Square(r2))+(2.0f*d*r1)+(6.0f*r2*r1)-(3.0f*(Square(r1))));
	return v/(12.0f*d);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSphere::Contains(CTriangle* pcTriangle)
{
	float32		fRadiusSq;
	int			i;
	SFloat32Vec3		sVec;

	fRadiusSq = SquareRoot(mfRadius);

	for (i = 0; i < 3; i++)
	{
		sVec = *mpsPosition - *pcTriangle->GetPoint(i);
		if (sVec.SquareMagnitude() > fRadiusSq)
		{
			return false;
		}
	}
	return true;
}

