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
#ifndef __MESH_FLAT_FACES_H__
#define __MESH_FLAT_FACES_H__
#include "BaseLib/ArrayInt.h"
#include "MeshModifier.h"


class CMeshFlatFaces : public CMeshModifier
{
CONSTRUCTABLE(CMeshFlatFaces);
public:
	void 	Init(void);
	void 	Kill(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);

	void	GenerateNormals(CMesh* pcMesh);
	void	Apply(CMeshEditor* pcMeshEditor);
};


#endif // __MESH_FLAT_FACES_H__

