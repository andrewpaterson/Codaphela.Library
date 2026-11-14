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
#ifndef __MESH_SKIN_H__
#define __MESH_SKIN_H__
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/Array.h"
#include "MeshDetail.h"
#include "Skeleton.h"


struct SSkinWeight
{
	float32	fWeight;
	int		iMatrix;  //The skin's own matrix: an index in mcInverseSkinMatricies and mapBones.
};


int CompareSkinWeight(const void* arg1, const void* arg2);


typedef CArrayTemplateEmbedded<SSkinWeight, 4>	CArrayVertWeight;


//Each one of these represents one vertex in the mesh.  The sum of each of the vert weights must equal one.
//  THE WEIGHTS MUST BE SORTED largest to smallest and not zero.  Call SortSkinWeights on new verts to be sure.
class CMeshSkinVert
{
public:
	CArrayVertWeight	mcWeights;

	void 	Init(void);
	void 	Kill(void);
	bool	Load(CFileReader* pcFile);
	bool	Save(CFileWriter* pcFile);
	void	SortSkinWeights(void);
};


typedef CArrayLargeTemplate<CMeshSkinVert> CArrayMeshSkinVert;


//This class can be applied to both positions and normals because the postion/normal index is implicit.
//
//Matrix i is mapBones[i], a bone of a skeleton, shared like the mesh.  An instance's connections playing those bones are where it's
//posed, and mcInverseSkinMatricies[i] brings a position to bone i's space as the mesh was bound: not, in general, its rest pose.
class CMeshSkin : public CMeshDetail
{
CONSTRUCTABLE(CMeshSkin);
DESTRUCTABLE(CMeshSkin);
public:
	CArrayMeshSkinVert	mcSkinVerts;  //mcSkinVerts.NumElements == mpcMesh->mcConnectivity.mcCorners.NumElements.  
									  //And there is a one to one mapping between corners and positions thus:
									  //mcSkinVerts[3] is applied to mpcMesh->mcPositions.mcPositions[3].
	
	CArrayFloat4x4		mcInverseSkinMatricies;  //Matricies to bring the object spaces verts to local matrix space.
	CArray<CBone>		mapBones;  //The bone of each matrix.

	void 	Init(void);
	void	Clear(void);
	void 	Free(void) override;
	void	Class(void) override;

	void	KillSkinVerts(void);
	
	bool	Load(CObjectReader* pcFile) override;
	bool	Save(CObjectWriter* pcFile) override;

	void	AddCorner(void);
	int		GetFaceMatrixCount(int iVert1, int iVert2, int iVert3);
	void	PrivateAddMatricies(CMeshSkinVert* pcVert, CArrayInt* paiMatricies);
};


#endif // __MESH_SKIN_H__

