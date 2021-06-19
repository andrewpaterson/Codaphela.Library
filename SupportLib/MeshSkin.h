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
#ifndef __MESH_SKIN_H__
#define __MESH_SKIN_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/GeometricTypes.h"
#include "MeshDetail.h"


struct SSkinWeight
{
	float	fWeight;
	int		iMatrix;  //0 ... miNumMatricies-1.  This is NOT a lookup into the ConnectionTracker.
};


int CompareSkinWeight(const void* arg1, const void* arg2);


typedef CArrayTemplateMinimal<SSkinWeight>	CArrayVertWeight;


//Each one of these represents one vertex in the mesh.  The sum of each of the vert weights must equal one.
// !! THE WEIGHTS MUST BE SORTED largest to smallest and not zero.  Call SortSkinWeights on new verts to be sure.
class CMeshSkinVert
{
public:
	CArrayVertWeight	mcWeights;

	void 	Init(void);
	void 	Kill(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
	void	SortSkinWeights(void);
};


typedef CArrayTemplate<CMeshSkinVert> CArrayMeshSkinVert;


//This class can be applied to both positions and normals because the postion/normal index is implicit.
class CMeshConnectivity;
class CMeshSkin : public CMeshDetail
{
CONSTRUCTABLE(CMeshSkin);
public:
	CArrayMeshSkinVert	mcSkinVerts;  //mcSkinVerts.NumElements == mpcMesh->mcConnectivity.mcCorners.NumElements.  
									  //And there is a one to one mapping between corners and positions thus:
									  //mcSkinVerts[3] is applied to mpcMesh->mcPositions.mcPositions[3].
	
	CArrayFloat4x4		mcInverseSkinMatricies;  //Matricies to bring the object spaces verts to local matrix space.

	void 	Init(void);
	void	ReInit(void);
	void 	Kill(void);
	void	KillSkinVerts(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
	void	Touch(void);
	void	AddCorner(void);
	int		GetFaceMatrixCount(int iVert1, int iVert2, int iVert3);
	int		GetFaceVertexMaxMatrixCount(int iVert1, int iVert2, int iVert3);
	void	PrivateAddMatricies(CMeshSkinVert* pcVert, CArrayInt* paiMatricies);
	void	IndexFaceCorners(int* paiDest, int iStride, CArrayInt* paiSourceFaces, CMeshConnectivity* pcConn);
};


#endif // !__MESH_SKIN_H__

