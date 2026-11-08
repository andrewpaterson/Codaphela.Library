/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterLib

Codaphela RasterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/FixedPoint.h"
#include "BaseLib/GeometricTypes.h"
#include "RasterMesh.h"
#include "RasterCamera.h"


//Perspective depth is stored as this divided by the distance from the eye, which is linear across the screen.
#define RASTER_DEPTH_NUMERATOR		((int64)1 << 40)
#define RASTER_NEAR_DISTANCE		(1 << 10)

//Keeps orthographic depths positive.
#define RASTER_ORTHOGRAPHIC_DEPTH	(1 << 27)

//Keeps edge function products in 64 bits.  Out by this far is about 32000 pixels from the centre.
#define RASTER_MAX_SCREEN			(1 << 19)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Init(void)
{
	meProjection = RP_Perspective;
	mfYaw = 0.0f;
	mfPitch = 0.0f;
	msPan.Init(0, 0);
	miDistance = RASTER_MESH_HALF_EXTENT * 4;
	miFocalLength = 1;
	miOrthographicScale = 0;
	msScreenCentre.Init(0, 0);
	masAxes[0].Init(1 << RASTER_AXIS_BITS, 0, 0);
	masAxes[1].Init(0, 1 << RASTER_AXIS_BITS, 0);
	masAxes[2].Init(0, 0, 1 << RASTER_AXIS_BITS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::SetProjection(ERasterProjection eProjection)
{
	meProjection = eProjection;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::SetOrbit(float32 fYaw, float32 fPitch)
{
	mfYaw = fYaw;
	mfPitch = fPitch;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::SetPan(int32 iX, int32 iY)
{
	msPan.Init(iX, iY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::SetPerspective(int32 iDistance, int32 iFocalLength)
{
	miDistance = iDistance;
	miFocalLength = iFocalLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::SetOrthographic(int64 iScale)
{
	miOrthographicScale = iScale;
}


//////////////////////////////////////////////////////////////////////////
//
// Sets both projections so that a mesh of any rotation fits the screen.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Fit(int32 iWidth, int32 iHeight)
{
	int32	iSmallest;

	iSmallest = (iWidth < iHeight) ? iWidth : iHeight;

	miFocalLength = iHeight;
	miDistance = RASTER_MESH_HALF_EXTENT + (int32)DivideFloor((int64)RASTER_MESH_HALF_EXTENT * miFocalLength * 2, iSmallest);
	miOrthographicScale = ((int64)iSmallest << 31) / RASTER_MESH_HALF_EXTENT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Prepare(int32 iWidth, int32 iHeight)
{
	SFloat4x4	sYaw;
	SFloat4x4	sPitch;
	SFloat4x4	sMatrix;

	Float4x4RotationZ(&sYaw, mfYaw);
	Float4x4RotationX(&sPitch, mfPitch);
	Float4x4Multiply(&sMatrix, &sYaw, &sPitch);

	masAxes[0].Init(FixedFromFloat(sMatrix.x.x, RASTER_AXIS_BITS), FixedFromFloat(sMatrix.y.x, RASTER_AXIS_BITS), FixedFromFloat(sMatrix.z.x, RASTER_AXIS_BITS));
	masAxes[1].Init(FixedFromFloat(sMatrix.x.y, RASTER_AXIS_BITS), FixedFromFloat(sMatrix.y.y, RASTER_AXIS_BITS), FixedFromFloat(sMatrix.z.y, RASTER_AXIS_BITS));
	masAxes[2].Init(FixedFromFloat(sMatrix.x.z, RASTER_AXIS_BITS), FixedFromFloat(sMatrix.y.z, RASTER_AXIS_BITS), FixedFromFloat(sMatrix.z.z, RASTER_AXIS_BITS));

	msScreenCentre.Init(FixedFromInt(iWidth, RASTER_SUBPIXEL_BITS - 1), FixedFromInt(iHeight, RASTER_SUBPIXEL_BITS - 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int32 RasterClampScreen(int64 i)
{
	if (i > RASTER_MAX_SCREEN)
	{
		return RASTER_MAX_SCREEN;
	}
	if (i < -RASTER_MAX_SCREEN)
	{
		return -RASTER_MAX_SCREEN;
	}
	return (int32)i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Project(SRasterScreenVertex* psDest, SInt32Vec3* psPosition)
{
	SInt64Vec3	sPosition;
	int64		x;
	int64		y;
	int64		z;
	int64		iEyeDistance;

	sPosition.Init(psPosition->x, psPosition->y, psPosition->z);
	x = (Int64Vec3Dot(&sPosition, &masAxes[0]) >> RASTER_AXIS_BITS) + msPan.x;
	y = (Int64Vec3Dot(&sPosition, &masAxes[1]) >> RASTER_AXIS_BITS) + msPan.y;
	z = Int64Vec3Dot(&sPosition, &masAxes[2]) >> RASTER_AXIS_BITS;

	if (meProjection == RP_Perspective)
	{
		iEyeDistance = miDistance - z;
		if (iEyeDistance < RASTER_NEAR_DISTANCE)
		{
			psDest->iDepth = 0;
			return;
		}
		psDest->sPosition.x = RasterClampScreen(DivideFloor((x * miFocalLength) << RASTER_SUBPIXEL_BITS, iEyeDistance) + msScreenCentre.x);
		psDest->sPosition.y = RasterClampScreen(msScreenCentre.y - DivideFloor((y * miFocalLength) << RASTER_SUBPIXEL_BITS, iEyeDistance));
		psDest->iDepth = (int32)(RASTER_DEPTH_NUMERATOR / iEyeDistance);
	}
	else
	{
		psDest->sPosition.x = RasterClampScreen(((x * miOrthographicScale) >> (32 - RASTER_SUBPIXEL_BITS)) + msScreenCentre.x);
		psDest->sPosition.y = RasterClampScreen(msScreenCentre.y - ((y * miOrthographicScale) >> (32 - RASTER_SUBPIXEL_BITS)));
		psDest->iDepth = (int32)z + RASTER_ORTHOGRAPHIC_DEPTH;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Rotates a direction on the screen back into the mesh, keeping its fixed point.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::ViewToMesh(SInt64Vec3* psDest, SInt64Vec3* psView)
{
	psDest->x = (psView->x * masAxes[0].x + psView->y * masAxes[1].x + psView->z * masAxes[2].x) >> RASTER_AXIS_BITS;
	psDest->y = (psView->x * masAxes[0].y + psView->y * masAxes[1].y + psView->z * masAxes[2].y) >> RASTER_AXIS_BITS;
	psDest->z = (psView->x * masAxes[0].z + psView->y * masAxes[1].z + psView->z * masAxes[2].z) >> RASTER_AXIS_BITS;
}

