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
#ifndef __MESH_FACE_TYPES_H__
#define __MESH_FACE_TYPES_H__
#include "MeshDetail.h"


#define MESH_FACE_NORMAL_FLAG1		0x00000001
#define MESH_FACE_COLOUR_FLAG1		0x00000002
#define MESH_FACE_UV_MASK1			0x0000ff00
#define MESH_FACE_UV_SHIFT			8
#define MESH_FACE_MATRIX_MASK1		0x00ff0000
#define MESH_FACE_MATRIX_SHIFT		16

#define MESH_FACE_MATERIAL_MASK2	0x0000ffff
#define MESH_FACE_MATERIAL_SHIFT	0


struct SMeshFaceType
{
	uint32	iFlags1;
	uint32	iFlags2;

	void	Clear(void);
	bool	IsNormal(void);
	bool	IsColour(void);
	size	GetUVs(void);
	size	GetMatricies(void);
	int		GetMaterial(void);
	bool	Equals(SMeshFaceType* psOther);
	bool	Load(CFileReader* pcFile);
	bool	Save(CFileWriter* pcFile);
};


int CompareMeshFaceTypes(const void* arg1, const void* arg2);


typedef CArrayTemplate<SMeshFaceType>	CArrayMeshFaceType;


//This class is basically a cache for each unique type of face.  
//That is, the number of draw calls that must be made to draw this mesh.
class CMeshNormals;
class CMeshSkin;
class CMeshUVCoords;
class CMeshMaterials;
class CMeshConnectivity;
class CMeshColours;
class CMesh;
class CMeshFaceTypes : public CMeshDetail
{
CONSTRUCTABLE(CMeshFaceTypes);
DESTRUCTABLE(CMeshFaceTypes);
public:
	CArrayMeshFaceType	mcFaces;
	CArrayMeshFaceType	mcUniqueTypes;

	void	Init(void);
	void	Free(void);
	void	Class(void);

	bool	Load(CObjectReader* pcFile);
	bool	Save(CObjectWriter* pcFile);

	void	Touch(CMesh* pcMesh);

	void	ClearFaceTypes(int iNumFaces);
	void	GenerateFromNormals(CMeshNormals* pcNormals);
	void	GenerateFromSkin(CMesh* pcMesh);
	void	GenerateFromUVCoords(CMeshUVCoords* pcUVs);
	void	GenerateFromMaterals(CMeshMaterials* pcMaterials);
	void	GenerateFromColours(CMeshColours* pcColours);
	void	GenerateUniques(void);
	void	UpdateUniquesSkin(void);

	void	GetFacesForType(SMeshFaceType* psFaceType, CArrayInt* paiFaceIndices);
};


#endif // __MESH_FACE_TYPES_H__

