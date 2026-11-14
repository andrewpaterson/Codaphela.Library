/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela OldDragonLib

Codaphela OldDragonLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela OldDragonLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela OldDragonLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __OLD_DRAGON_CONVERTER_H__
#define __OLD_DRAGON_CONVERTER_H__
#include "OldDragonScene.h"


//Writes an old scene as a scene file (see SupportLib's SceneFileGeneral.h).  What the old files got wrong or left strange stops here:
//
//	Each mesh placed in the scene is an instance of its own.  Nothing in the old files says which meshes make up one thing.
//	Faces are polygons of 3, with each mesh having its own list of the materials it uses.
//	Pixels are RGB or RGBA, and a single channel is called A.  Into Oblivion and the 2009 files hold BGR and BGRA, and the Max
//	exporter's already hold RGB and RGBA.  All have the top row first.
//	A key's time is from the start of its animation.  The old files hold the time since the previous key, and repeat the last key.
//	A key is the connection's local matrix, relative to its parent's at that time.  The old files hold world matrices.
//	A camera's field of view is horizontal, in radians.  The 2009 and Max files hold degrees, and Into Oblivion 3/4 of it in radians.
//	A camera looks along its connection's -z with y up, and a light shines along -z.  The exporter's looked along their -y with -z
//	up.  Into Oblivion's one camera is a mirrored matrix that its game replaced when it ran.
//	A light whose name says it's directional is, as the Max exporter called everything that wasn't an omni a spot.
//	An Into Oblivion light's intensity is 1: its colour says how bright it is, and it has no intensity of its own.
//
//Lights and cameras aren't instanced: one placed many times is many, each attached to its connection.  One placed nowhere is
//attached to nothing.
//
//A sequence moves the connection with its name, and failing to find one fails.  Connections are written by their place in the
//scene's list, where the old files numbered them in a way that in Goober isn't that place.
//
//A skinned mesh's instance's connections are a skeleton's bones, in the scene's order, and its instance has them in that order too.
//The sequences moving them are the skeleton's animation, and one of those moving a connection that isn't a bone, with nothing placed
//on it or under it, is a helper the exporter wrote, as Goober's IK chains and finger helpers are: it's left out, with its connection,
//as the bones' keys already hold what it did.  A bone's parent is the nearest bone above it, and a root bone's the nearest kept
//connection above it.
bool	WriteOldDragonSceneFile(COldDragonScene* pcScene, const char* szFilename);


#endif // __OLD_DRAGON_CONVERTER_H__

