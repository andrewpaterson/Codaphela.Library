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
#ifndef __MESH_REMOVE_DEGENERATE_H__
#define __MESH_REMOVE_DEGENERATE_H__
#include "MeshModifier.h"


//Takes out the faces with no area: two of whose corners are one, or in the same place, or whose corners are in a line.
//CMeshWeldCorners already takes out a face whose corners it makes one, but not one whose corners aren't the same colour or
//that is a line, which what's exported from CAD and what's simplified both have.  A face that only loses its area when it's
//rounded to a renderer's units is the renderer's to take out, as how big a unit is is up to it (see BuildRasterMesh).  Only
//the corners and normals still used are kept.
class CMeshRemoveDegenerate : public CMeshModifier
{
CONSTRUCTABLE(CMeshRemoveDegenerate);
protected:
	size		muiNumRemoved;

public:
	void		Init(void);

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;

	size		NumRemoved(void);

	static bool	IsDegenerate(CMesh* pcMesh, int iFace);
};


#endif // __MESH_REMOVE_DEGENERATE_H__
