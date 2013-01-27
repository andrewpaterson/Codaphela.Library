/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __MESH_H__
#define __MESH_H__
#include "StandardLib/NamedObject.h"
#include "ShapesIndexed.h"
#include "MeshFaceReturn.h"
#include "MeshPositions.h"
#include "MeshConnectivity.h"
#include "MeshNormals.h"
#include "MeshUVCoords.h"
#include "MeshSkin.h"
#include "MeshMaterials.h"
#include "MeshColours.h"
#include "MeshFaceTypes.h"
#include "MeshCache.h"


class CMesh : public CNamedObject
{
BASE_FUNCTIONS(CMesh);
protected:
	CMeshPositions		mcPositions;		
	CMeshConnectivity	mcConnectivity;		

public:
	CMeshNormals		mcNormals;			
	CMeshUVCoords		mcUVs;
	CMeshMaterials		mcMaterials;		
	CMeshColours		mcColours;			
	CMeshSkin			mcSkin;				

	CMeshFaceTypes		mcFaceTypes;		//Touched, Optional: Normals, Material, Skin, UVCoords
	CMeshCache			mcCache;			//Touched, Depends: FaceTypes

	void			Init(void);
	void 			Init(int iUniqueID, char* szName);
	void 			KillData(void);

	BOOL			Load(CObjectDeserialiser* pcFile);
	BOOL			Save(CObjectSerialiser* pcFile);
	void			Copy(CMesh* pcMesh);
	
	BOOL			Touch(void);

	int				NumCorners(void);
	int				NumFaces(void);
	int				NumEdges(void);
	int				TestNumPositions(void);  //For testing.  NumCorners should always be equal to NumPositions.

	int				FaceChunkSize(void);
	int				CornerChunkSize(void);
	int				TestPositionChunkSize(void);

	SFloat3*		GetPosition(int iIndex);
	CMeshCorner*	GetCorner(int iIndex);
	CMeshEdge*		GetEdge(int iIndex);
	CMeshFace*		GetFace(int iIndex);

	void			ReInitConnectivity(int iCornerChunkSize, int iFaceChunkSize);

	void 			AddCorner(float x, float y, float z);
	void			AddCorner(SFloat3* psPosition);

	CMFRet			AddFace(int iCorner1, int iCorner2, int iCorner3);
	CMFRet			AddQuad(int iCorner1, int iCorner2, int iCorner3, int iCorner4);
	CMFRet			AddPolygon(CArrayInt* paiCorners);
	
	void			RemoveFace(int iFace);
	void			RemoveFaces(CArrayInt* paiFaces);

	void			RemoveCorner(int iCorner);
	
	void			MovePosition(int iPosition, SFloat3* psPosition);
	void			MovePositions(CArrayInt* paiPositions, SFloat3* psPosition);

	void			AddUV(float u, float v, int iLayer = 0);
	void			AddNormal(float x, float y, float z);

	void			Dump(void);

	CMeshConnectivity*	GetConnectivity(void);

protected:
	void	PrivateInit(void);
};


#endif // __MESH_H__

