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
#ifndef __MESH_SIMPLIFY_H__
#define __MESH_SIMPLIFY_H__
#include "MeshModifier.h"


//Merges everything of a colour closer than a cell, for a mesh that is always drawn so small that a cell is about a
//pixel.  Each corner is the average of those in its cell and each merged normal the average of those in its cell that
//are close, so a sharp edge stays sharp and a curve is only faceted a little.  Two layers of different colours a hair
//apart, as a board's copper and solder mask are, stay that hair apart.
class CMeshSimplify : public CMeshModifier
{
CONSTRUCTABLE(CMeshSimplify);
protected:
	float32		mfCellSize;  //In the mesh's own units.

public:
	void		Init(float32 fCellSize);

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;
};


#endif // __MESH_SIMPLIFY_H__
