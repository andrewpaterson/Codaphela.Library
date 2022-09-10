/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __MESH_SHAPE_H__
#define __MESH_SHAPE_H__
#include "MeshTriangles.h"
#include "MeshLeaves.h"
#include "MeshBoundingBox.h"
#include "MeshConvexHull.h"


class CMesh;
class CMeshShape : public CUnknown
{
CONSTRUCTABLE(CMeshShape);
public:
	CMesh*				mpcMesh;
	CMeshTriangles		mcTriangles;		//Touched, Depends: Normals
	CMeshLeaves			mcLeaves;			//Touched, Depends: Triangles
	CMeshBoundingBox	mcBoundingBox;		//Touched, Optional Depends: Hull, Incompatible: Skin
	CMeshConvexHull		mcHull;				//Touched, Incompatible: Skin

	void 	Init(CMesh* pcMesh);
	void 	Kill(void);

	bool	Load(CFileReader* pcFile);
	bool	Save(CFileWriter* pcFile);
};


#endif // !__MESH_SHAPE_H__

