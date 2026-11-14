/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SCENE_FILE_MESH_H__
#define __SCENE_FILE_MESH_H__
#include "BaseLib/ArrayTemplatePrimitive.h"
#include "Material.h"
#include "MeshEditor.h"
#include "SceneFileReader.h"
#include "SceneFileWriter.h"
#include "Skeleton.h"


//Large: a mesh's corners and faces can pass 64KB with a 16 bit size (W65C816).
typedef CArrayLargeTemplatePrimitive<int32>	CArraySceneFileIndex;


//Fills an empty editor, and its mesh, from a mesh in a scene file.  A polygon is cut into faces with the edges inside it hidden, so that the
//editor still sees one polygon.  papMaterials are the file's materials, which the mesh's slots refer to.
//
//If the file has smoothing groups but no normals then the mesh gets its normals from its groups, which is what they're for.  Skin weights are put
//in the order CMeshSkinVert needs them.  papSkeletons are the file's skeletons, which its skin's bones are in, and can be NULL for a file
//with none.
bool	ReadSceneFileMesh(CSceneFileReader* pcReader, size uiMesh, CMeshEditor* pcEditor, CArrayMaterial* papMaterials, CArray<CSkeleton>* papSkeletons = NULL);

//Writes an editor's mesh.  A polygon of the editor's is written as one, unless one polygon couldn't say what its faces do, as with a seam across
//it, when its faces are.  It starts at its lowest numbered corner.  paiMaterials has the index in the file of each of the mesh's materials,
//and papSkeletons are the skeletons the file has, one of which holds its skin's bones.  False if none does.
bool	WriteSceneFileMesh(CSceneFileWriter* pcWriter, CMeshEditor* pcEditor, CArraySceneFileIndex* paiMaterials, CArray<CSkeleton>* papSkeletons = NULL);

//The place in papSkeletons of the skeleton holding pBone, and the bone's place in it.  False if none holds it.
bool	FindSceneFileBone(CArray<CSkeleton>* papSkeletons, Ptr<CBone> pBone, int32* piSkeleton, int32* piBone);


#endif // __SCENE_FILE_MESH_H__

