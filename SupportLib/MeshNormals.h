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
#ifndef __MESH_NORMALS_H__
#define __MESH_NORMALS_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/GeometricTypes.h"
#include "MeshDefines.h"
#include "MeshDetail.h"


struct SMeshNormalFace
{
	int		aiCornerNormals[NUM_FACE_CORNERS];
	int		iFaceNormal;
};


typedef CArrayTemplate<SMeshNormalFace>	CArrayMeshFaceNormal;


class CMeshSmoothGroups;
class CMeshConnectivity;
class CMeshPositions;
class CMesh;
class CMeshNormals : public CMeshDetail
{
BASE_FUNCTIONS(CMeshNormals);
public:
	CArrayFloat3			mcNormals;  //0 .. Flats, Flats+1 .. End
	CArrayMeshFaceNormal	mcFaces;  //mcFaces.NumElements == mpcMesh->mcFaces.NumElements

	void 		Init(void);
	void 		ReInit(void);
	void 		Kill(void);
	BOOL		Load(CFileReader* pcFile);
	BOOL		Save(CFileWriter* pcFile);

	void		GenerateFlatFaceNormals(CMeshPositions* pcPositions, CMeshConnectivity* pcConn);
	void		AddFace(int iFaceNum, CMeshPositions* pcPositions, CMeshConnectivity* pcConn);
	void		IndexFaceCorners(int* paiDest, int iStride, CArrayInt* paiSourceFaces);

	SFloat3*	GetFaceNormal(int iFace);
	SFloat3*	GetCornerNormal(int iCorner);
};


#endif // __MESH_NORMALS_H__

