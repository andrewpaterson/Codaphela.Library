/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __MESH_CONVERTER_H__
#define __MESH_CONVERTER_H__
#include "BaseLib/GeometricTypes.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshTypeIndices.h"
#include "SupportLib/Scene.h"
#include "WorldLib/GraphicsObject.h"
#include "WorldLib/MeshObject.h"
#include "SelectionOverrides.h"


class CWorld;


#define MAX_BONES 4


class CVertexBufferArrayFormat
{
public:
	CMeshVertexArrayArray	macMeshVertexArrays;
	SD3DVertexType*			mpsVertexType;
	int						miNumVerticies;

	void	Init(SD3DVertexType* psVertexType);
	void	Kill(void);
	void	Add(CMeshVertexArray* pcMeshVertexArray);
};


typedef CArrayTemplate<CVertexBufferArrayFormat> CArrayVertexBufferArrayFormat;


class CSceneConverter;
class CMeshConverter
{
protected:
	CSceneConverter*		mpcSceneConverter;
	CGraphicsObject*		mpcGraphicsObject;
	CMeshObject*			mpcMeshObject;
	int						miNumPositions;
	char					s[512];
	CMesh*					mpcMesh;
	CArrayPointer*			mpcConnectionsAndIndices;

	CArrayInt				maiMatrixIndices;

public:
	void				Init(CSceneConverter* pcSceneConverter);
	void 				Kill(void);
	BOOL				Convert(CGraphicsObject** pcGraphicsObject, CMeshObject** ppcMeshObject, CMesh* pcMesh, CArrayPointer* pcConnectionAndIndex);
	SD3DVertexType*		GetVertexFormatForFaceType(SMeshFaceType sFaceType);
	void				CollectVertexBuffersByFormat(CArrayVertexBufferArrayFormat* pcArray);
	BOOL				ConvertMesh(void);
	BOOL				ConvertMaterials(void);
	BOOL				CreateLinkNodes(void);
	void				FillUniqueMatricies(void);
	void				OptimiseGraphicsObject(void);
	BOOL				CreateIndexBuffer(void);
	BOOL				SetGraphicsPrimitive(int iPrimitive, int iMaterial, int iNumFaces, int iNumIndices, int iStartIndex, int iStartVertex, int iVertexBufferIndex);
	void				PopulatePositions(SD3DVertexType* psVertexType, void* pvFirstNewVert, CVertexArray* pcVertexArray);
	void				PopulateNormals(SD3DVertexType* psVertexType, void* pvFirstNewVert, CVertexArray* pcVertexArray);
	void				PopulateColours(SD3DVertexType* psVertexType, void* pvFirstNewVert, CVertexArray* pcVertexArray);
	void				PopulateUVs(SD3DVertexType* psVertexType, void* pvFirstNewVert, CVertexArray* pcVertexArray);
	void				PopulateVertexWeights(SD3DVertexType* psVertexType, void* pvFirstNewVert, CVertexArray* pcVertexArray);
	void				WeightAdjustIndices(int iNumWeights, SSkinWeight* psWeights, CMeshSkinVert* psSkinVert);
	BOOL				SetObjectMatricies(void);
};


#endif //__MESH_CONVERTER_H__


