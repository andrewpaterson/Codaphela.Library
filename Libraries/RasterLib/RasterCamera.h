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
#ifndef __RASTER_CAMERA_H__
#define __RASTER_CAMERA_H__
#include "BaseLib/Int32Vec2.h"
#include "BaseLib/Int32Vec3.h"
#include "BaseLib/Int64Vec3.h"


//Screen positions are fixed point: 16 subpixels to a pixel, y down.
#define RASTER_SUBPIXEL_BITS	4
#define RASTER_SUBPIXELS		(1 << RASTER_SUBPIXEL_BITS)

//The camera's axes are fixed point: 1.0 is 65536.
#define RASTER_AXIS_BITS		16


enum ERasterProjection
{
	RP_Perspective,
	RP_Orthographic,
};


//A vertex on the screen.  A larger iDepth is nearer the viewer and an iDepth of 0 is not drawable (behind the near plane).
struct SRasterScreenVertex
{
	SInt32Vec2	sPosition;
	int32		iDepth;
	int32		iShade;  //0 .. 256.
};


//Yaw turns the mesh around its Z axis and pitch then tilts it around the screen's X axis, using BaseLib's float matrices.
//Prepare converts the result to fixed point once a frame.  Project is entirely integer.
class CRasterCamera
{
protected:
	ERasterProjection	meProjection;
	float32				mfYaw;
	float32				mfPitch;
	SInt32Vec2			msPan;
	int32				miDistance;
	int32				miFocalLength;  //Pixels.
	int64				miOrthographicScale;  //Pixels per unit with 32 fraction bits.
	SInt32Vec2			msScreenCentre;
	SInt64Vec3			masAxes[3];  //What the mesh's position is dotted with for the screen's x (right), y (up) and z (out).

public:
	void				Init(void);

	void				SetProjection(ERasterProjection eProjection);
	void				SetOrbit(float32 fYaw, float32 fPitch);
	void				SetPan(int32 iX, int32 iY);
	void				SetPerspective(int32 iDistance, int32 iFocalLength);
	void				SetOrthographic(int64 iScale);
	void				Fit(int32 iWidth, int32 iHeight);

	void				Prepare(int32 iWidth, int32 iHeight);
	void				Project(SRasterScreenVertex* psDest, SInt32Vec3* psPosition);
	void				ViewToMesh(SInt64Vec3* psDest, SInt64Vec3* psView);
};


#endif // __RASTER_CAMERA_H__

