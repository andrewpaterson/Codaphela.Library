/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SceneViewLib

Codaphela SceneViewLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SceneViewLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SceneViewLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "Turntable.h"


//////////////////////////////////////////////////////////////////////////
//
// A unit from the origin, looking along -z.
//
//////////////////////////////////////////////////////////////////////////
void STurntable::Init(void)
{
	sTarget.Init(0, 0, 0);
	fYaw = 0;
	fPitch = 0;
	fDistance = 1.0f;
	fFOV = TURNTABLE_FOV;
}


//////////////////////////////////////////////////////////////////////////
//
// Looking along -z at psCentre from far enough away to see all of a
// sphere of fRadius about it across the shorter way of a 4 by 3 view.
//
//////////////////////////////////////////////////////////////////////////
void STurntable::Frame(SFloat32Vec3* psCentre, float32 fRadius)
{
	float32	fHalfFOV;

	fFOV = TURNTABLE_FOV;
	fHalfFOV = atanf(tanf(fFOV * 0.5f) * 3.0f / 4.0f);
	fYaw = 0;
	fPitch = 0;
	fDistance = fRadius / sinf(fHalfFOV);
	sTarget = *psCentre;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a camera with the matrix psWorld is, with its field of view,
// looking the way it does about the point in front of it nearest
// psCentre (fRadius in front if that's behind it), its roll left out as
// up stays up.  False if the matrix has no z.
//
//////////////////////////////////////////////////////////////////////////
bool STurntable::From(SFloat4x4* psWorld, float32 fCameraFOV, SFloat32Vec3* psCentre, float32 fRadius)
{
	SFloat32Vec3	sBack;
	SFloat32Vec3	sEye;
	SFloat32Vec3	sToCamera;
	float32			fLength;

	sBack.Init(psWorld->z.x, psWorld->z.y, psWorld->z.z);
	fLength = sBack.Magnitude();
	if (fLength <= 0)
	{
		return false;
	}
	sBack = sBack / fLength;
	sEye.Init(psWorld->pos.x, psWorld->pos.y, psWorld->pos.z);

	sToCamera = sEye - *psCentre;
	fDistance = Float32Vec3Dot(&sToCamera, &sBack);
	if (fDistance <= 0)
	{
		fDistance = fRadius;
	}
	fFOV = fCameraFOV;
	fPitch = asinf(fmaxf(-1.0f, fminf(1.0f, sBack.y)));
	fYaw = atan2f(sBack.x, sBack.z);
	sTarget = sEye - (sBack * fDistance);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Dragging right turns what's seen right, and dragging down tips it
// towards the view.
//
//////////////////////////////////////////////////////////////////////////
void STurntable::Orbit(int32 iRight, int32 iDown)
{
	fYaw -= iRight * TURNTABLE_ORBIT_RADIANS;
	fPitch += iDown * TURNTABLE_ORBIT_RADIANS;
	fPitch = fmaxf(-TURNTABLE_PITCH_LIMIT, fminf(TURNTABLE_PITCH_LIMIT, fPitch));
}


//////////////////////////////////////////////////////////////////////////
//
// What's seen follows the pointer in a view iWidth pixels wide: a pixel
// is what a pixel covers at the target.
//
//////////////////////////////////////////////////////////////////////////
void STurntable::Pan(int32 iRight, int32 iDown, int32 iWidth)
{
	SFloat4x4	sWorld;
	float32		fPerPixel;

	if (iWidth <= 0)
	{
		return;
	}
	World(&sWorld);
	fPerPixel = 2.0f * fDistance * tanf(fFOV * 0.5f) / iWidth;
	sTarget.x += (-sWorld.x.x * iRight + sWorld.y.x * iDown) * fPerPixel;
	sTarget.y += (-sWorld.x.y * iRight + sWorld.y.y * iDown) * fPerPixel;
	sTarget.z += (-sWorld.x.z * iRight + sWorld.y.z * iDown) * fPerPixel;
}


//////////////////////////////////////////////////////////////////////////
//
// More than 1 is closer.
//
//////////////////////////////////////////////////////////////////////////
void STurntable::Zoom(float32 fFactor)
{
	fDistance /= fFactor;
}


//////////////////////////////////////////////////////////////////////////
//
// The camera's matrix: its z, which it looks away from, is towards it
// from the target, its x is level and its y is z across x.
//
//////////////////////////////////////////////////////////////////////////
void STurntable::World(SFloat4x4* psWorld)
{
	float32		fCosPitch;
	float32		fBackX;
	float32		fBackY;
	float32		fBackZ;
	float32		fSinYaw;
	float32		fCosYaw;

	fCosPitch = cosf(fPitch);
	fSinYaw = sinf(fYaw);
	fCosYaw = cosf(fYaw);
	fBackX = fCosPitch * fSinYaw;
	fBackY = sinf(fPitch);
	fBackZ = fCosPitch * fCosYaw;

	psWorld->x.Init(fCosYaw, 0, -fSinYaw, 0);
	psWorld->y.Init(-fBackY * fSinYaw, fCosPitch, -fBackY * fCosYaw, 0);
	psWorld->z.Init(fBackX, fBackY, fBackZ, 0);
	psWorld->pos.Init(sTarget.x + fBackX * fDistance, sTarget.y + fBackY * fDistance, sTarget.z + fBackZ * fDistance, 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Where the ray through the middle of the pixel x, y of a view iWidth by
// iHeight meets the plane through the target facing the view: the plane
// of the screen.  The focal length is as the renderers have it, half the
// width over the tangent of half the field of view.  False for a view
// with no size or no field of view.
//
//////////////////////////////////////////////////////////////////////////
bool STurntable::ScreenToPlane(int32 x, int32 y, int32 iWidth, int32 iHeight, SFloat32Vec3* psDest)
{
	SFloat4x4		sWorld;
	SFloat32Vec3	sRight;
	SFloat32Vec3	sUp;
	SFloat32Vec3	sBack;
	SFloat32Vec3	sEye;
	SFloat32Vec3	sRay;
	float32			fFocal;

	if ((iWidth <= 0) || (iHeight <= 0) || (fFOV <= 0.0f))
	{
		return false;
	}

	World(&sWorld);
	sRight.Init(sWorld.x.x, sWorld.x.y, sWorld.x.z);
	sUp.Init(sWorld.y.x, sWorld.y.y, sWorld.y.z);
	sBack.Init(sWorld.z.x, sWorld.z.y, sWorld.z.z);
	sEye.Init(sWorld.pos.x, sWorld.pos.y, sWorld.pos.z);

	//The ray goes fFocal along -z, and the plane is fDistance along -z from the eye.
	fFocal = ((float32)iWidth * 0.5f) / tanf(fFOV * 0.5f);
	sRay = (sRight * ((float32)x + 0.5f - ((float32)iWidth * 0.5f))) + (sUp * (((float32)iHeight * 0.5f) - (float32)y - 0.5f)) - (sBack * fFocal);
	*psDest = sEye + (sRay * (fDistance / fFocal));
	return true;
}

