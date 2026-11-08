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
#include <math.h>
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

//KiCad's 3D viewer: a 45 degree vertical field of view, whose focal length is half the height over tan(22.5), and an eye
//two widths of the mesh away.
#define RASTER_FOCAL_THOUSANDTHS	1207
#define RASTER_INITIAL_DISTANCE		(RASTER_MESH_HALF_EXTENT * 4)

//KiCad's limits on its camera's distance over its initial distance.
#define RASTER_MIN_ZOOM				0.02f
#define RASTER_MAX_ZOOM				2.0f

//KiCad's trackball: Gavin Bell's sphere, which is a hyperbola from 1 / sqrt(2) of the way out.
#define RASTER_TRACKBALL_SIZE		0.8

//KiCad's wheel pan: 0.7 for each step with its initial distance of 16.
#define RASTER_PAN_STEP_NUMERATOR	7
#define RASTER_PAN_STEP_DENOMINATOR	160


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Init(void)
{
	meProjection = RP_Perspective;
	msRotation.Identity();
	msPivot.Init(0, 0, 0);
	msPan.Init(0, 0);
	miDistance = RASTER_INITIAL_DISTANCE;
	miInitialDistance = RASTER_INITIAL_DISTANCE;
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
// Replaces the rotation: turned around the mesh's Z then tilted around the
// screen's X.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::SetOrbit(float32 fYaw, float32 fPitch)
{
	SFloat4x4	sYaw;
	SFloat4x4	sPitch;

	Float4x4RotationZ(&sYaw, fYaw);
	Float4x4RotationX(&sPitch, fPitch);
	Float4x4Multiply(&msRotation, &sYaw, &sPitch);
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
	miInitialDistance = iDistance;
	miFocalLength = iFocalLength;
}


//////////////////////////////////////////////////////////////////////////
//
// The initial view of a mesh.  It doesn't change the rotation.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Fit(int32 iWidth, int32 iHeight)
{
	miDistance = RASTER_INITIAL_DISTANCE;
	miInitialDistance = RASTER_INITIAL_DISTANCE;
	msPivot.Init(0, 0, 0);
	msPan.Init(0, 0);
	Resize(iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps the view and its field of view as the screen changes size.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Resize(int32 iHeight)
{
	miFocalLength = (iHeight * RASTER_FOCAL_THOUSANDTHS) / 1000;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float64 RasterTrackballDepth(float64 x, float64 y)
{
	float64	fDistance;
	float64	fEdge;

	fDistance = sqrt(x * x + y * y);
	if (fDistance < RASTER_TRACKBALL_SIZE * 0.70710678118654752440)
	{
		return sqrt(RASTER_TRACKBALL_SIZE * RASTER_TRACKBALL_SIZE - fDistance * fDistance);
	}

	fEdge = RASTER_TRACKBALL_SIZE / 1.41421356237309504880;
	return (fEdge * fEdge) / fDistance;
}


//////////////////////////////////////////////////////////////////////////
//
// A trackball: the pointer's last and new positions on a sphere filling the
// screen give the rotation, in the screen's axes, that takes one to the other.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Drag(int32 iLastX, int32 iLastY, int32 iX, int32 iY, int32 iWidth, int32 iHeight)
{
	float64			ax;
	float64			ay;
	float64			az;
	float64			bx;
	float64			by;
	float64			bz;
	float64			fChord;
	SFloat32Vec3	sAxis;
	SFloat4x4		sSpin;

	if ((iLastX == iX) && (iLastY == iY))
	{
		return;
	}

	ax = (2.0 * iLastX - iWidth) / iWidth;
	ay = ((float64)iHeight - 2.0 * iLastY) / iHeight;
	az = RasterTrackballDepth(ax, ay);
	bx = (2.0 * iX - iWidth) / iWidth;
	by = ((float64)iHeight - 2.0 * iY) / iHeight;
	bz = RasterTrackballDepth(bx, by);

	sAxis.Init((float32)(ay * bz - az * by), (float32)(az * bx - ax * bz), (float32)(ax * by - ay * bx));
	sAxis.Normalize();

	fChord = sqrt((ax - bx) * (ax - bx) + (ay - by) * (ay - by) + (az - bz) * (az - bz)) / (2.0 * RASTER_TRACKBALL_SIZE);
	if (fChord > 1.0)
	{
		fChord = 1.0;
	}

	Float4x4RotationVector(&sSpin, &sAxis, (float32)(2.0 * asin(fChord)));
	Float4x4Multiply(&msRotation, &msRotation, &sSpin);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves what is at the pivot's depth this many pixels on the screen.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Pan(int32 iRight, int32 iDown)
{
	msPan.x += (int32)(((int64)iRight * miDistance) / miFocalLength);
	msPan.y -= (int32)(((int64)iDown * miDistance) / miFocalLength);
}


//////////////////////////////////////////////////////////////////////////
//
// One step of the wheel, in a direction of -1, 0 or 1 on each axis.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::PanStep(int32 iRight, int32 iUp)
{
	int32	iStep;

	iStep = (int32)(((int64)miDistance * RASTER_PAN_STEP_NUMERATOR) / RASTER_PAN_STEP_DENOMINATOR);
	msPan.x += iRight * iStep;
	msPan.y += iUp * iStep;
}


//////////////////////////////////////////////////////////////////////////
//
// A factor above 1 moves the eye closer.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Zoom(float32 fFactor)
{
	float32	fDistance;
	float32	fMin;
	float32	fMax;

	fDistance = (float32)miDistance / fFactor;
	fMin = (float32)miInitialDistance * RASTER_MIN_ZOOM;
	fMax = (float32)miInitialDistance * RASTER_MAX_ZOOM;
	if (fDistance < fMin)
	{
		fDistance = fMin;
	}
	else if (fDistance > fMax)
	{
		fDistance = fMax;
	}
	miDistance = (int32)fDistance;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the point of the box (the mesh's bounds) under pixel iX, iY the pivot
// and moves it to the middle of the screen.  False, changing nothing, if the
// pointer isn't over the box.  Uses the axes from the last Prepare.
//
//////////////////////////////////////////////////////////////////////////
bool CRasterCamera::LookAtScreen(int32 iX, int32 iY, SInt32Vec3* psMin, SInt32Vec3* psMax)
{
	float64		fRight;
	float64		fUp;
	float64		afViewOrigin[3];
	float64		afViewDirection[3];
	float64		afOrigin[3];
	float64		afDirection[3];
	float64		afMin[3];
	float64		afMax[3];
	float64		fNear;
	float64		fFar;
	float64		fA;
	float64		fB;
	float64		fSwap;
	int			i;
	int			k;

	fRight = (float64)(FixedFromInt(iX, RASTER_SUBPIXEL_BITS) + RASTER_SUBPIXELS / 2 - msScreenCentre.x) / RASTER_SUBPIXELS;
	fUp = (float64)(msScreenCentre.y - FixedFromInt(iY, RASTER_SUBPIXEL_BITS) - RASTER_SUBPIXELS / 2) / RASTER_SUBPIXELS;

	if (meProjection == RP_Perspective)
	{
		afViewOrigin[0] = -(float64)msPan.x;
		afViewOrigin[1] = -(float64)msPan.y;
		afViewOrigin[2] = miDistance;
		afViewDirection[0] = fRight / miFocalLength;
		afViewDirection[1] = fUp / miFocalLength;
		afViewDirection[2] = -1.0;
	}
	else
	{
		afViewOrigin[0] = (fRight * miDistance) / miFocalLength - msPan.x;
		afViewOrigin[1] = (fUp * miDistance) / miFocalLength - msPan.y;
		afViewOrigin[2] = (float64)RASTER_ORTHOGRAPHIC_DEPTH;
		afViewDirection[0] = 0.0;
		afViewDirection[1] = 0.0;
		afViewDirection[2] = -1.0;
	}

	afOrigin[0] = msPivot.x + (afViewOrigin[0] * masAxes[0].x + afViewOrigin[1] * masAxes[1].x + afViewOrigin[2] * masAxes[2].x) / (1 << RASTER_AXIS_BITS);
	afOrigin[1] = msPivot.y + (afViewOrigin[0] * masAxes[0].y + afViewOrigin[1] * masAxes[1].y + afViewOrigin[2] * masAxes[2].y) / (1 << RASTER_AXIS_BITS);
	afOrigin[2] = msPivot.z + (afViewOrigin[0] * masAxes[0].z + afViewOrigin[1] * masAxes[1].z + afViewOrigin[2] * masAxes[2].z) / (1 << RASTER_AXIS_BITS);
	for (k = 0; k < 3; k++)
	{
		afDirection[k] = 0.0;
		for (i = 0; i < 3; i++)
		{
			afDirection[k] += afViewDirection[i] * (float64)((k == 0) ? masAxes[i].x : ((k == 1) ? masAxes[i].y : masAxes[i].z)) / (1 << RASTER_AXIS_BITS);
		}
	}

	afMin[0] = psMin->x;
	afMin[1] = psMin->y;
	afMin[2] = psMin->z;
	afMax[0] = psMax->x;
	afMax[1] = psMax->y;
	afMax[2] = psMax->z;

	//The ray against each pair of the box's planes.
	fNear = 0.0;
	fFar = 1e30;
	for (k = 0; k < 3; k++)
	{
		if (afDirection[k] == 0.0)
		{
			if ((afOrigin[k] < afMin[k]) || (afOrigin[k] > afMax[k]))
			{
				return false;
			}
		}
		else
		{
			fA = (afMin[k] - afOrigin[k]) / afDirection[k];
			fB = (afMax[k] - afOrigin[k]) / afDirection[k];
			if (fA > fB)
			{
				fSwap = fA;
				fA = fB;
				fB = fSwap;
			}
			if (fA > fNear)
			{
				fNear = fA;
			}
			if (fB < fFar)
			{
				fFar = fB;
			}
			if (fNear > fFar)
			{
				return false;
			}
		}
	}

	msPivot.Init((int32)(afOrigin[0] + afDirection[0] * fNear), (int32)(afOrigin[1] + afDirection[1] * fNear), (int32)(afOrigin[2] + afDirection[2] * fNear));
	msPan.Init(0, 0);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Prepare(int32 iWidth, int32 iHeight)
{
	masAxes[0].Init(FixedFromFloat(msRotation.x.x, RASTER_AXIS_BITS), FixedFromFloat(msRotation.y.x, RASTER_AXIS_BITS), FixedFromFloat(msRotation.z.x, RASTER_AXIS_BITS));
	masAxes[1].Init(FixedFromFloat(msRotation.x.y, RASTER_AXIS_BITS), FixedFromFloat(msRotation.y.y, RASTER_AXIS_BITS), FixedFromFloat(msRotation.z.y, RASTER_AXIS_BITS));
	masAxes[2].Init(FixedFromFloat(msRotation.x.z, RASTER_AXIS_BITS), FixedFromFloat(msRotation.y.z, RASTER_AXIS_BITS), FixedFromFloat(msRotation.z.z, RASTER_AXIS_BITS));

	miOrthographicScale = ((int64)miFocalLength << 32) / miDistance;
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

	sPosition.Init((int64)psPosition->x - msPivot.x, (int64)psPosition->y - msPivot.y, (int64)psPosition->z - msPivot.z);
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

