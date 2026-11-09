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
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "Camera.h"
#include "ConnectionAnimation.h"
#include "Instance.h"
#include "Light.h"
#include "SceneParticles.h"


//What's in a world.  Meshes, materials, images and connections are reached through what uses them, so one nothing uses isn't here, and isn't
//kept from a file that has it.  ReadFile adds to what's here.  WriteFile fails, leaving no file, if a scene file can't hold what's here.
//UpdateConnections makes every connection's world matrix from its local one and its parents'.
class CScene : public CObject
{
CONSTRUCTABLE(CScene);
DESTRUCTABLE(CScene);
public:
	CArray<CInstance>		mapInstances;
	CArray<CLight>			mapLights;
	CArray<CCamera>			mapCameras;
	CArray<CSceneAnimation>	mapAnimations;
	CArray<CSceneParticles>	mapParticles;

protected:
	size					muiUpdates;

public:
	void 	Init(void);
	void 	Free(void) override;
	void	Class(void) override;

	bool	ReadFile(const char* szFilename);
	bool	WriteFile(const char* szFilename);

	void	UpdateConnections(void);
};


#endif // __SCENE_H__

