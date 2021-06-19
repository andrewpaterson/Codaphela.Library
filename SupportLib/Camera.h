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
#include "BaseLib/ChunkFile.h"
#include "StandardLib/NamedObject.h"


enum ECameraType
{
	CT_Unknown,
	CT_Perspective,
	CT_Orthonormal,
};


class CCamera : public CNamedObject
{
CONSTRUCTABLE(CCamera); 
public:
	ECameraType		meCameraType;
	float			mfFOV;

	void 	Init(void);
	void 	Init(ECameraType eCameraType, float fFOV);
	void 	Free(void);

	BOOL	Load(CObjectReader* pcFile);
	BOOL	Save(CObjectWriter* pcFile);
	void	Copy(CCamera* pcCamera);
};


#endif // !__CAMERA_H__

