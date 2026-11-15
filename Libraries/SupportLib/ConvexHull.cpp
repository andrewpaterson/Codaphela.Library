/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "QuickHull.h"
#include "ConvexHull.h"


//////////////////////////////////////////////////////////////////////////
//
// False, and nothing, for points CQuickHull can't make a hull of.
//
//////////////////////////////////////////////////////////////////////////
bool CConvexHull::Init(SFloat32Vec3* psPoints, size uiStride, size uiNumPoints)
{
	CQuickHull	cHull;
	CArrayInt	aiCorners;
	size		i;
	bool		bResult;

	PreInit();
	CShape::Init();
	mcPlanes.Init();
	mcCorners.Init();

	cHull.Init();
	bResult = cHull.Generate(psPoints, uiStride, uiNumPoints);
	if (bResult)
	{
		for (i = 0; i < cHull.NumFaces(); i++)
		{
			*mcPlanes.Add() = cHull.GetFace(i)->sPlane;
		}
		aiCorners.Init();
		cHull.GetVertices(&aiCorners);
		for (i = 0; i < aiCorners.NumElements(); i++)
		{
			*mcCorners.Add() = *GetPosition(psPoints, uiStride, (size)aiCorners.GetValue(i));
		}
		aiCorners.Kill();
	}
	cHull.Kill();

	PostInit();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHull::Class(void)
{
	CShape::Class();
	U_Unknown(CArrayFloat32Plane, mcPlanes);
	U_Unknown(CArrayFloat32Vec3, mcCorners);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHull::Free(void)
{
	mcCorners.Kill();
	mcPlanes.Kill();
	CShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CConvexHull::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(CShape::Save(pcFile));
	ReturnOnFalse(mcPlanes.Write(pcFile));
	return mcCorners.Write(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CConvexHull::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(CShape::Load(pcFile));
	ReturnOnFalse(mcPlanes.Read(pcFile));
	return mcCorners.Read(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CConvexHull::NumPlanes(void)
{
	return mcPlanes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CConvexHull::Distance(SFloat32Vec3* psPosition)
{
	float32		fDistance;

	FindFurthestPlane(psPosition, &fDistance);
	return fDistance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHull::Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	float32		fDistance;
	int			iPlane;

	iPlane = FindFurthestPlane(psPosition, &fDistance);
	if (iPlane != -1)
	{
		*psDest = mcPlanes.Get((size)iPlane)->sNormal;
	}
	else
	{
		psDest->Zero();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHull::GetBounds(SFloat32Bounds* psDest)
{
	psDest->Init(mcCorners.GetData(), sizeof(SFloat32Vec3), mcCorners.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CConvexHull::IsExact(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The plane psPosition is furthest in front of (or least behind), the first
// of those as far, and how far.  -1, as far as can be, with no planes.
//
//////////////////////////////////////////////////////////////////////////
int CConvexHull::FindFurthestPlane(SFloat32Vec3* psPosition, float32* pfDistance)
{
	size		i;
	int			iFurthest;
	float32		fDistance;

	iFurthest = -1;
	*pfDistance = FLOAT32_BOUNDS_LARGEST;
	for (i = 0; i < mcPlanes.NumElements(); i++)
	{
		fDistance = mcPlanes.Get(i)->Distance(psPosition);
		if ((iFurthest == -1) || (fDistance > *pfDistance))
		{
			iFurthest = (int)i;
			*pfDistance = fDistance;
		}
	}
	return iFurthest;
}
