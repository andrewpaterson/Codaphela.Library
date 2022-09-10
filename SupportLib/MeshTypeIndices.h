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
#ifndef __MESH_TYPE_INDICES_H__
#define __MESH_TYPE_INDICES_H__
#include "MeshDefines.h"
#include "MeshIndexArray.h"
#include "Mesh.h"


class CMeshTypeIndices
{
public:
	int						miStride;
	CMeshVertexArray*		mpcVertexArray;
	CArrayBlock				maVerts;  //'Vertex Buffer',  however each vertex contains indcies into the data rather than the data itself.
	int						miNormalOffset;
	int						miColourOffset;
	int						miUVOffset;
	int						miSkinOffset;

	void	Init(CMeshVertexArray* pcVertexArray, int iStride, int iNormalOffset, int iColourOffset, int iUVOffset, int iSkinOffset);
	void	Kill(void);

	bool	Generate(CArrayInt* paiFaces, void* pavFaceIndices);
	void	SetVertIndex(int* piVertIndex, void* pvFaceVertIndex);
	int		FindExisingVert(int* piFaceVertIndex);

	int		GetPositionIndex(int iVert);
	int		GetNormalIndex(int iVert);
	int		GetColourIndex(int iVert);
	int		GetUVIndex(int iVert, int iLayer);
	int		GetSkinIndex(int iVert);

	int		FindExistingVert(int* piFaceVertIndex);
	int		NumVerts(void);
};


typedef CArrayTemplate<CMeshTypeIndices>	CArrayMeshTypeIndices;


#endif // !__MESH_TYPE_INDICES_H__

