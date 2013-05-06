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
#include "SupportLib/MeshEditor.h"
#include "SupportLib/Scene.h"
#include "MeshSelectionConverter.h"
#include "World.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::Init(CScene* pcScene, CWorld* pcWorld)
{
	mpcWorld = pcWorld;
	mpcScene = pcScene;

	mfZOffset = 0.5f;
	mfNormalLength = 4.0f;
	mbNormals = FALSE;
	mbVerticies = FALSE;
	mbEdges = TRUE;
	mbFaces = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::ConvertFaceSelection(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours)
{
	if (mbFaces)
	{
		PrivateConvert(pcGraphicsObject, pcGraphicsInstance, iMaterialIndex, pcMeshEditor, pcSelectionColours);
		AddSelectedFaces();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::ConvertEdgeSelection(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours)
{
	if (mbEdges)
	{
		PrivateConvert(pcGraphicsObject, pcGraphicsInstance, iMaterialIndex, pcMeshEditor, pcSelectionColours);
		AddSelectedEdges();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::ConvertNormalSelection(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours)
{
	if (mbNormals)
	{
		//PrivateConvert(pcGraphicsObject, pcGraphicsInstance, iMaterialIndex, pcMesh, pcSelectionColours);
		//AddSelectedNormals();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::ConvertVertSelection(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours)
{
	if (mbVerticies)
	{
		PrivateConvert(pcGraphicsObject, pcGraphicsInstance, iMaterialIndex, pcMeshEditor, pcSelectionColours);
		AddSelectedVerts();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::ConvertBoundingbox(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours)
{
	//PrivateConvert(pcGraphicsObject, pcGraphicsInstance, iMaterialIndex, pcMesh, pcSelectionColours);
	//AddBoundingBox();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::PrivateConvert(CGraphicsObject* pcGraphicsObject, CGraphicsInstance* pcGraphicsInstance, int iMaterialIndex, CMeshEditor* pcMeshEditor, CSelectionColourArray* pcSelectionColours)
{
	mpcObject = pcGraphicsObject;
	mpcMeshEditor = pcMeshEditor;
	mpcSelectionColours = pcSelectionColours;
	mpcInstance = pcGraphicsInstance;
	miMaterialIndex = iMaterialIndex;

	mpsCameraView = &mpcWorld->GetCurrentCamera()->GetViewTransform()->sD3DMatrix;
	mpsCameraPos = mpcWorld->GetCurrentCamera()->GetWorldTransform()->sD3DMatrix.Pos();
	mpsCameraDir = mpcWorld->GetCurrentCamera()->GetWorldTransform()->sD3DMatrix.At();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::AddSelectedFaces(void)
{
	int					iFaceNum;
	CMeshFace*			psFace;	
	SSelectionColour*	psSelectionColour;
	int					iIndex;
	int					iCornerOffset;
	CMeshCorner*		psCorner;
	int					iNumFaces;
	int					iStartVertex;
	CArrayInt			aiSkinIndices;
	int					iColour;
	BOOL				bSelected;

	iStartVertex = mpcObject->GetNumberOfVerticesOfType(D3DFVF_CVERTEX);
	iNumFaces = 0;

	aiSkinIndices.Init(8);
	for (iFaceNum = 0; iFaceNum < mpcMeshEditor->mcSelections.mcFaces.NumElements(); iFaceNum++)
	{
		psFace = mpcMeshEditor->mpcMesh->GetFace(iFaceNum);
		bSelected = FixBool(mpcMeshEditor->mcSelections.mcFaces.Get(iFaceNum));
		iIndex = mpcSelectionColours->FindWithIntKey(bSelected, 0);
		iNumFaces++;

		if (iIndex != -1)
		{
			psSelectionColour = mpcSelectionColours->Get(iIndex);
			iColour = psSelectionColour->iColour;
		}
		else
		{
			iColour = 0xff808080;
		}

		for (iCornerOffset = 0; iCornerOffset < NUM_FACE_CORNERS; iCornerOffset++)
		{
			psCorner = mpcMeshEditor->mpcMesh->GetCorner(psFace->asCorner[iCornerOffset]);
			AssignCorner(psFace->asCorner[iCornerOffset], iColour, mfZOffset, &aiSkinIndices);
		}
	}
	aiSkinIndices.Kill();

	mpcObject->GrowPrimitive(iNumFaces, D3DPT_TRIANGLELIST, iNumFaces * 3, 0, miMaterialIndex, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::AddSelectedEdges(void)
{
	int					iEdgeNum;
	CMeshEdge*			psEdge;	
	SSelectionColour*	psSelectionColour;
	int					iIndex;
	int					iCornerOffset;
	CMeshCorner*		psCorner;
	int					iNumEdges;
	int					iStartVertex;
	CArrayInt			aiSkinIndices;
	BOOL				bSelected;

	iStartVertex = mpcObject->GetNumberOfVerticesOfType(D3DFVF_CVERTEX);
	iNumEdges = 0;

	aiSkinIndices.Init(8);
	for (iEdgeNum = 0; iEdgeNum < mpcMeshEditor->mcSelections.mcEdges.NumElements(); iEdgeNum++)
	{
		bSelected = FixBool(mpcMeshEditor->mcSelections.mcEdges.Get(iEdgeNum));
		iIndex = mpcSelectionColours->FindWithIntKey(bSelected, 0);
		if (iIndex != -1)
		{
			psEdge = mpcMeshEditor->mpcMesh->GetEdge(iEdgeNum);
			iNumEdges++;
			psSelectionColour = mpcSelectionColours->Get(iIndex);
			for (iCornerOffset = 0; iCornerOffset < NUM_EDGE_CORNERS; iCornerOffset++)
			{
				psCorner = mpcMeshEditor->mpcMesh->GetCorner(psEdge->aiCorner[iCornerOffset]);
				AssignCorner(psEdge->aiCorner[iCornerOffset], psSelectionColour->iColour, mfZOffset*2, &aiSkinIndices);
			}
		}
	}
	aiSkinIndices.Kill();

	mpcObject->GrowPrimitive(iNumEdges, D3DPT_LINELIST, iNumEdges * 2, 0, miMaterialIndex, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//void CMeshSelectionConverter::AddSelectedNormals(void)
//{
//	int				iCornerNum;
//	int				iIndex;
//	CMeshCorner*	psCorner;
//	int				iNumNormals;
//	int				iStartVertex;
//	CArrayInt		aiSkinIndices;
//	D3DCVERTEX*		psVertex;
//	int				iPosition;
//	SFloat3*		psPosition;
//	int				iNormal;
//	SFloat3*		psNormal;
//	CArrayInt		aiNormalIndices;
//	SFloat3			sTemp;
//
//
//	iStartVertex = mpcObject->GetNumberOfVerticesOfType(D3DFVF_CVERTEX);
//	iNumNormals = 0;
//
//	aiSkinIndices.Init(8);
//	aiNormalIndices.Init(8);
//
//	for (iCornerNum = 0; iCornerNum < mpcMeshEditor->mcCorners.NumElements(); iCornerNum++)
//	{
//		psCorner = mpcMeshEditor->GetCorner(iCornerNum);
//		psCorner->asType.GetIndicesForTypeInGroup(miSkinVertID, 0, &aiSkinIndices);
//		iPosition = psCorner->asType.GetIndexForTypeInGroup(miPositionID, 0, 0);
//		psPosition = (SFloat3*)(mpcMeshEditor->GetDataArray(miPositionID)->mcData.Get(iPosition));
//		psCorner->asType.GetIndicesForTypeInGroup(miNormalID, 0, &aiNormalIndices);
//		for (iIndex = 0; iIndex < aiNormalIndices.NumElements(); iIndex++)
//		{
//			iNormal = aiNormalIndices.GetValue(iIndex);
//			psNormal = (SFloat3*)(mpcMeshEditor->GetDataArray(miNormalID)->mcData.Get(iNormal));
//			psVertex = (D3DCVERTEX*)mpcObject->AddDynamicVertex(D3DFVF_CVERTEX);
//			if (aiSkinIndices.NumElements() == 0)
//			{
//				AssignVertex(psVertex, psPosition, 0x80ff80, 0.0f, mpcInstance->GetMatrix(0));
//			}
//			else
//			{
//				AssignSkinnedVertex(psVertex, psPosition, 0x80ff80, 0.0f, &aiSkinIndices);
//			}
//			sTemp = *psNormal;
//			sTemp *= mfNormalLength;
//			sTemp += *psPosition;
//			psVertex = (D3DCVERTEX*)mpcObject->AddDynamicVertex(D3DFVF_CVERTEX);
//			if (aiSkinIndices.NumElements() == 0)
//			{
//				AssignVertex(psVertex, &sTemp, 0x80ff80, 0.0f, mpcInstance->GetMatrix(0));
//			}
//			else
//			{
//				AssignSkinnedVertex(psVertex, &sTemp, 0x80ff80, 0.0f, &aiSkinIndices);
//			}
//			iNumNormals++;
//		}
//	}
//
//	aiNormalIndices.Kill();
//	aiSkinIndices.Kill();
//
//	mpcObject->GrowPrimitive(iNumNormals, D3DPT_LINELIST, iNumNormals * 2, 0, miMaterialIndex, 0, 0);
//}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::AddSelectedVerts(void)
{
	SSelectionColour*	psSelectionColour;
	int					iIndex;
	int					iCornerNum;
	int					iNumCorners;
	int					iStartVertex;
	CArrayInt			aiSkinIndices;
	BOOL				bSelected;
	
	iStartVertex = mpcObject->GetNumberOfVerticesOfType(D3DFVF_CVERTEX);
	iNumCorners = 0;

	aiSkinIndices.Init(8);
	for (iCornerNum = 0; iCornerNum < mpcMeshEditor->mcSelections.mcVerts.NumElements(); iCornerNum++)
	{
		bSelected = FixBool(mpcMeshEditor->mcSelections.mcVerts.Get(iCornerNum));
		iIndex = mpcSelectionColours->FindWithIntKey(bSelected, 0);
		if (iIndex != -1)
		{
			iNumCorners++;
			psSelectionColour = mpcSelectionColours->Get(iIndex);

			AssignCorner(iCornerNum, psSelectionColour->iColour, mfZOffset*3, &aiSkinIndices);
		}
	}
	aiSkinIndices.Kill();

	mpcObject->GrowPrimitive(iNumCorners, D3DPT_POINTLIST, iNumCorners, 0, miMaterialIndex, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::AssignCorner(int iPosition, int iColour, float fZOffset, CArrayInt* paiSkinIndices)
{
	SFloat3*		psPosition;
	D3DCVERTEX*		psVertex;
	SFloat3			sPos;
	SFloat3			sTemp;
	CMeshSkinVert*	pcSkinVert;

	psPosition = mpcMeshEditor->mpcMesh->GetPosition(iPosition);
	psVertex = (D3DCVERTEX*)mpcObject->AddDynamicVertex(D3DFVF_CVERTEX);

	if (!mpcMeshEditor->mpcMesh->mcSkin.InUse())
	{
		AssignVertex(psVertex, psPosition, iColour, fZOffset, mpcInstance->GetMatrix(0));
	}
	else
	{
		pcSkinVert = mpcMeshEditor->mpcMesh->mcSkin.mcSkinVerts.Get(iPosition);
		AssignSkinnedVertex(psVertex, psPosition, iColour, fZOffset, pcSkinVert);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//void CMeshSelectionConverter::AddBoundingBox(void)
//{
//	int					iNumPrimitives;
//	int					iVertexBuffer;
//	int					iMaterialID;
//	CBoxIndexed*		pcBox;
//	SFloat3			sCorners[8];
//	SFloat3*			psCenter;
//
//	iNumPrimitives = 0;
//	iMaterialID = 1;
//	iVertexBuffer = mpcObject->GetVertexBufferIndex(D3DFVF_CVERTEX);
//
//	pcBox = (CBoxIndexed*)NULL;
//	psCenter = (SFloat3*)mpcMeshEditor->GetDataElement(miPositionID, pcBox->miCenterIndex);
//
//	//front, top, left
//	sCorners[0].x = pcBox->msMinimum.x;
//	sCorners[0].y = pcBox->msMinimum.y;
//	sCorners[0].z = pcBox->msMinimum.z;
//
//	//front, top, right
//	sCorners[1].x = pcBox->msMaximum.x;
//	sCorners[1].y = pcBox->msMinimum.y;
//	sCorners[1].z = pcBox->msMinimum.z;
//
//	//back, top, right
//	sCorners[2].x = pcBox->msMaximum.x;
//	sCorners[2].y = pcBox->msMinimum.y;
//	sCorners[2].z = pcBox->msMaximum.z;
//
//	//back, top, left
//	sCorners[3].x = pcBox->msMinimum.x;
//	sCorners[3].y = pcBox->msMinimum.y;
//	sCorners[3].z = pcBox->msMaximum.z;
//
//	//front, bottom, left
//	sCorners[4].x = pcBox->msMinimum.x;
//	sCorners[4].y = pcBox->msMaximum.y;
//	sCorners[4].z = pcBox->msMinimum.z;
//
//	//front, bottom, right
//	sCorners[5].x = pcBox->msMaximum.x;
//	sCorners[5].y = pcBox->msMaximum.y;
//	sCorners[5].z = pcBox->msMinimum.z;
//
//	//back, bottom, right
//	sCorners[6].x = pcBox->msMaximum.x;
//	sCorners[6].y = pcBox->msMaximum.y;
//	sCorners[6].z = pcBox->msMaximum.z;
//
//	//back, bottom, left
//	sCorners[7].x = pcBox->msMinimum.x;
//	sCorners[7].y = pcBox->msMaximum.y;
//	sCorners[7].z = pcBox->msMaximum.z;
//
//	iNumPrimitives = 0;
//	iNumPrimitives += AddBrokenLine(&sCorners[0], &sCorners[1]);
//	iNumPrimitives += AddBrokenLine(&sCorners[1], &sCorners[2]);
//	iNumPrimitives += AddBrokenLine(&sCorners[2], &sCorners[3]);
//	iNumPrimitives += AddBrokenLine(&sCorners[3], &sCorners[0]);
//
//	iNumPrimitives += AddBrokenLine(&sCorners[4], &sCorners[5]);
//	iNumPrimitives += AddBrokenLine(&sCorners[5], &sCorners[6]);
//	iNumPrimitives += AddBrokenLine(&sCorners[6], &sCorners[7]);
//	iNumPrimitives += AddBrokenLine(&sCorners[7], &sCorners[4]);
//
//	iNumPrimitives += AddBrokenLine(&sCorners[0], &sCorners[4]);
//	iNumPrimitives += AddBrokenLine(&sCorners[1], &sCorners[5]);
//	iNumPrimitives += AddBrokenLine(&sCorners[2], &sCorners[6]);
//	iNumPrimitives += AddBrokenLine(&sCorners[3], &sCorners[7]);
//
//	mpcObject->GrowPrimitive(iNumPrimitives, D3DPT_LINELIST, iNumPrimitives*2, iVertexBuffer, miMaterialIndex, 0);
//}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::AssignVertex(D3DCVERTEX* psVertex, SFloat3* psPosition, unsigned int iColour, float fZOffset, SMatrix* psMatrix)
{
	SFloat3			sPos;
	SFloat3			sDir;

	Float3TransformCoord((SFloat3*)&sPos, psPosition, &psMatrix->sD3DMatrix);
	if (fZOffset != 0.0f)
	{
		sDir = -(*mpsCameraDir);

		Float3Subtract(&sDir, mpsCameraPos, &sPos);
		Float3Normalize(&sDir, &sDir);
		Float3Scale(&sDir, &sDir, fZOffset);
		Float3Add(&sPos, &sPos, &sDir);
	}
	*((SFloat3*)psVertex) = sPos;
	psVertex->color = iColour;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::AssignSkinnedVertex(D3DCVERTEX* psVertex, SFloat3* psPosition, unsigned int iColour, float fZOffset, CMeshSkinVert* pcSkinVert)
{
	SSkinWeight*	psSkinWeight;
	int				i;
	SMatrix*		psMatrix;
	SFloat3		sPos;
	SFloat3		sTemp;
	SFloat4x4*		psAdjMatrix;

	sPos.Zero();
	for (i = 0; i < pcSkinVert->mcWeights.NumElements(); i++)
	{
		psSkinWeight = pcSkinVert->mcWeights.Get(i);
		psMatrix = mpcInstance->GetMatrix(psSkinWeight->iMatrix);
		if (mpcInstance->GetGraphicsObject()->GetAdjMatricies()->IsNotEmpty())
		{
			psAdjMatrix = mpcInstance->GetGraphicsObject()->GetAdjMatricies()->Get(psSkinWeight->iMatrix);
			Float3TransformCoord(&sTemp, psPosition, psAdjMatrix);
			Float3TransformCoord(&sTemp, &sTemp, &psMatrix->sD3DMatrix);
		}
		else
		{
			Float3TransformCoord(&sTemp, psPosition, &psMatrix->sD3DMatrix);
		}
		sPos += sTemp * psSkinWeight->fWeight;
	}
	AssignTransformedVertex(psVertex, &sPos, iColour, fZOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::AssignTransformedVertex(D3DCVERTEX* psVertex, SFloat3* psPosition, unsigned int iColour, float fZOffset)
{
	SFloat3			sPos;
	SFloat3			sDir;

	if (fZOffset != 0.0f)
	{
		Float3Subtract(&sDir, mpsCameraPos, (SFloat3*)psPosition);
		Float3Normalize(&sDir, &sDir);
		Float3Scale(&sDir, &sDir, fZOffset);
		*((SFloat3*)psVertex) = *((SFloat3*)psPosition) + sDir;
	}
	else
	{
		*((SFloat3*)psVertex) = *((SFloat3*)psPosition);
	}
	psVertex->color = iColour;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelectionConverter::FinalisePrimitives(void)
{
	int						i;
	CGraphicsPrimitive*		pcPrimitive;

	for (i = 0; i < mpcObject->GetPrimitives()->NumElements(); i++)
	{
		pcPrimitive = mpcObject->GetPrimitive(i);
		pcPrimitive->miFlags = GRAPH_PRIM_FLAGS_DRAW;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshSelectionConverter::IsNormalFacingCamera(SFloat3* psNormal)
{
	return TRUE;

	//SFloat3	cOut;

	//if (psNormal)
	//{
	//	Float3TransformNormal(&cOut, (SFloat3*)psNormal, mpsCameraView);
	//	return cOut.z <= 0;
	//}
	//return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSelectionConverter::AddBrokenLine(SFloat3* psStart, SFloat3* psEnd, SMatrix* psMatrix)
{
	D3DCVERTEX*			psVertex;
	SFloat3			s1;
	SFloat3			s2;

	Float3InterpolatePosition((SFloat3*)&s1, (SFloat3*)psStart, (SFloat3*)psEnd, 0.9f);
	Float3InterpolatePosition((SFloat3*)&s2, (SFloat3*)psStart, (SFloat3*)psEnd, 0.1f);

	psVertex = (D3DCVERTEX*)mpcObject->AddDynamicVertex(D3DFVF_CVERTEX);
	AssignVertex(psVertex, psStart, 0xffffffff, 0.0f, psMatrix);
	psVertex = (D3DCVERTEX*)mpcObject->AddDynamicVertex(D3DFVF_CVERTEX);
	AssignVertex(psVertex, &s1, 0xffffffff, 0.0f, psMatrix);

	psVertex = (D3DCVERTEX*)mpcObject->AddDynamicVertex(D3DFVF_CVERTEX);
	AssignVertex(psVertex, &s2, 0xffffffff, 0.0f, psMatrix);
	psVertex = (D3DCVERTEX*)mpcObject->AddDynamicVertex(D3DFVF_CVERTEX);
	AssignVertex(psVertex, psEnd, 0xffffffff, 0.0f, psMatrix);

	return 2;
}

