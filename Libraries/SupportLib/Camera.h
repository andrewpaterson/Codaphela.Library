/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "BaseLib/Chars.h"
#include "StandardLib/Object.h"
#include "Connection.h"


enum ECameraType
{
	CAT_Unknown,
	CAT_Perspective,
	CAT_Orthonormal,
};


//Looks along its connection's -z with y up.  mfFOV is horizontal, in radians.  mpConnection is NULL if it isn't placed anywhere.
class CCamera : public CObject
{
CONSTRUCTABLE(CCamera);
DESTRUCTABLE(CCamera);
public:
	CChars				mszName;
	ECameraType			meCameraType;
	float32				mfFOV;
	float32				mfNear;
	float32				mfFar;
	Ptr<CConnection>	mpConnection;

	Ptr<CCamera>	Init(void);
	void 			Free(void) override;
	void			Class(void) override;
};


#endif // __CAMERA_H__

