/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SceneLib

Codaphela SceneLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SceneLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SceneLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SCENE_H__
#define __SCENE_H__
#include "BaseLib/ChunkFile.h"
#include "Image.h"
#include "ImageCelGroup.h"
#include "Mesh.h"
#include "Light.h"
#include "Camera.h"
#include "Connection.h"
#include "ConnectionAnimation.h"
#include "Instance.h"
#include "Material.h"
#include "FakeTracker.h"


class CScene
{
public:
	int					iUniqueID;

	CFakeTracker		mcConnectionTracker;
	CFakeTracker		mcMeshTracker;
	CFakeTracker		mcCameraTracker;
	CFakeTracker		mcLightTracker;
	CFakeTracker		mcSequenceTracker;
	CFakeTracker		mcImageTracker;
	CFakeTracker		mcMaterialTracker;
	CFakeTracker		mcInstanceTracker;

	void Init(void);
	bool Load(CFileReader* pcChunkFile);
	void Kill(void);
	bool Touch(void);
};


#endif // __SCENE_H__

