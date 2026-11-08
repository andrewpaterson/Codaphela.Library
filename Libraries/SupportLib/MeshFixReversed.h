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
#ifndef __MESH_FIX_REVERSED_H__
#define __MESH_FIX_REVERSED_H__
#include "MeshModifier.h"


//Turns round the faces that wind against their own corner normals, which back face culling would leave as holes.
//A face whose corners have no normals of their own can't be reversed: its flat normal comes from how it winds.
class CMeshFixReversed : public CMeshModifier
{
CONSTRUCTABLE(CMeshFixReversed);
protected:
	size		muiNumFixed;

public:
	void		Init(void);

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;

	size		NumFixed(void);

protected:
	bool		IsReversed(CMesh* pcMesh, int iFace);
	bool		HasReversed(CMesh* pcMesh);
	Ptr<CMesh>	Fix(CMesh* pcMesh);
};


#endif // __MESH_FIX_REVERSED_H__
