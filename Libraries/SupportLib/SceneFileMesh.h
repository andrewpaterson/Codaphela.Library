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


typedef CArrayTemplatePrimitive<int32>	CArraySceneFileIndex;


//Fills an empty editor, and its mesh, from a mesh in a scene file.  A polygon is cut into faces with the edges inside it hidden, so that the
//editor still sees one polygon.  papMaterials are the file's materials, which the mesh's slots refer to.
//
//A CMesh doesn't hold smoothing groups, so they go in a CMeshSmoothGroups on the editor's stack.  If the file has no normals then the mesh gets
//them from its groups, which is what they're for.  Skin weights are put in the order CMeshSkinVert needs them.
bool	ReadSceneFileMesh(CSceneFileReader* pcReader, size uiMesh, CMeshEditor* pcEditor, CArrayMaterial* papMaterials);

//Writes an editor's mesh.  A polygon of the editor's is written as one, unless one polygon couldn't say what its faces do, as with a seam across
//it, when its faces are.  It starts at its lowest numbered corner.  paiMaterials has the index in the file of each of the mesh's materials.
bool	WriteSceneFileMesh(CSceneFileWriter* pcWriter, CMeshEditor* pcEditor, CArraySceneFileIndex* paiMaterials);


#endif // __SCENE_FILE_MESH_H__

