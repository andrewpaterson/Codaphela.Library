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
#ifndef __INSTANCE_H__
#define __INSTANCE_H__
#include "BaseLib/Chars.h"
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "Connection.h"
#include "Material.h"
#include "MeshEditor.h"


//A mesh in an instance.  Its editor is shared with other instances, and so can't point at connections, as each instance has its own.
//mapConnections are the instance's connections that the mesh's matrices use, in the mesh's order: one for a rigid mesh, and one for
//each of its matrices for a skinned one.  They must be the instance's own, as a file holds them as places in its list.  mapMaterials
//replace the mesh's defaults, NULL to keep one, and none to keep them all.
class CInstanceMesh : public CObject
{
CONSTRUCTABLE(CInstanceMesh);
DESTRUCTABLE(CInstanceMesh);
public:
	Ptr<CMeshEditor>	mpEditor;
	CArray<CConnection>	mapConnections;
	CArrayMaterial		mapMaterials;

	void 	Init(Ptr<CMeshEditor> pEditor);
	void 	Free(void) override;
	void	Class(void) override;
};


//One thing in a game, one character say.  Two characters are two instances that share their meshes and are drawn in different places.
//The order of its connections matters, as its meshes refer to them by their place.
class CInstance : public CObject
{
CONSTRUCTABLE(CInstance);
DESTRUCTABLE(CInstance);
public:
	CChars					mszName;
	CArray<CConnection>		mapConnections;
	CArray<CInstanceMesh>	mapMeshes;

	void 	Init(void);
	void 	Free(void) override;
	void	Class(void) override;
};


#endif // __INSTANCE_H__

