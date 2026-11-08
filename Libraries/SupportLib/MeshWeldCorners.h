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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __MESH_WELD_CORNERS_H__
#define __MESH_WELD_CORNERS_H__
#include "MeshModifier.h"


//Makes face corners in exactly the same place and of the same colour one corner, so that faces exported from CAD as
//separate triangles touch.  Nothing moves and each face keeps its own normals, so a sharp edge stays sharp.  A corner
//used by faces of different colours becomes one for each colour, as they are different surfaces.  A face left with no
//area because its corners became one goes.
class CMeshWeldCorners : public CMeshModifier
{
CONSTRUCTABLE(CMeshWeldCorners);
public:
	void		Init(void);

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;
};


#endif // __MESH_WELD_CORNERS_H__
