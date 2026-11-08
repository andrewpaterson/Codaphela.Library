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
#ifndef __MESH_EDGE_COLLAPSE_H__
#define __MESH_EDGE_COLLAPSE_H__
#include "MeshModifier.h"


//Makes a mesh simpler by joining the two ends of an edge, the one that changes its shape least first, for as long as that
//keeps its corners within fMaxError of the planes of the faces they came from.  It is quadric edge collapse (Garland and
//Heckbert 1997, with their planes along open edges from 1998) with probabilistic quadrics (Trettner and Kobbelt 2020), so
//a flat area has a best position as readily as a curved one.
//
//fMaxError isn't a limit on how far a surface moves, as the middle of a large new face can be further from what was there
//than its corners are.  Only connected corners join, so parts that don't touch stay apart and a closed part keeps at least
//four faces.  Corners that are only near each other aren't connected: use CMeshWeldCorners first on what's exported from
//CAD.  A face keeps its colour, name and the normals of its corners.
class CMeshEdgeCollapse : public CMeshModifier
{
CONSTRUCTABLE(CMeshEdgeCollapse);
protected:
	float32		mfMaxError;  //In the mesh's own units.

public:
	void		Init(float32 fMaxError);

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;
};


#endif // __MESH_EDGE_COLLAPSE_H__
