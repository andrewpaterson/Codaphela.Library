/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
void CSphere::Init(SFloat3* psPosition)
{
	mpsPosition = psPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Set(SFloat3* pos, float rad)
{
	*mpsPosition = *pos;
	mfRadius = rad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Set(float x, float y, float z, float rad)
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
void CSphere::SetFromPointsUsingAveragePosition(SFloat3* psPoints, int iStride, int iNumPoints)
{
	int			i;
	SFloat3*	psPBase;
	SFloat3		cPoint;
	float		fMaxLen;
	float		fLen;
	SFloat3		cCenter;

	psPBase = psPoints;
	cCenter.Init(0,0,0);
	for (i = 0; i < iNumPoints; i++)
	{
		cCenter += *psPoints;
		psPoints = (SFloat3*)RemapSinglePointer(psPoints, iStride);
	}

	cCenter /= (float)iNumPoints;

	fMaxLen = 0.0f;
	psPoints = psPBase;
	for (i = 0; i < iNumPoints; i++)
	{
		cPoint = cCenter - *psPoints;
		fLen = Float3LengthSq(&cPoint);
		if (fLen > fMaxLen)
		{
			fMaxLen = fLen;
		}
		psPoints = (SFloat3*)RemapSinglePointer(psPoints, iStride);
	}
	mfRadius = sqrtf(fMaxLen);
	*mpsPosition = cCenter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::SetFromPointsUsingBestFit(SFloat3* psPoints, int iStride, int iNumPoints)
{
	int			i, j;
	SFloat3*	psPoints1;
	SFloat3*	psPoints2;
	SFloat3*	psEndPoint1;
	SFloat3*	psEndPoint2;
	SFloat3	cPoint;
	float		fMaxLen;
	float		fLen;

	//This could be optimised quite easily by discarding points which fall inside the sphere.
	fMaxLen = 0.0f;
	psPoints1 = psPoints;
	for (i = 0; i < iNumPoints; i++)
	{
		psPoints2 = psPoints;
		for (j = 0; j < iNumPoints; j++)
		{
			if (i != j)
			{
				cPoint = *psPoints1 - *psPoints2;
				fLen = Float3LengthSq(&cPoint);
				if (fLen > fMaxLen)
				{
					psEndPoint1 = psPoints1;
					psEndPoint2 = psPoints2;
					fMaxLen = fLen;
				}
			}
			psPoints2 = (SFloat3*)RemapSinglePointer(psPoints2, iStride);
		}
		psPoints1 = (SFloat3*)RemapSinglePointer(psPoints1, iStride);
	}
	mfRadius = sqrtf(fMaxLen) / 2.0f;
	*mpsPosition = (*psEndPoint1 + *psEndPoint2) / 2.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CSphere::Volume(void)
{
	return ((4.0f/3.0f)*N_PI)*mfRadius*mfRadius*mfRadius;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CSphere::IntersectSphereVolume(CSphere* pcOther)
{
	float 		r1;
	float 		r2;
	SFloat3		st;
	float		d;
	float		v;

	r1 = this->mfRadius;
	r2 = pcOther->mfRadius;
	ga_memcpy_fast[sizeof(SFloat3)](&st, this->mpsPosition);
	st -= *((SFloat3*)pcOther->mpsPosition);
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

	v = N_PI * sqf(r1+r2-d)*(sqf(d)+(2.0f*d*r2)-(3.0f*sqf(r2))+(2.0f*d*r1)+(6.0f*r2*r1)-(3.0f*(sqf(r1))));
	return v/(12.0f*d);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSphere::Contains(CTriangle* pcTriangle)
{
	float		fRadiusSq;
	int			i;
	SFloat3		sVec;

	fRadiusSq = sqf(mfRadius);

	for (i = 0; i < 3; i++)
	{
		sVec = *mpsPosition - *pcTriangle->GetPoint(i);
		if (sVec.SquareMagnitude() > fRadiusSq)
		{
			return FALSE;
		}
	}
	return TRUE;
}

