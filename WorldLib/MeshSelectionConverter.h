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
#ifndef __MESH_SELECTION_CONVERTER_H__
#define __MESH_SELECTION_CONVERTER_H__
#include "BaseLib/ArrayTemplate.h"
#include "SelectionOverrides.h"
#include "WorldLib/GraphicsObject.h"
#include "DirectXLib/FlexibleVertexFormats.h"
#include "GraphicsInstance.h"


class CWorld;
class CScene;
class CMeshEditor;
class CMeshSkinVert;
class CMeshSelectionConverter
{
private:
	void PrivateConvert(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours);

public:
	CSelectionColourArray*	mpcSelectionColours;
	CMeshEditor*			mpcMeshEditor;
	CWorld*					mpcWorld;
	CScene*					mpcScene;
	CGraphicsObject*		mpcObject;
	CGraphicsInstance*		mpcInstance;
	SFloat4x4*				mpsCameraView;
	int						miMaterialIndex;
	SFloat3*				mpsCameraPos;
	SFloat3*				mpsCameraDir;
	float					mfZOffset;
	float					mfNormalLength;
	BOOL					mbNormals;
	BOOL					mbVerticies;
	BOOL					mbEdges;
	BOOL					mbFaces;

	void	Init(CScene* pcScene, CWorld* pcWorld);
	void	Kill(void);

	void 	ConvertFaceSelection(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours);
	void 	ConvertEdgeSelection(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours);
	void 	ConvertNormalSelection(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours);
	void 	ConvertVertSelection(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours);
	void	ConvertBoundingbox(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours);
	void 	AddSelectedFaces(void);
	void 	AddSelectedEdges(void);
//	void 	AddSelectedNormals(void);
	void	AddSelectedVerts(void);
//	void	AddBoundingBox(void);
	void	AddMatrices(void);
	void	AssignCorner(int iPosition, int iColour, float fZOffset, CArrayInt* pacSkinIndices);
	void 	AssignVertex(D3DCVERTEX* psVertex, SFloat3* psPosition, unsigned int iColour, float fZOffset, SMatrix* psMatrix);
	void	AssignSkinnedVertex(D3DCVERTEX* psVertex, SFloat3* psPosition, unsigned int iColour, float fZOffset, CMeshSkinVert* pcSkinVert);
	void 	AssignTransformedVertex(D3DCVERTEX* psVertex, SFloat3* psPosition, unsigned int iColour, float fZOffset);
	void	FinalisePrimitives(void);
	BOOL	IsNormalFacingCamera(SFloat3* psNormal);
	int		AddBrokenLine(SFloat3* psStart, SFloat3* psEnd, SMatrix* psMatrix);
};


#endif //__MESH_SELECTION_CONVERTER_H__

