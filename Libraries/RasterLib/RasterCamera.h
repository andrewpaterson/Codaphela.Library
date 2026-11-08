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
#include "BaseLib/Float4x4.h"
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


//The eye looks along -Z from miDistance away, with the mesh turned around msPivot by a rotation held in BaseLib's float
//matrix.  Prepare converts it to fixed point once a frame and Project is then entirely integer.
//
//Drag, Pan, PanStep, Zoom and LookAtScreen move the camera as KiCad's 3D viewer does: a virtual trackball, a pan that
//keeps what is at the pivot's depth under the pointer, and zooming by moving the eye (so a very close eye can cut
//off triangles, see Project).
//
//FitOrbit is for a view that spins by itself: a box turns around its middle with SetOrbit's yaw and stays on the screen.
class CRasterCamera
{
protected:
	ERasterProjection	meProjection;
	SFloat4x4			msRotation;
	SInt32Vec3			msPivot;  //The point in the mesh the rotation turns it around.
	SInt32Vec2			msPan;
	int32				miDistance;
	int32				miInitialDistance;  //What the limits on zooming are relative to.
	int32				miFocalLength;  //Pixels.
	int64				miOrthographicScale;  //Pixels per unit with 32 fraction bits.  At the pivot both projections are the same size.
	SInt32Vec2			msScreenCentre;
	SInt64Vec3			masAxes[3];  //What the mesh's position is dotted with for the screen's x (right), y (up) and z (out).

public:
	void				Init(void);

	void				SetProjection(ERasterProjection eProjection);
	void				SetOrbit(float32 fYaw, float32 fPitch);
	void				SetPan(int32 iX, int32 iY);
	void				SetPerspective(int32 iDistance, int32 iFocalLength);
	void				Fit(int32 iWidth, int32 iHeight);
	void				FitOrbit(SInt32Vec3* psMin, SInt32Vec3* psMax, float32 fPitch, int32 iWidth, int32 iHeight, int32 iMargin);
	void				Resize(int32 iHeight);

	void				Drag(int32 iLastX, int32 iLastY, int32 iX, int32 iY, int32 iWidth, int32 iHeight);
	void				Pan(int32 iRight, int32 iDown);
	void				PanStep(int32 iRight, int32 iUp);
	void				Zoom(float32 fFactor);
	bool				LookAtScreen(int32 iX, int32 iY, SInt32Vec3* psMin, SInt32Vec3* psMax);

	void				Prepare(int32 iWidth, int32 iHeight);
	void				Project(SRasterScreenVertex* psDest, SInt32Vec3* psPosition);
	void				ViewToMesh(SInt64Vec3* psDest, SInt64Vec3* psView);
};


#endif // __RASTER_CAMERA_H__

