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
#ifndef __OLD_DRAGON_READER_H__
#define __OLD_DRAGON_READER_H__
#include "OldDragonFile.h"
#include "OldDragonScene.h"


//Reads any old .DRG into pcScene, which must already be Init'ed and is the caller's to Kill, even on failure.
bool	ReadOldDragonFile(const char* szFilename, COldDragonScene* pcScene);

bool	ReadOldDragonIOX(COldDragonFile* pcFile, COldDragonScene* pcScene);
bool	ReadOldDragon2009(COldDragonFile* pcFile, COldDragonScene* pcScene);
bool	ReadOldDragonMax(COldDragonFile* pcFile, COldDragonScene* pcScene);

//Adds the points in a Viewer particle file to pcScene.  These are CSV files named by the Viewer's Config.txt, and a scene like
//Goober's star field is only complete with them.  The headings X and Y are required.  Z, R, G, B, Radius and Intensity are
//optional, in any order.  The Viewer's defaults for missing columns aren't known: Z is 0 and the others are 1 here.
bool	ReadOldDragonParticles(const char* szFilename, COldDragonScene* pcScene);

//Adds the Editable Polys in a 3ds Max .max file to pcScene as meshes, with triangles, UVs, smoothing and named materials, turned
//to y up as the exporter did.  Nothing places them, and Editable Meshes and anything else are counted in muiSkippedChunks.
bool	ReadOldDragonMaxScene(const char* szFilename, COldDragonScene* pcScene);

//Gives meshes with no faces, as all of Melee.DRG's are, the faces of the mesh in a .max file with the same positions in the same
//order.  Materials are matched by name.  Not filling a mesh isn't a failure, so check *puiMeshesFilled.
bool	AddOldDragonMaxFaces(const char* szMaxFilename, COldDragonScene* pcScene, size* puiMeshesFilled);


#endif // __OLD_DRAGON_READER_H__

