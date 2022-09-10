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
#include "HalfSpace.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/FloatHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::Init(SFloat3* psNormal)
{
	mpsNormal = psNormal;
	d = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHalfSpace::Contains(SFloat3* psPosition)
{
	return FloatLessThan(Float3Dot(mpsNormal, psPosition), d);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHalfSpace::NotContains(SFloat3* psPosition)
{
	return FloatGreaterThan(Float3Dot(mpsNormal, psPosition), d);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHalfSpace::On(SFloat3* psPosition)
{
	return FloatEqual(Float3Dot(mpsNormal, psPosition), d);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::Set(SFloat3* v1, SFloat3* v2, SFloat3* v3)
{
	SFloat3 side1; 
	SFloat3 side2;

	side1 = *v2 - *v1;
	side2 = *v3 - *v1;
	Set2(v1, &side1, &side2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::Set2(SFloat3* psPosition1, SFloat3* psVector1, SFloat3* psVector2)
{
	Float3Cross(mpsNormal, psVector1, psVector2);
	Float3Normalize(mpsNormal, mpsNormal);
	Set(psPosition1, mpsNormal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::Set(SFloat3* psPosition1, SFloat3* psNormal)
{
	*mpsNormal = *psNormal;
	D(psPosition1, mpsNormal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::SetZPlane(SFloat3* v1, SFloat3* v2)
{
	SFloat3	side1;
	SFloat3	z;

	z.Init(0,0,1);

	side1 = *v1 - *v2;
	Float3Cross(mpsNormal, &side1, &z);
	Float3Normalize(mpsNormal, mpsNormal);
	D(v1, mpsNormal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHalfSpace::FindFurthestPoint(SFloat3* psPoints, int iStride, int iNumPoints)
{
	int					iFarIndex;
	int					i;
	SFloat3*			psOther;
	float				fDist;
	float				fMaxDist;

	fMaxDist = SMALL_NUMBER;
	iFarIndex = -1;
	for (i = 0; i < iNumPoints; i++)
	{
		psOther = GetPosition(psPoints, iStride, i);

		fDist = Float3Dot(mpsNormal, psOther);
		if (fDist >= fMaxDist)
		{
			fMaxDist = fDist;
			iFarIndex = i;
		}
	}
	return iFarIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHalfSpace::FindUnsignedFurthestPoint(SFloat3* psPoints, int iStride, int iNumPoints)
{
	int					iFarIndex;
	int					i;
	SFloat3*			psOther;
	float				fDist;
	float				fMaxDist;

	fMaxDist = SMALL_NUMBER;
	iFarIndex = -1;
	for (i = 0; i < iNumPoints; i++)
	{
		psOther = GetPosition(psPoints, iStride, i);

		fDist = fabsf(Float3Dot(mpsNormal, psOther));
		if (fDist >= fMaxDist)
		{
			fMaxDist = fDist;
			iFarIndex = i;
		}
	}
	return iFarIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHalfSpace::FindFurthestPoint(SFloat3* psPoints, int iStride, int* aiIndices, int iNumIndices)
{
	int					iFarIndex;
	int					i;
	SFloat3*			psOther;
	float				fDist;
	float				fMaxDist;
	int					iIndex;

	fMaxDist = SMALL_NUMBER;
	iFarIndex = -1;
	for (i = 0; i < iNumIndices; i++)
	{
		iIndex = aiIndices[i];
		psOther = GetPosition(psPoints, iStride, iIndex);

		fDist = Float3Dot(mpsNormal, psOther)-d;
		if (fDist > fMaxDist)
		{
			fMaxDist = fDist;
			iFarIndex = iIndex;
		}
	}
	return iFarIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHalfSpace::Parallel(CHalfSpace* psOther)
{
	return FloatGreaterThanOrEqual(Float3Dot(psOther->mpsNormal, mpsNormal), 1.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::D(SFloat3* psPosition1, SFloat3* psNormal)
{
	d = Float3Dot(psNormal, psPosition1);
}