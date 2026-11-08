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
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif
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

//What FitOrbit looks at: enough turns of the box to find its furthest corners within a fraction of a pixel.
#define RASTER_FIT_PASSES			8
#define RASTER_FIT_YAWS				72


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
	msOrthographicScale = SFixed<int64, int64, 32>::FromRaw(0);
	msScreenCentre.Init(SRasterSubpixel::FromRaw(0), SRasterSubpixel::FromRaw(0));
	masAxes[0].Init(SRasterAxis::FromInt(1), SRasterAxis::FromRaw(0), SRasterAxis::FromRaw(0));
	masAxes[1].Init(SRasterAxis::FromRaw(0), SRasterAxis::FromInt(1), SRasterAxis::FromRaw(0));
	masAxes[2].Init(SRasterAxis::FromRaw(0), SRasterAxis::FromRaw(0), SRasterAxis::FromInt(1));
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
// The pivot, focal length and pan that keep the box on the screen, but for
// iMargin pixels each side, at every yaw with this pitch, turning around
// its middle.  The eye stays where it is, so the perspective doesn't change.
// It sits in the middle up and down, and turning about the middle leaves it
// there across.  Panning changes how big its near corners look, so it takes
// a few goes to settle.  It leaves the camera at a yaw of 0.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::FitOrbit(SInt32Vec3* psMin, SInt32Vec3* psMax, float32 fPitch, int32 iWidth, int32 iHeight, int32 iMargin)
{
	SRasterScreenVertex	sScreen;
	SInt32Vec3			sCorner;
	int32				iLeft;
	int32				iRight;
	int32				iTop;
	int32				iBottom;
	int64				iAcross;
	int64				iDown;
	int64				iRoomAcross;
	int64				iRoomDown;
	int					iPass;
	int					iYaw;
	int					i;

	msPivot.Init((int32)(((int64)psMin->x + psMax->x) / 2), (int32)(((int64)psMin->y + psMax->y) / 2), (int32)(((int64)psMin->z + psMax->z) / 2));
	msPan.Init(0, 0);
	Resize(iHeight);
	iRoomAcross = SRasterSubpixel::FromInt(iWidth - 2 * iMargin).miRaw;
	iRoomDown = SRasterSubpixel::FromInt(iHeight - 2 * iMargin).miRaw;
	for (iPass = 0; iPass < RASTER_FIT_PASSES; iPass++)
	{
		iLeft = RASTER_MAX_SCREEN;
		iTop = RASTER_MAX_SCREEN;
		iRight = -RASTER_MAX_SCREEN;
		iBottom = -RASTER_MAX_SCREEN;
		for (iYaw = 0; iYaw < RASTER_FIT_YAWS; iYaw++)
		{
			SetOrbit((float32)iYaw * 6.28318530718f / RASTER_FIT_YAWS, fPitch);
			Prepare(iWidth, iHeight);
			for (i = 0; i < 8; i++)
			{
				sCorner.Init((i & 1) ? psMax->x : psMin->x, (i & 2) ? psMax->y : psMin->y, (i & 4) ? psMax->z : psMin->z);
				Project(&sScreen, &sCorner);
				iLeft = (sScreen.sPosition.x.miRaw < iLeft) ? sScreen.sPosition.x.miRaw : iLeft;
				iTop = (sScreen.sPosition.y.miRaw < iTop) ? sScreen.sPosition.y.miRaw : iTop;
				iRight = (sScreen.sPosition.x.miRaw > iRight) ? sScreen.sPosition.x.miRaw : iRight;
				iBottom = (sScreen.sPosition.y.miRaw > iBottom) ? sScreen.sPosition.y.miRaw : iBottom;
			}
		}

		//Across is from the middle, so that the box turns in the middle of the screen.
		iAcross = (iRight - msScreenCentre.x.miRaw > msScreenCentre.x.miRaw - iLeft) ? 2 * ((int64)iRight - msScreenCentre.x.miRaw) : 2 * ((int64)msScreenCentre.x.miRaw - iLeft);
		iDown = (int64)iBottom - iTop;
		if ((iAcross <= 0) || (iDown <= 0))
		{
			break;
		}

		//A pixel is the eye's distance over the focal length of the mesh's units.
		msPan.y += (int32)((((int64)iTop + iBottom) / 2 - msScreenCentre.y.miRaw) * miDistance / ((int64)miFocalLength << SRasterSubpixel::Bits));
		if (iRoomAcross * iDown < iRoomDown * iAcross)
		{
			miFocalLength = (int32)((int64)miFocalLength * iRoomAcross / iAcross);
		}
		else
		{
			miFocalLength = (int32)((int64)miFocalLength * iRoomDown / iDown);
		}
	}
	SetOrbit(0.0f, fPitch);
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

	fRight = (SRasterSubpixel::FromInt(iX) + SRasterSubpixel::FromRaw(8) - msScreenCentre.x).ToDouble();
	fUp = (msScreenCentre.y - SRasterSubpixel::FromInt(iY) - SRasterSubpixel::FromRaw(8)).ToDouble();

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

	afOrigin[0] = msPivot.x + afViewOrigin[0] * masAxes[0].x.ToDouble() + afViewOrigin[1] * masAxes[1].x.ToDouble() + afViewOrigin[2] * masAxes[2].x.ToDouble();
	afOrigin[1] = msPivot.y + afViewOrigin[0] * masAxes[0].y.ToDouble() + afViewOrigin[1] * masAxes[1].y.ToDouble() + afViewOrigin[2] * masAxes[2].y.ToDouble();
	afOrigin[2] = msPivot.z + afViewOrigin[0] * masAxes[0].z.ToDouble() + afViewOrigin[1] * masAxes[1].z.ToDouble() + afViewOrigin[2] * masAxes[2].z.ToDouble();
	for (k = 0; k < 3; k++)
	{
		afDirection[k] = 0.0;
		for (i = 0; i < 3; i++)
		{
			afDirection[k] += afViewDirection[i] * ((k == 0) ? masAxes[i].x : ((k == 1) ? masAxes[i].y : masAxes[i].z)).ToDouble();
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
	masAxes[0].Init(SRasterAxis::FromFloat(msRotation.x.x), SRasterAxis::FromFloat(msRotation.y.x), SRasterAxis::FromFloat(msRotation.z.x));
	masAxes[1].Init(SRasterAxis::FromFloat(msRotation.x.y), SRasterAxis::FromFloat(msRotation.y.y), SRasterAxis::FromFloat(msRotation.z.y));
	masAxes[2].Init(SRasterAxis::FromFloat(msRotation.x.z), SRasterAxis::FromFloat(msRotation.y.z), SRasterAxis::FromFloat(msRotation.z.z));

	msOrthographicScale = SFixed<int64, int64, 32>::FromRaw(((int64)miFocalLength << 32) / miDistance);
	msScreenCentre.Init(SRasterSubpixel::FromRaw(iWidth * 8), SRasterSubpixel::FromRaw(iHeight * 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SRasterSubpixel RasterClampScreen(SFixed<int64, int64, 4> s)
{
	if (s.miRaw > RASTER_MAX_SCREEN)
	{
		return SRasterSubpixel::FromRaw(RASTER_MAX_SCREEN);
	}
	if (s.miRaw < -RASTER_MAX_SCREEN)
	{
		return SRasterSubpixel::FromRaw(-RASTER_MAX_SCREEN);
	}
	return SRasterSubpixel::FromRaw((int32)s.miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER) && defined(_M_X64)
#define RASTER_MULTIPLY_HIGH(a, b)	__umulh(a, b)
#elif defined(__SIZEOF_INT128__)
#define RASTER_MULTIPLY_HIGH(a, b)	((uint64)(((unsigned __int128)(a) * (b)) >> 64))
#endif


//////////////////////////////////////////////////////////////////////////
//
// A vertex divides three numbers by the same distance.  One real division
// makes this, and RasterDivideFloor makes each of them from it.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE uint64 RasterDivideInverse(uint64 uiDivisor)
{
#ifdef RASTER_MULTIPLY_HIGH
	return ~(uint64)0 / uiDivisor;
#else
	return 0;
#endif
}


//////////////////////////////////////////////////////////////////////////
//
// Exactly what DivideFloor gives, for a divisor above 0.  Multiplying by the
// inverse can only come out a little low, and the remainder corrects that.
// Where there's no multiply that gives the high 64 bits this just divides.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE int64 RasterDivideFloor(int64 iNumerator, uint64 uiDivisor, uint64 uiInverse)
{
#ifdef RASTER_MULTIPLY_HIGH
	uint64	uiNumerator;
	uint64	uiQuotient;
	uint64	uiRemainder;

	uiNumerator = (iNumerator < 0) ? (uint64)0 - (uint64)iNumerator : (uint64)iNumerator;
	uiQuotient = RASTER_MULTIPLY_HIGH(uiNumerator, uiInverse);
	uiRemainder = uiNumerator - uiQuotient * uiDivisor;
	while (uiRemainder >= uiDivisor)
	{
		uiQuotient++;
		uiRemainder -= uiDivisor;
	}
	if (iNumerator >= 0)
	{
		return (int64)uiQuotient;
	}
	return -(int64)uiQuotient - ((uiRemainder != 0) ? 1 : 0);
#else
	(void)uiInverse;
	return DivideFloor(iNumerator, (int64)uiDivisor);
#endif
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int32 RasterDot(SInt32Vec3* psPosition, SRasterAxisVec3* psAxis)
{
	return (SRasterWhole::FromRaw(psPosition->x) * psAxis->x + SRasterWhole::FromRaw(psPosition->y) * psAxis->y + SRasterWhole::FromRaw(psPosition->z) * psAxis->z).Floor();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::Project(SRasterScreenVertex* psDest, SInt32Vec3* psPosition)
{
	typedef SFixed<int64, int64, 0>		SWide;
	typedef SFixed<int64, int64, 4>		SWideSubpixel;

	SInt32Vec3		sPosition;
	int32			x;
	int32			y;
	int32			z;
	int64			iEyeDistance;
	uint64			uiInverse;
	SWideSubpixel	sCentreX;
	SWideSubpixel	sCentreY;
	SWideSubpixel	sScreenX;
	SWideSubpixel	sScreenY;

	Int32Vec3Subtract(&sPosition, psPosition, &msPivot);
	x = RasterDot(&sPosition, &masAxes[0]) + msPan.x;
	y = RasterDot(&sPosition, &masAxes[1]) + msPan.y;
	z = RasterDot(&sPosition, &masAxes[2]);

	sCentreX = msScreenCentre.x.To<SWideSubpixel>();
	sCentreY = msScreenCentre.y.To<SWideSubpixel>();

	if (meProjection == RP_Perspective)
	{
		iEyeDistance = (int64)miDistance - z;
		if (iEyeDistance < RASTER_NEAR_DISTANCE)
		{
			psDest->iDepth = 0;
			return;
		}
		sScreenX = (SRasterWhole::FromRaw(x) * SRasterWhole::FromRaw(miFocalLength)).To<SWideSubpixel>();
		sScreenY = (SRasterWhole::FromRaw(y) * SRasterWhole::FromRaw(miFocalLength)).To<SWideSubpixel>();
		uiInverse = RasterDivideInverse((uint64)iEyeDistance);
		psDest->sPosition.x = RasterClampScreen(SWideSubpixel::FromRaw(RasterDivideFloor(sScreenX.miRaw, (uint64)iEyeDistance, uiInverse)) + sCentreX);
		psDest->sPosition.y = RasterClampScreen(sCentreY - SWideSubpixel::FromRaw(RasterDivideFloor(sScreenY.miRaw, (uint64)iEyeDistance, uiInverse)));
		psDest->iDepth = (int32)RasterDivideFloor(RASTER_DEPTH_NUMERATOR, (uint64)iEyeDistance, uiInverse);
	}
	else
	{
		sScreenX = (SWide::FromRaw(x) * msOrthographicScale).ToFloor<SWideSubpixel>();
		sScreenY = (SWide::FromRaw(y) * msOrthographicScale).ToFloor<SWideSubpixel>();
		psDest->sPosition.x = RasterClampScreen(sScreenX + sCentreX);
		psDest->sPosition.y = RasterClampScreen(sCentreY - sScreenY);
		psDest->iDepth = z + RASTER_ORTHOGRAPHIC_DEPTH;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Rotates a direction on the screen back into the mesh, keeping its fixed point.
//
//////////////////////////////////////////////////////////////////////////
void CRasterCamera::ViewToMesh(SRasterUnitVec3* psDest, SRasterUnitVec3* psView)
{
	typedef SFixed<int32, int64, 14>	SWideUnit;

	SWideUnit	sX;
	SWideUnit	sY;
	SWideUnit	sZ;

	sX = psView->x.To<SWideUnit>();
	sY = psView->y.To<SWideUnit>();
	sZ = psView->z.To<SWideUnit>();
	psDest->x = (sX * masAxes[0].x + sY * masAxes[1].x + sZ * masAxes[2].x).ToFloor<SRasterUnit>();
	psDest->y = (sX * masAxes[0].y + sY * masAxes[1].y + sZ * masAxes[2].y).ToFloor<SRasterUnit>();
	psDest->z = (sX * masAxes[0].z + sY * masAxes[1].z + sZ * masAxes[2].z).ToFloor<SRasterUnit>();
}

