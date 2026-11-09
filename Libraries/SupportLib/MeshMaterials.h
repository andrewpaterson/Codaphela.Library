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
#ifndef __MESH_MATERIALS_H__
#define __MESH_MATERIALS_H__
#include "BaseLib/ArrayInt.h"
#include "Material.h"
#include "MeshDetail.h"


//mapMaterials are the mesh's default materials, which an instance can replace.  mcFaces has a slot in them for each of the mesh's
//faces, -1 for none.
class CMeshMaterials : public CMeshDetail
{
CONSTRUCTABLE(CMeshMaterials);
DESTRUCTABLE(CMeshMaterials);
public:
	CArrayMaterial	mapMaterials;
	CArrayInt		mcFaces;

	void 	Init(void);
	void	Class(void) override;
	void 	Free(void) override;

	void	Clear(void);

	bool	Load(CObjectReader* pcFile) override;
	bool	Save(CObjectWriter* pcFile) override;

	void	AddFace(void);
};


#endif // __MESH_MATERIALS_H__

