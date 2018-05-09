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
#ifndef __MESH_VERTEX_COLOURS_H__
#define __MESH_VERTEX_COLOURS_H__
#include "BaseLib/ArrayInt.h"
#include "MeshDetail.h"


struct SMeshFaceColour
{
	int		aiCornerColours[NUM_FACE_CORNERS];
};


typedef CArrayTemplate<SMeshFaceColour>	CArrayMeshFaceColours;


class CMeshColours : public CMeshDetail
{
BASE_FUNCTIONS(CMeshColours);
public:
	CArrayInt				mcColours;
	CArrayMeshFaceColours	mcFaces;  //mcFaces.NumElements == mpcMesh->mcFaces.NumElements.

	void 	Init(void);
	void	ReInit(void);
	void 	Kill(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);

	void	AddFace(void);

	void	IndexFaceCorners(int* paiDest, int iStride, CArrayInt* paiSourceFaces);
};


#endif // __MESH_VERTEX_COLOURS_H__

