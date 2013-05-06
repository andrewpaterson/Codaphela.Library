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
#ifndef __MESH_INDEXER_H__
#define __MESH_INDEXER_H__
#include "MeshTypeIndices.h"


class CMeshCacher
{
public:
	CMesh*						mpcMesh;
	CArrayMeshTypeIndices		mcTypeIndices;

	void	Init(CMesh* pcMesh);
	void	Kill(void);

	BOOL	Index(void);
	BOOL	Vertex(void);

private:
	int		GetIndexSize(SMeshFaceType* psFaceType);
	void	PopulatePositions(CMeshVertexArray* pcVertexArray, CMeshTypeIndices* pcTypeIndices);
	void	PopulateNormals(CMeshVertexArray* pcVertexArray, CMeshTypeIndices* pcTypeIndices);
	void	PopulateColours(CMeshVertexArray* pcVertexArray, CMeshTypeIndices* pcTypeIndices);
	void	PopulateUVs(CMeshVertexArray* pcVertexArray, CMeshTypeIndices* pcTypeIndices);
	void	PopulateSkin(CMeshVertexArray* pcVertexArray, CMeshTypeIndices* pcTypeIndices);
};


#endif // __MESH_INDEXER_H__

