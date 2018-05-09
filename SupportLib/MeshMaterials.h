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
#ifndef __MESH_MATERIALS_H__
#define __MESH_MATERIALS_H__
#include "BaseLib/ArrayInt.h"
#include "MeshDetail.h"


class CMeshMaterials : public CMeshDetail
{
BASE_FUNCTIONS(CMeshMaterials);
public:
	CArrayInt	mcMaterials;  //mcMaterials.NumElements is the number of materials in this mesh.  It maps 0 -> A, 1 -> B, 2 -> C
							  //where A, B, C etc... are indexes in the Material Tracker.
							  //This is a default group of materials rather than the actual list of materials that appears on the Mesh Instance.
	CArrayInt	mcFaces;  //mcFaces.NumElements == mpcMesh->mcFaces.NumElements.  The int is an index lookup into the MaterialTracker.

	void 	Init(void);
	void	ReInit(void);
	void 	Kill(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
	void	AddFace(void);
};


#endif // __MESH_MATERIALS_H__

