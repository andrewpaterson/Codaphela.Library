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
#ifndef __TURNTABLE_H__
#define __TURNTABLE_H__
#include "BaseLib/Float32Vec3.h"
#include "BaseLib/Float4x4.h"


#define TURNTABLE_FOV				0.9f	//Radians across the width, when nothing says otherwise.
#define TURNTABLE_ORBIT_RADIANS		0.01f	//For each pixel dragged.
#define TURNTABLE_PITCH_LIMIT		1.55f	//Radians up or down, short of straight, where up would flip.


//A view that turns about a point by its yaw and pitch, so up (y) stays up: where it looks at, from which way and how far,
//and how wide it sees.  It knows nothing of what draws it: World is the camera's matrix, and the pixels it's told of are
//the view's.
struct STurntable
{
	SFloat32Vec3	sTarget;  //What the view turns about.
	float32			fYaw;  //Radians about y, 0 looking along -z.
	float32			fPitch;  //Radians up from level, the view looking down.
	float32			fDistance;  //From the target.
	float32			fFOV;  //Radians across the width.

	void	Init(void);
	void	Frame(SFloat32Vec3* psCentre, float32 fRadius);
	bool	From(SFloat4x4* psWorld, float32 fCameraFOV, SFloat32Vec3* psCentre, float32 fRadius);

	void	Orbit(int32 iRight, int32 iDown);
	void	Pan(int32 iRight, int32 iDown, int32 iWidth);
	void	Zoom(float32 fFactor);

	void	World(SFloat4x4* psWorld);
	bool	ScreenToPlane(int32 x, int32 y, int32 iWidth, int32 iHeight, SFloat32Vec3* psDest);
};


#endif // __TURNTABLE_H__

